//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/Options.h"
#include "IECoreScene/Renderer.h"

using namespace IECore;
using namespace IECoreScene;
using namespace boost;

static IndexedIO::EntryID g_optionsEntry("options");
const unsigned int Options::m_ioVersion = 0;
IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( Options );

Options::Options( CompoundDataPtr options )
	:	m_options( options ? options : CompoundDataPtr( new CompoundData() ) )
{
}

Options::Options( const CompoundDataMap &options )
	: m_options( new CompoundData( options ) )
{
}

Options::~Options()
{
}

CompoundDataMap &Options::options()
{
	return m_options->writable();
}

const CompoundDataMap &Options::options() const
{
	return m_options->readable();
}

CompoundDataPtr Options::optionsData()
{
	return m_options;
}

void Options::render( Renderer *renderer ) const
{
	for( CompoundDataMap::const_iterator it=options().begin(); it!=options().end(); it++ )
	{
		renderer->setOption( it->first, it->second );
	}
}

bool Options::isEqualTo( const Object *other ) const
{
	if( !PreWorldRenderable::isEqualTo( other ) )
	{
		return false;
	}
	const Options *s = static_cast<const Options *>( other );
	return m_options->isEqualTo( s->m_options.get() );
}

void Options::memoryUsage( Object::MemoryAccumulator &a ) const
{
	PreWorldRenderable::memoryUsage( a );
	a.accumulate( m_options.get() );
}

void Options::copyFrom( const Object *other, CopyContext *context )
{
	PreWorldRenderable::copyFrom( other, context );
	const Options *s = static_cast<const Options *>( other );
	m_options = context->copy<CompoundData>( s->m_options.get() );
}

void Options::save( SaveContext *context ) const
{
	PreWorldRenderable::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	context->save( m_options.get(), container.get(), g_optionsEntry );
}

void Options::load( LoadContextPtr context )
{
	PreWorldRenderable::load( context );
	unsigned int v = m_ioVersion;
	ConstIndexedIOPtr container = context->container( staticTypeName(), v );
	m_options = context->load<CompoundData>( container.get(), g_optionsEntry );
}

void Options::hash( MurmurHash &h ) const
{
	PreWorldRenderable::hash( h );
	m_options->hash( h );
}
