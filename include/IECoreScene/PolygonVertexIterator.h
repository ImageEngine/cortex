//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORESCENE_POLYGONVERTEXITERATOR_H
#define IECORESCENE_POLYGONVERTEXITERATOR_H

#include <vector>

namespace IECoreScene
{

/// An iterator type suitable for applying the PolygonAlgo.h algorithms to a MeshPrimitive.
/// VertexValueIterator is required to be a model of random_access_iterator. Generally it'll
/// be a std::vector::const_iterator instantiated from the Data of a PrimitiveVariable.
/// Generally PolygonVertexIterators aren't created directly, instead they'd be created with
/// the PolygonIterator::vertexBegin() and PolygonIterator::vertexEnd() methods.
/// \ingroup geometryProcessingGroup
template<typename VertexValueIterator, typename VertexIndexIterator=std::vector<int>::const_iterator>
class PolygonVertexIterator
{

	public :

		typedef std::forward_iterator_tag iterator_category;
		typedef typename VertexValueIterator::value_type value_type;
		typedef typename VertexValueIterator::difference_type difference_type;
		typedef typename VertexValueIterator::pointer pointer;
		typedef typename VertexValueIterator::reference reference;

		/// Uninitialised.
		PolygonVertexIterator();
		PolygonVertexIterator( VertexIndexIterator vertexIndexIterator, VertexValueIterator vertexValuesBegin );

		PolygonVertexIterator &operator++();
		PolygonVertexIterator operator++( int );

		reference operator*() const;
		pointer operator->() const;

		bool operator==( const PolygonVertexIterator &rhs ) const;
		bool operator!=( const PolygonVertexIterator &rhs ) const;

		PolygonVertexIterator &operator=( const PolygonVertexIterator &rhs );

	private :

		VertexValueIterator m_vertexValuesBegin;
		VertexIndexIterator m_vertexIndexIterator;

};

} // namespace IECoreScene

#include "IECoreScene/PolygonVertexIterator.inl"

#endif // IECORESCENE_POLYGONVERTEXITERATOR_H
