//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

using namespace IECore;

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
