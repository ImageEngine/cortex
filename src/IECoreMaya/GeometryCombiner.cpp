//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#include "maya/MFnGenericAttribute.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MFnNumericAttribute.h"
#include "maya/MFnEnumAttribute.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MFnPluginData.h"
#include "maya/MFnStringData.h"
#include "maya/MPlugArray.h"
#include "maya/MDagPath.h"

#include "IECore/Group.h"

#include "IECoreMaya/GeometryCombiner.h"
#include "IECoreMaya/MayaTypeIds.h"
#include "IECoreMaya/ObjectData.h"
#include "IECoreMaya/FromMayaShapeConverter.h"

using namespace IECoreMaya;

const MTypeId GeometryCombiner::id = GeometryCombinerId;
const MString GeometryCombiner::typeName = "ieGeometryCombiner";
MObject GeometryCombiner::aConvertPrimVars;
MObject GeometryCombiner::aConvertBlindData;
MObject GeometryCombiner::aBlindDataAttrPrefix;
MObject GeometryCombiner::aConversionSpace;
MObject GeometryCombiner::aInputGeometry;
MObject GeometryCombiner::aOutputGroup;

GeometryCombiner::GeometryCombiner()
{
}

GeometryCombiner::~GeometryCombiner()
{
}

void *GeometryCombiner::creator()
{
	return new GeometryCombiner;
}

MStatus GeometryCombiner::initialize()
{

	MFnNumericAttribute fnNAttr;
	MFnEnumAttribute fnEAttr;
	MFnTypedAttribute tAttr;

	aConvertPrimVars = fnNAttr.create( "convertPrimVars", "cpv", MFnNumericData::kBoolean, 0.0f );
	addAttribute( aConvertPrimVars );

	aBlindDataAttrPrefix = tAttr.create( "blindDataAttrPrefix", "bda", MFnData::kString, MFnStringData().create( "" ) );
	addAttribute( aBlindDataAttrPrefix );
	
	aConvertBlindData = fnNAttr.create( "convertBlindData", "cbd", MFnNumericData::kBoolean, 0.0f );
	addAttribute( aConvertBlindData );

	aConversionSpace = fnEAttr.create( "conversionSpace", "cs", FromMayaShapeConverter::World );
    fnEAttr.addField( "World", FromMayaShapeConverter::World );
    fnEAttr.addField( "Object", FromMayaShapeConverter::Object );
	addAttribute( aConversionSpace );
	
	MFnGenericAttribute fnGAttr;
	
	aInputGeometry = fnGAttr.create( "inputGeometry", "ig" );
	fnGAttr.addDataAccept( MFnData::kMesh );
	fnGAttr.addDataAccept( MFnData::kNurbsCurve );
	fnGAttr.setReadable( false );
	fnGAttr.setWritable( true );
	fnGAttr.setStorable( false );
	fnGAttr.setConnectable( true );
	fnGAttr.setHidden( false );
	fnGAttr.setArray( true );
	fnGAttr.setIndexMatters( false );
	addAttribute( aInputGeometry );

	MFnTypedAttribute fnTAttr;
	
	aOutputGroup = fnTAttr.create( "outputGroup", "og", ObjectData::id );
	fnTAttr.setReadable( true );
	fnTAttr.setWritable( false );
	fnTAttr.setStorable( true );
	fnTAttr.setConnectable( true );
	fnTAttr.setHidden( false );
	addAttribute( aOutputGroup );
	
	attributeAffects( aConvertPrimVars, aOutputGroup );
	attributeAffects( aBlindDataAttrPrefix, aOutputGroup );
	attributeAffects( aConvertBlindData, aOutputGroup );
	attributeAffects( aConversionSpace, aOutputGroup );
	attributeAffects( aInputGeometry, aOutputGroup );
	
	return MS::kSuccess;
}

MStatus GeometryCombiner::compute( const MPlug &plug, MDataBlock &dataBlock )
{

	if( plug==aOutputGroup )
	{
	
		bool convertPrimVars = dataBlock.inputValue( aConvertPrimVars ).asBool();
		MString blindDataAttrPrefix = dataBlock.inputValue( aBlindDataAttrPrefix ).asString();
		bool convertBlindData = dataBlock.inputValue( aConvertBlindData ).asBool();
		FromMayaShapeConverter::Space conversionSpace = (FromMayaShapeConverter::Space)dataBlock.inputValue( aConversionSpace ).asInt();
	
		IECore::GroupPtr group = new IECore::Group;
			
		MArrayDataHandle arrayHandle = dataBlock.inputArrayValue( aInputGeometry );

		unsigned numInputs = arrayHandle.elementCount();
		for( unsigned inputIndex = 0; inputIndex < numInputs; ++inputIndex, arrayHandle.next() )
		{
			// whether we go the cheating route below (where we access our input nodes) or not, it's
			// essential that we pull on our input plugs.
			MObject input = arrayHandle.inputValue().data();
			
			FromMayaShapeConverterPtr converter = 0;
			if( !convertPrimVars && !convertBlindData )
			{
				// we can play it by the book and just convert the data presented to us in the datablock.
				converter = IECore::runTimeCast<FromMayaShapeConverter>( FromMayaObjectConverter::create( input ) );
			}
			else
			{
				// we need to cheat and find the incoming node in order to be able to convert primvars
				// and/or blinddata.
				MPlug inputGeometryPlug( thisMObject(), aInputGeometry );
				MPlug element = inputGeometryPlug.elementByLogicalIndex( arrayHandle.elementIndex() );
				MPlugArray inputConnections;
				element.connectedTo( inputConnections, true, false );
				if( inputConnections.length() )
				{
					MDagPath path;
					MDagPath::getAPathTo( inputConnections[0].node(), path );
					converter = FromMayaShapeConverter::create( path );					
				}
			
			}
			
			if( converter )
			{
				converter->spaceParameter()->setNumericValue( conversionSpace );

				if( !convertPrimVars )
				{
					converter->primVarAttrPrefixParameter()->setTypedValue( "" );
				}
				if( convertBlindData )
				{
					converter->blindDataAttrPrefixParameter()->setTypedValue( blindDataAttrPrefix.asChar() );
				}
				else
				{
					converter->blindDataAttrPrefixParameter()->setTypedValue( "" );
				}

				IECore::VisibleRenderablePtr cortexGeometry = IECore::runTimeCast<IECore::VisibleRenderable>( converter->convert() );

				if( cortexGeometry )
				{
					group->addChild( cortexGeometry );
				}
			}			
		}
		
		MFnPluginData fnD;
		MObject data = fnD.create( IECoreMaya::ObjectData::id );
		IECoreMaya::ObjectData *objectData = dynamic_cast<IECoreMaya::ObjectData *>( fnD.data() );
		objectData->setObject( group );
		
		dataBlock.outputValue( aOutputGroup ).set( objectData );
		dataBlock.setClean( aOutputGroup );

		return MS::kSuccess;
	
	}

	return MS::kUnknownParameter;
}

