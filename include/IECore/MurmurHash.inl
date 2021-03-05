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

#ifndef IECORE_MURMURHASH_INL
#define IECORE_MURMURHASH_INL

#include "IECore/InternedString.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathColor.h"
#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathQuat.h"
#include "OpenEXR/ImathVec.h"
IECORE_POP_DEFAULT_VISIBILITY

#include <vector>

#include <stdint.h>

namespace IECore
{

inline uint64_t rotl64( uint64_t x, int8_t r )
{
	return (x << r) | (x >> (64 - r));
}

inline uint64_t fmix( uint64_t k )
{
	k ^= k >> 33;
	k *= 0xff51afd7ed558ccd;
	k ^= k >> 33;
	k *= 0xc4ceb9fe1a85ec53;
	k ^= k >> 33;

	return k;
}

inline MurmurHash::MurmurHash()
    :   m_h1( 0 ), m_h2( 0 )
{
}

inline MurmurHash::MurmurHash( const MurmurHash &other )
    :   m_h1( other.m_h1 ), m_h2( other.m_h2 )
{
}

inline MurmurHash::MurmurHash( uint64_t h1, uint64_t h2 )
    :   m_h1( h1 ), m_h2( h2 )
{
}

inline void MurmurHash::appendRaw( const void *data, size_t bytes, int elementSize )
{
	const size_t nBlocks = bytes / 16;

	const uint64_t c1 = 0x87c37b91114253d5;
	const uint64_t c2 = 0x4cf5ad432745937f;

	// local copies of m_h1, and m_h2. we'll work
	// with these before copying back at the end.
	// this gives the optimiser more freedom to do
	// its thing.
	uint64_t h1 = m_h1;
	uint64_t h2 = m_h2;

	// body

	const uint64_t *blocks = (const uint64_t *)data;
	for( size_t i = 0; i < nBlocks; i++ )
	{
		uint64_t k1 = blocks[i*2];
		uint64_t k2 = blocks[i*2+1];

		k1 *= c1; k1  = rotl64( k1, 31 ); k1 *= c2; h1 ^= k1;

		h1 = rotl64( h1, 27 ); h1 += h2; h1 = h1*5 + 0x52dce729;

		k2 *= c2; k2  = rotl64( k2, 33 ); k2 *= c1; h2 ^= k2;

		h2 = rotl64( h2, 31); h2 += h1; h2 = h2*5 + 0x38495ab5;
	}

	// tail

	const uint8_t * tail = ((const uint8_t*)data) + nBlocks*16;

	uint64_t k1 = 0;
	uint64_t k2 = 0;

	switch( bytes & 15)
	{
	case 15: k2 ^= uint64_t(tail[14]) << 48;
	case 14: k2 ^= uint64_t(tail[13]) << 40;
	case 13: k2 ^= uint64_t(tail[12]) << 32;
	case 12: k2 ^= uint64_t(tail[11]) << 24;
	case 11: k2 ^= uint64_t(tail[10]) << 16;
	case 10: k2 ^= uint64_t(tail[ 9]) << 8;
	case  9: k2 ^= uint64_t(tail[ 8]) << 0;
		   k2 *= c2; k2  = rotl64(k2,33); k2 *= c1; h2 ^= k2;

	case  8: k1 ^= uint64_t(tail[ 7]) << 56;
	case  7: k1 ^= uint64_t(tail[ 6]) << 48;
	case  6: k1 ^= uint64_t(tail[ 5]) << 40;
	case  5: k1 ^= uint64_t(tail[ 4]) << 32;
	case  4: k1 ^= uint64_t(tail[ 3]) << 24;
	case  3: k1 ^= uint64_t(tail[ 2]) << 16;
	case  2: k1 ^= uint64_t(tail[ 1]) << 8;
	case  1: k1 ^= uint64_t(tail[ 0]) << 0;
		   k1 *= c1; k1  = rotl64(k1,31); k1 *= c2; h1 ^= k1;
	};

	// finalisation

	h1 ^= bytes; h2 ^= bytes;

	h1 += h2;
	h2 += h1;

	h1 = fmix( h1 );
	h2 = fmix( h2 );

	h1 += h2;
	h2 += h1;

	m_h1 = h1;
	m_h2 = h2;
}

template<typename T>
inline MurmurHash &MurmurHash::append( const T &data )
{
	appendInternal( data );
	return *this;
}

template<typename T>
inline MurmurHash &MurmurHash::append( const T *data, size_t numElements )
{
	appendInternal( data, numElements );
	return *this;
}

template<typename T>
inline void MurmurHash::appendInternal( const T &data, typename std::enable_if<std::is_arithmetic<T>::value>::type *enabler )
{
	appendRaw( &data, sizeof( T ), sizeof( T ) );
}

template<typename T>
inline void MurmurHash::appendInternal( const T *data, size_t numElements, typename std::enable_if<std::is_arithmetic<T>::value>::type *enabler )
{
	appendRaw( data, numElements * sizeof( T ), sizeof( T ) );
}

template<typename T>
inline void MurmurHash::appendInternal( const T &data, typename std::enable_if<!std::is_arithmetic<T>::value>::type *enabler )
{
	murmurHashAppend( *this, data );
}

template<typename T>
inline void MurmurHash::appendInternal( const T *data, size_t numElements, typename std::enable_if<!std::is_arithmetic<T>::value>::type *enabler )
{
	murmurHashAppend( *this, data, numElements );
}

inline const MurmurHash &MurmurHash::operator = ( const MurmurHash &other )
{
	m_h1 = other.m_h1;
	m_h2 = other.m_h2;
	return *this;
}

inline bool MurmurHash::operator == ( const MurmurHash &other ) const
{
	return m_h1 == other.m_h1 && m_h2 == other.m_h2;
}

inline bool MurmurHash::operator != ( const MurmurHash &other ) const
{
	return m_h1 != other.m_h1 || m_h2 != other.m_h2;
}

inline bool MurmurHash::operator < ( const MurmurHash &other ) const
{
	return m_h1 < other.m_h1 || ( m_h1 == other.m_h1 && m_h2 < other.m_h2 );
}

inline uint64_t MurmurHash::h1() const
{
	return m_h1;
}

inline uint64_t MurmurHash::h2() const
{
	return m_h2;
}

/// Implementation of tbb_hasher for MurmurHash, allowing MurmurHash to be used
/// as a key in tbb::concurrent_hash_map.
inline size_t tbb_hasher( const MurmurHash &h )
{
	return h.m_h1 ^ h.m_h2;
}

/// Implementation of hash_value for MurmurHash, allowing it to be used with boost::hash,
/// and therefore as a key in boost::unordered_map.
inline size_t hash_value( const MurmurHash &h )
{
	return h.m_h1 ^ h.m_h2;
}

/// Implementations of `murmurHashAppend()` for non-arithmetic types that are commonly
/// used in Cortex.

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const T &data, typename std::enable_if<std::is_enum<T>::value>::type *enabler = nullptr )
{
	h.append( static_cast<typename std::underlying_type<T>::type>( data ) );
}

