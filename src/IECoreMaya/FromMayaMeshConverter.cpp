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

#include "IECoreMaya/FromMayaMeshConverter.h"
#include "IECoreMaya/FromMayaPlugConverter.h"
#include "IECoreMaya/MArrayIter.h"
#include "IECoreMaya/VectorTraits.h"
#include "IECoreMaya/FromMayaRenderableConverterUtil.h"

#include "IECore/MeshPrimitive.h"
#include "IECore/VectorOps.h"
#include "IECore/CompoundParameter.h"
#include "IECore/NumericParameter.h"
#include "IECore/MessageHandler.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/ClassData.h"

#include "maya/MFn.h"
#include "maya/MFnMesh.h"
#include "maya/MFnAttribute.h"

#include <algorithm>

using namespace IECoreMaya;
using namespace IECore;
using namespace std;
using namespace Imath;

static const MFn::Type fromTypes[] = { MFn::kMesh, MFn::kMeshData, MFn::kInvalid };
static const IECore::TypeId toTypes[] = { BlindDataHolderTypeId, RenderableTypeId, VisibleRenderableTypeId, PrimitiveTypeId, MeshPrimitiveTypeId, InvalidTypeId };

FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaMeshConverter> FromMayaMeshConverter::m_description( fromTypes, toTypes );

struct FromMayaMeshConverter::ExtraData
{
	IntParameterPtr m_space;
};

typedef ClassData< FromMayaMeshConverter, FromMayaMeshConverter::ExtraData*, Deleter<FromMayaMeshConverter::ExtraData*> > FromMayaMeshConverterClassData;
static FromMayaMeshConverterClassData g_classData;

FromMayaMeshConverter::FromMayaMeshConverter( const MObject &object )
	:	FromMayaObjectConverter( "FromMayaMeshConverter", "Converts poly meshes to IECore::MeshPrimitive objects.", object )
{
	ExtraData *extraData = g_classData.create( this, new ExtraData() );
	assert( extraData );

	// interpolation
	StringParameter::PresetsMap interpolationPresets;
	interpolationPresets["poly"] = "linear";
	interpolationPresets["subdiv"] = "catmullClark";

	m_interpolation = new StringParameter(
		"interpolation",
		"Sets the interpolation type of the new mesh",
		"linear",
		interpolationPresets
	);
		
	parameters()->addParameter( m_interpolation );
	
	// points
	BoolParameter::PresetsMap pointsPresets;
	pointsPresets["poly"] = true;
	pointsPresets["subdiv"] = true;

	m_points = new BoolParameter(
		"points",
		"When this is on the mesh points are added to the result as a primitive variable named \"P\".",
		true,
		pointsPresets
	);
	
	parameters()->addParameter( m_points );
	
	// normals
	BoolParameter::PresetsMap normalsPresets;
	normalsPresets["poly"] = true;
	normalsPresets["subdiv"] = false;

	m_normals = new BoolParameter(
		"normals",
		"When this is on the mesh normals are added to the result as a primitive variable named \"N\".",
		true,
		normalsPresets
	);
	
	parameters()->addParameter( m_normals );
	
	// st
	BoolParameter::PresetsMap stPresets;
	stPresets["poly"] = true;
	stPresets["subdiv"] = true;

	m_st = new BoolParameter(
		"st",
		"When this is on the default uv set is added to the result as primitive variables named \"s\" and \"t\".",
		true,
		stPresets
	);
	
	parameters()->addParameter( m_st );
	
	// extra st
	BoolParameter::PresetsMap extraSTPresets;
	extraSTPresets["poly"] = true;
	extraSTPresets["subdiv"] = true;

	m_extraST = new BoolParameter(
		"extraST",
		"When this is on any additional uv sets are added to the result as primitive variables named \"setName_s\" and \"setName_t\".",
		true,
		extraSTPresets
	);
	
	parameters()->addParameter( m_extraST );
	
	// additional primvars
	StringParameter::PresetsMap primVarAttrPrefixPresets;
	primVarAttrPrefixPresets["MTOR"] = "rman";
	primVarAttrPrefixPresets["3Delight"] = "delight";
	primVarAttrPrefixPresets["None"] = "";
	m_primVarAttrPrefix = new StringParameter(
		"primVarAttrPrefix",
		"Any attribute names beginning with this prefix are considered to represent primitive variables and are converted as such."
		"The interpolation type of the variable is guessed, unless the attribute name begins with prefix_?_, in which case the ? is"
		"used to specify type - C for constant, U for uniform, V for Vertex, Y for varying and F for facevarying",
		"delight", // compatibility with 3delight by default
		primVarAttrPrefixPresets
	);

	parameters()->addParameter( m_primVarAttrPrefix );
	
	IntParameter::PresetsMap spacePresets;
	spacePresets["Transform"] = Transform;
	spacePresets["PreTransform"] = PreTransform;
	spacePresets["PostTransform"] = PostTransform;
	spacePresets["Object"] = Object;	
	spacePresets["World"] = World;		
	extraData->m_space = new IntParameter(
		"space",
		"The space obtain the mesh in",
		Transform,
		Transform,
		Object,
		spacePresets,
		true
	);
		
	parameters()->addParameter( extraData->m_space );

	FromMayaRenderableConverterUtilPtr m = new FromMayaRenderableConverterUtil();
	parameters()->addParameters( m->parameters()->orderedParameters().begin(), m->parameters()->orderedParameters().end() );
}

