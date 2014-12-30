//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "boost/math/special_functions/factorials.hpp"

#include "IECore/SphericalHarmonics.h"
#include "IECore/SphericalHarmonicsTensor.h"
#include "IECore/Math.h"

namespace IECore
{

// Internal class used by the operator *( SH, SH )
template< typename T, typename U, typename V >
class SHProduct
{
	public:

		SHProduct( const SphericalHarmonics<T> &shl, const SphericalHarmonics<U> &shr, SphericalHarmonics<V> &res ) : m_shl( shl ), m_shr( shr ), m_shRes( res )
		{
			m_shRes.setBands( std::min( shl.bands(), shr.bands() ) );
			m_shRes = V(0);
		}

		void operator() ( unsigned int i, unsigned int j, unsigned int k, double tensor )
		{
			m_shRes.coefficients()[ i ] += m_shl.coefficients()[ j ] * m_shr.coefficients()[ k ] * tensor;
		}

	private :

		const SphericalHarmonics<T> &m_shl;
		const SphericalHarmonics<U> &m_shr;
		SphericalHarmonics<V> &m_shRes;
};


template <class S, class T>
SphericalHarmonics<S> operator * ( const SphericalHarmonics<S> &sh1, const SphericalHarmonics<T> &sh2 )
{
	SphericalHarmonics<S> shResult;

	SHProduct< S, T, S > func( sh1, sh2, shResult );
	SphericalHarmonicsTensor::tensor().evaluate( shResult.bands(), func );
	return shResult;
}

template <class S, class T>
const SphericalHarmonics<S> operator *= ( SphericalHarmonics<S> &sh1, const SphericalHarmonics<T> &sh2 )
{
	sh1 = sh1 * sh2;
	return sh1;
}

template < class T >
SphericalHarmonics<T> lambertianKernel( unsigned int bands, bool normalized )
{
	SphericalHarmonics<T> sh( bands );
	typename SphericalHarmonics<T>::CoefficientVector::iterator it;
	double normalizationFactor = 1;
	if (normalized)
	{
		normalizationFactor = 1.0/M_PI;
	}
	for ( unsigned int b = 0; b < bands; b++ )
	{
		T &coeff = sh.coefficients()[ b*(b+1) ];
		if ( b == 0 )
			coeff = T( normalizationFactor*M_PI / sqrt( 4 * M_PI ) );
		else if ( b == 1 )
			coeff = T( normalizationFactor*sqrt( M_PI / 3 ) );
		else if ( !(b & 1) )
		{
			double facHalfB = boost::math::factorial<double>( b/2 );
			coeff = T( normalizationFactor*2*M_PI * sqrt((2*b+1)/(4*M_PI)) * ((b & 2 ? 1.0 : -1.0) / ((b+2)*(b-1))) * 
						( boost::math::factorial<double>( b ) / ( (1<<b) * facHalfB * facHalfB ) ) );
		}
	}
	return sh;
}

template < class T >
SphericalHarmonics<T> rotatedKernel( const SphericalHarmonics<T> &kernel, const Imath::V3f &direction )
{
	IECore::EuclideanToSphericalTransform3f2f euc2sph;
	Imath::V2f sph = euc2sph.transform( direction );
	// evaluate harmonics at the given direction
	std::vector< double > evaluations;
	IECore::RealSphericalHarmonicFunction<double>::evaluate( sph[0], sph[1], kernel.bands(), evaluations );
	std::vector< double >::const_iterator evIt = evaluations.begin();
	// rotate kernel taking advantage of it's rotation symmetry on Z.
	int l = 0;
	int m = 0;
	T basis = kernel.coefficients()[0] * sqrt( (4*M_PI)/(2*l+1) );
	SphericalHarmonics<T> sh( kernel.bands() );
	for ( typename SphericalHarmonics<T>::CoefficientVector::iterator shIt = sh.coefficients().begin(); shIt != sh.coefficients().end(); shIt++, evIt++ )
	{
		*shIt = basis * (*evIt);
		m++;
		if ( m > l )
		{
			l++;
			m = -l;
			basis = kernel.coefficients()[ l*(l+1) ] * sqrt( (4*M_PI)/(2*l+1) );
		}
	}
	return sh;
}

template < class T >
void windowingFilter( SphericalHarmonics<T> &sh, float windowSize )
{
	typename SphericalHarmonics<T>::CoefficientVector::iterator it = sh.coefficients().begin();
	for ( unsigned int l = 0; l < sh.bands(); l++ )
	{
		typename SphericalHarmonics<T>::BaseType factor = cos(M_PI/2.0 * ((float)l/windowSize));
		for ( unsigned m = 0; m < l*2+1; m++, it++ )
		{
			*it *= factor;
		}
	}
}


} // namespace IECore
