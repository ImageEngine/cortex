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
#include "IECoreScene/PrimitiveVariable.h"

#include "IECore/PointDistribution.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TriangleAlgo.h"

#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

//////////////////////////////////////////////////////////////////////////
// Distribute Points
//////////////////////////////////////////////////////////////////////////

namespace
{

template< typename T >
void triangleCornerPrimVarValues(
	PrimitiveVariable::Interpolation interpolation, const PrimitiveVariable::IndexedView<T> &view,
	const std::vector<int> &vertexIds,
	int triangleIdx,
	T& a, T& b, T& c
)
{
	switch ( interpolation )
	{
		case PrimitiveVariable::Uniform :
			assert( triangleIdx < (int)view.size() );

			a = b = c = view[ triangleIdx ];
			return;

		case PrimitiveVariable::Vertex :
		case PrimitiveVariable::Varying:
		case PrimitiveVariable::FaceVarying:
		{
			size_t v0I = triangleIdx * 3;
			size_t v1I = v0I + 1;
			size_t v2I = v1I + 1;

			if( interpolation != PrimitiveVariable::FaceVarying )
			{
				v0I = vertexIds[v0I];
				v1I = vertexIds[v1I];
				v2I = vertexIds[v2I];
			}

			assert( v0I < view.size() );
			assert( v1I < view.size() );
			assert( v2I < view.size() );

			a = view[ v0I ];
			b = view[ v1I ];
			c = view[ v2I ];
			return;
		}

		default :
			/// Unimplemented primvar interpolation, or Constant, which doesn't support IndexedView
			assert( false );
	}
}

template< typename T >
T triangleInterpolatedPrimVarValue(
	PrimitiveVariable::Interpolation interpolation, const PrimitiveVariable::IndexedView<T> &view,
	const std::vector<int> &vertexIds,
	int triangleIdx, const V3f &bary
)
{
	switch( interpolation )
	{
		case PrimitiveVariable::Uniform :
			assert( triangleIdx < (int)view.size() );

			return view[ triangleIdx ];

		case PrimitiveVariable::Vertex :
		case PrimitiveVariable::Varying:
		case PrimitiveVariable::FaceVarying:
		{

			T a, b, c;
			triangleCornerPrimVarValues<T>( interpolation, view, vertexIds, triangleIdx, a, b, c );
			return a * bary[0] + b * bary[1] + c * bary[2];
		}

		default :
			/// Unimplemented primvar interpolation, or Constant, which doesn't support IndexedView
			assert( false );
			return T();
	}
}

void processInputs(
	const MeshPrimitive *mesh,
	const std::string &refPosition, const std::string &uvSet, const std::string &densityMask,
	const Canceller *canceller,
	MeshPrimitivePtr &triangulatedMesh,
	PrimitiveVariable &uvVar,
	PrimitiveVariable &faceAreaVar, PrimitiveVariable &textureAreaVar, PrimitiveVariable &densityVar
)
{
	if( !mesh )
	{
		throw InvalidArgumentException( "MeshAlgo::distributePoints : The input mesh is not valid" );
	}

	// \todo - remove unused primvars
	triangulatedMesh = MeshAlgo::triangulate( mesh, canceller );

	if ( !triangulatedMesh || !triangulatedMesh->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "MeshAlgo::distributePoints : The input mesh could not be triangulated" );
	}

	auto uvIt = triangulatedMesh->variables.find( uvSet );
	if( uvIt != triangulatedMesh->variables.end() )
	{
		PrimitiveVariable::Interpolation interp = uvIt->second.interpolation;
		if(
			uvIt->second.data->typeId() == V2fVectorDataTypeId &&
			(
				interp == PrimitiveVariable::Vertex ||
				interp == PrimitiveVariable::Varying ||
				interp == PrimitiveVariable::FaceVarying
			)
		)
		{
			uvVar = uvIt->second;
		}
	}
	if( !uvVar.data )
	{
		std::string e = boost::str( boost::format(
			"MeshAlgo::distributePoints : MeshPrimitive has no uv primitive variable named \"%s\" of type FaceVarying or Vertex." )
		% uvSet );
		throw InvalidArgumentException( e );
	}

