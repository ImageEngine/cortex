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

#include "IECore/DataAlgo.h"
#include "IECore/PointDistribution.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TriangleAlgo.h"
#include "IECore/TypeTraits.h"

#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"

#include <any>

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
			a = b = c = T(0.0f);
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

class TriangleTester
{
public:
	TriangleTester( const V2f (&points)[3] )
	{
		for( int i = 0; i < 3; i++ )
		{
			V2f a = points[ ( i + 1 ) % 3];
			V2f b = points[ ( i + 2 ) % 3 ];

			// Swap the vertices so that the edge is always handled in the same order, regardless of
			// which triangle it belongs to.  This ensures consistency that if two triangles share an
			// edge, any point will always be one side of the edge or the other
			bool swap;
			if( a.y != b.y )
			{
				swap = a.y > b.y;
			}
			else
			{
				swap = a.x < b.x;
			}

			if( swap )
			{
				V2f temp = a;
				a = b;
				b = temp;
			}

			// Store values that will make it easier to compute the signed area of a triangle formed
			// by connected a point to this edge
			m_edgeOrigins[i] = a;
			m_edgeNormals[i] = V2f( b.y - a.y, a.x - b.x );

			// Compute the signed edge of the triangle relative to this edge.
			// Note that all signed areas in this class are actually stored as 2 times the area
			// of the triangle, since the area of the extended parallelogram is what naturally
			// falls out of the cross product, and all the areas are relative so it doesn't matter
			V2f c = points[ i ];
			float doubleSignedArea = ( c - m_edgeOrigins[i] ).dot( m_edgeNormals[i] );

			// Store which side of the edge is inside the triangle
			m_insideDir[i] = doubleSignedArea >= 0;

			if( i == 0 )
			{
				// Having computed the signed area of the whole triangle to get the sign, we can store it
				// so we have our divisor for when we divide the areas to get barycentric coordinates.
				m_areaNormalize = 1.0f / fabsf( doubleSignedArea );
			}
		}
	}

	inline bool contains( const V2f &p, V3f &bary )
	{
		// Compute the signed areas formed by connecting this point to the 3 edges
		float doubleSignedAreas[3];
		for( int i = 0; i < 3; i++ )
		{
			doubleSignedAreas[i] = ( p - m_edgeOrigins[i] ).dot( m_edgeNormals[i] );
		}

		// To be inside the triangle, all the comparisons must match.  Note that
		// m_insideDir stores which side we need to be on, but the comparison is
		// always a >= comparison that is the same on either side of the edge.
		// This ensures that a point near the edge will appear in exactly one
		// of two triangles sharing the edge.
		if(
			( ( doubleSignedAreas[0] >= 0 ) != m_insideDir[0] ) ||
			( ( doubleSignedAreas[1] >= 0 ) != m_insideDir[1] ) ||
			( ( doubleSignedAreas[2] >= 0 ) != m_insideDir[2] )
		)
		{
			return false;
		}

		// Compute 2 barycentric coordinates by using the ratios of the areas to the total area.
		bary[0] = fabsf( doubleSignedAreas[0] ) * m_areaNormalize;
		bary[1] = fabsf( doubleSignedAreas[1] ) * m_areaNormalize;

		// The 3rd barycentric coordinate is determined by the first two.
		bary[2] = 1.0f - bary[0] - bary[1];

		return true;
	}

private:

	float m_areaNormalize;
	V2f m_edgeOrigins[3];
	V2f m_edgeNormals[3];
	bool m_insideDir[3];
};

