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


#ifndef IE_CORE_MESHPRIMITIVEBUILDER_H
#define IE_CORE_MESHPRIMITIVEBUILDER_H

#include "IECore/VectorTypedData.h"
#include "IECore/MeshPrimitive.h"

namespace IECore
{

/// MeshPrimitiveBuilder is a class which allows incremental construction of IECore::MeshPrimitive meshes, templated
/// on the base type of the resulting point/normal data (e.g. float or double). Other builders could be created by
/// using this as a model. One possible client of this class is IECore::MarchingCubes.
/// \todo This is too specific to MarchingCubes. Some algorithms want to make faces other than triangles, and
/// some algorithms don't want to supply normals.
class MeshPrimitiveBuilder : public RefCounted
{
	public:
	
		IE_CORE_DECLAREMEMBERPTR( MeshPrimitiveBuilder );
	
		MeshPrimitiveBuilder();

		/// Add a vertex position and normal
		template<typename T>						
		void addVertex( const Imath::Vec3<T> &p, const Imath::Vec3<T> &n );

		/// Construct a triangle from the 3 specified vertex indices
		/// \todo Define and check winding order
		void addTriangle( int v0, int v1, int v2 );
				
		/// Retrieve the resultant mesh		
		MeshPrimitivePtr mesh() const;
		
	protected:
				
		V3fVectorDataPtr m_P;
		V3fVectorDataPtr m_N;		
		IntVectorDataPtr m_verticesPerFace;
		IntVectorDataPtr m_vertexIds;
};

IE_CORE_DECLAREPTR( MeshPrimitiveBuilder );     

}

#include "IECore/MeshPrimitiveBuilder.inl"

#endif
