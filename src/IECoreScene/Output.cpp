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

#include "IECoreScene/Output.h"

#include "IECoreScene/Renderer.h"

#include "IECore/MurmurHash.h"

using namespace IECore;
using namespace IECoreScene;
using namespace Imath;
using namespace std;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( Output );

static IndexedIO::EntryID g_nameEntry("name");
static IndexedIO::EntryID g_typeEntry("type");
static IndexedIO::EntryID g_dataEntry("data");
static IndexedIO::EntryID g_parametersEntry("parameters");
const unsigned int Output::m_ioVersion = 0;

Output::Output( const std::string &name, const std::string &type, const std::string &data, CompoundDataPtr parameters )
	:	m_name( name ), m_type( type ), m_data( data ), m_parameters( parameters )
{
}

Output::~Output()
{
}

void Output::copyFrom( const Object *other, CopyContext *context )
{
	PreWorldRenderable::copyFrom( other, context );
	const Output *tOther = static_cast<const Output *>( other );
	m_name = tOther->m_name;
	m_type = tOther->m_type;
	m_data = tOther->m_data;
	m_parameters = context->copy<CompoundData>( tOther->m_parameters.get() );
}

void Output::save( SaveContext *context ) const
{
	PreWorldRenderable::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	container->write( g_nameEntry, m_name );
	container->write( g_typeEntry, m_type );
	container->write( g_dataEntry, m_data );
	context->save( m_parameters.get(), container.get(), g_parametersEntry );
}

void Output::load( LoadContextPtr context )
{
	PreWorldRenderable::load( context );
	unsigned int v = m_ioVersion;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );
	container->read( g_nameEntry, m_name );
	container->read( g_typeEntry, m_type );
	container->read( g_dataEntry, m_data );
	m_parameters = context->load<CompoundData>( container.get(), g_parametersEntry );
}

bool Output::isEqualTo( const Object *other ) const
{
	if( !PreWorldRenderable::isEqualTo( other ) )
	{
		return false;
	}

	const Output *tOther = static_cast<const Output *>( other );

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
	if( !m_parameters->isEqualTo( tOther->m_parameters.get() ) )
	{
		return false;
	}

	return true;
}

void Output::memoryUsage( Object::MemoryAccumulator &a ) const
{
	PreWorldRenderable::memoryUsage( a );
	a.accumulate( m_name.capacity() );
	a.accumulate( m_type.capacity() );
	a.accumulate( m_data.capacity() );
	a.accumulate( m_parameters.get() );
}

void Output::hash( MurmurHash &h ) const
{
	PreWorldRenderable::hash( h );
	h.append( m_name );
	h.append( m_type );
	h.append( m_data );
	m_parameters->hash( h );
}

void Output::setName( const std::string &name )
{
	m_name = name;
}

const std::string &Output::getName() const
{
	return m_name;
}

void Output::setType( const std::string &type )
{
	m_type = type;
}

const std::string &Output::getType() const
{
	return m_type;
}

void Output::setData( const std::string &data )
{
	m_data = data;
}

const std::string &Output::getData() const
{
	return m_data;
}

CompoundDataMap &Output::parameters()
{
	return m_parameters->writable();
}

const CompoundDataMap &Output::parameters() const
{
	return m_parameters->readable();
}

CompoundData *Output::parametersData()
{
	return m_parameters.get();
}

const IECore::CompoundData *Output::parametersData() const
{
	return m_parameters.get();
}

void Output::render( Renderer *renderer ) const
{
	renderer->display( m_name, m_type, m_data, m_parameters->readable() );
}
