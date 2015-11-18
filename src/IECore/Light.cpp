//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#include "boost/format.hpp"

#include "IECore/Light.h"
#include "IECore/Renderer.h"
#include "IECore/MurmurHash.h"

using namespace IECore;
using namespace boost;

static IndexedIO::EntryID g_nameEntry("name");
static IndexedIO::EntryID g_handleEntry("handle");
static IndexedIO::EntryID g_parametersEntry("parameters");


const unsigned int Light::m_ioVersion = 0;
Object::TypeDescription<Light> Light::m_description;

Light::Light(  const std::string &name, const std::string &handle, const CompoundDataMap &parameters )
	: m_name( name ), m_handle( handle ), m_parameters( new CompoundData( parameters ) )
{
	if ( m_handle.empty() )
	{
		m_handle = ( boost::format( "%p" ) % this ).str();
	}
}

Light::~Light()
{
}

const std::string &Light::getName() const
{
	return m_name;
}

void Light::setName( const std::string &name )
{
	m_name = name;
}

const std::string &Light::getHandle() const
{
	return m_handle;
}

void Light::setHandle( const std::string &handle )
{
	m_handle = handle;
}

CompoundDataMap &Light::parameters()
{
	return m_parameters->writable();
}

const CompoundDataMap &Light::parameters() const
{
	return m_parameters->readable();
}

CompoundDataPtr Light::parametersData()
{
	return m_parameters;
}

const CompoundDataPtr Light::parametersData() const
{
	return m_parameters;
}

void Light::render( Renderer *renderer ) const
{
	renderer->light( m_name, m_handle, parameters() );
}

bool Light::isEqualTo( const Object *other ) const
{
	if( !StateRenderable::isEqualTo( other ) )
	{
		return false;
	}
	const Light *s = static_cast<const Light *>( other );
	if( m_name!=s->m_name )
	{
		return false;
	}
	if( m_handle!=s->m_handle )
	{
		return false;
	}
	return m_parameters->isEqualTo( s->m_parameters.get() );
}

void Light::memoryUsage( Object::MemoryAccumulator &a ) const
{
	StateRenderable::memoryUsage( a );
	a.accumulate( m_name.capacity() );
	a.accumulate( m_handle.capacity() );
	a.accumulate( m_parameters.get() );
}

void Light::copyFrom( const Object *other, CopyContext *context )
{
	StateRenderable::copyFrom( other, context );
	const Light *s = static_cast<const Light *>( other );
	m_name = s->m_name;
	m_handle = s->m_handle;
	m_parameters = context->copy<CompoundData>( s->m_parameters.get() );
}

void Light::save( SaveContext *context ) const
{
	StateRenderable::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	container->write( g_nameEntry, m_name );
	container->write( g_handleEntry, m_handle );
	context->save( m_parameters.get(), container.get(), g_parametersEntry );
}

void Light::load( LoadContextPtr context )
{
	StateRenderable::load( context );
	unsigned int v = m_ioVersion;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );
	container->read( g_nameEntry, m_name );
	container->read( g_handleEntry, m_handle );
	m_parameters = context->load<CompoundData>( container.get(), g_parametersEntry );
}

void Light::hash( MurmurHash &h ) const
{
	StateRenderable::hash( h );
	h.append( m_name );
	h.append( m_handle );
	m_parameters->hash( h );
}

