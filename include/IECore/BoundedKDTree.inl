//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include <algorithm>
#include <cassert>

#include "IECore/VectorTraits.h"
#include "IECore/VectorOps.h"
#include "IECore/BoxOps.h"

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
			return VectorTraits< BaseType >::get( boxCenter(*i), m_axis)
				< VectorTraits< BaseType >::get( boxCenter(*j), m_axis);
		}
		
	private :
		const unsigned int m_axis;
};


template<class BoundIterator>
BoundedKDTree<BoundIterator>::Node::Node() : m_cutAxisAndLeaf(0)
{
	BoxTraits<Bound>::makeEmpty( m_bound );
	m_perm.first = 0;
	m_perm.last = 0;			
}

template<class BoundIterator>
void BoundedKDTree<BoundIterator>::Node::makeLeaf( PermutationIterator permFirst, PermutationIterator permLast )
{
	m_cutAxisAndLeaf = 255;
	m_perm.first = &(*permFirst);
	m_perm.last = &(*permLast);
}

template<class BoundIterator>
void BoundedKDTree<BoundIterator>::Node::makeBranch( unsigned char cutAxis )
{
	m_cutAxisAndLeaf = cutAxis;
}

template<class BoundIterator>
bool BoundedKDTree<BoundIterator>::Node::isLeaf() const
{
	return m_cutAxisAndLeaf==255;
}

template<class BoundIterator>
BoundIterator *BoundedKDTree<BoundIterator>::Node::permFirst() const
{
	assert( isLeaf() );

	return m_perm.first;
}

template<class BoundIterator>
BoundIterator *BoundedKDTree<BoundIterator>::Node::permLast() const
{
	assert( isLeaf() );

	return m_perm.last;
}

template<class BoundIterator>
bool BoundedKDTree<BoundIterator>::Node::isBranch() const
{
	return m_cutAxisAndLeaf!=255;
}

template<class BoundIterator>
unsigned char BoundedKDTree<BoundIterator>::Node::cutAxis() const
{
	assert( isBranch() );

	return m_cutAxisAndLeaf;
}

template<class BoundIterator>
typename BoundedKDTree<BoundIterator>::Bound &BoundedKDTree<BoundIterator>::Node::bound()
{
	return m_bound;
}

template<class BoundIterator>
const typename BoundedKDTree<BoundIterator>::Bound &BoundedKDTree<BoundIterator>::Node::bound() const
{
	return m_bound;
}		
		
template<class BoundIterator>
unsigned char BoundedKDTree<BoundIterator>::majorAxis( PermutationConstIterator permFirst, PermutationConstIterator permLast )
{
	BaseType min, max;
	vecSetAll( min, Imath::limits<typename BaseType::BaseType>::max() );
	vecSetAll( max, Imath::limits<typename BaseType::BaseType>::min() );	
		
	for( PermutationConstIterator it=permFirst; it!=permLast; it++ )
	{
		BaseType center = boxCenter(**it);
	
		for( unsigned char i=0; i<VectorTraits<BaseType>::dimensions(); i++ )
		{
			if( VectorTraits<BaseType>::get(center, i) < VectorTraits<BaseType>::get(min, i) )
			{
				VectorTraits<BaseType>::set(min, i, VectorTraits<BaseType>::get(center, i) );
			} 
			if( VectorTraits<BaseType>::get(center, i) > VectorTraits<BaseType>::get(min, i) )
			{
				VectorTraits<BaseType>::set(max, i, VectorTraits<BaseType>::get(center, i) );
			}
		}
		
	}
	unsigned char major = 0;
	BaseType size;
	vecSub( max, min, size );
	for( unsigned char i=1; i<VectorTraits<BaseType>::dimensions(); i++ )
	{
		if( VectorTraits<BaseType>::get(size, i) > VectorTraits<BaseType>::get(size, major) )
		{
			major = i;
		}
	}
	return major;
}

