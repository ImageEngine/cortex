//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_MESHVERTEXREORDEROP_H
#define IECORE_MESHVERTEXREORDEROP_H

#include <set>
#include <vector>
#include <map>

#include "IECore/SimpleTypedParameter.h"
#include "IECore/TypedPrimitiveOp.h"

namespace IECore
{

/// A MeshPrimitiveOp to reorder the vertices of a mesh based on an initial selection of 3 vertices
class MeshVertexReorderOp : public MeshPrimitiveOp
{
	public:

		MeshVertexReorderOp();
		virtual ~MeshVertexReorderOp();

		IE_CORE_DECLARERUNTIMETYPED( MeshVertexReorderOp, MeshPrimitiveOp );
		
		V3iParameterPtr startingVerticesParameter();
		ConstV3iParameterPtr startingVerticesParameter() const;		

	protected:
	
		virtual void modifyTypedPrimitive( MeshPrimitivePtr mesh, ConstCompoundObjectPtr operands );
	
	private :
	
		V3iParameterPtr m_startingVerticesParameter;
	
		struct ReorderFn;
		struct HandleErrors;
		
		typedef int FaceId;
		typedef int EdgeId;
		typedef int VertexId;
		
		typedef std::pair< VertexId, VertexId > Edge;
		
		typedef std::vector< FaceId > FaceList;
		typedef std::set< FaceId > FaceSet;
		typedef std::vector< Edge > EdgeList;		
		typedef std::vector<VertexId> VertexList;
		
		typedef std::map< FaceId, EdgeList > FaceToEdgesMap;
		typedef std::map< FaceId, VertexList > FaceToVerticesMap;
		typedef std::map< VertexId, FaceSet > VertexToFacesMap;
		typedef std::map< Edge, FaceList > EdgeToConnectedFacesMap;
		
		FaceToEdgesMap m_faceToEdgesMap;
		FaceToVerticesMap m_faceToVerticesMap;
		EdgeToConnectedFacesMap m_edgeToConnectedFacesMap;
		VertexToFacesMap m_vertexToFacesMap;
		VertexList m_faceVaryingOffsets;
		int m_numFaces;
		int m_numVerts;
		
		void buildInternalTopology( ConstMeshPrimitivePtr mesh );
		
		int faceDirection( FaceId face, Edge edge );
				
		void visitFace(
			ConstMeshPrimitivePtr mesh,
			FaceId currentFace,
			Edge currentEdge,
			std::vector<VertexId> &vertexMap,
			std::vector<VertexId> &vertexRemap,			
			std::vector<int> &newVerticesPerFace,
			std::vector<VertexId> &newVertexIds,
			std::vector<int> &faceVaryingRemap,
			std::vector<int> &faceRemap,			
			int &nextVertex
		);
		
};

IE_CORE_DECLAREPTR( MeshVertexReorderOp );

} // namespace IECore

#endif // IECORE_MESHVERTEXREORDEROP_H

