//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include "boost/math/tools/rational.hpp"
#include "boost/test/floating_point_comparison.hpp"

#include "OpenEXR/ImathMath.h"
#include "OpenEXR/ImathRandom.h"

using namespace IECore;
using namespace Imath;
using namespace std;

namespace IECore
{

/// LevenbergMarquardtTestSimple
template<typename T>
class LevenbergMarquardtTestSimple<T>::Fn
{
	public:

		Fn( unsigned num ) : m_num( num )
		{
		}

		void operator()(
		        typename TypedData< std::vector<T> >::ConstPtr parameters,
		        typename TypedData< std::vector<T> >::Ptr errors
		)
		{
			for ( unsigned i = 1; i <= m_num; i ++ )
			{
				/// Distance between our guess and the real function
				errors->writable()[i-1] = Imath::Math<T>::fabs(
					parameters->readable()[i-1] -
					i * i
				);
			}
		}

		unsigned numErrors() const
		{
			return m_num;
		}

	protected :

		unsigned m_num;
};

template<typename T>
LevenbergMarquardtTestSimple<T>::LevenbergMarquardtTestSimple()
{
}

template<typename T>
LevenbergMarquardtTestSimple<T>::~LevenbergMarquardtTestSimple()
{
}

template<typename T>
void LevenbergMarquardtTestSimple<T>::test()
{
	unsigned num = 8;

	Fn fn( num );

	typename TypedData< std::vector<T> >::Ptr params = new TypedData< std::vector<T> >();
	params->writable().resize( num, 1.0 );

	IECore::LevenbergMarquardt< T, Fn > lm;
	lm.solve( params, fn );

	typename TypedData< std::vector<T> >::Ptr errors = new TypedData< std::vector<T> >();
	errors->writable().resize( num, 1.0 );

	fn( params, errors );

	for ( unsigned i = 1; i <= num; i ++ )
	{
		BOOST_CHECK_CLOSE( params->readable()[i-1], T( i * i ), T( 1 ) );
		BOOST_CHECK_SMALL( errors->readable()[i-1], T( 1.e-1 ) );
	}

}

/// LevenbergMarquardtTestPolynomialFit
template<typename T>
template<int N>
class LevenbergMarquardtTestPolynomialFit<T>::Fn
{
	public:

		Fn( unsigned num, Imath::Rand32 &r ) : m_num( num )
		{
			for ( int i = 0; i < N; i ++ )
			{
				m_coeffs[i] = r.nextf();
			}
		}

		void operator()(
		        typename TypedData< std::vector<T> >::ConstPtr parameters,
		        typename TypedData< std::vector<T> >::Ptr errors
		)
		{

			boost::array<T, N> testCoeffs;
			for ( unsigned i = 0; i < N; i ++ )
			{
				testCoeffs[i] = parameters->readable()[i];
			}

			for ( unsigned i = 0; i < m_num; i ++ )
			{
				/// Evaluate in range [-5, 5]
				T v1 = boost::math::tools::evaluate_polynomial<N, T, T>( testCoeffs, (T(i) / m_num - 0.5) * 10.0 );
				T v2 = boost::math::tools::evaluate_polynomial<N, T, T>( m_coeffs, (T(i) / m_num - 0.5) * 10.0);

				/// Distance between our guess and the real function
				errors->writable()[i] = Imath::Math<T>::fabs( v1 - v2 );
			}
		}

		unsigned numErrors() const
		{
			return m_num;
		}

		void check( typename TypedData< std::vector<T> >::ConstPtr parameters )
		{
			for ( unsigned i = 0; i < N; i ++ )
			{
				/// Allow 15% margin of error
				BOOST_REQUIRE_CLOSE( parameters->readable()[i], m_coeffs[i], T( 15.0 ) );
			}
		}

	protected :

		unsigned m_num;
		boost::array<T, N> m_coeffs;
};

template<typename T>
LevenbergMarquardtTestPolynomialFit<T>::LevenbergMarquardtTestPolynomialFit()
{
}

template<typename T>
LevenbergMarquardtTestPolynomialFit<T>::~LevenbergMarquardtTestPolynomialFit()
{
}

template<typename T>
template<int N>
void LevenbergMarquardtTestPolynomialFit<T>::test()
{
	Imath::Rand32 r( 88 );

	const unsigned numTests = 20;
	const int numSamples = N * 5;

	for ( unsigned j = 0; j < numTests; j ++ )
	{
		Fn<N> fn( numSamples, r );

		typename TypedData< std::vector<T> >::Ptr params = new TypedData< std::vector<T> >();

		/// Initial guess of coefficients = 1.0
		params->writable().resize( N, 1.0 );

		IECore::LevenbergMarquardt< T, Fn<N> > lm;
		lm.solve( params, fn );

		typename TypedData< std::vector<T> >::Ptr errors = new TypedData< std::vector<T> >();
		errors->writable().resize( numSamples, 1.0 );

		/// Find the coefficients used in the test function
		fn( params, errors );

		/// Verify
		fn.check( params );
	}
}

}
