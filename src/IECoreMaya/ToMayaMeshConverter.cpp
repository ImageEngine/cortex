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
#include "maya/MUintArray.h"
#include "maya/MItMeshVertex.h"

#include "IECore/MessageHandler.h"
#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/PrimitiveVariable.h"

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/ToMayaMeshConverter.h"
#include "IECoreMaya/FromMayaMeshConverter.h"

using namespace IECoreMaya;

ToMayaMeshConverter::Description ToMayaMeshConverter::g_meshDataDescription( IECoreScene::MeshPrimitive::staticTypeId(), MFn::kMeshData );
ToMayaMeshConverter::Description ToMayaMeshConverter::g_meshDescription( IECoreScene::MeshPrimitive::staticTypeId(), MFn::kMesh );

ToMayaMeshConverter::ToMayaMeshConverter( IECore::ConstObjectPtr object )
: ToMayaObjectConverter( "Converts IECoreScene::MeshPrimitive objects to a Maya object.", object)
{
}

void ToMayaMeshConverter::assignDefaultShadingGroup( MObject &shape ) const
{
	MFnDagNode fnDN( shape );
	// it would perhaps be preferable to use MFnSet::addMember() instead but at the time of
	// writing (maya 2010) that seems to print out "Result : initialShadingGroup" totally unnecessarily.
	MGlobal::executeCommand( "sets -addElement initialShadingGroup " + fnDN.fullPathName() );
}

void ToMayaMeshConverter::addUVSet( MFnMesh &fnMesh, const MIntArray &polygonCounts, const IECoreScene::MeshPrimitive *mesh, IECoreScene::PrimitiveVariableMap::const_iterator &uvIt ) const
{
	// Maya's default UV set is named "map1" instead of "uv"
	MString uvSetName = uvIt->first == "uv" ? "map1" : uvIt->first.c_str();

	if( uvIt->second.interpolation != IECoreScene::PrimitiveVariable::FaceVarying )
	{
		IECore::msg( IECore::Msg::Warning,"ToMayaMeshConverter::doConversion",  boost::format(  "PrimitiveVariable \"%s\" has unsupported interpolation (expected FaceVarying).") % uvSetName.asChar() );
		return;
	}

	const IECore::V2fVectorData *uvData = IECore::runTimeCast<const IECore::V2fVectorData>( uvIt->second.data.get() );
	if ( !uvData )
	{
		IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", boost::format( "PrimitiveVariable \"%s\" has unsupported type \"%s\"." ) % uvSetName.asChar() % uvIt->second.data->typeName() );
		return;
	}

	bool setExists = false;
	MStringArray existingSets;
	fnMesh.getUVSetNames( existingSets );
	for ( unsigned i=0; i < existingSets.length(); ++i )
	{
		if( uvSetName == existingSets[i] )
		{
			fnMesh.clearUVs( &uvSetName );
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
			fnMesh.createUVSetWithName( uvSetName );
		}
		else
		{
			fnMesh.createUVSetDataMeshWithName( uvSetName );
		}
	}

	const std::vector<Imath::V2f> &uvs = uvData->readable();

	MIntArray uvIds;
	MFloatArray uArray;
	MFloatArray vArray;

	if( uvIt->second.indices )
	{
		const std::vector<int> &indices = uvIt->second.indices->readable();
		size_t numIndices = indices.size();
		uvIds.setLength( numIndices );
		for( size_t i = 0; i < numIndices; ++i )
		{
			uvIds[i] = indices[i];
		}

		size_t numUVs = uvs.size();
		uArray.setLength( numUVs );
		vArray.setLength( numUVs );
		for( size_t i = 0; i < numUVs; ++i )
		{
			uArray[i] = uvs[i][0];
			vArray[i] = uvs[i][1];
		}
	}
	else
	{
		// If for some reason we cannot find the uv indices, set the UVs using the old way
		// the performances in maya won't be good (for weigth painting in particular)
		// We've already guaranteed that the data is FaceVarying above.

		size_t numUVs = uvs.size();
		uvIds.setLength( numUVs );
		uArray.setLength( numUVs );
		vArray.setLength( numUVs );

		for( size_t i = 0; i < numUVs; ++i )
		{
			uvIds[i] = i;
			uArray[i] = uvs[i][0];
			vArray[i] = uvs[i][1];
		}
	}

	MStatus s = fnMesh.setUVs( uArray, vArray, &uvSetName );
	if ( !s )
	{
		IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", "Failed to set UVs." );
		return;
	}
	s = fnMesh.assignUVs( polygonCounts, uvIds, &uvSetName );
	if ( !s )
	{
		IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", "Failed to assign UVs." );
		return;
	}
}

