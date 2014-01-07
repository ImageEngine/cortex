//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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
/// \ingroup mathGroup
template<class PointIterator>
class KDTree
{
	public :

		typedef PointIterator Iterator;
		typedef typename std::iterator_traits<PointIterator>::value_type Point;
		typedef typename VectorTraits<Point>::BaseType BaseType;
		class Node;
		typedef std::vector<Node> NodeVector;
		typedef typename NodeVector::size_type NodeIndex;

		/// Constructs an unititialised tree - you must call init()
		/// before using it.
		KDTree();

		/// Creates a tree for the fast searching of points.
		/// Note that the tree does not own the passed points -
		/// it is up to you to ensure that they remain valid and
		/// unchanged as long as the KDTree is in use.
		KDTree( PointIterator first, PointIterator last, int maxLeafSize=4 );

		/// Builds the tree for the specified points - the iterator range
		/// must remain valid and unchanged as long as the tree is in use.
		/// This method can be called again to rebuild the tree at any time.
		/// \threading This can't be called while other threads are
		/// making queries.
		void init( PointIterator first, PointIterator last, int maxLeafSize=4  );

		/// Returns an iterator to the nearest neighbour to the point p.
		/// \threading May be called by multiple concurrent threads.
		PointIterator nearestNeighbour( const Point &p ) const;
		/// Returns an iterator to the nearest neighbour to the point p, and places
		/// the squared distance between these two points in distSquared. The initial value
		/// of distSquared constrains the search to return only points which are closer
		/// than sqrt(distSquared) to p. In the event of no such point being found, an iterator
		/// to the end of the points is returned (the "last" parameter which was passed to
		/// the constructor).
		/// \threading May be called by multiple concurrent threads.
		PointIterator nearestNeighbour( const Point &p, BaseType &distSquared ) const;

		/// Populates the passed vector of iterators with the neighbours of point p which are closer than radius r. Returns the number of points found.
		/// \todo There should be a form where nearNeighbours is an output iterator, to allow any container to be filled.
		/// See enclosedPoints for an example of this form.
		/// \threading May be called by multiple concurrent threads provided they are each using a different vector for the result.
		unsigned int nearestNeighbours( const Point &p, BaseType r, std::vector<PointIterator> &nearNeighbours ) const;
		
		class Neighbour;
		/// Populates the passed vector with the N closest neighbours to p, sorted with the closest first. Returns the number found.
		/// \threading May be called by multiple concurrent threads provided they are each using a different vector for the result.
		unsigned int nearestNNeighbours( const Point &p, unsigned int numNeighbours, std::vector<Neighbour> &nearNeighbours ) const;

		/// Finds all the points contained by the specified bound, outputting them to the specified iterator.
		/// \threading May be called by multiple concurrent threads.
		template<typename Box, typename OutputIterator>
		void enclosedPoints( const Box &bound, OutputIterator it ) const;

		/// Returns the number of nodes in the tree.
		inline NodeIndex numNodes() const;
		/// Returns the specified Node of the tree. See rootIndex(), lowChildIndex() and highChildIndex() for
		/// means of getting appropriate indices. This can be used to implement algorithms not provided as
		/// member functions.
		inline const Node &node( NodeIndex index ) const;
		/// Returns the index for the root node.
		inline NodeIndex rootIndex() const;
		/// Returns the index for the "low" child of the specified Node. This will only
		/// be valid if parent.isBranch() is true.
		inline NodeIndex lowChildIndex( NodeIndex parentIndex ) const;
		/// Returns the index for the "high" child of the specified Node. This will only
		/// be valid if parent.isBranch() is true.
		inline NodeIndex highChildIndex( NodeIndex parentIndex ) const;

	private :

		typedef std::vector<PointIterator> Permutation;
		typedef typename Permutation::iterator PermutationIterator;
		typedef typename Permutation::const_iterator PermutationConstIterator;

		class AxisSort;

		unsigned char majorAxis( PermutationConstIterator permFirst, PermutationConstIterator permLast );
		void build( NodeIndex nodeIndex, PermutationIterator permFirst, PermutationIterator permLast );

		void nearestNeighbourWalk( NodeIndex nodeIndex, const Point &p, PointIterator &closestPoint, BaseType &distSquared ) const;

		void nearestNeighboursWalk( NodeIndex nodeIndex, const Point &p, BaseType r2, std::vector<PointIterator> &nearNeighbours ) const;

		template<typename Box, typename OutputIterator>
		void enclosedPointsWalk( NodeIndex nodeIndex, const Box &bound, OutputIterator it ) const;
	
		void nearestNNeighboursWalk( NodeIndex nodeIndex, const Point &p, unsigned int numNeighbours, std::vector<Neighbour> &nearNeighbours, BaseType &maxDistSquared ) const;

		Permutation m_perm;
		NodeVector m_nodes;
		int m_maxLeafSize;
		PointIterator m_lastPoint;

};

/// The Node class which is used to implement the branching structure in the KDTree.
template<class PointIterator>
class KDTree<PointIterator>::Node
{
	public :

		/// Returns true if this is a leaf node of the tree.
		inline bool isLeaf() const;
		/// Returns a pointer to an iterator referencing the first
		/// child of this Node. Only valid if isLeaf() is true.
		inline PointIterator *permFirst() const;
		/// Returns a pointer to an iterator referencing the last
		/// child of this Node. Only valid if isLeaf() is true.
		inline PointIterator *permLast() const;
		/// Returns true if this is a branch node of the tree;
		inline bool isBranch() const;
		/// Returns the axis in which this node cuts the space. Only
		/// valid if isBranch() is true.
		inline unsigned char cutAxis() const;
		/// Returns the point within cutAxis() at which the node
		/// cuts the space.
		inline BaseType cutValue() const;

	private :

		friend class KDTree<PointIterator>;

		inline void makeLeaf( PermutationIterator permFirst, PermutationIterator permLast );
		inline void makeBranch( unsigned char cutAxis, BaseType cutValue );

		unsigned char m_cutAxisAndLeaf;
		union {
			BaseType m_cutValue;
			struct {
				PointIterator *first;
				/// \todo Could we just store an offset instead of last? If we limit the maxLeafSize to 255
				/// then this could be a single byte instead of 8 bytes.
				PointIterator *last;
			} m_perm;
		};

};

/// The Neighbour class is used to return information from the KDTree::nearestNNeighbours() query.
template<class PointIterator>
class KDTree<PointIterator>::Neighbour
{
	public :

		Neighbour( Iterator p, BaseType d2 )
			:	point( p ), distSquared( d2 )
		{
		}
		
		Iterator point;
		BaseType distSquared;
	
		bool operator < ( const Neighbour &other ) const
		{
			return distSquared < other.distSquared;
		}
		
};

typedef KDTree<std::vector<Imath::V2f>::const_iterator> V2fTree;
typedef KDTree<std::vector<Imath::V2d>::const_iterator> V2dTree;
typedef KDTree<std::vector<Imath::V3f>::const_iterator> V3fTree;
typedef KDTree<std::vector<Imath::V3d>::const_iterator> V3dTree;

} // namespace IECore

#include "KDTree.inl"

#endif // IE_CORE_KDTREE_H
