//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "boost/utility/enable_if.hpp"
#include "boost/mpl/and.hpp"
#include "boost/mpl/not.hpp"

namespace IECore
{

/// Optimised specialisation for identity conversions - just returns a cheap copy of the original data
template<typename F, typename T, typename C>
struct DataConvert< F, T, C, typename boost::enable_if< typename C::IsIdentity >::type >
{
	typename T::Ptr operator()( typename F::ConstPtr f )
	{
		assert( f );
		return f->copy();
	}

	typename T::Ptr operator()( typename F::ConstPtr f, C &c )
	{
		assert( f );
		return f->copy();
	}
};

template<typename F, typename T, typename C>
struct DataConvert< F, T, C, typename boost::enable_if< boost::mpl::and_< boost::mpl::not_< typename C::IsIdentity >, TypeTraits::IsVectorTypedData<F> > >::type >
{

	typename T::Ptr operator()( typename F::ConstPtr f )
	{
		C c;
		return this->operator()( f, c );
	}

	typename T::Ptr operator()( typename F::ConstPtr f, C &c )
	{
		assert( f );

		typename T::Ptr result = new T();
		assert( result );
		result->writable().resize( f->readable().size() );

		assert( result->readable().size() == f->readable().size() );
		std::transform( f->readable().begin(),  f->readable().end(), result->writable().begin(), c );

		return result;
	}
};

template<typename F, typename T, typename C>
struct DataConvert< F, T, C, typename boost::enable_if< boost::mpl::and_< boost::mpl::not_< typename C::IsIdentity >, TypeTraits::IsSimpleTypedData<F> > >::type >
{
	typename T::Ptr operator()( typename F::ConstPtr f )
	{
		C c;
		return this->operator()( f, c );
	}

	typename T::Ptr operator()( typename F::ConstPtr f, C &c )
	{
		assert( f );

		typename T::Ptr result = new T();
		assert( result );
		result->writable() = c( f->readable() );

		return result;
	}
};

}
