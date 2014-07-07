//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreRI/private/RendererImplementation.h"
#include "IECoreRI/PrimitiveVariableList.h"
#include "IECoreRI/ParameterList.h"
#include "IECoreRI/Convert.h"
#include "IECoreRI/ScopedContext.h"

#include "IECore/MessageHandler.h"
#include "IECore/Shader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/MatrixAlgo.h"
#include "IECore/Transform.h"
#include "IECore/MatrixTransform.h"
#include "IECore/Group.h"
#include "IECore/MurmurHash.h"

#include "boost/algorithm/string/case_conv.hpp"
#include "boost/format.hpp"

#include <iostream>

#include "ri.h"
#include "rx.h"

using namespace IECore;
using namespace IECoreRI;
using namespace Imath;
using namespace std;
using namespace boost;

////////////////////////////////////////////////////////////////////////
// AttributeState implementation
////////////////////////////////////////////////////////////////////////

IECoreRI::RendererImplementation::AttributeState::AttributeState()
{
}

IECoreRI::RendererImplementation::AttributeState::AttributeState( const AttributeState &other )
{
	primVarTypeHints = other.primVarTypeHints;
}

////////////////////////////////////////////////////////////////////////
// IECoreRI::RendererImplementation implementation
////////////////////////////////////////////////////////////////////////

IECoreRI::RendererImplementation::ContextToSharedDataMapMutex IECoreRI::RendererImplementation::s_contextToSharedDataMapMutex;
IECoreRI::RendererImplementation::ContextToSharedDataMap IECoreRI::RendererImplementation::s_contextToSharedDataMap;
tbb::queuing_rw_mutex IECoreRI::RendererImplementation::g_nLoopsMutex;
std::vector<int> IECoreRI::RendererImplementation::g_nLoops;

// This constructor launches a render within the current application. It creates a SharedData instance,
// which gets propagated down to the renderers this object creates by launching procedurals.
IECoreRI::RendererImplementation::RendererImplementation( const std::string &name )
	:	m_sharedData( new SharedData ), m_inWorld( false )
{
	m_options = new CompoundData();
	constructCommon();
	if( name!="" )
	{
		RiBegin( (char *)name.c_str() );
	}
	else
	{
#ifdef PRMANEXPORT	
		RiBegin( "launch:prman? -t" );
#else
		RiBegin( 0 );
#endif		
	}
	m_context = RiGetContext();
	
	// Add a correspondance between the current context and this object's SharedData instance,
	// in case RendererImplementation() gets called later on with no arguments. This creates a
	// RendererImplementation in the current context, which must have access to this object's
	// SharedData instance.
	
	m_contextToSharedDataMapKey = m_context;
	ContextToSharedDataMapMutex::scoped_lock l( s_contextToSharedDataMapMutex );
	s_contextToSharedDataMap.insert( std::make_pair( m_contextToSharedDataMapKey, m_sharedData ) );
}

// This constructor gets called in procSubdivide(), and inherits the SharedData from the RendererImplementation
// that launched the procedural
IECoreRI::RendererImplementation::RendererImplementation( SharedData::Ptr sharedData, IECore::CompoundDataPtr options )
	:	m_context( 0 ), m_sharedData( sharedData ), m_options( options ), m_inWorld( true )
{
	constructCommon();
	
	// Add a correspondance between the current context and this object's SharedData instance,
	// in case RendererImplementation() gets called later on with no arguments. This creates a
	// RendererImplementation in the current context, which must have access to this object's
	// SharedData instance.
	
	m_contextToSharedDataMapKey = RiGetContext();
	ContextToSharedDataMapMutex::scoped_lock l( s_contextToSharedDataMapMutex );
	s_contextToSharedDataMap.insert( std::make_pair( m_contextToSharedDataMapKey, m_sharedData ) );
}

// This constructor creates a RendererImplementation using the current RtContext. The SharedData is acquired by 
// querying m_contextToSharedDataMap in one of two ways:
//
// 1)	If there's an entry for the current RtContext, this means the user's manually called Renderer() with no arguments
//	in the course of a render, and we set m_sharedData to this entry.
// 2)	If there isn't an entry for the current context, this means we're in a procedural that's been launched from a rib,
//	and in this case we use a null context as a key into the map. If there's an entry for the null context, we
//	set m_sharedData to that entry, otherwise we set it to a new SharedData.
//
IECoreRI::RendererImplementation::RendererImplementation()
	:	m_context( 0 ), m_options( 0 ), m_inWorld( true )
{
	constructCommon();
	
	m_contextToSharedDataMapKey = RiGetContext();
	ContextToSharedDataMapMutex::scoped_lock l( s_contextToSharedDataMapMutex );

	ContextToSharedDataMap::iterator it = s_contextToSharedDataMap.find( m_contextToSharedDataMapKey );
	if( it == s_contextToSharedDataMap.end() )
	{
		m_contextToSharedDataMapKey = 0;
		it = s_contextToSharedDataMap.find( m_contextToSharedDataMapKey );
		if( it == s_contextToSharedDataMap.end() )
		{
			m_sharedData = new SharedData;
		}
		else
		{
			m_sharedData = it->second;
		}
	}
	else
	{
		m_sharedData = it->second;
	}
	
	s_contextToSharedDataMap.insert( std::make_pair( m_contextToSharedDataMapKey, m_sharedData ) );
}

void IECoreRI::RendererImplementation::constructCommon()
{
	m_attributeStack.push( AttributeState() );

	const char *fontPath = getenv( "IECORE_FONT_PATHS" );
	if( fontPath )
	{
		m_fontSearchPath.setPaths( fontPath, ":" );
	}

	m_shaderCache = defaultShaderCache();

	m_setOptionHandlers["ri:searchpath:shader"] = &IECoreRI::RendererImplementation::setShaderSearchPathOption;
	m_setOptionHandlers["ri:pixelsamples"] = &IECoreRI::RendererImplementation::setPixelSamplesOption;
	m_setOptionHandlers["ri:pixelSamples"] = &IECoreRI::RendererImplementation::setPixelSamplesOption;
	m_setOptionHandlers["searchPath:font"] = &IECoreRI::RendererImplementation::setFontSearchPathOption;

	m_getOptionHandlers["shutter"] = &IECoreRI::RendererImplementation::getShutterOption;
	m_getOptionHandlers["camera:shutter"] = &IECoreRI::RendererImplementation::getShutterOption;
	m_getOptionHandlers["camera:resolution"] = &IECoreRI::RendererImplementation::getResolutionOption;
	m_getOptionHandlers["searchPath:font"] = &IECoreRI::RendererImplementation::getFontSearchPathOption;

	m_setAttributeHandlers["ri:shadingRate"] = &IECoreRI::RendererImplementation::setShadingRateAttribute;
	m_setAttributeHandlers["ri:matte"] = &IECoreRI::RendererImplementation::setMatteAttribute;
	m_setAttributeHandlers["ri:color"] = &IECoreRI::RendererImplementation::setColorAttribute;
	m_setAttributeHandlers["color"] = &IECoreRI::RendererImplementation::setColorAttribute;
	m_setAttributeHandlers["ri:opacity"] = &IECoreRI::RendererImplementation::setOpacityAttribute;
	m_setAttributeHandlers["opacity"] = &IECoreRI::RendererImplementation::setOpacityAttribute;
	m_setAttributeHandlers["ri:sides"] = &IECoreRI::RendererImplementation::setSidesAttribute;
	m_setAttributeHandlers["doubleSided"] = &IECoreRI::RendererImplementation::setDoubleSidedAttribute;
	m_setAttributeHandlers["rightHandedOrientation"] = &IECoreRI::RendererImplementation::setRightHandedOrientationAttribute;
	m_setAttributeHandlers["ri:geometricApproximation:motionFactor"] = &IECoreRI::RendererImplementation::setGeometricApproximationAttribute;
	m_setAttributeHandlers["ri:geometricApproximation:focusFactor"] = &IECoreRI::RendererImplementation::setGeometricApproximationAttribute;
	m_setAttributeHandlers["name"] = &IECoreRI::RendererImplementation::setNameAttribute;
	m_setAttributeHandlers["ri:subsurface"] = &IECoreRI::RendererImplementation::setSubsurfaceAttribute;
	m_setAttributeHandlers["ri:detail"] = &IECoreRI::RendererImplementation::setDetailAttribute;
	m_setAttributeHandlers["ri:detailRange"] = &IECoreRI::RendererImplementation::setDetailRangeAttribute;
	m_setAttributeHandlers["ri:textureCoordinates"] = &IECoreRI::RendererImplementation::setTextureCoordinatesAttribute;
	m_setAttributeHandlers["ri:automaticInstancing"] = &IECoreRI::RendererImplementation::setAutomaticInstancingAttribute;

	m_getAttributeHandlers["ri:shadingRate"] = &IECoreRI::RendererImplementation::getShadingRateAttribute;
	m_getAttributeHandlers["ri:matte"] = &IECoreRI::RendererImplementation::getMatteAttribute;
	m_getAttributeHandlers["doubleSided"] = &IECoreRI::RendererImplementation::getDoubleSidedAttribute;
	m_getAttributeHandlers["rightHandedOrientation"] = &IECoreRI::RendererImplementation::getRightHandedOrientationAttribute;
	m_getAttributeHandlers["name"] = &IECoreRI::RendererImplementation::getNameAttribute;
	m_getAttributeHandlers["ri:textureCoordinates"] = &IECoreRI::RendererImplementation::getTextureCoordinatesAttribute;
	m_getAttributeHandlers["ri:automaticInstancing"] = &IECoreRI::RendererImplementation::getAutomaticInstancingAttribute;

	m_commandHandlers["ri:readArchive"] = &IECoreRI::RendererImplementation::readArchiveCommand;
	m_commandHandlers["ri:archiveRecord"] = &IECoreRI::RendererImplementation::archiveRecordCommand;
	m_commandHandlers["ri:illuminate"] = &IECoreRI::RendererImplementation::illuminateCommand;

	m_motionType = None;
	m_numDisplays = 0;
}

