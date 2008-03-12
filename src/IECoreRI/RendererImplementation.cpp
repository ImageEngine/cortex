//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/format.hpp>

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
// Matrix conversion utility
////////////////////////////////////////////////////////////////////////

static void convertMatrix( const Imath::M44f &m, RtMatrix mm )
{
	for( unsigned int i=0; i<4; i++ )
	{
		for( unsigned int j=0; j<4; j++ )
		{
			mm[i][j] = m[i][j];
		}
	}
}
		
////////////////////////////////////////////////////////////////////////
// IECoreRI::RendererImplementation implementation
////////////////////////////////////////////////////////////////////////

const unsigned int IECoreRI::RendererImplementation::g_shaderCacheSize = 10 * 1024 * 1024;	
std::vector<int> IECoreRI::RendererImplementation::g_nLoops;

IECoreRI::RendererImplementation::RendererImplementation()
	:	m_context( 0 )
{
	constructCommon();
}

IECoreRI::RendererImplementation::RendererImplementation( const std::string &name )
{
	constructCommon();
	if( name!="" )
	{
		RiBegin( (char *)name.c_str() );
	}
	else
	{
		RiBegin( 0 );
	}
	m_context = RiGetContext();
}

void IECoreRI::RendererImplementation::constructCommon()
{
	m_camera = new Camera;
	m_camera->addStandardParameters();

	m_attributeStack.push( AttributeState() );
	
	const char *shaderPathE = getenv( "DL_SHADERS_PATH" );
	m_shaderCache = new CachedReader( SearchPath( shaderPathE ? shaderPathE : "", ":" ), g_shaderCacheSize );
	
	m_setOptionHandlers["ri:searchpath:shader"] = &IECoreRI::RendererImplementation::setShaderSearchPathOption;
	m_setOptionHandlers["ri:pixelsamples"] = &IECoreRI::RendererImplementation::setPixelSamplesOption;
	m_setOptionHandlers["ri:pixelSamples"] = &IECoreRI::RendererImplementation::setPixelSamplesOption;

	m_getOptionHandlers["shutter"] = &IECoreRI::RendererImplementation::getShutterOption;
	m_getOptionHandlers["camera:shutter"] = &IECoreRI::RendererImplementation::getShutterOption;
	m_getOptionHandlers["camera:resolution"] = &IECoreRI::RendererImplementation::getResolutionOption;
	
	m_setAttributeHandlers["ri:shadingRate"] = &IECoreRI::RendererImplementation::setShadingRateAttribute;
	m_setAttributeHandlers["ri:matte"] = &IECoreRI::RendererImplementation::setMatteAttribute;
	m_setAttributeHandlers["ri:color"] = &IECoreRI::RendererImplementation::setColorAttribute;
	m_setAttributeHandlers["color"] = &IECoreRI::RendererImplementation::setColorAttribute;
	m_setAttributeHandlers["ri:opacity"] = &IECoreRI::RendererImplementation::setOpacityAttribute;
	m_setAttributeHandlers["opacity"] = &IECoreRI::RendererImplementation::setOpacityAttribute;
	m_setAttributeHandlers["ri:sides"] = &IECoreRI::RendererImplementation::setSidesAttribute;
	m_setAttributeHandlers["doubleSided"] = &IECoreRI::RendererImplementation::setDoubleSidedAttribute;
	m_setAttributeHandlers["ri:geometricApproximation:motionFactor"] = &IECoreRI::RendererImplementation::setGeometricApproximationAttribute;
	m_setAttributeHandlers["ri:geometricApproximation:focusFactor"] = &IECoreRI::RendererImplementation::setGeometricApproximationAttribute;
	m_setAttributeHandlers["name"] = &IECoreRI::RendererImplementation::setNameAttribute;

	m_getAttributeHandlers["doubleSided"] = &IECoreRI::RendererImplementation::getDoubleSidedAttribute;
	m_getAttributeHandlers["name"] = &IECoreRI::RendererImplementation::getNameAttribute;

	m_commandHandlers["ri:readArchive"] = &IECoreRI::RendererImplementation::readArchiveCommand;
	m_commandHandlers["objectBegin"] = &IECoreRI::RendererImplementation::objectBeginCommand;
	m_commandHandlers["ri:objectBegin"] = &IECoreRI::RendererImplementation::objectBeginCommand;
	m_commandHandlers["objectEnd"] = &IECoreRI::RendererImplementation::objectEndCommand;
	m_commandHandlers["ri:objectEnd"] = &IECoreRI::RendererImplementation::objectEndCommand;
	m_commandHandlers["objectInstance"] = &IECoreRI::RendererImplementation::objectInstanceCommand;
	m_commandHandlers["ri:objectInstance"] = &IECoreRI::RendererImplementation::objectInstanceCommand;
	m_commandHandlers["ri:archiveRecord"] = &IECoreRI::RendererImplementation::archiveRecordCommand;
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
}

