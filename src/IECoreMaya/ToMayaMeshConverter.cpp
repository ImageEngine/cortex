//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "boost/format.hpp"

#include "maya/MFnMeshData.h"
#include "maya/MFnMesh.h"
#include "maya/MPointArray.h"
#include "maya/MFloatPointArray.h"
#include "maya/MFloatVectorArray.h"
#include "maya/MFloatArray.h"
#include "maya/MIntArray.h"

#include "IECore/MeshPrimitive.h"
#include "IECore/PrimitiveVariable.h"
#include "IECore/MessageHandler.h"

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/ToMayaMeshConverter.h"

using namespace IECoreMaya;

ToMayaMeshConverter::Description ToMayaMeshConverter::g_meshDataDescription( IECore::MeshPrimitive::staticTypeId(), MFn::kMeshData );
ToMayaMeshConverter::Description ToMayaMeshConverter::g_meshDescription( IECore::MeshPrimitive::staticTypeId(), MFn::kMesh );

ToMayaMeshConverter::ToMayaMeshConverter( IECore::ConstObjectPtr object ) 
: ToMayaObjectConverter( "ToMayaMeshConverter", "Converts IECore::MeshPrimitive objects to a Maya object.", object)
{
}

void ToMayaMeshConverter::addUVSet( MFnMesh &fnMesh, const MIntArray &polygonCounts, IECore::ConstMeshPrimitivePtr mesh, const std::string &sPrimVarName, const std::string &tPrimVarName, MString *uvSetName ) const
{	
	IECore::PrimitiveVariableMap::const_iterator sIt = mesh->variables.find( sPrimVarName );
	bool haveS = sIt != mesh->variables.end();
	IECore::PrimitiveVariableMap::const_iterator tIt = mesh->variables.find( tPrimVarName );	
	bool haveT = tIt != mesh->variables.end();
	
	if ( haveS && haveT )
	{
		if ( sIt->second.interpolation != IECore::PrimitiveVariable::FaceVarying )
		{
			IECore::msg( IECore::Msg::Warning,"ToMayaMeshConverter::doConversion",  boost::format(  "PrimitiveVariable \"%s\" has unsupported interpolation (expected FaceVarying).") % sPrimVarName );
			return;
		}

		if ( tIt->second.interpolation != IECore::PrimitiveVariable::FaceVarying )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", boost::format( "PrimitiveVariable \"%s\" has unsupported interpolation (expected FaceVarying).") % tPrimVarName);
			return;
		}

		if ( !sIt->second.data )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", boost::format( "PrimitiveVariable \"%s\" has no data." ) % sPrimVarName );
		}

		if ( !tIt->second.data )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", boost::format( "PrimitiveVariable \"%s\" has no data." ) % tPrimVarName );
		}

		/// \todo Employ some M*Array converters to simplify this
		int numUVs = mesh->variableSize( IECore::PrimitiveVariable::FaceVarying );											

		MFloatArray uArray;												
		IECore::ConstFloatVectorDataPtr u = IECore::runTimeCast<const IECore::FloatVectorData>(sIt->second.data);

		if ( !u )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", boost::format( "PrimitiveVariable \"%s\" has unsupported type \"%s\"." ) % sPrimVarName % sIt->second.data->typeName() );
			return;
		}

		assert( (int)u->readable().size() == numUVs );
		uArray.setLength( numUVs );

		MFloatArray vArray;
		IECore::ConstFloatVectorDataPtr v = IECore::runTimeCast<const IECore::FloatVectorData>(tIt->second.data);
		if ( !v )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", boost::format( "PrimitiveVariable \"%s\" has unsupported type \"%s\"." ) % tPrimVarName % tIt->second.data->typeName() );
			return;
		}

		assert( (int)v->readable().size() == numUVs );
		vArray.setLength( numUVs );

	
		/// \todo The FromMayaMeshConverter inverts the UVs - shouldn't we do the same here?
		MIntArray uvIds;
		uvIds.setLength( numUVs );
		for ( int i = 0; i < numUVs; i++)
		{
			uArray[i] = u->readable()[i];
			vArray[i] = v->readable()[i];					
			uvIds[i] = i;
		}

		if ( uvSetName )
		{
			fnMesh.createUVSetWithName( *uvSetName );
		}

		MStatus s = fnMesh.setUVs( uArray, vArray, uvSetName );
		if ( !s )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", "Failed to set UVs." );
			return;
		}

		fnMesh.assignUVs( polygonCounts, uvIds, uvSetName );
		if ( !s )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", "Failed to assign UVs." );
			return;			
		}		
	}
	else if ( haveS )		
	{
		IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", boost::format( "Primitive variable \"%s\" found, but not \"%s\"." ) % sPrimVarName % tPrimVarName );
	}
	else if ( haveT )		
	{
		IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", boost::format( "Primitive variable \"%s\" found, but not \"%s\"." ) % tPrimVarName % sPrimVarName );
	}
	else
	{
		assert( !uvSetName );
	}
}