	faceAreaVar = MeshAlgo::calculateFaceArea( triangulatedMesh.get(), refPosition, canceller );
	textureAreaVar = MeshAlgo::calculateFaceTextureArea( triangulatedMesh.get(), uvSet, refPosition, canceller );

	auto densityVarIt = triangulatedMesh->variables.find( densityMask );
	if( densityVarIt != triangulatedMesh->variables.end() && ( densityVarIt->second.data->typeId() == FloatVectorDataTypeId || densityVarIt->second.data->typeId() == FloatDataTypeId ) )
	{
		densityVar = densityVarIt->second;
	}
	else
	{
		densityVar = PrimitiveVariable( PrimitiveVariable::Constant, new FloatData( 1.0 ) );
	}
}

struct BaryAndFaceIdx
{
	V3f bary;
	int faceIdx;
};

void distributePointsInTriangle(
	PrimitiveVariable::Interpolation uvInterpolation, const PrimitiveVariable::IndexedView<Imath::V2f> &uvView, const V2f &offset,
	PrimitiveVariable::Interpolation densityInterpolation, const PrimitiveVariable::IndexedView<float> &densityView,
	const std::vector<int> &vertexIds, int faceIdx, float textureDensity,
	std::vector< BaryAndFaceIdx >& results, const Canceller *canceller
)
{
	Imath::V2f uv0, uv1, uv2;
	triangleCornerPrimVarValues< Imath::V2f >( uvInterpolation, uvView, vertexIds, faceIdx, uv0, uv1, uv2 );
	uv0 += offset;
	uv1 += offset;
	uv2 += offset;

	Imath::Box2f uvBounds;
	uvBounds.extendBy( uv0 );
	uvBounds.extendBy( uv1 );
	uvBounds.extendBy( uv2 );

	const float maxCandidatePoints = 1e9;
	const float approxCandidatePoints = uvBounds.size().x * uvBounds.size().y * textureDensity;
	if( ! ( approxCandidatePoints <= maxCandidatePoints ) )
	{
		std::string e = boost::str( boost::format(
			"MeshAlgo::distributePoints : Cannot generate more than %i candidate points per polygon. Trying to generate %i. There are circumstances where the output would be reasonable, but this happens during processing due to a polygon with a large area in 3D space which is extremely thin in UV space, in which case you may need to clean your UVs. Alternatively, maybe you really want to put an extraordinary number of points on one polygon - please subdivide it before distributing points to help with performance." )
		% size_t( maxCandidatePoints ) % size_t( approxCandidatePoints ) );
		throw Exception( e );
	}

	int cancelCounter = 0;

	PointDistribution::defaultInstance()(
		uvBounds, textureDensity,
		[&]( const Imath::V2f pos, float densityThreshold )
		{
			cancelCounter++;
			if( canceller && ( cancelCounter % 1000 ) == 0 )
			{
				Canceller::check( canceller );
			}

			Imath::V3f bary;
			if( triangleContainsPoint( uv0, uv1, uv2, pos, bary ) )
			{
				if( densityView )
				{
					float d = triangleInterpolatedPrimVarValue( densityInterpolation, densityView, vertexIds, faceIdx, bary );
					if( d < densityThreshold )
					{
						return;
					}
				}
				results.push_back( BaryAndFaceIdx{ bary, faceIdx } );
			}
		}
	);
}

} // namespace

