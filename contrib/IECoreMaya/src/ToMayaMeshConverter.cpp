//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "maya/MFnMeshData.h"
#include "maya/MFnMesh.h"
#include "maya/MPointArray.h"
#include "maya/MFloatPointArray.h"
#include "maya/MIntArray.h"

#include "IECore/MeshPrimitive.h"

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/ToMayaMeshConverter.h"

using namespace IECoreMaya;

ToMayaMeshConverter::Description ToMayaMeshConverter::g_meshDataDescription( IECore::MeshPrimitive::staticTypeId(), MFn::kMeshData );
ToMayaMeshConverter::Description ToMayaMeshConverter::g_meshDescription( IECore::MeshPrimitive::staticTypeId(), MFn::kMesh );

ToMayaMeshConverter::ToMayaMeshConverter( IECore::ConstObjectPtr object ) 
: ToMayaObjectConverter( "ToMayaMeshConverter", "Converts IECore::MeshPrimitive objects to a Maya object.", object)
{
}

bool ToMayaMeshConverter::doConvert( MObject &obj ) const
{
	MStatus s;
	
	IECore::ConstMeshPrimitivePtr mesh = boost::dynamic_pointer_cast<const IECore::MeshPrimitive>(object());
	assert( mesh );
		
	MFloatPointArray vertexArray;
	MIntArray polygonCounts;
	MIntArray polygonConnects;
	
	MFnMesh fnMesh;
	
	int numVertices = 0;	
	IECore::PrimitiveVariableMap::const_iterator it = mesh->variables.find("P");
	if ( it != mesh->variables.end() )
	{
		IECore::ConstV3fVectorDataPtr p = boost::dynamic_pointer_cast<const IECore::V3fVectorData>(it->second.data);
		if (p)
		{
			numVertices = p->readable().size();

			vertexArray.setLength( numVertices );
			for (int i = 0; i < numVertices; i++)
			{
				vertexArray[i] = IECoreMaya::convert<MFloatPoint, Imath::V3f>( p->readable()[i] );
			}	
		}
		else
		{
			IECore::ConstV3dVectorDataPtr p = boost::dynamic_pointer_cast<const IECore::V3dVectorData>(it->second.data);
			if (p)
			{
				numVertices = p->readable().size();
					
				vertexArray.setLength( numVertices );
				for (int i = 0; i < numVertices; i++)
				{
					vertexArray[i] = IECoreMaya::convert<MFloatPoint, Imath::V3d>( p->readable()[i] );
				}	
			}
			else
			{	
				// "P" is not convertible to an array of "points"
				return false;
			}
		}	
	}

#if 0	
	if (!numVertices)
	{
		return true;
	}
#endif	
						
	IECore::ConstIntVectorDataPtr verticesPerFace = mesh->verticesPerFace();
	int numPolygons = verticesPerFace->readable().size();
	
	polygonCounts.setLength( numPolygons );
	for (int i = 0; i < numPolygons; i++)
	{
		polygonCounts[i] = verticesPerFace->readable()[i];
	}
	
	IECore::ConstIntVectorDataPtr vertexIds = mesh->vertexIds();
	int numPolygonConnects = vertexIds->readable().size();
	polygonConnects.setLength( numPolygonConnects );
	for (int i = 0; i < numPolygonConnects; i++)
	{
		polygonConnects[i] = vertexIds->readable()[i];
	}
			
	fnMesh.create( numVertices, numPolygons, vertexArray, polygonCounts, polygonConnects, obj, &s );
	
	if (!s)
	{			
		return false;
	}
		
	return true;
}
