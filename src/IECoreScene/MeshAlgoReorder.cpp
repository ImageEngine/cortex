//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

//////////////////////////////////////////////////////////////////////////
// Private implementation
//////////////////////////////////////////////////////////////////////////

namespace
{

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

struct ReorderFn
{

		ReorderFn( const std::vector<int> &remapping ) : m_remapping( remapping )
		{
		}

		template<typename T>
		DataPtr operator()( const TypedData<std::vector<T> > *d, const std::string &name )
		{
			const auto &inputs = d->readable();

			typename TypedData<std::vector<T> >::Ptr data = d->copy();
			auto &outputs = data->writable();

			int i = 0;
			for ( std::vector<int>::const_iterator it = m_remapping.begin(); it != m_remapping.end(); ++it, ++i )
			{
				outputs[i] = inputs[ *it ];
			}

			return data;
		}

		DataPtr operator()( Data *d, const std::string &name )
		{
			string e = boost::str( boost::format( "MeshAlgo::reorderVertices : \"%s\" has unsupported data type \"%s\"." ) % name % d->typeName() );
			throw InvalidArgumentException( e );
		}

	private:

		const std::vector<int> &m_remapping;

};

inline int index( int i, const int l )
{
	assert( l > 0 );

	while ( i < 0 )
	{
		i += l;
	}

	return i % l;
}

int faceDirection( FaceToVerticesMap &faceToVerticesMap, FaceId face, Edge edge )
{
	FaceToVerticesMap::const_iterator vit = faceToVerticesMap.find( face );
	assert( vit != faceToVerticesMap.end() );
	const VertexList &faceVertices = vit->second;

	int numFaceVertices = faceVertices.size();

	VertexList::const_iterator it = std::find( faceVertices.begin(), faceVertices.end(), edge.first );
	assert( it != faceVertices.end() );

	int edgeVertexOrigin = std::distance( faceVertices.begin(), it );

	assert( faceVertices[ index( edgeVertexOrigin, numFaceVertices )] == edge.first );

	int direction = 0;
	if ( faceVertices[ index( edgeVertexOrigin+1, numFaceVertices )] == edge.second )
	{
		direction = 1;
	}
	else
	{
		assert( faceVertices[ index( edgeVertexOrigin-1, numFaceVertices )] == edge.second ) ;
		direction = -1;
	}

	assert( direction == 1 || direction == -1 );
	return direction;
}

void visitFace(
	const MeshPrimitive * mesh,
	FaceId currentFace,
	Edge currentEdge,
	FaceToVerticesMap &faceToVerticesMap,
	FaceToEdgesMap &faceToEdgesMap,
	VertexList &faceVaryingOffsets,
	EdgeToConnectedFacesMap &edgeToConnectedFacesMap,
	std::vector<VertexId> &vertexMap,
	std::vector<VertexId> &vertexRemap,
	std::vector<int> &newVerticesPerFace,
	std::vector<VertexId> &newVertexIds,
	std::vector<int> &faceVaryingRemap,
	std::vector<int> &faceRemap,
	int &nextVertex
)
{
	assert( currentEdge.first != currentEdge.second );

	if ( faceRemap[ currentFace ] != -1 )
	{
		return;
	}

	FaceToEdgesMap::const_iterator eit = faceToEdgesMap.find( currentFace );
	assert( eit != faceToEdgesMap.end() );
	const EdgeList &faceEdges = eit->second;

	assert( faceEdges.size() >= 3 );

	FaceToVerticesMap::const_iterator vit = faceToVerticesMap.find( currentFace );
	assert( vit != faceToVerticesMap.end() );
	const VertexList &faceVertices = vit->second;

	int numFaceVertices = faceVertices.size();

	VertexList::const_iterator it = std::find( faceVertices.begin(), faceVertices.end(), currentEdge.first );
	assert( it != faceVertices.end() );

	int currentEdgeVertexOrigin = std::distance( faceVertices.begin(), it );

	assert( faceVertices[ index( currentEdgeVertexOrigin, numFaceVertices )] == currentEdge.first );

	int faceVerticesDirection = faceDirection( faceToVerticesMap, currentFace, currentEdge );

	EdgeList faceEdgesSorted( numFaceVertices );
	VertexList faceVerticesSorted( numFaceVertices );

	int i;
	for ( i = 0; i < numFaceVertices; i++ )
	{
		faceVerticesSorted[i] = faceVertices[index( currentEdgeVertexOrigin + i * faceVerticesDirection, numFaceVertices )];

		if ( faceVerticesDirection == 1 )
		{
			faceEdgesSorted[i] = faceEdges[index( currentEdgeVertexOrigin + i , numFaceVertices )];
		}
		else
		{
			faceEdgesSorted[i] = faceEdges[index( currentEdgeVertexOrigin - 1 - i, numFaceVertices )];
		}
	}

	for ( i = 0; i < numFaceVertices; i++ )
	{
		VertexId vertIndex = faceVerticesSorted[i];

		if ( vertexMap[vertIndex] == -1 )
		{
			vertexMap[vertIndex] = nextVertex++;
			vertexRemap[ vertexMap[vertIndex] ] = vertIndex;
		}
	}

	/// Create the "uniform" mapping
	faceRemap[currentFace] = newVerticesPerFace.size();

	/// Create the "vertex"/"varying" mapping
	newVerticesPerFace.push_back( numFaceVertices );
	for ( i = 0; i < numFaceVertices; i++ )
	{
		newVertexIds.push_back( vertexMap[ faceVerticesSorted[i] ] );
	}

	/// Create the "face-varying" mapping
	int faceVaryingRemapStart = faceVaryingOffsets[ currentFace ];
	int fvRelativeIdx = currentEdgeVertexOrigin;
	for ( i = 0; i < numFaceVertices; i++ )
	{
		assert( faceVaryingRemapStart >= 0 );
		assert( fvRelativeIdx >= 0 );
		assert( fvRelativeIdx < numFaceVertices );

		int fvIdx = faceVaryingRemapStart + fvRelativeIdx;

		assert( fvIdx >= 0 );
		assert( fvIdx < ( int )mesh->variableSize( PrimitiveVariable::FaceVarying ) );

		faceVaryingRemap.push_back( fvIdx );

		fvRelativeIdx += faceVerticesDirection;
		fvRelativeIdx = index( fvRelativeIdx, numFaceVertices );
	}

	/// Follow current face's edges in order, recursing onto adjacent faces
	for ( EdgeList::const_iterator edgeIt = faceEdgesSorted.begin(); edgeIt != faceEdgesSorted.end(); ++edgeIt )
	{
		Edge nextEdge( *edgeIt );

		EdgeToConnectedFacesMap::const_iterator eit = edgeToConnectedFacesMap.find( nextEdge );
		assert( eit != edgeToConnectedFacesMap.end() );
		const FaceList &connectedFaces = eit->second;

		/// Recurse onto the face adjacent to the next edge
		if ( connectedFaces.size() > 1 )
		{
			int nextFace = ( connectedFaces[0] == currentFace ? connectedFaces[1] : connectedFaces[0] );

			if ( faceDirection( faceToVerticesMap, nextFace, nextEdge ) != faceVerticesDirection )
			{
				std::swap( nextEdge.first, nextEdge.second );
				assert( faceDirection( faceToVerticesMap, nextFace, nextEdge ) == faceVerticesDirection );
			}

			visitFace(
				mesh,
				nextFace,
				nextEdge,
				faceToVerticesMap,
				faceToEdgesMap,
				faceVaryingOffsets,
				edgeToConnectedFacesMap,
				vertexMap,
				vertexRemap,
				newVerticesPerFace,
				newVertexIds,
				faceVaryingRemap,
				faceRemap,
				nextVertex
			);
		}
	}
}

void buildInternalTopology(
	const std::vector<int> &vertexIds,
	const std::vector<int> &verticesPerFace,
	FaceToEdgesMap &faceToEdgesMap,
	FaceToVerticesMap &faceToVerticesMap,
	EdgeToConnectedFacesMap &edgeToConnectedFacesMap,
	VertexToFacesMap &vertexToFacesMap,
	VertexList &faceVaryingOffsets,
	int numFaces,
	int numVerts
)
{
	int vertOffset = 0;
	int faceVaryingIdx = 0;

	for( int f = 0; f < numFaces; ++f )
	{
		int numFaceVertices = verticesPerFace[f];
		assert( numFaceVertices >= 3 );

		faceVaryingOffsets.push_back( faceVaryingIdx );
		faceVaryingIdx += numFaceVertices;

		for( int v = 0; v < numFaceVertices; ++v )
		{
			assert( vertOffset < (int)vertexIds.size() );
			int vertexId = vertexIds[ vertOffset + v ];

			assert( vertexId < numVerts );

			vertexToFacesMap[ vertexId ].insert( f );

			faceToVerticesMap[ f ].push_back( vertexId );

			int nextVertexId = vertexIds[ vertOffset + (( v + 1 ) % numFaceVertices )];

			faceToEdgesMap[ f ].push_back( Edge( vertexId, nextVertexId ) );

			edgeToConnectedFacesMap[ Edge( vertexId, nextVertexId )].push_back( f );
			edgeToConnectedFacesMap[ Edge( nextVertexId, vertexId )].push_back( f );
		}

		vertOffset += numFaceVertices;
	}
}

IntVectorDataPtr reorderIds( const std::vector<int> &ids, std::vector<VertexId> &vertexMap )
{
	IntVectorDataPtr result = new IntVectorData;
	auto &outIds = result->writable();
	outIds.reserve( ids.size() );
	for( auto id : ids )
	{
		outIds.push_back( vertexMap[id] );
	}

	return result;
}

} // namespace

