//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2014, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/FromMayaMeshConverter.h"
#include "IECoreMaya/FromMayaPlugConverter.h"
#include "IECoreMaya/MArrayIter.h"
#include "IECoreMaya/VectorTraits.h"

#include "IECore/VectorOps.h"
#include "IECore/CompoundParameter.h"
#include "IECore/NumericParameter.h"
#include "IECore/MessageHandler.h"
#include "IECore/DespatchTypedData.h"
#include "IECoreScene/MeshPrimitive.h"

#include "maya/MFn.h"
#include "maya/MFnMesh.h"
#include "maya/MFnAttribute.h"
#include "maya/MString.h"

#if MAYA_API_VERSION >= 201800
#include "maya/MDGContextGuard.h"
#endif

#include <algorithm>

using namespace IECoreMaya;
using namespace IECore;
using namespace IECoreScene;
using namespace std;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( FromMayaMeshConverter );

FromMayaShapeConverter::Description<FromMayaMeshConverter> FromMayaMeshConverter::m_description( MFn::kMesh, MeshPrimitive::staticTypeId(), true );
FromMayaShapeConverter::Description<FromMayaMeshConverter> FromMayaMeshConverter::m_dataDescription( MFn::kMeshData, MeshPrimitive::staticTypeId(), true );

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// structors
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FromMayaMeshConverter::FromMayaMeshConverter( const MObject &object )
	:	FromMayaShapeConverter( "Converts poly meshes to IECoreScene::MeshPrimitive objects.", object )
{
	constructCommon();
}

FromMayaMeshConverter::FromMayaMeshConverter( const MDagPath &dagPath )
	:	FromMayaShapeConverter( "Converts poly meshes to IECoreScene::MeshPrimitive objects.", dagPath )
{
	constructCommon();
}

void FromMayaMeshConverter::constructCommon()
{

	// interpolation
	StringParameter::PresetsContainer interpolationPresets;
	interpolationPresets.push_back( StringParameter::Preset( "poly", "linear" ) );
	interpolationPresets.push_back( StringParameter::Preset( "subdiv", "catmullClark" ) );
	// the last interpolation type is 'default'
	interpolationPresets.push_back( StringParameter::Preset( "default", "default" ) );

	StringParameterPtr interpolation = new StringParameter(
		"interpolation",
		"Sets the interpolation type of the new mesh. When 'default' is used it will query the attribute 'ieMeshInterpolation' from the Mesh instead (and use linear if nonexistent).",
		"default",
		interpolationPresets
	);

	parameters()->addParameter( interpolation );
	// normals
	BoolParameterPtr normals = new BoolParameter(
		"normals",
		"When this is on the mesh normals are added to the result as a primitive variable named \"N\". "
		"Note that normals will only ever be added to meshes created with linear interpolation as "
		"vertex normals are unsuitable for meshes which will be rendered with some form of "
		"subdivision.",
		true
	);
	parameters()->addParameter( normals );

	// uv
	BoolParameterPtr uv = new BoolParameter(
		"uv",
		"When this is on the uv sets are added to the result as primitive variables",
		true
	);
	parameters()->addParameter( uv );

	// colors
	BoolParameterPtr colors = new BoolParameter(
		"colors",
		"When this is on the default color set is added to the result as primitive variable named \"Cs\".",
		false
	);

	parameters()->addParameter( colors );

	// extra colors
	BoolParameterPtr extraColors = new BoolParameter(
		"extraColors",
		"When this is on, all color sets are added to the result as primitive variables named \"setName_Cs\".",
		false
	);

	parameters()->addParameter( extraColors );

	BoolParameterPtr creases = new BoolParameter(
		"creases",
		"When this is on, all corners and edge creases are added to the result.",
		true
	);

	parameters()->addParameter( creases );

}

FromMayaMeshConverter::~FromMayaMeshConverter()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// parameter access
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IECore::StringParameter *FromMayaMeshConverter::interpolationParameter()
{
	return parameters()->parameter< StringParameter >( "interpolation" );
}

const IECore::StringParameter *FromMayaMeshConverter::interpolationParameter() const
{
	return parameters()->parameter< StringParameter >( "interpolation" );
}

