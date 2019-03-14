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
#include "IECore/DespatchTypedData.h"
#include "IECore/TriangleAlgo.h"

#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

namespace
{

/// A functor for use with despatchTypedData, which copies elements from another vector, as specified by an array of indices into that data
struct TriangleDataRemap
{
	typedef void ReturnType;

	TriangleDataRemap( const std::vector<int> &indices ) : m_other( nullptr ), m_indices( indices )
	{
	}

	const Data *m_other;
	const std::vector<int> &m_indices;

	template<typename T>
	void operator()( T *data )
	{
		assert( data );
		typename T::ValueType &dataWritable = data->writable();

		const T *otherData = runTimeCast<const T, const Data>( m_other );
		assert( otherData );
		const typename T::ValueType &otherDataReadable = otherData->readable();

		dataWritable.clear();
		dataWritable.resize( m_indices.size() );

		auto f = [&dataWritable, &otherDataReadable, this]( const tbb::blocked_range<size_t> &r )
		{
			for( size_t i = r.begin(); i != r.end(); ++i )
			{
				dataWritable[i] = otherDataReadable[m_indices[i]];
			}
		};

		const bool useTBB = false;
		if ( useTBB )
		{
			tbb::task_group_context taskGroupContext( tbb::task_group_context::isolated );
			tbb::parallel_for( tbb::blocked_range<size_t>( 0, m_indices.size() ), f, taskGroupContext );
		}
		else
		{
			f( tbb::blocked_range<size_t>( 0, m_indices.size() ) );
		}

		assert( dataWritable.size() == m_indices.size() );
	}
};

/// A simple class to allow MeshAlgo::triangulate to operate on either V3fVectorData or V3dVectorData using
/// despatchTypedData
struct TriangulateFn
{
	typedef void ReturnType;

	MeshPrimitive *m_mesh;
	float m_tolerance;
	bool m_throwExceptions;

	TriangulateFn( MeshPrimitive *mesh, float tolerance, bool throwExceptions ) : m_mesh( mesh ), m_tolerance( tolerance ), m_throwExceptions( throwExceptions )
	{
	}

