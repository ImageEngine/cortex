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

#include "IECore/CompoundObject.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/TriangulateOp.h"
#include "IECore/TypedDataDespatch.h"
#include "IECore/TriangleAlgo.h"
#include "IECore/Exception.h"
#include "IECore/CompoundParameter.h"

using namespace IECore;

TriangulateOp::TriangulateOp() : MeshPrimitiveOp( staticTypeName(), "A MeshPrimitiveOp to triangulate a mesh" )
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

FloatParameterPtr TriangulateOp::toleranceParameter()
{
	return m_toleranceParameter;
}

ConstFloatParameterPtr TriangulateOp::toleranceParameter() const
{
	return m_toleranceParameter;
}

BoolParameterPtr TriangulateOp::throwExceptionsParameter()
{
	return m_throwExceptionsParameter;
}

ConstBoolParameterPtr TriangulateOp::throwExceptionsParameter() const
{
	return m_throwExceptionsParameter;
}

struct TriangleDataRemapArgs
{
	TriangleDataRemapArgs( const std::vector<int> &indices ) : m_indices( indices )
	{
	}

	ConstDataPtr m_other;
	const std::vector<int> &m_indices;
};

/// A functor for use with despatchVectorTypedDataFn, which copies elements from another vector, as specified by an array of indices into that data
template<typename T>
struct TriangleDataRemap
{
	size_t operator() ( boost::intrusive_ptr<T> data, TriangleDataRemapArgs args )
	{
		boost::intrusive_ptr<const T> otherData = runTimeCast<const T>( args.m_other );
		assert( otherData );

		data->writable().clear();
		data->writable().reserve( args.m_indices.size() );

		for ( std::vector<int>::const_iterator it = args.m_indices.begin(); it != args.m_indices.end(); ++it )
		{
			data->writable().push_back( otherData->readable()[ *it ] );
		}
		
		assert( data->readable().size() == args.m_indices.size() );

		return data->readable().size();
	}
};