////////////////////////////////////////////////////////////////////////
// options
////////////////////////////////////////////////////////////////////////
	
void IECoreRI::RendererImplementation::setOption( const std::string &name, IECore::ConstDataPtr value )
{
	ScopedContext scopedContext( m_context );
	
	SetOptionHandlerMap::iterator it = m_setOptionHandlers.find( name );
	if( it!=m_setOptionHandlers.end() )
	{
		(this->*(it->second))( name, value );
	}
	else if( name.compare( 0, 3, "ri:" )==0 )
	{
		size_t i = name.find_first_of( ":", 3 );
		if( i==string::npos )
		{
			msg( Msg::Warning, "IECoreRI::RendererImplementation::setOption", format( "Expected option name matching \"ri:*:*\" but got \"%s\"." ) % name );
		}
		else
		{
			string s1( name, 3, i-3 );
			string s2( name, i+1 );
			ParameterList pl( s2, value );
			RiOptionV( (char *)s1.c_str(), pl.n(), pl.tokens(), pl.values() );
		}
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		string s( name, 5 );
		ParameterList pl( s, value );
		RiOptionV( "user", pl.n(), pl.tokens(), pl.values() );
	}
	else if( name.find_first_of( ":" )!=string::npos )
	{
		// ignore options prefixed for some other RendererImplementation
		return;
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::setOption", format( "Unknown option \"%s\"." ) % name );
	}
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getOption( const std::string &name ) const
{
	ScopedContext scopedContext( m_context );
	GetOptionHandlerMap::const_iterator it = m_getOptionHandlers.find( name );
	if( it!=m_getOptionHandlers.end() )
	{
		return (this->*(it->second))( name );
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		string s( name, 5 );
		char result[16 * sizeof( RtFloat )]; // enough room for a matrix return type
		RxInfoType_t resultType;
		int resultCount;
		if( 0==RxOption( (char *)name.c_str(), result, 16 * sizeof( RtFloat ), &resultType, &resultCount ) )
		{
			return convert( result, resultType, resultCount );
		}
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
		m_shaderCache = new CachedReader( SearchPath( s->readable(), ":" ), g_shaderCacheSize );
		ParameterList p( "shader", d );
		RiOptionV( "searchpath", p.n(), p.tokens(), p.values() ); 
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

void IECoreRI::RendererImplementation::camera( const std::string &name, IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );
	// just store the camera so we can emit it just before RiWorldBegin.
	CompoundDataPtr parameterData = (new CompoundData( parameters ))->copy();
	m_camera = new Camera( name, 0, parameterData );
	m_camera->addStandardParameters(); // it simplifies worldBegin() to know that the camera is complete
	
	CompoundDataMap::const_iterator transformIt=parameters.find( "transform" );
	if( transformIt!=parameters.end() )
	{
		if( M44fDataPtr m = runTimeCast<M44fData>( transformIt->second ) )
		{
			m_camera->setTransform( new MatrixTransform( m->readable() ) );
		}
		else
		{
			msg( Msg::Error, "IECoreRI::RendererImplementation::camera", "\"transform\" parameter should be of type M44fData." );
		}
	}
	else
	{
		m_camera->setTransform( new MatrixTransform( getTransform() ) );
	}
}

/// \todo This should be outputting several calls to display as a series of secondary displays, and also trying to find the best display
/// to be used as the primary display.
void IECoreRI::RendererImplementation::display( const std::string &name, const std::string &type, const std::string &data, IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );
	ParameterList pl( parameters );
	RiDisplayV( (char *)name.c_str(), (char *)type.c_str(), (char *)data.c_str(), pl.n(), pl.tokens(), pl.values() );
}

