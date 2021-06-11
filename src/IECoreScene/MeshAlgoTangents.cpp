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
#include "IECore/DataAlgo.h"

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

//////////////////////////////////////////////////////////////////////////
// Calculate tangents
//////////////////////////////////////////////////////////////////////////

namespace
{

struct Basis
{
	V3f tangent;
	V3f bitangent;
	V3f normal;
};

//! Calculate the directions of the U and V directions in provided *world* space
void calculcateBasis( const V3f &p0, const V3f &p1, const V3f &p2, const V2f &uv0, const V2f &uv1, const V2f &uv2, Basis &outBasis )
{
	// compute tangents and normal for this *triangle*
	const V3f e0 = p1 - p0;
	const V3f e1 = p2 - p0;

	const V2f e0uv = uv1 - uv0;
	const V2f e1uv = uv2 - uv0;

	outBasis.tangent = ( e0 * -e1uv.y + e1 * e0uv.y ).normalized();
	outBasis.bitangent = ( e0 * -e1uv.x + e1 * e0uv.x ).normalized();

	outBasis.normal = ( p2 - p1 ).cross( p0 - p1 );
	outBasis.normal.normalize();
}

} // namespace

std::pair<PrimitiveVariable, PrimitiveVariable> IECoreScene::MeshAlgo::calculateTangents(
	const MeshPrimitive *mesh,
	const std::string &uvSet, /* = "uv" */
	bool orthoTangents, /* = true */
	const std::string &position /* = "P" */
)
{
	return calculateTangentsFromUV( mesh, uvSet, position, orthoTangents, false );
}