FromMayaMeshConverter::~FromMayaMeshConverter()
{
	g_classData.erase( this );
}

IECore::V3fVectorDataPtr FromMayaMeshConverter::points() const
{
	MFnMesh fnMesh( object() );
	
	MFloatPointArray mPoints;
	fnMesh.getPoints( mPoints, space() );
	
	V3fVectorDataPtr points = new V3fVectorData;
	points->writable().resize( mPoints.length() );
	std::transform( MArrayIter<MFloatPointArray>::begin( mPoints ), MArrayIter<MFloatPointArray>::end( mPoints ), points->writable().begin(), VecConvert<MFloatPoint, V3f>() );
	return points;
}

IECore::V3fVectorDataPtr FromMayaMeshConverter::normals() const
{
	MFnMesh fnMesh( object() );

	V3fVectorDataPtr normalsData = new V3fVectorData;
	vector<V3f> &normals = normalsData->writable();
	normals.resize( fnMesh.numFaceVertices() );
	
	int numPolygons = fnMesh.numPolygons();
	MFloatVectorArray faceNormals;
	unsigned int normalIndex = 0;
	for( int i=0; i<numPolygons; i++ )
	{
		fnMesh.getFaceVertexNormals( i, faceNormals, space() );
		for( int j=faceNormals.length()-1; j>=0; j-- )
		{
			normals[normalIndex++] = vecConvert<MVector, V3f>( faceNormals[j] );
		}
	}
	assert( normalIndex==normals.size() );
	return normalsData;
}

IECore::FloatVectorDataPtr FromMayaMeshConverter::sOrT( const MString &uvSet, unsigned int index ) const
{
	MFnMesh fnMesh( object() );
	FloatVectorDataPtr resultData = new FloatVectorData;
	vector<float> &result = resultData->writable();
	result.resize( fnMesh.numFaceVertices() );
	int numPolygons = fnMesh.numPolygons();
	unsigned int resultIndex = 0;
	for( int i=0; i<numPolygons; i++ )
	{
		for( int j=fnMesh.polygonVertexCount( i )-1; j>=0; j-- )
		{
			float uv[2];
			fnMesh.getPolygonUV( i, j, uv[0], uv[1], &uvSet );
			result[resultIndex++] = index==1 ? 1-uv[index] : uv[index];
		}
	}
	return resultData;

}

IECore::FloatVectorDataPtr FromMayaMeshConverter::s( const MString &uvSet ) const
{
	return sOrT( uvSet, 0 );
}

IECore::FloatVectorDataPtr FromMayaMeshConverter::t( const MString &uvSet ) const
{
	return sOrT( uvSet, 1 );
}

