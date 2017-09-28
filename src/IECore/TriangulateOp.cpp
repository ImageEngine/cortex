//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CompoundObject.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/TriangulateOp.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TriangleAlgo.h"
#include "IECore/Exception.h"
#include "IECore/CompoundParameter.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( TriangulateOp );

TriangulateOp::TriangulateOp() : MeshPrimitiveOp( "A MeshPrimitiveOp to triangulate a mesh" )
{
	m_toleranceParameter = new FloatParameter(
		"tolerance",
		"The floating point tolerance to use for various operations, such as determining planarity of faces",
		1.e-6f,
		0.0f
	);

	m_throwExceptionsParameter = new BoolParameter(
		"throwExceptions",
		"When enabled, exceptions are thrown when invalid geometry is encountered (e.g. non-planar or concave faces).",
		true
	);


	parameters()->addParameter( m_toleranceParameter );
	parameters()->addParameter( m_throwExceptionsParameter );
}

TriangulateOp::~TriangulateOp()
{
}

FloatParameter * TriangulateOp::toleranceParameter()
{
	return m_toleranceParameter.get();
}

const FloatParameter * TriangulateOp::toleranceParameter() const
{
	return m_toleranceParameter.get();
}

BoolParameter * TriangulateOp::throwExceptionsParameter()
{
	return m_throwExceptionsParameter.get();
}

const BoolParameter * TriangulateOp::throwExceptionsParameter() const
{
	return m_throwExceptionsParameter.get();
}

/// A functor for use with despatchTypedData, which copies elements from another vector, as specified by an array of indices into that data
struct TriangleDataRemap
{
	typedef void ReturnType;

	TriangleDataRemap( const std::vector<int> &indices ) : m_other(0), m_indices( indices )
	{
	}

	const Data * m_other;
	const std::vector<int> &m_indices;

	template<typename T>
	void operator() ( T * data )
	{
		assert( data );
		typename T::ValueType &dataWritable = data->writable();

		const T * otherData = runTimeCast<const T, const Data>( m_other );
		assert( otherData );
		const typename T::ValueType &otherDataReadable = otherData->readable();

		dataWritable.clear();
		dataWritable.reserve( m_indices.size() );

		for ( std::vector<int>::const_iterator it = m_indices.begin(); it != m_indices.end(); ++it )
		{
			dataWritable.push_back( otherDataReadable[ *it ] );
		}

		assert( dataWritable.size() == m_indices.size() );
	}
};

/// A simple class to allow TriangulateOp to operate on either V3fVectorData or V3dVectorData using
/// despatchTypedData
struct TriangulateOp::TriangulateFn
{
	typedef void ReturnType;

	MeshPrimitive * m_mesh;
	float m_tolerance;
	bool m_throwExceptions;

	TriangulateFn( MeshPrimitive * mesh, float tolerance, bool throwExceptions )
	: m_mesh( mesh ), m_tolerance( tolerance ), m_throwExceptions( throwExceptions )
	{
	}

