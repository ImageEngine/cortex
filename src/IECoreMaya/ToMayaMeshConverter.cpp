//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#include "maya/MDagPath.h"
#include "maya/MFnMeshData.h"
#include "maya/MFnMesh.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MPointArray.h"
#include "maya/MFloatPointArray.h"
#include "maya/MFloatVectorArray.h"
#include "maya/MFloatArray.h"
#include "maya/MIntArray.h"
#include "maya/MItMeshPolygon.h"
#include "maya/MGlobal.h"
#include "maya/MPlug.h"
#include "maya/MFnEnumAttribute.h"

#include "IECore/MeshPrimitive.h"
#include "IECore/PrimitiveVariable.h"
#include "IECore/MessageHandler.h"

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/ToMayaMeshConverter.h"
#include "IECoreMaya/FromMayaMeshConverter.h"

using namespace IECoreMaya;

ToMayaMeshConverter::Description ToMayaMeshConverter::g_meshDataDescription( IECore::MeshPrimitive::staticTypeId(), MFn::kMeshData );
ToMayaMeshConverter::Description ToMayaMeshConverter::g_meshDescription( IECore::MeshPrimitive::staticTypeId(), MFn::kMesh );

ToMayaMeshConverter::ToMayaMeshConverter( IECore::ConstObjectPtr object )
: ToMayaObjectConverter( "Converts IECore::MeshPrimitive objects to a Maya object.", object)
{
}

void ToMayaMeshConverter::assignDefaultShadingGroup( MObject &shape ) const
{
	MFnDagNode fnDN( shape );
	// it would perhaps be preferable to use MFnSet::addMember() instead but at the time of
	// writing (maya 2010) that seems to print out "Result : initialShadingGroup" totally unnecessarily.
	MGlobal::executeCommand( "sets -addElement initialShadingGroup " + fnDN.fullPathName() );
}

