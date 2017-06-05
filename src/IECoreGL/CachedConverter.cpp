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

#include "boost/lexical_cast.hpp"
#include "boost/format.hpp"
#include "boost/bind.hpp"
#include "boost/bind/placeholders.hpp"

#include "IECore/LRUCache.h"
#include "IECore/MurmurHash.h"

#include "IECoreGL/ToGLConverter.h"
#include "IECoreGL/CachedConverter.h"

using namespace IECoreGL;

namespace
{

// Conceptually the key for the cache is just the hash of
// the object, but we also need the key to carry the object,
// so that the getter can use it for the source of the conversion.
// We therefore pass the object as well as the hash in the key,
// but never access the object outside of the getter() - as there is
// no guarantee that the object is alive outside of the
// call to convert().
struct CacheKey
{

	CacheKey()
		:	object( NULL )
	{
	}

	CacheKey( const IECore::Object *o )
		: object( o ), hash( o->hash() )
	{
	}

	bool operator == ( const CacheKey &other ) const
	{
		return hash == other.hash;
	}

	mutable const IECore::Object *object;
	IECore::MurmurHash hash;

};

inline size_t hash_value( const CacheKey &cacheKey )
{
	return hash_value( cacheKey.hash );
}

} // namespace

struct CachedConverter::MemberData
{
	MemberData( size_t maxMemory )
		:	cache( getter, boost::bind( &MemberData::removalCallback, this, ::_1, ::_2 ), maxMemory )
	{
	}

	static IECore::RunTimeTypedPtr getter( const CacheKey &key, size_t &cost )
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
		// It would be unsafe to access object from outside of this function,
		// so we zero it out so that it will be obvious if anyone ever does.
		// The only way I could see this happening is if the LRUCache implementation
		// changed.
		key.object = 0;
		return converter->convert();
	}

	void removalCallback( const CacheKey &key, const IECore::RunTimeTypedPtr &value )
	{
		deferredRemovals.push_back( value );
	}

	typedef IECore::LRUCache<CacheKey, IECore::RunTimeTypedPtr> Cache;
	Cache cache;
	std::vector<IECore::RunTimeTypedPtr> deferredRemovals;

};

CachedConverter::CachedConverter( size_t maxMemory )
{
	m_data = new MemberData( maxMemory );
}

CachedConverter::~CachedConverter()
{
	delete m_data;
}

IECore::ConstRunTimeTypedPtr CachedConverter::convert( const IECore::Object *object )
{
	return m_data->cache.get( CacheKey( object ) );
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
	m_data->deferredRemovals.clear();
}

CachedConverter *CachedConverter::defaultCachedConverter()
{
	static CachedConverterPtr c = 0;
	if( !c )
	{
		const char *m = getenv( "IECOREGL_CACHEDCONVERTER_MEMORY" );
		int mi = m ? boost::lexical_cast<int>( m ) : 500;
		c = new CachedConverter( 1024 * 1024 * mi );
	}
	return c.get();
}
