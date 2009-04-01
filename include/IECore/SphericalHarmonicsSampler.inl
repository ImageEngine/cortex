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
SphericalHarmonicsSampler<V>::SphericalHarmonicsSampler( unsigned int bands, unsigned int samples, unsigned long int seed ) : 
	m_bands( bands ),
	m_sphericalCoordinates( new V2fVectorData() ), 
	m_euclidianCoordinates( 0 ),
	m_shEvaluations(),
	m_weights( 0 )
{
	int sqrtSamples = int(round(sqrt((double)samples)));

	Imath::Rand32 random( seed );
	V invN = 1.0 / V(sqrtSamples);
	Imath::V2f sc;

	m_sphericalCoordinates->writable().resize( sqrtSamples * sqrtSamples );

	std::vector< Imath::V2f >::iterator it = m_sphericalCoordinates->writable().begin();

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
	evaluateSphericalHarmonicsSamples();
}

template < typename V >
SphericalHarmonicsSampler<V>::SphericalHarmonicsSampler( unsigned int bands, ConstV2fVectorDataPtr &sphericalCoordinates ) : 
	m_bands( bands ),
	m_sphericalCoordinates( sphericalCoordinates->copy() ), 
	m_euclidianCoordinates( 0 ),
	m_shEvaluations(),
	m_weights( 0 )
{
	evaluateSphericalHarmonicsSamples();
}
	
template < typename V >
SphericalHarmonicsSampler<V>::SphericalHarmonicsSampler( unsigned int bands, ConstV2fVectorDataPtr &sphericalCoordinates, ConstFloatVectorDataPtr &weights ) :
	m_bands( bands ),
	m_sphericalCoordinates( sphericalCoordinates->copy() ), 
	m_euclidianCoordinates( 0 ),
	m_shEvaluations(),
	m_weights( weights->copy() )
{
	assert( sphericalCoordinates->readable().size() == weights->readable().size() );
	evaluateSphericalHarmonicsSamples();
}

template < typename V >
ConstV3fVectorDataPtr SphericalHarmonicsSampler<V>::euclidianCoordinates() const
{
	if (!m_euclidianCoordinates)
	{
		m_euclidianCoordinates = new V3fVectorData();
		m_euclidianCoordinates->writable().resize( m_sphericalCoordinates->readable().size() );
		std::vector< Imath::V3f >::iterator it = m_euclidianCoordinates->writable().begin();
		std::vector< Imath::V2f >::const_iterator cit = m_sphericalCoordinates->readable().begin();
		for ( ; it != m_euclidianCoordinates->writable().end(); it++, cit++ )
		{
			*it = sphericalCoordsToUnitVector( *cit );
		}
	}
	return m_euclidianCoordinates;
}

template< typename V >
template< typename T, typename U > 
void SphericalHarmonicsSampler<V>::polarProjection( T functor, boost::intrusive_ptr< SphericalHarmonics< U > > result ) const
{
	double factor;

	// zero coefficients to start accumulation.
	for (typename SphericalHarmonics<U>::CoefficientVector::iterator rit = result->coefficients().begin(); rit != result->coefficients().end(); rit++ )
	{
		*rit = U(0);
	}
	std::vector< Imath::V2f >::const_iterator cit = m_sphericalCoordinates->readable().begin();
	if ( m_weights )
	{
		std::vector< float >::const_iterator wit = m_weights->readable().begin();
		for ( typename EvaluationSamples::const_iterator it = m_shEvaluations.begin(); it != m_shEvaluations.end(); it++, cit++, wit++ )
		{
			addProjection( result->coefficients(), *it, functor( *cit ) * (*wit) );
		}
		factor = 1. / (double)m_shEvaluations.size();

	}
	else
	{
		// uniform distribution weights
		for ( typename EvaluationSamples::const_iterator it = m_shEvaluations.begin(); it != m_shEvaluations.end(); it++, cit++ )
		{
			addProjection( result->coefficients(), *it, functor( *cit ) );
		}
		const double weight = 4 * M_PI;
		factor = weight / (double)m_shEvaluations.size();
	}
	for (typename SphericalHarmonics<U>::CoefficientVector::iterator rit = result->coefficients().begin(); rit != result->coefficients().end(); rit++ )
	{
		*rit *= factor;
	}
}

