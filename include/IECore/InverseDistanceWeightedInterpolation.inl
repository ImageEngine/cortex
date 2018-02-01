//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECore/VectorOps.h"

#include "OpenEXR/ImathLimits.h"

#include <algorithm>

namespace IECore
{

template<typename PointIterator, typename ValueIterator>
InverseDistanceWeightedInterpolation<PointIterator, ValueIterator>::InverseDistanceWeightedInterpolation(
	PointIterator firstPoint,
	PointIterator lastPoint,
	ValueIterator firstValue,
	ValueIterator lastValue,
	unsigned int numNeighbours,
	int maxLeafSize
) : m_firstPoint( firstPoint ), m_firstValue( firstValue ), m_numNeighbours( numNeighbours )
{
	assert( lastPoint-firstPoint == lastValue-firstValue );
	m_tree = new Tree( firstPoint, lastPoint );
}

template<typename PointIterator, typename ValueIterator>
InverseDistanceWeightedInterpolation<PointIterator, ValueIterator>::~InverseDistanceWeightedInterpolation()
{
	assert( m_tree );
	delete m_tree;
}

template<typename PointIterator, typename ValueIterator>
typename InverseDistanceWeightedInterpolation<PointIterator, ValueIterator>::Value InverseDistanceWeightedInterpolation<PointIterator, ValueIterator>::operator()( const Point &p ) const
{
	NeighbourVector neighbours;
	return operator() ( p, neighbours );
}

template<typename PointIterator, typename ValueIterator>
typename InverseDistanceWeightedInterpolation<PointIterator, ValueIterator>::Value InverseDistanceWeightedInterpolation<PointIterator, ValueIterator>::operator()( const Point &p, NeighbourVector &neighbours ) const
{
	assert( m_tree );

	Value result;
	vecSetAll( result, 0 );

	neighbours.clear();
	unsigned int neighbourCount = m_tree->nearestNNeighbours( p, m_numNeighbours, neighbours );

	if( neighbourCount )
	{

		PointBaseType distanceToFurthest = std::max<PointBaseType>( Imath::Math<PointBaseType>::sqrt( neighbours.rbegin()->distSquared ), 1.e-6 );

		PointBaseType totalNeighbourWeight = 0.0;

		for( typename NeighbourVector::const_iterator it = neighbours.begin(); it != neighbours.end(); ++it )
		{
			const PointIterator &neighbourPointIt = it->point;

			const Value &neighbourValue = *(m_firstValue + (neighbourPointIt - m_firstPoint));

			PointBaseType distanceToNeighbour = std::max<PointBaseType>( Imath::Math<PointBaseType>::sqrt( it->distSquared ), 1.e-6 );
			assert( distanceToNeighbour <= distanceToFurthest );

			// Franke & Nielson's (1980) improvement on Shephard's original weight function
			PointBaseType neighbourWeight = ( distanceToFurthest - distanceToNeighbour ) / ( distanceToFurthest * distanceToNeighbour );
			assert( neighbourWeight >= -Imath::limits<PointBaseType>::epsilon() );

			neighbourWeight = std::max<PointBaseType>(neighbourWeight * neighbourWeight, 1.e-6 );

			result = result + ( neighbourValue * neighbourWeight );

			totalNeighbourWeight += neighbourWeight;
		}

		result = result * ( 1.0 / totalNeighbourWeight );
	}

	return result;
}


} // namespace IECore