bool ToMayaMeshConverter::doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus s;

	IECoreScene::ConstMeshPrimitivePtr mesh = IECore::runTimeCast<const IECoreScene::MeshPrimitive>( from );
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
	IECoreScene::PrimitiveVariableMap::const_iterator it = mesh->variables.find("P");
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
		if (it->second.interpolation == IECoreScene::PrimitiveVariable::FaceVarying )
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

	/// Add UV sets
	for ( it = mesh->variables.begin(); it != mesh->variables.end(); ++it )
	{
		if( const IECore::V2fVectorData *data = IECore::runTimeCast<const IECore::V2fVectorData>( it->second.data.get() ) )
		{
			if( data->getInterpretation() == IECore::GeometricData::UV )
			{
				addUVSet( fnMesh, polygonCounts, mesh.get(), it );
			}
		}
	}

	/// Add corners and edge creases

	MUintArray cornerIdsMaya;
	MDoubleArray cornerSharpnessMaya;

	const IECore::IntVectorData *cornerIdsData = mesh->cornerIds();
	const std::vector<int> &cornerIds = cornerIdsData->readable();

	if( !cornerIds.empty() )
	{
		const IECore::FloatVectorData *cornerSharpnessesData = mesh->cornerSharpnesses();
		const std::vector<float> &cornerSharpnesses = cornerSharpnessesData->readable();

		for( size_t i = 0; i < cornerIds.size(); ++i )
		{
			cornerIdsMaya.append( cornerIds[i] );
			cornerSharpnessMaya.append( cornerSharpnesses[i] );
		}

		s = fnMesh.setCreaseVertices( cornerIdsMaya, cornerSharpnessMaya );
		if( !s )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", boost::format( "Failed to set crease vertices with message: %s" ) % s.errorString().asChar() );
		}
	}

	MUintArray edgeIdsMaya;
	MDoubleArray creaseSharpnessMaya;

	const IECore::IntVectorData *creaseIdsData = mesh->creaseIds();
	const std::vector<int> &creaseIds = creaseIdsData->readable();

	if( !creaseIds.empty() )
	{
		const IECore::FloatVectorData *creaseSharpnessesData = mesh->creaseSharpnesses();
		const std::vector<float> &creaseSharpnesses = creaseSharpnessesData->readable();

		const IECore::IntVectorData *creaseLengthsData = mesh->creaseLengths();
		const std::vector<int> &creaseLengths = creaseLengthsData->readable();

		// Cortex stores vertex ids to specify creases. Maya uses edge ids
		// instead. The following handles the conversion.

		MItMeshVertex vertexIt( mObj );

		int offset = 0;
		for( size_t i = 0; i < creaseLengths.size(); ++i )
		{
			if( creaseSharpnesses[i] == 0 )
			{
				continue;
			}

			int length = creaseLengths[i];

			for( int j = 1; j < length; ++j )
			{
				int vertexId1 = creaseIds[offset + j - 1];
				int vertexId2 = creaseIds[offset + j];

				int previousIdx; // we don't need this

				MIntArray connectedEdges;
				vertexIt.setIndex( vertexId1, previousIdx );
				vertexIt.getConnectedEdges( connectedEdges );

				bool found = false;
				for( size_t edgeIdIdx = 0; edgeIdIdx < connectedEdges.length(); ++edgeIdIdx )
				{
					int oppositeVertexId;
					vertexIt.getOppositeVertex( oppositeVertexId, connectedEdges[edgeIdIdx] );

					if( vertexId2 == oppositeVertexId )
					{
						found = true;
						edgeIdsMaya.append( connectedEdges[edgeIdIdx] );
						creaseSharpnessMaya.append( creaseSharpnesses[i] );
						break;
					}
				}

				if( !found )
				{
					IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", boost::format( "Failed to find edge for vertex pair (%i, %i)" ) % vertexId1 % vertexId2 );
				}
			}

			offset += length;
		}

		if( !fnMesh.setCreaseEdges( edgeIdsMaya, creaseSharpnessMaya ) )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaMeshConverter::doConversion", "Failed to set crease edges" );
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
