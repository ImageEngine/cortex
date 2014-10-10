//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2014, Image Engine Design Inc. All rights reserved.
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

#include <string.h>

#include "tbb/spin_rw_mutex.h"
#include "tbb/concurrent_hash_map.h"

#include "boost/multi_index_container.hpp"
#include "boost/multi_index/hashed_index.hpp"
#include "boost/lexical_cast.hpp"

#include "IECore/InternedString.h"

namespace IECore
{

namespace Detail
{

// Simple type for specifying a range of characters, to
// represent non-null-terminated strings.
typedef std::pair<const char *, const char *> CharRange;

// Hash for strings of various types.
// By overloading it for multiple types, we are able to do
// lookups into HashSet using any type as a key, and without
// needing to construct a temporary std::string when the type
// is a raw c string.
struct Hash
{

	// Dan Bernstein's original string hash
	size_t operator()( const char *s ) const
	{
		size_t hash = 5381;
		while( *s )
		{
			hash = ( ( hash << 5 ) + hash ) + *s++;
		}

		return hash;
	}

	size_t operator()( const std::string &s ) const
	{
		return (*this)( s.c_str() );
	}

	size_t operator()( const CharRange &range ) const
	{
		size_t hash = 5381;
		for( const char *s = range.first; s != range.second; ++s )
		{
			hash = ( ( hash << 5 ) + hash ) + *s;
		}

		return hash;
	}

};

// Equality operator between strings of various types.
// As above, this allows HashSet lookups to be performed
// using any type, without the overhead of constructing
// temporary std::strings.
struct Equal
{

	bool operator()( const std::string &s1, const std::string &s2 ) const
	{
		return s1 == s2;
	}

	bool operator()( const char *c, const std::string &s ) const
	{
		return strcmp( c, s.c_str() )==0;
	}

	bool operator()( const std::string &s, const char *c ) const
	{
		return strcmp( c, s.c_str() )==0;
	}

	bool operator()( const CharRange &c, const std::string &s ) const
	{
		return s.compare( 0, std::string::npos, c.first, c.second - c.first )==0;
	}

	bool operator()( const std::string &s, const CharRange &c ) const
	{
		return s.compare( 0, std::string::npos, c.first, c.second - c.first )==0;
	}

};

typedef boost::multi_index::multi_index_container<
	std::string,
	boost::multi_index::indexed_by<
		boost::multi_index::hashed_unique<
			boost::multi_index::identity<std::string>,
			Hash,
			Equal
		>
	>
> HashSet;

typedef HashSet::nth_index<0>::type Index;
typedef HashSet::nth_index_const_iterator<0>::type ConstIterator;
typedef tbb::spin_rw_mutex Mutex;

static HashSet *hashSet()
{
	static HashSet g_hashSet;
	return &g_hashSet;
}

static Mutex *mutex()
{
	static Detail::Mutex g_mutex;
	return &g_mutex;
}

} // namespace Detail

const std::string *InternedString::internedString( const char *value )
{
	Detail::HashSet *hashSet = Detail::hashSet();
	Detail::Index &hashIndex = hashSet->get<0>();
	Detail::Mutex::scoped_lock lock( *Detail::mutex(), false ); // read-only lock
	Detail::HashSet::const_iterator it = hashIndex.find( value );
	if( it!=hashIndex.end() )
	{
		return &(*it);
	}
	else
	{
		lock.upgrade_to_writer();
		return &(*(hashSet->insert( std::string( value ) ).first ) );
	}
}

const std::string *InternedString::internedString( const char *value, size_t length )
{
	Detail::HashSet *hashSet = Detail::hashSet();
	Detail::Index &hashIndex = hashSet->get<0>();
	Detail::Mutex::scoped_lock lock( *Detail::mutex(), false ); // read-only lock
	Detail::HashSet::const_iterator it = hashIndex.find( Detail::CharRange( value, value + length ) );
	if( it!=hashIndex.end() )
	{
		return &(*it);
	}
	else
	{
		lock.upgrade_to_writer();
		return &(*(hashSet->insert( std::string( value, length ) ).first ) );
	}
}

size_t InternedString::numUniqueStrings()
{
	Detail::Mutex::scoped_lock lock( *Detail::mutex(), false ); // read-only lock
	Detail::HashSet *hashSet = Detail::hashSet();
	return hashSet->size();
}

static InternedString g_emptyString("");

const InternedString &InternedString::emptyString()
{
	return g_emptyString;
}

// make sure we create the g_numbers map at load time.
static InternedString g_zero((int64_t)0);

const InternedString &InternedString::numberString( int64_t number )
{
	typedef tbb::concurrent_hash_map< int64_t, InternedString > NumbersMap;
	static NumbersMap *g_numbers = 0;
	if ( !g_numbers )
	{
		g_numbers = new NumbersMap;
	}
	NumbersMap::accessor it;
	if ( g_numbers->insert( it, number ) )
	{
		it->second = InternedString( boost::lexical_cast<std::string>( number ) );
	}
	return it->second;
}

std::ostream &operator << ( std::ostream &o, const InternedString &str )
{
	o << str.c_str();
	return o;
}

} // namespace IECore