	template<typename T>
	ReturnType operator()( T * p )
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
		for ( IntVectorData::ValueType::const_iterator it = verticesPerFaceReadable.begin(); it != verticesPerFaceReadable.end(); ++it, ++faceIdx )
		{
			int numFaceVerts = *it;

			if ( numFaceVerts > 3 )
			{
				/// For the time being, just do a simple triangle fan.

				const int i0 = faceVertexIdStart + 0;
				const int v0 = vertexIdsReadable[ i0 ];

				int i1 = faceVertexIdStart + 1;
				int i2 = faceVertexIdStart + 2;
				int v1 = vertexIdsReadable[ i1 ];
				int v2 = vertexIdsReadable[ i2 ];

				const Vec firstTriangleNormal = triangleNormal( pReadable[ v0 ], pReadable[ v1 ], pReadable[ v2 ] );

				if (m_throwExceptions)
				{
					/// Convexivity test - for each edge, all other vertices must be on the same "side" of it
					for (int i = 0; i < numFaceVerts - 1; i++)
					{
						const int edgeStartIndex = faceVertexIdStart + i + 0;
						const int edgeStart = vertexIdsReadable[ edgeStartIndex ];

						const int edgeEndIndex = faceVertexIdStart + i + 1;
						const int edgeEnd = vertexIdsReadable[ edgeEndIndex ];

						const Vec edge = pReadable[ edgeEnd ] - pReadable[ edgeStart ];
						const float edgeLength = edge.length();

						if (edgeLength > m_tolerance)
						{
							const Vec edgeDirection = edge / edgeLength;

							/// Construct a plane whose normal is perpendicular to both the edge and the polygon's normal
							const Vec planeNormal = edgeDirection.cross( firstTriangleNormal );
							const float planeConstant = planeNormal.dot( pReadable[ edgeStart ] );

							int sign = 0;
							bool first = true;
							for (int j = 0; j < numFaceVerts; j++)
							{
								const int testVertexIndex = faceVertexIdStart + j;
								const int testVertex = vertexIdsReadable[ testVertexIndex ];

								if ( testVertex != edgeStart && testVertex != edgeEnd )
								{
									float signedDistance = planeNormal.dot( pReadable[ testVertex ] ) - planeConstant;

									if ( fabs(signedDistance) > m_tolerance)
									{
										int thisSign = 1;
										if ( signedDistance < 0.0 )
										{
											thisSign = -1;
										}
										if (first)
										{
											sign = thisSign;
											first = false;
										}
										else if ( thisSign != sign )
										{
											assert( sign != 0 );
											throw InvalidArgumentException("TriangulateOp cannot deal with concave polygons");
										}
									}
								}
							}
						}
					}
				}

				for (int i = 1; i < numFaceVerts - 1; i++)
				{
					i1 = faceVertexIdStart + ( (i + 0) % numFaceVerts );
					i2 = faceVertexIdStart + ( (i + 1) % numFaceVerts );
					v1 = vertexIdsReadable[ i1 ];
					v2 = vertexIdsReadable[ i2 ];

					if ( m_throwExceptions && fabs( triangleNormal( pReadable[ v0 ], pReadable[ v1 ], pReadable[ v2 ] ).dot( firstTriangleNormal ) - 1.0 ) > m_tolerance )
					{
						throw InvalidArgumentException("TriangulateOp cannot deal with non-planar polygons");
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
				newVertexIdsWritable.push_back( vertexIdsReadable[ i0 ] );
				newVertexIdsWritable.push_back( vertexIdsReadable[ i1 ] );
				newVertexIdsWritable.push_back( vertexIdsReadable[ i2 ] );

				/// Store the indices required to rebuild the facevarying primvars
				faceVaryingIndices.push_back( i0 );
				faceVaryingIndices.push_back( i1 );
				faceVaryingIndices.push_back( i2 );

				uniformIndices.push_back( faceIdx );
			}

			faceVertexIdStart += numFaceVerts;
		}

		m_mesh->setTopology( newVerticesPerFace, newVertexIds, m_mesh->interpolation() );

		/// Rebuild all the facevarying primvars, using the list of indices into the old data we created above.
		assert( faceVaryingIndices.size() == newVertexIds->readable().size() );
		TriangleDataRemap varyingRemap( faceVaryingIndices );
		TriangleDataRemap uniformRemap( uniformIndices );
		for ( PrimitiveVariableMap::iterator it = m_mesh->variables.begin(); it != m_mesh->variables.end(); ++it )
		{
			TriangleDataRemap *remap = nullptr;
			if ( it->second.interpolation == PrimitiveVariable::FaceVarying )
			{
				remap = &varyingRemap;
			}
			else if ( it->second.interpolation == PrimitiveVariable::Uniform )
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
		void operator()( const T * data, const F& functor )
		{
			assert( data );

			throw InvalidArgumentException( ( boost::format( "TriangulateOp: Invalid data type \"%s\" for primitive variable \"P\"." ) % Object::typeNameFromTypeId( data->typeId() ) ).str() );
                }
        };
};

void TriangulateOp::modifyTypedPrimitive( MeshPrimitive * mesh, const CompoundObject * operands )
{

	if (! mesh->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "Mesh with invalid primitive variables given to TriangulateOp");
	}

	ConstIntVectorDataPtr verticesPerFace = mesh->verticesPerFace();

	if ( mesh->maxVerticesPerFace() == 3 )
	{
		// already triangulated
		return;
	}

	const float tolerance = toleranceParameter()->getNumericValue();
	bool throwExceptions = static_cast<const BoolData *>(throwExceptionsParameter()->getValue())->readable();

	PrimitiveVariableMap::const_iterator pvIt = mesh->variables.find("P");
	if (pvIt != mesh->variables.end())
	{
		const DataPtr &verticesData = pvIt->second.data;
		assert( verticesData );

		TriangulateFn fn( mesh, tolerance, throwExceptions );

		despatchTypedData<
			TriangulateFn,
			TypeTraits::IsFloatVec3VectorTypedData,
			TriangulateFn::ErrorHandler
		>( verticesData.get(), fn );
	}
	else
	{
		throw InvalidArgumentException("TriangulateOp: MeshPrimitive has no \"P\" data");
	}
}