void FromMayaMeshConverter::addPrimVars( IECore::PrimitivePtr primitive, const MString &prefix ) const
{
	MFnDependencyNode fnNode( object() );
	unsigned int n = fnNode.attributeCount();
	for( unsigned int i=0; i<n; i++ )
	{
		MObject attr = fnNode.attribute( i );
		MFnAttribute fnAttr( attr );
		MString attrName = fnAttr.name();
		if( attrName.substring( 0, prefix.length()-1 )==prefix && attrName.length() > prefix.length() )
		{
			MPlug plug = fnNode.findPlug( attr );
			if( !plug.parent().isNull() )
			{
				continue; // we don't want to pick up the children of compound numeric attributes
			}
			MString plugName = plug.name();
			
			// find a converter for the plug, asking for conversion to float types by preference			
			FromMayaConverterPtr converter = FromMayaPlugConverter::create( plug, IECore::FloatDataTypeId );
			if( !converter )
			{
				converter = FromMayaPlugConverter::create( plug, IECore::V3fDataTypeId );
			}
			if( !converter )
			{
				converter = FromMayaPlugConverter::create( plug, IECore::V3fVectorDataTypeId );
			}
			if( !converter )
			{
				converter = FromMayaPlugConverter::create( plug, IECore::FloatVectorDataTypeId );
			}
			if( !converter )
			{
				converter = FromMayaPlugConverter::create( plug );
			}
			
			// run the conversion and check we've got data as a result
			DataPtr data = 0;
			if( converter )
			{
				 data = runTimeCast<Data>( converter->convert() );
			}
			if( !data )
			{
				msg( Msg::Warning, "FromMayaMeshConverter::addPrimVars", boost::format( "Attribute \"%s\" could not be converted to Data." ) % plugName.asChar() );
				continue;
			}
			
			// convert V3fData to Color3fData if attribute has usedAsColor() set.
			if( V3fDataPtr vData = runTimeCast<V3fData>( data ) )
			{
				if( fnAttr.isUsedAsColor() )
				{
					V3f v = vData->readable();
					data = new Color3fData( Color3f( v.x, v.y, v.z ) ); 
				}
			}

			// see if interpolation has been specified, and find primitive variable name
			string primVarName = attrName.asChar() + prefix.length();
			PrimitiveVariable::Interpolation interpolation = PrimitiveVariable::Invalid;
			if( attrName.length()>prefix.length()+3 )
			{
				const char *c = attrName.asChar();
				if( c[prefix.length()]=='_' && c[prefix.length()+2]=='_' )
				{
					char t = c[prefix.length()+1];
					primVarName = attrName.asChar() + prefix.length() + 3;
					switch( t )
					{
						case 'C' :
							interpolation = PrimitiveVariable::Constant;
							break;
						case 'U' :
							interpolation = PrimitiveVariable::Uniform;
							break;
						case 'V' :
							interpolation = PrimitiveVariable::Vertex;
							break;
						case 'Y' :
							interpolation = PrimitiveVariable::Varying;
							break;
						case 'F' :
							interpolation = PrimitiveVariable::FaceVarying;
							break;
						default :
							msg( Msg::Warning, "FromMayaMeshConverter::addPrimVars", boost::format( "Attribute \"%s\" has unknown interpolation - guessing interpolation." ) % plugName.asChar() );
							break;
					}
				}
			}

			// guess interpolation if not specified
			if( interpolation==PrimitiveVariable::Invalid )
			{
				size_t s = 1;
				try
				{
					s = despatchTypedData< TypedDataSize, TypeTraits::IsVectorTypedData>( data );
				}
				catch( ... )
				{
				}
				/// \todo Maybe this would make a useful utility function in IECore::Primitive()
				if( s==primitive->variableSize( PrimitiveVariable::Constant ) )
				{
					interpolation = PrimitiveVariable::Constant;
				}
				else if( s==primitive->variableSize( PrimitiveVariable::Uniform ) )
				{
					interpolation = PrimitiveVariable::Uniform;
				}
				else if( s==primitive->variableSize( PrimitiveVariable::Vertex ) )
				{
					interpolation = PrimitiveVariable::Vertex;
				}
				else if( s==primitive->variableSize( PrimitiveVariable::Varying ) )
				{
					interpolation = PrimitiveVariable::Varying;
				}
				else if( s==primitive->variableSize( PrimitiveVariable::FaceVarying ) )
				{
					interpolation = PrimitiveVariable::FaceVarying;
				}
				else
				{
					msg( Msg::Warning, "FromMayaMeshConverter::addPrimVars", boost::format( "Attribute \"%s\" has unsuitable size to guess interpolation." ) % plugName.asChar() );
					continue;
				}
			}

			// finally add the primvar
			primitive->variables[primVarName] = PrimitiveVariable( interpolation, data );

		}
	}
}
		
