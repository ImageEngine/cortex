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
#include "IECore/EuclidianToSphericalTransform.h"

namespace IECore
{

template < typename V >
SphericalHarmonics<V>::SphericalHarmonics( unsigned int bands ) : m_bands(bands)
{
	m_coefficients.resize( bands * bands, V(0) );
}

template < typename V >
SphericalHarmonics<V>::SphericalHarmonics( const SphericalHarmonics &sh ) : m_bands( sh.bands() )
{
	m_coefficients.resize( sh.coefficients().size() );
	std::copy( sh.coefficients().begin(), sh.coefficients().end(), m_coefficients.begin() );
}

template < typename V >
SphericalHarmonics<V>::~SphericalHarmonics()
{
}

template < typename V >
const SphericalHarmonics<V> & SphericalHarmonics<V>::operator =( const SphericalHarmonics<V> &sh )
{
	m_bands = sh.bands();
	m_coefficients.resize( sh.coefficients().size() );
	std::copy( sh.coefficients().begin(), sh.coefficients().end(), m_coefficients.begin() );
	return *this;
}

template < typename V >
void SphericalHarmonics<V>::setBands( unsigned int bands )
{
	m_bands = bands;
	m_coefficients.resize( bands * bands, V(0) );
}

template < typename V >
V SphericalHarmonics<V>::operator() ( const Imath::Vec2< BaseType > &phiTheta ) const
{
	return operator()( phiTheta, m_bands );
}

template < typename V >
V SphericalHarmonics<V>::operator() ( const Imath::Vec2< BaseType > &phiTheta, unsigned int bands ) const
{
	if ( bands > m_bands )
	{
		bands = m_bands;
	}
	SHEvaluator evaluator( this );
	RealSphericalHarmonicFunction< BaseType >::evaluate( phiTheta.x, phiTheta.y, bands, evaluator );
	return evaluator.result();
/*
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
*/
}

template < typename V >
V SphericalHarmonics<V>::operator() ( const Imath::Vec3< BaseType > &xyz ) const
{
	return operator()( EuclidianToSphericalTransform< Imath::Vec3< BaseType >, Imath::Vec2< BaseType > >().transform( xyz ), m_bands );
}

template < typename V >
V SphericalHarmonics<V>::operator() ( const Imath::Vec3< BaseType > &xyz, unsigned int bands ) const
{
	return operator()( EuclidianToSphericalTransform< Imath::Vec3< BaseType >, Imath::Vec2< BaseType > >().transform( xyz ), bands );
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

template< typename V >
template < typename T >
void SphericalHarmonics<V>::convolve( const SphericalHarmonics<T> &sh )
{
	const int numBands = std::min( m_bands, sh.bands() );
	typename CoefficientVector::iterator it = m_coefficients.begin();
	for( int l=0; l<numBands; l++ )
	{
		const double alpha = sqrt( 4*M_PI / (2*l + 1) );
		T coeffs = sh.coefficients()[ l*(l+1) ];
		for( int m=-l; m <= l; m++, it++ )
		{
			(*it) *= alpha * coeffs;
		}
	}
	for( ; it != m_coefficients.end(); it++ )
	{
		(*it) = 0;
	}
}

template <class S>
SphericalHarmonics<S> operator + ( const SphericalHarmonics<S> &lsh, const SphericalHarmonics<S> &rsh )
{
	SphericalHarmonics<S> sh( max( lsh.bands(), rsh.bands() ) );
	typename SphericalHarmonics<S>::CoefficientVector::iterator it;
	typename SphericalHarmonics<S>::CoefficientVector::const_iterator lit, rit;
	for ( it = sh.coefficients().begin(), rit = rsh.coefficients().begin(), lit = lsh.coefficients().begin(); rit != rsh.coefficients().end() && lit != lsh.coefficients().end(); it++, rit++, lit++ )
	{
		*it = (*lit) + (*rit);
	}
	for ( ; rit != rsh.coefficients().end(); it++, rit++ )
	{
		*it = (*rit);
	}
	for ( ; lit != lsh.coefficients().end(); it++, lit++ )
	{
		*it = (*lit);
	}
	return sh;
}

template <class S>
const SphericalHarmonics<S> & operator += ( SphericalHarmonics<S> &lsh, const SphericalHarmonics<S> &rsh )
{
	if ( lsh.bands() < rsh.bands() )
	{
		lsh.setBands( rsh.bands() );
	}
	typename SphericalHarmonics<S>::CoefficientVector::iterator it;
	typename SphericalHarmonics<S>::CoefficientVector::const_iterator cit;
	for ( cit = rsh.coefficients().begin(), it = lsh.coefficients().begin(); cit != rsh.coefficients().end() && it != lsh.coefficients().end(); cit++, it++ )
	{
		*it += (*cit);
	}
	return lsh;
}

template <class S>
SphericalHarmonics<S> operator - ( const SphericalHarmonics<S> &lsh, const SphericalHarmonics<S> &rsh )
{
	SphericalHarmonics<S> sh( max( lsh.bands(), rsh.bands() ) );
	typename SphericalHarmonics<S>::CoefficientVector::iterator it;
	typename SphericalHarmonics<S>::CoefficientVector::const_iterator lit, rit;
	for ( it = sh.coefficients().begin(), rit = rsh.coefficients().begin(), lit = lsh.coefficients().begin(); rit != rsh.coefficients().end() && lit != lsh.coefficients().end(); it++, rit++, lit++ )
	{
		*it = (*lit) - (*rit);
	}
	for ( ; rit != rsh.coefficients().end(); it++, rit++ )
	{
		*it = -(*rit);
	}
	for ( ; lit != lsh.coefficients().end(); it++, lit++ )
	{
		*it = (*lit);
	}
	return sh;
}

template <class S>
const SphericalHarmonics<S> & operator -= ( SphericalHarmonics<S> &lsh, const SphericalHarmonics<S> &rsh )
{
	if ( lsh.bands() < rsh.bands() )
	{
		lsh.setBands( rsh.bands() );
	}
	typename SphericalHarmonics<S>::CoefficientVector::iterator it;
	typename SphericalHarmonics<S>::CoefficientVector::const_iterator cit;
	for ( cit = rsh.coefficients().begin(), it = lsh.coefficients().begin(); cit != rsh.coefficients().end() && it != lsh.coefficients().end(); cit++, it++ )
	{
		*it -= (*cit);
	}
	return lsh;
}

template <class S, class T>
SphericalHarmonics<S> operator * ( const SphericalHarmonics<S> &lsh, const T &scale )
{
	SphericalHarmonics<S> sh( lsh.bands() );
	typename SphericalHarmonics<S>::CoefficientVector::iterator it;
	typename SphericalHarmonics<S>::CoefficientVector::const_iterator lit;
	for ( it = sh.coefficients().begin(), lit = lsh.coefficients().begin(); lit != lsh.coefficients().end(); it++, lit++ )
	{
		*it = (*lit) * scale;
	}
	return sh;
}

template <class S, class T>
const SphericalHarmonics<S> & operator *= ( SphericalHarmonics<S> &lsh, const T &scale )
{
	typename SphericalHarmonics<S>::CoefficientVector::iterator it;
	for ( it = lsh.coefficients().begin(); it != lsh.coefficients().end(); it++ )
	{
		*it *= scale;
	}
	return lsh;
}


}	// namespace IECore