IECore::BoolParameter *FromMayaMeshConverter::normalsParameter()
{
	return parameters()->parameter< BoolParameter >( "normals" );
}
const IECore::BoolParameter *FromMayaMeshConverter::normalsParameter() const
{
	return parameters()->parameter< BoolParameter >( "normals" );
}

IECore::BoolParameter *FromMayaMeshConverter::uvParameter()
{
	return parameters()->parameter< BoolParameter >( "uv" );
}

const IECore::BoolParameter *FromMayaMeshConverter::uvParameter() const
{
	return parameters()->parameter< BoolParameter >( "uv" );
}

IECore::BoolParameter *FromMayaMeshConverter::colorsParameter()
{
	return parameters()->parameter< BoolParameter >( "colors" );
}

const IECore::BoolParameter *FromMayaMeshConverter::colorsParameter() const
{
	return parameters()->parameter< BoolParameter >( "colors" );
}

IECore::BoolParameter *FromMayaMeshConverter::extraColorsParameter()
{
	return parameters()->parameter< BoolParameter >( "extraColors" );
}

const IECore::BoolParameter *FromMayaMeshConverter::extraColorsParameter() const
{
	return parameters()->parameter< BoolParameter >( "extraColors" );
}

IECore::BoolParameter *FromMayaMeshConverter::creasesParameter()
{
	return parameters()->parameter< BoolParameter >( "creases" );
}

