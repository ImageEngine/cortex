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
ConstObjectPtr ComputationCache<T>::retrieve( const T &args )
{
	MurmurHash computationHash = m_hashFn(args);
	MurmurHash h = m_cache.get(computationHash);
	if ( h == MurmurHash() )
	{
		return 0;
	}
	return m_objectPool->retrieve( h );
}

template< typename T >
ConstObjectPtr ComputationCache<T>::get( const T &args )
{
	ConstObjectPtr obj(0);
	MurmurHash computationHash = m_hashFn(args);
	MurmurHash objectHash = m_cache.get(computationHash);

	if ( objectHash == MurmurHash() )
	{
		obj = m_computeFn(args);
		if ( obj )
		{
			m_cache.set( computationHash, obj->hash(), 1 );
			obj = m_objectPool->store( obj );
		}
	}
	else
	{
		obj = m_objectPool->retrieve(objectHash);
		if ( !obj )
		{
			obj = m_computeFn(args);
			if ( obj )
			{
				obj = m_objectPool->store( obj );
			}
		}
	}
	return obj;
}

template< typename T >
void ComputationCache<T>::set( const T &args, ConstObjectPtr obj )
{
	MurmurHash computationHash = m_hashFn(args);
	if ( obj )
	{
		m_objectPool->store(obj);
		m_cache.set( computationHash, obj->hash(), 1 );
	}
}

template< typename T >
MurmurHash ComputationCache<T>::cacheGetter( const MurmurHash &h, size_t &cost )
{
	cost = 1;
	return MurmurHash();
}

} // namespace IECore

#endif // IECORE_COMPUTATIONCACHE_H

