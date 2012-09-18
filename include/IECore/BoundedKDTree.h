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

#ifndef IE_CORE_BOUNDEDKDTREE_H
#define IE_CORE_BOUNDEDKDTREE_H

#include <vector>

#include "OpenEXR/ImathBox.h"

#include "IECore/BoxTraits.h"

namespace IECore
{

/// Builds a KDTree of bounded volumes to permit fast intersection/overlap tests.
/// \ingroup mathGroup
template<class BoundIterator>
class BoundedKDTree
{
	public:

		typedef BoundIterator Iterator;
		typedef typename std::iterator_traits<BoundIterator>::value_type Bound;
		typedef typename BoxTraits<Bound>::BaseType BaseType;
		class Node;
		typedef std::vector<Node> NodeVector;
		typedef typename NodeVector::size_type NodeIndex;

		/// Construncts an uninitialised tree - you must call init() before
		/// using it.
		BoundedKDTree();

		/// Creates a tree for the fast searching of bounds.
		/// Note that the tree does not own the passed bounds -
		/// it is up to you to ensure that they remain valid and
		/// unchanged as long as the BoundedKDTree is in use.
		BoundedKDTree( BoundIterator first, BoundIterator last, int maxLeafSize=4 );

		/// Builds the tree for the specified bounds - the iterator range
		/// must remain valid and unchanged as long as the tree is in use.
		/// This method can be called again to rebuild the tree at any time.
		/// \threading This can't be called while other threads are
		/// making queries.
		void init( BoundIterator first, BoundIterator last, int maxLeafSize=4 );

		/// Populates the passed vector of iterators with the bounds which intersect "b". Returns the number of bounds found.
		/// \threading May be called by multiple concurrent threads provided they each use a different vector for the result.
		/// \todo There should be a form where nearNeighbours is an output iterator, to allow any container to be filled.
		template<typename S>
		unsigned int intersectingBounds( const S &b, std::vector<BoundIterator> &bounds ) const;

		/// Returns the number of nodes in the tree.
		inline NodeIndex numNodes() const;

		/// Retrieve the node associated with a given index
		const Node& node( NodeIndex idx ) const;
		
		/// Returns the index for the root node
		NodeIndex rootIndex() const;

		/// Retrieve the index of the "low" child node
		static NodeIndex lowChildIndex( NodeIndex index );

		/// Retrieve the index of the "high" child node
		static NodeIndex highChildIndex( NodeIndex index );

	private:

		typedef std::vector<BoundIterator> Permutation;
		typedef typename Permutation::iterator PermutationIterator;
		typedef typename Permutation::const_iterator PermutationConstIterator;

		class AxisSort;

		unsigned char majorAxis( PermutationConstIterator permFirst, PermutationConstIterator permLast );
		void build( NodeIndex nodeIndex, PermutationIterator permFirst, PermutationIterator permLast );
		void bound( NodeIndex nodeIndex );

		template<typename S>
		void intersectingBoundsWalk( NodeIndex nodeIndex, const S &p, std::vector<BoundIterator> &bounds ) const;

		Permutation m_perm;
		NodeVector m_nodes;
		int m_maxLeafSize;
		BoundIterator m_lastBound;
};

template<class BoundIterator>
class BoundedKDTree<BoundIterator>::Node
{
	public :

		/// Must be default constructible for use as element within std::vector
		Node();

		inline bool isLeaf() const;

		inline BoundIterator *permFirst() const;

		inline BoundIterator *permLast() const;

		inline bool isBranch() const;

		inline unsigned char cutAxis() const;

		inline const Bound &bound() const;

	private :

		friend class BoundedKDTree<BoundIterator>;

		inline void makeLeaf( PermutationIterator permFirst, PermutationIterator permLast );
		inline void makeBranch( unsigned char cutAxis );

		inline Bound &bound();

		unsigned char m_cutAxisAndLeaf;

		Bound m_bound;

		struct
		{
			/// \todo Could this be 0 for branch nodes? This could allow us to remove m_cutAxisAndLeaf.
			BoundIterator *first;
			/// \todo Could we just store an offset instead of last? If we limit the maxLeafSize to 255
			/// then this could be a single byte instead of 8 bytes.
			BoundIterator *last;
		} m_perm;
};

typedef BoundedKDTree<std::vector<Imath::Box2f>::const_iterator> Box2fTree;
typedef BoundedKDTree<std::vector<Imath::Box2d>::const_iterator> Box2dTree;
typedef BoundedKDTree<std::vector<Imath::Box3f>::const_iterator> Box3fTree;
typedef BoundedKDTree<std::vector<Imath::Box3d>::const_iterator> Box3dTree;

}

#include "BoundedKDTree.inl"

#endif // IE_CORE_BOUNDEDKDTREE_H