/////////////////////////////////////////////////////////////////////////////////////////
// world
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::worldBegin()
{
	ScopedContext scopedContext( m_context );
	// push out the camera we saved earlier
	// we can assume that all the standard parameters of the camera
	// have valid values of the right type as we called
	// addStandardParameters() in camera().
	/////////////////////////////////////////////////////////
	
	// then shutter
	CompoundDataMap::const_iterator it = m_camera->parameters().find( "shutter" );
	ConstV2fDataPtr shutterD = runTimeCast<const V2fData>( it->second );
	RiShutter( shutterD->readable()[0], shutterD->readable()[1] );
	
	// then hider
	it = m_camera->parameters().find( "ri:hider" );
	if( it!=m_camera->parameters().end() )
	{
		ConstStringDataPtr d = runTimeCast<const StringData>( it->second );
		if( d )
		{
			ParameterList p( m_camera->parameters(), "ri:hider:" );
			RiHiderV( (char *)d->readable().c_str(), p.n(), p.tokens(), p.values() );
		}
		else
		{
			msg( Msg::Error, "IECoreRI::RendererImplementation::worldBegin", "Camera \"ri:hider\" parameter should be of type StringData." );
		}
	}
	
	// then resolution
	it = m_camera->parameters().find( "resolution" );
	ConstV2iDataPtr d = runTimeCast<const V2iData>( it->second );
	RiFormat( d->readable().x, d->readable().y, 1 );
	
	// then screen window
	it = m_camera->parameters().find( "screenWindow" );
	ConstBox2fDataPtr screenWindowD = runTimeCast<const Box2fData>( it->second );
	RiScreenWindow( screenWindowD->readable().min.x, screenWindowD->readable().max.x, screenWindowD->readable().min.y, screenWindowD->readable().max.y );
	
	// then crop window
	it = m_camera->parameters().find( "cropWindow" );
	ConstBox2fDataPtr cropWindowD = runTimeCast<const Box2fData>( it->second );
	RiCropWindow( cropWindowD->readable().min.x, cropWindowD->readable().max.x, cropWindowD->readable().min.y, cropWindowD->readable().max.y );
	
	// then clipping
	it = m_camera->parameters().find( "clippingPlanes" );
	ConstV2fDataPtr clippingD = runTimeCast<const V2fData>( it->second );
	RiClipping( clippingD->readable()[0], clippingD->readable()[1] );
	
	// then projection
	it = m_camera->parameters().find( "projection" );
	ConstStringDataPtr projectionD = runTimeCast<const StringData>( it->second );
	ParameterList p( m_camera->parameters(), "projection:" );
	RiProjectionV( (char *)projectionD->readable().c_str(), p.n(), p.tokens(), p.values() );

	// transform last
	if( m_camera->getTransform() )
	{
		M44f cameraInverse = m_camera->getTransform()->transform().inverse();
		setTransform( cameraInverse );
	}
	
	RiWorldBegin();
}

void IECoreRI::RendererImplementation::worldEnd()
{
	ScopedContext scopedContext( m_context );
	RiWorldEnd();
}

/////////////////////////////////////////////////////////////////////////////////////////
// transforms
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::transformBegin()
{
	ScopedContext scopedContext( m_context );
	RiTransformBegin();
}

void IECoreRI::RendererImplementation::transformEnd()
{
	ScopedContext scopedContext( m_context );
	RiTransformEnd();
}

void IECoreRI::RendererImplementation::setTransform( const Imath::M44f &m )
{
	ScopedContext scopedContext( m_context );
	RtMatrix mm;
	convertMatrix( m, mm );
	RiTransform( mm );
}

void IECoreRI::RendererImplementation::setTransform( const std::string &coordinateSystem )
{
	ScopedContext scopedContext( m_context );
	RiCoordSysTransform( (char *)coordinateSystem.c_str() );
}

