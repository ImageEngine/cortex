//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

//! \file HexConversion.h
/// Defines template conversions between hex and decimal

#ifndef IE_CORE_HEXCONVERSION_H
#define IE_CORE_HEXCONVERSION_H

#include <iostream>
#include <cassert>
#include <string>

#include "boost/static_assert.hpp"
#include "boost/type_traits/is_integral.hpp"
#include "boost/type_traits/is_same.hpp"

namespace IECore
{

template<typename T, typename OutputIterator>
inline void decToHex( T value, OutputIterator result )
{
	BOOST_STATIC_ASSERT( boost::is_integral<T>::value );
	typedef typename std::iterator_traits<OutputIterator>::value_type CharType;
	BOOST_STATIC_ASSERT( ( boost::is_same<CharType, char>::value ) );

	for( int i = sizeof( T ) * 2 - 1; i >= 0 ; --i )
	{
		int m = ( value >> (i * 4) ) & 0xF;
		*result = "0123456789ABCDEF"[m];
		++result;
	}
}

template<typename InputIterator, typename OutputIterator>
inline void decToHex( InputIterator first, InputIterator last, OutputIterator result )
{
	for( ; first!=last; ++first )
	{
		decToHex( *first, result );
		result += 2;
	}
}

template<typename RandomAccessIterator>
inline std::string decToHex( RandomAccessIterator first, RandomAccessIterator last )
{
	typedef typename std::iterator_traits<RandomAccessIterator>::value_type ValueType;
	std::string result; result.resize( sizeof( ValueType ) * 2 * ( last - first ) );
	decToHex( first, last, result.begin() );
	return result;
}

template<typename T>
inline std::string decToHex( T n )
{
	BOOST_STATIC_ASSERT( boost::is_integral<T>::value );

	std::string r; r.resize( sizeof( T ) * 2 );
	decToHex( n, r.begin() );

	return r;
}

template<typename T, typename InputIterator>
inline T hexToDec( InputIterator first, InputIterator last )
{
	BOOST_STATIC_ASSERT( boost::is_integral<T>::value );
	typedef typename std::iterator_traits<InputIterator>::value_type CharType;
	BOOST_STATIC_ASSERT( ( boost::is_same<CharType, char>::value ) );

	T n = 0;

	for( ; first != last; ++first )
	{
		n <<= 4;

		switch ( *first )
		{
		case '0' :
			n |= 0;
			break;
		case '1' :
			n |= 1;
			break;
		case '2' :
			n |= 2;
			break;
		case '3' :
			n |= 3;
			break;
		case '4' :
			n |= 4;
			break;
		case '5' :
			n |= 5;
			break;
		case '6' :
			n |= 6;
			break;
		case '7' :
			n |= 7;
			break;
		case '8' :
			n |= 8;
			break;
		case '9' :
			n |= 9;
			break;
		case 'A' :
		case 'a' :
			n |= 10;
			break;
		case 'B' :
		case 'b' :
			n |= 11;
			break;
		case 'C' :
		case 'c' :
			n |= 12;
			break;
		case 'D' :
		case 'd' :
			n |= 13;
			break;
		case 'E' :
		case 'e' :
			n |= 14;
			break;
		case 'F' :
		case 'f' :
			n |= 15;
			break;
		default:
			assert( false );
		}
	}

	return n;
}

template<typename T>
inline T hexToDec( const std::string &s )
{
	assert( s.size() <= sizeof( T ) * 2 );
	return hexToDec<T>( s.begin(), s.end() );
}

template<typename T, typename InputIterator, typename OutputIterator>
inline void hexToDec( InputIterator first, InputIterator last, OutputIterator result )
{
	typedef typename std::iterator_traits<InputIterator>::value_type CharType;
	BOOST_STATIC_ASSERT( ( boost::is_same<CharType, char>::value ) );

	for( ; first != last; first += sizeof( T ) * 2 )
	{
		*result = hexToDec<T>( first, first + sizeof( T ) * 2 );
		++result;
	}
}

} // namespace IECore

#endif // IE_CORE_HEXCONVERSION_H
