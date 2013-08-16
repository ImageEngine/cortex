
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
	m_nThreads( std::max( 16u, Thread::numCPUs + Thread::numThreads + 1 ) ),
	m_lensModel( 0 ),
	m_numNewKnobs( 0 ),
	m_kModel( NULL ),
	m_mode( Distort ),
	m_assetPath( "" ),
	m_hasValidFileSequence( false ),
	m_useFileSequence( false )
{
	// Work out the most threads we can have and create an instance of the lens model for each one.
	// This is useful for any lens model implementation that uses the ldpk library is as it is not thread safe.
	m_locks = new Lock[m_nThreads];
	m_model.resize( m_nThreads, NULL );
	for( int i = 0; i < m_nThreads; ++i ) m_model[i] = IECore::LensModel::create(modelNames()[0]);
}

void LensDistort::updateLensModel( bool updateKnobsFromParameters ) 
{
	if ( updateKnobsFromParameters ) m_pluginAttributes.clear(); // Give us a fresh start by clearing our internal parameter list.

	// Get the parameters from our lens model.
	std::vector<IECore::ParameterPtr> params( m_model[0]->parameters()->orderedParameters() );

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
		for (unsigned int i = attrIdx; i < m_pluginAttributes.size(); i++)
		{
			if ( (*j)->name() == m_pluginAttributes[i].m_name )
			{
				// Move the existing parameter to the correct location in our list of parameters.
				parameterExists = true;
				m_pluginAttributes.insert( m_pluginAttributes.begin()+attrIdx, m_pluginAttributes[i] );
				m_pluginAttributes.erase( m_pluginAttributes.begin()+i+1 );
				break;
			}
		}
		++attrIdx;
	
		if ( parameterExists ) continue;
	
		// Add the new attribute.
		double value( d->readable() );
		m_pluginAttributes.insert( m_pluginAttributes.begin()+attrIdx-1, PluginAttribute( (*j)->name().c_str(), value ) );
	}
	
	// Erase all remaining parameters that are not in our current lens model.
	m_pluginAttributes.erase( m_pluginAttributes.begin()+attrIdx, m_pluginAttributes.end() );

}

void LensDistort::setLensModel( IECore::ConstCompoundObjectPtr parameters )
{
	const unsigned int nPlugins( m_nThreads );
	if ( parameters )
	{
		for( unsigned int i = 0; i < nPlugins; ++i )
		{
			m_model[i] = IECore::LensModel::create( parameters );
		}
	}

	updateLensModel( true );
}

void LensDistort::setLensModel( std::string modelName )
{
	const unsigned int nPlugins( m_nThreads );
	for( unsigned int i = 0; i < nPlugins; ++i )
	{
		m_model[i] = IECore::LensModel::create( modelName );
	}

	updateLensModel();
}

void LensDistort::_validate(bool for_real)
{
	copy_info();
	
	m_filter.initialize();

	// Update the lens model if the frame or frame offset changed.
	std::string filePath;
	if ( m_useFileSequence )
	{
		m_hasValidFileSequence = setLensFromFile( filePath );
	}

	// Handle the knobs' status.
	if ( knob("model" ) != NULL) knob("model")->enable( !m_useFileSequence );
	for ( PluginAttributeList::iterator it = m_pluginAttributes.begin(); it != m_pluginAttributes.end(); it++ )
	{
		if ( it->m_knob != NULL) it->m_knob->enable( !m_useFileSequence );
	}

	// Check that the path isn't null.
	if ( (!m_hasValidFileSequence) && m_useFileSequence )
	{
		error( boost::str( boost::format( "Can not find lens file \"%s\"" ) % filePath ).c_str() );
		set_out_channels( Mask_None );
		return;
	}

	// Iterate over our list of parameters and update the lens models.
	for( int i = 0; i < m_nThreads; i++ )
	{
		for( PluginAttributeList::const_iterator j = m_pluginAttributes.begin(); j != m_pluginAttributes.end(); j++)
		{
			m_model[i]->parameters()->parameter<IECore::DoubleParameter>( j->m_name )->setNumericValue( j->m_value );
		}
		m_model[i]->validate();
	}
	
	// Set the output bounding box according to the lens model.
	Imath::Box2i input( Imath::V2i( input0().info().x(), input0().info().y() ), Imath::V2i( input0().info().r()-1, input0().info().t()-1 ) );
	Imath::Box2i box( m_model[0]->bounds( m_mode, input, format().width(), format().height() ) );
	info_.set( box.min.x, box.min.y, box.max.x, box.max.y );
	
	set_out_channels( Mask_All );
}

// Given an output bounding box, compute the input bounding box and request the image data that we need.
// We do this be using the output size and getting the bounds of the inverse distortion.
void LensDistort::_request( int x, int y, int r, int t, ChannelMask channels, int count )
{
	Imath::Box2i requestArea( Imath::V2i( x, y ), Imath::V2i( r-1, t-1 ) );
	Imath::Box2i box( m_model[0]->bounds( !m_mode, requestArea, format().width(), format().height() ) );
	DD::Image::Box distortedRequestedBox( box.min.x, box.min.y, box.max.x+1, box.max.y+1 );
	distortedRequestedBox.intersect( input0().info() );
	input0().request( distortedRequestedBox, channels, count );
}

void LensDistort::engine( int y, int x, int r, ChannelMask channels, Row & outrow )
{
	const Info &info = input0().info();
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
				dp = m_model[lensIdx]->distort( p );
			}
			else
			{
				dp = m_model[lensIdx]->undistort( p );
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
	DD::Image::Tile t( input0(), int(floor(x_min)), int(floor(y_min)), int(ceil(x_max)), int(ceil(y_max)), channels );
	
	// Loop over our array of precomputed points, and ask nuke to perform a filtered lookup for us
	for( int i = x; i < r; i++ )
	{
		if(aborted()) break;
		input0().sample( distort[i-x].x+0.5, distort[i-x].y+0.5, 1.0, 1.0, &m_filter, out );
		foreach ( z, channels )
		{
			outrow.writable(z)[i] = out[z];
		}
	}
	
}