void processInputs(
	const MeshPrimitive *mesh,
	const std::string &refPosition, const std::string &uvSet, const std::string &densityMask,
	const StringAlgo::MatchPattern &primitiveVariables,
	const Canceller *canceller,
	MeshPrimitivePtr &processedMesh,
	PrimitiveVariable &uvVar, PrimitiveVariable &densityVar,
	PrimitiveVariable &faceAreaVar, PrimitiveVariable &textureAreaVar
)
{
	if( !mesh )
	{
		throw InvalidArgumentException( "MeshAlgo::distributePoints : The input mesh is not valid" );
	}

	IECoreScene::MeshPrimitivePtr meshWithUsedPrimVars = new MeshPrimitive();

	// We need the topolgy of the source mesh to triangulate it
	meshWithUsedPrimVars->setTopologyUnchecked( mesh->verticesPerFace(), mesh->vertexIds(), mesh->variableSize( PrimitiveVariable::Vertex ), mesh->interpolation() );

	// Note that we do not transfer creases or corners - they do not affect distribution of points. If we were
	// to add support for distributing onto the limit surface of a subdiv, then we might need to keep those ...
	// but that would need to happen on an untriangulated mesh anyway.

	// Transfer the subset of variables that we need
	for( auto var : mesh->variables )
	{
		if(
			var.first == uvSet || var.first == densityMask || var.first == refPosition || var.first == "P" ||
			StringAlgo::matchMultiple( var.first, primitiveVariables )
		)
		{
			meshWithUsedPrimVars->variables[ var.first ] = var.second;
		}
	}

	processedMesh = MeshAlgo::triangulate( meshWithUsedPrimVars.get(), canceller );
	if ( !processedMesh || !processedMesh->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "MeshAlgo::distributePoints : The input mesh could not be triangulated" );
	}

	auto uvIt = processedMesh->variables.find( uvSet );
	if( uvIt != processedMesh->variables.end() )
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

	faceAreaVar = MeshAlgo::calculateFaceArea( processedMesh.get(), refPosition, canceller );
	// It is ambiguous whether to pass "P" or refPosition here - the position argument of
	// calculateFaceTextureArea is not used for anything, and if it was used for something, I'm not sure what
	// it would be
	textureAreaVar = MeshAlgo::calculateFaceTextureArea( processedMesh.get(), uvSet, refPosition, canceller );

	if( !StringAlgo::matchMultiple( uvSet, primitiveVariables ) )
	{
		processedMesh->variables.erase( uvSet );
	}

	if(
		refPosition != "P" &&
		processedMesh->variables.find( refPosition ) != processedMesh->variables.end() &&
		!StringAlgo::matchMultiple( refPosition, primitiveVariables )
	)
	{
		processedMesh->variables.erase( refPosition );
	}

	auto densityVarIt = processedMesh->variables.find( densityMask );
	if( densityVarIt != processedMesh->variables.end() )
	{
		if( densityVarIt->second.data->typeId() == FloatVectorDataTypeId || densityVarIt->second.data->typeId() == FloatDataTypeId )
		{
			densityVar = densityVarIt->second;
		}

		if( !StringAlgo::matchMultiple( densityMask, primitiveVariables ) )
		{
			processedMesh->variables.erase( densityMask );
		}
	}

	if( !densityVar.data )
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
	Imath::V2f uvs[3];
	triangleCornerPrimVarValues< Imath::V2f >( uvInterpolation, uvView, vertexIds, faceIdx, uvs[0], uvs[1], uvs[2] );
	uvs[0] += offset;
	uvs[1] += offset;
	uvs[2] += offset;

	Imath::Box2f uvBounds;
	uvBounds.extendBy( uvs[0] );
	uvBounds.extendBy( uvs[1] );
	uvBounds.extendBy( uvs[2] );

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

	bool hasDensityVar = false;
	float finalDensity = textureDensity;
	float cornerDensities[3];
	if( densityView )
	{
		hasDensityVar = true;
		Imath::V3f bary;
		triangleCornerPrimVarValues(
			densityInterpolation, densityView, vertexIds, faceIdx,
			cornerDensities[0], cornerDensities[1], cornerDensities[2]
		);
		float maxDensity = std::max( 0.0f, std::max( std::max( cornerDensities[0], cornerDensities[1] ), cornerDensities[2] ) );

		// Apply the max density from the primvar to the density passed in to PointDistribution
		finalDensity *= maxDensity;

		// Compensate the corner densities to account for the factor that is already handled when passed in
		// to PointDistribution
		float invMaxDensity = 1.0f / maxDensity;
		cornerDensities[0] *= invMaxDensity;
		cornerDensities[1] *= invMaxDensity;
		cornerDensities[2] *= invMaxDensity;
	}

	TriangleTester triTester( uvs );
	PointDistribution::defaultInstance()(
		uvBounds, finalDensity,
		[&]( const Imath::V2f pos, float densityThreshold )
		{
			cancelCounter++;
			if( canceller && ( cancelCounter % 1000 ) == 0 )
			{
				Canceller::check( canceller );
			}

			Imath::V3f bary;
			if( triTester.contains( pos, bary ) )
			{
				if( hasDensityVar )
				{
					float d =
						bary[0] * cornerDensities[0] +
						bary[1] * cornerDensities[1] +
						bary[2] * cornerDensities[2];

					if( d <= densityThreshold )
					{
						return;
					}
				}
				results.push_back( BaryAndFaceIdx{ bary, faceIdx } );
			}
		}
	);
}

template <typename T>
constexpr bool supportsAddMult()
{
	return std::is_arithmetic_v<T> || TypeTraits::IsColor<T>::value || TypeTraits::IsVec<T>::value || TypeTraits::IsMatrix<T>::value;
}

} // namespace

