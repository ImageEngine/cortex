//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_TRIANGULATOR_INL
#define IECORE_TRIANGULATOR_INL

#include "IECore/TriangleAlgo.h"

namespace IECore
{

template<typename PointIterator, typename MeshBuilder>
Triangulator<PointIterator, MeshBuilder>::Triangulator( typename MeshBuilder::Ptr builder )
	:	m_builder( builder ), m_baseVertexIndex( 0 )
{
}

template<typename PointIterator, typename MeshBuilder>
Triangulator<PointIterator, MeshBuilder>::~Triangulator()
{
}

template<typename PointIterator, typename MeshBuilder>
void Triangulator<PointIterator, MeshBuilder>::triangulate( PointIterator first, PointIterator last )
{

	// put all the vertices into the builder, and build a list containing
	// the vertices for our own traversal
	VertexList vertices;
	unsigned int size = 0;
	for( PointIterator it=first; it!=last; it++ )
	{
		m_builder->addVertex( *it, Point( 0 ) );
		vertices.push_back( Vertex( m_baseVertexIndex + size++, it ) );
	}
	m_baseVertexIndex += size;

	// and triangulate 'em
	triangulate( vertices, size );

}

template<typename PointIterator, typename MeshBuilder>
template<typename LoopIterator>
void Triangulator<PointIterator, MeshBuilder>::triangulate( LoopIterator first, LoopIterator last )
{
	unsigned nextBaseVertexIndex = m_baseVertexIndex;

	// put all the vertices of the outer loop into the builder,
	// and into the vertex list used for ear clipping
	const Loop &outer = *first;
	VertexList vertices;
	unsigned int size = 0;
	for( PointIterator it=outer.first; it!=outer.second; it++ )
	{
		m_builder->addVertex( *it, Point( 0 ) );
		vertices.push_back( Vertex( m_baseVertexIndex + size++, it ) );
	}
	nextBaseVertexIndex += size;

	// sort the holes by their maximum x coordinate
	/// \todo Need to sort by a different coordinate for polygons in yz plane
	typedef CircularIterator<PointIterator> CircularPointIterator;

	typedef std::multimap<BaseType, CircularPointIterator, std::greater<BaseType> > HoleMap;
	typedef typename HoleMap::value_type HoleMapValue;
	HoleMap holes;
	for( LoopIterator lIt=++first; lIt!=last; lIt++ )
	{
		BaseType m = Imath::limits<BaseType>::min();
		PointIterator mIt;
		for( PointIterator it=lIt->first; it!=lIt->second; it++ )
		{
			if( (*it)[0] > m )
			{
				m = (*it)[0];
				mIt = it;
			}
		}
		holes.insert( HoleMapValue( m, CircularPointIterator( lIt->first, lIt->second, mIt ) ) );
	}

	// now integrate the holes into the vertex list for the outer loop
	for( typename HoleMap::const_iterator it = holes.begin(); it!=holes.end(); it++ )
	{
		// search around the outer loop till we find a good point to connect
		// to the inner loop. we only start considering points when we've reached one which
		// is to the right of the inner point.
		const Point &innerPoint = *(it->second);
		VertexIterator joinIt;
		for( joinIt=vertices.begin(); joinIt!=vertices.end(); joinIt++ )
		{
			const Point &outerPoint = *(joinIt->second);
			if( outerPoint[0] <= innerPoint[0] )
			{
				continue;
			}

			const Edge joinEdge( innerPoint, outerPoint );
			// if the join edge intersects any of the existing edges then
			// we have to reject it.
			bool reject = false;
			CircularVertexIterator edgeStartIt( &vertices, joinIt );
			edgeStartIt++; // don't worry about the edge starting at the join point
			CircularVertexIterator edgeEndIt( edgeStartIt ); edgeEndIt++;
			do
			{
				Edge edge( *(edgeStartIt->second), *(edgeEndIt->second) );
				// i wonder if the tolerance below shouldn't be related to the
				// lengths of the line segments somehow.
				if( joinEdge.distanceTo( edge ) < 1e-3 )
				{
					reject = true;
					break;
				}
				edgeStartIt++;
				edgeEndIt++;
			} while( edgeEndIt!=joinIt ); // stop just before the edge ending at the join point

			if( !reject )
			{
				// we've found our join point
				break;
			}
		}

		// now integrate the hole into the outer loop
		VertexIterator insertPos = joinIt; insertPos++;
		CircularPointIterator holeIt = it->second;
		unsigned firstHoleVertIndex = m_baseVertexIndex + size;
		do {
			m_builder->addVertex( *holeIt, Point( 0 ) );
			vertices.insert( insertPos, Vertex( m_baseVertexIndex + size++, holeIt ) );
			nextBaseVertexIndex++;
			holeIt++;
		} while( holeIt!=it->second );
		vertices.insert( insertPos, Vertex( firstHoleVertIndex, it->second ) );
		vertices.insert( insertPos, *joinIt );

	}

	m_baseVertexIndex = nextBaseVertexIndex;

	// do the ear clipping
	triangulate( vertices, vertices.size() );
}

template<typename PointIterator, typename MeshBuilder>
void Triangulator<PointIterator, MeshBuilder>::triangulate( VertexList &vertices, unsigned size )
{

	// iterate round the vertex list clipping off ears as we go,
	// until we've clipped all the ears.
	////////////////////////////////////////////////////////////

	// a candidate for clipping
	CircularVertexIterator candidate( &vertices, ++vertices.begin() );
	// the point at which we've tried clipping all the vertices,
	// but found nothing suitable. at this point we have to just
	// clip anyway. this should only happen in the case of invalid input -
	// either the wrong winding order or intersecting edges.
	CircularVertexIterator failurePoint( &vertices, vertices.begin() );

	while( size>2 )
	{

		bool reject = false;
		// try to reject the candidate, unless we're about to fail
		// in which case we have to accept it anyway.
		if( candidate!=failurePoint )
		{

			CircularVertexIterator vIt = candidate;
			// the points and indices of the candidate triangle.
			// candidate actually points to the first vertex
			// but it's the second vertex we're trying to remove.
			const Point &v0 = *(vIt->second); unsigned int i0 = vIt->first; vIt++;
			const Point &v1 = *(vIt->second); unsigned int i1 = vIt->first; vIt++;
			const Point &v2 = *(vIt->second); unsigned int i2 = vIt->first; vIt++;

			if( (v0-v1).cross( v2-v1 ).z >= 0 )
			{
				// it's not a convex vertex so we can't clip.
				// this test assumes right handed winding order.
				reject = true;
			}
			else
			{
				// check to see if the triangle contains any of the remaining
				// vertices, and reject if it does
				while( vIt != candidate )
				{
					// the first test checks that the vertex we're testing for containment
					// isn't one of the vertices of the triangle. for single loops this
					// test isn't necessary, as we skipped past those above already. however,
					// when there are holes we introduce additional vertices which have the
					// same index into the original point list and we need to avoid those.
					if( vIt->first!=i0 && vIt->first!=i1 && vIt->first!=i2 )
					{
						if( triangleContainsPoint( v0, v1, v2, *(vIt->second) ) )
						{
							reject = true;
							break;
						}
					}
					vIt++;
				}

			}

		}

		// if we didn't reject it then clip it.
		if( !reject )
		{
			CircularVertexIterator triIt = candidate;
			unsigned int i0 = triIt->first; triIt++;
			unsigned int i1 = triIt->first; triIt = vertices.erase( triIt );
			unsigned int i2 = triIt->first;
			m_builder->addTriangle( i0, i1, i2 );
			failurePoint = candidate;
			size--;
		}

		candidate++;
	}

}

} // namespace IECore

#include "IECore/Triangulator.inl"

#endif // IECORE_TRIANGULATOR_INL