PointsPrimitivePtr MeshAlgo::distributePoints( const MeshPrimitive *mesh, float density, const Imath::V2f &offset, const std::string &densityMask, const std::string &uvSet, const std::string &refPosition, const Canceller *canceller )
{
	if( density < 0 )
	{
		throw InvalidArgumentException( "MeshAlgo::distributePoints : The density of the distribution cannot be negative." );
	}

	MeshPrimitivePtr triangulatedMesh;
	PrimitiveVariable uvVar, faceAreaVar, textureAreaVar, densityVar;

	// Make sure we have a triangulated mesh, and valid values for all the primitive variables we need.
	processInputs(
		mesh, refPosition, uvSet, densityMask, canceller,
		triangulatedMesh, uvVar, faceAreaVar, textureAreaVar, densityVar
	);

	PrimitiveVariable::IndexedView<V2f> uvView( uvVar );

	const std::vector<float> &faceArea = static_cast< const FloatVectorData* >( faceAreaVar.data.get() )->readable();
	const std::vector<float> &textureArea = static_cast< const FloatVectorData* >( textureAreaVar.data.get() )->readable();

	PrimitiveVariable::IndexedView<float> densityView;
	if( densityVar.interpolation == PrimitiveVariable::Constant )
	{
		density *= std::min( 1.0f, std::max( 0.0f, (IECore::runTimeCast<FloatData>(densityVar.data.get()))->readable() ) );
	}
	else
	{
		densityView = PrimitiveVariable::IndexedView<float>( densityVar );
	}

	const size_t numFaces = triangulatedMesh->verticesPerFace()->readable().size();
	const std::vector<int> &vertexIds = triangulatedMesh->vertexIds()->readable();

	const int facesPerChunk = std::max( 1, std::min( (int)( numFaces / 100 ), 10000 ) );
	const int numChunks = ( numFaces + facesPerChunk - 1 ) / facesPerChunk;

	std::vector< std::vector<BaryAndFaceIdx > > chunkResults( numChunks );

	tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );

	tbb::parallel_for(
		tbb::blocked_range<size_t>( 0, numChunks ),
		[numFaces, facesPerChunk, &density, &faceArea, &textureArea, &uvVar, &uvView, &offset, &densityVar, &densityView, &vertexIds, &chunkResults, canceller]( const tbb::blocked_range<size_t> &range )
		{
			for ( size_t chunkIndex = range.begin(); chunkIndex != range.end(); ++chunkIndex )
			{
				Canceller::check( canceller );

				int startFace = chunkIndex * facesPerChunk;
				int endFace = std::min( ( chunkIndex + 1 ) * facesPerChunk, numFaces );

				for( int faceIdx = startFace; faceIdx < endFace; faceIdx++ )
				{
					float textureDensity = 0;
					if( textureArea[faceIdx] != 0 )
					{
						textureDensity = density * faceArea[faceIdx] / textureArea[faceIdx];
					}

					// Store the barycentric coordinates and faceIndexes for all points in this triangle
					// into the appropriate chunkResults
					distributePointsInTriangle(
						uvVar.interpolation, uvView, offset,
						densityVar.interpolation, densityView,
						vertexIds, faceIdx, textureDensity,
						chunkResults[chunkIndex], canceller
					);
				}
			}
		},
		taskGroupContext
	);

	// Sum the points output for each chunk so we know where each chunk starts in the output,
	// and the total number of points.
	int numPoints = 0;
	std::vector<int> chunkOffsets( numChunks );
	for( int chunkIndex = 0; chunkIndex < numChunks; chunkIndex++ )
	{
		chunkOffsets[chunkIndex] = numPoints;
		numPoints += chunkResults[chunkIndex].size();
	}

	const PrimitiveVariable::IndexedView<Imath::V3f> pView = triangulatedMesh->variableIndexedView<V3fVectorData>(
		"P", PrimitiveVariable::Interpolation::Vertex, /* throwIfInvalid */ true
	).value();

	Canceller::check( canceller );
	V3fVectorDataPtr pNewData = new V3fVectorData();
	std::vector<V3f> &pNew = pNewData->writable();
	pNew.resize( numPoints );

	// Use the barycentric coordinates to sample all the primitive variables we need
	tbb::parallel_for(
		tbb::blocked_range<size_t>( 0, numChunks ),
		[&chunkResults, &chunkOffsets, &vertexIds, &pView, &pNew, canceller]( const tbb::blocked_range<size_t> &range )
		{
			for ( size_t chunkIndex = range.begin(); chunkIndex != range.end(); ++chunkIndex )
			{
				int outputIndex = chunkOffsets[chunkIndex];
				Canceller::check( canceller );
				for( auto &i : chunkResults[chunkIndex] )
				{
					pNew[outputIndex] = triangleInterpolatedPrimVarValue( PrimitiveVariable::Vertex, pView, vertexIds, i.faceIdx, i.bary );
					outputIndex++;
				}
			}
		},
		taskGroupContext
	);

	return new PointsPrimitive( pNewData );
}