IECoreRI::RendererImplementation::~RendererImplementation()
{
	if( m_context )
	{
		RtContextHandle c = RiGetContext();
		RiContext( m_context );
		RiEnd();
		if( c!=m_context )
		{
			RiContext( c );
		}
	}
	
	// A null m_contextToSharedDataMapKey means this was launched straight from a rib. In this case we don't
	// remove its entry from the map, as this could lead to the global shared data getting destroyed and recreated
	// multiple times.
	if( m_contextToSharedDataMapKey != 0 )
	{
		ContextToSharedDataMapMutex::scoped_lock l( s_contextToSharedDataMapMutex );
		ContextToSharedDataMap::iterator it = s_contextToSharedDataMap.find( m_contextToSharedDataMapKey );
		if( it == s_contextToSharedDataMap.end() )
		{
			IECore::msg( Msg::Warning, "IECoreRI::RendererImplementation::~RendererImplementation", "couldn't remove context->sharedData entry." );
		}
		else
		{
			s_contextToSharedDataMap.erase( it );
		}
	}
}

////////////////////////////////////////////////////////////////////////
// options
////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::setOption( const std::string &name, IECore::ConstDataPtr value )
{
	if( !m_options )
	{
		IECore::msg( Msg::Error, "IECoreRI::RendererImplementation::setOption", "Cannot call setOption on non-root renderer." );
		return;
	}

	// we need to group related options together into a single RiOption or RiHider call, so we
	// just accumulate the options until worldBegin() where we'll emit them.
	m_options->writable()[name] = value->copy();
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getOption( const std::string &name ) const
{
	if( m_options )
	{
		// we were created to perform a render from the beginning, and have been keeping
		// track of the options ourselves.
		const IECore::Data *result = m_options->member<IECore::Data>( name );
		if( result )
		{
			return result;
		}
		else
		{
			// we don't have the option set explicitly, so fall through and
			// try to use getRxOption to query the value from renderman directly.
		}
	}

	ScopedContext scopedContext( m_context );
	GetOptionHandlerMap::const_iterator it = m_getOptionHandlers.find( name );
	if( it!=m_getOptionHandlers.end() )
	{
		return (this->*(it->second))( name );
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
#ifdef PRMANEXPORT	
		return getRxOption( name.c_str() + 5 );
#else		
		return getRxOption( name.c_str() );		
#endif		
	}
	else if( name.compare( 0, 3, "ri:" )==0 )
	{
		return getRxOption( name.c_str() + 3 );
	}
	else if( name.find_first_of( ":" )!=string::npos )
	{
		// silently ignore options prefixed for some other RendererImplementation
		return 0;
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::getOption", format( "Unknown option \"%s\"." ) % name );
	}
	return 0;
}

void IECoreRI::RendererImplementation::setShaderSearchPathOption( const std::string &name, IECore::ConstDataPtr d )
{
	if( ConstStringDataPtr s = runTimeCast<const StringData>( d ) )
	{
		m_shaderCache = new CachedReader( SearchPath( s->readable(), ":" ) );
		// no need to call RiOption as that'll be done in worldBegin().
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::setOption", "Expected StringData for \"ri:searchpath:shader\"." );
	}
}

void IECoreRI::RendererImplementation::setPixelSamplesOption( const std::string &name, IECore::ConstDataPtr d )
{
	if( ConstV2iDataPtr s = runTimeCast<const V2iData>( d ) )
	{
		RiPixelSamples( s->readable().x, s->readable().y );
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::setOption", "Expected V2iData for \"ri:pixelSamples\"." );
	}
}

