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

#include "IECore/Camera.h"
#include "IECore/Transform.h"
#include "IECore/Renderer.h"
#include "IECore/SimpleTypedData.h"

using namespace IECore;
using namespace Imath;
using namespace std;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(Camera);

const unsigned int Camera::m_ioVersion = 0;

Camera::Camera( const std::string &name, TransformPtr transform, CompoundDataPtr parameters )
	:	m_name( name ), m_transform( transform ), m_parameters( parameters )
{
}

Camera::~Camera()
{
}

void Camera::copyFrom( ConstObjectPtr other, CopyContext *context )
{
	PreWorldRenderable::copyFrom( other, context );
	const Camera *tOther = static_cast<const Camera *>( other.get() );
	m_name = tOther->m_name;
	if( tOther->m_transform )
	{
		m_transform = context->copy<Transform>( tOther->m_transform );
	}
	else
	{
		m_transform = 0;
	}
	m_parameters = context->copy<CompoundData>( tOther->m_parameters );
}

void Camera::save( SaveContext *context ) const
{
	PreWorldRenderable::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), m_ioVersion );
	container->write( "name", m_name );
	if( m_transform )
	{
		context->save( m_transform, container, "transform" );
	}
	context->save( m_parameters, container, "parameters" );
}

void Camera::load( LoadContextPtr context )
{
	PreWorldRenderable::load( context );
	unsigned int v = m_ioVersion;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	
	container->read( "name", m_name );
	m_transform = 0;
	try
	{
		m_transform = context->load<Transform>( container, "transform" );
	}
	catch( ... )
	{
	}
	m_parameters = context->load<CompoundData>( container, "parameters" );
}

bool Camera::isEqualTo( ConstObjectPtr other ) const
{
	if( !PreWorldRenderable::isEqualTo( other ) )
	{
		return false;
	}
	
	const Camera *tOther = static_cast<const Camera *>( other.get() );

	// check name
	if( m_name!=tOther->m_name )
	{
		return false;
	}
	
	// check transform
	if( (bool)m_transform != (bool)tOther->m_transform )
	{
		return false;
	}
	
	if( m_transform && !tOther->m_transform->isEqualTo( m_transform ) )
	{
		return false;
	}
	
	// check parameters
	if( !m_parameters->isEqualTo( tOther->m_parameters ) )
	{
		return false;
	}
	
	return true;
}

void Camera::memoryUsage( Object::MemoryAccumulator &a ) const
{
	PreWorldRenderable::memoryUsage( a );
	a.accumulate( m_name.capacity() );
	if( m_transform )
	{
		a.accumulate( m_transform );
	}
	a.accumulate( m_parameters );
}

void Camera::setName( const std::string &name )
{
	m_name = name;
}

const std::string &Camera::getName() const
{
	return m_name;
}
		
void Camera::setTransform( TransformPtr transform )
{
	m_transform = transform;
}

TransformPtr Camera::getTransform()
{
	return m_transform;
}

ConstTransformPtr Camera::getTransform() const
{
	return m_transform;
}

CompoundDataMap &Camera::parameters()
{
	return m_parameters->writable();
}

const CompoundDataMap &Camera::parameters() const
{
	return m_parameters->readable();
}

CompoundDataPtr Camera::parametersData()
{
	return m_parameters;
}

