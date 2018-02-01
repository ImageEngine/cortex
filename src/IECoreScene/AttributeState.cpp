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

#include "IECoreScene/AttributeState.h"

#include "IECoreScene/Renderer.h"

using namespace IECore;
using namespace IECoreScene;
using namespace boost;

static IndexedIO::EntryID g_attributesEntry("attributes");
const unsigned int AttributeState::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( AttributeState );

AttributeState::AttributeState( CompoundDataPtr attributes )
	:	m_attributes( attributes ? attributes : CompoundDataPtr( new CompoundData() ) )
{
}

AttributeState::AttributeState( const CompoundDataMap &attributes )
	: m_attributes( new CompoundData( attributes ) )
{
}

AttributeState::~AttributeState()
{
}

CompoundDataMap &AttributeState::attributes()
{
	return m_attributes->writable();
}

const CompoundDataMap &AttributeState::attributes() const
{
	return m_attributes->readable();
}

CompoundDataPtr AttributeState::attributesData()
{
	return m_attributes;
}

void AttributeState::render( Renderer *renderer ) const
{
	for( CompoundDataMap::const_iterator it=attributes().begin(); it!=attributes().end(); it++ )
	{
		renderer->setAttribute( it->first, it->second );
	}
}

bool AttributeState::isEqualTo( const Object *other ) const
{
	if( !StateRenderable::isEqualTo( other ) )
	{
		return false;
	}
	const AttributeState *s = static_cast<const AttributeState *>( other );
	return m_attributes->isEqualTo( s->m_attributes.get() );
}

void AttributeState::memoryUsage( Object::MemoryAccumulator &a ) const
{
	StateRenderable::memoryUsage( a );
	a.accumulate( m_attributes.get() );
}

void AttributeState::copyFrom( const Object *other, CopyContext *context )
{
	StateRenderable::copyFrom( other, context );
	const AttributeState *s = static_cast<const AttributeState *>( other );
	m_attributes = context->copy<CompoundData>( s->m_attributes.get() );
}

void AttributeState::save( SaveContext *context ) const
{
	StateRenderable::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	context->save( m_attributes.get(), container.get(), g_attributesEntry );
}

void AttributeState::load( LoadContextPtr context )
{
	StateRenderable::load( context );
	unsigned int v = m_ioVersion;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );
	m_attributes = context->load<CompoundData>( container.get(), g_attributesEntry );
}

void AttributeState::hash( MurmurHash &h ) const
{
	StateRenderable::hash( h );
	m_attributes->hash( h );
}
