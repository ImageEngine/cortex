//////////////////////////////////////////////////////////////////////////
//
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

#ifndef IECORE_COMPUTATIONCACHE_H
#define IECORE_COMPUTATIONCACHE_H

#include "boost/function.hpp"

#include "IECore/LRUCache.h"
#include "IECore/ObjectPool.h"

namespace IECore
{

/// LRUCache for generic computation that results on Object derived classes. It uses ObjectPool for the storage and retrieval of 
/// the computation results, and internally it only holds a map of computationHash to objectHash. The get functions will return the resulting 
/// Object, which should be copied prior to modification. The retrieve function will only query the cache and not force computation.
template< typename T >
class ComputationCache : public RefCounted
{
	public :

		typedef boost::function<IECore::ConstObjectPtr ( const T & )> ComputeFn;
		typedef boost::function<IECore::MurmurHash ( const T & )> HashFn;

		IE_CORE_DECLAREMEMBERPTR( ComputationCache )

		/// Constructs a cache for the given computation function and hash functions.
		/// \param maxResults Limits the number of computation results this cache will hold.
		/// \param objectPool Allows overriding the ObjectPool instance to be used for holding the resulting computed objects.
		ComputationCache( ComputeFn computeFn, HashFn hashFn, size_t maxResults = 10000, ObjectPoolPtr objectPool = ObjectPool::defaultObjectPool() );

		virtual ~ComputationCache();

		/// Removes all the stored computation information from the cache.
		void clear();

		/// Returns the maximum number of stored computations in the cache.
		size_t getMaxComputations() const;

		/// Defines the maximum number of stored computations allowed in the cache. May trigger deallocation.
		void setMaxComputations( size_t maxComputations );

		/// Returns the number of stored computations
		size_t cachedComputations() const;

		/// Returns the computation result (by computing it or retrieving from the cache if available).
		ConstObjectPtr get( const T &args );

		/// Retrieves the computation result if available on the cache, or returns NULL if not available.
		ConstObjectPtr retrieve( const T &args );

		/// Registers the result of a computation explicitly
		void set( const T &args, ConstObjectPtr obj );

	private :

		ComputeFn m_computeFn;
		HashFn m_hashFn;

		typedef IECore::LRUCache<MurmurHash, MurmurHash> Cache;
		Cache m_cache;

		ObjectPoolPtr m_objectPool;

		static MurmurHash cacheGetter( const MurmurHash &h, size_t &cost );
};


} // namespace IECore

#include "IECore/ComputationCache.inl"

#endif // IECORE_COMPUTATIONCACHE_H

