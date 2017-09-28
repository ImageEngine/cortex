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

#include <algorithm>

#include "IECore/CompoundObject.h"
#include "IECore/MurmurHash.h"

using namespace IECore;
using namespace std;

static IndexedIO::EntryID g_membersEntry("members");

IE_CORE_DEFINEOBJECTTYPEDESCRIPTION( CompoundObject );

const unsigned int CompoundObject::m_ioVersion = 0;

CompoundObject::CompoundObject()
{
}

CompoundObject::~CompoundObject()
{
}

const CompoundObject::ObjectMap &CompoundObject::members() const
{
	return m_members;
}

CompoundObject::ObjectMap &CompoundObject::members()
{
	return m_members;
}

void CompoundObject::copyFrom( const Object *other, CopyContext *context )
{
	Object::copyFrom( other, context );
	const CompoundObject *tOther = static_cast<const CompoundObject *>( other );
	m_members.clear();
	for( ObjectMap::const_iterator it=tOther->m_members.begin(); it!=tOther->m_members.end(); it++ )
	{
		if ( !it->second )
		{
			throw Exception( "Cannot copy CompoundObject will NULL data pointers!" );
		}
		m_members[it->first] = context->copy<Object>( it->second.get() );
	}
}

void CompoundObject::save( SaveContext *context ) const
{
	Object::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), m_ioVersion );
	container = container->subdirectory( g_membersEntry, IndexedIO::CreateIfMissing );
	ObjectMap::const_iterator it;
	for( it=m_members.begin(); it!=m_members.end(); it++ )
	{
		context->save( it->second.get(), container.get(), it->first );
	}
}

void CompoundObject::load( LoadContextPtr context )
{
	Object::load( context );
	unsigned int v = m_ioVersion;

	ConstIndexedIOPtr container = context->container( staticTypeName(), v );
	m_members.clear();
	container = container->subdirectory( g_membersEntry );

	IndexedIO::EntryIDList memberNames;
	container->entryIds( memberNames );
	IndexedIO::EntryIDList::const_iterator it;

	for( it=memberNames.begin(); it!=memberNames.end(); it++ )
	{
		m_members[*it] = context->load<Object>( container.get(), *it );
	}
}

bool CompoundObject::isEqualTo( const Object *other ) const
{
	if( !Object::isEqualTo( other ) )
	{
		return false;
	}
	const CompoundObject *tOther = static_cast<const CompoundObject *>( other );
	if( m_members.size()!=tOther->m_members.size() )
	{
		return false;
	}
	ObjectMap::const_iterator it1 = m_members.begin();
	ObjectMap::const_iterator it2 = tOther->m_members.begin();
	while( it1!=m_members.end() )
	{
		if( it1->first!=it2->first )
		{
			return false;
		}
		if ( it1->second != it2->second )
		{
			if ( !it1->second || !it2->second )
			{
				/// either one of the pointers is NULL
				return false;
			}
			if( ! it1->second->isEqualTo( it2->second.get() ) )
			{
				return false;
			}
		}
		it1++;
		it2++;
	}
	return true;
}

void CompoundObject::memoryUsage( Object::MemoryAccumulator &a ) const
{
	Object::memoryUsage( a );
	a.accumulate( m_members.size() * sizeof( ObjectMap::value_type ) );
	for( ObjectMap::const_iterator it=m_members.begin(); it!=m_members.end(); it++ )
	{
		if ( it->second )
		{
			a.accumulate( it->second.get() );
		}
	}
}

static inline bool comp( CompoundObject::ObjectMap::const_iterator a, CompoundObject::ObjectMap::const_iterator b )
{
	return a->first.value() < b->first.value();
}

void CompoundObject::hash( MurmurHash &h ) const
{
	Object::hash( h );

	// the ObjectMap is sorted by InternedString::operator <,
	// which just compares addresses of the underlying interned object.
	// this isn't stable between multiple processes.
	std::vector<ObjectMap::const_iterator> iterators;
	iterators.reserve( m_members.size() );
	for( ObjectMap::const_iterator it=m_members.begin(); it!=m_members.end(); it++ )
	{
		iterators.push_back( it );
	}

	// so we have to sort again based on the string values
	// themselves.
	sort( iterators.begin(), iterators.end(), comp );

	// and then hash everything in the stable order.
	std::vector<ObjectMap::const_iterator>::const_iterator it;
	for( it=iterators.begin(); it!=iterators.end(); it++ )
	{
		if ( !((*it)->second) )
		{
			throw Exception( "Cannot compute hash from a CompoundObject will NULL data pointers!" );
		}
		h.append( (*it)->first.value() );
		(*it)->second->hash( h );
	}
}

CompoundObject *CompoundObject::defaultInstance()
{
	static CompoundObjectPtr o = new CompoundObject();
	return o.get();
}
