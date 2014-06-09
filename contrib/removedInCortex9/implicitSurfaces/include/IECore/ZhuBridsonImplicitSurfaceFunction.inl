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

#include "IECore/VectorTraits.h"

namespace IECore
{

template<typename P, typename V>
ZhuBridsonImplicitSurfaceFunction<P,V>::ZhuBridsonImplicitSurfaceFunction( typename PointVectorData::ConstPtr p, ConstDoubleVectorDataPtr r, V smoothingRadius ) : m_smoothingRadius( smoothingRadius )
{
	assert( p );
	assert( r );
	
	m_p = p->copy();
	m_radius = r->copy();

	if ( m_p->readable().size() != m_radius->readable().size() )
	{
		throw InvalidArgumentException( "Incompatible point/radius data given to ZhuBridsonImplicitSurfaceFunction" );
	}

	m_tree = new Tree( m_p->readable().begin(), m_p->readable().end() );

	assert( m_tree );
}

template<typename P, typename V>
ZhuBridsonImplicitSurfaceFunction<P,V>::~ZhuBridsonImplicitSurfaceFunction()
{
	assert( m_tree );
	delete m_tree;
}

template<typename P, typename V>
typename ZhuBridsonImplicitSurfaceFunction<P,V>::Value ZhuBridsonImplicitSurfaceFunction<P,V>::k( const Value &s ) const
{
	Value t = Value( 1.0 ) - s*s;

	return std::max< Value >( 0, t*t*t );
}

template<typename P, typename V>
typename ZhuBridsonImplicitSurfaceFunction<P,V>::Value ZhuBridsonImplicitSurfaceFunction<P,V>::operator()( const Point &p )
{
	assert( m_tree );
	assert( m_p );
	assert( m_radius );

	std::vector<PointIterator> nearest;

	m_tree->nearestNeighbours( p, m_smoothingRadius, nearest );
	
	if ( !nearest.size() )
	{
		/// \todo The paper does not specify what to do in the case that there are no neighbouring particles. Here we just
		/// return "a long way outside", but that may not always be correct. Ideally the particles would be fairly well distributed
		/// inside the desired surface to mesh, and the smoothing radius would be set correctly, however.
		return Imath::limits<Value>::max();
	}

	/// Store all weights and compute their sum
	Value weightSum( 0.0 );
	std::vector< Value > weights( nearest.size() );
	size_t weightIndex = 0;
	for ( typename std::vector<PointIterator>::const_iterator it = nearest.begin(); it != nearest.end(); ++it, ++weightIndex )
	{
		const Point &neighbourPosition = *(*it);
		Value weight = k( vecDistance( p, neighbourPosition ) / m_smoothingRadius );
		assert( weight >= 0.0 );

		weights[ weightIndex ] = weight;
		weightSum += weight;
	}

	weightIndex = 0;
	Value weightedAverageRadius( 0.0 );
	Point weightedAveragePosition;
	vecSetAll( weightedAveragePosition, 0.0 );
	for ( typename std::vector<PointIterator>::const_iterator it = nearest.begin(); it != nearest.end(); ++it, ++weightIndex )
	{
		const size_t pointIndex = std::distance<PointIterator>( m_p->readable().begin(), *it );
		assert( pointIndex >= 0 );
		assert( pointIndex < m_p->readable().size() );
		
		/// Normalize weight
		const Value neighbourWeight = weights[ weightIndex ] / weightSum;		
		
		const Value &neighbourRadius = m_radius->readable()[ pointIndex ];
		weightedAverageRadius += neighbourWeight * neighbourRadius;

		Point weightedNeighbourPosition = *(*it);
		vecMul( weightedNeighbourPosition, neighbourWeight );
		vecAdd( weightedAveragePosition, weightedNeighbourPosition, weightedAveragePosition );		
	}
	
	return vecDistance( p, weightedAveragePosition ) - weightedAverageRadius;
}

template<typename P, typename V>
typename ZhuBridsonImplicitSurfaceFunction<P,V>::Value ZhuBridsonImplicitSurfaceFunction<P,V>::getValue( const Point &p )
{
	return this->operator()( p );
}

} // namespace IECore