void IECoreRI::RendererImplementation::setFontSearchPathOption( const std::string &name, IECore::ConstDataPtr d )
{
	if( ConstStringDataPtr s = runTimeCast<const StringData>( d ) )
	{
		m_fontSearchPath.setPaths( s->readable(), ":" );
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::setOption", "Expected StringData for \"searchPath:font\"." );
	}
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getFontSearchPathOption( const std::string &name ) const
{
	return new StringData( m_fontSearchPath.getPaths( ":" ) );
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getShutterOption( const std::string &name ) const
{
	float shutter[2];
	RxInfoType_t resultType;
	int resultCount;
	int s = RxOption( "Shutter", shutter, 2 * sizeof( float ), &resultType, &resultCount );
	if( s==0 )
	{
		if( resultType==RxInfoFloat && resultCount==2 )
		{
			return new V2fData( V2f( shutter[0], shutter[1] ) );
		}
	}
	return 0;
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getResolutionOption( const std::string &name ) const
{
	float format[3];
	RxInfoType_t resultType;
	int resultCount;
	int s = RxOption( "Format", format, 3 * sizeof( float ), &resultType, &resultCount );
	if( s==0 )
	{
		if( resultType==RxInfoFloat && resultCount==3 )
		{
			return new V2iData( V2i( (int)format[0], (int)format[1] ) );
		}
	}
	return 0;
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getRxOption( const char *name ) const
{
	char result[16 * sizeof( RtFloat )]; // enough room for a matrix return type
	memset( result, 0, 16 * sizeof( RtFloat ) ); // 3delight has a bug where it'll try to free some random part of memory if this is not null (v 7.0.54)
	RxInfoType_t resultType;
	int resultCount;
	if( 0==RxOption( (char *)name, result, 16 * sizeof( RtFloat ), &resultType, &resultCount ) )
	{
		return convert( result, resultType, resultCount );
	}
	return 0;
}

void IECoreRI::RendererImplementation::camera( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );
	CompoundDataPtr parameterData = (new CompoundData( parameters ))->copy();

	CameraPtr camera = new Camera( name, 0, parameterData );
	camera->addStandardParameters(); // it simplifies things to know that the camera is complete

	// output shutter
	CompoundDataMap::const_iterator it = camera->parameters().find( "shutter" );
	ConstV2fDataPtr shutterD = runTimeCast<const V2fData>( it->second );
	RiShutter( shutterD->readable()[0], shutterD->readable()[1] );

	// then resolution
	it = camera->parameters().find( "resolution" );
	ConstV2iDataPtr d = runTimeCast<const V2iData>( it->second );
	RiFormat( d->readable().x, d->readable().y, 1 );

	// then screen window
	it = camera->parameters().find( "screenWindow" );
	ConstBox2fDataPtr screenWindowD = runTimeCast<const Box2fData>( it->second );
	RiScreenWindow( screenWindowD->readable().min.x, screenWindowD->readable().max.x, screenWindowD->readable().min.y, screenWindowD->readable().max.y );

	// then crop window
	it = camera->parameters().find( "cropWindow" );
	ConstBox2fDataPtr cropWindowD = runTimeCast<const Box2fData>( it->second );
	RiCropWindow( cropWindowD->readable().min.x, cropWindowD->readable().max.x, cropWindowD->readable().min.y, cropWindowD->readable().max.y );

	// then clipping
	it = camera->parameters().find( "clippingPlanes" );
	ConstV2fDataPtr clippingD = runTimeCast<const V2fData>( it->second );
	RiClipping( clippingD->readable()[0], clippingD->readable()[1] );

	// then projection
	RiIdentity();
	it = camera->parameters().find( "projection" );
	ConstStringDataPtr projectionD = runTimeCast<const StringData>( it->second );
	ParameterList p( camera->parameters(), "projection:" );
	RiProjectionV( (char *)projectionD->readable().c_str(), p.n(), p.tokens(), p.values() );

	// then transform
	const size_t numSamples = m_preWorldTransform.numSamples();
	if( numSamples > 1 )
	{
		vector<float> sampleTimes;
		for( size_t i = 0; i < numSamples; ++i )
		{
			sampleTimes.push_back( m_preWorldTransform.sampleTime( i ) );
		}
		RiMotionBegin( sampleTimes.size(), &sampleTimes.front() );
	}
	
	for( size_t i = 0; i < numSamples; ++i )
	{
		M44f m = m_preWorldTransform.sample( i );
		m.scale( V3f( 1.0f, 1.0f, -1.0f ) );
		m.invert();
		RtMatrix mm;
		convert( m, mm );
		RiTransform( mm );
	}
	
	if( numSamples > 1 )
	{
		RiMotionEnd();
	}
	
	// then camera itself
	RiCamera( name.c_str(), RI_NULL );
	
	if( name == m_lastCamera )
	{
		// we're in an edit, and need to update
		// the world camera as well.
		RiCamera( RI_WORLD, RI_NULL );
	}
	else
	{
		// remember which camera we output last
		m_lastCamera = name;
	}
}

/// \todo This should be outputting several calls to display as a series of secondary displays, and also trying to find the best display
/// to be used as the primary display.
void IECoreRI::RendererImplementation::display( const std::string &name, const std::string &type, const std::string &data, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );
	
	std::string prefixedName = name;
	if( m_numDisplays )
	{
		prefixedName = "+" + prefixedName;
	}
	
	ParameterList pl( parameters );
	RiDisplayV( (char *)prefixedName.c_str(), (char *)type.c_str(), (char *)data.c_str(), pl.n(), pl.tokens(), pl.values() );
	
	m_numDisplays++;
}

/////////////////////////////////////////////////////////////////////////////////////////
// world
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::worldBegin()
{
	if( m_inWorld )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::worldBegin", "Already in a world block." );
		return;
	}

	if( m_lastCamera == "" )
	{
		// no camera was output explicitly. output one ourselves
		// so that we end up with the cortex default camera rather
		// than the renderman one.
		camera( "main", CompoundDataMap() );
	}

	ScopedContext scopedContext( m_context );
	
	// we implement the "editable" option by specifying the raytrace hider with
	// an "editable" parameter. preprocess our options to reflect that, warning
	// the user if they were trying to use any hider other than the raytrace one.
	
	const BoolData *editableData = m_options->member<BoolData>( "editable" );
	if( editableData && editableData->readable() )
	{
		const StringData *hiderData = m_options->member<StringData>( "ri:hider" );
		if( hiderData && hiderData->readable() != "raytrace" )
		{
			msg( Msg::Warning, "IECoreRI::RendererImplementation::setOption", "Forcing hider to \"raytrace\" to support editable render." );
		}
		m_options->writable()["ri:hider"] = new StringData( "raytrace" );
		m_options->writable()["ri:hider:editable"] = new BoolData( true );
		m_options->writable()["ri:hider:progressive"] = new BoolData( true );
	}
	
	// output all our stored options
	
	std::set<std::string> categoriesDone;
	for( CompoundDataMap::const_iterator it = m_options->readable().begin(), eIt = m_options->readable().end(); it != eIt; it++ )
	{
		const std::string &name = it->first.string();
		bool processed = false;
		
		if( name.compare( 0, 8, "ri:hider" ) == 0 )
		{
			if( categoriesDone.find( "hider" ) == categoriesDone.end() )
			{		
				const StringData *hiderData = m_options->member<StringData>( "ri:hider" );
				const string hider = hiderData ? hiderData->readable() : "hidden";
				ParameterList pl( m_options->readable(), "ri:hider:" );
				RiHiderV( (char *)hider.c_str(), pl.n(), pl.tokens(), pl.values() ); 
				categoriesDone.insert( "hider" );
			}
			processed = true;
		}
		else if( name.compare( 0, 3, "ri:" )==0 )
		{
			size_t i = name.find_first_of( ":", 3 );
			if( i!=string::npos )
			{
				// ri:*:*
				string category( name, 3, i-3 );
				if( categoriesDone.find( category ) == categoriesDone.end() )
				{
					string prefix( name, 0, i+1 );
					ParameterList pl( m_options->readable(), prefix );
					RiOptionV( (char *)category.c_str(), pl.n(), pl.tokens(), pl.values() );
					categoriesDone.insert( category );
				}
				processed = true;
			}
		}
		else if( name.compare( 0, 5, "user:" )==0 )
		{
			// user:*
			if( categoriesDone.find( "user" ) == categoriesDone.end() )
			{
				string s( name, 5 );
				ParameterList pl( m_options->readable(), "user:" );
				RiOptionV( "user", pl.n(), pl.tokens(), pl.values() );
				categoriesDone.insert( "user" );
			}
			processed = true;
		}
		else if( name == "editable" )
		{
			processed = true;
		}
		
		// we might have custom handlers in addition to the default
		// handling above. invoke those.
		SetOptionHandlerMap::iterator hIt = m_setOptionHandlers.find( name );
		if( hIt!=m_setOptionHandlers.end() )
		{
			(this->*(hIt->second))( name, it->second );
			processed = true;
		}
		
		if( !processed && ( name.find_first_of( ":" )==string::npos || name.compare( 0, 3, "ri:" ) == 0 ) )
		{
			msg( Msg::Warning, "IECoreRI::RendererImplementation::setOption", format( "Unknown option \"%s\"." ) % name );
		}
	}
	
	// get the world fired up
	
	RiWorldBegin();
	m_inWorld = true;
}

void IECoreRI::RendererImplementation::worldEnd()
{	
	if( !m_inWorld )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::worldEnd", "Not in a world block." );
		return;
	}
	
	// we can't simply use ScopedContext here to manage our context
	// as we do in the other methods, because 3delight versions >= 11.0.0
	// actually change context in RiWorldEnd when rerendering. the old
	// context disappears off onto some background thread which performs
	// rerendering and we must now talk to a new context which is current
	// when RiWorldEnd returns.
	
	// remember the previous context so we can restore it after doing
	// the work we want in our context.
	RtContextHandle previousContext = RiGetContext();
	if( previousContext == m_context )
	{
		// we don't want to restore the previous context if it is actually
		// our own - because after RiWorldEnd it might have disappeared off onto
		// another thread.
		previousContext = 0;
	}
	
	RiContext( m_context );
	RiWorldEnd();
	m_inWorld = false;
	
	// get our new context which we can emit edits on. we can no longer make
	// calls to our original context, and we must call RiEnd() with the new one
	// rather than the old.
	m_context = RiGetContext();
	
	if( previousContext )
	{
		// restore whatever context was current when we entered this method
		RiContext( previousContext );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// transforms
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::transformBegin()
{
	if( m_inWorld )
	{
		ScopedContext scopedContext( m_context );
		RiTransformBegin();
	}
	else
	{
		m_preWorldTransform.push();
	}
}

void IECoreRI::RendererImplementation::transformEnd()
{
	if( m_inWorld )
	{
		ScopedContext scopedContext( m_context );
		RiTransformEnd();
	}
	else
	{
		m_preWorldTransform.pop();
	}
}

void IECoreRI::RendererImplementation::setTransform( const Imath::M44f &m )
{
	ScopedContext scopedContext( m_context );
	delayedMotionBegin( Transform );

	if( m_inWorld )
	{
		RtMatrix mm;
		convert( m, mm );
		RiTransform( mm );
	}
	else
	{
		m_preWorldTransform.set( m );
	}
}

void IECoreRI::RendererImplementation::setTransform( const std::string &coordinateSystem )
{
	ScopedContext scopedContext( m_context );
	delayedMotionBegin( Transform );

	if( m_inWorld )
	{
		RiCoordSysTransform( (char *)coordinateSystem.c_str() );
	}
}

Imath::M44f IECoreRI::RendererImplementation::getTransform() const
{
	return getTransform( "object" );
}

Imath::M44f IECoreRI::RendererImplementation::getTransform( const std::string &coordinateSystem ) const
{
	ScopedContext scopedContext( m_context );

	M44f result;
	RtMatrix matrix;
	if( RxTransform( (char *)coordinateSystem.c_str(), "world", 0.0f, matrix ) == 0 )
	{
		result = M44f( matrix );
	}
	else
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::getTransform", boost::format( "Unable to transform to coordinate system \"%s\"." ) % coordinateSystem );
	}

	return result;
}

void IECoreRI::RendererImplementation::concatTransform( const Imath::M44f &m )
{
	ScopedContext scopedContext( m_context );
	delayedMotionBegin( Transform );

	if( m_inWorld )
	{
		RtMatrix mm;
		convert( m, mm );
		RiConcatTransform( mm );
	}
	else
	{
		m_preWorldTransform.concatenate( m );
	}
}

void IECoreRI::RendererImplementation::coordinateSystem( const std::string &name )
{
	ScopedContext scopedContext( m_context );
	RiScopedCoordinateSystem( (char *)name.c_str() );
}

