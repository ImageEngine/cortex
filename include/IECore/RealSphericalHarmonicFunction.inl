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
	if ( m > 0 )
	{
		return Imath::Math<V>::sqrt(2.0) * 
				AssociatedLegendre<double>::normalizationFactor( l, static_cast<unsigned int>(m) ) * 
				Imath::Math<V>::cos( m*phi ) * 
				AssociatedLegendre<double>::evaluate( l, static_cast<unsigned int>(m), Imath::Math<V>::cos( theta ) );
	}
	if ( m < 0 )
	{
		return Imath::Math<V>::sqrt(2.0) * 
				AssociatedLegendre<double>::normalizationFactor( l, static_cast<unsigned int>(-m) ) * 
				Imath::Math<V>::sin( -m*phi ) * 
				AssociatedLegendre<double>::evaluate( l, static_cast<unsigned int>(-m), Imath::Math<V>::cos( theta ) );
	}
	return AssociatedLegendre<double>::normalizationFactor( l, 0 ) * 
				AssociatedLegendre<double>::evaluate( l, 0, Imath::Math<V>::cos(theta) );
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


} // namespace IECore