void ToMayaMeshConverter::addUVSet( MFnMesh &fnMesh, const MIntArray &polygonCounts, IECore::ConstMeshPrimitivePtr mesh, const std::string &sPrimVarName, const std::string &tPrimVarName, const std::string &stIdPrimVarName, MString *uvSetName ) const
{
	IECore::PrimitiveVariableMap::const_iterator sIt = mesh->variables.find( sPrimVarName );
	bool haveS = sIt != mesh->variables.end();
	IECore::PrimitiveVariableMap::const_iterator tIt = mesh->variables.find( tPrimVarName );
	bool haveT = tIt != mesh->variables.end();
	IECore::PrimitiveVariableMap::const_iterator stIdIt = mesh->variables.find( stIdPrimVarName );
	bool haveSTId = stIdIt != mesh->variables.end();

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

		IECore::ConstFloatVectorDataPtr u = IECore::runTimeCast<const IECore::FloatVectorData>(sIt->second.data);

		if ( !u )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", boost::format( "PrimitiveVariable \"%s\" has unsupported type \"%s\"." ) % sPrimVarName % sIt->second.data->typeName() );
			return;
		}

		assert( (int)u->readable().size() == numUVs );
		
		IECore::ConstFloatVectorDataPtr v = IECore::runTimeCast<const IECore::FloatVectorData>(tIt->second.data);
		if ( !v )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", boost::format( "PrimitiveVariable \"%s\" has unsupported type \"%s\"." ) % tPrimVarName % tIt->second.data->typeName() );
			return;
		}

		assert( (int)v->readable().size() == numUVs );
		
		const std::vector<float> &uAll = u->readable();
		const std::vector<float> &vAll = v->readable();

		if ( uvSetName )
		{
			bool setExists = false;
			MStringArray existingSets;
			fnMesh.getUVSetNames( existingSets );
			for ( unsigned i=0; i < existingSets.length(); ++i )
			{
				if ( *uvSetName == existingSets[i] )
				{
					fnMesh.clearUVs( uvSetName );
					setExists = true;
					break;
				}
			}
			
			if ( !setExists )
			{
				MDagPath dag;
				MStatus s = fnMesh.getPath( dag );
				if ( s )
				{
					fnMesh.createUVSetWithName( *uvSetName );
				}
				else
				{
					fnMesh.createUVSetDataMeshWithName( *uvSetName );
				}
			}
		}
		
		MIntArray uvIds;
		uvIds.setLength( numUVs );
		MFloatArray uArray;
		MFloatArray vArray;
		
		if( haveSTId )
		{
			// Get compressed uv values by matching them with their uvId.
			IECore::ConstIntVectorDataPtr uvId = IECore::runTimeCast<const IECore::IntVectorData>(stIdIt->second.data);
			if ( !uvId )
			{
				IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", boost::format( "PrimitiveVariable \"%s\" has unsupported type \"%s\"." ) % stIdPrimVarName % stIdIt->second.data->typeName() );
				return;
			}
			
			const std::vector<int> &uvIdData = uvId->readable();
	
			assert( (int)uvIdData.size() == numUVs );
			
			int highestId = 0;
			for ( int i = 0; i < numUVs; i++)
			{
				uvIds[i] = uvIdData[i];
				if( uvIdData[i] > highestId )
				{
					highestId = uvIdData[i];
				}
			}
			
			// u and v arrays need only be as long as the number of unique uvIds
			uArray.setLength( highestId + 1 );
			vArray.setLength( highestId + 1 );
			
			for ( int i = 0; i < numUVs; i++ )
			{
				uArray[ uvIds[i] ] = uAll[i];
				vArray[ uvIds[i] ] = vAll[i];
			}
		}
		else
		{
			// If for some reason we cannot find the uv indices, set the UVs using the old way
			// the performances in maya won't be good (for weigth painting in particular)
			uArray.setLength( numUVs );
			vArray.setLength( numUVs );

			for ( int i = 0; i < numUVs; i++)
			{
				uArray[i] = u->readable()[i];
				vArray[i] = v->readable()[i];
			}
			
			for ( int i = 0; i < numUVs; i++)
			{
				uvIds[i] = i;
			}
		}
		
		MStatus s = fnMesh.setUVs( uArray, vArray, uvSetName );
		if ( !s )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", "Failed to set UVs." );
			return;
		}

		s = fnMesh.assignUVs( polygonCounts, uvIds, uvSetName );
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

	MObject mObj = fnMesh.create( numVertices, numPolygons, vertexArray, polygonCounts, polygonConnects, to, &s );

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
			MVectorArray vertexNormalsArray;
			IECore::ConstV3fVectorDataPtr n = IECore::runTimeCast<const IECore::V3fVectorData>(it->second.data);
			if (n)
			{
				int numVertexNormals = n->readable().size();

				vertexNormalsArray.setLength( numVertexNormals );
				for (int i = 0; i < numVertexNormals; i++)
				{
					vertexNormalsArray[i] = IECore::convert<MVector, Imath::V3f>( n->readable()[i] );
				}
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
						vertexNormalsArray[i] = IECore::convert<MVector, Imath::V3d>( n->readable()[i] );
					}
				}
				else
				{
					IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", boost::format( "PrimitiveVariable \"N\" has unsupported type \"%s\"." ) % it->second.data->typeName() );
				}
			}
			
			if ( vertexNormalsArray.length() )
			{
				MStatus status;
				MItMeshPolygon itPolygon( mObj, &status );
				if( status != MS::kSuccess )
				{
					IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", "Failed to create mesh iterator" );
				}

				unsigned v = 0;
				MIntArray vertexIds;
				MIntArray faceIds;
				
				for ( ; !itPolygon.isDone(); itPolygon.next() )
				{
					for ( v=0; v < itPolygon.polygonVertexCount(); ++v )
					{
						faceIds.append( itPolygon.index() );
						vertexIds.append( itPolygon.vertexIndex( v ) );
					}
				}

				if( !fnMesh.setFaceVertexNormals( vertexNormalsArray, faceIds, vertexIds ) )
				{
					IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", "Setting normals failed" );
				}
			}
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", "PrimitiveVariable \"N\" has unsupported interpolation (expected FaceVarying)." );
		}
	}

	bool haveDefaultUVs = false;
	IECore::PrimitiveVariableMap::const_iterator sIt = mesh->variables.find( "s" );
	IECore::RefCountedPtr sDataRef = ( sIt == mesh->variables.end() ) ? 0 : static_cast<IECore::RefCountedPtr>( sIt->second.data );

	/// Add named UV sets
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
				std::string stIdName = uvSetNameStr + "Indices";

				addUVSet( fnMesh, polygonCounts, mesh, sName, tName, stIdName, &uvSetName );

				uvSets.insert( uvSetNameStr );
				
				if ( sDataRef == static_cast<IECore::RefCountedPtr>( it->second.data ) )
				{
					haveDefaultUVs = true;
				}
			}
		}
	}

	/// Add default UV set if it isn't just a reference to a named set
	if ( !haveDefaultUVs )
	{
		addUVSet( fnMesh, polygonCounts, mesh, "s", "t", "stIndices" );
	}

	// We do the search again, but looking for primvars ending "_t", so we can catch cases where either "UVSETNAME_s" or "UVSETNAME_t" is present, but not both, taking care
	// not to attempt adding any duplicate sets
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
				std::string stIdName = uvSetNameStr + "Indices";
				
				addUVSet( fnMesh, polygonCounts, mesh, sName, tName, stIdName, &uvSetName );
				uvSets.insert( uvSetNameStr );
			}
		}
	}

	/// If we're making a mesh node (rather than a mesh data) then make sure it belongs
	/// to the default shading group and add the ieMeshInterpolation attribute.
	MObject oMesh = fnMesh.object();
	if( oMesh.apiType()==MFn::kMesh )
	{
		assignDefaultShadingGroup( oMesh );
		setMeshInterpolationAttribute( oMesh, mesh->interpolation() );
	}

	/// \todo Other primvars, e.g. vertex color ("Cs")

	return true;
}

