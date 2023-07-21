//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2019, Image Engine Design Inc. All rights reserved.
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

#include "IECore/PolygonAlgo.h"

#include "boost/format.hpp"
#include "boost/iterator/transform_iterator.hpp"
#include "boost/iterator/zip_iterator.hpp"
#include "boost/tuple/tuple.hpp"

#include "tbb/parallel_for.h"

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

namespace {

void populateFaceVertexNormals(
	const std::vector<V3f> &points, const int *vertIds, const int numVerts, MeshAlgo::NormalWeighting weighting,
	V3f *faceVertexNormals, float *normalWeights = nullptr
)
{
	V3f vPrevEdge = points[vertIds[0]] - points[ vertIds[numVerts - 1] ];

	if( weighting == MeshAlgo::NormalWeighting::Angle )
	{
		vPrevEdge.normalize();
	}

	V3f faceNormal = polygonNormal(
		PolygonVertexIterator( vertIds, &points[0] ),
		PolygonVertexIterator( vertIds + numVerts, &points[0] ),
		/* normalized */ false
	);

	float area = 0;
	if( weighting == MeshAlgo::NormalWeighting::Area )
	{
		area = faceNormal.length();
	}

	for( int i=0; i < numVerts; ++i )
	{
		V3f vEdge = points[vertIds[ ( i + 1 ) % numVerts ] ] - points[ vertIds[ i ] ];

		if( weighting == MeshAlgo::NormalWeighting::Angle )
		{
			vEdge.normalize();
		}

		V3f faceVertNormal = -vEdge.cross( vPrevEdge );
		float faceVertNormalLength = faceVertNormal.length();
		float normalize = 1.0f / faceVertNormalLength;

		if( faceVertNormal.dot( faceNormal ) < 0.0f )
		{
			normalize = -normalize;
		}

		float weight = 1.0f;
		if( weighting == MeshAlgo::NormalWeighting::Angle )
		{
			weight = asinf( std::min( 1.0f, faceVertNormalLength ) );
			if( vEdge.dot( vPrevEdge ) > 0 )
			{
				weight = ((float)M_PI) - weight;
			}
		}
		else if( weighting == MeshAlgo::NormalWeighting::Area )
		{
			weight = area;
		}


		if( !normalWeights )
		{
			// Without a separate output for weights, we bake it into the normal
			faceVertexNormals[ i ] = faceVertNormal * normalize * weight;
		}
		else
		{
			faceVertexNormals[ i ] = faceVertNormal * normalize;
			normalWeights[ i ] = weight;
		}

		vPrevEdge = vEdge;
	}
}

// Given two different vertices, i and j, out of n total vertices, return an index into a list
// of length `n * ( n - 1 ) / 2` that is unique to this pair of vertices. Used to store
// whether 2 vertices are joined based on the angle threshold
int joinedMatrixIndex( int i, int j, int n )
{
	if( i < j )
	{
		return i * ( n - 2 ) - ( (i - 1) * i ) / 2 + j - 1;
	}
	else
	{
		return j * ( n - 2 ) - ( (j - 1) * j ) / 2 + i - 1;
	}
}

PrimitiveVariable calculateNormalsImpl( const MeshPrimitive *mesh, PrimitiveVariable::Interpolation interpolation, MeshAlgo::NormalWeighting weighting, float thresholdAngle, const std::string &position, const Canceller *canceller )
{
	const V3fVectorData *pData = mesh->variableData<V3fVectorData>( position, PrimitiveVariable::Vertex );
	if( !pData )
	{
		throw InvalidArgumentException( boost::str( boost::format( "MeshAlgo::calculateNormals : MeshPrimitive has no \"%s\" primitive variable." ) % position ) );
	}
	const std::vector<V3f> &points = pData->readable();

	assert(
		interpolation == PrimitiveVariable::Uniform ||
		interpolation == PrimitiveVariable::Vertex ||
		interpolation == PrimitiveVariable::FaceVarying
	);

	const auto &verticesPerFace = mesh->verticesPerFace()->readable();

	Canceller::check( canceller );

	std::vector<int> startPerFace;
	startPerFace.reserve( verticesPerFace.size() );

	Canceller::check( canceller );

	int offset = 0;
	for( auto numVerts : verticesPerFace )
	{
		startPerFace.push_back( offset );
		offset += numVerts;
	}

	const auto &vertIds = mesh->vertexIds()->readable();

	tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );

