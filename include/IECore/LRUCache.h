//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2014, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_LRUCACHE_H
#define IECORE_LRUCACHE_H

#include "tbb/spin_mutex.h"
#include "tbb/concurrent_unordered_map.h"

#include "boost/noncopyable.hpp"
#include "boost/function.hpp"

namespace IECore
{

/// A mapping from keys to values, where values are computed from keys using a user
/// supplied function. Recently computed values are stored in the cache to accelerate
/// subsequent lookups. Each value has a cost associated with it, and the cache has
/// a maximum total cost above which it will remove the least recently accessed items. 
///
/// The Key type must have a tbb_hasher implementation.
///
/// The Value type must be default constructible, copy constructible and assignable.
/// Note that Values are returned by value, and erased by assigning a default constructed
/// value. In practice this means that a smart pointer is the best choice of Value.
///
/// \threading It is safe to call the methods of LRUCache from concurrent threads.
/// \ingroup utilityGroup
template<typename Key, typename Value>
class LRUCache : private boost::noncopyable
{
	public:
	
		typedef size_t Cost;
		
		/// The GetterFunction is responsible for computing the value and cost for a cache entry
		/// when given the key. It should throw a descriptive exception if it can't get the data for
		/// any reason. It is unsafe to access the LRUCache itself from the GetterFunction.
		typedef boost::function<Value ( const Key &key, Cost &cost )> GetterFunction;
		/// The optional RemovalCallback is called whenever an item is discarded from the cache.
		///  It is unsafe to access the LRUCache itself from the RemovalCallback.
		typedef boost::function<void ( const Key &key, const Value &data )> RemovalCallback;

		LRUCache( GetterFunction getter );
		LRUCache( GetterFunction getter, Cost maxCost );
		LRUCache( GetterFunction getter, RemovalCallback removalCallback, Cost maxCost );
		virtual ~LRUCache();

		/// Retrieves an item from the cache, computing it if necessary.
		/// The item is returned by value, as it may be removed from the
		/// cache at any time by operations on another thread, or may not
		/// even be stored in the cache if it exceeds the maximum cost.
		/// Throws if the item can not be computed.
		Value get( const Key &key );

		/// Adds an item to the cache directly, bypassing the GetterFunction.
		/// Returns true for success and false on failure - failure can occur
		/// if the cost exceeds the maximum cost for the cache. Note that even
		/// when true is returned, the item may be removed from the cache by a
		/// subsequent (or concurrent) operation.
		bool set( const Key &key, const Value &value, Cost cost );

		/// Returns true if the object is in the cache. Note that the
		/// return value may be invalidated immediately by operations performed
		/// by another thread.
		bool cached( const Key &key ) const;

		/// Erases the item if it was cached. Returns true if it was cached
		/// and false if it wasn't cached and therefore wasn't removed.
		bool erase( const Key &key );

		/// Erases all cached items. Note that when this returns, the cache
		/// may have been repopulated with items if other threads have called
		/// set() or get() concurrently.
		void clear();

		/// Sets the maximum cost of the items held in the cache, discarding any
		/// items if necessary to meet the new limit.
		void setMaxCost( Cost maxCost );

		/// Returns the maximum cost.
		Cost getMaxCost() const;

		/// Returns the current cost of all cached items.
		Cost currentCost() const;

	private :
		
		// Data
		//////////////////////////////////////////////////////////////////////////

		// A function for computing values, and one for notifying of removals.
		GetterFunction m_getter;
		RemovalCallback m_removalCallback;

		// Status of each item in the cache.
		enum Status
		{
			New, // brand new unpopulated entry
			Cached, // entry complete with value
			Erased, // entry once had value but it was removed to meet cost limits
			TooCostly, // entry cost exceeds m_maxCost and therefore isn't stored
			Failed // m_getter failed when computing entry
		};
		
		// The type used to store a single cached item.
		struct CacheEntry;