Imath::M44f IECoreRI::RendererImplementation::getTransform() const
{
	ScopedContext scopedContext( m_context );
	return getTransform( "object" );
}

Imath::M44f IECoreRI::RendererImplementation::getTransform( const std::string &coordinateSystem ) const
{
	ScopedContext scopedContext( m_context );
	
	M44f result;
	RtPoint p[4] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 0, 0 } };
	if( RiTransformPoints( (char *)coordinateSystem.c_str(), "world", 4, p ) )
	{
		V3f o = convert<Imath::V3f>( p[3] );
		V3f x = convert<Imath::V3f>( p[0] ) - o;
		V3f y = convert<Imath::V3f>( p[1] ) - o;
		V3f z = convert<Imath::V3f>( p[2] ) - o;

		result = IECore::matrixFromBasis( x, y, z, o );
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
	RtMatrix mm;
	convertMatrix( m, mm );
	RiConcatTransform( mm );
}

void IECoreRI::RendererImplementation::coordinateSystem( const std::string &name )
{
	ScopedContext scopedContext( m_context );
	RiCoordinateSystem( (char *)name.c_str() );
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
			msg( Msg::Warning, "IECoreRI::RendererImplementation::setAttribute", format( "Expected attribute name matching \"ri:*:*\" but got \"%s\"." ) % name );
			return;
		}
		string s1( name, 3, i-3 );
		string s2( name, i+1 );
		ParameterList pl( s2, value );
		RiAttributeV( (char *)s1.c_str(), pl.n(), pl.tokens(), pl.values() );
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		string s( name, 5 );
		ParameterList pl( s, value );
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
	ParameterList pl( "name", f );
	RiAttributeV( "identifier", pl.n(), pl.tokens(), pl.values() );
}

IECore::ConstDataPtr IECoreRI::RendererImplementation::getAttribute( const std::string &name ) const
{
	ScopedContext scopedContext( m_context );
	GetAttributeHandlerMap::const_iterator it = m_getAttributeHandlers.find( name );
	if( it!=m_getAttributeHandlers.end() )
	{
		return (this->*(it->second))( name );
	}
	else if( name.compare( 0, 5, "user:" )==0 )
	{
		string s( name, 5 );
		char result[16 * sizeof( RtFloat )]; // enough room for a matrix return type
		RxInfoType_t resultType;
		int resultCount;
		if( 0==RxAttribute( (char *)name.c_str(), result, 16 * sizeof( RtFloat ), &resultType, &resultCount ) )
		{
			return convert( result, resultType, resultCount );
		}
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::getAttribute", format( "Unknown attribute \"%s\"." ) % name );
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

void IECoreRI::RendererImplementation::shader( const std::string &type, const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );
	
	ConstShaderPtr s = 0;
	try 
	{
		s = runTimeCast<const Shader>( m_shaderCache->read( name + ".sdl" ) );
	}
	catch( ... )
	{
		// we don't want exceptions to halt rendering - we'd rather just report the error below
	}
	
	if( s )
	{
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
						state.primVarTypeHints.insert( std::pair<string, string>( it->first, static_pointer_cast<const StringData>( it->second )->readable() ) );
					}
				}
			}
		}
		ParameterList pl( parameters, &state.primVarTypeHints );
		if( type=="surface" || type=="ri:surface" )
		{
			RiSurfaceV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
		}
		else if( type=="displacement" || type=="ri:displacement" )
		{
			RiDisplacementV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
		}
		else if( type=="light" || type=="ri:light" )
		{
			RiLightSourceV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
		}
		else if( type=="atmosphere" || type=="ri:atmosphere" )
		{
			RiAtmosphereV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
		}
		else if( type=="interior" || type=="ri:interior" )
		{
			RiInteriorV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
		}
		else if( type=="exterior" || type=="ri:exterior" )
		{
			RiExteriorV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
		}
		else if( type=="deformation" || type=="ri:deformation" )
		{
			RiDeformationV( (char *)name.c_str(), pl.n(), pl.tokens(), pl.values() );
		}
	}
	else
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::shader", format( "Couldn't load shader \"%s\"." ) % name );
	}
}

void IECoreRI::RendererImplementation::light( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );
	shader( "light", name, parameters );
}

