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
#include "IECore/VectorTraits.h"
#include "IECore/RealSphericalHarmonicFunction.h"

namespace IECore
{

template < typename V >
V SphericalHarmonics<V>::operator() ( const Imath::Vec2< BaseType > &phiTheta ) const
{
	V res(0);
	typename CoefficientVector::const_iterator cit = m_coefficients.begin();
	for ( unsigned int l = 0; l < m_bands; l++ )
	{
		for (int m = -static_cast<int>(l); m <= static_cast<int>(l); m++, cit++ )
		{
			res += (*cit) * V( RealSphericalHarmonicFunction< BaseType >::evaluate( phiTheta.x, phiTheta.y, l, m ) );
		}
	}
	return res;
}

template < typename V >
V SphericalHarmonics<V>::operator() ( const Imath::Vec2< BaseType > &phiTheta, unsigned int bands ) const
{
	if ( bands > m_bands )
	{
		bands = m_bands;
	}
	V res(0);
	typename CoefficientVector::const_iterator cit = m_coefficients.begin();
	for ( unsigned int l = 0; l < bands; l++ )
	{
		for (int m = -static_cast<int>(l); m <= static_cast<int>(l); m++, cit++ )
		{
			res += (*cit) * V( RealSphericalHarmonicFunction< BaseType >::evaluate( phiTheta.x, phiTheta.y, l, m ) );
		}
	}
	return res;
}

template< typename V >
template< typename T, typename R >
R SphericalHarmonics<V>::dot( const SphericalHarmonics<T> &s ) const
{
	T res(0);
	typename CoefficientVector::const_iterator ita = m_coefficients.begin();
	typename SphericalHarmonics<T>::CoefficientVector::const_iterator itb = s.coefficients().begin();

	for ( ; ita != m_coefficients.end() && itb != s.coefficients().end(); ita++, itb++ )
	{
		res += (*ita) * (*itb);
	}
	return res;
}


}	// namespace IECore