void Camera::addStandardParameters()
{
	// resolution
	V2i resolution( 0 );
	CompoundDataMap::const_iterator resIt=parameters().find( "resolution" );
	if( resIt != parameters().end() && resIt->second->isInstanceOf( V2iDataTypeId ) )
	{
		resolution = boost::static_pointer_cast<V2iData>( resIt->second )->readable();
	}
	if( resolution.x < 1 || resolution.y < 1 ) 
	{
		resolution = V2i( 640, 480 );
		parameters()["resolution"] = new V2iData( resolution );
	}
	
	// screen window
	Box2f screenWindow;
	CompoundDataMap::const_iterator screenWindowIt=parameters().find( "screenWindow" );
	if( screenWindowIt!=parameters().end() && screenWindowIt->second->isInstanceOf( Box2fDataTypeId ) )
	{
		screenWindow = boost::static_pointer_cast<Box2fData>( screenWindowIt->second )->readable();
	}
	if( screenWindow.isEmpty() )
	{
		float aspectRatio = (float)resolution.x/(float)resolution.y;
		if( aspectRatio < 1.0f )
		{
			screenWindow.min.x = -1;
			screenWindow.max.x = 1;
			screenWindow.min.y = -1.0f / aspectRatio;
			screenWindow.max.y = 1.0f / aspectRatio;
		}
		else
		{
			screenWindow.min.y = -1;
			screenWindow.max.y = 1;
			screenWindow.min.x = -aspectRatio;
			screenWindow.max.x = aspectRatio;			
		}
		parameters()["screenWindow"] = new Box2fData( screenWindow );
	}
	
	// crop window
	Box2f cropWindow;
	CompoundDataMap::const_iterator cropWindowIt=parameters().find( "cropWindow" );
	if( cropWindowIt!=parameters().end() && cropWindowIt->second->isInstanceOf( Box2fDataTypeId ) )
	{
		cropWindow = boost::static_pointer_cast<Box2fData>( cropWindowIt->second )->readable();
	}
	if( cropWindow.isEmpty() || cropWindow.min.x < 0.0f || cropWindow.min.y < 0.0f ||
		cropWindow.max.x > 1.0f || cropWindow.max.y > 1.0f )
	{
		cropWindow = Box2f( V2f( 0 ), V2f( 1 ) );
		parameters()["cropWindow"] = new Box2fData( cropWindow );
	}
	
	// projection
	string projection = "";
	CompoundDataMap::const_iterator projectionIt=parameters().find( "projection" );
	if( projectionIt!=parameters().end() && projectionIt->second->isInstanceOf( StringDataTypeId ) )
	{
		projection = boost::static_pointer_cast<StringData>( projectionIt->second )->readable();
	}
	if( projection=="" )
	{
		projection = "orthographic";
		parameters()["projection"] = new StringData( projection );
	}
	
	// fov
	if( projection=="perspective" )
	{
		float fov = -1.0f;
		CompoundDataMap::const_iterator fovIt=parameters().find( "projection:fov" );
		if( fovIt!=parameters().end() && fovIt->second->isInstanceOf( FloatDataTypeId ) )
		{
			fov = boost::static_pointer_cast<FloatData>( fovIt->second )->readable();
		}
		if( fov < 0.0f )
		{
			fov = 90;
			parameters()["projection:fov"] = new FloatData( fov );
		}
	}
	
	// clipping planes
	V2f clippingPlanes( -1.0f );
	CompoundDataMap::const_iterator clippingIt=parameters().find( "clippingPlanes" );
	if( clippingIt != parameters().end() && clippingIt->second->isInstanceOf( V2fDataTypeId ) )
	{
		clippingPlanes = boost::static_pointer_cast<V2fData>( clippingIt->second )->readable();
	}
	if( clippingPlanes[0] < 0.0f || clippingPlanes[1] < 0.0f )
	{
		clippingPlanes = V2f( 0.01f, 100000.0f );
		parameters()["clippingPlanes"] = new V2fData( clippingPlanes );
	}
	
	// shutter
	V2f shutter( 1.0f, -1.0f );
	CompoundDataMap::const_iterator shutterIt=parameters().find( "shutter" );
	if( shutterIt != parameters().end() && shutterIt->second->isInstanceOf( V2fDataTypeId ) )
	{
		shutter = boost::static_pointer_cast<V2fData>( shutterIt->second )->readable();
	}
	if( shutter[0] > shutter[1] )
	{
		shutter = V2f( 0.0f );
		parameters()["shutter"] = new V2fData( shutter );
	}
}

void Camera::render( RendererPtr renderer )
{
	if( m_transform )
	{
		renderer->transformBegin();
		m_transform->render( renderer );
	}
	
			/// \todo We should be calling readable() but we can't
			/// because the parameters parameter in the camera() call
			/// should be const and isn't
			renderer->camera( m_name, m_parameters->writable() );
	
	if( m_transform )
	{
		renderer->transformEnd();
	}
}