bool ToMayaMeshConverter::doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus s;
	
	IECore::ConstMeshPrimitivePtr mesh = IECore::runTimeCast<const IECore::MeshPrimitive>( from );
	assert( mesh );
	
	if ( !mesh->arePrimitiveVariablesValid() )
	{
		return false;
	}
		
	MFloatPointArray vertexArray;
	MIntArray polygonCounts;
	MIntArray polygonConnects;
	
	MFnMesh fnMesh;
	
	int numVertices = 0;	
	IECore::PrimitiveVariableMap::const_iterator it = mesh->variables.find("P");
	if ( it != mesh->variables.end() )
	{
		/// \todo Employ some M*Array converters to simplify this
		IECore::ConstV3fVectorDataPtr p = IECore::runTimeCast<const IECore::V3fVectorData>(it->second.data);
		if (p)
		{
			numVertices = p->readable().size();

			vertexArray.setLength( numVertices );
			for (int i = 0; i < numVertices; i++)
			{
				vertexArray[i] = IECore::convert<MFloatPoint, Imath::V3f>( p->readable()[i] );
			}	
		}
		else
		{
			IECore::ConstV3dVectorDataPtr p = IECore::runTimeCast<const IECore::V3dVectorData>(it->second.data);
			if (p)
			{
				numVertices = p->readable().size();
					
				vertexArray.setLength( numVertices );
				for (int i = 0; i < numVertices; i++)
				{
					vertexArray[i] = IECore::convert<MFloatPoint, Imath::V3d>( p->readable()[i] );
				}	
			}
			else
			{	
				// "P" is not convertible to an array of "points"
				return false;
			}
		}	
	}

						
	IECore::ConstIntVectorDataPtr verticesPerFace = mesh->verticesPerFace();
	assert( verticesPerFace );
	int numPolygons = verticesPerFace->readable().size();
	
	polygonCounts.setLength( numPolygons );
	for (int i = 0; i < numPolygons; i++)
	{
		polygonCounts[i] = verticesPerFace->readable()[i];
	}
	
	IECore::ConstIntVectorDataPtr vertexIds = mesh->vertexIds();
	assert( vertexIds );
	int numPolygonConnects = vertexIds->readable().size();
	polygonConnects.setLength( numPolygonConnects );
	for (int i = 0; i < numPolygonConnects; i++)
	{
		polygonConnects[i] = vertexIds->readable()[i];
	}
			
	fnMesh.create( numVertices, numPolygons, vertexArray, polygonCounts, polygonConnects, to, &s );
	
	if (!s)
	{			
		return false;
	}
	
	it = mesh->variables.find("N");
	if ( it != mesh->variables.end() )
	{
		if (it->second.interpolation == IECore::PrimitiveVariable::FaceVarying )
		{	
			/// \todo Employ some M*Array converters to simplify this		
			MFloatVectorArray vertexNormalsArray;
			IECore::ConstV3fVectorDataPtr n = IECore::runTimeCast<const IECore::V3fVectorData>(it->second.data);
			if (n)
			{
				int numVertexNormals = n->readable().size();

				vertexNormalsArray.setLength( numVertexNormals );
				for (int i = 0; i < numVertexNormals; i++)
				{
					vertexNormalsArray[i] = IECore::convert<MFloatVector, Imath::V3f>( n->readable()[i] );
				}	

				fnMesh.setNormals( vertexNormalsArray );			
			}
			else
			{
				IECore::ConstV3dVectorDataPtr n = IECore::runTimeCast<const IECore::V3dVectorData>(it->second.data);
				if (n)
				{
					int numVertexNormals = n->readable().size();

					vertexNormalsArray.setLength( numVertexNormals );
					for (int i = 0; i < numVertexNormals; i++)
					{
						vertexNormalsArray[i] = IECore::convert<MFloatVector, Imath::V3d>( n->readable()[i] );
					}

					fnMesh.setNormals( vertexNormalsArray );

				}
				else
				{
					IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", boost::format( "PrimitiveVariable \"N\" has unsupported type \"%s\"." ) % it->second.data->typeName() );
				}				
			}			
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", "PrimitiveVariable \"N\" has unsupported interpolation (expected FaceVarying)." );
		}	
	}
		
	/// Add default UV set	
	addUVSet( fnMesh, polygonCounts, mesh, "s", "t" );
	
	/// Add other UV sets
	std::set< std::string > uvSets;
	for ( it = mesh->variables.begin(); it != mesh->variables.end(); ++it )
	{
		const std::string &sName = it->first;
		
		size_t suffixOffset = sName.rfind( "_s" );
		
		if ( ( suffixOffset != std::string::npos) && ( suffixOffset == sName.length() - 2 ) )
		{
			std::string uvSetNameStr = sName.substr( 0, suffixOffset );
			
			if ( uvSetNameStr.size() )
			{
				MString uvSetName = uvSetNameStr.c_str();
				std::string tName = uvSetNameStr + "_t";

				addUVSet( fnMesh, polygonCounts, mesh, sName, tName, &uvSetName );

				uvSets.insert( uvSetNameStr );						
			}
		}
	}
	
	/// We do the search again, but looking for primvars ending "_t", so we can catch cases where either "UVSETNAME_s" or "UVSETNAME_t" is present, but not both, taking care
	/// not to attempt adding any duplicate sets
	for ( it = mesh->variables.begin(); it != mesh->variables.end(); ++it )
	{
		const std::string &tName = it->first;
		
		size_t suffixOffset = tName.rfind( "_t" );
		
		if ( ( suffixOffset != std::string::npos) && ( suffixOffset == tName.length() - 2 ) )
		{
			std::string uvSetNameStr = tName.substr( 0, suffixOffset );
			
			if ( uvSetNameStr.size() && uvSets.find( uvSetNameStr ) == uvSets.end() )
			{			
				MString uvSetName = uvSetNameStr.c_str();
				std::string sName = uvSetNameStr + "_s";
			
				addUVSet( fnMesh, polygonCounts, mesh, sName, tName, &uvSetName );
				uvSets.insert( uvSetNameStr );	
			}
		}
	}
	
	/// \todo Other primvars, e.g. vertex color ("Cs")
	
	return true;
}
