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

#include "IECore/MeshPrimitive.h"
#include "IECore/VectorOps.h"
#include "IECore/CompoundParameter.h"
#include "IECore/NumericParameter.h"
#include "IECore/MessageHandler.h"
#include "IECore/DespatchTypedData.h"

#include "maya/MFn.h"
#include "maya/MFnMesh.h"
#include "maya/MFnAttribute.h"
#include "maya/MString.h"

#include <algorithm>

using namespace IECoreMaya;
using namespace IECore;
using namespace std;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( FromMayaMeshConverter );

FromMayaShapeConverter::Description<FromMayaMeshConverter> FromMayaMeshConverter::m_description( MFn::kMesh, MeshPrimitiveTypeId, true );
FromMayaShapeConverter::Description<FromMayaMeshConverter> FromMayaMeshConverter::m_dataDescription( MFn::kMeshData, MeshPrimitiveTypeId, true );

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// structors
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FromMayaMeshConverter::FromMayaMeshConverter( const MObject &object )
	:	FromMayaShapeConverter( "Converts poly meshes to IECore::MeshPrimitive objects.", object )
{
	constructCommon();
}

FromMayaMeshConverter::FromMayaMeshConverter( const MDagPath &dagPath )
	:	FromMayaShapeConverter( "Converts poly meshes to IECore::MeshPrimitive objects.", dagPath )
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

	m_interpolation = new StringParameter(
		"interpolation",
		"Sets the interpolation type of the new mesh. When 'default' is used it will query the attribute 'ieMeshInterpolation' from the Mesh instead (and use linear if nonexistent).",
		"default",
		interpolationPresets
	);

	parameters()->addParameter( m_interpolation );

	// points
	BoolParameter::PresetsContainer pointsPresets;
	pointsPresets.push_back( BoolParameter::Preset( "poly", true ) );
	pointsPresets.push_back( BoolParameter::Preset( "subdiv", true ) );

	m_points = new BoolParameter(
		"points",
		"When this is on the mesh points are added to the result as a primitive variable named \"P\".",
		true,
		pointsPresets
	);

	parameters()->addParameter( m_points );

	// normals
	BoolParameter::PresetsContainer normalsPresets;
	normalsPresets.push_back( BoolParameter::Preset( "poly", true ) );
	normalsPresets.push_back( BoolParameter::Preset( "subdiv", true ) );

	m_normals = new BoolParameter(
		"normals",
		"When this is on the mesh normals are added to the result as a primitive variable named \"N\". "
		"Note that normals will only ever be added to meshes created with linear interpolation as "
		"vertex normals are unsuitable for meshes which will be rendered with some form of "
		"subdivision.",
		true,
		normalsPresets
	);

	parameters()->addParameter( m_normals );

	// st
	BoolParameter::PresetsContainer stPresets;
	stPresets.push_back( BoolParameter::Preset( "poly", true ) );
	stPresets.push_back( BoolParameter::Preset( "subdiv", true ) );

	m_st = new BoolParameter(
		"st",
		"When this is on the default uv set is added to the result as primitive variables named \"s\" and \"t\".",
		true,
		stPresets
	);

	parameters()->addParameter( m_st );

	// extra st
	BoolParameter::PresetsContainer extraSTPresets;
	extraSTPresets.push_back( BoolParameter::Preset( "poly", true ) );
	extraSTPresets.push_back( BoolParameter::Preset( "subdiv", true ) );

	m_extraST = new BoolParameter(
		"extraST",
		"When this is on, all uv sets are added to the result as primitive variables named \"setName_s\" and \"setName_t\".",
		true,
		extraSTPresets
	);

	parameters()->addParameter( m_extraST );

	// colors
	BoolParameter::PresetsContainer colorsPresets;
	colorsPresets.push_back( BoolParameter::Preset( "poly", false ) );
	colorsPresets.push_back( BoolParameter::Preset( "subdiv", false ) );

	BoolParameterPtr colors = new BoolParameter(
		"colors",
		"When this is on the default color set is added to the result as primitive variable named \"Cs\".",
		false,
		colorsPresets
	);

	parameters()->addParameter( colors );

	// extra colors
	BoolParameter::PresetsContainer extraColorsPresets;
	extraColorsPresets.push_back( BoolParameter::Preset( "poly", false ) );
	extraColorsPresets.push_back( BoolParameter::Preset( "subdiv", false ) );

	BoolParameterPtr extraColors = new BoolParameter(
		"extraColors",
		"When this is on, all color sets are added to the result as primitive variables named \"setName_Cs\".",
		false,
		extraColorsPresets
	);

	parameters()->addParameter( extraColors );

}

