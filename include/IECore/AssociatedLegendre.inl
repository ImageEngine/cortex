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

#include <cassert>

#include "OpenEXR/ImathMath.h"

namespace IECore
{


template < typename V >
V AssociatedLegendre<V>::evaluate( unsigned int mm, V x )
{
	double pmm = 1.0;
	if ( mm > 0 )
	{
		double somx2 = sqrt( static_cast<double>( (1.0 - x)*(1.0 + x) ) );
		double fact = 1.0;
		for ( unsigned int i = 1; i <= mm; i++ )
		{
			pmm *= (-fact) * somx2;
			fact += 2.0;
		}
	}
	return static_cast<V>(pmm);
}

template < typename V >
V AssociatedLegendre<V>::evaluateFromRecurrence1( unsigned int l, unsigned int m, V x, V p1, V p2 )
{
	return static_cast<V>( ( x*(2.0*l-1.0)*p1 - (l+m-1.0)*p2 ) / (l-m) );
}

template < typename V >
V AssociatedLegendre<V>::evaluateFromRecurrence2( unsigned int l, V x, V p1 )
{
	return static_cast<V>( x*(2.0*l+1.0)*p1 );
}

template < typename V >
V AssociatedLegendre<V>::evaluate( unsigned int l, unsigned int m, V x )
{
	V p2 = evaluate( m, x );
	if ( l == m )
	{
		return p2;
	}
	V p1 = evaluateFromRecurrence2( m, x, p2 );

	if ( l == m+1 )
	{
		return p1;
	}

	V pl = 0.0;
	for ( unsigned int ll = m+2; ll <= l; ll++ )
	{
		pl = evaluateFromRecurrence1( ll, m, x, p1, p2 );
		p2 = p1;
		p1 = pl;
	}
	return pl;
}

template < typename V >
V AssociatedLegendre<V>::normalizationFactor( unsigned int l, unsigned int m )
{
	computeFactorials(l);
	std::vector< double > &f = factorials();
	double temp = ((2.0 * l + 1.0) * f[l-m]) / (4.0*M_PI*f[l+m]);
	return static_cast<V>( sqrt(temp) );
}

template< typename V >
std::vector< double > &AssociatedLegendre<V>::factorials()
{
	static std::vector< double > f;
	return f;
}

template < typename V >
void AssociatedLegendre<V>::computeFactorials( unsigned int l )
{
	std::vector< double > &f = factorials();
	unsigned int curSize = f.size();
	unsigned int newSize = l*2+1;
	if ( curSize >= newSize )
	{
		return;
	}
	f.resize( newSize );
	double previous = 1;
	if (!curSize)
	{
		f[0] = 1;
		curSize++;
	}

	previous = f[curSize-1];

	for ( std::vector<double>::iterator it = f.begin() + curSize; curSize < newSize; curSize++, it++ )
	{
		*it = previous * curSize;
		previous = *it;
	}
}

}	// namespace IECore
