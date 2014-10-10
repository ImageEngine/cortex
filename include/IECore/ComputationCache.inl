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

#ifndef IECORE_COMPUTATIONCACHE_INL
#define IECORE_COMPUTATIONCACHE_INL

#include "IECore/MessageHandler.h"

namespace IECore
{

template< typename T >
ComputationCache<T>::ComputationCache( ComputeFn computeFn, HashFn hashFn, size_t maxResults, ObjectPoolPtr objectPool ) : 
	m_computeFn(computeFn), m_hashFn(hashFn), m_cache( &ComputationCache<T>::cacheGetter, maxResults), m_objectPool(objectPool)
{
}

template< typename T >
ComputationCache<T>::~ComputationCache()
{
}

template< typename T >
void ComputationCache<T>::clear()
{
	m_cache.clear();
}

template< typename T >
void ComputationCache<T>::erase( const T &args )
{
	MurmurHash computationHash = m_hashFn(args);
	m_cache.erase( computationHash );
}

template< typename T >
size_t ComputationCache<T>::getMaxComputations() const
{
	return m_cache.getMaxCost();
}

template< typename T >
void ComputationCache<T>::setMaxComputations( size_t maxComputations )
{
	m_cache.setMaxCost(maxComputations);
}

template< typename T >
size_t ComputationCache<T>::cachedComputations() const
{
	return m_cache.currentCost();
}

template< typename T >
ConstObjectPtr ComputationCache<T>::get( const T &args, ComputationCache::MissingBehaviour missingBehaviour )
{
	ConstObjectPtr obj(0);
	MurmurHash computationHash = m_hashFn(args);
	MurmurHash objectHash = m_cache.get(computationHash);

	if ( objectHash == MurmurHash() )
	{
		/// don't know the computation hash... check the missing behaviour
		if ( missingBehaviour == ThrowIfMissing )
		{
			throw Exception( "Computation not available in the cache!" );
		}
		else if ( missingBehaviour == NullIfMissing )
		{
			return 0;
		}
		obj = m_computeFn(args);
		if ( obj )
		{
			m_cache.set( computationHash, obj->hash(), 1 );
			obj = m_objectPool->store( obj.get(), ObjectPool::StoreReference );
		}
	}
	else
	{
		obj = m_objectPool->retrieve(objectHash);
		if ( !obj )
		{
			/// the computation result was not in the object pool.... check the missing behavour
			if ( missingBehaviour == ThrowIfMissing )
			{
				throw Exception( "Computation result not available in the cache!" );
			}
			else if ( missingBehaviour == NullIfMissing )
			{
				return 0;
			}
			obj = m_computeFn(args);
			if ( obj )
			{
				obj = m_objectPool->store( obj.get(), ObjectPool::StoreReference );
				MurmurHash h = obj->hash();
				if ( h != objectHash )
				{
					/// the computation returned a different object for some reason, so we have to update the hash
					m_cache.set( computationHash, h, 1 );	
					msg( Msg::Warning, "ComputationCache::get", "Inconsistent hash detected." );
				}
			}
		}
	}
	return obj;
}

template< typename T >
void ComputationCache<T>::set( const T &args, const Object *obj, StoreMode storeMode )
{
	MurmurHash computationHash = m_hashFn(args);
	if ( obj )
	{
		m_objectPool->store(obj, storeMode);
		m_cache.set( computationHash, obj->hash(), 1 );
	}
}

template< typename T >
MurmurHash ComputationCache<T>::cacheGetter( const MurmurHash &h, size_t &cost )
{
	cost = 1;
	return MurmurHash();
}

template< typename T >
ObjectPool *ComputationCache<T>::objectPool() const
{
	return m_objectPool.get();
}

} // namespace IECore

#endif // IECORE_COMPUTATIONCACHE_H