std::pair<PrimitiveVariable, PrimitiveVariable> IECoreScene::MeshAlgo::calculateTangentsFromUV(
	const MeshPrimitive *mesh,
	const std::string &uvSet, /* = "uv" */
	const std::string &position /* = "P" */,
	bool orthoTangents, /* = true */
	bool leftHanded,
	const Canceller *canceller
)
{
	const V3fVectorData *positionData = mesh->variableData<V3fVectorData>( position );
	if( !positionData )
	{
		std::string e = boost::str( boost::format( "MeshAlgo::calculateTangentsFromUV : MeshPrimitive has no Vertex \"%s\" primitive variable." ) % position );
		throw InvalidArgumentException( e );
	}

	const V3fVectorData::ValueType &points = positionData->readable();

	const IntVectorData *vertsPerFaceData = mesh->verticesPerFace();
	const IntVectorData::ValueType &vertsPerFace = vertsPerFaceData->readable();

	const IntVectorData *vertIdsData = mesh->vertexIds();
	const IntVectorData::ValueType &vertIds = vertIdsData->readable();

	const auto uvIt = mesh->variables.find( uvSet );
	if( uvIt == mesh->variables.end() ||  uvIt->second.data->typeId() != V2fVectorDataTypeId )
	{
		throw InvalidArgumentException( ( boost::format( "MeshAlgo::calculateTangentsFromUV : MeshPrimitive has no V2fVectorData primitive variable named \"%s\"."  ) % ( uvSet ) ).str() );
	}

	const std::vector<Imath::V2f> *uvData = nullptr;
	const std::vector<int> *uvIndices = nullptr;

	std::vector<int> tmpIndices;

	PrimitiveVariable::Interpolation uvInterpolation = uvIt->second.interpolation;
	if( uvInterpolation == IECoreScene::PrimitiveVariable::Interpolation::FaceVarying )
	{
		uvData = &runTimeCast<const IECore::V2fVectorData>( uvIt->second.data )->readable();
		if ( uvIt->second.indices )
		{
			uvIndices = &uvIt->second.indices->readable();
		}
	}
	else if( uvInterpolation == IECoreScene::PrimitiveVariable::Interpolation::Vertex ||
		uvIt->second.interpolation == IECoreScene::PrimitiveVariable::Interpolation::Varying )
	{
		uvData = &runTimeCast<const IECore::V2fVectorData>( uvIt->second.data )->readable();

		if( uvIt->second.indices )
		{
			Canceller::check( canceller );
			tmpIndices.reserve( mesh->vertexIds()->readable().size() );

			Canceller::check( canceller );
			for( auto vertexId : mesh->vertexIds()->readable() )
			{
				tmpIndices.push_back( uvIt->second.indices->readable()[vertexId] );
			}

			uvIndices = &tmpIndices;
		}
		else
		{
			uvIndices = &mesh->vertexIds()->readable();
		}
	}
	else
	{
		throw InvalidArgumentException(
			(
				boost::format(
					"MeshAlgo::calculateTangentsFromUV : MeshPrimitive primitive variable named \"%s\"  has incorrect interpolation, must be either Vertex or FaceVarying"
				) % ( uvSet )
			).str()
		);
	}

	PrimitiveVariable::IndexedView<V2f> uvIndexedView( *uvData, uvIndices );

	size_t numUVs = IECore::size( uvIt->second.data.get() );

	Canceller::check( canceller );
	std::vector<V3f> uTangents( numUVs, V3f( 0 ) );
	Canceller::check( canceller );
	std::vector<V3f> vTangents( numUVs, V3f( 0 ) );
	Canceller::check( canceller );
	std::vector<V3f> normals( numUVs, V3f( 0 ) );

	size_t vertStart = 0;
	for( size_t faceIndex = 0; faceIndex < vertsPerFace.size(); faceIndex++ )
	{
		if( ( faceIndex % 1000 ) == 0 )
		{
			Canceller::check( canceller );
		}

		for ( size_t faceVertIndex = 0; faceVertIndex < (size_t)vertsPerFace[faceIndex]; ++faceVertIndex)
		{
			// indices into the facevarying data for this *triangle*
			size_t fvi0 = vertStart + faceVertIndex;
			size_t fvi1 = vertStart + (faceVertIndex + 1) % vertsPerFace[faceIndex];
			size_t fvi2 = vertStart + (faceVertIndex + 2) % vertsPerFace[faceIndex];

			assert( fvi0 < vertIds.size() );
			assert( fvi0 < uvIndexedView.size() );

			assert( fvi1 < vertIds.size() );
			assert( fvi1 < uvIndexedView.size() );

			assert( fvi2 < vertIds.size() );
			assert( fvi2 < uvIndexedView.size() );

			// positions for each vertex of this face
			const V3f &p0 = points[vertIds[fvi0]];
			const V3f &p1 = points[vertIds[fvi1]];
			const V3f &p2 = points[vertIds[fvi2]];

			// uv coordinates for each vertex of this face
			const V2f &uv0 = uvIndexedView[fvi0];
			const V2f &uv1 = uvIndexedView[fvi1];
			const V2f &uv2 = uvIndexedView[fvi2];

			Basis basis;
			calculcateBasis( p0, p1, p2, uv0, uv1, uv2, basis );

			// and accumulate them into the computation so far
			uTangents[uvIndexedView.index(fvi0)] += basis.tangent;
			vTangents[uvIndexedView.index(fvi0)] += basis.bitangent;
			normals[uvIndexedView.index(fvi0)] += basis.normal;
		}

		vertStart += vertsPerFace[faceIndex];
	}

	// normalize and orthogonalize everything
	for( size_t i = 0; i < uTangents.size(); i++ )
	{
		if( ( i % 1000 ) == 0 )
		{
			Canceller::check( canceller );
		}

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
		if ( !leftHanded )
		{
			if( uTangents[i].cross( vTangents[i] ).dot( normals[i] ) < 0.0f )
			{
				uTangents[i] *= -1.0f;
			}
		}
		else
		{
			if( uTangents[i].cross( vTangents[i] ).dot( normals[i] ) > 0.0f )
			{
				uTangents[i] *= -1.0f;
			}
		}

	}

	// convert the tangents back to facevarying data and add that to the mesh
	V3fVectorDataPtr fvUD = new V3fVectorData( uTangents );
	V3fVectorDataPtr fvVD = new V3fVectorData( vTangents );

	IntVectorDataPtr indices;

	if ( uvIt->second.indices )
	{
		indices = uvIt->second.indices;
	}

	PrimitiveVariable tangentPrimVar( uvInterpolation, fvUD, indices );
	PrimitiveVariable biTangentPrimVar( uvInterpolation, fvVD, indices );

	return std::make_pair( tangentPrimVar, biTangentPrimVar );
}