const IECore::BoolParameter *FromMayaMeshConverter::creasesParameter() const
{
	return parameters()->parameter< BoolParameter >( "creases" );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// conversion
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IECoreScene::PrimitiveVariable FromMayaMeshConverter::points() const
{
	MFnMesh fnMesh;
	const MDagPath *d = dagPath( true );
	if( d )
	{
		fnMesh.setObject( *d );
	}
	else
	{
		fnMesh.setObject( object() );
	}

	V3fVectorDataPtr points = new V3fVectorData;
	points->setInterpretation( GeometricData::Point );
	int numVerts = fnMesh.numVertices();
	points->writable().resize( numVerts );

	if( space() == MSpace::kObject )
	{
		const V3f* rawPoints = ( const V3f* )fnMesh.getRawPoints(0);
		copy( rawPoints, rawPoints + numVerts, points->writable().begin() );
	}
	else
	{
		MFloatPointArray mPoints;
		fnMesh.getPoints( mPoints, space() );
		std::transform( MArrayIter<MFloatPointArray>::begin( mPoints ), MArrayIter<MFloatPointArray>::end( mPoints ), points->writable().begin(), VecConvert<MFloatPoint, V3f>() );
	}

	return PrimitiveVariable( PrimitiveVariable::Vertex, points );
}

IECoreScene::PrimitiveVariable FromMayaMeshConverter::normals() const
{
	MFnMesh fnMesh;
	const MDagPath *d = dagPath( true );
	if( d )
	{
		fnMesh.setObject( *d );
	}
	else
	{
		fnMesh.setObject( object() );
	}

	V3fVectorDataPtr normalsData = new V3fVectorData;
	normalsData->setInterpretation( GeometricData::Normal );
	vector<V3f> &normals = normalsData->writable();
	normals.reserve( fnMesh.numFaceVertices() );

	int numPolygons = fnMesh.numPolygons();
	V3f blankVector;

	if( space() == MSpace::kObject )
	{
		const float* rawNormals = fnMesh.getRawNormals(0);
		MIntArray normalIds;
		for( int i=0; i<numPolygons; i++ )
		{
			fnMesh.getFaceNormalIds( i, normalIds );
			for( unsigned j=0; j < normalIds.length(); ++j )
			{
				const float* normalIt = rawNormals + 3 * normalIds[j];
				normals.push_back( blankVector );
				V3f& nn = normals.back();
				nn.x = *normalIt++;
				nn.y = *normalIt++;
				nn.z = *normalIt;
			}
		}
	}
	else
	{
		MFloatVectorArray faceNormals;
		for( int i=0; i<numPolygons; i++ )
		{
			fnMesh.getFaceVertexNormals( i, faceNormals, space() );
			for( unsigned j=0; j<faceNormals.length(); j++ )
			{
				MFloatVector& n = faceNormals[j];
				normals.push_back( blankVector );
				V3f& nn = normals.back();
				nn.x = n.x;
				nn.y = n.y;
				nn.z = n.z;
			}
		}
	}

	return PrimitiveVariable( PrimitiveVariable::FaceVarying, normalsData );
}

IECoreScene::PrimitiveVariable FromMayaMeshConverter::uvs( const MString &uvSet, const std::vector<int> &vertsPerFace ) const
{
	MFnMesh fnMesh( object() );

	IntVectorDataPtr indexData = new IntVectorData;
	vector<int> &indices = indexData->writable();
	indices.reserve( fnMesh.numFaceVertices() );

	// get uv data. A list of uv counts per polygon, and a bunch of uv ids:
	MIntArray uvCounts, uvIds;
	fnMesh.getAssignedUVs( uvCounts, uvIds, &uvSet );

	int uvIdIndex = 0;
	for( size_t i=0; i < vertsPerFace.size(); ++i )
	{
		int numPolyUvs = uvCounts[i];
		int numPolyVerts = vertsPerFace[i];

		if( numPolyUvs == 0 )
		{
			for( int j=0; j < numPolyVerts; ++j )
			{
				indices.push_back( 0 );
			}
		}
		else
		{
			for( int j=0; j < numPolyVerts; ++j )
			{
				indices.push_back( uvIds[ uvIdIndex++ ] );
			}
		}
	}

	V2fVectorDataPtr uvData = new V2fVectorData;
	uvData->setInterpretation( GeometricData::UV );
	std::vector<Imath::V2f> &uvs = uvData->writable();

	MFloatArray uArray, vArray;
	fnMesh.getUVs( uArray, vArray, &uvSet );

	size_t numIndices = indices.size();
	if( uArray.length() == 0 )
	{
		uvs.resize( numIndices, Imath::V2f( .0f ) );
	}
	else
	{
		uvs.reserve( uArray.length() );
		for( size_t i=0; i < uArray.length(); ++i )
		{
			uvs.emplace_back( uArray[i], vArray[i] );
		}
	}

	return PrimitiveVariable( PrimitiveVariable::FaceVarying, uvData, indexData );
}

IECoreScene::PrimitiveVariable FromMayaMeshConverter::colors( const MString &colorSet, bool forceRgb ) const
{
	MFnMesh fnMesh( object() );
	MFnMesh::MColorRepresentation rep = fnMesh.getColorRepresentation( colorSet );

	int numColors = fnMesh.numFaceVertices();
	MColorArray colors;
	MColor defaultColor(0,0,0,1);
	if ( !fnMesh.getFaceVertexColors( colors, &colorSet, &defaultColor ) )
	{
		throw Exception( ( boost::format( "Failed to obtain colors from color set '%s'" ) % colorSet ).str() );
	}

	int availableColors = colors.length();
	if ( availableColors > numColors )
	{
		availableColors = numColors;
	}

	DataPtr data;

	if ( rep == MFnMesh::kAlpha )
	{
		if ( forceRgb )
		{
			Color3fVectorDataPtr colorVec = new Color3fVectorData();
			colorVec->writable().resize( numColors, Imath::Color3f(1) );
			std::vector< Imath::Color3f >::iterator it = colorVec->writable().begin();
			for ( int i = 0; i < availableColors; i++, it++ )
			{
				*it = Imath::Color3f( colors[i][3] );
			}
			data = colorVec;
		}
		else
		{
			FloatVectorDataPtr colorVec = new FloatVectorData();
			colorVec->writable().resize( numColors, 1 );
			std::vector< float >::iterator it = colorVec->writable().begin();
			for ( int i = 0; i < availableColors; i++, it++ )
			{
				*it = colors[i][3];
			}
			data = colorVec;
		}
	}
	else
	{
		if ( rep == MFnMesh::kRGB || forceRgb )
		{
			Color3fVectorDataPtr colorVec = new Color3fVectorData();
			colorVec->writable().resize( numColors, Imath::Color3f(0,0,0) );
			std::vector< Imath::Color3f >::iterator it = colorVec->writable().begin();
			for ( int i = 0; i < availableColors; i++, it++ )
			{
				const MColor &c = colors[i];
				*it = Imath::Color3f( c[0], c[1], c[2] );
			}
			data = colorVec;
		}
		else
		{
			Color4fVectorDataPtr colorVec = new Color4fVectorData();
			colorVec->writable().resize( numColors, Imath::Color4f(0,0,0,1) );
			std::vector< Imath::Color4f >::iterator it = colorVec->writable().begin();
			for ( int i = 0; i < availableColors; i++, it++ )
			{
				const MColor &c = colors[i];
				*it = Imath::Color4f( c[0], c[1], c[2], c[3] );
			}
			data = colorVec;
		}
	}
	return PrimitiveVariable( PrimitiveVariable::FaceVarying, data );
}

void FromMayaMeshConverter::corners( MeshPrimitive *mesh ) const
{
	MFnMesh fnMesh( object() );

	MUintArray vertexIds;
	MDoubleArray creaseData;

	MStatus s = fnMesh.getCreaseVertices( vertexIds, creaseData );

	if( !s )
	{
		// Instead of returning empty data in the case where no creases are
		// present, Maya considers this a failure. We ignore the error status and
		// return without modifying the given vectors.
		return;
	}

	assert( vertexIds.length() == creaseData.length() );

	IntVectorDataPtr cornerIdsData = new IntVectorData();
	std::vector<int> &cornerIds = cornerIdsData->writable();
	cornerIds.reserve( vertexIds.length() );

	FloatVectorDataPtr cornerSharpnessesData = new FloatVectorData();
	std::vector<float> &cornerSharpnesses = cornerSharpnessesData->writable();
	cornerSharpnesses.reserve( creaseData.length() );

	for( size_t i = 0; i < vertexIds.length(); ++i )
	{
		cornerIds.push_back( vertexIds[i] );
		cornerSharpnesses.push_back( creaseData[i] );
	}

	mesh->setCorners( cornerIdsData.get(), cornerSharpnessesData.get() );
}

void FromMayaMeshConverter::creases( MeshPrimitive *mesh ) const
{
	MFnMesh fnMesh( object() );

	MUintArray edgeIds;
	MDoubleArray creaseData;

	MStatus s = fnMesh.getCreaseEdges( edgeIds, creaseData );
	if( !s )
	{
		// Instead of returning empty data in the case where no creases are
		// present, Maya considers this a failure. We ignore the error status and
		// return without modifying the given vectors.
		return;
	}

	assert( edgeIds.length() == creaseData.length() );

	IntVectorDataPtr creaseLengthsData = new IntVectorData();
	std::vector<int> &creaseLengths = creaseLengthsData->writable();
	creaseLengths.resize( edgeIds.length(), 2 );

	IntVectorDataPtr creaseIdsData = new IntVectorData();
	std::vector<int> &creaseIds = creaseIdsData->writable();
	creaseIds.reserve( creaseLengths.size() * 2 );

	FloatVectorDataPtr creaseSharpnessesData = new FloatVectorData();
	std::vector<float> &creaseSharpnesses = creaseSharpnessesData->writable();
	creaseSharpnesses.reserve( creaseLengths.size() );

	// Maya stores creases via edge id. Cortex uses vertex ids instead. The
	// following handles the conversion.
	// \todo: Cortex supports a more compact crease representation that saves
	// some memory. An additional compacting step here would allow us to reap
	// the benefits.

	int2 vertexList;
	for( size_t i = 0; i < edgeIds.length(); ++i )
	{
		fnMesh.getEdgeVertices( edgeIds[i], vertexList );

		creaseIds.push_back( vertexList[0] );
		creaseIds.push_back( vertexList[1] );

		creaseSharpnesses.push_back( creaseData[ i ] );
	}

	mesh->setCreases( creaseLengthsData.get(), creaseIdsData.get(), creaseSharpnessesData.get() );
}

IECoreScene::PrimitivePtr FromMayaMeshConverter::doPrimitiveConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnMesh fnMesh( object );
	return doPrimitiveConversion( fnMesh );
}

