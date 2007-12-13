//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_PERLINNOISE_INL
#define IE_CORE_PERLINNOISE_INL

#include "IECore/VectorOps.h"
#include "IECore/FastFloat.h"
#include "IECore/ImathRandAdapter.h"

#include "OpenEXR/ImathFun.h"
#include "OpenEXR/ImathRandom.h"

#include <vector>
#include <algorithm>

#include <cassert>

namespace IECore 
{

template<typename P, typename V, typename F>
PerlinNoise<P, V, F>::PerlinNoise( unsigned long int seed )
{
	assert( m_maxPointDimensions>=PointTraits::dimensions() );

	// fill the permutation table
	m_perm.resize( m_permSize * 2 );
	
	// fill first half with the values 0-m_permSize
	unsigned int value=0;
	std::vector<unsigned int>::iterator it = m_perm.begin();
	while (it != m_perm.begin()+m_permSize)
	{
		*it++ = value++;
	}
	ImathRandAdapter<Imath::Rand32> random(seed);
	std::random_shuffle( m_perm.begin(), m_perm.begin() + m_permSize, random );
	// fill second half of table with a copy of first half
	std::copy( m_perm.begin(), m_perm.begin() + m_permSize, m_perm.begin() + m_permSize );
	
	// and initialise the gradients	
	initGradients( seed );
}

template<typename P, typename V, typename F>
PerlinNoise<P, V, F>::PerlinNoise( const PerlinNoise &other )
	: m_perm( other.m_perm ), m_grad( other.m_grad )
{
}

template<typename P, typename V, typename F>
void PerlinNoise<P, V, F>::initGradients( unsigned long int seed )
{
	m_grad.resize( m_permSize * PointTraits::dimensions() );
	Imath::Rand32 random( seed );
	for( unsigned int i=0; i<m_permSize; i++ )
	{
		Value *grad = &m_grad[i*PointTraits::dimensions()];
		for( unsigned int j=0; j<ValueTraits::dimensions(); j++ )
		{
			ValueBaseType length = 0;
			for( unsigned int d=0; d<PointTraits::dimensions(); d++ )
			{
				ValueBaseType v = random.nextf( -1.0f, 1.0f );
				vecSet( grad[d], j, v );
				length += v * v;
			}
			if( PointTraits::dimensions()!=1 )
			{
				length = Imath::Math<ValueBaseType>::sqrt( length );
				for( unsigned int d=0; d<PointTraits::dimensions(); d++ )
				{
					vecSet( grad[d], j, vecGet( grad[d], j ) / length );
				}
			}
		}
	}
}

template<typename P, typename V, typename F>
inline typename PerlinNoise<P, V, F>::Value PerlinNoise<P, V, F>::noise( const Point &p ) const
{
	int pi[m_maxPointDimensions];
	for( unsigned int i=0; i<PointTraits::dimensions(); i++ )
	{
		pi[i] = fastFloatFloor( vecGet( p, i ) );
	}
	return noiseWalk( pi, p, PointTraits::dimensions()-1 );
}

template<typename P, typename V, typename F>
inline typename PerlinNoise<P, V, F>::Value PerlinNoise<P, V, F>::operator()( const Point &p ) const
{
	return noise( p );
}

template<typename P, typename V, typename F>
inline typename PerlinNoise<P, V, F>::Value PerlinNoise<P, V, F>::noiseWalk( int *pi, const P &p, int d ) const
{
	if( d==-1 )
	{
		unsigned int perm = 0;
		for( unsigned int i=0; i<PointTraits::dimensions(); i++ )
		{
			perm = m_perm[perm+(pi[i]&m_permSize-1)];
		}
		const Value *grad = &m_grad[perm*PointTraits::dimensions()];
		V g( 0 );
		for( unsigned int i=0; i<PointTraits::dimensions(); i++ )
		{
			g += grad[i] * ( vecGet( p, i ) - pi[i] );
		}
		return g;
	}
	else
	{
		Value v0 = noiseWalk( pi, p, d-1 );
		pi[d] += 1;
		Value v1 = noiseWalk( pi, p, d-1 );
		pi[d] -= 1;
		return Imath::lerp( v0, v1, m_falloff( vecGet( p, d ) - pi[d] ) ); 
	}
}

template<typename P, typename V, typename F>
inline typename PerlinNoise<P, V, F>::ValueBaseType PerlinNoise<P, V, F>::weight( PointBaseType t )
{
	float t3 = t * t * t;
	float t4 = t3 * t;
	return 6.0f * t4 * t - 15.0f * t4 + 10.0f * t3;
	
}

template<class T>
inline T SmoothStepFalloff<T>::operator()( T t ) const
{
	return t * t * ( T( 3 ) - T( 2 ) * t );
};
 
template<class T>
inline T SmootherStepFalloff<T>::operator()( T t ) const
{
	T t3 = t * t * t;
	T t4 = t3 * t;
	return T( 6 ) * t4 * t - T( 15 ) * t4 + T( 10 ) * t3;
}
	
} // namespace IECore

#endif // IE_CORE_PERLINNOISE_INL
