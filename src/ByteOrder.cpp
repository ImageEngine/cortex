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

#include "IECore/ByteOrder.h"

using namespace IECore;

template<>
short reverseBytes<short>( const short &x )
{
	return 	((x & 255) << 8) |
			((x >> 8) & 255 );
}

template<>
unsigned short reverseBytes<unsigned short>( const unsigned short &x )
{
	return 	((x & 255) << 8) |
			((x >> 8) & 255 );
}

template<>
int reverseBytes<int>( const int &x )
{
	return 	((x & 255) << 24) |
			(((x >> 8) & 255 ) << 16 ) |
			(((x >> 16) & 255 ) << 8 ) |
			((x >> 24) & 255 );
}

template<>
unsigned int reverseBytes<unsigned int>( const unsigned int &x )
{
	return 	((x & 255) << 24) |
			(((x >> 8) & 255 ) << 16 ) |
			(((x >> 16) & 255 ) << 8 ) |
			((x >> 24) & 255 );
}

template<>
float reverseBytes<float>( const float &x )
{
	union {
		int i;
		float f;
	} xx;
	xx.f = x;
	xx.i = reverseBytes( xx.i );
	return xx.f;
}

template<>
Imf::Int64 reverseBytes<Imf::Int64>( const Imf::Int64 &x )
{
	return ((x & 255) << 56) |
			(((x >> 8) & 255) << 48) |
			(((x >> 16) & 255 ) << 40 ) |
			(((x >> 24) & 255 ) << 32 ) |
			(((x >> 32) & 255 ) << 24 ) |
			(((x >> 40) & 255 ) << 16 ) |
			(((x >> 48) & 255 ) << 8 ) |
			((x >> 56) & 255 );
}


template<>
double reverseBytes<double>( const double &x )
{
	union {
		Imf::Int64 i;
		double d;
	} xx;
	xx.d = x;
	xx.i = 	reverseBytes<Imf::Int64>(xx.i);
	return xx.d;
}
