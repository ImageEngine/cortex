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

#ifndef IE_CORE_KDTREE_H
#define IE_CORE_KDTREE_H

#include <set>
#include <vector>

#include "OpenEXR/ImathVec.h"

#include "IECore/VectorTraits.h"

namespace IECore
{
	
/// The KDTree class provides accelerated searching of pointsets. It is
/// templated so that it can operate on a wide variety of datatypes, and uses
/// the VectorTraits.h and VectorOps.h functionality to assist in this.
template<class PointIterator>
class KDTree
{
	public :
		typedef PointIterator Iterator;
		typedef typename std::iterator_traits<PointIterator>::value_type Point;
		typedef typename VectorTraits<Point>::BaseType BaseType;
	
		/// Creates a tree for the fast searching of points.
		/// Note that the tree does not own the passed points -
		/// it is up to you to ensure that they remain valid and
		/// unchanged as long as the KDTree is in use.
		KDTree( PointIterator first, PointIterator last, int maxLeafSize=4 );
	
		/// Returns an iterator to the nearest neighbour to the point p.
		PointIterator nearestNeighbour( const Point &p ) const;
		/// Returns an iterator to the nearest neighbour to the point p, and places
		/// the squared distance between these two points in distSquared. The initial value
		/// of distSquared constrains the search to return only points which are closer
		/// than sqrt(distSquared) to p. In the event of no such point being found, an iterator
		/// to the end of the points is returned (the "last" parameter which was passed to
		/// the constructor).
		PointIterator nearestNeighbour( const Point &p, BaseType &distSquared ) const;
	
		/// Populates the passed vector of iterators with the neighbours of point p which are closer than radius r. Returns the number of points found.
		unsigned int nearestNeighbours( const Point &p, BaseType r, std::vector<PointIterator> &nearNeighbours ) const;
		
		/// Populates the passed vector of iterators with the N closest neighbours to p. Returns the number of points found.
		unsigned int nearestNNeighbours( const Point &p, unsigned int numNeighbours, std::vector<PointIterator> &nearNeighbours ) const;
	
	private :
		
		typedef std::vector<PointIterator> Permutation;
		typedef typename Permutation::iterator PermutationIterator;
		typedef typename Permutation::const_iterator PermutationConstIterator;
	
		class Node;
		typedef std::vector<Node> NodeVector;
		typedef typename NodeVector::size_type NodeIndex;
	
		class AxisSort;
		
		unsigned char majorAxis( PermutationConstIterator permFirst, PermutationConstIterator permLast );
		void build( NodeIndex nodeIndex, PermutationIterator permFirst, PermutationIterator permLast );
		
		void nearestNeighbourWalk( NodeIndex nodeIndex, const Point &p, PointIterator &closestPoint, BaseType &distSquared ) const;
		
		void nearestNeighboursWalk( NodeIndex nodeIndex, const Point &p, BaseType r2, std::vector<PointIterator> &nearNeighbours ) const;
		
		struct NearNeighbour;
		
		void nearestNNeighboursWalk( NodeIndex nodeIndex, const Point &p, unsigned int numNeighbours, std::set<NearNeighbour> &nearNeighbours, BaseType &maxDistSquared ) const;
		
		Permutation m_perm;
		NodeVector m_nodes;
		const int m_maxLeafSize;
		PointIterator m_lastPoint;
	
};

typedef KDTree<std::vector<Imath::V2f>::const_iterator> V2fTree;
typedef KDTree<std::vector<Imath::V2d>::const_iterator> V2dTree;
typedef KDTree<std::vector<Imath::V3f>::const_iterator> V3fTree;
typedef KDTree<std::vector<Imath::V3d>::const_iterator> V3dTree;

} // namespace IECore

#include "KDTree.inl"

#endif // IE_CORE_KDTREE_H
