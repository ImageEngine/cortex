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
#include <cassert>

#include "IECore/VectorTraits.h"

namespace IECore
{

template<class BoundIterator>
class BoundedKDTree<BoundIterator>::AxisSort
{
	public :
		AxisSort( unsigned int axis ) : m_axis( axis )
		{
		}
		
		bool operator() ( BoundIterator i, BoundIterator j )
		{
			return i->center()[m_axis] < j->center()[m_axis];
		}
		
	private :
		const unsigned int m_axis;
};

template<class BoundIterator>
class BoundedKDTree<BoundIterator>::Node
{
	public :
	
		Node() 
		{
			m_bound = Bound();
		}
		
		inline void makeLeaf( PermutationIterator permFirst, PermutationIterator permLast )
		{
			m_cutAxisAndLeaf = 255;
			m_perm.first = &(*permFirst);
			m_perm.last = &(*permLast);
		}
		
		inline void makeBranch( unsigned char cutAxis )
		{
			m_cutAxisAndLeaf = cutAxis;
		}
	
		inline bool isLeaf() const
		{
			return m_cutAxisAndLeaf==255;
		}
		
		inline BoundIterator *permFirst() const
		{
			assert( isLeaf() );
	
			return m_perm.first;
		}
		
		inline BoundIterator *permLast() const
		{
			assert( isLeaf() );
	
			return m_perm.last;
		}
				
		inline bool isBranch() const
		{
			return m_cutAxisAndLeaf!=255;
		}
		
		inline unsigned char cutAxis() const
		{
			assert( isBranch() );
	
			return m_cutAxisAndLeaf;
		}
		
		inline Bound &bound()
		{
			return m_bound;
		}
		
		inline Bound bound() const
		{
			return m_bound;
		}
	
		static NodeIndex rootIndex()
		{
			return 1;
		}
		
		static NodeIndex lowChildIndex( NodeIndex index )
		{
			return index * 2;
		}
		
		static NodeIndex highChildIndex( NodeIndex index )
		{
			return index * 2 + 1;
		}
		
	private :
		
		unsigned char m_cutAxisAndLeaf;
		
		Bound m_bound;
		
		struct {
			BoundIterator *first;
			BoundIterator *last;
		} m_perm;
		
};

template<class BoundIterator>
unsigned char BoundedKDTree<BoundIterator>::majorAxis( PermutationConstIterator permFirst, PermutationConstIterator permLast )
{
	BaseType min, max;
	for( unsigned char i=0; i<VectorTraits<BaseType>::dimensions(); i++ ) {
		min[i] = Imath::limits<typename BaseType::BaseType>::max();
		max[i] = Imath::limits<typename BaseType::BaseType>::min();
	}
	
	/// \todo Find a better cutting axis
	for( PermutationConstIterator it=permFirst; it!=permLast; it++ )
	{
		BaseType center = (*it)->center();
	
		for( unsigned char i=0; i<VectorTraits<BaseType>::dimensions(); i++ )
		{
			if( center[i] < min[i] )
			{
				min[i] = center[i];
			}
			if( center[i] > max[i] )
			{
				max[i] = center[i];
			}
		}
		
	}
	unsigned char major = 0;
	BaseType size = max - min;
	for( unsigned char i=1; i<VectorTraits<BaseType>::dimensions(); i++ )
	{
		if( size[i] > size[major] )
		{
			major = i;
		}
	}
	return major;
}

template<class BoundIterator>
void BoundedKDTree<BoundIterator>::bound( NodeIndex nodeIndex  )
{
	const Node &node = m_nodes[nodeIndex];
	if( node.isLeaf() )
	{
		BoundIterator *permLast = node.permLast();
		for( BoundIterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			node.bound().extendBy( **perm );
		}
	}
	else
	{	
		assert( node.isBranch() );
		
		bound( Node::lowChildIndex( nodeIndex ) );						
		bound( Node::highChildIndex( nodeIndex ) );
		node.bound().extendBy( m_nodes[Node::lowChildIndex( nodeIndex )].bound() );
		node.bound().extendBy( m_nodes[Node::highChildIndex( nodeIndex )].bound() );			
	}
}


template<class BoundIterator>
void BoundedKDTree<BoundIterator>::build( NodeIndex nodeIndex, PermutationIterator permFirst, PermutationIterator permLast )
{
	// make room for the new node
	if( nodeIndex>=m_nodes.size() )
	{
		m_nodes.resize( nodeIndex+1 );
	}
	
	Node &node = m_nodes[nodeIndex];
	
	if( permLast - permFirst > m_maxLeafSize )
	{
		unsigned int cutAxis = majorAxis( permFirst, permLast );
		PermutationIterator permMid = permFirst  + (permLast - permFirst)/2;
		std::nth_element( permFirst, permMid, permLast, AxisSort( cutAxis ) );

		// insert node
		node.makeBranch( cutAxis );
		
		build( Node::lowChildIndex( nodeIndex ), permFirst, permMid );						
		build( Node::highChildIndex( nodeIndex ), permMid, permLast );		
	}
	else
	{
		// leaf node
		node.makeLeaf( permFirst, permLast );
	}
}


template<class BoundIterator>
BoundedKDTree<BoundIterator>::BoundedKDTree( BoundIterator first, BoundIterator last, int maxLeafSize )
	:	m_maxLeafSize( maxLeafSize ), m_lastBound( last )
{
	m_perm.resize( last - first );
	unsigned int i=0;
	for( BoundIterator it=first; it!=last; it++ )
	{
		m_perm[i++] = it;
	}
	
	build( Node::rootIndex(), m_perm.begin(), m_perm.end() );
	bound( Node::rootIndex() );
}


template<class BoundIterator>	
unsigned int BoundedKDTree<BoundIterator>::intersectingBounds( const Bound &b, std::vector<BoundIterator> &bounds ) const
{	
	bounds.clear();
			
	intersectingBoundsWalk(Node::rootIndex(), b, bounds );
	
	return bounds.size();
}

template<class BoundIterator>	
void BoundedKDTree<BoundIterator>::intersectingBoundsWalk(  NodeIndex nodeIndex, const Bound &b, std::vector<BoundIterator> &bounds ) const
{
	const Node &node = m_nodes[nodeIndex];
	if( node.isLeaf() )
	{
		BoundIterator *permLast = node.permLast();
		for( BoundIterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			const Bound &bb = **perm;
			
			if ( bb.intersects(b) )
			{
				bounds.push_back( *perm );
			}
		}
	}
	else
	{	
		NodeIndex firstChild = Node::highChildIndex( nodeIndex );
		if ( m_nodes[firstChild].bound().intersects(b) )
		{
			intersectingBoundsWalk( firstChild, b, bounds );
		}
		NodeIndex secondChild = Node::lowChildIndex( nodeIndex );
		if ( m_nodes[secondChild].bound().intersects(b) )
		{
			intersectingBoundsWalk( secondChild, b, bounds );
		}
	}
}


} // namespace IECore
