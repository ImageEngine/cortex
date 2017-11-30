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

#include "IECore/MurmurHash.h"
#include "IECoreScene/Shader.h"
#include "IECoreScene/Renderer.h"

using namespace IECore;
using namespace IECoreScene;
using namespace boost;

static IndexedIO::EntryID g_nameEntry("name");
static IndexedIO::EntryID g_typeEntry("type");
static IndexedIO::EntryID g_parametersEntry("parameters");

const unsigned int Shader::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( Shader );

Shader::Shader(  const std::string &name, const std::string &type, const CompoundDataMap &parameters )
	: m_name( name ), m_type( type ), m_parameters( new CompoundData( parameters ) )
{
}

Shader::~Shader()
{
}

const std::string &Shader::getName() const
{
	return m_name;
}

void Shader::setName( const std::string &name )
{
	m_name = name;
}

const std::string &Shader::getType() const
{
	return m_type;
}

void Shader::setType( const std::string &type )
{
	m_type = type;
}

CompoundDataMap &Shader::parameters()
{
	return m_parameters->writable();
}

const CompoundDataMap &Shader::parameters() const
{
	return m_parameters->readable();
}

CompoundData *Shader::parametersData()
{
	return m_parameters.get();
}

const CompoundData *Shader::parametersData() const
{
	return m_parameters.get();
}

void Shader::render( Renderer *renderer ) const
{
	renderer->shader( m_type, m_name, parameters() );
}

bool Shader::isEqualTo( const Object *other ) const
{
	if( !StateRenderable::isEqualTo( other ) )
	{
		return false;
	}
	const Shader *s = static_cast<const Shader *>( other );
	if( m_name!=s->m_name )
	{
		return false;
	}
	if( m_type!=s->m_type )
	{
		return false;
	}
	return m_parameters->isEqualTo( s->m_parameters.get() );
}

void Shader::memoryUsage( Object::MemoryAccumulator &a ) const
{
	StateRenderable::memoryUsage( a );
	a.accumulate( m_name.capacity() );
	a.accumulate( m_type.capacity() );
	a.accumulate( m_parameters.get() );
}

void Shader::copyFrom( const Object *other, CopyContext *context )
{
	StateRenderable::copyFrom( other, context );
	const Shader *s = static_cast<const Shader *>( other );
	m_name = s->m_name;
	m_type = s->m_type;
	m_parameters = context->copy<CompoundData>( s->m_parameters.get() );
}

void Shader::save( SaveContext *context ) const
{
	StateRenderable::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	container->write( g_nameEntry, m_name );
	container->write( g_typeEntry, m_type );
	context->save( m_parameters.get(), container.get(), g_parametersEntry );
}

void Shader::load( LoadContextPtr context )
{
	StateRenderable::load( context );
	unsigned int v = m_ioVersion;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );
	container->read( g_nameEntry, m_name );
	container->read( g_typeEntry, m_type );
	m_parameters = context->load<CompoundData>( container.get(), g_parametersEntry );
}

void Shader::hash( MurmurHash &h ) const
{
	StateRenderable::hash( h );
	h.append( m_name );
	h.append( m_type );
	m_parameters->hash( h );
}
