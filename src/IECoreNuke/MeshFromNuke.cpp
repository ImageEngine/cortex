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

#include "IECoreNuke/MeshFromNuke.h"
#include "IECoreNuke/Convert.h"

#include "IECore/MeshPrimitive.h"

#include <algorithm>

using namespace IECoreNuke;
using namespace IECore;

MeshFromNuke::MeshFromNuke( const DD::Image::GeoInfo *geo )
	:	FromNukeConverter( "MeshFromNuke", "Converts nuke meshes to IECore meshes." ), m_geo( geo )
{
}

MeshFromNuke::~MeshFromNuke()
{
}

IECore::ObjectPtr MeshFromNuke::doConversion( IECore::ConstCompoundObjectPtr operands ) const
{
	IntVectorDataPtr verticesPerFaceData = new IntVectorData;
	IntVectorDataPtr vertexIdsData = new IntVectorData;
	std::vector<int> &verticesPerFace = verticesPerFaceData->writable();
	std::vector<int> &vertexIds = vertexIdsData->writable();
		
	unsigned numPrimitives = m_geo->primitives();
	const DD::Image::Primitive **primitives = m_geo->primitive_array();
	std::vector<unsigned> tmpFaceVertices;
	for( unsigned primIndex=0; primIndex<numPrimitives; primIndex++ )
	{
		const DD::Image::Primitive *prim = primitives[primIndex];
	
		unsigned numFaces = prim->faces();
		for( unsigned faceIndex=0; faceIndex<numFaces; faceIndex++ )
		{
			unsigned numFaceVertices = prim->face_vertices( faceIndex );
			verticesPerFace.push_back( numFaceVertices );
			tmpFaceVertices.resize( numFaceVertices );
			prim->get_face_vertices( faceIndex, &(tmpFaceVertices[0]) );
			for( unsigned i=0; i<numFaceVertices; i++ )
			{
				vertexIds.push_back( prim->vertex( tmpFaceVertices[i] ) );
			}
		}
	}
	
	MeshPrimitivePtr result = new MeshPrimitive( verticesPerFaceData, vertexIdsData, "linear" );
	
	if( const DD::Image::PointList *pl = m_geo->point_list() )
	{
		V3fVectorDataPtr p = new V3fVectorData();
		p->writable().resize( pl->size() );
		std::transform( pl->begin(), pl->end(), p->writable().begin(), IECore::convert<Imath::V3f, DD::Image::Vector3> );
		result->variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, p );
	}

	return result;
}
