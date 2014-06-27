//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2014, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#ifndef IECORE_LRUCACHE_INL
#define IECORE_LRUCACHE_INL

#include <cassert>
#include <iostream>

#include "IECore/Exception.h"

namespace IECore
{

template<typename Key, typename Value>
LRUCache<Key, Value>::CacheEntry::CacheEntry()
	:	value(), cost( 0 ), previous( NULL ), next( NULL ), status( New )
{
}
			
template<typename Key, typename Value>
LRUCache<Key, Value>::LRUCache( GetterFunction getter )
	:	m_getter( getter ), m_removalCallback( nullRemovalCallback ), m_maxCost( 500 )
{
	m_currentCost = 0;
	
	m_listStart.second.previous = NULL;
	m_listStart.second.next = &m_listEnd;
	
	m_listEnd.second.previous = &m_listStart;
	m_listEnd.second.next = NULL;
}

template<typename Key, typename Value>
LRUCache<Key, Value>::LRUCache( GetterFunction getter, Cost maxCost )
	:	m_getter( getter ), m_removalCallback( nullRemovalCallback ), m_maxCost( maxCost )
{
	m_currentCost = 0;
	
	m_listStart.second.previous = NULL;
	m_listStart.second.next = &m_listEnd;
		
	m_listEnd.second.previous = &m_listStart;
	m_listEnd.second.next = NULL;
}

template<typename Key, typename Value>
LRUCache<Key, Value>::LRUCache( GetterFunction getter, RemovalCallback removalCallback, Cost maxCost )
	:	m_getter( getter ), m_removalCallback( removalCallback ), m_maxCost( maxCost )
{
	m_currentCost = 0;
	
	m_listStart.second.previous = NULL;
	m_listStart.second.next = &m_listEnd;
	
	m_listEnd.second.previous = &m_listStart;
	m_listEnd.second.next = NULL;
}

template<typename Key, typename Value>
LRUCache<Key, Value>::~LRUCache()
{
}

template<typename Key, typename Value>
void LRUCache<Key, Value>::clear()
{
	ListMutex::scoped_lock listLock( m_listMutex );	
	for( typename Map::iterator it = m_map.begin(); it != m_map.end(); ++it )
	{
		eraseInternal( &*it );
	}
}

template<typename Key, typename Value>
void LRUCache<Key, Value>::setMaxCost( Cost maxCost )
{
	m_maxCost = maxCost;
	limitCost();
}

template<typename Key, typename Value>
typename LRUCache<Key, Value>::Cost LRUCache<Key, Value>::getMaxCost() const
{
	return m_maxCost;
}

template<typename Key, typename Value>
typename LRUCache<Key, Value>::Cost LRUCache<Key, Value>::currentCost() const
{
	return m_currentCost;
}

template<typename Key, typename Value>
Value LRUCache<Key, Value>::get( const Key& key )
{

	MapIterator it = m_map.insert( MapValue( key, CacheEntry() ) ).first;
	CacheEntry &cacheEntry = it->second;
	tbb::spin_mutex::scoped_lock lock( cacheEntry.mutex );
		
	if( cacheEntry.status==New || cacheEntry.status==Erased || cacheEntry.status==TooCostly )
	{
		assert( cacheEntry.value==Value() );
		assert( cacheEntry.next == NULL && cacheEntry.previous == NULL );
		
		Value value = Value();
		Cost cost = 0;
		try
		{
			value = m_getter( key, cost );
		}
		catch( ... )
		{
			cacheEntry.status = Failed;
			throw;
		}
		
		assert( cacheEntry.status != Cached ); // this would indicate that another thread somehow
		assert( cacheEntry.status != Failed ); // loaded the same thing as us, which is not the intention.
		
		setInternal( &*it, value, cost );
		
		assert( cacheEntry.status == Cached || cacheEntry.status == TooCostly );
	
		lock.release();
		
		/// \todo We might want to consider ways of avoiding having to manipulate the
		/// list for cache hits, because we want this to be our fastest code path.
		/// Adopting an approximate LRU heuristic like Second Chance would be one way
		/// of doing this.
		updateListPosition( &*it );
		limitCost();
	
		return value;
	}
	else if( cacheEntry.status==Cached )
	{
		Value result = cacheEntry.value;
		lock.release();
		updateListPosition( &*it );
		return result;
	}
	else
	{
		assert( cacheEntry.status==Failed );
		throw Exception( "Previous attempt to get item failed." );
	}
}

template<typename Key, typename Value>
bool LRUCache<Key, Value>::set( const Key &key, const Value &value, Cost cost )
{
	MapIterator it = m_map.insert( MapValue( key, CacheEntry() ) ).first;
	CacheEntry &cacheEntry = it->second;
	tbb::spin_mutex::scoped_lock lock( cacheEntry.mutex );

	const bool result = setInternal( &*it, value, cost );
	
	lock.release();
	updateListPosition( &*it );
	limitCost();
	
	return result;
}

template<typename Key, typename Value>
bool LRUCache<Key, Value>::setInternal( MapValue *mapValue, const Value &value, Cost cost )
{
	CacheEntry &cacheEntry = mapValue->second;
	if( cacheEntry.status==Cached )
	{
		m_removalCallback( mapValue->first, cacheEntry.value );
		m_currentCost -= cacheEntry.cost;
		cacheEntry.value = Value();
	}

	bool result = true;
	if( cost <= m_maxCost )
	{
		cacheEntry.value = value;
		cacheEntry.cost = cost;
		cacheEntry.status = Cached;
		m_currentCost += cost;
	}
	else
	{
		cacheEntry.status = TooCostly;
		result = false;
	}
	
	return result;
}

template<typename Key, typename Value>
bool LRUCache<Key, Value>::cached( const Key &key ) const
{
	ConstMapIterator it = m_map.find( key );
	if( it == m_map.end() )
	{
		return false;
	}
	tbb::spin_mutex::scoped_lock lock( it->second.mutex );
	return it->second.status==Cached;
}

template<typename Key, typename Value>
bool LRUCache<Key, Value>::erase( const Key &key )
{
	MapIterator it = m_map.find( key );
	if( it == m_map.end() )
	{
		return false;
	}

	ListMutex::scoped_lock listLock( m_listMutex );	
	return eraseInternal( &*it );
}

template<typename Key, typename Value>
bool LRUCache<Key, Value>::eraseInternal( MapValue *mapValue )
{	
	CacheEntry &cacheEntry = mapValue->second;
	tbb::spin_mutex::scoped_lock lock( cacheEntry.mutex );
		
	const Status originalStatus = (Status)cacheEntry.status;
	cacheEntry.status = Erased;
	
	if( originalStatus != Cached ) 
	{
		return false;
	}
	
	m_removalCallback( mapValue->first, cacheEntry.value );
	m_currentCost -= cacheEntry.cost;
	cacheEntry.value = Value();
	
	listErase( mapValue );	
	return true;
}

template<typename Key, typename Value>
void LRUCache<Key, Value>::limitCost()
{
	ListMutex::scoped_lock lock( m_listMutex );

	// While we're above the cost limit, and there are still things
	// in the list, erase the first item in the list. Note that it _is_
	// possible for the list to become empty before we meet the cost limit,
	// because another thread may have cached an item and incremented
	// m_currentCost, but still be waiting to add the item to the list,
	// because we hold m_listMutex.
	while( m_currentCost > m_maxCost && m_listStart.second.next != &m_listEnd )
	{
		eraseInternal( m_listStart.second.next );
	}
}

template<typename Key, typename Value>
void LRUCache<Key, Value>::updateListPosition( MapValue *mapValue )
{
	ListMutex::scoped_lock lock( m_listMutex );
	
	tbb::spin_mutex::scoped_lock( mapValue->second.mutex );
	
	if( mapValue->second.previous )
	{
		listErase( mapValue );
	}
	
	if( mapValue->second.status == Cached )
	{
		listInsertAtEnd( mapValue );
	}
}

template<typename Key, typename Value>
void LRUCache<Key, Value>::listErase( MapValue *mapValue )
{	
	MapValue *previous = mapValue->second.previous;	
	assert( previous );

	previous->second.next = mapValue->second.next;
	mapValue->second.next->second.previous = previous;
	mapValue->second.next = mapValue->second.previous = NULL;
}

template<typename Key, typename Value>
void LRUCache<Key, Value>::listInsertAtEnd( MapValue *mapValue )
{
	assert( mapValue->second.previous == NULL );
	assert( mapValue->second.next == NULL );

	MapValue *previous = m_listEnd.second.previous;
	previous->second.next = mapValue;
	mapValue->second.previous = previous;
	
	mapValue->second.next = &m_listEnd;
	m_listEnd.second.previous = mapValue;
}

template<typename Key, typename Value>
void LRUCache<Key, Value>::nullRemovalCallback( const Key &key, const Value &value )
{
}

} // namespace IECore

#endif // IECORE_LRUCACHE_INL