inline void murmurHashAppend( MurmurHash &h, const half &data )
{
	static_assert( sizeof( half ) == sizeof( unsigned short ), "Unexpected size for half" );
	h.append( reinterpret_cast<const unsigned short *>( &data ), 1 );
}

inline void murmurHashAppend( MurmurHash &h, const char *data )
{
	h.append( data, strlen( data ) );
}

inline void murmurHashAppend( MurmurHash &h, const std::string &data )
{
	h.append( data.c_str(), data.size() + 1 );
}

inline void murmurHashAppend( IECore::MurmurHash &h, const InternedString &data )
{
	h.append( data.value() );
}

inline void murmurHashAppend( MurmurHash &h, const IECore::MurmurHash &data )
{
	static_assert( sizeof( MurmurHash ) == sizeof( uint64_t ) * 2, "Unexpected size for MurmurHash" );
	h.append( reinterpret_cast<const uint64_t *>( &data ), 2 );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Vec2<T> &data )
{
	h.append( data.getValue(), 2 );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Vec3<T> &data )
{
	h.append( data.getValue(), 3 );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Color3<T> &data )
{
	h.append( data.getValue(), 3 );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Color4<T> &data )
{
	h.append( data.getValue(), 4 );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Quat<T> &data )
{
	h.append( &data.r, 4 );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Matrix33<T> &data )
{
	h.append( data.getValue(), 9 );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Matrix44<T> &data )
{
	h.append( data.getValue(), 16 );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Box<T> &data )
{
	h.append( &data.min, 2 );
}

template<typename T>
inline void murmurHashAppend( IECore::MurmurHash &h, const std::vector<T> &data )
{
	h.append( data.data(), data.size() );
}

/// Implementations of `murmurHashAppend()` for arrays of non-arithmetic types that are commonly
/// used in Cortex.

inline void murmurHashAppend( MurmurHash &h, const half *data, size_t numElements )
{
	static_assert( sizeof( half ) == sizeof( unsigned short ), "Unexpected size for half" );
	h.append( reinterpret_cast<const unsigned short *>( data ), numElements );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Vec2<T> *data, size_t numElements )
{
	h.append( data->getValue(), 2 * numElements );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Vec3<T> *data, size_t numElements )
{
	h.append( data->getValue(), 3 * numElements );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Color3<T> *data, size_t numElements )
{
	h.append( data->getValue(), 3 * numElements );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Color4<T> *data, size_t numElements )
{
	h.append( data->getValue(), 4 * numElements );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Matrix33<T> *data, size_t numElements )
{
	h.append( data->getValue(), 9 * numElements );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Matrix44<T> *data, size_t numElements )
{
	h.append( data->getValue(), 16 * numElements );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Box<T> *data, size_t numElements )
{
	h.append( &data->min, 2 * numElements );
}

template<typename T>
inline void murmurHashAppend( MurmurHash &h, const Imath::Quat<T> *data, size_t numElements )
{
	h.append( &data->r, 4 * numElements );
}

// If no specific case for arrays matches, then hash each element of the array separately.
template<typename T>
inline void murmurHashAppend( MurmurHash &h, const T *data, size_t numElements )
{
	for( size_t i=0; i<numElements; i++ )
	{
		h.append( *data );
		data++;
	}
}

} // namespace IECore

#endif // IECORE_MURMURHASH_INL
