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

#ifndef IE_CORE_BYTEORDER_H
#define IE_CORE_BYTEORDER_H

#include <boost/static_assert.hpp>

#include "OpenEXR/ImfInt64.h"

namespace IECore
{

#ifdef __APPLE__
	#define IE_CORE_BIG_ENDIAN
#else
	#define IE_CORE_LITTLE_ENDIAN
#endif

/// Returns true if running on a little endian
/// platform.
inline bool littleEndian()
{
#ifdef IE_CORE_LITTLE_ENDIAN
	return true;
#else
	return false;
#endif
}

/// Returns true if running on a big endian
/// platform.
inline bool bigEndian()
{
#ifdef IE_CORE_BIG_ENDIAN
	return true;
#else
	return false;
#endif
}

/// Returns a copy of x with reversed byte order.
template<typename T>
T reverseBytes( const T &x )
{
	// needs specialising for each type
	BOOST_STATIC_ASSERT(sizeof(T)==0);
}

template<>
short reverseBytes<short>( const short &x );

template<>
unsigned short reverseBytes<unsigned short>( const unsigned short &x );

template<>
int reverseBytes<int>( const int &x );

template<>
unsigned int reverseBytes<unsigned int>( const unsigned int &x );

template<>
float reverseBytes<float>( const float &x );

template<>
Imf::Int64 reverseBytes<Imf::Int64>( const Imf::Int64 &x );

template<>
double reverseBytes<double>( const double &x );

/// If running on a big endian platform,
/// returns a copy of x with reversed bytes,
/// otherwise returns x unchanged.
template<typename T>
T asLittleEndian( const T &x )
{
	if( bigEndian() )
	{
		return reverseBytes( x );
	}
	else
	{
		return x;
	}
}

/// If running on a little endian platform,
/// returns a copy of x with reversed bytes,
/// otherwise returns x unchanged.
template<typename T>
T asBigEndian( const T &x )
{
	if( littleEndian() )
	{
		return reverseBytes( x );
	}
	else
	{
		return x;
	}
}

}

#endif // IE_CORE_BYTEORDER_H
