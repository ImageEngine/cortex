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

using namespace IECore;

TriangulateOp::TriangulateOp() : MeshPrimitiveOp( staticTypeName(), "A MeshPrimitiveOp to triangulate a mesh" )
{
}

TriangulateOp::~TriangulateOp()
{
}

struct TriangleDataAppendArgs
{
	DataPtr m_other;
	int m_i0;
	int m_i1;
	int m_i2;
	
	TriangleDataAppendArgs( DataPtr other, int i0, int i1, int i2 ) : m_other( other ), m_i0( i0 ), m_i1( i1 ), m_i2( i2 )
	{
	}		
};

/// A functor for use with despatchVectorTypedDataFn, which appends three elements of specified index from another vector of the same data type.
template<typename T>
struct TriangleDataAppend
{
	int operator() ( boost::intrusive_ptr<T> data, TriangleDataAppendArgs args )
	{		
		boost::intrusive_ptr<T> otherData = runTimeCast<T>( args.m_other );
		assert( otherData );
		
		data->writable().push_back( otherData->readable()[ args.m_i0 ] );
		data->writable().push_back( otherData->readable()[ args.m_i1 ] );
		data->writable().push_back( otherData->readable()[ args.m_i2 ] );				
		
		/// We have to return something, unfortunately
		return 0;
	}
};	

void TriangulateOp::modifyTypedPrimitive( MeshPrimitivePtr mesh, ConstCompoundObjectPtr operands )
{
	bool alreadyTriangulated = true;
	ConstIntVectorDataPtr verticesPerFace = mesh->verticesPerFace();
	IntVectorData::ValueType::const_iterator it = verticesPerFace->readable().begin();
	while (it != verticesPerFace->readable().end() && alreadyTriangulated )
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
	
	ConstIntVectorDataPtr vertexIds = mesh->vertexIds();	
		
	IntVectorDataPtr newVertexIds = new IntVectorData();
	newVertexIds->writable().reserve( vertexIds->readable().size() );
	
	IntVectorDataPtr newVerticesPerFace = new IntVectorData();	
	newVerticesPerFace->writable().reserve( verticesPerFace->readable().size() );
	
	PrimitiveVariableMap fvPrimVars;
	
	/// We need to "triangulate" every facevarying primvar as well, so create some empty containers ready to hold
	/// the "triangulated" data.
	for (PrimitiveVariableMap::const_iterator it = mesh->variables.begin(); it != mesh->variables.end(); ++it)
	{
		if ( it->second.interpolation == PrimitiveVariable::FaceVarying )
		{
			fvPrimVars[ it->first ] = it->second;
			fvPrimVars[ it->first ].data = fvPrimVars[ it->first ].data->copy();
			
			despatchVectorTypedDataFn<int, VectorTypedDataClear, VectorTypedDataClearArgs>( fvPrimVars[ it->first ].data , VectorTypedDataClearArgs() );
		}
	}
	
	int faceNum = 0;
	int faceVertexIdStart = 0;
	for (IntVectorData::ValueType::const_iterator it = verticesPerFace->readable().begin(); it != verticesPerFace->readable().end(); ++it)
	{
		int numFaceVerts = *it;
		
		if ( numFaceVerts > 3 )
		{
			const int i0 = faceVertexIdStart + 0;
			const int v0 = vertexIds->readable()[ i0 ];
			
			/// For the time being, just do a simple triangle fan. 			
			for (int i = 1; i < numFaceVerts - 1; i++)
			{		
				int i1 = faceVertexIdStart + ( (i + 0) % numFaceVerts );
				int i2 = faceVertexIdStart + ( (i + 1) % numFaceVerts );	
				int v1 = vertexIds->readable()[ i1 ];
				int v2 = vertexIds->readable()[ i2 ];
				
				/// Create a new triangle
				newVerticesPerFace->writable().push_back( 3 );
				
				/// Triangulate the vertices
				newVertexIds->writable().push_back( v0 );
				newVertexIds->writable().push_back( v1 );
				newVertexIds->writable().push_back( v2 );	
				
				/// "Triangulate" the face-varying primvar data too
				/// \todo We can defer the despatched VectorTypedDataFn until the end if we merely maintain a list of
				/// indices to copy. This will probably speed things up a little.
				for (PrimitiveVariableMap::iterator it = fvPrimVars.begin(); it != fvPrimVars.end(); ++it)
				{
					TriangleDataAppendArgs args( mesh->variables[it->first].data, i0, i1, i2 );
					despatchVectorTypedDataFn<int, TriangleDataAppend, TriangleDataAppendArgs>( it->second.data, args );
				}
			}			
		}
		else 
		{
			assert( numFaceVerts == 3 );
			
			newVerticesPerFace->writable().push_back( 3 );
			
			int i0 = faceVertexIdStart + 0;
			int i1 = faceVertexIdStart + 1;
			int i2 = faceVertexIdStart + 2;
			
			/// Copy across the vertexId data						
			newVertexIds->writable().push_back( vertexIds->readable()[ i0 ] );
			newVertexIds->writable().push_back( vertexIds->readable()[ i1 ] );
			newVertexIds->writable().push_back( vertexIds->readable()[ i2 ] );
			
			/// Copy across the face-varying primvar data too
			/// \todo We can defer the despatched VectorTypedDataFn until the end if we merely maintain a list of
			/// indices to copy. This will probably speed things up a little.
			for (PrimitiveVariableMap::iterator it = fvPrimVars.begin(); it != fvPrimVars.end(); ++it)
			{
				TriangleDataAppendArgs args( mesh->variables[it->first].data, i0, i1, i2 );
				despatchVectorTypedDataFn<int, TriangleDataAppend, TriangleDataAppendArgs>( it->second.data, args );
			}
		}
		
		faceVertexIdStart += numFaceVerts;
		faceNum++;
	}

	mesh->setTopology( newVerticesPerFace, newVertexIds );
	
	for (PrimitiveVariableMap::iterator it = fvPrimVars.begin(); it != fvPrimVars.end(); ++it)
	{
		mesh->variables[ it->first ].data = it->second.data;
	}	
}