		// Map from keys to items - this forms the basis of
		// our cache. The concurrent_unordered_map has the
		// following pertinent properties :
		//
		// - find(), insert() and traversal may be invoked concurrently
		// - erase() may _not_ be invoked concurrently (hence we don't use it)
		// - value access is not locked in any way (we are responsible for
		// our own locking when accessing the CacheEntry).
		typedef tbb::concurrent_unordered_map<Key, CacheEntry> Map;
		typedef typename Map::iterator MapIterator;
		typedef typename Map::const_iterator ConstMapIterator;
		typedef typename Map::value_type MapValue;
		Map m_map;

		// CacheEntry implementation - a single item of the cache.
		struct CacheEntry
		{
			CacheEntry(); // status == New, previous == next == NULL
			CacheEntry( const CacheEntry &other );
			
			Value value; // value for this item
			Cost cost; // the cost for this item
			
			// Pointers to previous and next items
			// in the LRU list. We use the CacheEntries
			// themselves to store the list because it is
			// quicker than using an external structure
			// like a std::list. Note that although the
			// purpose of the list is to track items where
			// status==Cached, the two are not updated
			// atomically, so it may _not_ be assumed
			// that status==Cached implies previous!=NULL
			// or status!=Cached implies previous==NULL
			// at any given moment.
			MapValue *previous;
			MapValue *next;
			
			char status; // status of this item
			// Mutex - must be held before accessing any
			// fields other than the list fields (previous
			// and next). To access the list fields, m_listMutex
			// must be held instead.
			tbb::spin_mutex mutex;
		};

		// Dummy MapValues to represent the start and end of our LRU list.
		// Items are moved to the end when they are accessed, and removed
		// from the start when we need to reduce costs.
		MapValue m_listStart;
		MapValue m_listEnd;
	
		// The list is inherently a serial data structure, so we must
		// protect all accesses with this mutex. The mutex _must_ be held
		// before the list fields of _any_ MapValue may be accessed.
		typedef tbb::spin_mutex ListMutex;
		ListMutex m_listMutex;
		
		// Total cost. We store the current cost atomically so it can be updated
		// concurrently by multiple threads.
		typedef tbb::atomic<Cost> AtomicCost;
		AtomicCost m_currentCost;
		Cost m_maxCost;

		// Methods
		//
		// Note that great care must be taken to properly handle the
		// CacheEntry and list mutexes to avoid deadlock. If both m_listMutex
		// and a CacheEntry::mutex must be held, the list mutex must be acquired
		// _first_, and the CacheEntry::mutex _second_. Pay attention to the
		// documentation for each method, to ensure that the right locks are held
		// at the right times.
		//////////////////////////////////////////////////////////////////////////

		// Updates the cache entry with the new value and updates m_currentCost
		// appropriately. Returns true if the value was stored and false if it
		// exceeded the maximum cost. The mutex for the CacheEntry must be held by
		// the caller.
		bool setInternal( MapValue *mapValue, const Value &value, Cost cost );
		
		// Sets the status for the cache entry to Erased, removes any
		// previously Cached value, updates m_currentCost and removes
		// the entry from the LRU list. The caller must hold m_listMutex, and
		// must _not_ hold the mutex for the cache entry.
		bool eraseInternal( MapValue *mapValue );

		// Caller must not hold any locks.
		void limitCost();

		// Either erases the item from the list, or moves it to
		// the end, depending on whether or not it is cached.
		// Caller must not hold any locks.
		void updateListPosition( MapValue *mapValue );

		// If the item is in the list, erases it, otherwise
		// does nothing. Caller must hold m_listMutex.
		void listErase( MapValue *mapValue );
		// Inserts the item at the end of the list - the
		// item must _not_ already be in the list.
		// Caller must hold m_listMutex.
		void listInsertAtEnd( MapValue *mapValue );

		static void nullRemovalCallback( const Key &key, const Value &value );

};

} // namespace IECore

#include "IECore/LRUCache.inl"

#endif // IECORE_LRUCACHE_H
