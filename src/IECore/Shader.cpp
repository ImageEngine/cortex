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

#include "IECore/Shader.h"
#include "IECore/Renderer.h"

using namespace IECore;
using namespace boost;

const unsigned int Shader::m_ioVersion = 0;
Object::TypeDescription<Shader> Shader::m_description;

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

CompoundDataPtr Shader::parametersData()
{
	return m_parameters;
}

void Shader::render( RendererPtr renderer )
{
	renderer->shader( m_type, m_name, parameters() );
}

bool Shader::isEqualTo( ConstObjectPtr other ) const
{
	if( !StateRenderable::isEqualTo( other ) )
	{
		return false;
	}
	ConstShaderPtr s = static_pointer_cast<const Shader>( other );
	if( m_name!=s->m_name )
	{
		return false;
	}
	if( m_type!=s->m_type )
	{
		return false;
	}
	return m_parameters->isEqualTo( s->m_parameters );
}

void Shader::memoryUsage( Object::MemoryAccumulator &a ) const
{
	StateRenderable::memoryUsage( a );
	a.accumulate( m_name.capacity() );
	a.accumulate( m_type.capacity() );
	a.accumulate( m_parameters );
}

void Shader::copyFrom( ConstObjectPtr other, CopyContext *context )
{
	StateRenderable::copyFrom( other, context );
	ConstShaderPtr s = static_pointer_cast<const Shader>( other );
	m_name = s->m_name;
	m_type = s->m_type;
	m_parameters = context->copy<CompoundData>( m_parameters );
}

void Shader::save( SaveContext *context ) const
{
	StateRenderable::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), m_ioVersion );
	container->write( "name", m_name );
	container->write( "type", m_type );
	context->save( m_parameters, container, "parameters" );
}

void Shader::load( LoadContextPtr context )
{
	StateRenderable::load( context );
	unsigned int v = m_ioVersion;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	container->read( "name", m_name );
	container->read( "type", m_type );
	m_parameters = context->load<CompoundData>( container, "parameters" );
}
