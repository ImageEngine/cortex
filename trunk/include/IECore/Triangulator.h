//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_TRIANGULATOR_H
#define IECORE_TRIANGULATOR_H

#include "IECore/VectorTypedData.h"
#include "IECore/MeshPrimitiveBuilder.h"
#include "IECore/VectorTraits.h"
#include "IECore/CircularIterator.h"
#include "IECore/LineSegment.h"

namespace IECore
{

/// The Triangulator template class performs triangulation of simple planar polygons. It
/// uses a MeshBuilder class to build the triangulated mesh.
template<typename PointIterator, typename MeshBuilder = MeshPrimitiveBuilder >
class Triangulator : public RefCounted
{
	public :

		typedef PointIterator Iterator;
		typedef typename std::iterator_traits<PointIterator>::value_type Point;
		typedef typename VectorTraits<Point>::BaseType BaseType;
		typedef MeshBuilder MeshBuilderType;
		/// A loop is defined by an iterator range from it's first
		/// to last point.
		typedef std::pair<PointIterator, PointIterator> Loop;

		IE_CORE_DECLAREMEMBERPTR( Triangulator );

		Triangulator( typename MeshBuilder::Ptr builder );
		virtual ~Triangulator();

		/// Triangulates the polygon specified by the points in the
		/// specified iterator range. Points must be planar and have
		/// an anticlockwise winding order.
		void triangulate( PointIterator first, PointIterator last );
		/// Triangulate the polygon with holes specified by the loops
		/// in the specified iterator range. The first loop defines the
		/// outer edges of the polygon, and must have an anticlockwise
		/// winding order. Additional loops define holes in the polygon
		/// and must have a clockwise winding order. The inner loops
		/// should not intersect each other or the outer loop.
		template<typename LoopIterator>
		void triangulate( LoopIterator first, LoopIterator last );

	private :

		/// Structures for iterating over a chain of points in
		/// a cyclic manner.
		/// The int is the index of the vertex in the original
		/// data, and the PointIterator points to the original
		/// point data too.
		typedef std::pair<unsigned int, PointIterator> Vertex;
		/// We use a list so we can remove vertices quickly
		/// as we clip ears.
		typedef std::list<Vertex> VertexList;
		/// Normal and circular iterators for the vertex lists.
		typedef typename VertexList::iterator VertexIterator;
		typedef CircularIterator<typename VertexList::iterator, VertexList> CircularVertexIterator;
		/// A line between two points.
		typedef LineSegment<Point> Edge;

		void triangulate( VertexList &vertices, unsigned size );

		typename MeshBuilder::Ptr m_builder;
		// the number of vertices already in the mesh at the start of each triangulate() call
		unsigned m_baseVertexIndex;

};

typedef Triangulator<std::vector<Imath::V2f>::const_iterator> V2fTriangulator;
typedef Triangulator<std::vector<Imath::V3f>::const_iterator> V3fTriangulator;
typedef Triangulator<std::vector<Imath::V2d>::const_iterator> V2dTriangulator;
typedef Triangulator<std::vector<Imath::V3d>::const_iterator> V3dTriangulator;

} // namespace IECore

#include "IECore/Triangulator.inl"

#endif // IECORE_TRIANGULATOR_H
