//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CoordinateSystem.h"
#include "IECore/Renderer.h"
#include "IECore/MurmurHash.h"
#include "IECore/Transform.h"
#include "IECore/TransformBlock.h"

using namespace IECore;
using namespace boost;

static IndexedIO::EntryID g_nameEntry("name");
static IndexedIO::EntryID g_transformEntry("transform");
const unsigned int CoordinateSystem::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( CoordinateSystem );

CoordinateSystem::CoordinateSystem()
	: m_name( "unspecified" ), m_transform( 0 )
{
}

CoordinateSystem::CoordinateSystem( const std::string &name, TransformPtr transform )
	: m_name( name ), m_transform( transform )
{
}

CoordinateSystem::~CoordinateSystem()
{
}

const std::string &CoordinateSystem::getName() const
{
	return m_name;
}

void CoordinateSystem::setName( const std::string &name )
{
	m_name = name;
}

Transform *CoordinateSystem::getTransform()
{
	return m_transform.get();
}

const Transform *CoordinateSystem::getTransform() const
{
	return m_transform.get();
}

void CoordinateSystem::setTransform( TransformPtr transform )
{
	m_transform = transform;
}

void CoordinateSystem::render( Renderer *renderer ) const
{
	TransformBlock transformBlock( renderer, m_transform );
	
	if( m_transform )
	{
		m_transform->render( renderer );
	}
	
	renderer->coordinateSystem( m_name );
}

bool CoordinateSystem::isEqualTo( const Object *other ) const
{
	if( !StateRenderable::isEqualTo( other ) )
	{
		return false;
	}
	const CoordinateSystem *c = static_cast<const CoordinateSystem *>( other );
	if( m_name != c->m_name )
	{
		return false;
	}
	if( (bool)m_transform != (bool)c->m_transform )
	{
		return false;
	}
	if( m_transform )
	{
		return m_transform->isEqualTo( c->m_transform.get() );
	}
	return true;
}

void CoordinateSystem::memoryUsage( Object::MemoryAccumulator &a ) const
{
	StateRenderable::memoryUsage( a );
	a.accumulate( m_name.capacity() + sizeof( m_name ) );
	if( m_transform )
	{
		a.accumulate( m_transform.get() );
	}
}

void CoordinateSystem::copyFrom( const Object *other, CopyContext *context )
{
	StateRenderable::copyFrom( other, context );
	const CoordinateSystem *c = static_cast<const CoordinateSystem *>( other );
	m_name = c->m_name;
	if( c->m_transform )
	{
		m_transform = context->copy<Transform>( c->m_transform.get() );
	}
	else
	{
		m_transform = 0;
	}
}

void CoordinateSystem::save( SaveContext *context ) const
{
	StateRenderable::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	container->write( g_nameEntry, m_name );
	if( m_transform )
	{
		context->save( m_transform.get(), container.get(), g_transformEntry );
	}
}

void CoordinateSystem::load( LoadContextPtr context )
{
	StateRenderable::load( context );
	unsigned int v = m_ioVersion;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );
	container->read( g_nameEntry, m_name );
	m_transform = 0;
	try
	{
		m_transform = context->load<Transform>( container.get(), g_transformEntry );
	}
	catch( ... )
	{
	}
}

void CoordinateSystem::hash( MurmurHash &h ) const
{
	StateRenderable::hash( h );
	h.append( m_name );
	if( m_transform )
	{
		m_transform->hash( h );
	}
}
