//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include <iomanip>
#include <sstream>

#include "IECore/MurmurHash.h"

using namespace IECore;

static inline uint64_t rotl64( uint64_t x, int8_t r )
{
  return (x << r) | (x >> (64 - r));
}

static inline uint64_t fmix( uint64_t k )
{
  k ^= k >> 33;
  k *= 0xff51afd7ed558ccd;
  k ^= k >> 33;
  k *= 0xc4ceb9fe1a85ec53;
  k ^= k >> 33;

  return k;
}

MurmurHash::MurmurHash()
	:	m_h1( 0 ), m_h2( 0 )
{
}

MurmurHash::MurmurHash( const MurmurHash &other )
	:	m_h1( other.m_h1 ), m_h2( other.m_h2 )
{
}

void MurmurHash::append( const void *data, size_t bytes, int elementSize )
{
	const int nBlocks = bytes / 16;
	
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
	for( int i = 0; i < nBlocks; i++ )
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

std::string MurmurHash::toString() const
{
	std::stringstream s;
	s << std::hex << std::setfill( '0' ) << std::setw( 16 ) << m_h1 << std::setw( 16 ) << m_h2;
	return s.str();
}

std::ostream &operator << ( std::ostream &o, const MurmurHash &hash )
{
	o << hash.toString();
	return o;
}

