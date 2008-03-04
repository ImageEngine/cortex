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

#ifndef IECORE_MESHPRIMITIVE_H
#define IECORE_MESHPRIMITIVE_H

#include "IECore/Primitive.h"
#include "IECore/VectorTypedData.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( MeshPrimitive )

/// MeshPrimitive represents a collection of closed simple polygonal facets
class MeshPrimitive : public Primitive
{	  
	
	public:

		IE_CORE_DECLAREOBJECT( MeshPrimitive, Primitive );

		/// Construct a MeshPrimitive with no faces.
		MeshPrimitive();
		/// Construct a MeshPrimitive. The number of faces specified by verticesPerFace->readable()->size().
		/// Copies of the IntVectorData objects are taken rather than references to the initial data.
		MeshPrimitive( ConstIntVectorDataPtr verticesPerFace, ConstIntVectorDataPtr vertexIds,
			const std::string &interpolation = "linear", V3fVectorDataPtr p = 0 );

		//! @name Topology access
		/// These functions allow access to get and set topology after construction.
		/////////////////////////////////////////////////////////////////////////////
		//@{
		ConstIntVectorDataPtr verticesPerFace() const;
		ConstIntVectorDataPtr vertexIds() const;
		const std::string &interpolation() const;
		virtual void setTopology( ConstIntVectorDataPtr verticesPerFace, ConstIntVectorDataPtr vertexIds, const std::string &interpolation = "linear" );
		//@}
	
		virtual size_t variableSize( PrimitiveVariable::Interpolation interpolation );

		/// Render the mesh
		virtual void render( RendererPtr renderer );

		/// Creates a custom box mesh.
		//\todo Add subdivisions and texture coordinates.
		static MeshPrimitivePtr createBox( Imath::Box3f b );
		/// Creates a plane at z=0.
		//\todo Add subdivisions and texture coordinates.
		static MeshPrimitivePtr createPlane( Imath::Box2f b );

	private:

		static const unsigned int m_ioVersion;

		IntVectorDataPtr m_verticesPerFace;
		IntVectorDataPtr m_vertexIds;
		size_t m_numVertices;
		std::string m_interpolation;
			
};  

}

#endif // IECORE_MESHPRIMITIVE
