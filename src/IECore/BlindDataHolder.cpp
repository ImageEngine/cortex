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

#include <cassert>
#include <iostream>

#include "IECore/BlindDataHolder.h"

using namespace IECore;

static IndexedIO::EntryID g_blindDataEntry("blindData");
const unsigned int BlindDataHolder::m_ioVersion = 1;

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION(BlindDataHolder);

BlindDataHolder::BlindDataHolder()
{
	m_data = new CompoundData();
}

BlindDataHolder::BlindDataHolder(CompoundDataPtr data) : m_data(data)
{
	assert(m_data);
}

BlindDataHolder::~BlindDataHolder()
{
}

CompoundData *BlindDataHolder::blindData()
{
	return m_data.get();
}

const CompoundData *BlindDataHolder::blindData() const
{
	return m_data.get();
}

void BlindDataHolder::copyFrom( const Object *other, CopyContext *context )
{
	Object::copyFrom( other, context );
	const BlindDataHolder *tOther = static_cast<const BlindDataHolder *>( other );
	m_data = context->copy<CompoundData>( tOther->m_data.get() );
}

void BlindDataHolder::save( SaveContext *context ) const
{
	Object::save( context );

	bool haveData = m_data->readable().size();

	if ( haveData )
	{
		IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
		context->save( m_data.get(), container.get(), g_blindDataEntry);
	}
}

void BlindDataHolder::load( LoadContextPtr context )
{
	Object::load( context );
	unsigned int v = m_ioVersion;
	IndexedIO::EntryID typeName = staticTypeName();

	ConstIndexedIOPtr container = context->container( typeName, v, false );
	if ( container )
	{
		m_data = context->load<CompoundData>( container.get(), g_blindDataEntry );
		assert(m_data);	
	}
	else
	{
		m_data = new CompoundData();
	}
}

bool BlindDataHolder::isEqualTo( const Object *other ) const
{
	if( !Object::isEqualTo( other ) )
	{
		return false;
	}
	const BlindDataHolder *tOther = static_cast<const BlindDataHolder *>( other );
	return m_data->isEqualTo( tOther->m_data.get() );
}

void BlindDataHolder::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Object::memoryUsage( a );
	a.accumulate( m_data.get() );
}

void BlindDataHolder::hash( MurmurHash &h ) const
{
	Object::hash( h );
	// Our hash when blindData is empty or when m_data is unnitialized is the same.
	// This is currently garanteed by CompoundData but we are safer this way.
	if ( m_data->readable().size() )
	{
		m_data->hash( h );
	}
}