std::pair<PrimitiveVariable, PrimitiveVariable> IECoreScene::MeshAlgo::calculateTangentsFromPrimitiveCentroid(
	const MeshPrimitive *mesh,
	const std::string &position /* = "P" */,
	const std::string &normal /* = "N" */,
	bool orthoTangents,
	bool leftHanded,
	const Canceller *canceller
)
{
	// get point and normal data
	const V3fVectorData *positionData = mesh->variableData<V3fVectorData>( position );
	const V3fVectorData *normalData = mesh->variableData<V3fVectorData>( normal );
	if( !positionData )
	{
		std::string e = boost::str( boost::format( "MeshAlgo::calculateTangentsFromPrimitiveCentroid : MeshPrimitive has no Vertex \"%s\" primitive variable." ) % position );
		throw InvalidArgumentException( e );
	}
	if( !normalData )
	{
		std::string e = boost::str( boost::format( "MeshAlgo::calculateTangentsFromPrimitiveCentroid : MeshPrimitive has no Vertex \"%s\" primitive variable." ) % normal );
		throw InvalidArgumentException( e );
	}

	const auto normalIt = mesh->variables.find( normal );
	if( normalIt == mesh->variables.end() ||  normalIt->second.interpolation != IECoreScene::PrimitiveVariable::Interpolation::Vertex )
	{
		std::string e = boost::str( boost::format( "MeshAlgo::calculateTangentsFromPrimitiveCentroid : The normal primitive variable \"%s\" needs to be Vertex interpolated." ) % normal );
		throw InvalidArgumentException( e );
	}

	const V3fVectorData::ValueType &points = positionData->readable();
	const auto &normals = normalData->readable();

	int numPoints = points.size();
	Canceller::check( canceller );
	std::vector<V3f> tangents( numPoints, V3f( 0 ) );
	Canceller::check( canceller );
	std::vector<V3f> biTangents( numPoints, V3f( 0 ) );

	const IntVectorData *vertsPerFaceData = mesh->verticesPerFace();
	const IntVectorData::ValueType &vertsPerFace = vertsPerFaceData->readable();

	const IntVectorData *vertIdsData = mesh->vertexIds();
	const IntVectorData::ValueType &vertIds = vertIdsData->readable();

	Canceller::check( canceller );
	std::vector<V3f> centroids( vertsPerFace.size(), V3f( 0 ) );
	Canceller::check( canceller );
	std::vector<int> faceIdPerVert( numPoints, -1 );

	// calculate centroids
	// TODO: generalize this to MeshAlgo::calculateCentroid
	size_t vertStart = 0;
	for( size_t faceIndex = 0; faceIndex < vertsPerFace.size(); ++faceIndex )
	{
		if( ( faceIndex % 1000 ) == 0 )
		{
			Canceller::check( canceller );
		}

		for ( size_t faceVertIndex = 0; faceVertIndex < (size_t)vertsPerFace[faceIndex]; ++faceVertIndex)
		{
			size_t fvi0 = vertStart + faceVertIndex;
			centroids[faceIndex] += points[vertIds[fvi0]];
			faceIdPerVert[vertIds[fvi0]] = faceIndex;
		}
		centroids[faceIndex] /= vertsPerFace[faceIndex];

		vertStart += vertsPerFace[faceIndex];
	}

	// calculate per vertex tangents from centroids
	for ( size_t i = 0; i < (size_t)points.size(); ++i )
	{
		if( ( i % 1000 ) == 0 )
		{
			Canceller::check( canceller );
		}

		tangents[i] = ( centroids[faceIdPerVert[i]] - points[i] ).normalized();
		biTangents[i] = normals[i].cross( tangents[i] ).normalized();
		if ( orthoTangents )
		{
			if ( leftHanded )
			{
				tangents[i] = normals[i].cross( biTangents[i] ).normalized();
			}
			else
			{
				tangents[i] = biTangents[i].cross( normals[i] ).normalized();
			}
		}
	}

	// construct the primvars
	Canceller::check( canceller );
	V3fVectorDataPtr tangentsDataPtr = new V3fVectorData( tangents );
	Canceller::check( canceller );
	V3fVectorDataPtr biTangentsDataPtr = new V3fVectorData( biTangents );

	Canceller::check( canceller );
	PrimitiveVariable tangentPrimVar( IECoreScene::PrimitiveVariable::Interpolation::Vertex, tangentsDataPtr );
	Canceller::check( canceller );
	PrimitiveVariable biTangentPrimVar( IECoreScene::PrimitiveVariable::Interpolation::Vertex, biTangentsDataPtr );

	return std::make_pair( tangentPrimVar, biTangentPrimVar );
}