	if( interpolation == PrimitiveVariable::Uniform )
	{
		V3fVectorDataPtr faceNormalsData = new V3fVectorData;
		faceNormalsData->setInterpretation( GeometricData::Normal );
		auto &faceNormals = faceNormalsData->writable();

		Canceller::check( canceller );

		faceNormals.resize( verticesPerFace.size() );

		tbb::parallel_for( tbb::blocked_range<int>( 0, verticesPerFace.size() ),
			[ &verticesPerFace, &startPerFace, &vertIds, &points, &faceNormals, &canceller ]
			( const tbb::blocked_range<int> &range )
			{
				for( int faceId = range.begin(); faceId != range.end(); ++faceId )
				{
					Canceller::check( canceller );

					int numVerts = verticesPerFace[faceId];
					int faceStart = startPerFace[faceId];

					faceNormals[faceId] = polygonNormal(
						PolygonVertexIterator( &vertIds[faceStart], &points[0] ),
						PolygonVertexIterator( &vertIds[faceStart + numVerts], &points[0] )
					);
				}
			},
			taskGroupContext
		);
		return PrimitiveVariable( interpolation, faceNormalsData );
	}


	Canceller::check( canceller );

	V3fVectorDataPtr faceVertexNormalsData = new V3fVectorData;
	faceVertexNormalsData->setInterpretation( GeometricData::Normal );
	auto &faceVertexNormals = faceVertexNormalsData->writable();

	faceVertexNormals.resize( vertIds.size() );

	std::vector< float > normalWeights;
	if( interpolation == PrimitiveVariable::FaceVarying )
	{
		Canceller::check( canceller );

		// When using FaceVarying interpolation, we need access to the normalized normals, so we can't
		// bake in the weighting
		normalWeights.resize( vertIds.size() );
	}

	tbb::parallel_for( tbb::blocked_range<int>( 0, verticesPerFace.size() ),
		[ &verticesPerFace, &startPerFace, &vertIds, &points, &faceVertexNormals, &normalWeights, &weighting, &canceller ]
		( const tbb::blocked_range<int> &range )
		{
			for( int faceId = range.begin(); faceId != range.end(); ++faceId )
			{
				Canceller::check( canceller );

				int numVerts = verticesPerFace[faceId];
				int faceStart = startPerFace[faceId];

				float *normalWeightsOutput = normalWeights.size() ? &normalWeights[ faceStart ] : nullptr;

				populateFaceVertexNormals(
					points, &vertIds[faceStart], numVerts, weighting,
					&faceVertexNormals[ faceStart ], normalWeightsOutput
				);
			}
		},
		taskGroupContext
	);

	if( interpolation == PrimitiveVariable::Vertex )
	{
		V3fVectorDataPtr vertexNormalsData = new V3fVectorData;
		vertexNormalsData->setInterpretation( GeometricData::Normal );
		auto &vertexNormals = vertexNormalsData->writable();
		vertexNormals.resize( points.size(), Imath::V3f( 0 ) );

		// accumulate the normals for each vertex
		for( int i = 0; i < (int)vertIds.size(); i++ )
		{
			vertexNormals[vertIds[i]] += faceVertexNormals[i];
		}

		// normalize each of the result normals
		tbb::parallel_for( tbb::blocked_range<int>( 0, vertexNormals.size() ),
			[ &vertexNormals, &canceller ]
			( const tbb::blocked_range<int> &range )
			{
				Canceller::check( canceller );
				for( int i = range.begin(); i != range.end(); ++i )
				{
					vertexNormals[i].normalize();
				}
			},
			taskGroupContext
		);

		return PrimitiveVariable( interpolation, vertexNormalsData );
	}

	// The complex case: face-varying normals, which may be faceted or not depending on thresholdAngle
	float cosThreshold;
	if( thresholdAngle <= 0.0f )
	{
		cosThreshold = 1.0f;
	}
	else if( thresholdAngle >= 180.0f )
	{
		cosThreshold = -1.0f;
	}
	else
	{
		cosThreshold = cosf( thresholdAngle / 180.0f * (float)M_PI );
	}

	auto [ faceVerticesData, faceVertexOffsetsData ] = MeshAlgo::correspondingFaceVertices( mesh, canceller );
	const std::vector<int> &faceVertices = faceVerticesData->readable();
	const std::vector<int> &faceVertexOffsets = faceVertexOffsetsData->readable();

