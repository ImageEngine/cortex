//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include "maya/MFnTypedAttribute.h"
#include "maya/MFnPluginData.h"

#include "IECoreScene/CurvesMergeOp.h"

#include "IECoreMaya/CurveCombiner.h"
#include "IECoreMaya/MayaTypeIds.h"
#include "IECoreMaya/ObjectData.h"
#include "IECoreMaya/FromMayaCurveConverter.h"

using namespace IECoreMaya;

const MTypeId CurveCombiner::id = CurveCombinerId;
const MString CurveCombiner::typeName = "ieCurveCombiner";
MObject CurveCombiner::aInputCurves;
MObject CurveCombiner::aOutputCurves;

CurveCombiner::CurveCombiner()
{
}

CurveCombiner::~CurveCombiner()
{
}

void *CurveCombiner::creator()
{
	return new CurveCombiner;
}

MStatus CurveCombiner::initialize()
{

	MFnTypedAttribute fnTAttr;

	aInputCurves = fnTAttr.create( "inputCurves", "ic", MFnData::kNurbsCurve );
	fnTAttr.setReadable( false );
	fnTAttr.setWritable( true );
	fnTAttr.setStorable( true );
	fnTAttr.setConnectable( true );
	fnTAttr.setHidden( false );
	fnTAttr.setArray( true );
	fnTAttr.setIndexMatters( false );

	addAttribute( aInputCurves );

	aOutputCurves = fnTAttr.create( "outputCurves", "oc", IECoreMaya::ObjectData::id );
	fnTAttr.setReadable( true );
	fnTAttr.setWritable( false );
	fnTAttr.setStorable( true );
	fnTAttr.setConnectable( true );
	fnTAttr.setHidden( false );

	addAttribute( aOutputCurves );

	attributeAffects( aInputCurves, aOutputCurves );

	return MS::kSuccess;
}

MStatus CurveCombiner::compute( const MPlug &plug, MDataBlock &dataBlock )
{

	if( plug==aOutputCurves )
	{

		MArrayDataHandle arrayHandle = dataBlock.inputArrayValue( aInputCurves );
		IECoreScene::CurvesPrimitivePtr combinedCurves = 0;

		IECoreScene::CurvesMergeOpPtr curvesMergeOp = new IECoreScene::CurvesMergeOp();
		curvesMergeOp->copyParameter()->setTypedValue( false );

		unsigned numCurves = arrayHandle.elementCount();
		for( unsigned curveIndex = 0; curveIndex < numCurves; curveIndex++, arrayHandle.next() )
		{
			MObject curve = arrayHandle.inputValue().asNurbsCurve();
			FromMayaCurveConverterPtr converter = new FromMayaCurveConverter( curve );
			// we want worldspace points if a worldShape is connected, and local otherwise
			converter->spaceParameter()->setNumericValue( FromMayaShapeConverter::World );
			IECoreScene::CurvesPrimitivePtr cortexCurve = boost::static_pointer_cast<IECoreScene::CurvesPrimitive>( converter->convert() );

			if( !combinedCurves )
			{
				combinedCurves = cortexCurve;
				curvesMergeOp->inputParameter()->setValue( combinedCurves );
			}
			else
			{
				curvesMergeOp->curvesParameter()->setValue( cortexCurve );
				curvesMergeOp->operate();
			}
		}

		if( !combinedCurves )
		{
			combinedCurves = new IECoreScene::CurvesPrimitive();
		}

		MFnPluginData fnD;
		MObject data = fnD.create( IECoreMaya::ObjectData::id );
		IECoreMaya::ObjectData *objectData = dynamic_cast<IECoreMaya::ObjectData *>( fnD.data() );
		objectData->setObject( combinedCurves );

		dataBlock.outputValue( aOutputCurves ).set( objectData );
		dataBlock.setClean( aOutputCurves );

		return MS::kSuccess;

	}

	return MS::kUnknownParameter;
}

