//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_MURMURHASH_H
#define IECORE_MURMURHASH_H

#include "IECore/Export.h"

#include <iostream>
#include <string>
#include <type_traits>

namespace IECore
{

/// A nice class for hashing arbitrary chunks of data, based on
/// code available at http://code.google.com/p/smhasher.
///
/// From that page :
///
/// "All MurmurHash versions are public domain software, and the
/// author disclaims all copyright to their code."
///
/// \todo Deal with endian-ness.
class IECORE_API MurmurHash
{

	public :

		inline MurmurHash();
		inline MurmurHash( const MurmurHash &other );

		// Construct directly from known internal values
		inline MurmurHash( uint64_t h1, uint64_t h2 );

		/// Appends a single value. Arithmetic types are supported natively,
		/// and custom types can be added by providing an implementation of
		/// `void murmurHashAppend( MurmurHash &h, const T &data )`.
		template<typename T>
		inline MurmurHash &append( const T &data );

		/// Appends an array of values. Arithmetic types are supported natively,
		/// and custom types can be added by providing an implementation of
		/// `void murmurHashAppend( MurmurHash &h, const T *data, size_t numElements )`
		template<typename T>
		inline MurmurHash &append( const T *data, size_t numElements );

		inline const MurmurHash &operator = ( const MurmurHash &other );

		inline bool operator == ( const MurmurHash &other ) const;
		inline bool operator != ( const MurmurHash &other ) const;

		inline bool operator < ( const MurmurHash &other ) const;

		std::string toString() const;

		// Access internal storage for special cases
		inline uint64_t h1() const;
		inline uint64_t h2() const;

	private :

		// Helper functions to do the dispatch needed by the public `append()` methods.
		// Starting with variants for known arithmetic types - these are dispatched to `appendRaw()`.
		template<typename T>
		inline void appendInternal( const T &data, typename std::enable_if<std::is_arithmetic<T>::value>::type *enabler = nullptr );
		template<typename T>
		inline void appendInternal( const T *data, size_t numElements, typename std::enable_if<std::is_arithmetic<T>::value>::type *enabler = nullptr );
		// Then variants for unknown types - these are dispatched to `murmurHashAppend()`.
		template<typename T>
		inline void appendInternal( const T &data, typename std::enable_if<!std::is_arithmetic<T>::value>::type *enabler = nullptr );
		template<typename T>
		inline void appendInternal( const T *data, size_t numElements, typename std::enable_if<!std::is_arithmetic<T>::value>::type *enabler = nullptr );

		// Does the actual work of appending to the hash. `elementSize` is required
		// so that we could support endian-independence in future.
		inline void appendRaw( const void *data, size_t bytes, int elementSize );

		uint64_t m_h1;
		uint64_t m_h2;

		friend size_t tbb_hasher( const MurmurHash &h );
		friend size_t hash_value( const MurmurHash &h );

};

IECORE_API std::ostream &operator << ( std::ostream &o, const MurmurHash &hash );

} // namespace IECore

#include "IECore/MurmurHash.inl"

#endif // IECORE_MURMURHASH_H
