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

#include <algorithm>
#include "OpenEXR/ImathLimits.h"
#include "IECore/VectorOps.h"

namespace IECore
{

template<class PointIterator>
inline bool KDTree<PointIterator>::Node::isLeaf() const
{
	return m_cutAxisAndLeaf==255;
}

template<class PointIterator>
inline PointIterator *KDTree<PointIterator>::Node::permFirst() const
{
	return m_perm.first;
}

template<class PointIterator>
inline PointIterator *KDTree<PointIterator>::Node::permLast() const
{
	return m_perm.last;
}

template<class PointIterator>
inline bool KDTree<PointIterator>::Node::isBranch() const
{
	return m_cutAxisAndLeaf!=255;
}

template<class PointIterator>
inline unsigned char KDTree<PointIterator>::Node::cutAxis() const
{
	return m_cutAxisAndLeaf;
}

template<class PointIterator>
inline typename KDTree<PointIterator>::BaseType KDTree<PointIterator>::Node::cutValue() const
{
	return m_cutValue;
}

template<class PointIterator>
inline void KDTree<PointIterator>::Node::makeLeaf( PermutationIterator permFirst, PermutationIterator permLast )
{
	m_cutAxisAndLeaf = 255;
	m_perm.first = &(*permFirst);
	m_perm.last = &(*permLast);
}

template<class PointIterator>
inline void KDTree<PointIterator>::Node::makeBranch( unsigned char cutAxis, BaseType cutValue )
{
	m_cutAxisAndLeaf = cutAxis;
	m_cutValue = cutValue;
}

template<class PointIterator>
class KDTree<PointIterator>::AxisSort
{
	public :
		AxisSort( unsigned int axis ) : m_axis( axis )
		{
		}

		bool operator() ( PointIterator i, PointIterator j )
		{
			return (*i)[m_axis] < (*j)[m_axis];
		}

	private :
		const unsigned int m_axis;
};

template<class PointIterator>
struct KDTree<PointIterator>::NearNeighbour
{
	NearNeighbour(PointIterator p, BaseType d) : m_point(p), m_distSqrd(d)
	{
	}

	bool operator < (const NearNeighbour &other) const
	{
		return m_distSqrd > other.m_distSqrd;
	}

