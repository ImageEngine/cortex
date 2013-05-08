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

#include "IECore/CompoundDataBase.h"
#include "IECore/TypedData.inl"

#include <iostream>
#include <algorithm>

using namespace std;
using namespace IECore;

namespace IECore
{

static IndexedIO::EntryID g_membersEntry("members");

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( CompoundDataBase, CompoundDataBaseTypeId )

template<>
void CompoundDataBase::memoryUsage( Object::MemoryAccumulator &accumulator ) const
{
	Data::memoryUsage( accumulator );
	const CompoundDataMap &data = readable();
	accumulator.accumulate( data.size() * sizeof( CompoundDataMap::value_type ) );
	
	CompoundDataMap::const_iterator iter = data.begin();
	while (iter != data.end())
	{
		if ( iter->second )
		{
			accumulator.accumulate( iter->second );
		}
		iter++;
	}	
}

template<>
void CompoundDataBase::copyFrom( const Object *other, CopyContext *context )
{
	Data::copyFrom( other, context );
	const CompoundDataBase *tOther = static_cast<const CompoundDataBase *>( other );
	CompoundDataMap &data = writable();
	data.clear();
	const CompoundDataMap &otherData = tOther->readable();
	for( CompoundDataMap::const_iterator it = otherData.begin(); it!=otherData.end(); it++ )
	{
		if ( !it->second )
		{
			throw Exception( "Cannot copy CompoundData will NULL data pointers!" );
		}
		data[it->first] = context->copy<Data>( it->second );
	}
}

template<>
bool CompoundDataBase::isEqualTo( const Object *other ) const
{
	if( !Data::isEqualTo( other ) )
	{
		return false;
	}
	const CompoundDataBase *tOther = static_cast<const CompoundDataBase *>( other );
	const CompoundDataMap &m1 = readable();
	const CompoundDataMap &m2 = tOther->readable();
	if( m1.size()!=m2.size() )
	{
		return false;
	}
	CompoundDataMap::const_iterator it1 = m1.begin();
	CompoundDataMap::const_iterator it2 = m2.begin();
	while( it1!=m1.end() )
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
			if( ! it1->second->isEqualTo( it2->second ) )
			{
				return false;
			}
		}
		it1++;
		it2++;
	}
	return true;
}

template<>
void CompoundDataBase::save( SaveContext *context ) const
{
	Data::save( context );
	IndexedIOPtr container = context->container( staticTypeName(), 0 );
	container = container->subdirectory( g_membersEntry, IndexedIO::CreateIfMissing );
	const CompoundDataMap &m = readable();
	CompoundDataMap::const_iterator it;
	for( it=m.begin(); it!=m.end(); it++ )
	{
		context->save( it->second, container, it->first );
	}
}

template<>
void CompoundDataBase::load( LoadContextPtr context )
{
	Data::load( context );
	unsigned int v = 0;
	ConstIndexedIOPtr container = 0;

	try
	{
		container = context->container( staticTypeName(), v );
	}
	catch( const IOException &e )
	{
		// probably a file with CORTEX_MAJOR_VERSION < 5, get the
		// data from CompoundData container instead.
		container = context->container( "CompoundData", v );
	}
	
	CompoundDataMap &m = writable();
	m.clear();
	container = container->subdirectory( g_membersEntry );

	IndexedIO::EntryIDList memberNames;
	container->entryIds( memberNames );
	IndexedIO::EntryIDList::const_iterator it;
	for( it=memberNames.begin(); it!=memberNames.end(); it++ )
	{
		m[*it] = context->load<Data>( container, *it );
	}
}

static inline bool comp( CompoundDataMap::const_iterator a, CompoundDataMap::const_iterator b )
{
	return a->first.value() < b->first.value();
}

template<>
void SimpleDataHolder<CompoundDataMap>::hash( MurmurHash &h ) const
{	
	// the CompoundDataMap is sorted by InternedString::operator <,
	// which just compares addresses of the underlying interned object.
	// this isn't stable between multiple processes.
	const CompoundDataMap &m = readable();
	std::vector<CompoundDataMap::const_iterator> iterators;
	iterators.reserve( m.size() );	
	for( CompoundDataMap::const_iterator it=m.begin(); it!=m.end(); it++ )
	{
		iterators.push_back( it );
	}

	// so we have to sort again based on the string values
	// themselves.
	sort( iterators.begin(), iterators.end(), comp );
	
	// and then hash everything in the stable order.
	std::vector<CompoundDataMap::const_iterator>::const_iterator it;
	for( it=iterators.begin(); it!=iterators.end(); it++ )
	{
		if ( !((*it)->second) )
		{
			throw Exception( "Cannot compute hash from a CompoundData will NULL data pointers!" );
		}

		h.append( (*it)->first.value() );
		(*it)->second->hash( h );
	}
}

} // namespace IECore

// Instantiate that bad boy.
template class TypedData<CompoundDataMap>;
