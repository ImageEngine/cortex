//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "OpenEXR/ImathMath.h"
#include <algorithm>

using namespace IECore;
using namespace Imath;
using namespace std;

namespace IECore
{

template <typename V >
static inline V sqr( V x ) { return x * x; }

template <typename V >
static inline V exp3( V x ) { return x * x * x; }

template <typename V >
static inline V exp4( V x ) { return x * x * x * x; }

template< typename T >
T AssociatedLegendreTest<T>::targetPolynomial( unsigned int l, unsigned int m, T x )
{
	if ( l == 0 && m == 0 )
		return 1;

	if ( l == 1 )
	{
		if ( m == 0 )
			return x;
		return -Imath::Math<T>::sqrt(1.-sqr(x));
	}
	if ( l == 2 )
	{
		if ( m == 0 )
			return (3.*sqr(x) - 1.) / 2.;
		if ( m == 1 )
			return -3.*x*Imath::Math<T>::sqrt(1.-sqr(x));
		if ( m == 2 )
			return 3. * (1.-sqr(x));
	}
	if ( l == 3 )
	{
		if ( m == 0 )
			return (5.*exp3(x) - 3.*x) / 2.;
		if ( m == 1 )
			return (-3. * (5*sqr(x) - 1.)*sqrt(1.-sqr(x))) / 2.;
		if ( m == 2 )
			return 15. * x * (1.-sqr(x));
		if ( m == 3 )
			return -15.*Imath::Math<T>::pow(1.-sqr(x), 3./2.);
	}
	if ( l == 4 )
	{
		if ( m == 0 )
			return (35.*exp4(x) - 30.*sqr(x)+3.) / 8.;
		if ( m == 1 )
			return (-5. * (7. * exp3(x) - 3.*x)*sqrt(1. - sqr(x))) / 2.;
		if ( m == 2 )
			return (15. * ( 7. * sqr(x) - 1. )* (1.-sqr(x)) ) / 2.;
		if ( m == 3 )
			return -105.*x*Imath::Math<T>::pow(1-sqr(x), 3./2.);
		if ( m == 4 )
			return 105.*sqr(1-sqr(x));
	}
	return 0;
}

template< typename T >
void AssociatedLegendreTest<T>::testEvaluation()
{
	T target;
	T res;

	for ( T x = 0; x < 1; x += 0.3 )
	{
		for ( unsigned int l = 0; l < 5; l++ )
		{
			for ( unsigned int m = 0; m <= l; m++ )
			{
				target = targetPolynomial( l, m, x );
				res = AssociatedLegendre<T>::evaluate( l, m, x );
				if (!Imath::equalWithRelError ( res, target, T(0.0001)))
				{
					cout << "Failed l: " << l << " m: " << m << " at x: " << x << endl;
					BOOST_CHECK_EQUAL( target, res );
				}
			}
		}
	}
}

template< typename T >
void AssociatedLegendreTest<T>::testDepthEvaluation()
{
	T x = 0.3;
	T res;
	for ( unsigned int l = 0; l < 50; l++ )
	{
		for ( unsigned int m = 0; m <= l; m++ )
		{
			res = AssociatedLegendre<T>::evaluate( l, m, x );
			BOOST_CHECK( !isnan( res ) );
			res = AssociatedLegendre<T>::normalizationFactor( l, m );
			BOOST_CHECK( !isnan( res ) );
		}
	}
}

}
