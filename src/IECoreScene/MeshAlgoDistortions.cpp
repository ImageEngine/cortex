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

#include "IECoreScene/MeshAlgo.h"
#include "IECoreScene/PolygonIterator.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

//////////////////////////////////////////////////////////////////////////
// Calculate distortions
//////////////////////////////////////////////////////////////////////////

namespace
{

struct UVDistortion
{
	Imath::V2f distortion;
	int counter;

	UVDistortion() : distortion(0), counter(0) 	{};

	void accumulateDistortion( float dist, const Imath::V2f &uv )
	{
		distortion.x += fabs( uv.x ) * dist;
		distortion.y += fabs( uv.y ) * dist;
		counter++;
	}
};

struct VertexDistortion
{
	float distortion;
	int counter;

	VertexDistortion() : distortion(0), counter(0) 	{};

	void accumulateDistortion( float dist )
	{
		distortion += dist;
		counter++;
	}
};

std::pair<PrimitiveVariable, PrimitiveVariable> calculateDistortionInternal(
	const vector<int> &vertsPerFace,
	const vector<int> &vertIds,
	const vector<Imath::V3f> &p,
	const vector<Imath::V3f> &pRef,
	const vector<Imath::V2f> &uvs,
	const vector<int> &uvIds
)
{

	vector<VertexDistortion> distortions;
	distortions.resize( p.size() );

	int numUniqueTangents = 1 + *max_element( uvIds.begin(), uvIds.end() );
	vector<UVDistortion> uvDistortions;
	uvDistortions.resize( numUniqueTangents );

	size_t fvi0 = 0;

	for( size_t faceIndex = 0; faceIndex < vertsPerFace.size() ; faceIndex++ )
	{
		size_t firstFvi = fvi0;
		unsigned vertex0 = vertIds[ fvi0 ];
		Imath::V2f uv0( uvs[ fvi0 ] );

		for ( int v = 1; v <= vertsPerFace[faceIndex]; v++, fvi0++ )
		{
			size_t fvi1 = fvi0 + 1;
			if ( v == vertsPerFace[faceIndex] )
			{
				// final edge must also be computed...
				fvi1 = firstFvi;
			}
			unsigned vertex1 = vertIds[ fvi1 ];
			// compute distortion along the edge
			const V3f &p0 = p[ vertex0 ];
			const V3f &refP0 = pRef[ vertex0 ];
			const V3f &p1 = p[ vertex1 ];
			const V3f &refP1 = pRef[ vertex1 ];
			V3f edge = p1 - p0;
			V3f refEdge = refP1 - refP0;
			float edgeLen = edge.length();
			float refEdgeLen = refEdge.length();
			float distortion = 0;
			if ( edgeLen >= refEdgeLen )
			{
				distortion = fabs((edgeLen / refEdgeLen) - 1.0f);
			}
			else
			{
				distortion = -fabs( (refEdgeLen / edgeLen) - 1.0f );
			}

			// accumulate vertex distortions
			distortions[ vertex0 ].accumulateDistortion( distortion );
			distortions[ vertex1 ].accumulateDistortion( distortion );
			vertex0 = vertex1;

			// compute uv vector
			Imath::V2f uv1( uvs[ fvi1 ] );
			const Imath::V2f uvDir = (uv1 - uv0).normalized();
			// accumulate uv distortion
			uvDistortions[ uvIds[fvi0] ].accumulateDistortion( distortion, uvDir );
			uvDistortions[ uvIds[fvi1] ].accumulateDistortion( distortion, uvDir );
			uv0 = uv1;
		}
		fvi0 = firstFvi + vertsPerFace[faceIndex];
	}

	// normalize distortions and build output vectors

	// create the distortion prim var.
	FloatVectorDataPtr distortionData = new FloatVectorData();
	std::vector<float> &distortionVec = distortionData->writable();
	distortionVec.reserve( distortions.size() );
	for( auto &dist : distortions )
	{
		float invCounter = 0;
		if ( dist.counter )
		{
			invCounter = ( 1.0f / dist.counter );
		}
		distortionVec.push_back( dist.distortion * invCounter );
	}

	// create U and V distortions
	V2fVectorDataPtr uvDistortionData = new V2fVectorData();
	std::vector<Imath::V2f> &uvDistortionVec = uvDistortionData->writable();
	uvDistortionVec.reserve( uvIds.size() );
	for( auto &id : uvIds )
	{
		UVDistortion &uvDist = uvDistortions[id];
		if ( uvDist.counter )
		{
			uvDist.distortion /= (float)uvDist.counter;
			uvDist.counter = 0;
		}
		uvDistortionVec.push_back( uvDist.distortion );
	}

	return std::make_pair(
		PrimitiveVariable( PrimitiveVariable::Vertex, distortionData ),
		PrimitiveVariable( PrimitiveVariable::FaceVarying, uvDistortionData )
	);
}

} // namespace

std::pair<PrimitiveVariable, PrimitiveVariable> MeshAlgo::calculateDistortion( const MeshPrimitive *mesh, const std::string &uvSet, const std::string &referencePosition, const std::string &position )
{
	const V3fVectorData *pData = mesh->variableData<V3fVectorData>( position, PrimitiveVariable::Vertex );
	if( !pData )
	{
		string e = boost::str( boost::format( "MeshAlgo::calculateDistortion : MeshPrimitive has no suitable \"%s\" primitive variable." ) % position );
		throw InvalidArgumentException( e );
	}

	const V3fVectorData *pRefData = mesh->variableData<V3fVectorData>( referencePosition, PrimitiveVariable::Vertex );
	if( !pRefData )
	{
		string e = boost::str( boost::format( "MeshAlgo::calculateDistortion : MeshPrimitive has no suitable \"%s\" primitive variable." ) % referencePosition );
		throw InvalidArgumentException( e );
	}

	PrimitiveVariableMap::const_iterator uvIt = mesh->variables.find( uvSet );
	if( uvIt == mesh->variables.end() || uvIt->second.interpolation != PrimitiveVariable::FaceVarying || uvIt->second.data->typeId() != V2fVectorDataTypeId )
	{
		string e = boost::str( boost::format( "MeshAlgo::calculateDistortion : MeshPrimitive has no suitable \"%s\" primitive variable." ) % uvSet );
		throw InvalidArgumentException( e );
	}
	const V2fVectorData *uvData = runTimeCast<const V2fVectorData>( uvIt->second.data.get() );
	const IntVectorData *uvIndicesData = uvIt->second.indices ? uvIt->second.indices.get() : mesh->vertexIds();

	return calculateDistortionInternal(
		mesh->verticesPerFace()->readable(),
		mesh->vertexIds()->readable(),
		pData->readable(),
		pRefData->readable(),
		uvData->readable(),
		uvIndicesData->readable()
	);
}