void MeshAlgo::reorderVertices( MeshPrimitive *mesh, int id0, int id1, int id2 )
{
	FaceToEdgesMap faceToEdgesMap;
	FaceToVerticesMap faceToVerticesMap;
	EdgeToConnectedFacesMap edgeToConnectedFacesMap;
	VertexToFacesMap vertexToFacesMap;
	VertexList faceVaryingOffsets;

	const std::vector<int> &vertexIds = mesh->vertexIds()->readable();
	const std::vector<int> &verticesPerFace = mesh->verticesPerFace()->readable();
	int numFaces = verticesPerFace.size();
	int numVerts = mesh->variableSize( PrimitiveVariable::Vertex );

	if ( !numFaces || numVerts < 3 )
	{
		throw InvalidArgumentException( "MeshAlgo::reorderVertices : Cannot reorder empty mesh." );
	}

	buildInternalTopology( vertexIds, verticesPerFace, faceToEdgesMap, faceToVerticesMap, edgeToConnectedFacesMap, vertexToFacesMap, faceVaryingOffsets, numFaces, numVerts );

	for( EdgeToConnectedFacesMap::const_iterator it = edgeToConnectedFacesMap.begin(); it != edgeToConnectedFacesMap.end(); ++it )
	{
		if ( it->second.size() > 2 || it->second.size() == 0 )
		{
			throw InvalidArgumentException( "MeshAlgo::reorderVertices : Cannot reorder non-manifold mesh." );
		}
	}

	VertexToFacesMap::const_iterator vIt0 = vertexToFacesMap.find( id0 );
	if( vIt0 == vertexToFacesMap.end() )
	{
		throw InvalidArgumentException( ( boost::format( "MeshAlgo::reorderVertices : Cannot find vertex %d" ) % id0 ).str() );
	}

	VertexToFacesMap::const_iterator vIt1 = vertexToFacesMap.find( id1 );
	if( vIt1 == vertexToFacesMap.end() )
	{
		throw InvalidArgumentException( ( boost::format( "MeshAlgo::reorderVertices : Cannot find vertex %d" ) % id1 ).str() );
	}

	VertexToFacesMap::const_iterator vIt2 = vertexToFacesMap.find( id2 );
	if( vIt2 == vertexToFacesMap.end() )
	{
		throw InvalidArgumentException( ( boost::format( "MeshAlgo::reorderVertices : Cannot find vertex %d" ) % id2 ).str() );
	}

	FaceSet tmp;

	const FaceSet &vtx0Faces = vIt0->second;
	const FaceSet &vtx1Faces = vIt1->second;
	const FaceSet &vtx2Faces = vIt2->second;

	std::set_intersection(
		vtx0Faces.begin(), vtx0Faces.end(),
		vtx1Faces.begin(), vtx1Faces.end(),
		std::inserter( tmp, tmp.end() )
	);

	FaceSet tmp2;
	std::set_intersection(
		tmp.begin(),  tmp.end(),
		vtx2Faces.begin(),  vtx2Faces.end(),
		std::inserter( tmp2, tmp2.end() )
	);

	if( tmp2.size() != 1 )
	{
		throw InvalidArgumentException( ( boost::format( "MeshAlgo::reorderVertices : Vertices %d, %d, and %d do not uniquely define a single polygon" ) % id0 % id1 % id2 ).str() );
	}

	int currentFace = *tmp2.begin();
	Edge currentEdge( id0, id1 );
	std::vector<int> faceRemap( numFaces, -1 );
	std::vector<VertexId> vertexMap( numVerts, -1 );
	std::vector<VertexId> vertexRemap( numVerts, -1 );
	std::vector<int> newVerticesPerFace;
	std::vector<VertexId> newVertexIds;
	std::vector<int> faceVaryingRemap;
	int nextVertex = 0;

	visitFace(
		mesh,
		currentFace,
		currentEdge,
		faceToVerticesMap,
		faceToEdgesMap,
		faceVaryingOffsets,
		edgeToConnectedFacesMap,
		vertexMap,
		vertexRemap,
		newVerticesPerFace,
		newVertexIds,
		faceVaryingRemap,
		faceRemap,
		nextVertex
	);

	assert( (int)vertexMap.size() == numVerts );
	assert( (int)vertexRemap.size() == numVerts );
	for( int i = 0; i < numVerts; ++i )
	{
		if( vertexMap[i] == -1 || vertexRemap[i] == -1 )
		{
			throw InvalidArgumentException( "MeshAlgo::reorderVertices : Found unvisited vertices during mesh traversal - ensure mesh is fully connected." );
		}
	}

	assert( (int)faceRemap.size() == numFaces );
	for ( int i = 0; i < numFaces; i++ )
	{
		if ( faceRemap[i] == -1 )
		{
			throw InvalidArgumentException( "MeshAlgo::reorderVertices : Found unvisited faces during mesh traversal - ensure mesh is fully connected." );
		}
	}

	assert( faceVaryingRemap.size() == mesh->variableSize( PrimitiveVariable::FaceVarying ) );
	assert( newVerticesPerFace.size() == verticesPerFace.size() );
	assert( newVertexIds.size() == vertexIds.size() );

	mesh->setTopologyUnchecked( new IntVectorData( newVerticesPerFace ), new IntVectorData( newVertexIds ), numVerts, mesh->interpolation() );

	ReorderFn vertexFn( vertexRemap );
	ReorderFn faceVaryingFn( faceVaryingRemap );
	ReorderFn uniformFn( faceRemap );

	const auto &cornerIds = mesh->cornerIds()->readable();
	if( !cornerIds.empty() )
	{
		mesh->setCorners( reorderIds( cornerIds, vertexMap ).get(), mesh->cornerSharpnesses() );
	}

	const auto &creaseIds = mesh->creaseIds()->readable();
	if( !creaseIds.empty() )
	{
		mesh->setCreases( mesh->creaseLengths(), reorderIds( creaseIds, vertexMap ).get(), mesh->creaseSharpnesses() );
	}

	for( PrimitiveVariableMap::iterator it = mesh->variables.begin(); it != mesh->variables.end(); ++it )
	{
		if( it->second.interpolation == PrimitiveVariable::FaceVarying )
		{
			assert( it->second.data );
			if( it->second.indices )
			{
				it->second.indices = runTimeCast<IntVectorData>( faceVaryingFn( it->second.indices.get(), it->first ) );
			}
			else
			{
				it->second.data = dispatch( it->second.data.get(), faceVaryingFn, it->first );
			}
		}
		else if( it->second.interpolation == PrimitiveVariable::Vertex || it->second.interpolation == PrimitiveVariable::Varying )
		{
			assert( it->second.data );
			if( it->second.indices )
			{
				it->second.indices = runTimeCast<IntVectorData>( vertexFn( it->second.indices.get(), it->first ) );
			}
			else
			{
				it->second.data = dispatch( it->second.data.get(), vertexFn, it->first );
			}
		}
		else if( it->second.interpolation == PrimitiveVariable::Uniform )
		{
			assert( it->second.data );
			if( it->second.indices )
			{
				it->second.indices = runTimeCast<IntVectorData>( uniformFn( it->second.indices.get(), it->first ) );
			}
			else
			{
				it->second.data = dispatch( it->second.data.get(), uniformFn, it->first );
			}
		}
	}

	assert( mesh->arePrimitiveVariablesValid() );
}