//////////////////////////////////////////////////////////////////////////////////////////
// attribute code
//////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::attributeBegin()
{
	ScopedContext scopedContext( m_context );
	m_attributeStack.push( m_attributeStack.top() );
	RiAttributeBegin();
}

void IECoreRI::RendererImplementation::attributeEnd()
{
	ScopedContext scopedContext( m_context );

	if( m_attributeStack.size() )
	{
		m_attributeStack.pop();
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::attributeEnd", "No matching attributeBegin call." );
	}
	RiAttributeEnd();
}

void IECoreRI::RendererImplementation::setAttribute( const std::string &name, IECore::ConstDataPtr value )
{
	ScopedContext scopedContext( m_context );
	SetAttributeHandlerMap::iterator it = m_setAttributeHandlers.find( name );
	if( it!=m_setAttributeHandlers.end() )
	{
		(this->*(it->second))( name, value );
	}
	else if( name.compare( 0, 3, "ri:" )==0 )
	{
		size_t i = name.find_first_of( ":", 3 );
		if( i==string::npos )
		{
			const CompoundData *compoundValue = runTimeCast<const CompoundData>( value.get() );
			if( !compoundValue )
			{
				msg( Msg::Warning, "IECoreRI::RendererImplementation::setAttribute", format( "Expected CompoundData for name matching \"ri:*\" but got \"%s\"." ) % value->typeName() );
			}
			else
			{
				ParameterList pl( compoundValue->readable() );
				RiAttributeV( (char *)name.c_str() + 3, pl.n(), pl.tokens(), pl.values() );
			}
		}
		else
		{
			string s1( name, 3, i-3 );
			string s2( name, i+1 );
			ParameterList pl( s2, value.get() );
			RiAttributeV( (char *)s1.c_str(), pl.n(), pl.tokens(), pl.values() );
		}
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		string s( name, 5 );
		ParameterList pl( s, value.get() );
		RiAttributeV( "user", pl.n(), pl.tokens(), pl.values() );
	}
	else if( name.find_first_of( ":" )!=string::npos )
	{
		// ignore attributes prefixed for some other RendererImplementation
		return;
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::setAttribute", format( "Unknown attribute \"%s\"." ) % name );
	}
}

void IECoreRI::RendererImplementation::setShadingRateAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstFloatDataPtr f = runTimeCast<const FloatData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", "ri:shadingRate attribute expects a FloatData value." );
		return;
	}

	RiShadingRate( f->readable() );
}

void IECoreRI::RendererImplementation::setMatteAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstBoolDataPtr f = runTimeCast<const BoolData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", "ri:matte attribute expects a BoolData value." );
		return;
	}

	RiMatte( f->readable() ? 1 : 0 );
}

void IECoreRI::RendererImplementation::setColorAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstColor3fDataPtr f = runTimeCast<const Color3fData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", "ri:color attribute expects a Color3fData value." );
		return;
	}

	RiColor( (RtFloat *)&(f->readable().x) );
}

void IECoreRI::RendererImplementation::setOpacityAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstColor3fDataPtr f = runTimeCast<const Color3fData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", "ri:opacity attribute expects a Color3fData value." );
		return;
	}

	RiOpacity( (RtFloat *)&(f->readable().x) );
}

void IECoreRI::RendererImplementation::setSidesAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstIntDataPtr f = runTimeCast<const IntData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", "ri:sides attribute expects an IntData value." );
		return;
	}

	RiSides( f->readable() );
}

void IECoreRI::RendererImplementation::setDoubleSidedAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstBoolDataPtr f = runTimeCast<const BoolData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", "doubleSided attribute expects a BoolData value." );
		return;
	}

	RiSides( f->readable() ? 2 : 1 );
}

void IECoreRI::RendererImplementation::setRightHandedOrientationAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstBoolDataPtr f = runTimeCast<const BoolData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", "rightHandedOrientation attribute expects a BoolData value." );
		return;
	}
	if( f->readable() )
	{
		RiOrientation( "rh" );
	}
	else
	{
		RiOrientation( "lh" );
	}
}

void IECoreRI::RendererImplementation::setGeometricApproximationAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstFloatDataPtr f = runTimeCast<const FloatData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", format( "%s attribute expects an IntData value." ) % name );
		return;
	}
	string s = string( name, name.find_last_of( ":" ) + 1 );
	to_lower( s );
	RiGeometricApproximation( (char *)s.c_str(), f->readable() );
}

void IECoreRI::RendererImplementation::setNameAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstStringDataPtr f = runTimeCast<const StringData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", format( "%s attribute expects a StringData value." ) % name );
		return;
	}
	ParameterList pl( "name", f.get() );
	RiAttributeV( "identifier", pl.n(), pl.tokens(), pl.values() );
}

void IECoreRI::RendererImplementation::setSubsurfaceAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstCompoundDataPtr c = runTimeCast<const CompoundData>( d );
	if( !c )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", format( "%s attribute expects a CompoundData value." ) % name );
		return;
	}

	CompoundDataMap::const_iterator it = c->readable().find( "visibility" );
	if( it==c->readable().end() )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", format( "%s attribute expected to contain a \"visibility\" value." ) % name );
		return;
	}

	ConstStringDataPtr v = runTimeCast<const StringData>( it->second );
	if( !v )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAttribute", format( "%s attribute \"visibility\" value must be of type StringData." ) % name );
		return;
	}

	const char *ssv = v->readable().c_str();
	RiAttribute( "visibility", "string subsurface", &ssv, 0 );

	CompoundDataPtr ssParms = c->copy();
	ssParms->writable().erase( "visibility" );
	ParameterList pl( ssParms->readable() );
	RiAttributeV( "subsurface", pl.n(), pl.tokens(), pl.values() );
}

void IECoreRI::RendererImplementation::setDetailAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstBox3fDataPtr b = runTimeCast<const Box3fData>( d );
	if( !b )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setDetailAttribute", format( "%s attribute expects a Box3fData value." ) % name );
		return;
	}
	
	RtBound bound;
	convert( b->readable(), bound );
	RiDetail( bound );
}

void IECoreRI::RendererImplementation::setDetailRangeAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	ConstFloatVectorDataPtr f = runTimeCast<const FloatVectorData>( d );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setDetailRangeAttribute", format( "%s attribute expects a FloatVectorData value." ) % name );
		return;
	}
	
	const vector<float> &values = f->readable();
	if( values.size()!=4 )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setDetailRangeAttribute", format( "Value must contain 4 elements (found %d)." ) %  values.size() );
		return;
	}
	
	RiDetailRange( values[0], values[1], values[2], values[3] );
}

void IECoreRI::RendererImplementation::setTextureCoordinatesAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	const FloatVectorData *f = runTimeCast<const FloatVectorData>( d.get() );
	if( !f )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setTextureCoordinatesAttribute", format( "%s attribute expects a FloatVectorData value." ) % name );
		return;
	}
	
	const vector<float> &values = f->readable();
	if( values.size()!=8 )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setTextureCoordinatesAttribute", format( "Value must contain 8 elements (found %d)." ) %  values.size() );
		return;
	}
	
	RiTextureCoordinates( values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7] );
}

