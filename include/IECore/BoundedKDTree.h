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

#ifndef IE_CORE_BOUNDEDKDTREE_H
#define IE_CORE_BOUNDEDKDTREE_H

#include <vector>

#include "OpenEXR/ImathBox.h"

#include "IECore/BoxTraits.h"

namespace IECore
{

/// Builds a KDTree of bounded volumes to permit fast intersection/overlap tests.
template<class BoundIterator>
class BoundedKDTree
{
	public:
		typedef BoundIterator Iterator;
		typedef typename std::iterator_traits<BoundIterator>::value_type Bound;;
		typedef typename BoxTraits<Bound>::BaseType BaseType;		
		
		/// Creates a tree for the fast searching of bounds.
		/// Note that the tree does not own the passed bounds -
		/// it is up to you to ensure that they remain valid and
		/// unchanged as long as the BoundedKDTree is in use.
		BoundedKDTree( BoundIterator first, BoundIterator last, int maxLeafSize=4 );
		
		/// Populates the passed vector of iterators with the bounds which intersect "b". Returns the number of bounds found.		
		unsigned int intersectingBounds( const Bound &b, std::vector<BoundIterator> &bounds ) const;
				
	private :
		
		typedef std::vector<BoundIterator> Permutation;
		typedef typename Permutation::iterator PermutationIterator;
		typedef typename Permutation::const_iterator PermutationConstIterator;
		
		class Node;
		typedef std::vector<Node> NodeVector;
		typedef typename NodeVector::size_type NodeIndex;
		
		class AxisSort;
		
		unsigned char majorAxis( PermutationConstIterator permFirst, PermutationConstIterator permLast );
		void build( NodeIndex nodeIndex, PermutationIterator permFirst, PermutationIterator permLast );
		
		void intersectingBoundsWalk( NodeIndex nodeIndex, const Bound &b, std::vector<BoundIterator> &bounds ) const;
		
		Permutation m_perm;
		NodeVector m_nodes;
		const int m_maxLeafSize;
		const BoundIterator m_lastBound;
};

typedef BoundedKDTree<std::vector<Imath::Box2f>::const_iterator> Box2fTree;
typedef BoundedKDTree<std::vector<Imath::Box2d>::const_iterator> Box2dTree;
typedef BoundedKDTree<std::vector<Imath::Box3f>::const_iterator> Box3fTree;
typedef BoundedKDTree<std::vector<Imath::Box3d>::const_iterator> Box3dTree;

}

#include "BoundedKDTree.inl"

#endif // IE_CORE_BOUNDEDKDTREE_H