std::pair<PrimitiveVariable, PrimitiveVariable> IECoreScene::MeshAlgo::calculateTangentsFromFirstEdge(
	const MeshPrimitive *mesh,
	const std::string &position /* = "P" */,
	const std::string &normal /* = "N" */,
	bool orthoTangents,
	bool leftHanded,
	const Canceller *canceller
)
{
	// get point data
	const V3fVectorData *positionData = mesh->variableData<V3fVectorData>( position );
	if( !positionData )
	{
		std::string e = boost::str( boost::format( "MeshAlgo::calculateTangentsFromFirstEdge : MeshPrimitive has no Vertex \"%s\" primitive variable." ) % position );
		throw InvalidArgumentException( e );
	}
	const V3fVectorData::ValueType &points = positionData->readable();

	// get normals
	const V3fVectorData *normalData = mesh->variableData<V3fVectorData>( normal );
	if( !normalData )
	{
		std::string e = boost::str( boost::format( "MeshAlgo::calculateTangentsFromFirstEdge : MeshPrimitive has no Vertex \"%s\" primitive variable." ) % normal );
		throw InvalidArgumentException( e );
	}
	const auto normalIt = mesh->variables.find( normal );
	if( normalIt == mesh->variables.end() ||  normalIt->second.interpolation != IECoreScene::PrimitiveVariable::Interpolation::Vertex )
	{
		std::string e = boost::str( boost::format( "MeshAlgo::calculateTangentsFromPrimitiveCentroid : The normal primitive variable \"%s\" needs to be Vertex interpolated." ) % normal );
		throw InvalidArgumentException( e );
	}

	const auto &normals = normalData->readable();

	int numPoints = points.size();
	Canceller::check( canceller );
	std::vector<V3f> tangents( numPoints, V3f( 0 ) );
	Canceller::check( canceller );
	std::vector<V3f> biTangents( numPoints, V3f( 0 ) );

	// get neighbors
	std::pair<IntVectorDataPtr, IntVectorDataPtr> tangentPtr = MeshAlgo::connectedVertices( mesh, canceller );
	IntVectorDataPtr neighborList = tangentPtr.first;
	IntVectorDataPtr offsets = tangentPtr.second;
	auto &neighborListR = neighborList->readable();
	auto &offsetsR = offsets->readable();

	// calculate tangents from first neighbor and biTangents as orthogonal vectors
	for ( unsigned int i = 0; i < points.size(); ++i )
	{
		if( ( i % 1000 ) == 0 )
		{
			Canceller::check( canceller );
		}

		int firstNeighborIndex = i > 0 ? offsetsR[i - 1] : 0;
		const V3f &firstNeighbor = points[neighborListR[firstNeighborIndex]];
		tangents[i] = ( firstNeighbor - points[i] ).normalized();
		biTangents[i] = normals[i].cross( tangents[i] ).normalized();
		if ( orthoTangents )
		{
			if ( leftHanded )
			{
				tangents[i] = normals[i].cross( biTangents[i] ).normalized();
			}
			else
			{
				tangents[i] = biTangents[i].cross( normals[i] ).normalized();
			}
		}
	}

	// construct the primvars
	Canceller::check( canceller );
	V3fVectorDataPtr tangentsDataPtr = new V3fVectorData( tangents );
	Canceller::check( canceller );
	V3fVectorDataPtr biTangentsDataPtr = new V3fVectorData( biTangents );

	Canceller::check( canceller );
	PrimitiveVariable tangentPrimVar( IECoreScene::PrimitiveVariable::Interpolation::Vertex, tangentsDataPtr );
	Canceller::check( canceller );
	PrimitiveVariable biTangentPrimVar( IECoreScene::PrimitiveVariable::Interpolation::Vertex, biTangentsDataPtr );

	return std::make_pair( tangentPrimVar, biTangentPrimVar );
}

