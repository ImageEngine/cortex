//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/Display.h"
#include "IECore/Renderer.h"

using namespace IECore;
using namespace Imath;
using namespace std;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( Display );

const unsigned int Display::m_ioVersion = 0;

Display::Display( const std::string &name, const std::string &type, const std::string &data, CompoundDataPtr parameters )
	:	m_name( name ), m_type( type ), m_data( data ), m_parameters( parameters )
{
}

Display::~Display()
{
}

void Display::copyFrom( ConstObjectPtr other, CopyContext *context )
{
	PreWorldRenderable::copyFrom( other, context );
	const Display *tOther = static_cast<const Display *>( other.get() );
	m_name = tOther->m_name;
	m_type = tOther->m_type;
	m_data = tOther->m_data;
	m_parameters = context->copy<CompoundData>( tOther->m_parameters );
}

void Display::save( SaveContext *context ) const
{
	PreWorldRenderable::save( context );
	IndexedIOInterfacePtr container = context->container( staticTypeName(), m_ioVersion );
	container->write( "name", m_name );
	container->write( "type", m_type );
	container->write( "data", m_data );
	context->save( m_parameters, container, "parameters" );
}

void Display::load( LoadContextPtr context )
{
	PreWorldRenderable::load( context );
	unsigned int v = m_ioVersion;
	IndexedIOInterfacePtr container = context->container( staticTypeName(), v );
	container->read( "name", m_name );
	container->read( "type", m_type );
	container->read( "data", m_data );
	m_parameters = context->load<CompoundData>( container, "parameters" );
}

bool Display::isEqualTo( ConstObjectPtr other ) const
{
	if( !PreWorldRenderable::isEqualTo( other ) )
	{
		return false;
	}
	
	const Display *tOther = static_cast<const Display *>( other.get() );

	// check name
	if( m_name!=tOther->m_name )
	{
		return false;
	}
	
	// check type
	if( m_type!=tOther->m_type )
	{
		return false;
	}
	
	// check data
	if( m_data!=tOther->m_data )
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

void Display::memoryUsage( Object::MemoryAccumulator &a ) const
{
	PreWorldRenderable::memoryUsage( a );
	a.accumulate( m_name.capacity() );
	a.accumulate( m_type.capacity() );
	a.accumulate( m_data.capacity() );
	a.accumulate( m_parameters );
}

void Display::setName( const std::string &name )
{
	m_name = name;
}

const std::string &Display::getName() const
{
	return m_name;
}

void Display::setType( const std::string &type )
{
	m_type = type;
}

const std::string &Display::getType() const
{
	return m_type;
}

void Display::setData( const std::string &data )
{
	m_data = data;
}

const std::string &Display::getData() const
{
	return m_data;
}

CompoundDataMap &Display::parameters()
{
	return m_parameters->writable();
}

const CompoundDataMap &Display::parameters() const
{
	return m_parameters->readable();
}

CompoundDataPtr Display::parametersData()
{
	return m_parameters;
}

void Display::render( RendererPtr renderer )
{
	renderer->display( m_name, m_type, m_data, m_parameters->readable() );
}
