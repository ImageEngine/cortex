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

#include "OpenEXR/ImathMath.h"
#include "AssociatedLegendre.h"

namespace IECore
{

template < typename V >
V RealSphericalHarmonicFunction<V>::evaluate( V phi, V theta, unsigned int l, int m )
{
	unsigned int absM = (unsigned int)( m < 0 ? -m : m );
	return evaluateFromLegendre( phi, l, m, AssociatedLegendre<double>::evaluate( l, absM, Imath::Math<V>::cos( theta ) ) );
}

template < typename V >
void RealSphericalHarmonicFunction<V>::evaluate( V phi, V theta, unsigned int bands, std::vector<V> &result )
{
	result.resize( bands * bands );
	typename std::vector<V>::iterator it = result.begin();
	for ( unsigned int l = 0; l < bands; l++ )
	{
		for (int m = -static_cast<int>(l); m <= static_cast<int>(l); m++, it++ )
		{
			*it = evaluate( phi, theta, l, m );
		}
	}

}

template < typename V >
V RealSphericalHarmonicFunction<V>::evaluateFromLegendre( V phi, unsigned int l, int m, double legendreEval )
{
	static const V sqrt2 = Imath::Math<V>::sqrt(2.0);
	if ( m > 0 )
	{
		return sqrt2 * AssociatedLegendre<double>::normalizationFactor( l, static_cast<unsigned int>(m) ) * Imath::Math<V>::cos( m*phi ) * legendreEval;
	}
	if ( m < 0 )
	{
		return sqrt2 * AssociatedLegendre<double>::normalizationFactor( l, static_cast<unsigned int>(-m) ) * Imath::Math<V>::sin( -m*phi ) * legendreEval;
	}
	return AssociatedLegendre<double>::normalizationFactor( l, 0 ) * legendreEval;
}

template< typename V >
void RealSphericalHarmonicFunction<V>::evaluate( V phi, V theta, unsigned int bands, boost::function< void ( unsigned int, int, V ) > functor )
{
	V cosTheta = Imath::Math<V>::cos(theta);
	V p1, p2, pl;
	for ( unsigned int l = 0; l < bands; l++ )
	{
		// compute Pmm ( l = m )
		p2 = AssociatedLegendre<double>::evaluate( l, cosTheta );
		functor( l, (int)l, evaluateFromLegendre( phi, l, (int)l, p2 ) );
		if ( l )
		{
			functor( l, -(int)l, evaluateFromLegendre( phi, l, -(int)l, p2 ) );
		}

		if ( l == bands - 1 )
			continue;

		// compute Pm+1m ( l = m+1 )
		p1 = AssociatedLegendre<double>::evaluateFromRecurrence2( l, cosTheta, p2 );
		functor( l+1, (int)l, evaluateFromLegendre( phi, l+1, (int)l, p1 ) );
		// compute m negative
		if ( l )
		{
			functor( l+1, -(int)l, evaluateFromLegendre( phi, l+1, -(int)l, p1 ) );
		}

		// compute Pm+1+xm ( l = [m+1,m+1+x] )
		for ( unsigned int ll = l+2; ll < bands; ll++ )
		{
			pl = AssociatedLegendre<double>::evaluateFromRecurrence1( ll, (int)l, cosTheta, p1, p2 );
			functor( ll, (int)l, evaluateFromLegendre( phi, ll, (int)l, pl ) );
			if ( l )
			{
				functor( ll, -(int)l, evaluateFromLegendre( phi, ll, -(int)l, pl ) );
			}
			p2 = p1;
			p1 = pl;
		}
	}
}

} // namespace IECore