template<class BoundIterator>
void BoundedKDTree<BoundIterator>::bound( NodeIndex nodeIndex  )
{
	assert( nodeIndex < m_nodes.size() );
	
	Node &node = m_nodes[nodeIndex];
	
	assert( BoxTraits<Bound>::isEmpty( node.bound() ) );
	
	if( node.isLeaf() )
	{
		BoundIterator *permLast = node.permLast();
		for( BoundIterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			boxExtend( node.bound(), **perm );
		}
	}
	else
	{	
		assert( node.isBranch() );
		
		assert( lowChildIndex( nodeIndex ) < m_nodes.size() );
		assert( highChildIndex( nodeIndex ) < m_nodes.size() );		
		
		bound( lowChildIndex( nodeIndex ) );						
		bound( highChildIndex( nodeIndex ) );
		boxExtend( node.bound(), m_nodes[lowChildIndex( nodeIndex )].bound() );
		boxExtend( node.bound(), m_nodes[highChildIndex( nodeIndex )].bound() );			
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
	
	assert( nodeIndex < m_nodes.size() );
	
	Node &node = m_nodes[nodeIndex];
	
	if( permLast - permFirst > m_maxLeafSize )
	{
		unsigned int cutAxis = majorAxis( permFirst, permLast );
		PermutationIterator permMid = permFirst  + (permLast - permFirst)/2;
		std::nth_element( permFirst, permMid, permLast, AxisSort( cutAxis ) );

		// insert node
		node.makeBranch( cutAxis );
		
		build( lowChildIndex( nodeIndex ), permFirst, permMid );						
		build( highChildIndex( nodeIndex ), permMid, permLast );		
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
		
	build( rootIndex(), m_perm.begin(), m_perm.end() );
	bound( rootIndex() );
}

template<class BoundIterator>
const typename BoundedKDTree<BoundIterator>::Node& BoundedKDTree<BoundIterator>::node( NodeIndex idx ) const
{
	assert( idx >= 0 );
	assert( idx < m_nodes.size() );

	return m_nodes[idx];
}

template<class BoundIterator>
typename BoundedKDTree<BoundIterator>::NodeIndex BoundedKDTree<BoundIterator>::rootIndex() const
{
	return 1;
}

template<class BoundIterator>
typename BoundedKDTree<BoundIterator>::NodeIndex BoundedKDTree<BoundIterator>::lowChildIndex( NodeIndex index )
{
	return index * 2;
}

template<class BoundIterator>
typename BoundedKDTree<BoundIterator>::NodeIndex BoundedKDTree<BoundIterator>::highChildIndex( NodeIndex index )
{
	return index * 2 + 1;
}

template<class BoundIterator>	
template<typename S>
unsigned int BoundedKDTree<BoundIterator>::intersectingBounds( const S &b, std::vector<BoundIterator> &bounds ) const
{	
	bounds.clear();
			
	intersectingBoundsWalk(rootIndex(), b, bounds );
	
	return bounds.size();
}

template<class BoundIterator>
template<typename S>	
void BoundedKDTree<BoundIterator>::intersectingBoundsWalk(  NodeIndex nodeIndex, const S &b, std::vector<BoundIterator> &bounds ) const
{
	const Node &node = m_nodes[nodeIndex];
	if( node.isLeaf() )
	{
		BoundIterator *permLast = node.permLast();
		for( BoundIterator *perm = node.permFirst(); perm!=permLast; perm++ )
		{
			const Bound &bb = **perm;
			
			if ( boxIntersects( bb, b ) )
			{
				bounds.push_back( *perm );
			}
		}
	}
	else
	{	
		NodeIndex firstChild = highChildIndex( nodeIndex );
		if ( boxIntersects( m_nodes[firstChild].bound(), b) )
		{
			intersectingBoundsWalk( firstChild, b, bounds );
		}
		NodeIndex secondChild = lowChildIndex( nodeIndex );
		if ( boxIntersects( m_nodes[secondChild].bound(), b) )
		{
			intersectingBoundsWalk( secondChild, b, bounds );
		}
	}
}

} // namespace IECore
