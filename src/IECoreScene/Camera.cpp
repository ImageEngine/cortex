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

#include "IECore/SimpleTypedData.h"
#include "IECore/MurmurHash.h"
#include "IECoreScene/Camera.h"
#include "IECoreScene/Transform.h"
#include "IECoreScene/Renderer.h"

using namespace IECore;
using namespace IECoreScene;
using namespace Imath;
using namespace std;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(Camera);

static IndexedIO::EntryID g_nameEntry("name");
static IndexedIO::EntryID g_transformEntry("transform");
static IndexedIO::EntryID g_parametersEntry("parameters");
const unsigned int Camera::m_ioVersion = 0;

Camera::Camera( const std::string &name, TransformPtr transform, CompoundDataPtr parameters )
	:	m_name( name ), m_transform( transform ), m_parameters( parameters )
{
}

Camera::~Camera()
{
}

void Camera::copyFrom( const Object *other, CopyContext *context )
{
	PreWorldRenderable::copyFrom( other, context );
	const Camera *tOther = static_cast<const Camera *>( other );
	m_name = tOther->m_name;
	if( tOther->m_transform )
	{
		m_transform = context->copy<Transform>( tOther->m_transform.get() );
	}
	else
	{
		m_transform = nullptr;
	}
	m_parameters = context->copy<CompoundData>( tOther->m_parameters.get() );
}

void Camera::save( SaveContext *context ) const
{
	PreWorldRenderable::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	container->write( g_nameEntry, m_name );
	if( m_transform )
	{
		context->save( m_transform.get(), container.get(), g_transformEntry );
	}
	context->save( m_parameters.get(), container.get(), g_parametersEntry );
}

void Camera::load( LoadContextPtr context )
{
	PreWorldRenderable::load( context );
	unsigned int v = m_ioVersion;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );

	container->read( g_nameEntry, m_name );
	m_transform = nullptr;
	try
	{
		m_transform = context->load<Transform>( container.get(), g_transformEntry );
	}
	catch( ... )
	{
	}
	m_parameters = context->load<CompoundData>( container.get(), g_parametersEntry );
}

bool Camera::isEqualTo( const Object *other ) const
{
	if( !PreWorldRenderable::isEqualTo( other ) )
	{
		return false;
	}

	const Camera *tOther = static_cast<const Camera *>( other );

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

	if( m_transform && !tOther->m_transform->isEqualTo( m_transform.get() ) )
	{
		return false;
	}

	// check parameters
	if( !m_parameters->isEqualTo( tOther->m_parameters.get() ) )
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
		a.accumulate( m_transform.get() );
	}
	a.accumulate( m_parameters.get() );
}

void Camera::hash( MurmurHash &h ) const
{
	PreWorldRenderable::hash( h );
	h.append( m_name );
	if( m_transform )
	{
		m_transform->hash( h );
	}
	m_parameters->hash( h );
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

Transform *Camera::getTransform()
{
	return m_transform.get();
}

const Transform *Camera::getTransform() const
{
	return m_transform.get();
}

CompoundDataMap &Camera::parameters()
{
	return m_parameters->writable();
}

const CompoundDataMap &Camera::parameters() const
{
	return m_parameters->readable();
}

CompoundData *Camera::parametersData()
{
	return m_parameters.get();
}

const CompoundData *Camera::parametersData() const
{
	return m_parameters.get();
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

	// pixel aspect ratio
	float pixelAspectRatio = 0.0f;
	if( FloatData *pixelAspectRatioData = parametersData()->member<FloatData>( "pixelAspectRatio" ) )
	{
		pixelAspectRatio = pixelAspectRatioData->readable();
	}
	if( pixelAspectRatio == 0.0f )
	{
		pixelAspectRatio = 1.0f;
		parameters()["pixelAspectRatio"] = new FloatData( pixelAspectRatio );
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
		float aspectRatio = ((float)resolution.x * pixelAspectRatio)/(float)resolution.y;
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
	if( cropWindow.isEmpty() )
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
	CompoundDataMap::const_iterator clippingIt=parameters().find( "clippingPlanes" );
	if( !( clippingIt != parameters().end() && clippingIt->second->isInstanceOf( V2fDataTypeId ) ) )
	{
		parameters()["clippingPlanes"] = new V2fData( V2f( 0.01f, 100000.0f ) );
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

void Camera::render( Renderer *renderer ) const
{
	if( m_transform )
	{
		renderer->transformBegin();
		m_transform->render( renderer );
	}

		renderer->camera( m_name, m_parameters->readable() );

	if( m_transform )
	{
		renderer->transformEnd();
	}
}
