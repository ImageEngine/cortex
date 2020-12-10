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

inline void MurmurHash::append( const void *data, size_t bytes, int elementSize )
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

inline MurmurHash &MurmurHash::append( char data )
{
	append( &data, sizeof( char ), sizeof( char ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( unsigned char data )
{
	append( &data, sizeof( unsigned char ), sizeof( unsigned char ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( short data )
{
	append( &data, sizeof( short ), sizeof( short ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( unsigned short data )
{
	append( &data, sizeof( unsigned short ), sizeof( unsigned short ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( int data )
{
	append( &data, sizeof( int ), sizeof( int ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( unsigned int data )
{
	append( &data, sizeof( unsigned int ), sizeof( unsigned int ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( int64_t data )
{
	append( &data, sizeof( int64_t ), sizeof( int64_t ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( uint64_t data )
{
	append( &data, sizeof( uint64_t ), sizeof( uint64_t ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( half data )
{
	append( &data, sizeof( half ), sizeof( half ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( float data )
{
	append( &data, sizeof( float ), sizeof( float ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( double data )
{
	append( &data, sizeof( double ), sizeof( double ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const char *data )
{
	append( data, strlen( data ), sizeof( char ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const std::string &data )
{
	append( data.c_str(), data.size() + 1, sizeof( char ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const InternedString &data )
{
	append( data.value() );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::V2i &data )
{
	append( data.getValue(), 2 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::V2f &data )
{
	append( data.getValue(), 2 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::V2d &data )
{
	append( data.getValue(), 2 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::V3i &data )
{
	append( data.getValue(), 3 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::V3f &data )
{
	append( data.getValue(), 3 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::V3d &data )
{
	append( data.getValue(), 3 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Color3f &data )
{
	append( data.getValue(), 3 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Color3<double> &data )
{
	append( data.getValue(), 3 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Color4f &data )
{
	append( data.getValue(), 4 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Color4<double> &data )
{
	append( data.getValue(), 4 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::M33f &data )
{
	append( data.getValue(), 9 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::M33d &data )
{
	append( data.getValue(), 9 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::M44f &data )
{
	append( data.getValue(), 16 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::M44d &data )
{
	append( data.getValue(), 16 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Box2i &data )
{
	append( data.min.getValue(), 4 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Box2f &data )
{
	append( data.min.getValue(), 4 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Box2d &data )
{
	append( data.min.getValue(), 4 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Box3i &data )
{
	append( data.min.getValue(), 6 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Box3f &data )
{
	append( data.min.getValue(), 6 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Box3d &data )
{
	append( data.min.getValue(), 6 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Quatf &data )
{
	append( &data.r, 4 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Quatd &data )
{
	append( &data.r, 4 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const MurmurHash &data )
{
	append( &data.m_h1, 2 );
	return *this;
}

inline MurmurHash &MurmurHash::append( const char *data, size_t numElements )
{
	append( data, numElements * sizeof( char ), sizeof( char ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const unsigned char *data, size_t numElements )
{
	append( data, numElements * sizeof( unsigned char ), sizeof( unsigned char ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const short *data, size_t numElements )
{
	append( data, numElements * sizeof( short ), sizeof( short ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const unsigned short *data, size_t numElements )
{
	append( data, numElements * sizeof( unsigned short ), sizeof( unsigned short ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const int *data, size_t numElements )
{
	append( data, numElements * sizeof( int ), sizeof( int ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const unsigned int *data, size_t numElements )
{
	append( data, numElements * sizeof( unsigned int ), sizeof( unsigned int ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const int64_t *data, size_t numElements )
{
	append( data, numElements * sizeof( int64_t ), sizeof( int64_t ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const uint64_t *data, size_t numElements )
{
	append( data, numElements * sizeof( uint64_t ), sizeof( uint64_t ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const half *data, size_t numElements )
{
	append( data, numElements * sizeof( half ), sizeof( half ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const float *data, size_t numElements )
{
	append( data, numElements * sizeof( float ), sizeof( float ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const double *data, size_t numElements )
{
	append( data, numElements * sizeof( double ), sizeof( double ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const std::string *data, size_t numElements )
{
	for( size_t i=0; i<numElements; i++ )
	{
		append( *data++ );
	}
	return *this;
}

inline MurmurHash &MurmurHash::append( const InternedString *data, size_t numElements )
{
	for( size_t i=0; i<numElements; i++ )
	{
		append( data->value() );
		data++;
	}
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::V2i *data, size_t numElements )
{
	append( data, numElements * 2 * sizeof( int ), sizeof( int ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::V2f *data, size_t numElements )
{
	append( data, numElements * 2 * sizeof( float ), sizeof( float ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::V2d *data, size_t numElements )
{
	append( data, numElements * 2 * sizeof( double ), sizeof( double ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::V3i *data, size_t numElements )
{
	append( data, numElements * 3 * sizeof( int ), sizeof( int ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::V3f *data, size_t numElements )
{
	append( data, numElements * 3 * sizeof( float ), sizeof( float ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::V3d *data, size_t numElements )
{
	append( data, numElements * 3 * sizeof( double ), sizeof( double ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Color3f *data, size_t numElements )
{
	append( data, numElements * 3 * sizeof( float ), sizeof( float ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Color3<double> *data, size_t numElements )
{
	append( data, numElements * 3 * sizeof( double ), sizeof( double ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Color4f *data, size_t numElements )
{
	append( data, numElements * 4 * sizeof( float ), sizeof( float ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Color4<double> *data, size_t numElements )
{
	append( data, numElements * 4 * sizeof( double ), sizeof( double ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::M33f *data, size_t numElements )
{
	append( data, numElements * 9 * sizeof( float ), sizeof( float ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::M33d *data, size_t numElements )
{
	append( data, numElements * 9 * sizeof( double ), sizeof( double ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::M44f *data, size_t numElements )
{
	append( data, numElements * 16 * sizeof( float ), sizeof( float ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::M44d *data, size_t numElements )
{
	append( data, numElements * 16 * sizeof( double ), sizeof( double ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Box2i *data, size_t numElements )
{
	append( data, numElements * 4 * sizeof( int ), sizeof( int ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Box2f *data, size_t numElements )
{
	append( data, numElements * 4 * sizeof( float ), sizeof( float ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Box2d *data, size_t numElements )
{
	append( data, numElements * 4 * sizeof( double ), sizeof( double ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Box3i *data, size_t numElements )
{
	append( data, numElements * 6 * sizeof( int ), sizeof( int ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Box3f *data, size_t numElements )
{
	append( data, numElements * 6 * sizeof( float ), sizeof( float ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Box3d *data, size_t numElements )
{
	append( data, numElements * 6 * sizeof( double ), sizeof( double ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Quatf *data, size_t numElements )
{
	append( data, numElements * 4 * sizeof( float ), sizeof( float ) );
	return *this;
}

inline MurmurHash &MurmurHash::append( const Imath::Quatd *data, size_t numElements )
{
	append( data, numElements * 4 * sizeof( double ), sizeof( double ) );
	return *this;
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

} // namespace IECore

#endif // IECORE_MURMURHASH_INL