FromMayaMeshConverter::~FromMayaMeshConverter()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// parameter access
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IECore::StringParameterPtr FromMayaMeshConverter::interpolationParameter()
{
	return m_interpolation;
}

IECore::StringParameterPtr FromMayaMeshConverter::interpolationParameter() const
{
	return m_interpolation;
}

IECore::BoolParameterPtr FromMayaMeshConverter::pointsParameter()
{
	return m_points;
}

IECore::BoolParameterPtr FromMayaMeshConverter::pointsParameter() const
{
	return m_points;
}

IECore::BoolParameterPtr FromMayaMeshConverter::normalsParameter()
{
	return m_normals;
}

IECore::BoolParameterPtr FromMayaMeshConverter::normalsParameter() const
{
	return m_normals;
}

IECore::BoolParameterPtr FromMayaMeshConverter::stParameter()
{
	return m_st;
}

IECore::BoolParameterPtr FromMayaMeshConverter::stParameter() const
{
	return m_st;
}

IECore::BoolParameterPtr FromMayaMeshConverter::extraSTParameter()
{
	return m_extraST;
}

IECore::BoolParameterPtr FromMayaMeshConverter::extraSTParameter() const
{
	return m_extraST;
}

IECore::BoolParameterPtr FromMayaMeshConverter::colorsParameter()
{
	return parameters()->parameter< BoolParameter >( "colors" );
}

IECore::ConstBoolParameterPtr FromMayaMeshConverter::colorsParameter() const
{
	return parameters()->parameter< BoolParameter >( "colors" );
}

IECore::BoolParameterPtr FromMayaMeshConverter::extraColorsParameter()
{
	return parameters()->parameter< BoolParameter >( "extraColors" );
}

IECore::ConstBoolParameterPtr FromMayaMeshConverter::extraColorsParameter() const
{
	return parameters()->parameter< BoolParameter >( "extraColors" );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// conversion
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IECore::V3fVectorDataPtr FromMayaMeshConverter::points() const
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
	return points;
}

IECore::V3fVectorDataPtr FromMayaMeshConverter::normals() const
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
	
	return normalsData;
}

void FromMayaMeshConverter::sAndT( const MString &uvSet, IECore::ConstIntVectorDataPtr stIndicesData, IECore::FloatVectorDataPtr& s, IECore::FloatVectorDataPtr& t ) const
{
	MFnMesh fnMesh( object() );
	
	MFloatArray uArray, vArray;
	fnMesh.getUVs( uArray, vArray, &uvSet );
	
	size_t numIndices = stIndicesData->readable().size();
	
	if( uArray.length() == 0 )
	{
		if( s )
		{
			s->writable().resize( numIndices, .0f );
		}
		if( t )
		{
			t->writable().resize( numIndices, .0f );
		}
	}
	else
	{
		const vector< int >& stIndices = stIndicesData->readable();
		if( s )
		{
			vector< float >& sValues = s->writable();
			sValues.reserve( numIndices );
			for( size_t i=0; i < numIndices; ++i )
			{
				sValues.push_back( uArray[ stIndices[i] ] );
			}
		}
		if( t )
		{
			vector< float >& tValues = t->writable();
			tValues.reserve( numIndices );
			for( size_t i=0; i < numIndices; ++i )
			{
				tValues.push_back( 1 - vArray[ stIndices[i] ] );
			}
		}
	}
	
}

IECore::FloatVectorDataPtr FromMayaMeshConverter::s( const MString &uvSet ) const
{
	FloatVectorDataPtr sData = new FloatVectorData;
	FloatVectorDataPtr tData = 0;
	IntVectorDataPtr stIndicesData = stIndices( uvSet );
	sAndT( uvSet, stIndicesData, sData, tData );
	
	return sData;
}

IECore::FloatVectorDataPtr FromMayaMeshConverter::t( const MString &uvSet ) const
{
	FloatVectorDataPtr sData = 0;
	FloatVectorDataPtr tData = new FloatVectorData;
	IntVectorDataPtr stIndicesData = stIndices( uvSet );
	sAndT( uvSet, stIndicesData, sData, tData );
	
	return tData;
}


IECore::IntVectorDataPtr FromMayaMeshConverter::stIndices( const MString &uvSet ) const
{
	MFnMesh fnMesh( object() );
	
	// get face vertex counts:
	int numPolygons = fnMesh.numPolygons();
	IntVectorDataPtr verticesPerFaceData = new IntVectorData;
	verticesPerFaceData->writable().resize( numPolygons );
	vector<int>::iterator verticesPerFaceIt = verticesPerFaceData->writable().begin();
	
	for( int i=0; i<numPolygons; i++ )
	{
		*verticesPerFaceIt++ = fnMesh.polygonVertexCount( i );
	}
	
	return getStIndices( uvSet, verticesPerFaceData );
}

