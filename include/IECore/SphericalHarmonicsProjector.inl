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

#include <cassert>
#include "OpenEXR/ImathRandom.h"

namespace IECore
{


template < typename V >
SphericalHarmonicsProjector<V>::SphericalHarmonicsProjector( unsigned int samples, unsigned long int seed ) : 
	m_bands( 0 ),
	m_sphericalCoordinates(), 
	m_euclidianCoordinates(),
	m_shEvaluations(),
	m_weights( 0 )
{
	int sqrtSamples = int(round(sqrt((double)samples)));

	Imath::Rand32 random( seed );
	V invN = 1.0 / V(sqrtSamples);
	Imath::V2f sc;

	m_sphericalCoordinates.resize( sqrtSamples * sqrtSamples );

	typename std::vector< Imath::Vec2<V> >::iterator it = m_sphericalCoordinates.begin();

	for( int a = 0; a < sqrtSamples; a++ )
	{
		for( int b = 0; b < sqrtSamples; b++, it++ )
		{
			V x = (a + random.nextf()) * invN;
			V y = (b + random.nextf()) * invN;
			sc[0] = 2.0 * Imath::Math<float>::acos( sqrt( 1.0 - x ) );
			sc[1] = 2.0 * M_PI * y;
			*it = sc;
		}
	}
}

template < typename V >
SphericalHarmonicsProjector<V>::SphericalHarmonicsProjector( const std::vector< Imath::Vec2< V > > &sphericalCoordinates ) : 
	m_bands( 0 ),
	m_sphericalCoordinates( sphericalCoordinates ), 
	m_euclidianCoordinates( 0 ),
	m_shEvaluations(),
	m_weights( 0 )
{
}
	
template < typename V >
SphericalHarmonicsProjector<V>::SphericalHarmonicsProjector( const std::vector< Imath::Vec2< V > > &sphericalCoordinates, const std::vector< V > &weights ) :
	m_bands( 0 ),
	m_sphericalCoordinates( sphericalCoordinates ), 
	m_euclidianCoordinates( 0 ),
	m_shEvaluations(),
	m_weights( weights )
{
	assert( sphericalCoordinates.size() == weights.size() );
}

template < typename V >
const std::vector< Imath::Vec3< V > > &SphericalHarmonicsProjector<V>::euclidianCoordinates() const
{
	if (!m_euclidianCoordinates.size())
	{
		m_euclidianCoordinates.resize( m_sphericalCoordinates.size() );
		typename std::vector< Imath::Vec3<V> >::iterator it = m_euclidianCoordinates.begin();
		typename std::vector< Imath::Vec2<V> >::const_iterator cit = m_sphericalCoordinates.begin();
		for ( ; it != m_euclidianCoordinates.end(); it++, cit++ )
		{
			*it = sphericalCoordsToUnitVector( *cit );
		}
	}
	return m_euclidianCoordinates;
}

template< typename V >
template< typename T, typename U > 
void SphericalHarmonicsProjector<V>::polarProjection( T functor, SphericalHarmonics< U > &result ) const
{
	double factor;

	evaluateSphericalHarmonicsSamples( result.bands() );

	// zero coefficients to start accumulation.
	for (typename SphericalHarmonics<U>::CoefficientVector::iterator rit = result.coefficients().begin(); rit != result.coefficients().end(); rit++ )
	{
		*rit = U(0);
	}
	typename std::vector< Imath::Vec2<V> >::const_iterator cit = m_sphericalCoordinates.begin();
	if ( m_weights.size() )
	{
		typename std::vector< V >::const_iterator wit = m_weights.begin();
		for ( typename EvaluationSamples::const_iterator it = m_shEvaluations.begin(); it != m_shEvaluations.end(); it++, cit++, wit++ )
		{
			addProjection( result.coefficients(), *it, functor( *cit ) * (*wit) );
		}
		factor = 1. / (double)m_shEvaluations.size();

	}
	else
	{
		// uniform distribution weights
		for ( typename EvaluationSamples::const_iterator it = m_shEvaluations.begin(); it != m_shEvaluations.end(); it++, cit++ )
		{
			addProjection( result.coefficients(), *it, functor( *cit ) );
		}
		const double weight = 4 * M_PI;
		factor = weight / (double)m_shEvaluations.size();
	}
	for (typename SphericalHarmonics<U>::CoefficientVector::iterator rit = result.coefficients().begin(); rit != result.coefficients().end(); rit++ )
	{
		*rit *= factor;
	}
}

template< typename V >
template< typename T, typename U > 
void SphericalHarmonicsProjector<V>::euclideanProjection( T functor, SphericalHarmonics< U > &result ) const
{
	double factor;

	evaluateSphericalHarmonicsSamples( result.bands() );

	// make sure our internal object is created.
	euclidianCoordinates();
	
	// zero coefficients to start accumulation.
	for (typename SphericalHarmonics<U>::CoefficientVector::iterator rit = result.coefficients().begin(); rit != result.coefficients().end(); rit++ )
	{
		*rit = U(0);
	}
	typename std::vector< Imath::Vec3<V> >::const_iterator cit = m_euclidianCoordinates.begin();
	if ( m_weights.size() )
	{
		typename std::vector< V >::const_iterator wit = m_weights.begin();
		for ( typename EvaluationSamples::const_iterator it = m_shEvaluations.begin(); it != m_shEvaluations.end(); it++, cit++, wit++ )
		{
			addProjection( result.coefficients(), *it, functor( *cit ) * (*wit) );
		}
		factor = 1. / m_shEvaluations.size();
	}
	else
	{
		// uniform distribution weights
		for ( typename EvaluationSamples::const_iterator it = m_shEvaluations.begin(); it != m_shEvaluations.end(); it++, cit++ )
		{
			addProjection( result.coefficients(), *it, functor( *cit ) );
		}
		const double weight = 4 * M_PI;
		factor = weight / m_shEvaluations.size();
	}
	for (typename SphericalHarmonics<U>::CoefficientVector::iterator rit = result.coefficients().begin(); rit != result.coefficients().end(); rit++ )
	{
		*rit *= factor;
	}
}

template< typename V >
Imath::Vec3<V> SphericalHarmonicsProjector<V>::sphericalCoordsToUnitVector( const Imath::Vec2<V> &sphericalCoord )
{
	V sinTheta = Imath::Math<V>::sin( sphericalCoord.x );
	return Imath::Vec3<V>( sinTheta*Imath::Math<V>::cos( sphericalCoord.y), sinTheta*Imath::Math<V>::sin( sphericalCoord.y ), Imath::Math<V>::cos( sphericalCoord.x ));
}

template< typename V >
void SphericalHarmonicsProjector<V>::evaluateSphericalHarmonicsSamples( unsigned int bands ) const
{
	if ( m_bands >= bands )
		return;

	m_bands = bands;
	m_shEvaluations.resize( m_sphericalCoordinates.size() );

	typename std::vector< Imath::Vec2<V> >::const_iterator cit = m_sphericalCoordinates.begin();
	for ( typename EvaluationSamples::iterator it = m_shEvaluations.begin(); it != m_shEvaluations.end(); it++, cit++ )
	{
		RealSphericalHarmonicFunction<V>::evaluate( m_bands, static_cast<V>(cit->x), static_cast<V>(cit->y), *it );
	}

}

template< typename V >
template< typename T >
void SphericalHarmonicsProjector<V>::addProjection( typename SphericalHarmonics<T>::CoefficientVector &c, const EvaluationVector &v, const T &scale )
{
	typename SphericalHarmonics<T>::CoefficientVector::iterator it;
	typename EvaluationVector::const_iterator cit;

	for ( it = c.begin(), cit = v.begin(); it != c.end() && cit != v.end(); it++, cit++ )
	{
		(*it) += (*cit) * scale;
	}
}

} // namespace IECore