PointsPrimitivePtr MeshAlgo::distributePoints( const MeshPrimitive *mesh, float density, const Imath::V2f &offset, const std::string &densityMask, const std::string &uvSet, const std::string &refPosition, const StringAlgo::MatchPattern &primitiveVariables, const Canceller *canceller )
{
	if( density < 0 )
	{
		throw InvalidArgumentException( "MeshAlgo::distributePoints : The density of the distribution cannot be negative." );
	}

	MeshPrimitivePtr processedMesh;
	PrimitiveVariable uvVar, faceAreaVar, textureAreaVar, densityVar;

	// Make sure we have a triangulated mesh, and valid values for all the primitive variables we need.
	processInputs(
		mesh, refPosition, uvSet, densityMask, primitiveVariables, canceller,
		processedMesh, uvVar, densityVar, faceAreaVar, textureAreaVar
	);

	PrimitiveVariable::IndexedView<V2f> uvView( uvVar );

	const std::vector<float> &faceArea = static_cast< const FloatVectorData* >( faceAreaVar.data.get() )->readable();
	const std::vector<float> &textureArea = static_cast< const FloatVectorData* >( textureAreaVar.data.get() )->readable();

	PrimitiveVariable::IndexedView<float> densityView;
	if( densityVar.interpolation == PrimitiveVariable::Constant )
	{
		density *= std::max( 0.0f, (IECore::runTimeCast<FloatData>(densityVar.data.get()))->readable() );
	}
	else
	{
		densityView = PrimitiveVariable::IndexedView<float>( densityVar );
	}

	const size_t numFaces = processedMesh->verticesPerFace()->readable().size();
	const std::vector<int> &vertexIds = processedMesh->vertexIds()->readable();

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

	PointsPrimitivePtr result = new PointsPrimitive( numPoints );

	struct ToResample
	{
		PrimitiveVariable::Interpolation sourceInterpolation;
		std::any sourceView;
		IECore::Data *target;
	};
	std::vector< ToResample > toResample;

	for( auto &i : processedMesh->variables )
	{
		if( i.second.interpolation == PrimitiveVariable::Constant )
		{
			result->variables[i.first] = i.second;
		}
		else
		{
			PrimitiveVariable::Interpolation sourceInterpolation = i.second.interpolation;
			dispatch( i.second.data.get(),
				[ &i, numPoints, sourceInterpolation, &result, &toResample]( const auto *sourceData )
				{
					using DataType = typename std::remove_const_t< std::remove_pointer_t< decltype( sourceData ) > >;
					if constexpr ( !TypeTraits::IsVectorTypedData<DataType>::value )
					{
						throw IECore::Exception( "MeshAlgo::distributePoints : Invalid PrimitiveVariable, data is not a vector." );
					}
					else
					{
						using ElementType = typename DataType::ValueType::value_type;
						if(
							sourceInterpolation != PrimitiveVariable::Uniform &&
							!supportsAddMult<ElementType>()
						)
						{
							throw IECore::Exception( "MeshAlgo::distributePoints : Cannot interpolate " + i.first );
						}

						typename DataType::Ptr newData = new DataType;
						newData->writable().resize( numPoints );

						if constexpr( TypeTraits::IsGeometricTypedData< DataType >::value )
						{
							if( i.first == "P" )
							{
								newData->setInterpretation( GeometricData::Point );
							}
							else
							{
								newData->setInterpretation( sourceData->getInterpretation() );
							}
						}

						result->variables[i.first] = PrimitiveVariable( PrimitiveVariable::Vertex, newData );
						toResample.push_back( {
							i.second.interpolation,
							PrimitiveVariable::IndexedView< ElementType >( i.second ),
							newData.get()
						} );
					}
				}
			);
		}
	}

	Canceller::check( canceller );

	// Use the barycentric coordinates to sample all the primitive variables we need
	tbb::parallel_for(
		tbb::blocked_range<size_t>( 0, numChunks ),
		[&chunkResults, &chunkOffsets, &vertexIds, &toResample, canceller]( const tbb::blocked_range<size_t> &range )
		{
			for( auto &var : toResample )
			{
				dispatch( var.target,
					[ &var, &vertexIds, &chunkResults, &chunkOffsets, &range, canceller ]( auto *targetData )
					{
						using DataType = typename std::remove_const_t< std::remove_pointer_t< decltype( targetData ) > >;
						if constexpr ( TypeTraits::IsVectorTypedData<DataType>::value )
						{
							using ElementType = typename DataType::ValueType::value_type;
							const PrimitiveVariable::IndexedView<ElementType> &view = std::any_cast<PrimitiveVariable::IndexedView< ElementType > >( var.sourceView );
							auto &target = targetData->writable();

							Canceller::check( canceller );
							for ( size_t chunkIndex = range.begin(); chunkIndex != range.end(); ++chunkIndex )
							{
								int outputIndex = chunkOffsets[chunkIndex];

								if( var.sourceInterpolation == PrimitiveVariable::Uniform )
								{
									for( auto &i : chunkResults[chunkIndex] )
									{
										target[outputIndex] = view[ i.faceIdx ];
										outputIndex++;
									}
								}
								else if constexpr( supportsAddMult<ElementType>() )
								{
									for( auto &i : chunkResults[chunkIndex] )
									{
										target[outputIndex] = triangleInterpolatedPrimVarValue(
											var.sourceInterpolation, view, vertexIds, i.faceIdx, i.bary
										);
										outputIndex++;
									}
								}
								else
								{
									// This branch never taken, because earlier test throws for non-uniform
									// non-interpolable
								}
							}
						}
					}
				);
			}
		},
		taskGroupContext
	);
	return result;
}
