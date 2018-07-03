//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/CachedConverter.h"

#include "IECoreGL/ToGLConverter.h"

#include "IECore/LRUCache.h"
#include "IECore/MurmurHash.h"

#include "boost/bind.hpp"
#include "boost/bind/placeholders.hpp"
#include "boost/format.hpp"
#include "boost/lexical_cast.hpp"

#include "tbb/concurrent_queue.h"

using namespace IECoreGL;

namespace
{

// Conceptually the key for the cache is just the hash of
// the object, but the getter also needs the object to be
// converted, so we take advantage of the LRUCache's GetterKey
// feature which allows an augmented key to be passed to the
// getter, while a simpler key is stored internally.
struct CacheGetterKey
{

	CacheGetterKey()
		:	object( nullptr )
	{
	}

	CacheGetterKey( const IECore::Object *o )
		: object( o ), hash( o->hash() )
	{
	}

	bool operator == ( const CacheGetterKey &other ) const
	{
		return hash == other.hash;
	}

	operator const IECore::MurmurHash & () const
	{
		return hash;
	}

	const IECore::Object *object;
	const IECore::MurmurHash hash;

};

} // namespace

struct CachedConverter::MemberData
{
	MemberData( size_t maxMemory )
		:	cache( getter, boost::bind( &MemberData::removalCallback, this, ::_1, ::_2 ), maxMemory )
	{
	}

	static IECore::RunTimeTypedPtr getter( const CacheGetterKey &key, size_t &cost )
	{
		cost = key.object->memoryUsage();
		ToGLConverterPtr converter = ToGLConverter::create( key.object );
		if( !converter )
		{
			throw IECore::Exception(
				boost::str(
					boost::format(
						"Unable to create converter for Object of type \"%s\""
					) % key.object->typeName()
				)
			);
		}
		return converter->convert();
	}

	void removalCallback( const IECore::MurmurHash &key, const IECore::RunTimeTypedPtr &value )
	{
		deferredRemovals.push( value );
	}

	typedef IECore::LRUCache<IECore::MurmurHash, IECore::RunTimeTypedPtr, IECore::LRUCachePolicy::Parallel, CacheGetterKey> Cache;
	Cache cache;
	tbb::concurrent_queue<IECore::RunTimeTypedPtr> deferredRemovals;

};

CachedConverter::CachedConverter( size_t maxMemory )
	:	m_data( new MemberData( maxMemory ) )
{
}

CachedConverter::~CachedConverter()
{
}

IECore::ConstRunTimeTypedPtr CachedConverter::convert( const IECore::Object *object )
{
	return m_data->cache.get( CacheGetterKey( object ) );
}

size_t CachedConverter::getMaxMemory() const
{
	return m_data->cache.getMaxCost();
}

void CachedConverter::setMaxMemory( size_t maxMemory )
{
	m_data->cache.setMaxCost( maxMemory );
	clearUnused();
}

void CachedConverter::clearUnused()
{
	IECore::RunTimeTypedPtr p;
	while( m_data->deferredRemovals.try_pop( p ) )
	{
		// No need to do anything, we're just removing
		// pointers from the queue.
	}
}

CachedConverter *CachedConverter::defaultCachedConverter()
{
	static CachedConverterPtr c = nullptr;
	if( !c )
	{
		const char *m = getenv( "IECOREGL_CACHEDCONVERTER_MEMORY" );
		int mi = m ? boost::lexical_cast<int>( m ) : 500;
		c = new CachedConverter( 1024 * 1024 * mi );
	}
	return c.get();
}
