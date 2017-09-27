//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MeshAlgo.h"

using namespace Imath;
using namespace IECore;

//////////////////////////////////////////////////////////////////////////
// Calculate tangents
//////////////////////////////////////////////////////////////////////////

std::pair<PrimitiveVariable, PrimitiveVariable> IECore::MeshAlgo::calculateTangents(
	const MeshPrimitive *mesh,
	const std::string &uvSet, /* = "uv" */
	bool orthoTangents, /* = true */
	const std::string &position /* = "P" */
)
{
	if( mesh->minVerticesPerFace() != 3 || mesh->maxVerticesPerFace() != 3 )
	{
		throw InvalidArgumentException( "MeshAlgo::calculateTangents : MeshPrimitive must only contain triangles" );
	}

	const V3fVectorData *positionData = mesh->variableData<V3fVectorData>( position );
	if( !positionData )
	{
		std::string e = boost::str( boost::format( "MeshAlgo::calculateTangents : MeshPrimitive has no Vertex \"%s\" primitive variable." ) % position );
		throw InvalidArgumentException( e );
	}

	const V3fVectorData::ValueType &points = positionData->readable();

	const IntVectorData *vertsPerFaceData = mesh->verticesPerFace();
	const IntVectorData::ValueType &vertsPerFace = vertsPerFaceData->readable();

	const IntVectorData *vertIdsData = mesh->vertexIds();
	const IntVectorData::ValueType &vertIds = vertIdsData->readable();

	const auto uvIt = mesh->variables.find( uvSet );
	if( uvIt == mesh->variables.end() || uvIt->second.interpolation != PrimitiveVariable::FaceVarying || uvIt->second.data->typeId() != V2fVectorDataTypeId )
	{
		throw InvalidArgumentException( ( boost::format( "MeshAlgo::calculateTangents : MeshPrimitive has no FaceVarying V2fVectorData primitive variable named \"%s\"."  ) % ( uvSet ) ).str() );
	}

	const V2fVectorData *uvData = runTimeCast<V2fVectorData>( uvIt->second.data.get() );
	const V2fVectorData::ValueType &uvs = uvData->readable();

	// I'm a little unsure about using the vertIds as a fallback for the stIndices.
	const IntVectorData::ValueType &uvIndices = uvIt->second.indices ? uvIt->second.indices->readable() : vertIds;

	size_t numUVs = uvs.size();

	std::vector<V3f> uTangents( numUVs, V3f( 0 ) );
	std::vector<V3f> vTangents( numUVs, V3f( 0 ) );
	std::vector<V3f> normals( numUVs, V3f( 0 ) );

	for( size_t faceIndex = 0; faceIndex < vertsPerFace.size(); faceIndex++ )
	{
		assert( vertsPerFace[faceIndex] == 3 );

		// indices into the facevarying data for this face
		size_t fvi0 = faceIndex * 3;
		size_t fvi1 = fvi0 + 1;
		size_t fvi2 = fvi1 + 1;
		assert( fvi2 < vertIds.size() );
		assert( fvi2 < uvIndices.size() );

		// positions for each vertex of this face
		const V3f &p0 = points[vertIds[fvi0]];
		const V3f &p1 = points[vertIds[fvi1]];
		const V3f &p2 = points[vertIds[fvi2]];

		// uv coordinates for each vertex of this face
		const V2f &uv0 = uvs[uvIndices[fvi0]];
		const V2f &uv1 = uvs[uvIndices[fvi1]];
		const V2f &uv2 = uvs[uvIndices[fvi2]];

		// compute tangents and normal for this face
		const V3f e0 = p1 - p0;
		const V3f e1 = p2 - p0;

		const V2f e0uv = uv1 - uv0;
		const V2f e1uv = uv2 - uv0;

		V3f tangent = ( e0 * -e1uv.y + e1 * e0uv.y ).normalized();
		V3f bitangent = ( e0 * -e1uv.x + e1 * e0uv.x ).normalized();

		V3f normal = ( p2 - p1 ).cross( p0 - p1 );
		normal.normalize();

		// and accumlate them into the computation so far
		uTangents[uvIndices[fvi0]] += tangent;
		uTangents[uvIndices[fvi1]] += tangent;
		uTangents[uvIndices[fvi2]] += tangent;

		vTangents[uvIndices[fvi0]] += bitangent;
		vTangents[uvIndices[fvi1]] += bitangent;
		vTangents[uvIndices[fvi2]] += bitangent;

		normals[uvIndices[fvi0]] += normal;
		normals[uvIndices[fvi1]] += normal;
		normals[uvIndices[fvi2]] += normal;

	}

	// normalize and orthogonalize everything
	for( size_t i = 0; i < uTangents.size(); i++ )
	{
		normals[i].normalize();

		uTangents[i].normalize();
		vTangents[i].normalize();

		// Make uTangent/vTangent orthogonal to normal
		uTangents[i] -= normals[i] * uTangents[i].dot( normals[i] );
		vTangents[i] -= normals[i] * vTangents[i].dot( normals[i] );

		uTangents[i].normalize();
		vTangents[i].normalize();

		if( orthoTangents )
		{
			vTangents[i] -= uTangents[i] * vTangents[i].dot( uTangents[i] );
			vTangents[i].normalize();
		}

		// Ensure we have set of basis vectors (n, uT, vT) with the correct handedness.
		if( uTangents[i].cross( vTangents[i] ).dot( normals[i] ) < 0.0f )
		{
			uTangents[i] *= -1.0f;
		}
	}

	// convert the tangents back to facevarying data and add that to the mesh
	V3fVectorDataPtr fvUD = new V3fVectorData();
	V3fVectorDataPtr fvVD = new V3fVectorData();

	std::vector<V3f> &fvU = fvUD->writable();
	std::vector<V3f> &fvV = fvVD->writable();
	fvU.resize( uvIndices.size() );
	fvV.resize( uvIndices.size() );

	for( unsigned i = 0; i < uvIndices.size(); i++ )
	{
		fvU[i] = uTangents[uvIndices[i]];
		fvV[i] = vTangents[uvIndices[i]];
	}

	PrimitiveVariable tangentPrimVar( PrimitiveVariable::FaceVarying, fvUD );
	PrimitiveVariable bitangentPrimVar( PrimitiveVariable::FaceVarying, fvVD );

	return std::make_pair( tangentPrimVar, bitangentPrimVar );
}