	const PointIterator m_point;
	const BaseType m_distSqrd;
};


// initialisation

template<class PointIterator>
KDTree<PointIterator>::KDTree( PointIterator first, PointIterator last, int maxLeafSize )
	:	m_maxLeafSize( maxLeafSize ), m_lastPoint( last )
{
	m_perm.resize( last - first );
	unsigned int i=0;
	for( PointIterator it=first; it!=last; it++ )
	{
		m_perm[i++] = it;
	}

	build( rootIndex(), m_perm.begin(), m_perm.end() );
}

template<class PointIterator>
unsigned char KDTree<PointIterator>::majorAxis( PermutationConstIterator permFirst, PermutationConstIterator permLast )
{
	Point min, max;
	for( unsigned char i=0; i<VectorTraits<Point>::dimensions(); i++ ) {
		min[i] = Imath::limits<BaseType>::max();
		max[i] = Imath::limits<BaseType>::min();
	}
	for( PermutationConstIterator it=permFirst; it!=permLast; it++ )
	{
		for( unsigned char i=0; i<VectorTraits<Point>::dimensions(); i++ )
		{
			if( (**it)[i] < min[i] )
			{
				min[i] = (**it)[i];
			}
			if( (**it)[i] > max[i] )
			{
				max[i] = (**it)[i];
			}
		}
	}
	unsigned char major = 0;
	Point size = max - min;
	for( unsigned char i=1; i<VectorTraits<Point>::dimensions(); i++ )
	{
		if( size[i] > size[major] )
		{
			major = i;
		}
	}
	return major;
}

template<class PointIterator>
void KDTree<PointIterator>::build( NodeIndex nodeIndex, PermutationIterator permFirst, PermutationIterator permLast )
{
	// make room for the new node
	if( nodeIndex>=m_nodes.size() )
	{
		m_nodes.resize( nodeIndex+1 );
	}

	if( permLast - permFirst > m_maxLeafSize )
	{
		unsigned int cutAxis = majorAxis( permFirst, permLast );
		PermutationIterator permMid = permFirst  + (permLast - permFirst)/2;
		std::nth_element( permFirst, permMid, permLast, AxisSort( cutAxis ) );
		BaseType cutValue = (**permMid)[cutAxis];
		// insert node
		m_nodes[nodeIndex].makeBranch( cutAxis, cutValue );

		build( lowChildIndex( nodeIndex ), permFirst, permMid );
		build( highChildIndex( nodeIndex ), permMid, permLast );
	}
	else
	{
		// leaf node
		m_nodes[nodeIndex].makeLeaf( permFirst, permLast );
	}
}

// nearest neighbour searching

template<class PointIterator>
PointIterator KDTree<PointIterator>::nearestNeighbour( const Point &p ) const
{
	BaseType maxDistSquared = Imath::limits<BaseType>::max();
	PointIterator closestPoint = m_lastPoint;
	nearestNeighbourWalk( rootIndex(), p, closestPoint, maxDistSquared );
	return closestPoint;
}

template<class PointIterator>
PointIterator KDTree<PointIterator>::nearestNeighbour( const Point &p, BaseType &distSquared ) const
{
	PointIterator closestPoint = m_lastPoint;
	nearestNeighbourWalk( rootIndex(), p, closestPoint, distSquared );
	return closestPoint;
}

template<class PointIterator>
unsigned int KDTree<PointIterator>::nearestNeighbours( const Point &p, BaseType r, std::vector<PointIterator> &nearNeighbours ) const
{
	nearNeighbours.clear();

	nearestNeighboursWalk(rootIndex(), p, r*r, nearNeighbours );

	return nearNeighbours.size();
}

template<class PointIterator>
unsigned int KDTree<PointIterator>::nearestNNeighbours( const Point &p, unsigned int numNeighbours, std::vector<PointIterator> &nearNeighbours ) const
{
	nearNeighbours.clear();

	if (numNeighbours)
	{
		BaseType maxDistSquared = Imath::limits<BaseType>::max();

		std::set<NearNeighbour> neighbourSet;
		nearestNNeighboursWalk(rootIndex(), p, numNeighbours, neighbourSet, maxDistSquared );

		typename std::set<NearNeighbour>::const_iterator it = neighbourSet.begin();
		for (; it != neighbourSet.end(); ++it)
		{
			nearNeighbours.push_back( it->m_point );
		}
	}

	return nearNeighbours.size();
}

template<class PointIterator>
void KDTree<PointIterator>::nearestNeighbourWalk( NodeIndex nodeIndex, const Point &p, PointIterator &closestPoint, BaseType &distSquared ) const
{
	const Node &node = m_nodes[nodeIndex];
	if( node.isLeaf() )
	{
		PointIterator *permLast = node.permLast();
		for( PointIterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			const Point &pp = **perm;
			BaseType dist2 = vecDistance2( p, pp );

			if( dist2 < distSquared )
			{
				distSquared = dist2;
				closestPoint = *perm;
			}
		}
	}
	else
	{
		// node is a branch
		BaseType d = p[node.cutAxis()] - node.cutValue();
		NodeIndex firstChild, secondChild;
		if( d>0.0 )
		{
			firstChild = highChildIndex( nodeIndex );
			secondChild = lowChildIndex( nodeIndex );
		}
		else
		{
			firstChild = lowChildIndex( nodeIndex );
			secondChild = highChildIndex( nodeIndex );
		}

		nearestNeighbourWalk( firstChild, p, closestPoint, distSquared );
		if( d*d < distSquared )
		{
			nearestNeighbourWalk( secondChild, p, closestPoint, distSquared );
		}
	}
}

template<class PointIterator>
void KDTree<PointIterator>::nearestNeighboursWalk( NodeIndex nodeIndex, const Point &p, BaseType r2, std::vector<PointIterator> &nearNeighbours ) const
{
	const Node &node = m_nodes[nodeIndex];
	if( node.isLeaf() )
	{
		PointIterator *permLast = node.permLast();
		for( PointIterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			const Point &pp = **perm;
			BaseType dist2 = vecDistance2( p, pp );

			if (dist2 < r2 )
			{
				nearNeighbours.push_back( *perm );
			}
		}
	}
	else
	{
		// node is a branch
		BaseType d = p[node.cutAxis()] - node.cutValue();
		NodeIndex firstChild, secondChild;
		if( d>0.0 )
		{
			firstChild = highChildIndex( nodeIndex );
			secondChild = lowChildIndex( nodeIndex );
		}
		else
		{
			firstChild = lowChildIndex( nodeIndex );
			secondChild = highChildIndex( nodeIndex );
		}

		nearestNeighboursWalk( firstChild, p, r2, nearNeighbours );
		if( d*d < r2 )
		{
			nearestNeighboursWalk( secondChild, p, r2, nearNeighbours );
		}
	}
}

template<class PointIterator>
void KDTree<PointIterator>::nearestNNeighboursWalk( NodeIndex nodeIndex, const Point &p, unsigned int numNeighbours, std::set<NearNeighbour> &nearNeighbours, BaseType &maxDistSquared ) const
{
	const Node &node = m_nodes[nodeIndex];
	if( node.isLeaf() )
	{
		PointIterator *permLast = node.permLast();
		for( PointIterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			const Point &pp = **perm;
			BaseType dist2 = vecDistance2( p, pp );

			if( dist2 < maxDistSquared || nearNeighbours.size()<numNeighbours )
			{
				NearNeighbour n(*perm, dist2);
				assert(nearNeighbours.size() <= numNeighbours);

				typename std::set<NearNeighbour>::iterator it;
				if (nearNeighbours.size() == numNeighbours)
				{
					it = nearNeighbours.begin();

					// Make sure we're deleting the most distant point. Nodes are sorted in descending order.
					assert( it->m_distSqrd >= nearNeighbours.rbegin()->m_distSqrd );

					nearNeighbours.erase( it );
				}
				assert(nearNeighbours.size() < numNeighbours);
				nearNeighbours.insert( n );

				assert(nearNeighbours.size() > 0);

				// First element is furthest point away
				it = nearNeighbours.begin();
				assert( it->m_distSqrd >= nearNeighbours.rbegin()->m_distSqrd );
				maxDistSquared = it->m_distSqrd;
			}
		}
	}
	else
	{
		// node is a branch
		BaseType d = p[node.cutAxis()] - node.cutValue();
		NodeIndex firstChild, secondChild;
		if( d>0.0 )
		{
			firstChild = highChildIndex( nodeIndex );
			secondChild = lowChildIndex( nodeIndex );
		}
		else
		{
			firstChild = lowChildIndex( nodeIndex );
			secondChild = highChildIndex( nodeIndex );
		}

		nearestNNeighboursWalk( firstChild, p, numNeighbours, nearNeighbours, maxDistSquared );
		if( d*d < maxDistSquared || nearNeighbours.size()<numNeighbours )
		{
			nearestNNeighboursWalk( secondChild, p, numNeighbours, nearNeighbours, maxDistSquared );
		}
	}
}

template<class PointIterator>
inline const typename KDTree<PointIterator>::Node &KDTree<PointIterator>::node( NodeIndex index ) const
{
	return m_nodes[index];
}

template<class PointIterator>
inline typename KDTree<PointIterator>::NodeIndex KDTree<PointIterator>::rootIndex() const
{
	return 1;
}

template<class PointIterator>
inline typename KDTree<PointIterator>::NodeIndex KDTree<PointIterator>::lowChildIndex( NodeIndex parentIndex ) const
{
	return parentIndex * 2;
}

template<class PointIterator>
inline typename KDTree<PointIterator>::NodeIndex KDTree<PointIterator>::highChildIndex( NodeIndex parentIndex ) const
{
	return parentIndex * 2 + 1;
}

} // namespace IECore
