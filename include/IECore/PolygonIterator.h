//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_POLYGONITERATOR_H
#define IECORE_POLYGONITERATOR_H

#include "boost/iterator/counting_iterator.hpp"

#include "IECore/Export.h"
#include "IECore/PolygonVertexIterator.h"

namespace IECore
{

/// An iterator type suitable for iterating over the faces of a MeshPrimitive. Generally
/// these wouldn't be created directly, instead being created by calls to MeshPrimitive::faceBegin() and MeshPrimitive::faceEnd().
/// \ingroup geometryProcessingGroup
class IECORE_API PolygonIterator
{

	public :

		/// An iterator for the vector returned by MeshPrimitive::vertexIds()->readable()
		typedef std::vector<int>::const_iterator NumVerticesIterator;
		typedef std::vector<int>::const_iterator VertexIndexIterator;

		/// Uninitialised.
		inline PolygonIterator();
		inline PolygonIterator( NumVerticesIterator numVerticesIterator, VertexIndexIterator vertexIndexIterator, int faceVaryingOffset );
	
		inline PolygonIterator &operator++();
		inline PolygonIterator operator++( int );

		inline bool operator==( const PolygonIterator &rhs ) const;
		inline bool operator!=( const PolygonIterator &rhs ) const;

		inline PolygonIterator &operator=( const PolygonIterator &rhs );
		
		/// Returns an iterator to the beginning of the range of vertex interpolated values
		/// for this polygon. Typically you should pass PrimitiveVariable::data::readable()::begin()
		/// for the primitive variable you're interested in.
		template<typename ValueIterator>
		PolygonVertexIterator<ValueIterator> vertexBegin( ValueIterator valuesBegin ) const;
		/// Returns the matching end of range iterator for vertexBegin.
		template<typename ValueIterator>
		PolygonVertexIterator<ValueIterator> vertexEnd( ValueIterator valuesBegin ) const;
		
		/// Returns an iterator to the beginning of the range of facevarying interpolated
		/// values for this polygon. Typically you should pass PrimitiveVariable::data::readable()::begin()
		/// for the primitive variable you're interested in.
		template<typename ValueIterator>
		ValueIterator faceVaryingBegin( ValueIterator valuesBegin ) const;
		/// Returns the matching end of range iterator for faceVaryingBegin.
		template<typename ValueIterator>
		ValueIterator faceVaryingEnd( ValueIterator valuesBegin ) const;
		
	private :

		typedef boost::counting_iterator<int> FaceVaryingVertexIndexIterator;
		
		NumVerticesIterator m_numVerticesIterator;
		VertexIndexIterator m_vertexIndexIterator;
		int m_faceVaryingOffset;
		
};

} // namespace IECore

#include "IECore/PolygonIterator.inl"

#endif // IECORE_POLYGONITERATOR_H