void IECoreRI::RendererImplementation::setAutomaticInstancingAttribute( const std::string &name, IECore::ConstDataPtr d )
{
	const BoolData *b = runTimeCast<const BoolData>( d.get() );
	if( !b )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::setAutomaticInstancingAttribute", format( "%s attribute expects a BoolData value." ) % name );
		return;
	}
	
	ParameterList pl( "cortexAutomaticInstancing", b );
	RiAttributeV( "user", pl.n(), pl.tokens(), pl.values() );

}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getAttribute( const std::string &name ) const
{
	ScopedContext scopedContext( m_context );
	GetAttributeHandlerMap::const_iterator it = m_getAttributeHandlers.find( name );
	if( it!=m_getAttributeHandlers.end() )
	{
		return (this->*(it->second))( name );
	}
	else if( name.compare( 0, 3, "ri:" )==0 )
	{
		size_t i = name.find_first_of( ":", 3 );
		if( i==string::npos )
		{
			msg( Msg::Warning, "IECoreRI::RendererImplementation::getAttribute", format( "Expected attribute name matching \"ri:*:*\" but got \"%s\"." ) % name );
			return 0;
		}
		char result[16 * sizeof( RtFloat )]; // enough room for a matrix return type
		memset( result, 0, 16 * sizeof( RtFloat ) ); // 3delight has a bug where it'll try to free some random part of memory if this is not null (v 7.0.54)
		RxInfoType_t resultType;
		int resultCount;
		if( 0==RxAttribute( (char *)name.c_str()+3, result, 16 * sizeof( RtFloat ), &resultType, &resultCount ) )
		{
			return convert( result, resultType, resultCount );
		}
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		char result[16 * sizeof( RtFloat )]; // enough room for a matrix return type
		RxInfoType_t resultType;
		int resultCount;
		if( 0==RxAttribute( (char *)name.c_str(), result, 16 * sizeof( RtFloat ), &resultType, &resultCount ) )
		{
			return convert( result, resultType, resultCount );
		}
	}
	else if( name.find_first_of( ":" )!=string::npos )
	{
		// silently ignore attributes prefixed for some other RendererImplementation
		return 0;
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::getAttribute", format( "Unknown attribute \"%s\"." ) % name );
	}
	return 0;
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getShadingRateAttribute( const std::string &name ) const
{
	float result = 0;
	RxInfoType_t resultType;
	int resultCount;
	if( 0==RxAttribute( "ShadingRate", (char *)&result, sizeof( float ), &resultType, &resultCount ) )
	{
		if( resultType==RxInfoFloat && resultCount==1 )
		{
			return new FloatData( result );
		}
	}
	return 0;
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getMatteAttribute( const std::string &name ) const
{
	float result = 0;
	RxInfoType_t resultType;
	int resultCount;
	if( 0==RxAttribute( "Matte", (char *)&result, sizeof( float ), &resultType, &resultCount ) )
	{
		if( resultType==RxInfoFloat && resultCount==1 )
		{
			return new BoolData( result > 0.0f );
		}
	}
	return 0;
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getDoubleSidedAttribute( const std::string &name ) const
{
	float result = 2;
	RxInfoType_t resultType;
	int resultCount;
	if( 0==RxAttribute( "Sides", (char *)&result, sizeof( float ), &resultType, &resultCount ) )
	{
		if( resultType==RxInfoFloat && resultCount==1 )
		{
			if( result==1 )
			{
				return new BoolData( false );
			}
			else
			{
				return new BoolData( true );
			}
		}
	}
	return 0;
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getRightHandedOrientationAttribute( const std::string &name ) const
{
	char *result = 0;
	RxInfoType_t resultType;
	int resultCount;
	if( 0==RxAttribute( "Ri:Orientation", &result, sizeof( char * ), &resultType, &resultCount ) )
	{
		if( resultType==RxInfoStringV && resultCount==1 )
		{
			// it'd be nice if we were just told "rh" or "lh"
			if( 0==strcmp( result, "rh" ) )
			{
				return new BoolData( true );
			}
			else if( 0==strcmp( result, "lh" ) )
			{
				return new BoolData( false );
			}
			// but in practice we seem to be told "outside" or "inside"
			else
			{
				bool determinantNegative = determinant( getTransform() ) < 0.0f;
				if( 0==strcmp( result, "outside" ) )
				{
					return new BoolData( !determinantNegative );
				}
				else if( 0==strcmp( result, "inside" ) )
				{
					return new BoolData( determinantNegative );
				}
			}
		}
	}
	return 0;
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getNameAttribute( const std::string &name ) const
{
	char *result = 0;
	RxInfoType_t resultType;
	int resultCount;
	if( 0==RxAttribute( "identifier:name", (char *)&result, sizeof( char * ), &resultType, &resultCount ) )
	{
		if( resultType==RxInfoStringV && resultCount==1 )
		{
			return new StringData( result );
		}
	}
	return 0;
}


IECore::ConstDataPtr IECoreRI::RendererImplementation::getTextureCoordinatesAttribute( const std::string &name ) const
{
	FloatVectorDataPtr result = new FloatVectorData;
	result->writable().resize( 8 );

	RxInfoType_t resultType;
	int resultCount;
	if( 0==RxAttribute( "Ri:TextureCoordinates", result->baseWritable(), sizeof( float ) * 8, &resultType, &resultCount ) )
	{
		if( resultType==RxInfoFloat && resultCount==8 )
		{
			return result;
		}
	}
	return NULL;
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getAutomaticInstancingAttribute( const std::string &name ) const
{
	return new BoolData( automaticInstancingEnabled() );
}

bool IECoreRI::RendererImplementation::automaticInstancingEnabled() const
{
	RtInt result = 0;
	RxInfoType_t resultType;
	int resultCount;
	RxAttribute( "user:cortexAutomaticInstancing", &result, sizeof( RtInt ), &resultType, &resultCount );
	return result;
}

IECore::CachedReaderPtr IECoreRI::RendererImplementation::defaultShaderCache()
{
	static IECore::CachedReaderPtr g_defaultShaderCache = new CachedReader(
		SearchPath( getenv( "DL_SHADERS_PATH" ) ? getenv( "DL_SHADERS_PATH" ) : "", ":" )
	);
	return g_defaultShaderCache;
}

void IECoreRI::RendererImplementation::shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );

	ConstShaderPtr s = 0;
	try
	{
		s = runTimeCast<const Shader>( m_shaderCache->read( name + ".sdl" ) );
		if( !s )
		{
			msg( Msg::Warning, "IECoreRI::RendererImplementation::shader", format( "Couldn't load shader \"%s\"." ) % name );
			return;
		}
	}
	catch( const std::exception &e )
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::shader", format( "Couldn't load shader \"%s\" - %s" ) % name % e.what() );
		return;
		// we don't want exceptions to halt rendering - we'd rather just report the error below
	}
	catch( ... )
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::shader", format( "Couldn't load shader \"%s\" - an unknown exception was thrown" ) % name );
		return;
	}

	// if we are here then we loaded a shader ok. now process the parameters for it and make the ri call.

	AttributeState &state = m_attributeStack.top();
	state.primVarTypeHints.clear();
	CompoundDataMap::const_iterator it = s->blindData()->readable().find( "ri:parameterTypeHints" );
	if( it!=s->blindData()->readable().end() )
	{
		if( ConstCompoundDataPtr h = runTimeCast<const CompoundData>( it->second ) )
		{
			for( it=h->readable().begin(); it!=h->readable().end(); it++ )
			{
				if( it->second->typeId()==StringData::staticTypeId() )
				{
					state.primVarTypeHints.insert( std::pair<string, string>( it->first, staticPointerCast<const StringData>( it->second )->readable() ) );
				}
			}
		}
	}
	if( type=="surface" || type=="ri:surface" )
	{
		ParameterList pl( parameters, &state.primVarTypeHints );
		RiSurfaceV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
	}
	else if( type=="displacement" || type=="ri:displacement" )
	{
		ParameterList pl( parameters, &state.primVarTypeHints );
		RiDisplacementV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
	}
	else if( type=="atmosphere" || type=="ri:atmosphere" )
	{
		ParameterList pl( parameters, &state.primVarTypeHints );
		RiAtmosphereV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
	}
	else if( type=="interior" || type=="ri:interior" )
	{
		ParameterList pl( parameters, &state.primVarTypeHints );
		RiInteriorV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
	}
	else if( type=="exterior" || type=="ri:exterior" )
	{
		ParameterList pl( parameters, &state.primVarTypeHints );
		RiExteriorV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
	}
	else if( type=="deformation" || type=="ri:deformation" )
	{
		ParameterList pl( parameters, &state.primVarTypeHints );
		RiDeformationV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
	}
	else if( type=="shader" || type=="ri:shader" )
	{
		const StringData *handleData = 0;
		CompoundDataMap::const_iterator it = parameters.find( "__handle" );
		if( it!=parameters.end() )
		{
			handleData = runTimeCast<const StringData>( it->second.get() );
		}
		if( !handleData )
		{
			msg( Msg::Error, "IECoreRI::RendererImplementation::shader", "Must specify StringData \"__handle\" parameter for coshaders." );
			return;
		}
		CompoundDataMap parametersCopy = parameters;
		parametersCopy.erase( "__handle" );
		ParameterList pl( parametersCopy, &state.primVarTypeHints );
		RiShaderV( (char *)name.c_str(), (char *)handleData->readable().c_str(), pl.n(), pl.tokens(), pl.values() );
	}
}

void IECoreRI::RendererImplementation::light( const std::string &name, const std::string &handle, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );
	IECore::CompoundDataMap parametersCopy = parameters;
	parametersCopy["__handleid"] = new StringData( handle );

	bool areaLight = false;
	CompoundDataMap::iterator it = parametersCopy.find( "ri:areaLight" );
	if( it != parametersCopy.end() )
	{
		BoolData *b = runTimeCast<BoolData>( it->second.get() );
		if( b && b->readable() )
		{
			areaLight = true;
		}
		parametersCopy.erase( it );
	}

	ParameterList pl( parametersCopy );

	if( areaLight )
	{
		RiAreaLightSourceV( const_cast<char *>(name.c_str()), pl.n(), pl.tokens(), pl.values() );
	}
	else
	{
		RiLightSourceV( const_cast<char *>(name.c_str()), pl.n(), pl.tokens(), pl.values() );
	}
}

void IECoreRI::RendererImplementation::illuminate( const std::string &lightHandle, bool on )
{
	ScopedContext scopedContext( m_context );
	RiIlluminate( (RtLightHandle)lightHandle.c_str(), on );
}