std::pair<PrimitiveVariable, PrimitiveVariable> IECoreScene::MeshAlgo::calculateTangentsFromTwoEdges(
	const MeshPrimitive *mesh,
	const std::string &position /* = "P" */,
	const std::string &normal /* = "N" */,
	bool orthoTangents,
	bool leftHanded,
	const Canceller *canceller
)
{
	// get point data
	const V3fVectorData *positionData = mesh->variableData<V3fVectorData>( position );
	if( !positionData )
	{
		std::string e = boost::str( boost::format( "MeshAlgo::calculateTangentsFromTwoEdges : MeshPrimitive has no Vertex \"%s\" primitive variable." ) % position );
		throw InvalidArgumentException( e );
	}
	const V3fVectorData::ValueType &points = positionData->readable();

	// get normals
	const V3fVectorData *normalData = mesh->variableData<V3fVectorData>( normal );
	if( !normalData )
	{
		std::string e = boost::str( boost::format( "MeshAlgo::calculateTangentsFromTwoEdges : MeshPrimitive has no Vertex \"%s\" primitive variable." ) % normal );
		throw InvalidArgumentException( e );
	}
	const auto normalIt = mesh->variables.find( normal );
	if( normalIt == mesh->variables.end() ||  normalIt->second.interpolation != IECoreScene::PrimitiveVariable::Interpolation::Vertex )
	{
		std::string e = boost::str( boost::format( "MeshAlgo::calculateTangentsFromPrimitiveCentroid : The normal primitive variable \"%s\" needs to be Vertex interpolated." ) % normal );
		throw InvalidArgumentException( e );
	}

	const auto &normals = normalData->readable();

	int numPoints = points.size();
	Canceller::check( canceller );
	std::vector<V3f> tangents( numPoints, V3f( 0 ) );
	Canceller::check( canceller );
	std::vector<V3f> biTangents( numPoints, V3f( 0 ) );

	// get neighbors
	std::pair<IntVectorDataPtr, IntVectorDataPtr> tangentPtr = MeshAlgo::connectedVertices( mesh, canceller );
	IntVectorDataPtr neighborList = tangentPtr.first;
	IntVectorDataPtr offsets = tangentPtr.second;
	auto &neighborListR = neighborList->readable();
	auto &offsetsR = offsets->readable();

	// calculate tangents from first neighbor and biTangents as orthogonal vectors
	for ( unsigned int i = 0; i < points.size(); ++i )
	{
		if( ( i % 1000 ) == 0 )
		{
			Canceller::check( canceller );
		}

		int firstNeighborIndex = i > 0 ? offsetsR[i - 1] : 0;
		int lastIndex =  offsetsR[i] > firstNeighborIndex ? firstNeighborIndex + 1 : firstNeighborIndex;  // if we only have one neighbor use the edge, else the next neighbor

		const V3f &firstNeighbor = points[neighborListR[firstNeighborIndex]];
		const V3f &secondNeighbor = points[neighborListR[lastIndex]];
		tangents[i] = ( ( firstNeighbor + (secondNeighbor - firstNeighbor ) * 0.5 ) - points[i] ).normalized();
		biTangents[i] = normals[i].cross( tangents[i] ).normalized();
		if ( orthoTangents )
		{
			if ( leftHanded )
			{
				tangents[i] = normals[i].cross( biTangents[i] ).normalized();
			}
			else
			{
				tangents[i] = biTangents[i].cross( normals[i] ).normalized();
			}
		}
	}

	// construct the primvars
	Canceller::check( canceller );
	V3fVectorDataPtr tangentsDataPtr = new V3fVectorData( tangents );
	Canceller::check( canceller );
	V3fVectorDataPtr biTangentsDataPtr = new V3fVectorData( biTangents );

	Canceller::check( canceller );
	PrimitiveVariable tangentPrimVar( IECoreScene::PrimitiveVariable::Interpolation::Vertex, tangentsDataPtr );
	Canceller::check( canceller );
	PrimitiveVariable biTangentPrimVar( IECoreScene::PrimitiveVariable::Interpolation::Vertex, biTangentsDataPtr );

	return std::make_pair( tangentPrimVar, biTangentPrimVar );
}
