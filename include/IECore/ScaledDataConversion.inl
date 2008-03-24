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
//	     other contributors to this software may be used to endorse or
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

#include <algorithm>

#include "boost/utility/enable_if.hpp"
#include "boost/mpl/and.hpp"
#include "boost/mpl/or.hpp"
#include "boost/type_traits/is_integral.hpp"
#include "boost/type_traits/is_signed.hpp"
#include "boost/type_traits/is_unsigned.hpp"
#include "boost/type_traits/is_floating_point.hpp"

#include "OpenEXR/half.h"
#include "OpenEXR/ImathLimits.h"

#include "boost/type_traits/detail/bool_trait_def.hpp"
namespace boost
{
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_floating_point,half,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_signed,half,true)
BOOST_TT_AUX_BOOL_TRAIT_CV_SPEC1(is_unsigned,half,false)
}
#include "boost/type_traits/detail/bool_trait_undef.hpp"

namespace IECore
{

template<typename F, typename T>
struct ScaledDataConversion< F, T, typename boost::enable_if< boost::mpl::and_< boost::mpl::and_< boost::is_integral<F>, boost::is_integral<T> >, boost::is_signed<T> > >::type >
{
	typedef F FromType;
	typedef T ToType;	
	
	T operator()( F f )
	{
		BOOST_STATIC_ASSERT( boost::is_signed< T >::value );	
		float result = static_cast<float>(f) / Imath::limits<F>::max() * Imath::limits<T>::max();
		return static_cast<T>( result + 0.5f );
	}
};

template<typename F, typename T>
struct ScaledDataConversion< F, T, typename boost::enable_if< boost::mpl::and_< boost::mpl::and_< boost::is_integral<F>, boost::is_integral<T> >, boost::is_unsigned<T> > >::type >
{
	typedef F FromType;
	typedef T ToType;

	T operator()( F f )
	{
		BOOST_STATIC_ASSERT( boost::is_unsigned< T >::value );
		f = std::max<F>( f, (F)(Imath::limits<T>::min() ) );
		float result = static_cast<float>(f) / Imath::limits<F>::max() * Imath::limits<T>::max();
		return static_cast<T>( result + 0.5f );
	}
};

template<typename F, typename T>
struct ScaledDataConversion< F, T, typename boost::enable_if< boost::mpl::and_< boost::mpl::and_< boost::is_floating_point<F>, boost::is_integral<T> >, boost::is_signed<T> > >::type >
{
	typedef F FromType;
	typedef T ToType;

	T operator()( F f )
	{
		BOOST_STATIC_ASSERT( boost::is_signed< T >::value );
		f = std::max<F>( f, (F)( -1.0 ) );		
		f = std::min<F>( f, (F)( 1.0 ) );				
		float result = static_cast<float>(f) * Imath::limits<T>::max();
		return static_cast<T>( result + 0.5f );
	}
};

template<typename F, typename T>
struct ScaledDataConversion< F, T, typename boost::enable_if< boost::mpl::and_< boost::mpl::and_< boost::is_floating_point<F>, boost::is_integral<T> >, boost::is_unsigned<T> > >::type >
{
	typedef F FromType;
	typedef T ToType;

	T operator()( F f )
	{		
		BOOST_STATIC_ASSERT( boost::is_unsigned< T >::value );
		f = std::max<F>( f, (F)(Imath::limits<T>::min() ) );
		f = std::min<F>( f, (F)( 1.0 ) );	
		float result = static_cast<float>(f) * Imath::limits<T>::max();
		return static_cast<T>( result + 0.5f );
	}
};

template<typename F, typename T>
struct ScaledDataConversion< F, T, typename boost::enable_if< boost::mpl::and_< boost::is_integral<F>, boost::is_floating_point<T> > >::type >
{
	typedef F FromType;
	typedef T ToType;

	T operator()( F f )
	{
		float result = static_cast<float>(f) / Imath::limits<F>::max();
		return static_cast<T>( result );
	}
};

template<typename F, typename T>
struct ScaledDataConversion< F, T, typename boost::enable_if< boost::mpl::and_< boost::is_floating_point<F>, boost::is_floating_point<T> > >::type >
{
	typedef F FromType;
	typedef T ToType;

	T operator()( F f )
	{
		return static_cast<T>( f );
	}
};

}
