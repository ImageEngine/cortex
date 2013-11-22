//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Weta Digital Limited. All rights reserved.
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include <list>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <sstream>
#include <fstream>

#include "DDImage/Pixel.h"
#include "DDImage/Tile.h"
#include "IECore/CachedReader.h"
#include "IECore/FastFloat.h"
#include "IECore/Reader.h"
#include "IECore/NumericParameter.h"
#include "boost/algorithm/string.hpp"
#include "boost/regex.hpp"
#include "IECoreNuke/LensDistort.h"

using namespace DD::Image;

namespace IECoreNuke
{

static const char* const CLASS = "ieLensDistort";

static const char* const HELP = "Applies or removes lens distortion from an input using a parameterised lens model.";

DD::Image::Op *LensDistort::build( Node *node ){ return new LensDistort( node ); }

const Iop::Description LensDistort::m_description( CLASS, LensDistort::build );

const char *LensDistort::Class() const { return m_description.name; }

const char *LensDistort::node_help() const { return HELP; }

const char ** LensDistort::modelNames()
{
	static bool init = false;
	static std::vector<std::string> lensModels = IECore::LensModel::lensModels();
	static const char ** cStrings = new const char*[lensModels.size()+1];
	
	if ( !init )
	{
		const unsigned int nLensModels( lensModels.size() );
		for ( unsigned int i = 0; i < nLensModels; i++ )
		{
			cStrings[i] = lensModels[i].c_str();
		}
		cStrings[lensModels.size()] = 0;
		init = true;
	}

	return cStrings;
}

int LensDistort::indexFromModelName( const char *name )
{
	const char** names = modelNames();
	for ( int idx = 0; names[idx] != 0; ++idx )
	{
		if (strcmp(names[idx], name) == 0) return idx;
	}
	return 0;
}

LensDistort::LensDistort( Node* node ) :
	Iop(node),
	m_useFileSequence( false ),
	m_hasValidFileSequence( false ),
	m_blackOutsideNode( NULL ),
	m_inputNode( NULL ),
	m_nThreads( std::max( 16u, Thread::numCPUs + Thread::numThreads + 1 ) ),
	m_assetPath( "" ),
	m_enableBlackOutside( false ),
	m_lensModel( 0 ),
	m_mode( Distort )
{
	// Work out the most threads we can have and create an instance of the lens model for each one.
	// This is useful for any lens model implementation that uses the ldpk library is as it is not thread safe.
	m_locks = new Lock[m_nThreads];
	m_lensModels.resize( m_nThreads, NULL );
	for( int i = 0; i < m_nThreads; ++i )
	{
		m_lensModels[i] = IECore::LensModel::create( modelNames()[0] );
	}
	
	memset( &m_knobData[0], 0, sizeof( double ) * IECORENUKE_LENSDISTORT_NUMBER_OF_STATIC_KNOBS );
}

void LensDistort::updateLensModel( bool updateKnobsFromParameters ) 
{
	if ( updateKnobsFromParameters ) m_pluginAttributes.clear(); // Give us a fresh start by clearing our internal parameter list.

	double tmpKnobData[IECORENUKE_LENSDISTORT_NUMBER_OF_STATIC_KNOBS];
	memset( &tmpKnobData[0], 0, sizeof( double ) * IECORENUKE_LENSDISTORT_NUMBER_OF_STATIC_KNOBS );

	// Get the parameters from our lens model.
	std::vector<IECore::ParameterPtr> params( m_lensModels[0]->parameters()->orderedParameters() );

	// For each parameter in the new lens model, check to see if we already have a similar knob on our UI. If we do then we copy the value
	// of the old parameter to the new.
	int attrIdx = 0;
	for( std::vector<IECore::ParameterPtr>::iterator j = params.begin(); j != params.end(); ++j )
	{
		// Get the default value.
		IECore::ConstObjectPtr o( (*j)->getValue() );
		
		// We only handle double parameters so ignore all other types...
		IECore::ConstDoubleDataPtr d = IECore::runTimeCast<const IECore::DoubleData>( o.get() );
		if (!d) continue;

		bool parameterExists = false;
		for(unsigned int i = 0; i < m_pluginAttributes.size(); ++i )
		{
			if ( (*j)->name() == m_pluginAttributes[i].m_name )
			{
				// Move the existing parameter to the correct location in our list of parameters.
				parameterExists = true;
				m_pluginAttributes.insert( m_pluginAttributes.begin()+attrIdx, m_pluginAttributes[i] );
				tmpKnobData[attrIdx] = m_knobData[i];
				m_pluginAttributes.erase( m_pluginAttributes.begin()+i+1 );
				break;
			}
		}
		++attrIdx;
	
		if ( parameterExists ) continue;
	
		// Add the new attribute.
		double value( d->readable() );
		tmpKnobData[attrIdx-1] = value;
		m_pluginAttributes.insert( m_pluginAttributes.begin()+attrIdx-1, PluginAttribute( (*j)->name().c_str() ) );
	}
	
	// Erase all remaining parameters that are not in our current lens model.
	m_pluginAttributes.erase( m_pluginAttributes.begin()+attrIdx, m_pluginAttributes.end() );

	// Copy the temporary knob values.
	memcpy( &m_knobData[0], &tmpKnobData[0], sizeof( double ) * IECORENUKE_LENSDISTORT_NUMBER_OF_STATIC_KNOBS );

	// Update the values of the knobs to match the internal data.
	for( PluginAttributeList::iterator it = m_pluginAttributes.begin(); it != m_pluginAttributes.end(); ++it )
	{
		int index = int( it - m_pluginAttributes.begin() );
		std::string knobName( parameterKnobName( index ) );
		Knob *k = knob( knobName.c_str() );
		if( k != NULL ) 
		{
			// Clear any animation on the knob and set it's value.
			std::stringstream s;
			s << m_knobData[index];
			k->from_script( s.str().c_str() );
		}
	}

	// Update the UI.
	updateUI();
}

void LensDistort::createInternalNodes()
{
	if( m_blackOutsideNode == NULL )
	{
		m_blackOutsideNode = dynamic_cast<DD::Image::Iop*>( create( "BlackOutside" ) );
	}
}

void LensDistort::connectInternalNodes()
{
	if( m_blackOutsideNode == NULL )
	{
		m_inputNode = input(0);
	}
	else if( m_enableBlackOutside == true )
	{
		m_inputNode = m_blackOutsideNode;
		m_blackOutsideNode->set_input( 0, input(0) );
	}
	else
	{
		m_inputNode = input(0);
	}
}

void LensDistort::_invalidate()
{
	if( m_blackOutsideNode != NULL )
	{
		 m_blackOutsideNode->invalidate();
	}
	Iop::_invalidate();
}

int LensDistort::currentLensModelIndex() const
{
	DD::Image::Knob *k = knob( "model" );
	if( k != NULL )
	{
		return (int)knob("model")->get_value();
	};
	return m_lensModel;
}

void LensDistort::setLensModel( IECore::ConstCompoundObjectPtr parameters )
{
	const unsigned int nPlugins( m_nThreads );
	if ( parameters )
	{
		for( unsigned int i = 0; i < nPlugins; ++i )
		{
			m_lensModels[i] = IECore::LensModel::create( parameters );
		}
	}

	updateLensModel( true );
}

void LensDistort::setLensModel( std::string modelName )
{
	const unsigned int nPlugins( m_nThreads );
	for( unsigned int i = 0; i < nPlugins; ++i )
	{
		m_lensModels[i] = IECore::LensModel::create( modelName );
	}

	updateLensModel();
}

void LensDistort::_validate( bool for_real )
{
	copy_info();
	
	m_filter.initialize();

	// Process the internal nodes.
	createInternalNodes();
	connectInternalNodes();

	// Validate the internal nodes.
	input0().validate( for_real );
	m_inputNode->validate( for_real );

	// Try to load the lens from a file. 
	std::string path;
	m_useFileSequence = fileSequencePath( path );
	if( m_useFileSequence )
	{
		m_hasValidFileSequence = setLensFromFile( path );
	}
	else
	{
		setLensModel( modelNames()[currentLensModelIndex()] );
	}

	// Check that the path isn't null.
	if ( ( !m_hasValidFileSequence ) && m_useFileSequence )
	{
		error( boost::str( boost::format( "Can not find lens file \"%s\"" ) % path ).c_str() );
		set_out_channels( Mask_None );
		return;
	}

	// Iterate over our list of parameters and update the lens models.
	for( int i = 0; i < m_nThreads; i++ )
	{
		for( PluginAttributeList::const_iterator j = m_pluginAttributes.begin(); j != m_pluginAttributes.end(); j++)
		{
			m_lensModels[i]->parameters()->parameter<IECore::DoubleParameter>( j->m_name )->setNumericValue( m_knobData[ j-m_pluginAttributes.begin() ] );
		}
		m_lensModels[i]->validate();
	}
	
	// Set the output bounding box according to the lens model.
	Imath::Box2i input( Imath::V2i( m_inputNode->info().x()-1, m_inputNode->info().y()-1 ), Imath::V2i( m_inputNode->info().r(), m_inputNode->info().t() ) );
	Imath::Box2i box( m_lensModels[0]->bounds( m_mode, input, format().width(), format().height() ) );
	bool black = m_inputNode->black_outside();
	info_.set( box.min.x-black, box.min.y-black, box.max.x+black, box.max.y+black );
	
	set_out_channels( Mask_All );
}

// Given an output bounding box, compute the input bounding box and request the image data that we need.
// We do this be using the output size and getting the bounds of the inverse distortion.
void LensDistort::_request( int x, int y, int r, int t, ChannelMask channels, int count )
{
	Imath::Box2i requestArea( Imath::V2i( x, y ), Imath::V2i( r, t ) );
	Imath::Box2i box( m_lensModels[0]->bounds( !m_mode, requestArea, format().width(), format().height() ) );
	DD::Image::Box distortedRequestedBox( box.min.x, box.min.y, box.max.x+1, box.max.y+1 );
	distortedRequestedBox.intersect( m_inputNode->info() );
	m_inputNode->request( distortedRequestedBox, channels, count );
}

void LensDistort::engine( int y, int x, int r, ChannelMask channels, Row & outrow )
{
	// Provide an early-out for any black rows.
	bool blackOutside = info().black_outside();
	if( blackOutside && ( y >= info().t()-1 || y <= info().y() ) )
	{
		outrow.erase( channels );
		return;
	}
	
	const Info &info = m_inputNode->info();
	const double h( format().height() );
	const double w( format().width() );
	const double v( double(y)/h );
	double x_min = std::numeric_limits<double>::max();
	double x_max = std::numeric_limits<double>::min();
	double y_min = std::numeric_limits<double>::max();
	double y_max = std::numeric_limits<double>::min();
	
	// Work out which of the array of lens models we should use depending on the current thread.
	int lensIdx = 0;
	const Thread::ThreadInfo * f = Thread::thisThread();
	if( f ) lensIdx=f->index;
	
	// Clamp the index to the array range.
	lensIdx = lensIdx > m_nThreads ? 0 : lensIdx;
	
	// Create an array of distorted values that we will populate.
	Imath::V2d distort[r-x];
	
	// Thread safe read of distortion data.
	{
		Guard l( m_locks[lensIdx] );
		
		// Distort each pixel on the row, store the result in an array and track the bounding box.
		for( int i = x; i < r; i++ )
		{
			double u = (i+0.0)/w;
	
			Imath::V2d &dp( distort[i-x] );
			Imath::V2d p(u, v);
			if( m_mode )
			{
				dp = m_lensModels[lensIdx]->distort( p );
			}
			else
			{
				dp = m_lensModels[lensIdx]->undistort( p );
			}
			
			dp.x *= w;
			dp.y *= h;
			
			// Clamp our distorted values to the bounding box.
			dp.x = std::max( double(info.x()), dp.x );
			dp.y = std::max( double(info.y()), dp.y );
			dp.x = std::min( double(info.r()-1), dp.x );
			dp.y = std::min( double(info.t()-1), dp.y );
			x_min = std::min( x_min, dp.x );
			y_min = std::min( y_min, dp.y );
			x_max = std::max( x_max, dp.x );
			y_max = std::max( y_max, dp.y );
		}
	}
	
	// Now we know which pixels we'll need, request them!
	y_max++;
	x_max++;
	
	DD::Image::Pixel out(channels);
	
	// Lock the tile into the cache
	DD::Image::Tile t( *m_inputNode, IECore::fastFloatFloor( x_min ), IECore::fastFloatFloor( y_min ), IECore::fastFloatCeil( x_max ), IECore::fastFloatCeil( y_max ), channels );

	// Write the black outside pixels.
	if( blackOutside )
	{
		if( x <= info_.x() )
		{
			foreach ( z, channels )
			{
				outrow.writable(z)[x] = 0.f;
			}
			x = std::max( x, info_.x() );
		}
		if( x >= info_.r()-1 )
		{
			foreach ( z, channels )
			{
				outrow.writable(z)[r-1] = 0.f;
			}
			x = std::min( x, info_.r()-1 );
		}
	}

	// Loop over our array of precomputed points, and ask nuke to perform a filtered lookup for us.
	for( int i = x; i < r; i++ )
	{
		if(aborted()) break;
		m_inputNode->sample( distort[i-x].x+0.5, distort[i-x].y+0.5, 1.0, 1.0, &m_filter, out );
		foreach ( z, channels )
		{
			outrow.writable(z)[i] = out[z];
		}
	}
}

void LensDistort::append( DD::Image::Hash &hash )
{
	std::string path;
	if ( fileSequencePath( path ) )
	{
		hash.append( path );
	}
	else
	{
		hash.append( m_assetPath );
	}

	for( unsigned int i = 0; i < IECORENUKE_LENSDISTORT_NUMBER_OF_STATIC_KNOBS; ++i )
	{
		hash.append( m_knobData[i] );
	}

	hash.append( m_lensModel );
	hash.append( m_mode );
	hash.append( m_hasValidFileSequence );
	hash.append( m_useFileSequence );
	hash.append( m_enableBlackOutside );
	hash.append( m_blackOutsideNode == NULL );
	hash.append( outputContext().frame() );
}

bool LensDistort::fileSequencePath( std::string& path )
{
	path = "";

	// Check that the path isn't null.
	try
	{
		if ( knob("lensFileSequence") != NULL )
		{
			std::stringstream pathStream;
			knob("lensFileSequence")->to_script( pathStream, &(outputContext()), false );
			path = pathStream.str();

			// If the text field has no data ...
			if ( path == "" ) return false;
		}
		return true;
	}
	catch( ... )
	{
	}
	return false;
}

bool LensDistort::setLensFromFile( std::string &returnPath )
{
	returnPath.clear();
	
	// Check that the returnPath isn't null.
	if ( !fileSequencePath( returnPath ) ) return false;

	// Get the returnPath required for the current context.
	const float frame = outputContext().frame();
	if ( !strchr( returnPath.c_str(), '#' ) )
	{
		boost::algorithm::replace_first( returnPath, "%07d", "%07i" );
		boost::algorithm::replace_first( returnPath, "%06d", "%06i" );
		boost::algorithm::replace_first( returnPath, "%05d", "%05i" );
		boost::algorithm::replace_first( returnPath, "%04d", "%04i" );
		boost::algorithm::replace_first( returnPath, "%03d", "%03i" );
		boost::algorithm::replace_first( returnPath, "%02d", "%02i" );
		boost::algorithm::replace_first( returnPath, "%d", "%1i" );
	}
	else
	{
		boost::algorithm::replace_first( returnPath, "#######", "%07i" );
		boost::algorithm::replace_first( returnPath, "######", "%06i" );
		boost::algorithm::replace_first( returnPath, "#####", "%05i" );
		boost::algorithm::replace_first( returnPath, "####", "%04i" );
		boost::algorithm::replace_first( returnPath, "###", "%03i" );
		boost::algorithm::replace_first( returnPath, "##", "%02i" );
		boost::algorithm::replace_first( returnPath, "#", "%1i" );
	}

	try
	{
		returnPath = boost::str( boost::format( returnPath ) % int(frame) );
	}
	catch( ... )
	{
		// it's ok if the above throws - it just means there wasn't a number specified in the
		// filename, so we just use the filename as is
	}

	// Check to see if the file returnPath is valid.
	std::ifstream ifile;
	ifile.open( returnPath.c_str(), std::ifstream::in );
	if ( !ifile.is_open() ) return false;
	
	// Try to open the lens file.
	try
	{
		IECore::CachedReaderPtr reader( IECore::CachedReader::defaultCachedReader() );
		IECore::ConstObjectPtr o = reader->read( returnPath );
		IECore::ConstCompoundObjectPtr p = IECore::runTimeCast< const IECore::CompoundObject >( o );
		if ( p )
		{
			// Update the lensModel knob.
			m_lensModel = indexFromModelName( p->member<IECore::StringData>( "lensModel", true )->readable().c_str() );
			knob("model")->set_value( m_lensModel );
			setLensModel( p );
			return true;
		}
	}
	catch( const IECore::Exception &e )
	{
		return false;
	}

	return false;
}

std::string LensDistort::parameterNameFromKnobName( std::string knobName ) const
{
	// This check should never fail but it is better to be safe than sorry...
	if( knobName.end() - knobName.begin() > 9 )
	{
		std::istringstream s( knobName.substr( 9, knobName.end() - knobName.begin() ) );

		unsigned int index = 0;
		s >> index;

		if( index < m_pluginAttributes.size() && index >= 0 )
		{
			return m_pluginAttributes[index].m_name;
		}
	}

	return "";
}

std::string LensDistort::parameterKnobName( unsigned int i ) const
{
	return std::string( boost::str( boost::format( "lensParam%d" ) % i ).c_str() );
}

void LensDistort::knobs( Knob_Callback f )
{
	// Process the internal nodes.
	createInternalNodes();
	connectInternalNodes();
	
	File_knob( f, &m_assetPath, "lensFileSequence", "Lens File Sequence" );
	SetFlags( f, Knob::KNOB_CHANGED_ALWAYS );
	SetFlags( f, Knob::ALWAYS_SAVE );
	SetFlags( f, Knob::NO_UNDO );
	Tooltip( f, "Directory name containing the lens files. Usually COB files..." );

	Divider( f );

	m_filter.knobs(f);
	Bool_knob( f, &m_enableBlackOutside, "blackOutside", "black outside" );
	Tooltip( f, "Fill the areas outside of the distorted image with black." );

	static const char * const modes[] = { "Distort", "Undistort", 0 };
	Enumeration_knob( f, &m_mode, modes, "mode", "Mode" );
	Tooltip( f, "Whether to Distort or Undistort the input by the current lens model." );
	SetFlags( f, Knob::KNOB_CHANGED_ALWAYS );
	SetFlags( f, Knob::ALWAYS_SAVE );
	
	Enumeration_knob( f, &m_lensModel, modelNames(), "model", "Model" );
	Tooltip( f, "Choose the lens model to distort the input with. This list is populated with all lens models that have been registered to Cortex." );
	SetFlags( f, Knob::KNOB_CHANGED_ALWAYS );
	SetFlags( f, Knob::ALWAYS_SAVE );
	SetFlags( f, Knob::NO_UNDO );
		
	if( f.makeKnobs() )
	{
		setLensModel( modelNames()[currentLensModelIndex()] );
	}
	
	for( unsigned int i = 0; i < IECORENUKE_LENSDISTORT_NUMBER_OF_STATIC_KNOBS; ++i )
	{
		if( i < m_pluginAttributes.size() )
		{
			Double_knob( f, &m_knobData[i], parameterKnobName(i).c_str(), m_pluginAttributes[i].m_name.c_str() );
			if( !m_useFileSequence )
			{
				ClearFlags( f, Knob::DISABLED );
			}
			else
			{
				SetFlags( f, Knob::DISABLED );
			}
			ClearFlags( f, Knob::HIDDEN );
		}
		else
		{
			Double_knob( f, &m_knobData[i], parameterKnobName(i).c_str() );
			SetFlags( f, Knob::DISABLED );
			SetFlags( f, Knob::HIDDEN );
		}

		SetFlags( f, Knob::KNOB_CHANGED_ALWAYS );
		SetFlags( f, Knob::ALWAYS_SAVE );
	}
}

int LensDistort::knob_changed(Knob* k)
{
	if( k->is( "blackOutside" ) )
	{
		m_enableBlackOutside = bool( k->get_value() );
		return true;
	}

	// If the lensFileSequence knob just changed then we need to check if it is valid and load it.
	if ( k->is( "lensFileSequence" ) )
	{
		std::string path;
		m_useFileSequence = fileSequencePath( path );
		if( m_useFileSequence )
		{
			m_hasValidFileSequence = setLensFromFile( path );
		}
		return true;
	}

	// If the lens model was just changed then we need to set it internally and then update the UI.
	if ( k->is( "model" ) )
	{
		setLensModel( modelNames()[currentLensModelIndex()] );
		return true;
	}

	// Update our internal reference of the knob value that just changed...
	if ( !m_hasValidFileSequence )
	{
		std::string parameterName = parameterNameFromKnobName( k->name() );
		for ( PluginAttributeList::iterator it = m_pluginAttributes.begin(); it != m_pluginAttributes.end(); it++ )
		{
			if( parameterName == it->m_name )
			{
				m_knobData[ it - m_pluginAttributes.begin() ] = k->get_value();
				return true;
			}
		}
	}

	// Do we need to update the UI?
	if ( k == &Knob::showPanel )
	{
		validate( false );
		updateUI();
		return true;
	}

	return Iop::knob_changed(k);
}

void LensDistort::updateUI()
{
	// Set the names of knobs that are associated with our lens model attributes.
	unsigned int index = 0;
	for ( ; index < m_pluginAttributes.size(); ++index )
	{
		std::string knobName( parameterKnobName( index ) );
		Knob *k = knob( knobName.c_str() );
		if( k != NULL ) 
		{
			k->enable( !m_useFileSequence );
			
			std::string label( k->label() );
			k->label( m_pluginAttributes[index].m_name.c_str() );
			k->visible( true );
			k->updateUI( outputContext() );
		}
	}
	
	// Hide all other knobs from sight.
	for ( ; index < IECORENUKE_LENSDISTORT_NUMBER_OF_STATIC_KNOBS; ++index )
	{
		std::string knobName( parameterKnobName( index ) );
		Knob *k = knob( knobName.c_str() );
		if( k != NULL ) 
		{
			k->enable( false );
			
			k->label( knobName.c_str() );
			k->visible( false );
			k->set_value( 0 );
			k->updateUI( outputContext() );
		}
	}

	if ( knob( "model" ) != NULL )
	{
		knob( "model" )->enable( !m_useFileSequence );
	}
}

} // namespace IECoreNuke