template< typename V >
template< typename T, typename U > 
void SphericalHarmonicsSampler<V>::euclideanProjection( T functor, boost::intrusive_ptr< SphericalHarmonics< U > > result ) const
{
	double factor;

	// make sure our internal object is created.
	euclidianCoordinates();
	
	// zero coefficients to start accumulation.
	for (typename SphericalHarmonics<U>::CoefficientVector::iterator rit = result->coefficients().begin(); rit != result->coefficients().end(); rit++ )
	{
		*rit = U(0);
	}
	std::vector< Imath::V3f >::const_iterator cit = m_euclidianCoordinates->readable().begin();
	if ( m_weights )
	{
		std::vector< float >::const_iterator wit = m_weights->readable().begin();
		for ( typename EvaluationSamples::const_iterator it = m_shEvaluations.begin(); it != m_shEvaluations.end(); it++, cit++, wit++ )
		{
			addProjection( result->coefficients(), *it, functor( *cit ) * (*wit) );
		}
		factor = 1. / m_shEvaluations.size();
	}
	else
	{
		// uniform distribution weights
		for ( typename EvaluationSamples::const_iterator it = m_shEvaluations.begin(); it != m_shEvaluations.end(); it++, cit++ )
		{
			addProjection( result->coefficients(), *it, functor( *cit ) );
		}
		const double weight = 4 * M_PI;
		factor = weight / m_shEvaluations.size();
	}
	for (typename SphericalHarmonics<U>::CoefficientVector::iterator rit = result->coefficients().begin(); rit != result->coefficients().end(); rit++ )
	{
		*rit *= factor;
	}
}

template< typename V >
template< typename T > 
void SphericalHarmonicsSampler<V>::reconstruction( const boost::intrusive_ptr< SphericalHarmonics< T > > sh, boost::intrusive_ptr< TypedData< std::vector< T > > > result ) const
{
	result->writable().resize( m_shEvaluations.size() );

	typename std::vector< T >::iterator rit = result->writable().begin();
	typename SphericalHarmonics<T>::CoefficientVector::const_iterator shIt;
	typename EvaluationVector::const_iterator eIt;
	
	for ( typename EvaluationSamples::const_iterator it = m_shEvaluations.begin(); it != m_shEvaluations.end(); it++, rit++ )
	{
		T acc(0);
		// multiplies the spherical harmonics coefficients by their evaluations at each sampling point.
		for ( shIt = sh->coefficients().begin(), eIt = it->begin(); shIt != sh->coefficients().end() && eIt != it->end(); shIt++, eIt++ )
		{
			acc += (*shIt) * (*eIt);
		}
		*rit = acc;
	}
}

template< typename V >
Imath::V3f SphericalHarmonicsSampler<V>::sphericalCoordsToUnitVector( const Imath::V2f &sphericalCoord )
{
	V sinTheta = Imath::Math<V>::sin( sphericalCoord.x );
	return Imath::V3f( sinTheta*Imath::Math<V>::cos( sphericalCoord.y), sinTheta*Imath::Math<V>::sin( sphericalCoord.y ), Imath::Math<V>::cos( sphericalCoord.x ));
}

template< typename V >
void SphericalHarmonicsSampler<V>::evaluateSphericalHarmonicsSamples()
{
	m_shEvaluations.resize( m_sphericalCoordinates->readable().size() );

	std::vector< Imath::V2f >::const_iterator cit = m_sphericalCoordinates->readable().begin();
	for ( typename EvaluationSamples::iterator it = m_shEvaluations.begin(); it != m_shEvaluations.end(); it++, cit++ )
	{
		RealSphericalHarmonicFunction<V>::evaluate( m_bands, static_cast<V>(cit->x), static_cast<V>(cit->y), *it );
	}

}

template< typename V >
template< typename T >
void SphericalHarmonicsSampler<V>::addProjection( typename SphericalHarmonics<T>::CoefficientVector &c, const EvaluationVector &v, const T &scale )
{
	typename SphericalHarmonics<T>::CoefficientVector::iterator it;
	typename EvaluationVector::const_iterator cit;

	for ( it = c.begin(), cit = v.begin(); it != c.end() && cit != v.end(); it++, cit++ )
	{
		(*it) += (*cit) * scale;
	}
}

} // namespace IECore
