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

#include <cassert>

#include "IECore/BoxTraits.h"

template<typename P, typename V>	
BlobbyImplicitSurfaceFunction<P,V>::BlobbyImplicitSurfaceFunction( typename PointVectorData::ConstPtr p, ConstDoubleVectorDataPtr r, ConstDoubleVectorDataPtr s ) : m_p( p ), m_radius( r ), m_strength( s )
{		
	assert( m_p );
	assert( m_radius );
	assert( m_strength );

	if (m_p->readable().size() != m_radius->readable().size())
	{
		throw InvalidArgumentException("Incompatible point/radius data given to BlobbyImplicitSurfaceFunction");
	}
	if (m_p->readable().size() != m_strength->readable().size())
	{
		throw InvalidArgumentException("Incompatible point/strength data given to BlobbyImplicitSurfaceFunction");
	}

	typename PointVector::const_iterator pit = m_p->readable().begin();

	std::vector<double>::const_iterator rit = m_radius->readable().begin();			

	for (; pit != m_p->readable().end(); ++pit, ++rit)
	{
		Point boundMin, boundMax, boundRadius;
		
		vecSetAll( boundRadius, *rit );
		vecSub( *pit, boundRadius, boundMin );
		vecAdd( *pit, boundRadius, boundMax );		
		
		m_bounds.push_back( BoxTraits<Bound>::create( boundMin, boundMax ) );
	}

	assert( m_bounds.size() == m_p->readable().size() );

	m_tree = new Tree( m_bounds.begin(), m_bounds.end() );

	assert( m_tree );			
}
		
template<typename P, typename V>	
BlobbyImplicitSurfaceFunction<P,V>::~BlobbyImplicitSurfaceFunction()
{
	assert( m_tree );
	delete m_tree;
}

template<typename P, typename V>	
typename BlobbyImplicitSurfaceFunction<P,V>::Value BlobbyImplicitSurfaceFunction<P,V>::operator()( const Point &p )
{
	assert( m_tree );
	assert( m_p );
	assert( m_radius );
	assert( m_strength );

	std::vector<BoundVectorConstIterator> intersecting;

	m_tree->intersectingBounds( p, intersecting );

	Value totalInfluence = -Imath::limits<Value>::epsilon();

	for (typename std::vector<BoundVectorConstIterator>::iterator it = intersecting.begin(); it != intersecting.end(); ++it)
	{
		const int boundIndex = std::distance<BoundVectorConstIterator>( m_bounds.begin(), *it );
		assert( boundIndex >= 0 );
		assert( boundIndex < (int)m_bounds.size() );		

		Point sep;
		vecSub( m_p->readable()[ boundIndex ] , p, sep );
		PointBaseType distSqrd = vecDot( sep, sep );
		
		const Value &b = m_radius->readable()[ boundIndex ];	

		/// Osaka University's original "metaballs" function, cheaper than Blinn's s*exp(-b*r*r)
		PointBaseType dist = sqrt( distSqrd );
		if ( dist < b / 3.0 )
		{
			totalInfluence += m_strength->readable()[ boundIndex ] * ( 1.0 - 3.0 * distSqrd / ( b * b ) );
		}
		else if (dist < b)
		{
			totalInfluence += 1.5 * m_strength->readable()[ boundIndex ] * ( 1.0 - dist / b ) * ( 1.0 - dist / b );
		}
	
	}

	return totalInfluence;
}

template<typename P, typename V>			
typename BlobbyImplicitSurfaceFunction<P,V>::Value BlobbyImplicitSurfaceFunction<P,V>::getValue( const Point &p )
{
	return this->operator()(p);
}