IECore::ObjectPtr FromMayaMeshConverter::doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnMesh fnMesh( object );
	
	// get basic topology and create a mesh
	int numPolygons = fnMesh.numPolygons();
	IntVectorDataPtr verticesPerFaceData = new IntVectorData;
	verticesPerFaceData->writable().resize( numPolygons );
	vector<int>::iterator verticesPerFaceIt = verticesPerFaceData->writable().begin();
	
	IntVectorDataPtr vertexIds = new IntVectorData;
	vertexIds->writable().resize( fnMesh.numFaceVertices() );	
	vector<int>::iterator vertexIdsIt = vertexIds->writable().begin();
	
	MIntArray polygonVertices;
	for( int i=0; i<numPolygons; i++ )
	{
		fnMesh.getPolygonVertices( i, polygonVertices );
		*verticesPerFaceIt++ = polygonVertices.length();
		/// \todo There's no need to reverse the winding order here - cortex winding order matches maya's.
		/// When fixing this we need to change the order we iterate in the s,t conversion code too.
		copy( MArrayIter<MIntArray>::reverseBegin( polygonVertices ), MArrayIter<MIntArray>::reverseEnd( polygonVertices ), vertexIdsIt );
		vertexIdsIt += polygonVertices.length();
	}

	/// \todo Allow construction of empty meshes. Currently this MeshPrimitive constructor throws if there are no polygons. So we either need to change
	/// that behaviour, or call the MeshPrimitive() constructor if there are no polygons.
	MeshPrimitivePtr result = new MeshPrimitive( verticesPerFaceData, vertexIds, m_interpolation->getTypedValue() );
			
	if( m_points->getTypedValue() )
	{
		result->variables["P"] = PrimitiveVariable( PrimitiveVariable::Vertex, points() );
	}
	if( m_normals->getTypedValue() )
	{
		/// \todo THIS IS A STUPID THING TO BE DOING IF WE'RE USING SUBDIV INTERPOLATION. STOP IT.
		result->variables["N"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, normals() );
	}
	
	MString currentUVSet;
	fnMesh.getCurrentUVSetName( currentUVSet );
	MStringArray uvSets;
	fnMesh.getUVSetNames( uvSets );
	for( unsigned int i=0; i<uvSets.length(); i++ )
	{
		FloatVectorDataPtr sData = s( uvSets[i] );
		FloatVectorDataPtr tData = t( uvSets[i] );
		if( uvSets[i]==currentUVSet )
		{
			if( m_st->getTypedValue() )
			{
				result->variables["s"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, sData );
				result->variables["t"] = PrimitiveVariable( PrimitiveVariable::FaceVarying, tData );
			}
		}
		else
		{
			if( m_extraST->getTypedValue() )
			{
				MString sName = uvSets[i] + "_s";
				MString tName = uvSets[i] + "_t";
				result->variables[sName.asChar()] = PrimitiveVariable( PrimitiveVariable::FaceVarying, sData );
				result->variables[tName.asChar()] = PrimitiveVariable( PrimitiveVariable::FaceVarying, tData );
			}
		}
	}
	
	if( m_primVarAttrPrefix->getTypedValue()!="" )
	{
		addPrimVars( result, m_primVarAttrPrefix->getTypedValue().c_str() );
	}
	
	FromMayaRenderableConverterUtil::addBlindDataAttributes( operands, object, result );

	return result;
}

IECore::IntParameterPtr FromMayaMeshConverter::spaceParameter()
{
	ExtraData *extraData = g_classData[this];
	assert( extraData );	
	
	return extraData->m_space;
}

IECore::ConstIntParameterPtr FromMayaMeshConverter::spaceParameter() const
{
	ExtraData *extraData = g_classData[this];
	assert( extraData );	
	
	return extraData->m_space;
}

MSpace::Space FromMayaMeshConverter::space() const
{
	ExtraData *extraData = g_classData[this];
	assert( extraData );	
	
	Space s = static_cast< Space > ( extraData->m_space->getNumericValue() );
	
	switch ( s )
	{
		case Transform:     return MSpace::kTransform;
		case PreTransform:  return MSpace::kPreTransform;
		case PostTransform: return MSpace::kPostTransform;
		case Object:        return MSpace::kObject;
		case World:         return MSpace::kWorld;
		default:
			assert( false );
			return MSpace::kTransform;
	}
}