/////////////////////////////////////////////////////////////////////////////////////////
// motion blur
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::motionBegin( const std::set<float> times )
{
	ScopedContext scopedContext( m_context );
	static vector<float> t;
	t.resize( max( t.size(), times.size() ) );
	unsigned int i = 0;
	for( set<float>::const_iterator it = times.begin(); it!=times.end(); it++ )
	{
		t[i++] = *it;
	}
	RiMotionBeginV( times.size(), &*(t.begin() ) );
}

void IECoreRI::RendererImplementation::motionEnd()
{
	ScopedContext scopedContext( m_context );
	RiMotionEnd();
}

/////////////////////////////////////////////////////////////////////////////////////////
// primitives
/////////////////////////////////////////////////////////////////////////////////////////

Imath::Box3f IECoreRI::RendererImplementation::textExtents(const std::string & t, const float width )
{
	return Box3f();
}

void IECoreRI::RendererImplementation::points( size_t numPoints, const IECore::PrimitiveVariableMap &primVars )
{
	ScopedContext scopedContext( m_context );
	PrimitiveVariableList pv( primVars, &( m_attributeStack.top().primVarTypeHints ) );
	RiPointsV( numPoints, pv.n(), pv.tokens(), pv.values() );
}

void IECoreRI::RendererImplementation::curves( const std::string &interpolation, bool periodic, ConstIntVectorDataPtr numVertices, const IECore::PrimitiveVariableMap &primVars )
{
	ScopedContext scopedContext( m_context );
	if( interpolation!="linear" && interpolation!="cubic" )
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::curves", "Unknown interpolation type \"%s\" - should be either \"cubic\" or \"linear\"." );	
		return;
	}
	
	PrimitiveVariableList pv( primVars, &( m_attributeStack.top().primVarTypeHints ) );
	vector<int> &numVerticesV = const_cast<vector<int> &>( numVertices->readable() );
	
	RiCurvesV(	(char *)interpolation.c_str(),
				numVerticesV.size(), &*( numVerticesV.begin() ),
				(char *)( periodic ? "periodic" : "nonperiodic" ),
				pv.n(), pv.tokens(), pv.values() );
}

void IECoreRI::RendererImplementation::text(const std::string &t, const float width )
{
	msg( Msg::Warning, "IECoreRI::RendererImplementation::text", "Not implemented" );	
}

void IECoreRI::RendererImplementation::image( const Imath::Box2i &dataWindow, const Imath::Box2i &displayWindow, const IECore::PrimitiveVariableMap &primVars )
{
	msg( Msg::Warning, "IECoreRI::RendererImplementation::image", "Not implemented" );	
}

void IECoreRI::RendererImplementation::mesh( IECore::ConstIntVectorDataPtr vertsPerFace, IECore::ConstIntVectorDataPtr vertIds, const std::string &interpolation, const IECore::PrimitiveVariableMap &primVars )
{
	ScopedContext scopedContext( m_context );
	PrimitiveVariableList pv( primVars, &( m_attributeStack.top().primVarTypeHints ) );

	if( interpolation=="catmullClark" )
	{
		char *tags[] = { "interpolateboundary" };
		int nargs[] = { 0, 0 };
	
		RiSubdivisionMeshV( "catmull-clark", vertsPerFace->readable().size(), (int *)&vertsPerFace->readable()[0], (int *)&vertIds->readable()[0],
			1, tags, nargs, 0, 0,
			pv.n(), pv.tokens(), pv.values() );
			
		return;
	}
	
	if( interpolation!="linear" )
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::mesh", boost::format( "Unsupported interpolation type \"%s\" - rendering as polygons." ) % interpolation );	
	}
	
	if( g_nLoops.size()<vertsPerFace->readable().size() )
	{
		g_nLoops.resize( vertsPerFace->readable().size(), 1 );
	}

	vector<int> &vertsPerFaceV = const_cast<vector<int> &>( vertsPerFace->readable() );
	vector<int> &vertIdsV = const_cast<vector<int> &>( vertIds->readable() );

	RiPointsGeneralPolygonsV( vertsPerFaceV.size(), &*(g_nLoops.begin()), &*(vertsPerFaceV.begin()), &*(vertIdsV.begin()), 
		pv.n(), pv.tokens(), pv.values() );
			
}