void LensDistort::append( DD::Image::Hash &hash )
{
	std::string path;
	if ( getFileSequencePath( path ) )
	{
		hash.append( path );
	}
	else
	{
		hash.append( m_assetPath );
	}

	hash.append( m_lensModel );
	hash.append( m_mode );
	hash.append( m_hasValidFileSequence );
	hash.append( m_useFileSequence );
	hash.append( outputContext().frame() );
}

bool LensDistort::getFileSequencePath( std::string& path )
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
	if ( !getFileSequencePath( returnPath ) ) return false;

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
	if ( !ifile.is_open() ) return false;;
	
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
			m_kModel->set_value( m_lensModel );
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

void LensDistort::knobs( Knob_Callback f )
{
	File_knob( f, &m_assetPath, "lensFileSequence", "Lens File Sequence" );
	SetFlags( f, Knob::KNOB_CHANGED_ALWAYS );
	SetFlags( f, Knob::ALWAYS_SAVE );
	SetFlags( f, Knob::NO_UNDO );
	Tooltip( f, "Directory name containing the lens files. Usually COB files..." );

	Divider( f );

	if( f.makeKnobs() )
	{
		setLensModel( modelNames()[0] );
	}
	
	m_filter.knobs(f);

	static const char * const modes[] = { "Distort", "Undistort", 0 };
	Enumeration_knob( f, &m_mode, modes, "mode", "Mode" );
	Tooltip( f, "Whether to Distort or Undistort the input by the current lens model." );
	SetFlags( f, Knob::KNOB_CHANGED_ALWAYS );
	SetFlags( f, Knob::ALWAYS_SAVE );
	
	m_lastStaticKnob = m_kModel = Enumeration_knob( f, &m_lensModel, modelNames(), "model", "Model" );
	Tooltip( f, "Choose the lens model to distort the input with. This list is populated with all lens models that have been registered to Cortex." );
	SetFlags( f, Knob::KNOB_CHANGED_ALWAYS );
	SetFlags( f, Knob::ALWAYS_SAVE );
	SetFlags( f, Knob::NO_UNDO );
	
	// Create the Dynamic knobs.
	if( f.makeKnobs() )
	{
		m_numNewKnobs = add_knobs( addDynamicKnobs, this->firstOp(), f );
		SetFlags( f, Knob::KNOB_CHANGED_ALWAYS );
		SetFlags( f, Knob::ALWAYS_SAVE );
	}
	else 
	{
		LensDistort::addDynamicKnobs( this->firstOp(), f );
		SetFlags( f, Knob::KNOB_CHANGED_ALWAYS );
		SetFlags( f, Knob::ALWAYS_SAVE );
	}
}

int LensDistort::knob_changed(Knob* k)
{
	bool updateUI = false;

	// If the lensFileSequence knob just changed then we need to check if it is valid and load it.
	// Once loaded then we set the updateUI flag to trigger a UI update.
	if ( k->is( "lensFileSequence" ) )
	{
		std::string path;
		bool oldValue = m_useFileSequence;
		m_useFileSequence = getFileSequencePath( path );
		updateUI |= oldValue != m_useFileSequence;

		if ( m_useFileSequence )
		{
			bool oldValue = m_hasValidFileSequence;
			std::string path;
			m_hasValidFileSequence = setLensFromFile( path );
			updateUI |= m_hasValidFileSequence != oldValue;
		}
		
	}

	// If the lens model was just changed then we need to set it internally and then update the UI.
	if ( k->is( "model" ) )
	{
		setLensModel( modelNames()[getLensModel()] );
		updateUI = true;
	}

	// Do we need to update the UI?
	if ( k == &Knob::showPanel || updateUI )
	{
		m_numNewKnobs = replace_knobs( m_lastStaticKnob, m_numNewKnobs, addDynamicKnobs, this->firstOp() );

		// Handle the knobs state.
		if ( knob("model" ) != NULL) knob("model")->enable( !m_useFileSequence );
		for ( PluginAttributeList::iterator it = m_pluginAttributes.begin(); it != m_pluginAttributes.end(); it++ )
		{
			if ( it->m_knob != NULL) it->m_knob->enable( !m_useFileSequence );
		}

		return true;
	}

	// Update our internal reference of the knob value that just changed...
	if ( !m_hasValidFileSequence )
	{
		for ( PluginAttributeList::iterator it = m_pluginAttributes.begin(); it != m_pluginAttributes.end(); it++ )
		{
			if ( k == it->m_knob )
			{
				it->m_value = k->get_value();
				return true;
			}
		}
	}

	if ( k->is( "lensFileSequence" ) ) return true;

	return Iop::knob_changed(k);
}

void LensDistort::addDynamicKnobs(void* p, DD::Image::Knob_Callback f) 
{
	PluginAttributeList& attributeList( ((LensDistort*)p)->attributeList() );
	const unsigned int nAttributes( attributeList.size() );
	for ( unsigned int i = 0; i < nAttributes; ++i )
	{
		attributeList[i].m_knob = Double_knob( f, &attributeList[i].m_value, attributeList[i].m_name.c_str(), attributeList[i].m_name.c_str() );
		SetFlags( f, Knob::KNOB_CHANGED_ALWAYS );
		SetFlags( f, Knob::ALWAYS_SAVE );
	}
}

} // namespace IECoreNuke
