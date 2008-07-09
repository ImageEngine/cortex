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

#include "IECore/CompoundParameter.h"
#include "IECore/MeshVertexReorderOp.h"
#include "IECore/DespatchTypedData.h"

#include "boost/format.hpp"

using namespace IECore;
using namespace std;

MeshVertexReorderOp::MeshVertexReorderOp() : MeshPrimitiveOp( staticTypeName(), "Calculates vertex normals for a mesh." )
{
	m_startingVerticesParameter = new V3iParameter(
	        "startingVertices",
	        "startingVertices description",
	        Imath::V3i( 0, 1, 2 )
	);

	parameters()->addParameter( m_startingVerticesParameter );
}

MeshVertexReorderOp::~MeshVertexReorderOp()
{
}

V3iParameterPtr MeshVertexReorderOp::startingVerticesParameter()
{
	return m_startingVerticesParameter;
}

ConstV3iParameterPtr MeshVertexReorderOp::startingVerticesParameter() const
{
	return m_startingVerticesParameter;
}

struct MeshVertexReorderOp::ReorderFn
{
	std::string m_name;
	
	typedef DataPtr ReturnType;

	ReorderFn( const std::vector<int> &indices ) : m_indices( indices )
	{
	}
	
	template<typename T>
	DataPtr operator()( typename T::Ptr d )
	{
		assert( d );
		typename T::Ptr data = runTimeCast<T>( d->copy() );
		assert( data );
		assert( data->readable().size()== m_indices.size() );

		int i = 0;
		for ( std::vector<int>::const_iterator it = m_indices.begin(); it != m_indices.end(); ++it, ++i )
		{
			data->writable()[i] = d->readable()[ *it ];
		}

		assert( data->readable().size() == m_indices.size() );

		return data;
	}

private:

	const std::vector<int> &m_indices;

};
struct MeshVertexReorderOp::HandleErrors
{
	template<typename T, typename F>
	void operator()( typename T::ConstPtr d, const F &f )
	{
		assert( d );
		string e = boost::str( boost::format( "MeshVertexReorderOp : \"%s\" has unsupported data type \"%s\"." ) % f.m_name % d->typeName() );
		throw InvalidArgumentException( e ); 
	}
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

int MeshVertexReorderOp::faceDirection(	FaceId face, Edge edge )
{
	FaceToVerticesMap::const_iterator vit = m_faceToVerticesMap.find( face );
	assert( vit != m_faceToVerticesMap.end() );
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

void MeshVertexReorderOp::visitFace(
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
)
{
	assert( mesh );
	assert( currentFace < m_numFaces );
	assert( currentEdge.first < m_numVerts );
	assert( currentEdge.second < m_numVerts );
	assert( currentEdge.first != currentEdge.second );

	if ( faceRemap[ currentFace ] != -1 )
	{
		return;
	}

	FaceToEdgesMap::const_iterator eit = m_faceToEdgesMap.find( currentFace );
	assert( eit != m_faceToEdgesMap.end() );
	const EdgeList &faceEdges = eit->second;

	assert( faceEdges.size() >= 3 );

	FaceToVerticesMap::const_iterator vit = m_faceToVerticesMap.find( currentFace );
	assert( vit != m_faceToVerticesMap.end() );
	const VertexList &faceVertices = vit->second;

	int numFaceVertices = faceVertices.size();

	VertexList::const_iterator it = std::find( faceVertices.begin(), faceVertices.end(), currentEdge.first );
	assert( it != faceVertices.end() );

	int currentEdgeVertexOrigin = std::distance( faceVertices.begin(), it );

	assert( faceVertices[ index( currentEdgeVertexOrigin, numFaceVertices )] == currentEdge.first );

	int faceVerticesDirection = faceDirection( currentFace, currentEdge );

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
	int faceVaryingRemapStart = m_faceVaryingOffsets[ currentFace ];
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

		EdgeToConnectedFacesMap::const_iterator eit = m_edgeToConnectedFacesMap.find( nextEdge );
		assert( eit != m_edgeToConnectedFacesMap.end() );
		const FaceList &connectedFaces = eit->second;

		/// Recurse onto the face adjacent to the next edge
		if ( connectedFaces.size() > 1 )
		{
			int nextFace = ( connectedFaces[0] == currentFace ? connectedFaces[1] : connectedFaces[0] );

			if ( faceDirection( nextFace, nextEdge ) != faceVerticesDirection )
			{
				std::swap( nextEdge.first, nextEdge.second );
				assert( faceDirection( nextFace, nextEdge ) == faceVerticesDirection );
			}

			visitFace( mesh, nextFace, nextEdge,
			           vertexMap, vertexRemap, newVerticesPerFace, newVertexIds, faceVaryingRemap, faceRemap, nextVertex );
		}
	}
}

void MeshVertexReorderOp::buildInternalTopology( ConstMeshPrimitivePtr mesh )
{
	assert( mesh );

	m_faceToEdgesMap.clear();
	m_faceToVerticesMap.clear();
	m_edgeToConnectedFacesMap.clear();
	m_faceVaryingOffsets.clear();
	m_vertexToFacesMap.clear();

	m_numFaces = mesh->verticesPerFace()->readable().size();
	m_numVerts = mesh->variableSize( PrimitiveVariable::Vertex );

	if ( !m_numFaces || m_numVerts < 3 )
	{
		throw InvalidArgumentException( "MeshVertexReorderOp : Cannot reorder empty mesh." );
	}

	int vertOffset = 0;
	int faceVaryingIdx = 0;

	for ( int f = 0; f < m_numFaces; f++ )
	{
		int numFaceVertices = mesh->verticesPerFace()->readable()[f];
		assert( numFaceVertices >= 3 );

		m_faceVaryingOffsets.push_back( faceVaryingIdx );
		faceVaryingIdx += numFaceVertices;

		for ( int v = 0; v < numFaceVertices; v++ )
		{
			assert( vertOffset < ( int )mesh->vertexIds()->readable().size() );
			int vertexId = mesh->vertexIds()->readable()[ vertOffset + v ];

			assert( vertexId < m_numVerts );

			m_vertexToFacesMap[ vertexId ].insert( f );

			m_faceToVerticesMap[ f ].push_back( vertexId );

			int nextVertexId = mesh->vertexIds()->readable()[ vertOffset + (( v + 1 ) % numFaceVertices )];

			m_faceToEdgesMap[ f ].push_back( Edge( vertexId, nextVertexId ) );

			m_edgeToConnectedFacesMap[ Edge( vertexId, nextVertexId )].push_back( f );
			m_edgeToConnectedFacesMap[ Edge( nextVertexId, vertexId )].push_back( f );
		}

		vertOffset += numFaceVertices;
	}
	
	for ( EdgeToConnectedFacesMap::const_iterator it = m_edgeToConnectedFacesMap.begin(); it != m_edgeToConnectedFacesMap.end(); ++it )
	{
		if ( it->second.size() > 2 || it->second.size() == 0 )
		{
			throw InvalidArgumentException( "MeshVertexReorderOp : Cannot reorder non-manifold mesh." );
		}
	}
}

void MeshVertexReorderOp::modifyTypedPrimitive( MeshPrimitivePtr mesh, ConstCompoundObjectPtr operands )
{
	PrimitiveVariableMap::const_iterator pvIt = mesh->variables.find( "P" );
	if ( pvIt==mesh->variables.end() || !pvIt->second.data )
	{
		throw InvalidArgumentException( "MeshVertexReorderOp : MeshPrimitive has no \"P\" primitive variable." );
	}

	if ( !mesh->isPrimitiveVariableValid( pvIt->second ) )
	{
		throw InvalidArgumentException( "MeshVertexReorderOp : \"P\" primitive variable is invalid." );
	}

	buildInternalTopology( mesh );

	Imath::V3i faceVtxSrc = m_startingVerticesParameter->getTypedValue();

	for ( int i = 0; i < 3; i++ )
	{
		if ( m_vertexToFacesMap.find( faceVtxSrc[i] ) == m_vertexToFacesMap.end() )
		{
			throw InvalidArgumentException(
			        ( boost::format( "MeshVertexReorderOp : Cannot find vertex %d" ) % faceVtxSrc[i] ).str()
			);
		}
	}

	FaceSet tmp;

	const FaceSet &vtx0Faces = m_vertexToFacesMap[ faceVtxSrc[0] ];
	const FaceSet &vtx1Faces = m_vertexToFacesMap[ faceVtxSrc[1] ];
	const FaceSet &vtx2Faces = m_vertexToFacesMap[ faceVtxSrc[2] ];

	std::set_intersection(
	        vtx0Faces.begin(),  vtx0Faces.end(),
	        vtx1Faces.begin(),  vtx1Faces.end(),
	        std::inserter( tmp, tmp.end() )
	);

	FaceSet tmp2;
	std::set_intersection(
	        tmp.begin(),  tmp.end(),
	        vtx2Faces.begin(),  vtx2Faces.end(),
	        std::inserter( tmp2, tmp2.end() )
	);

	if ( tmp2.size() != 1 )
	{
		throw InvalidArgumentException(
		        ( boost::format( "MeshVertexReorderOp : Vertices %d, %d, and %d do not uniquely define a single polygon" ) %
		          faceVtxSrc[0] % faceVtxSrc[1] % faceVtxSrc[2] ).str()
		);
	}

	int currentFace = *tmp2.begin();
	Edge currentEdge( faceVtxSrc[0], faceVtxSrc[1] );
	std::vector<int> faceRemap( m_numFaces, -1 );
	std::vector<VertexId> vertexMap( m_numVerts, -1 );
	std::vector<VertexId> vertexRemap( m_numVerts, -1 );
	std::vector<int> newVerticesPerFace;
	std::vector<VertexId> newVertexIds;
	std::vector<int> faceVaryingRemap;
	int nextVertex = 0;

	visitFace( mesh, currentFace, currentEdge, vertexMap, vertexRemap, newVerticesPerFace, newVertexIds,
	           faceVaryingRemap, faceRemap, nextVertex );

	assert( (int)vertexMap.size() == m_numVerts );
	assert( (int)vertexRemap.size() == m_numVerts );
	for ( int i = 0; i < m_numVerts; i++ )
	{
		if ( vertexMap[i] == -1 || vertexRemap[i] == -1 )
		{
			throw InvalidArgumentException( "MeshVertexReorderOp : Found unvisited vertices during mesh traversal - ensure mesh is fully connected." );
		}
	}

	assert( (int)faceRemap.size() == m_numFaces );
	for ( int i = 0; i < m_numFaces; i++ )
	{
		if ( faceRemap[i] == -1 )
		{
			throw InvalidArgumentException( "MeshVertexReorderOp : Found unvisited faces during mesh traversal - ensure mesh is fully connected." );
		}
	}

	assert( faceVaryingRemap.size() == mesh->variableSize( PrimitiveVariable::FaceVarying ) );
	assert( newVerticesPerFace.size() == mesh->verticesPerFace()->readable().size() );
	assert( newVertexIds.size() == mesh->vertexIds()->readable().size() );
	mesh->setTopology( new IntVectorData( newVerticesPerFace ), new IntVectorData( newVertexIds ) );

	ReorderFn vertexFn( vertexRemap );
	ReorderFn faceVaryingFn( faceVaryingRemap );
	ReorderFn uniformFn( faceRemap );

	for ( PrimitiveVariableMap::iterator it = mesh->variables.begin(); it != mesh->variables.end(); ++it )
	{
		if ( it->second.interpolation == PrimitiveVariable::FaceVarying )
		{
			assert( it->second.data );

			faceVaryingFn.m_name = it->first;
			it->second.data = despatchTypedData<ReorderFn, TypeTraits::IsVectorTypedData>( it->second.data, faceVaryingFn );
		}
		else if ( it->second.interpolation == PrimitiveVariable::Vertex || it->second.interpolation == PrimitiveVariable::Varying )
		{
			assert( it->second.data );

			vertexFn.m_name = it->first;
			it->second.data = despatchTypedData<ReorderFn, TypeTraits::IsVectorTypedData>( it->second.data, vertexFn );
		}
		else if ( it->second.interpolation == PrimitiveVariable::Uniform )
		{
			assert( it->second.data );

			uniformFn.m_name = it->first;
			it->second.data = despatchTypedData<ReorderFn, TypeTraits::IsVectorTypedData>( it->second.data, uniformFn );
		}
	}

	assert( mesh->arePrimitiveVariablesValid() );
}