IECore::IntVectorDataPtr FromMayaMeshConverter::getStIndices( const MString &uvSet, IECore::ConstIntVectorDataPtr verticesPerFaceData ) const
{
	MFnMesh fnMesh( object() );
	IntVectorDataPtr resultData = new IntVectorData;
	vector<int> &result = resultData->writable();
	result.reserve( fnMesh.numFaceVertices() );
	
	// get uv data. A list of uv counts per polygon, and a bunch of uv ids:
	MIntArray uvCounts, uvIds;
	fnMesh.getAssignedUVs( uvCounts, uvIds, &uvSet );
	
	// get per face vertex count data:
	const std::vector<int> &vertsPerPoly = verticesPerFaceData->readable();
	
	int numPolygons = fnMesh.numPolygons();
	int uvIdIndex = 0;
	for( int i=0; i < numPolygons; ++i )
	{
		int numPolyUvs = uvCounts[i];
		int numPolyVerts = vertsPerPoly[i];
		
		if( numPolyUvs == 0 )
		{
			for( int j=0; j < numPolyVerts; ++j )
			{
				result.push_back( 0 );
			}
		}
		else
		{
			for( int j=0; j < numPolyVerts; ++j )
			{
				result.push_back( uvIds[ uvIdIndex++ ] );
			}
		}
		
	}
	
	return resultData;

}

IECore::DataPtr  FromMayaMeshConverter::colors( const MString &colorSet, bool forceRgb ) const
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
	return data;
}

IECore::PrimitivePtr FromMayaMeshConverter::doPrimitiveConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnMesh fnMesh( object );
	return doPrimitiveConversion( fnMesh );
}

IECore::PrimitivePtr FromMayaMeshConverter::doPrimitiveConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnMesh fnMesh( dagPath );
	return doPrimitiveConversion( fnMesh );
}

IECore::PrimitivePtr FromMayaMeshConverter::doPrimitiveConversion( MFnMesh &fnMesh ) const
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
	
	std::string interpolation = m_interpolation->getTypedValue();
	if ( interpolation == "default" )
	{
		MStatus st;
		MPlug interpolationPlug = fnMesh.findPlug( "ieMeshInterpolation", &st );
		if ( st )
		{
			unsigned int interpolationIndex = interpolationPlug.asInt(MDGContext::fsNormal, &st);
			if ( st )
			{
				if ( interpolationIndex < m_interpolation->getPresets().size() - 1 )
				{
					// convert interpolation index to the preset value
					interpolation = boost::static_pointer_cast< StringData >( m_interpolation->getPresets()[interpolationIndex].second )->readable();
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

	if( m_points->getTypedValue() )
	{
		result->variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, points() );
	}
	if( m_normals->getTypedValue() && interpolation=="linear" )
	{
		result->variables["N"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, normals() );
	}

	MString currentUVSet;
	fnMesh.getCurrentUVSetName( currentUVSet );
	MStringArray uvSets;
	fnMesh.getUVSetNames( uvSets );
	for( unsigned int i=0; i<uvSets.length(); i++ )
	{

		FloatVectorDataPtr sData = new FloatVectorData;
		FloatVectorDataPtr tData = new FloatVectorData;
		
		IntVectorDataPtr stIndicesData = getStIndices( uvSets[i], verticesPerFaceData );
		
		sAndT( uvSets[i], stIndicesData, sData, tData );
		
		if( uvSets[i]==currentUVSet )
		{
			if( m_st->getTypedValue() )
			{
				result->variables["s"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, sData );
				result->variables["t"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, tData );
				result->variables["stIndices"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, stIndicesData );
			}
		}
		
		if( m_extraST->getTypedValue() )
		{
			MString sName = uvSets[i] + "_s";
			MString tName = uvSets[i] + "_t";
			MString indicesName = uvSets[i] + "Indices";
			result->variables[sName.asChar()] = PrimitiveVariable( PrimitiveVariable::FaceVarying, sData );
			result->variables[tName.asChar()] = PrimitiveVariable( PrimitiveVariable::FaceVarying, tData );
			result->variables[indicesName.asChar()] = PrimitiveVariable( PrimitiveVariable::FaceVarying, stIndicesData );
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
				result->variables["Cs"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, colors( currentColorSet, true ) );
			}
			
			if( convertExtraColors )
			{
				MString sName = colorSets[i] + "_Cs";
				// Extra color sets are not converted
				result->variables[sName.asChar()] = PrimitiveVariable( PrimitiveVariable::FaceVarying, colors( colorSets[i] ) );
			}
		}
	}

	return result;
}
