//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_HASHTABLE_H
#define IE_CORE_HASHTABLE_H

#include <iostream>
#include <cassert>
#include <string>

#include "boost/static_assert.hpp"
#include "boost/multi_index_container.hpp"
#include "boost/multi_index/hashed_index.hpp"
#include "boost/multi_index/member.hpp"

namespace IECore
{


template<typename T>
struct Hash
{
	size_t operator()( const T &s ) const
	{
		return static_cast<size_t>(s);
	}
};

template<typename T>
struct Hash<T*>
{
	size_t operator()( const T *s ) const
	{
		return reinterpret_cast<size_t>(s);
	}
};

template<>
struct Hash<const char *>
{
	/// Dan Bernstein's original string hash
	size_t operator()( const char *s ) const
	{
		assert(s);
		size_t hash = 5381;
		while (*s)
		{
			hash = ( ( hash << 5 ) + hash ) + *s++;
		}

		return hash;
	}
};

template<>
struct Hash<std::string>
{
	/// Dan Bernstein's original string hash
	size_t operator()( const std::string &s ) const
	{
		size_t hash = 5381;
		for ( std::string::size_type i = 0; i < s.length(); i++)
		{
			hash = ( ( hash << 5 ) + hash ) + s[i];
		}

		return hash;
	}
};


template<typename Key, typename Data, typename HashFn = Hash<Key> >
struct HashTable : public boost::multi_index::multi_index_container
		<
			std::pair< Key, Data >,
			boost::multi_index::indexed_by<
				boost::multi_index::hashed_non_unique<
					boost::multi_index::member<std::pair< Key, Data >, Key, &std::pair< Key, Data >::first>,
					HashFn
				>
			>
		>
{
	typedef Key key_type;
	typedef Data data_type;
	typedef std::pair< Key, Data > value_type;
};

} // namespace IECore

#endif // IE_CORE_HASHTABLE_H