IECoreScene::PrimitivePtr FromMayaMeshConverter::doPrimitiveConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnMesh fnMesh( dagPath );
	return doPrimitiveConversion( fnMesh );
}

IECoreScene::PrimitivePtr FromMayaMeshConverter::doPrimitiveConversion( MFnMesh &fnMesh ) const
{
	// get basic topology and create a mesh
	int numPolygons = fnMesh.numPolygons();
	IntVectorDataPtr verticesPerFaceData = new IntVectorData;
	verticesPerFaceData->writable().resize( numPolygons );
	vector<int>::iterator verticesPerFaceIt = verticesPerFaceData->writable().begin();

	IntVectorDataPtr vertexIds = new IntVectorData;
	// We are calling fnMesh.numFaceVertices() twice to work around a known bug in Maya. When accessing
	// certain MFnMesh API calls, given a mesh with 6 or more UV sets, which has never been evaluated
	// before, the first call returns 0 and kFailure, and the second call works as expected.
	// See ToMayaMeshConverterTest.testManyUVConversionsFromPlug for an example of how this might occur.
	fnMesh.numFaceVertices();
	vertexIds->writable().resize( fnMesh.numFaceVertices() );
	vector<int>::iterator vertexIdsIt = vertexIds->writable().begin();

	MIntArray vertexCounts, polygonVertices;
	fnMesh.getVertices( vertexCounts, polygonVertices );

	copy( MArrayIter<MIntArray>::begin( vertexCounts ), MArrayIter<MIntArray>::end( vertexCounts ), verticesPerFaceIt );
	copy( MArrayIter<MIntArray>::begin( polygonVertices ), MArrayIter<MIntArray>::end( polygonVertices ), vertexIdsIt );

	std::string interpolation = interpolationParameter()->getTypedValue();
	if ( interpolation == "default" )
	{
		MStatus st;
		MPlug interpolationPlug = fnMesh.findPlug( "ieMeshInterpolation", false, &st );
		if ( st )
		{
#if MAYA_API_VERSION >= 201800
			unsigned int interpolationIndex;
			{
				MDGContextGuard ctxGuard( MDGContext::fsNormal );
				interpolationIndex = interpolationPlug.asInt(&st);
			}
#else
			unsigned int interpolationIndex = interpolationPlug.asInt(MDGContext::fsNormal, &st);
#endif
			if ( st )
			{
				if ( interpolationIndex < interpolationParameter()->getPresets().size() - 1 )
				{
					// convert interpolation index to the preset value
					interpolation = boost::static_pointer_cast< StringData >( interpolationParameter()->getPresets()[interpolationIndex].second )->readable();
				}
				else
				{
					interpolation = "linear";
				}
			}
			else
			{
				interpolation = "linear";
			}
		}
		else
		{
			interpolation = "linear";
		}
	}

	MeshPrimitivePtr result = new MeshPrimitive( verticesPerFaceData, vertexIds, interpolation );

	result->variables["P"] = points();

	if( normalsParameter()->getTypedValue() && interpolation=="linear" )
	{
		result->variables["N"] = normals();
	}

	MString currentUVSet;
	fnMesh.getCurrentUVSetName( currentUVSet );

	if( uvParameter()->getTypedValue() && currentUVSet.length() )
	{
		MStringArray uvSets;
		fnMesh.getUVSetNames( uvSets );
		for( unsigned int i = 0; i < uvSets.length(); i++ )
		{
			if( uvSets[i] == currentUVSet )
			{
				result->variables["uv"] = uvs( currentUVSet, verticesPerFaceData->readable() );
			}
			else
			{
				result->variables[uvSets[i].asChar()] = uvs( uvSets[i], verticesPerFaceData->readable() );
			}
		}
	}

	bool convertColors = colorsParameter()->getTypedValue();
	bool convertExtraColors = extraColorsParameter()->getTypedValue();
	if ( convertColors || convertExtraColors )
	{
		MString currentColorSet;
		fnMesh.getCurrentColorSetName( currentColorSet );
		MStringArray colorSets;
		fnMesh.getColorSetNames( colorSets );
		for( unsigned int i=0; i<colorSets.length(); i++ )
		{
			if( convertColors && colorSets[i]==currentColorSet )
			{
				// Cs is always converted to Color3f
				result->variables["Cs"] = colors( currentColorSet, true );
			}

			if( convertExtraColors )
			{
				MString sName = colorSets[i] + "_Cs";
				// Extra color sets are not converted
				result->variables[sName.asChar()] = colors( colorSets[i] );
			}
		}
	}

	bool convertCreases = creasesParameter()->getTypedValue();
	if( convertCreases )
	{
		corners( result.get() );
		creases( result.get() );
	}

	return result;
}