/////////////////////////////////////////////////////////////////////////////////////////
// motion blur
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::motionBegin( const std::set<float> &times )
{
	m_delayedMotionTimes.resize( times.size() );
	unsigned int i = 0;
	for( set<float>::const_iterator it = times.begin(); it!=times.end(); it++ )
	{
		m_delayedMotionTimes[i++] = *it;
	}
	m_motionType = Pending;
}

void IECoreRI::RendererImplementation::delayedMotionBegin( MotionType type )
{
	assert( type == Transform || type == Primitive );
	
	if( m_motionType == Pending )
	{
		if( m_inWorld )
		{
			RiMotionBeginV( m_delayedMotionTimes.size(), &*(m_delayedMotionTimes.begin() ) );
		}
		else
		{
			m_preWorldTransform.motionBegin( m_delayedMotionTimes );
		}
		m_motionType = type;
		m_delayedMotionTimes.clear();
	}
}

void IECoreRI::RendererImplementation::motionEnd()
{
	ScopedContext scopedContext( m_context );
	
	if( m_motionType == Transform )
	{
		if( m_inWorld )
		{
			RiMotionEnd();
		}
		else
		{
			m_preWorldTransform.motionEnd();
		}
	}
	else
	{
		// motionType == Primitive
		if( !automaticInstancingEnabled() )
		{
			RiMotionEnd();
		}
		else
		{
		
			MurmurHash h;
			for( std::vector<float>::const_iterator it = m_delayedMotionTimes.begin(); it!=m_delayedMotionTimes.end(); it++ )
			{
				h.append( *it );
			}
			for( std::vector<IECore::ConstPrimitivePtr>::const_iterator it = m_motionPrimitives.begin(); it!=m_motionPrimitives.end(); it++ )
			{
				(*it)->hash( h );
			}
		
			std::string instanceName = "cortexAutomaticInstance" + h.toString();
			
			SharedData::ObjectHandlesMutex::scoped_lock objectHandlesLock( m_sharedData->objectHandlesMutex);

			SharedData::ObjectHandleMap::const_iterator it = m_sharedData->objectHandles.find( instanceName );
			if( it != m_sharedData->objectHandles.end() )
			{
				instance( instanceName );
			}
			else
			{
				instanceBegin( instanceName, CompoundDataMap() );
					emitPrimitiveAttributes( m_motionPrimitives[0].get() );
					RiMotionBeginV( m_delayedMotionTimes.size(), &*(m_delayedMotionTimes.begin() ) );
						for( std::vector<IECore::ConstPrimitivePtr>::const_iterator it = m_motionPrimitives.begin(); it!=m_motionPrimitives.end(); it++ )					
						{
							emitPrimitive( it->get() );
						}
					RiMotionEnd();
				instanceEnd();
				instance( instanceName );
				m_delayedMotionTimes.clear();
				m_motionPrimitives.clear();
			}
		}
	}
	m_motionType = None;
}

/////////////////////////////////////////////////////////////////////////////////////////
// primitives
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::points( size_t numPoints, const IECore::PrimitiveVariableMap &primVars )
{
	PointsPrimitivePtr points = new PointsPrimitive( numPoints );
	points->variables = primVars;
	addPrimitive( points );
	
}

void IECoreRI::RendererImplementation::disk( float radius, float z, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	DiskPrimitivePtr disk = new DiskPrimitive( radius, z, thetaMax );
	disk->variables = primVars;
	addPrimitive( disk );
}

void IECoreRI::RendererImplementation::curves( const IECore::CubicBasisf &basis, bool periodic, ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars )
{
	CurvesPrimitivePtr curves = new CurvesPrimitive();
	curves->setTopology( numVertices, basis, periodic );
	curves->variables = primVars;
	addPrimitive( curves );
}

void IECoreRI::RendererImplementation::text( const std::string &font, const std::string &text, float kerning, const IECore::PrimitiveVariableMap &primVars )
{
#ifdef IECORE_WITH_FREETYPE
	IECore::FontPtr f = 0;
	FontMap::const_iterator it = m_fonts.find( font );
	if( it!=m_fonts.end() )
	{
		f = it->second;
	}
	else
	{
		string file = m_fontSearchPath.find( font ).string();
		if( file!="" )
		{
			try
			{
				f = new IECore::Font( file );
			}
			catch( const std::exception &e )
			{
				IECore::msg( IECore::Msg::Warning, "Renderer::text", e.what() );
			}
		}
		m_fonts[font] = f;
	}

	if( !f )
	{
		IECore::msg( IECore::Msg::Warning, "Renderer::text", boost::format( "Font \"%s\" not found." ) % font );
		return;
	}

	f->setKerning( kerning );
	addPrimitive( f->mesh( text ) );
#else
	IECore::msg( IECore::Msg::Warning, "Renderer::text", "IECore was not built with FreeType support." );
#endif // IECORE_WITH_FREETYPE
}

void IECoreRI::RendererImplementation::sphere( float radius, float zMin, float zMax, float thetaMax, const IECore::PrimitiveVariableMap &primVars )
{
	SpherePrimitivePtr sphere = new SpherePrimitive( radius, zMin, zMax, thetaMax );
	sphere->variables = primVars;
	addPrimitive( sphere );
}

void IECoreRI::RendererImplementation::image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreRI::RendererImplementation::image", "Not implemented" );
}

void IECoreRI::RendererImplementation::mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECore::PrimitiveVariableMap &primVars )
{
	IECore::MeshPrimitivePtr mesh = new IECore::MeshPrimitive;
	IECore::PrimitiveVariableMap::const_iterator it = primVars.find( "P" );
	if( it == primVars.end() )
	{
		IECore::msg( IECore::Msg::Warning, "IECoreRI::RendererImplementation::mesh", "Trying to render a mesh without \"P\"" );
		return;
	}
		
	IECore::V3fVectorDataPtr pData = runTimeCast< IECore::V3fVectorData >( it->second.data );
	if( !pData )
	{
		IECore::msg( IECore::Msg::Warning, "IECoreRI::RendererImplementation::mesh", "Mesh \"P\" variable has incorrect type" );
	}
		
	mesh->setTopologyUnchecked( vertsPerFace, vertIds, pData->readable().size(), interpolation );
	mesh->variables = primVars;
	addPrimitive( mesh );
}

void IECoreRI::RendererImplementation::nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECore::PrimitiveVariableMap &primVars )
{
	NURBSPrimitivePtr nurbs = new NURBSPrimitive( uOrder, uKnot, uMin, uMax, vOrder, vKnot, vMin, vMax );
	nurbs->variables = primVars;
	addPrimitive( nurbs );
}

void IECoreRI::RendererImplementation::patchMesh( const CubicBasisf &uBasis, const CubicBasisf &vBasis, int nu, bool uPeriodic, int nv, bool vPeriodic, const PrimitiveVariableMap &primVars )
{
	bool uLinear = uBasis == CubicBasisf::linear();
	bool vLinear = vBasis == CubicBasisf::linear();

	if( uLinear != vLinear )
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::mesh", "Mismatched u/v basis.");
		return;
	}
	
	PatchMeshPrimitivePtr mesh = new PatchMeshPrimitive( nu, nv, uBasis, vBasis, uPeriodic, vPeriodic );
	mesh->variables = primVars;
	addPrimitive( mesh );
}

void IECoreRI::RendererImplementation::geometry( const std::string &type, const CompoundDataMap &topology, const PrimitiveVariableMap &primVars )
{
	ScopedContext scopedContext( m_context );

	if( type=="teapot" || type=="ri:teapot" )
	{
		RiGeometry( "teapot", 0 );
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::geometry", format( "Unsupported geometry type \"%s\"." ) % type );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// primitive processing. the primitive methods above just create IECore::Primitives
// and then pass them into addPrimitive(), where we do automatic instancing and suchlike
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::addPrimitive( IECore::ConstPrimitivePtr primitive )
{
	ScopedContext scopedContext( m_context );
	
	if( !automaticInstancingEnabled() )
	{
		if( m_motionType == None || m_motionType == Pending )
		{
			emitPrimitiveAttributes( primitive.get() );		
		}
		delayedMotionBegin( Primitive );
		emitPrimitive( primitive.get() );
	}
	else
	{
		if( !m_delayedMotionTimes.size() )
		{
			// no motion blur - emit the primitive in instanced form
			MurmurHash h;
			primitive->hash( h );
			std::string instanceName = "cortexAutomaticInstance" + h.toString();

			SharedData::ObjectHandlesMutex::scoped_lock objectHandlesLock( m_sharedData->objectHandlesMutex );
			
			SharedData::ObjectHandleMap::const_iterator it = m_sharedData->objectHandles.find( instanceName );
			if( it != m_sharedData->objectHandles.end() )
			{
				instance( instanceName );
			}
			else
			{
				instanceBegin( instanceName, CompoundDataMap() );
					emitPrimitiveAttributes( primitive.get() );
					emitPrimitive( primitive.get() );
				instanceEnd();
				instance( instanceName );
			}
		}
		else
		{
			// motion blur. queue up the primitive for later emission.
			m_motionPrimitives.push_back( primitive );
		}
	}
}