	tbb::parallel_for( tbb::blocked_range<int>( 0, faceVertexOffsets.size() ),
		[ &faceVertexNormals, &normalWeights, &faceVertexOffsets, &faceVertices, &cosThreshold, &canceller ]
		( const tbb::blocked_range<int> &range )
		{
			std::vector<V3f> faceVertexNormalsForVert;
			int startOffset = range.begin() == 0 ? 0 : faceVertexOffsets[ range.begin() - 1 ];
			std::vector<char> faceVerticesJoined;

			for( int vertex = range.begin(); vertex != range.end(); ++vertex )
			{
				Canceller::check( canceller );

				int endOffset = faceVertexOffsets[ vertex ];
				int numFaceVerts = endOffset - startOffset;

				int numPossibleJoins = ( numFaceVerts * ( numFaceVerts - 1 ) ) / 2;
				int numMatchingVerts;

				if( cosThreshold == 1.0f )
				{
					// Nothing can match this threshold ( unless the normals are identical, in which case
					// averaging them together won't change anything anyway )
					numMatchingVerts = 0;
				}
				else if( cosThreshold == -1.0f )
				{
					// All vertices will match according to this threshold
					numMatchingVerts = numPossibleJoins;
				}
				else if( numFaceVerts > 64 )
				{
					// For vertices where a very large number of faces meet, it would be very inefficient to
					// test all pairs of face-verts for whether they match. The only reasonable cases for a
					// vertex like this involve radial symmetry, so we'll just test one arbitrarily chosen
					// vert against every other vert, and treat the vertex either as fully smooth
					// ( appropriate for the pole of a sphere ) or fully hard edged ( appropriate for the tip of 
					// a cone )

					bool allMatch = true;
					V3f refNormal = faceVertexNormals[ faceVertices[ startOffset ] ];
					for( int i = 0; i < numFaceVerts; i++ )
					{
						int faceVertI = faceVertices[ startOffset + i ];
						if( !( faceVertexNormals[ faceVertI ].dot( refNormal ) >= cosThreshold ) )
						{
							allMatch = false;
							break;
						}
					}

					numMatchingVerts = allMatch ? numPossibleJoins : 0;
				}
				else
				{
					// We have a non-trivial threshold that some edges will pass and others fail.
					// We need to compute for every face-vertex pair whether their normals are
					// within the threshold
					faceVerticesJoined.resize( numPossibleJoins );

					numMatchingVerts = 0;
					for( int i = 0; i < numFaceVerts; i++ )
					{
						for( int j = i + 1; j < numFaceVerts; j++ )
						{
							int faceVertI = faceVertices[ startOffset + i ];
							int faceVertJ = faceVertices[ startOffset + j ];
							bool join = faceVertexNormals[ faceVertI ].dot( faceVertexNormals[ faceVertJ ] ) >=
								cosThreshold;
							numMatchingVerts += join;
							faceVerticesJoined[ joinedMatrixIndex( i, j, numFaceVerts ) ] = join;
						}
					}
				}

				if( numMatchingVerts == 0 )
				{
					// Nothing matches the requested threshold, each face vertex just keeps
					// it's own normal
				}
				else if( numMatchingVerts == numPossibleJoins )
				{
					// Everything matches the requested threshold, we compute one average normal for the whole
					// vertex

					V3f average( 0.0f );
					for( int i = 0; i < numFaceVerts; i++ )
					{
						int faceVertI = faceVertices[ startOffset + i ];
						average += faceVertexNormals[ faceVertI ] * normalWeights[ faceVertI ];
					}

					average.normalize();

					for( int i = 0; i < numFaceVerts; i++ )
					{
						faceVertexNormals[ faceVertices[ startOffset + i ] ] = average;
					}
				}
				else
				{
					// Some face verts match, some don't. Need to compute a separate average for each face
					// vert of everything that matches it.

					// We need to store the initial face vertex normals while we're overwriting with the
					// averaged normals
					faceVertexNormalsForVert.resize( numFaceVerts );
					for( int i = 0; i < numFaceVerts; i++ )
					{
						int faceVertI = faceVertices[ startOffset + i ];
						faceVertexNormalsForVert[i] = faceVertexNormals[ faceVertI ] * normalWeights[ faceVertI ];
					}

					for( int i = 0; i < numFaceVerts; i++ )
					{
						// Compute average of matching face vertices
						V3f average = faceVertexNormalsForVert[ i ];
						for( int j = 0; j < numFaceVerts; j++ )
						{
							if( j == i )
							{
								continue;
							}

							if( faceVerticesJoined[ joinedMatrixIndex( i, j, numFaceVerts ) ] )
							{
								average += faceVertexNormalsForVert[ j ];
							}
						}

						average.normalize();

						// Write averaged value into output ( it's OK that we're overwriting,
						// because this iteration is the only thing that needs to read the previous value,
						// and we've already cached it to faceVertexNormalsForVert
						faceVertexNormals[ faceVertices[ startOffset + i ] ] = average;
					}
				}

				startOffset = endOffset;
			}
		},
		taskGroupContext
	);