void TriangulateOp::modifyTypedPrimitive( MeshPrimitivePtr mesh, ConstCompoundObjectPtr operands )
{
	bool alreadyTriangulated = true;
	ConstIntVectorDataPtr verticesPerFace = mesh->verticesPerFace();
	IntVectorData::ValueType::const_iterator it = verticesPerFace->readable().begin();
	while ( it != verticesPerFace->readable().end() && alreadyTriangulated )
	{
		if (*it++ != 3)
		{
			alreadyTriangulated = false;
		}
	}

	if ( alreadyTriangulated )
	{
		return;
	}
	
	const float tolerance = toleranceParameter()->getNumericValue();
	bool throwExceptions = boost::static_pointer_cast<const BoolData>(throwExceptionsParameter()->getValue())->readable();
	
	ConstV3fVectorDataPtr p = 0;
	
	PrimitiveVariableMap::const_iterator pvIt = mesh->variables.find("P");
	if (pvIt != mesh->variables.end())
	{
		const DataPtr &verticesData = pvIt->second.data;
		if (runTimeCast<V3fVectorData>(verticesData))
		{
			p = runTimeCast<V3fVectorData>(verticesData);			
		}		
		else
		{
			std::string err = "MeshPrimitive has \"P\" of unsupported type \"";
			err += verticesData->typeName();
			err += "\" in TriangulateOp";
			throw InvalidArgumentException( err );
		}
	}
	else
	{
		throw InvalidArgumentException("MeshPrimitive has no \"P\" data in TriangulateOp");
	}
	assert( p );
	

	ConstIntVectorDataPtr vertexIds = mesh->vertexIds();

	IntVectorDataPtr newVertexIds = new IntVectorData();
	newVertexIds->writable().reserve( vertexIds->readable().size() );

	IntVectorDataPtr newVerticesPerFace = new IntVectorData();
	newVerticesPerFace->writable().reserve( verticesPerFace->readable().size() );

	std::vector<int> faceVaryingIndices;
	int faceVertexIdStart = 0;
	for ( IntVectorData::ValueType::const_iterator it = verticesPerFace->readable().begin(); it != verticesPerFace->readable().end(); ++it )
	{
		int numFaceVerts = *it;

		if ( numFaceVerts > 3 )
		{		
			/// For the time being, just do a simple triangle fan.
			
			const int i0 = faceVertexIdStart + 0;
			const int v0 = vertexIds->readable()[ i0 ]; 
			
			int i1 = faceVertexIdStart + 1;
			int i2 = faceVertexIdStart + 2;
			int v1 = vertexIds->readable()[ i1 ];
			int v2 = vertexIds->readable()[ i2 ];
			
			const Imath::V3f firstTriangleNormal = triangleNormal( p->readable()[ v0 ], p->readable()[ v1 ], p->readable()[ v2 ] );
			
			if (throwExceptions)
			{
				/// Convexivity test - for each edge, all other vertices must be on the same "side" of it
				for (int i = 0; i < numFaceVerts - 1; i++)
				{
					const int edgeStartIndex = faceVertexIdStart + i + 0;
					const int edgeStart = vertexIds->readable()[ edgeStartIndex ];				

					const int edgeEndIndex = faceVertexIdStart + i + 1;
					const int edgeEnd = vertexIds->readable()[ edgeEndIndex ];								

					const Imath::V3f edge = p->readable()[ edgeEnd ] -  p->readable()[ edgeStart ];
					const float edgeLength = edge.length();				

					if (edgeLength > tolerance)
					{
						const Imath::V3f edgeDirection = edge / edgeLength;

						/// Construct a plane whose normal is perpendicular to both the edge and the polygon's normal
						const Imath::V3f planeNormal = edgeDirection.cross( firstTriangleNormal );
						const float planeConstant = planeNormal.dot( p->readable()[ edgeStart ] );	

						int sign = 0;
						bool first = true;
						for (int j = 0; j < numFaceVerts; j++)
						{
							const int testVertexIndex = faceVertexIdStart + j;						
							const int testVertex = vertexIds->readable()[ testVertexIndex ];

							if ( testVertex != edgeStart && testVertex != edgeEnd )
							{
								float signedDistance = planeNormal.dot( p->readable()[ testVertex ] ) - planeConstant;

								if ( fabs(signedDistance) > tolerance)
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
				v1 = vertexIds->readable()[ i1 ];
				v2 = vertexIds->readable()[ i2 ];						
				
				if ( throwExceptions && fabs( triangleNormal( p->readable()[ v0 ], p->readable()[ v1 ], p->readable()[ v2 ] ).dot( firstTriangleNormal ) - 1.0 ) > tolerance )
				{
					throw InvalidArgumentException("TriangulateOp cannot deal with non-planar polygons");
				}

				/// Create a new triangle
				newVerticesPerFace->writable().push_back( 3 );

				/// Triangulate the vertices
				newVertexIds->writable().push_back( v0 );
				newVertexIds->writable().push_back( v1 );
				newVertexIds->writable().push_back( v2 );

				/// Store the indices required to rebuild the facevarying primvars
				faceVaryingIndices.push_back( i0 );
				faceVaryingIndices.push_back( i1 );
				faceVaryingIndices.push_back( i2 );
			}
		}
		else
		{
			assert( numFaceVerts == 3 );

			int i0 = faceVertexIdStart + 0;
			int i1 = faceVertexIdStart + 1;
			int i2 = faceVertexIdStart + 2;

			newVerticesPerFace->writable().push_back( 3 );

			/// Copy across the vertexId data
			newVertexIds->writable().push_back( vertexIds->readable()[ i0 ] );
			newVertexIds->writable().push_back( vertexIds->readable()[ i1 ] );
			newVertexIds->writable().push_back( vertexIds->readable()[ i2 ] );

			/// Store the indices required to rebuild the facevarying primvars
			faceVaryingIndices.push_back( i0 );
			faceVaryingIndices.push_back( i1 );
			faceVaryingIndices.push_back( i2 );
		}

		faceVertexIdStart += numFaceVerts;
	}

	mesh->setTopology( newVerticesPerFace, newVertexIds );

	/// Rebuild all the facevarying primvars, using the list of indices into the old data we created above.
	assert( faceVaryingIndices.size() == newVertexIds->readable().size() );
	TriangleDataRemapArgs args( faceVaryingIndices );
	for ( PrimitiveVariableMap::iterator it = mesh->variables.begin(); it != mesh->variables.end(); ++it )
	{
		if ( it->second.interpolation == PrimitiveVariable::FaceVarying )
		{
			args.m_other = it->second.data;
			DataPtr data = it->second.data->copy();

			size_t primVarSize = despatchVectorTypedDataFn<int, TriangleDataRemap, TriangleDataRemapArgs>( data, args );
			assert( primVarSize == faceVaryingIndices.size() );
			(void)primVarSize;

			it->second.data = data;
		}
	}
}