void IECoreRI::RendererImplementation::emitPrimitiveAttributes( const IECore::Primitive *primitive )
{
	switch( primitive->typeId() )
	{
		case CurvesPrimitiveTypeId :
			emitCurvesPrimitiveAttributes( static_cast<const CurvesPrimitive *>( primitive ) );
			break;
		case PatchMeshPrimitiveTypeId :
			emitPatchMeshPrimitiveAttributes( static_cast<const PatchMeshPrimitive *>( primitive ) );
			break;
		default :
			// the other primitive types don't have weird attribute state interactions
			break;
	}
}

void IECoreRI::RendererImplementation::emitCurvesPrimitiveAttributes( const IECore::CurvesPrimitive *primitive )
{
	const CubicBasisf &basis = primitive->basis();
	RtMatrix b;
	convert( basis.matrix, b );
	RiBasis( b, basis.step, b, basis.step );
}

void IECoreRI::RendererImplementation::emitPatchMeshPrimitiveAttributes( const IECore::PatchMeshPrimitive *primitive )
{
	const CubicBasisf &uBasis = primitive->uBasis();
	const CubicBasisf &vBasis = primitive->vBasis();
	RtMatrix ub, vb;
	convert( uBasis.matrix, ub );
	convert( vBasis.matrix, vb );
	RiBasis( ub, uBasis.step, vb, vBasis.step );
}

void IECoreRI::RendererImplementation::emitPrimitive( const IECore::Primitive *primitive )
{
	switch( primitive->typeId() )
	{
		case PointsPrimitiveTypeId :
			emitPointsPrimitive( static_cast<const PointsPrimitive *>( primitive ) );
			break;
		case MeshPrimitiveTypeId :
			emitMeshPrimitive( static_cast<const MeshPrimitive *>( primitive ) );
			break;
		case CurvesPrimitiveTypeId :
			emitCurvesPrimitive( static_cast<const CurvesPrimitive *>( primitive ) );
			break;
		case DiskPrimitiveTypeId :
			emitDiskPrimitive( static_cast<const DiskPrimitive *>( primitive ) );
			break;
		case SpherePrimitiveTypeId :
			emitSpherePrimitive( static_cast<const SpherePrimitive *>( primitive ) );
			break;	
		case NURBSPrimitiveTypeId :
			emitNURBSPrimitive( static_cast<const NURBSPrimitive *>( primitive ) );
			break;
		case PatchMeshPrimitiveTypeId :
			emitPatchMeshPrimitive( static_cast<const PatchMeshPrimitive *>( primitive ) );
			break;
		default :
			// shouldn't be here
			assert( 0 );
	}
}

void IECoreRI::RendererImplementation::emitPointsPrimitive( const IECore::PointsPrimitive *primitive )
{
	PrimitiveVariableList pv( primitive->variables, &( m_attributeStack.top().primVarTypeHints ) );
	RiPointsV( primitive->getNumPoints(), pv.n(), pv.tokens(), pv.values() );
}

void IECoreRI::RendererImplementation::emitDiskPrimitive( const IECore::DiskPrimitive *primitive )
{
	PrimitiveVariableList pv( primitive->variables, &( m_attributeStack.top().primVarTypeHints ) );
	RiDiskV( primitive->getZ(), primitive->getRadius(), primitive->getThetaMax(), pv.n(), pv.tokens(), pv.values() );
}

void IECoreRI::RendererImplementation::emitCurvesPrimitive( const IECore::CurvesPrimitive *primitive )
{
	PrimitiveVariableList pv( primitive->variables, &( m_attributeStack.top().primVarTypeHints ) );
	vector<int> &numVerticesV = const_cast<vector<int> &>( primitive->verticesPerCurve()->readable() );

	RiCurvesV(	(char *)( primitive->basis()==CubicBasisf::linear() ? "linear" : "cubic" ),
				numVerticesV.size(), &*( numVerticesV.begin() ),
				(char *)( primitive->periodic() ? "periodic" : "nonperiodic" ),
				pv.n(), pv.tokens(), pv.values() );
}
	
void IECoreRI::RendererImplementation::emitMeshPrimitive( const IECore::MeshPrimitive *primitive )
{
	PrimitiveVariableList pv( primitive->variables, &( m_attributeStack.top().primVarTypeHints ) );
	const std::vector<int> &vertsPerFace = primitive->verticesPerFace()->readable();
	const std::vector<int> &vertIds = primitive->vertexIds()->readable();

	if( primitive->interpolation()=="catmullClark" )
	{
		int numNames = 1;
		const char *names[] = { "interpolateboundary", 0, 0, 0, 0, 0 };
		int defaultNArgs[] = { 0, 0 };
		const int *nArgs = defaultNArgs;
		const float *floats = 0;
		const int *integers = 0;
		
		IECore::PrimitiveVariableMap::const_iterator tagIt = primitive->variables.find( "tags" );
		if( tagIt != primitive->variables.end() )
		{
			const CompoundData *tagsData = runTimeCast<const CompoundData>( tagIt->second.data.get() );
			if( tagsData )
			{
				const StringVectorData *namesData = tagsData->member<const StringVectorData>( "names" );
				const IntVectorData *nArgsData = tagsData->member<const IntVectorData>( "nArgs" );
				const FloatVectorData *floatsData = tagsData->member<const FloatVectorData>( "floats" );
				const IntVectorData *integersData = tagsData->member<const IntVectorData>( "integers" );
				if( namesData && nArgsData && floatsData && integersData )
				{
					// we only have room for 6 names in the array above as we're trying to avoid having to dynamically allocate
					// any memory here - should be ok as there are only 5 tag types currently specified by 3delight.
					numNames = std::min( (int)namesData->readable().size(), 6 );
					for( int i=0; i<numNames; i++ )
					{
						names[i] = namesData->readable()[i].c_str();
					}
					nArgs = &(nArgsData->readable()[0]);
					floats = &(floatsData->readable()[0]);
					integers = &(integersData->readable()[0]);
				}
				else
				{
					msg( Msg::Warning, "IECoreRI::RendererImplementation::emitMesh", "Primitive variable \"tags\" does not contain the required members - ignoring." );
				}
			}
			else
			{
				msg( Msg::Warning, "IECoreRI::RendererImplementation::emitMesh", "Primitive variable \"tags\" is not of type CompoundData - ignoring." );
			}
		}

		RiSubdivisionMeshV( "catmull-clark", vertsPerFace.size(), const_cast<int *>( &vertsPerFace[0] ), const_cast<int *>( &vertIds[0] ),
			numNames, (char **)names, (int *)nArgs, (int *)integers, (float *)floats,
			pv.n(), pv.tokens(), pv.values() );

		return;
	}

	if( primitive->interpolation()!="linear" )
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::emitMesh", boost::format( "Unsupported interpolation type \"%s\" - rendering as polygons." ) % primitive->interpolation() );
	}

	tbb::queuing_rw_mutex::scoped_lock lock( g_nLoopsMutex, false /* read only */ );
	if( g_nLoops.size()<vertsPerFace.size() )
	{
		lock.upgrade_to_writer();
			if( g_nLoops.size()<vertsPerFace.size() ) // checking again as i think g_nLoops may have been resized by another thread getting the write lock first
			{
				g_nLoops.resize( vertsPerFace.size(), 1 );
			}
		lock.downgrade_to_reader();
	}

	RiPointsGeneralPolygonsV( vertsPerFace.size(), &*(g_nLoops.begin()), const_cast<int *>( &vertsPerFace[0] ), const_cast<int *>( &vertIds[0] ),
		pv.n(), pv.tokens(), pv.values() );
}

void IECoreRI::RendererImplementation::emitSpherePrimitive( const IECore::SpherePrimitive *primitive )
{
	PrimitiveVariableList pv( primitive->variables, &( m_attributeStack.top().primVarTypeHints ) );
	RiSphereV(
		primitive->radius(),
		primitive->zMin() * primitive->radius(),
		primitive->zMax() * primitive->radius(),
		primitive->thetaMax(),
		pv.n(), pv.tokens(), pv.values()
	);
}

