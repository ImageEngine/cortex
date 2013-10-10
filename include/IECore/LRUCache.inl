//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "tbb/tbb_thread.h"

#include "IECore/Exception.h"

namespace IECore
{

template<typename Key, typename Ptr>
LRUCache<Key, Ptr>::CacheEntry::CacheEntry()
	:	cost( 0 ), status( New ), data()
{
}
			
template<typename Key, typename Ptr>
LRUCache<Key, Ptr>::LRUCache( GetterFunction getter )
	:	m_getter( getter ), m_removalCallback( nullRemovalCallback ), m_maxCost( 500 ), m_currentCost( 0 )
{
}

template<typename Key, typename Ptr>
LRUCache<Key, Ptr>::LRUCache( GetterFunction getter, Cost maxCost )
	:	m_getter( getter ), m_removalCallback( nullRemovalCallback ), m_maxCost( maxCost ), m_currentCost( 0 )
{
}

template<typename Key, typename Ptr>
LRUCache<Key, Ptr>::LRUCache( GetterFunction getter, RemovalCallback removalCallback, Cost maxCost )
	:	m_getter( getter ), m_removalCallback( removalCallback ), m_maxCost( maxCost ), m_currentCost( 0 )
{
}

template<typename Key, typename Ptr>
LRUCache<Key, Ptr>::~LRUCache()
{
}

template<typename Key, typename Ptr>
void LRUCache<Key, Ptr>::clear()
{
	Mutex::scoped_lock lock( m_mutex );
	
	m_currentCost = Cost(0);
	m_list.clear();

	// we don't actually remove the entries from the cache for two reasons. firstly, it would
	// invalidate iterators currently in use on other threads in get(). secondly, at
	// some point we'll hopefully collect statistics for cache misses, reloads etc,
	// and we'd want those to reflect the use of clear(). we could perhaps call limitCost( 0 )
	// instead of having separate clearing code here, but this code avoids doing the many
	// lookups that limitCost( 0 ) would do (because it removes entries in an order dictated
	// by m_list, and therefore has to do lookups in m_cache). 
	for( typename Cache::iterator it = m_cache.begin(); it != m_cache.end(); ++it )
	{
		if( it->second.status == Cached )
		{
			m_removalCallback( it->first, it->second.data );
		}
		it->second.status = Erased;
		it->second.cost = 0;
		it->second.data = Ptr();
	}
	
}


template<typename Key, typename Ptr>
void LRUCache<Key, Ptr>::setMaxCost( Cost maxCost )
{
	Mutex::scoped_lock lock( m_mutex );

	assert( maxCost >= Cost(0) );
	m_maxCost = maxCost;

	limitCost( m_maxCost );
}

template<typename Key, typename Ptr>
typename LRUCache<Key, Ptr>::Cost LRUCache<Key, Ptr>::getMaxCost() const
{
	Mutex::scoped_lock lock( m_mutex );
	return m_maxCost;
}

template<typename Key, typename Ptr>
typename LRUCache<Key, Ptr>::Cost LRUCache<Key, Ptr>::currentCost() const
{
	Mutex::scoped_lock lock( m_mutex );
	return m_currentCost;
}

template<typename Key, typename Ptr>
bool LRUCache<Key, Ptr>::cached( const Key &key ) const
{
	Mutex::scoped_lock lock( m_mutex );
	ConstCacheIterator it = m_cache.find( key );
	return ( it != m_cache.end() && it->second.status==Cached );
}

template<typename Key, typename Ptr>
Ptr LRUCache<Key, Ptr>::get( const Key& key )
{
	Mutex::scoped_lock lock( m_mutex );

	CacheEntry &cacheEntry = m_cache[key]; // creates an entry if one doesn't exist yet
	
	while( cacheEntry.status==Caching )
	{
		// another thread is doing the work. we need to wait
		// until it is done.
		lock.release();
			while( cacheEntry.status==Caching )
			{
				tbb::this_tbb_thread::yield();	
			}
		lock.acquire( m_mutex );
		
		// we use a while loop, because at this point it's possible another thread could have
		// set the status back to Caching, meaning we'd end up in the else block below and
		// trigger an assertion failure.
	}
	
	if( cacheEntry.status==New || cacheEntry.status==Erased || cacheEntry.status==TooCostly )
	{
		assert( cacheEntry.data==Ptr() );
		Ptr data = Ptr();
		Cost cost = 0;
		try
		{
			cacheEntry.status = Caching;
			lock.release(); // allows other threads to do stuff while we're computing the value
				data = m_getter( key, cost );
			lock.acquire( m_mutex );
		}
		catch( ... )
		{
			lock.acquire( m_mutex );
			cacheEntry.status = Failed;
			throw;
		}
		assert( cacheEntry.status != Cached ); // this would indicate that another thread somehow
		assert( cacheEntry.status != Failed ); // loaded the same thing as us, which is not the intention.
		set( key, data, cost );
		assert( m_list.size() <= m_cache.size() );
		return data;
	}
	else if( cacheEntry.status==Cached )
	{
		// move the entry to the front of the list
		m_list.erase( cacheEntry.listIterator );
		m_list.push_front( key );
		cacheEntry.listIterator = m_list.begin();
		assert( m_list.size() <= m_cache.size() );
		return cacheEntry.data;
	}
	else
	{
		assert( cacheEntry.status==Failed );
		throw Exception( "Previous attempt to get item failed." );
	}
}

template<typename Key, typename Ptr>
bool LRUCache<Key, Ptr>::set( const Key &key, const Ptr &data, Cost cost )
{
	Mutex::scoped_lock lock( m_mutex );

	CacheEntry &cacheEntry = m_cache[key]; // creates an entry if one doesn't exist yet
	
	if( cacheEntry.status==Cached )
	{
		m_currentCost -= cacheEntry.cost;
		cacheEntry.data = Ptr();
		m_list.erase( cacheEntry.listIterator );
	}
	
	if( cost > m_maxCost )
	{
		cacheEntry.status = TooCostly;
		return false;
	}
	
	limitCost( m_maxCost - cost );
	
	cacheEntry.data = data;
	cacheEntry.cost = cost;
	cacheEntry.status = Cached;
	m_list.push_front( key );
	cacheEntry.listIterator = m_list.begin();
	
	m_currentCost += cost;
	
	assert( m_list.size() <= m_cache.size() );
	
	return true;
}

template<typename Key, typename Ptr>
void LRUCache<Key, Ptr>::limitCost( Cost cost )
{
	Mutex::scoped_lock lock( m_mutex );

	assert( cost >= Cost(0) );

	while( m_currentCost > cost && !m_list.empty() )
	{
		bool erased = erase( m_list.back() );
		assert( erased ); (void)erased;
	}
	
	assert( m_currentCost <= cost );
}


template<typename Key, typename Ptr>
bool LRUCache<Key, Ptr>::erase( const Key &key )
{
	Mutex::scoped_lock lock( m_mutex );

	typename Cache::iterator it = m_cache.find( key );

	if( it == m_cache.end() )
	{
		return false;
	}

	if( it->second.status==Cached ) 
	{
		m_removalCallback( key, it->second.data );
		m_currentCost -= it->second.cost;
		m_list.erase( it->second.listIterator );
		it->second.data = Ptr();
	}
	
	it->second.status = Erased;
	return true;
}

template<typename Key, typename Ptr>
void LRUCache<Key, Ptr>::nullRemovalCallback( const Key &key, const Ptr &data )
{
}

} // namespace IECore

#endif // IECORE_LRUCACHE_INL