bool ToMayaMeshConverter::setMeshInterpolationAttribute( MObject &object, std::string interpolation )
{
	MStatus st;
	MFnDependencyNode fnDep(object, &st);
	if ( !st )
	{
		return false;
	}

	int interpolationValue = 0;

	FromMayaMeshConverter fromMaya(object);
	const IECore::Parameter::PresetsContainer &presets = fromMaya.interpolationParameter()->getPresets();
	IECore::Parameter::PresetsContainer::const_iterator it;

	if ( interpolation != "default" )
	{
		int index = 0;

		for ( it = presets.begin(); it != presets.end(); it++, index++ )
		{
			if ( interpolation == it->first || interpolation == boost::static_pointer_cast< IECore::StringData >(it->second)->readable() )
			{
				interpolationValue = index;
				break;
			}
		}
		if ( it == presets.end() )
		{
			return false;
		}
	}

	MPlug interpPlug = fnDep.findPlug( "ieMeshInterpolation", &st );
	
	if ( !st )
	{
		MFnEnumAttribute fnAttrib;
		MObject newAttr = fnAttrib.create( "ieMeshInterpolation", "interp", 0, &st );
		if ( !st )
		{
			return false;
		}

		int index = 0;
		for ( it = presets.begin(); it != presets.end(); it++ )
		{
			if ( it->first == "default" )
			{
				continue;
			}
			fnAttrib.addField( it->first.c_str(), index );
			index++;
		}

		// looks like the attribute does not exist yet..
		st = fnDep.addAttribute( newAttr );
		if ( !st )
		{
			return false;
		}
		interpPlug = fnDep.findPlug( "ieMeshInterpolation", &st );
		if ( !st )
		{
			return false;
		}
	}

	st = interpPlug.setValue( interpolationValue );
	if ( !st )
	{
		return false;
	}

	return true;
}