void IECoreRI::RendererImplementation::emitNURBSPrimitive( const IECore::NURBSPrimitive *primitive )
{
	PrimitiveVariableList pv( primitive->variables, &( m_attributeStack.top().primVarTypeHints ) );
	
	const std::vector<float> &uKnot = primitive->uKnot()->readable();
	const std::vector<float> &vKnot = primitive->vKnot()->readable();
	
	RiNuPatchV(
		uKnot.size() - primitive->uOrder(), // nu
		primitive->uOrder(),
		const_cast<float *>( &(uKnot[0]) ),
		primitive->uMin(),
		primitive->uMax(),
		vKnot.size() - primitive->vOrder(), // nv
		primitive->vOrder(),
		const_cast<float *>( &(vKnot[0]) ),
		primitive->vMin(),
		primitive->vMax(),
		pv.n(), pv.tokens(), pv.values()
	);
}

void IECoreRI::RendererImplementation::emitPatchMeshPrimitive( const IECore::PatchMeshPrimitive *primitive )
{
	PrimitiveVariableList pv( primitive->variables, &( m_attributeStack.top().primVarTypeHints ) );
	bool uLinear = primitive->uBasis() == CubicBasisf::linear();
	const char *type = uLinear ? "bilinear" : "bicubic";
	RiPatchMeshV(
		const_cast< char * >( type ),
		primitive->uPoints(),
		(char *)( primitive->uPeriodic() ? "periodic" : "nonperiodic" ),
		primitive->vPoints(),
		(char *)( primitive->vPeriodic() ? "periodic" : "nonperiodic" ),
		pv.n(), pv.tokens(), pv.values()
	);
}
	
/////////////////////////////////////////////////////////////////////////////////////////
// procedurals
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::procedural( IECore::Renderer::ProceduralPtr proc )
{
	ScopedContext scopedContext( m_context );

	Imath::Box3f bound = proc->bound();
	if( bound.isEmpty() )
	{
		return;
	}

	RtBound riBound;
	riBound[0] = bound.min.x;
	riBound[1] = bound.max.x;
	riBound[2] = bound.min.y;
	riBound[3] = bound.max.y;
	riBound[4] = bound.min.z;
	riBound[5] = bound.max.z;

	ProceduralData *data = new ProceduralData;
	data->procedural = proc;
	data->sharedData = m_sharedData;
	data->options = m_options;
	
#ifdef IECORERI_WITH_PROCEDURALV
	
	IECore::MurmurHash h = proc->hash();
	
	if( h == IECore::MurmurHash() )
	{
		// empty hash => no procedural level instancing
		RiProcedural( data, riBound, procSubdivide, procFree );
	}
	else
	{
		std::string hashStr = h.toString();

		// specify an instance key for procedural level instancing
		const char *tokens[] = { "instancekey" };
		const char *keyPtr = hashStr.c_str();
		void *values[] = { &keyPtr };

		RiProceduralV( data, riBound, procSubdivide, procFree, 1, tokens, values );
	}
	
#else

	RiProcedural( data, riBound, procSubdivide, procFree );
	
#endif

}

void IECoreRI::RendererImplementation::procSubdivide( void *data, float detail )
{
	ProceduralData *proceduralData = reinterpret_cast<ProceduralData *>( data );
	// we used to try to use the same IECoreRI::Renderer that had the original procedural() call issued to it.
	// this turns out to be incorrect as each procedural subdivide invocation is in a new RtContextHandle
	// and the original renderer would be trying to switch to its original context. so we just create a temporary
	// renderer which doesn't own a context and therefore use the context 3delight has arranged to call subdivide with.
	// we do however share SharedData with the parent renderer, so that we can share instances among procedurals.
	IECoreRI::RendererPtr renderer = new IECoreRI::Renderer( new RendererImplementation( proceduralData->sharedData, proceduralData->options ) );
	proceduralData->procedural->render( renderer );
}

void IECoreRI::RendererImplementation::procFree( void *data )
{
	ProceduralData *proceduralData = reinterpret_cast<ProceduralData *>( data );
	delete proceduralData;
}

/////////////////////////////////////////////////////////////////////////////////////////
// instancing
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::instanceBegin( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );
#ifdef IECORERI_WITH_OBJECTBEGINV
	// we get to choose the name for the object
	const char *tokens[] = { "__handleid", "scope" };
	const char *namePtr = name.c_str();
	// we make the instance scope global so it's possible to share instances across world blocks
	const char *scope = "global";
	const void *values[] = { &namePtr, &scope };
	SharedData::ObjectHandlesMutex::scoped_lock objectHandlesLock( m_sharedData->objectHandlesMutex );
	m_sharedData->objectHandles[name] = RiObjectBeginV( 2, (char **)&tokens, (void **)&values );
#else
	// we have to put up with a rubbish name
	SharedData::ObjectHandlesMutex::scoped_lock objectHandlesLock( m_sharedData->objectHandlesMutex );
	m_sharedData->objectHandles[name] = RiObjectBegin();
#endif
}

void IECoreRI::RendererImplementation::instanceEnd()
{
	ScopedContext scopedContext( m_context );
	RiObjectEnd();
}

void IECoreRI::RendererImplementation::instance( const std::string &name )
{
	ScopedContext scopedContext( m_context );
	
	SharedData::ObjectHandlesMutex::scoped_lock objectHandlesLock( m_sharedData->objectHandlesMutex );
	SharedData::ObjectHandleMap::const_iterator hIt = m_sharedData->objectHandles.find( name );
	if( hIt==m_sharedData->objectHandles.end() )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::instance", boost::format( "No object named \"%s\" available for instancing." ) % name );
		return;
	}
	RiObjectInstance( const_cast<void *>( hIt->second ) );
}

/////////////////////////////////////////////////////////////////////////////////////////
// commands
/////////////////////////////////////////////////////////////////////////////////////////

IECore::DataPtr IECoreRI::RendererImplementation::command( const std::string &name, const CompoundDataMap &parameters )
{
   ScopedContext scopedContext( m_context );

	CommandHandlerMap::iterator it = m_commandHandlers.find( name );
	if( it==m_commandHandlers.end() )
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::command", boost::format( "Unknown command \"%s\"" ) % name );
		return 0;
	}
	return (this->*(it->second))( name, parameters );
}

IECore::DataPtr IECoreRI::RendererImplementation::readArchiveCommand( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );

	ConstStringDataPtr nameData;
	CompoundDataMap::const_iterator it = parameters.find( "name" );
	if( it!=parameters.end() )
	{
		nameData = runTimeCast<StringData>( it->second );
	}
	if( !nameData )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::command", "ri:readArchive command expects a StringData value called \"name\"." );
		return 0;
	}
	RiReadArchiveV( (char *)nameData->readable().c_str(), 0, 0, 0, 0 );
	return 0;
}

IECore::DataPtr IECoreRI::RendererImplementation::archiveRecordCommand( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );

	ConstStringDataPtr typeData;
	ConstStringDataPtr recordData;
	CompoundDataMap::const_iterator typeIt = parameters.find( "type" );
	CompoundDataMap::const_iterator recordIt = parameters.find( "record" );
	if( typeIt!=parameters.end() )
	{
		typeData = runTimeCast<StringData>( typeIt->second );
	}
	if( recordIt!=parameters.end() )
	{
		recordData = runTimeCast<StringData>( recordIt->second );
	}

	if( !(typeData && recordData) )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::command", "ri:archiveRecord command expects StringData values called \"type\" and \"record\"." );
		return 0;
	}

	// if there are printf style format specifiers in the record then we're in trouble - we're about to pass them through a c interface which
	// isn't type safe, and not provide any additional arguments for them. try to avoid that by using boost format to catch the problem.
	try
	{
		string tested = boost::format( recordData->readable() ).str();
		RiArchiveRecord( const_cast<char *>( typeData->readable().c_str() ), const_cast<char *>( tested.c_str() ) );
	}
	catch( ... )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::command", "ri:archiveRecord \"record\" parameter appears to contain printf format specifiers." );
	}
	return 0;
}

IECore::DataPtr IECoreRI::RendererImplementation::illuminateCommand( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );

	ConstStringDataPtr handleData = 0;
	ConstBoolDataPtr stateData = 0;
	CompoundDataMap::const_iterator handleIt = parameters.find( "handle" );
	CompoundDataMap::const_iterator stateIt = parameters.find( "state" );
	if( handleIt!=parameters.end() )
	{
		handleData = runTimeCast<StringData>( handleIt->second );
	}
	if( stateIt!=parameters.end() )
	{
		stateData = runTimeCast<BoolData>( stateIt->second );
	}

	if( !(handleData && stateData) )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::command", "ri:illuminate command expects a StringData value called \"handle\" and a BoolData value called \"state\"." );
		return 0;
	}

	RiIlluminate( (void *)handleData->readable().c_str(), stateData->readable() );
	return 0;
}

void RendererImplementation::editBegin( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );
	
	ParameterList p( parameters );
	RiEditBeginV( name.c_str(), p.n(), p.tokens(), p.values() );
	
	m_inWorld = name != "option";
}

void RendererImplementation::editEnd()
{
	ScopedContext scopedContext( m_context );
	RiEditEnd();
	
	m_inWorld = false;
}
