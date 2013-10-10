//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#include <map>
#include <list>

#include "tbb/recursive_mutex.h"

#include "boost/noncopyable.hpp"
#include "boost/function.hpp"

namespace IECore
{

/// A templated cache with a Least-Recently-Used disposal mechanism. Each item to be retrieved is "calculated"
/// by a function which can also state the "cost" of that piece of data. The cache has a maximum cost, and
/// attempts to add any data which would exceed this results in the LRU items being discarded.
/// Template parameters are the key by which the data is accessed and a smart pointer type which can be used
/// to point to the data.
/// \threading It should be safe to call the methods of LRUCache from concurrent threads.
/// \ingroup utilityGroup
template<typename Key, typename Ptr>
class LRUCache : private boost::noncopyable
{
	public:
	
		typedef Key KeyType;
		typedef Ptr PtrType;
		typedef size_t Cost;
		
		/// The GetterFunction is responsible for computing the value and cost for a cache entry
		/// when given the key. It should throw a descriptive exception if it can't get the data for
		/// any reason.
		typedef boost::function<Ptr ( const Key &key, Cost &cost )> GetterFunction;
		/// The optional RemovalCallback is called whenever an item is discarded from the cache.
		typedef boost::function<void ( const Key &key, const Ptr &data )> RemovalCallback;

		LRUCache( GetterFunction getter );
		LRUCache( GetterFunction getter, Cost maxCost );
		LRUCache( GetterFunction getter, RemovalCallback removalCallback, Cost maxCost );
		virtual ~LRUCache();

		void clear();

		// Erases the given key if it is contained in the cache. Returns whether any item was removed.
		bool erase( const Key &key );

		/// Set the maximum cost of the items held in the cache, discarding any items if necessary.
		void setMaxCost( Cost maxCost );

		/// Get the maximum possible cost of cacheable items
		Cost getMaxCost() const;

		/// Returns the current cost of items held in the cache
		Cost currentCost() const;

		/// Retrieves the item from the cache, computing it if necessary. Throws if the item can not be
		/// computed.
		Ptr get( const Key &key );

		/// Registers an object in the cache directly. Returns true for success and false on failure -
		/// failure can occur if the cost exceeds the maximum cost for the cache.
		bool set( const Key &key, const Ptr &data, Cost cost );

		/// Returns true if the object is in the cache.
		bool cached( const Key &key ) const;

	protected:
		
		typedef std::list<Key> List;
		typedef typename std::list<Key>::iterator ListIterator;
				
		typedef tbb::recursive_mutex Mutex;
		
		enum Status
		{
			New, // brand new unpopulated entry
			Caching, // unpopulated entry which is waiting for m_getter to return
			Cached, // entry complete with value
			Erased, // entry once had value but removed by limitCost
			TooCostly, // entry cost exceeds m_maxCost and therefore isn't stored
			Failed // m_getter failed when computing entry
		};
		
		struct CacheEntry
		{
			CacheEntry();
			
			Cost cost;
			ListIterator listIterator;
			Status status;
			Ptr data;
		};

		typedef std::map<Key, CacheEntry> Cache;
		typedef typename std::map<Key, CacheEntry>::const_iterator ConstCacheIterator;

		/// Clear out any data with a least-recently-used strategy until the current cost does not exceed the specified cost.
		void limitCost( Cost cost );

		static void nullRemovalCallback( const Key &key, const Ptr &data );

		GetterFunction m_getter;
		RemovalCallback m_removalCallback;

		mutable Mutex m_mutex;

		Cost m_maxCost;
		Cost m_currentCost;
		
		List m_list;
		Cache m_cache;
};

} // namespace IECore

#include "IECore/LRUCache.inl"

#endif // IECORE_LRUCACHE_H
