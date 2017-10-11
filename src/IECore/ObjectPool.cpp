//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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

#include "boost/lexical_cast.hpp"
#include "IECore/LRUCache.h"
#include "IECore/ObjectPool.h"

using namespace IECore;

////////////////////////////////////////////////////////////////////////
// MemberData
////////////////////////////////////////////////////////////////////////

struct ObjectPool::MemberData
{

	MemberData( size_t maxMemory ) : cache( getter, maxMemory )
	{
	}

	LRUCache< MurmurHash, ConstObjectPtr > cache;

	/// our getter always returns NULL
	static ConstObjectPtr getter( const MurmurHash &h, size_t &cost )
	{
		cost = 0;
		return nullptr;
	}
};

//////////////////////////////////////////////////////////////////////////
// ObjectPool
//////////////////////////////////////////////////////////////////////////

ObjectPool::ObjectPool( size_t maxMemory )
	:	m_data( new MemberData(maxMemory) )
{
}

ObjectPool::~ObjectPool()
{
}

ConstObjectPtr ObjectPool::retrieve( const MurmurHash &hash ) const
{
	return m_data->cache.get(hash);
}

ConstObjectPtr ObjectPool::store( const Object *obj, StoreMode mode )
{
	MurmurHash h = obj->hash();

	// first tries to see if the object is already in the cache and return that one quickly.
	ConstObjectPtr cachedObj = m_data->cache.get(h);
	if ( cachedObj )
	{
		return cachedObj;
	}

	if ( mode == StoreCopy )
	{
		cachedObj = obj->copy();
		m_data->cache.set( h, cachedObj, obj->memoryUsage() );
		return cachedObj;
	}
	else if ( mode == StoreReference )
	{
		m_data->cache.set( h, obj, obj->memoryUsage() );
		return obj;
	}
	else
	{
		throw Exception( "Invalid store mode!" );
	}
}

bool ObjectPool::contains( const MurmurHash &hash ) const
{
	return m_data->cache.cached(hash);
}

void ObjectPool::clear()
{
	m_data->cache.clear();
}

bool ObjectPool::erase( const MurmurHash &hash )
{
	return m_data->cache.erase(hash);
}

void ObjectPool::setMaxMemoryUsage( size_t maxMemory )
{
	m_data->cache.setMaxCost(maxMemory);
}

size_t ObjectPool::getMaxMemoryUsage() const
{
	return m_data->cache.getMaxCost();
}

size_t ObjectPool::memoryUsage() const
{
	return m_data->cache.currentCost();
}

ObjectPool *ObjectPool::defaultObjectPool()
{
	static ObjectPoolPtr c = nullptr;
	if( !c )
	{
		const char *m = getenv( "IECORE_OBJECTPOOL_MEMORY" );
		size_t mi = m ? boost::lexical_cast<size_t>( m ) : 500;
		c = new ObjectPool(1024 * 1024 * mi);
	}
	return c.get();
}

/// make sure the default pool is created at load time and avoid
/// running conditions on multi-threaded environments.
static ObjectPoolPtr initializer = ObjectPool::defaultObjectPool();