	return PrimitiveVariable( interpolation, faceVertexNormalsData );
}

} // namespace

PrimitiveVariable MeshAlgo::calculateUniformNormals(
	const MeshPrimitive *mesh, const std::string &position, const Canceller *canceller
)
{
	return calculateNormalsImpl(
		mesh, PrimitiveVariable::Uniform, /* unused */ NormalWeighting( 0 ), /* unused */ 0,
		position, canceller
	);
}

PrimitiveVariable MeshAlgo::calculateVertexNormals(
	const MeshPrimitive *mesh, NormalWeighting weighting, const std::string &position, const Canceller *canceller
)
{
	return calculateNormalsImpl(
		mesh, PrimitiveVariable::Vertex, weighting, /* unused */ 0,
		position, canceller
	);
}

PrimitiveVariable MeshAlgo::calculateFaceVaryingNormals(
	const MeshPrimitive *mesh, NormalWeighting weighting, float thresholdAngle, const std::string &position, const Canceller *canceller
)
{
	return calculateNormalsImpl(
		mesh, PrimitiveVariable::FaceVarying, weighting, thresholdAngle,
		position, canceller
	);
}

// Keeping around the inaccurate method solely for backwards compatibility
PrimitiveVariable MeshAlgo::calculateNormals( const MeshPrimitive *mesh, PrimitiveVariable::Interpolation interpolation, const std::string &position, const Canceller *canceller )
{
	const V3fVectorData *pData = mesh->variableData<V3fVectorData>( position, PrimitiveVariable::Vertex );
	if( !pData )
	{
		throw InvalidArgumentException( boost::str( boost::format( "MeshAlgo::calculateNormals : MeshPrimitive has no \"%s\" primitive variable." ) % position ) );
	}
	const std::vector<V3f> &points = pData->readable();

	if( interpolation != PrimitiveVariable::Vertex && interpolation != PrimitiveVariable::Uniform )
	{
		throw InvalidArgumentException( "MeshAlgo::calculateNormals : \"interpolation\" must be Vertex or Uniform" );
	}

	V3fVectorDataPtr normalsData = new V3fVectorData;
	normalsData->setInterpretation( GeometricData::Normal );
	auto &normals = normalsData->writable();

	const auto &verticesPerFace = mesh->verticesPerFace()->readable();
	if( interpolation == PrimitiveVariable::Uniform )
	{
		normals.reserve( verticesPerFace.size() );
	}
	else
	{
		normals.resize( points.size(), Imath::V3f( 0 ) );
	}

	const auto &vertIds = mesh->vertexIds()->readable();
	const int *vertId = &(vertIds[0]);
	for( auto numVerts : verticesPerFace )
	{
		Canceller::check( canceller );
		// calculate the face normal. note that this method is very naive, and doesn't
		// cope with colinear vertices or concave faces - we could use polygonNormal() from
		// PolygonAlgo.h to deal with that, but currently we'd prefer to avoid the overhead.
		const V3f &p0 = points[*vertId];
		const V3f &p1 = points[*(vertId+1)];
		const V3f &p2 = points[*(vertId+2)];

		V3f normal = ( p2 - p1 ).cross( p0 - p1 );
		normal.normalize();

		if( interpolation == PrimitiveVariable::Uniform )
		{
			normals.push_back( normal );
			vertId += numVerts;
		}
		else
		{
			// accumulate the face normal onto each of the vertices for this face.
			for( int i=0; i < numVerts; ++i )
			{
				normals[*vertId] += normal;
				++vertId;
			}
		}
	}

	// normalize each of the vertex normals
	if( interpolation == PrimitiveVariable::Vertex )
	{
		for( size_t i = 0; i < normals.size(); i++ )
		{
			if( i % 1000 )
			{
				Canceller::check( canceller );
			}

			normals[i].normalize();
		}
	}

	return PrimitiveVariable( interpolation, normalsData );
}