void IECoreRI::RendererImplementation::nurbs( int uOrder, IECore::ConstFloatVectorDataPtr uKnot, float uMin, float uMax, int vOrder, IECore::ConstFloatVectorDataPtr vKnot, float vMin, float vMax, const IECore::PrimitiveVariableMap &primVars )
{
	ScopedContext scopedContext( m_context );

	PrimitiveVariableList pv( primVars, &( m_attributeStack.top().primVarTypeHints ) );
	RiNuPatchV(
		uKnot->readable().size() - uOrder, // nu
		uOrder,
		const_cast<float *>( &(uKnot->readable()[0]) ),
		uMin,
		uMax,
		vKnot->readable().size() - vOrder, // nv
		vOrder,
		const_cast<float *>( &(vKnot->readable()[0]) ),
		vMin,
		vMax,
		pv.n(), pv.tokens(), pv.values()
	);
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
	ProcData *data = new ProcData;
	data->proc = proc;
	data->that = this;
	RiProcedural( data, riBound, procSubdivide, procFree );
}

void IECoreRI::RendererImplementation::procSubdivide( void *data, float detail )
{
	ProcData *procData = (ProcData *)data;
	procData->proc->render( procData->that );
}

void IECoreRI::RendererImplementation::procFree( void *data )
{
	ProcData *procData = (ProcData *)data;
	delete procData;
}

/////////////////////////////////////////////////////////////////////////////////////////
// commands
/////////////////////////////////////////////////////////////////////////////////////////

void IECoreRI::RendererImplementation::command( const std::string &name, const CompoundDataMap &parameters )
{
   ScopedContext scopedContext( m_context );

	CommandHandlerMap::iterator it = m_commandHandlers.find( name );
	if( it==m_commandHandlers.end() )
	{
		msg( Msg::Warning, "IECoreRI::RendererImplementation::command", boost::format( "Unknown command \"%s\"" ) % name );
		return;
	}
	(this->*(it->second))( name, parameters );
}

void IECoreRI::RendererImplementation::readArchiveCommand( const std::string &name, const IECore::CompoundDataMap &parameters )
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
		return;
	}
	RiReadArchiveV( (char *)nameData->readable().c_str(), 0, 0, 0, 0 );
}

void IECoreRI::RendererImplementation::objectBeginCommand( const std::string &name, const IECore::CompoundDataMap &parameters )
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
		msg( Msg::Error, "IECoreRI::RendererImplementation::command", "ri:objectBegin command expects a StringData value called \"name\"." );
		return;
	}
	
#ifdef IECORERI_WITH_OBJECTBEGINV
	// we get to choose the name for the object
	ParameterList p( "__handleid", nameData );
	m_objectHandles[nameData->readable()] = RiObjectBeginV( p.n(), p.tokens(), p.values() );
#else
	// we have to put up with a rubbish name
	m_objectHandles[nameData->readable()] = RiObjectBegin();	
#endif
}

void IECoreRI::RendererImplementation::objectEndCommand( const std::string &name, const IECore::CompoundDataMap &parameters )
{
	ScopedContext scopedContext( m_context );
	RiObjectEnd();
}

void IECoreRI::RendererImplementation::objectInstanceCommand( const std::string &name, const IECore::CompoundDataMap &parameters )
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
		msg( Msg::Error, "IECoreRI::RendererImplementation::command", "ri:objectInstance command expects a StringData value called \"name\"." );
		return;
	}
	ObjectHandleMap::const_iterator hIt = m_objectHandles.find( nameData->readable() );
	if( hIt==m_objectHandles.end() )
	{
		msg( Msg::Error, "IECoreRI::RendererImplementation::command", boost::format( "No object named \"%s\" available for instancing." ) % nameData->readable() );
		return;
	}
	RiObjectInstance( const_cast<void *>( hIt->second ) );
}

void IECoreRI::RendererImplementation::archiveRecordCommand( const std::string &name, const IECore::CompoundDataMap &parameters )
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
		return;
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
}
