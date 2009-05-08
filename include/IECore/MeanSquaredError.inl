//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_MEANSQUAREDERROR_INL
#define IE_CORE_MEANSQUAREDERROR_INL

#include <cassert>

#include "boost/utility/enable_if.hpp"
#include "boost/type_traits.hpp"

#include "IECore/TypeTraits.h"

namespace IECore
{

template<typename T, typename R>
struct MeanSquaredError< T, R, typename boost::enable_if< boost::is_arithmetic<T> >::type >
{
	typedef R ReturnType;

	R operator()( const T &a, const T &b ) const
	{
		return ( a - b ) * ( a - b );
	}
};

template<typename T, typename R>
struct MeanSquaredError< std::vector<T>, R, typename boost::enable_if< boost::is_arithmetic<T> >::type >
{
	typedef R ReturnType;

	R operator()( const std::vector<T> &a, const std::vector<T> &b ) const
	{
		assert( a.size() == b.size() );

		typename std::vector<T>::size_type n = a.size();

		if ( !n )
		{
			return R(0);
		}

		R e( 0 );
		for ( typename std::vector<T>::size_type i = 0; i < n; i++)
		{
			e += ( a[i] - b[i] ) * ( a[i] - b[i] );
		}

		return e / n;
	}
};

template<typename T, typename R>
struct MeanSquaredError< T, R, typename boost::enable_if< TypeTraits::IsNumericVectorTypedData<T> >::type > : private MeanSquaredError< typename T::ValueType, R >
{
	typedef R ReturnType;

	R operator()( typename T::ConstPtr a, typename T::ConstPtr b ) const
	{
		assert( a );
		assert( b );

		return MeanSquaredError< typename T::ValueType, R >::operator()( a->readable(), b->readable() );
	}
};

template<typename T, typename R>
struct MeanSquaredError< T, R, typename boost::enable_if< TypeTraits::IsNumericSimpleTypedData<T> >::type > : private MeanSquaredError< T, R >
{
	typedef R ReturnType;

	R operator()( typename T::ConstPtr a, typename T::ConstPtr b ) const
	{
		assert( a );
		assert( b );

		return MeanSquaredError< T, R >::operator()( a->readable(), b->readable() );
	}
};

} // namespace IECore

#endif // IE_CORE_MEANSQUAREDERROR_INL