	template<typename T>
	ReturnType operator()( T *p )
	{
		typedef typename T::ValueType::value_type Vec;

		const typename T::ValueType &pReadable = p->readable();

		MeshPrimitivePtr meshCopy = m_mesh->copy();

		ConstIntVectorDataPtr verticesPerFace = m_mesh->verticesPerFace();
		const std::vector<int> &verticesPerFaceReadable = verticesPerFace->readable();
		ConstIntVectorDataPtr vertexIds = m_mesh->vertexIds();
		const std::vector<int> &vertexIdsReadable = vertexIds->readable();

		IntVectorDataPtr newVertexIds = new IntVectorData();
		std::vector<int> &newVertexIdsWritable = newVertexIds->writable();
		newVertexIdsWritable.reserve( vertexIdsReadable.size() );

		IntVectorDataPtr newVerticesPerFace = new IntVectorData();
		std::vector<int> &newVerticesPerFaceWritable = newVerticesPerFace->writable();
		newVerticesPerFaceWritable.reserve( verticesPerFaceReadable.size() );

		std::vector<int> faceVaryingIndices;
		std::vector<int> uniformIndices;
		int faceVertexIdStart = 0;
		int faceIdx = 0;
		for( IntVectorData::ValueType::const_iterator it = verticesPerFaceReadable.begin(); it != verticesPerFaceReadable.end(); ++it, ++faceIdx )
		{
			int numFaceVerts = *it;

			if( numFaceVerts > 3 )
			{
				/// For the time being, just do a simple triangle fan.

				const int i0 = faceVertexIdStart + 0;
				const int v0 = vertexIdsReadable[i0];

				int i1 = faceVertexIdStart + 1;
				int i2 = faceVertexIdStart + 2;
				int v1 = vertexIdsReadable[i1];
				int v2 = vertexIdsReadable[i2];

				const Vec firstTriangleNormal = triangleNormal( pReadable[v0], pReadable[v1], pReadable[v2] );

				if( m_throwExceptions )
				{
					/// Convexivity test - for each edge, all other vertices must be on the same "side" of it
					for( int i = 0; i < numFaceVerts - 1; i++ )
					{
						const int edgeStartIndex = faceVertexIdStart + i + 0;
						const int edgeStart = vertexIdsReadable[edgeStartIndex];

						const int edgeEndIndex = faceVertexIdStart + i + 1;
						const int edgeEnd = vertexIdsReadable[edgeEndIndex];

						const Vec edge = pReadable[edgeEnd] - pReadable[edgeStart];
						const float edgeLength = edge.length();

						if( edgeLength > m_tolerance )
						{
							const Vec edgeDirection = edge / edgeLength;

							/// Construct a plane whose normal is perpendicular to both the edge and the polygon's normal
							const Vec planeNormal = edgeDirection.cross( firstTriangleNormal );
							const float planeConstant = planeNormal.dot( pReadable[edgeStart] );

							int sign = 0;
							bool first = true;
							for( int j = 0; j < numFaceVerts; j++ )
							{
								const int testVertexIndex = faceVertexIdStart + j;
								const int testVertex = vertexIdsReadable[testVertexIndex];

								if( testVertex != edgeStart && testVertex != edgeEnd )
								{
									float signedDistance = planeNormal.dot( pReadable[testVertex] ) - planeConstant;

									if( fabs( signedDistance ) > m_tolerance )
									{
										int thisSign = 1;
										if( signedDistance < 0.0 )
										{
											thisSign = -1;
										}
										if( first )
										{
											sign = thisSign;
											first = false;
										}
										else if( thisSign != sign )
										{
											assert( sign != 0 );
											throw InvalidArgumentException( "MeshAlgo::triangulate cannot deal with concave polygons" );
										}
									}
								}
							}
						}
					}
				}

				for( int i = 1; i < numFaceVerts - 1; i++ )
				{
					i1 = faceVertexIdStart + ( ( i + 0 ) % numFaceVerts );
					i2 = faceVertexIdStart + ( ( i + 1 ) % numFaceVerts );
					v1 = vertexIdsReadable[i1];
					v2 = vertexIdsReadable[i2];

					if( m_throwExceptions &&
						fabs( triangleNormal( pReadable[v0], pReadable[v1], pReadable[v2] ).dot( firstTriangleNormal ) - 1.0 ) > m_tolerance )
					{
						throw InvalidArgumentException( "MeshAlgo::triangulate cannot deal with non-planar polygons" );
					}

					/// Create a new triangle
					newVerticesPerFaceWritable.push_back( 3 );

					/// Triangulate the vertices
					newVertexIdsWritable.push_back( v0 );
					newVertexIdsWritable.push_back( v1 );
					newVertexIdsWritable.push_back( v2 );

					/// Store the indices required to rebuild the facevarying primvars
					faceVaryingIndices.push_back( i0 );
					faceVaryingIndices.push_back( i1 );
					faceVaryingIndices.push_back( i2 );

					uniformIndices.push_back( faceIdx );
				}
			}
			else
			{
				assert( numFaceVerts == 3 );

				int i0 = faceVertexIdStart + 0;
				int i1 = faceVertexIdStart + 1;
				int i2 = faceVertexIdStart + 2;

				newVerticesPerFaceWritable.push_back( 3 );

				/// Copy across the vertexId data
				newVertexIdsWritable.push_back( vertexIdsReadable[i0] );
				newVertexIdsWritable.push_back( vertexIdsReadable[i1] );
				newVertexIdsWritable.push_back( vertexIdsReadable[i2] );

				/// Store the indices required to rebuild the facevarying primvars
				faceVaryingIndices.push_back( i0 );
				faceVaryingIndices.push_back( i1 );
				faceVaryingIndices.push_back( i2 );

				uniformIndices.push_back( faceIdx );
			}

			faceVertexIdStart += numFaceVerts;
		}

		m_mesh->setTopologyUnchecked( newVerticesPerFace, newVertexIds, pReadable.size(), m_mesh->interpolation() );

		/// Rebuild all the facevarying primvars, using the list of indices into the old data we created above.
		assert( faceVaryingIndices.size() == newVertexIds->readable().size() );
		TriangleDataRemap varyingRemap( faceVaryingIndices );
		TriangleDataRemap uniformRemap( uniformIndices );
		for( PrimitiveVariableMap::iterator it = m_mesh->variables.begin(); it != m_mesh->variables.end(); ++it )
		{
			TriangleDataRemap *remap = nullptr;
			if( it->second.interpolation == PrimitiveVariable::FaceVarying )
			{
				remap = &varyingRemap;
			}
			else if( it->second.interpolation == PrimitiveVariable::Uniform )
			{
				remap = &uniformRemap;
			}
			else
			{
				continue;
			}

			const Data *inputData = it->second.indices ? it->second.indices.get() : it->second.data.get();
			DataPtr result = inputData->copy();
			remap->m_other = inputData;

			despatchTypedData<TriangleDataRemap, TypeTraits::IsVectorTypedData>( result.get(), *remap );

			if( it->second.indices )
			{
				it->second.indices = runTimeCast<IntVectorData>( result );
			}
			else
			{
				it->second.data = result;
			}
		}

		assert( m_mesh->arePrimitiveVariablesValid() );
	}

	struct ErrorHandler
	{
		template<typename T, typename F>
		void operator()( const T *data, const F &functor )
		{
			assert( data );

			throw InvalidArgumentException(
				(
					boost::format( "MeshAlgo::triangulate: Invalid data type \"%s\" for primitive variable \"P\"." ) %
						Object::typeNameFromTypeId( data->typeId() )
				).str()
			);
		}
	};
};

void reportError( const std::string &context, const std::string &message, bool throwExceptions )
{
	if ( throwExceptions )
	{
		throw InvalidArgumentException( boost::str ( boost::format("%s : %s") % context % message ) );
	}
	else
	{
		msg( MessageHandler::Level::Error, context, message );
	}
}

} // namespace

MeshPrimitivePtr MeshAlgo::triangulate(
	const MeshPrimitive *mesh, float tolerance, bool throwExceptions
)
{
	MeshPrimitivePtr meshCopy = mesh->copy();

	if ( !mesh->arePrimitiveVariablesValid() )
	{
		reportError( "MeshAlgo::triangulate", "Mesh with invalid primitive variables ", throwExceptions );

		if ( !throwExceptions )
		{
			return nullptr;
		}
	}

	// already triangulated
	if ( mesh->maxVerticesPerFace() == 3 )
	{
		return meshCopy;
	}

	PrimitiveVariableMap::const_iterator pvIt = meshCopy->variables.find( "P" );
	if( pvIt != meshCopy->variables.end() )
	{
		const DataPtr &verticesData = pvIt->second.data;

		TriangulateFn fn( meshCopy.get(), tolerance, throwExceptions );

		despatchTypedData<TriangulateFn, TypeTraits::IsFloatVec3VectorTypedData, TriangulateFn::ErrorHandler>( verticesData.get(), fn );

		return meshCopy;
	}

	reportError( "MeshAlgo::triangulate", "MeshPrimitive has no \"P\" data", throwExceptions );
	return nullptr;
}
