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

#include <cassert>
#include <iostream>

#include "maya/MFnNumericAttribute.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MFnPluginData.h"

#include "IECore/PointsPrimitive.h"

#include "IECoreMaya/ObjectData.h"

#include "typeIds/TypeIds.h"

#include "TestObjectDataNode.h"

using namespace IECore;
using namespace IECoreMaya;

/// \todo Find a better ID!
MTypeId TestObjectDataNode::id( 0x80555 );

MObject TestObjectDataNode::aObjectDataIn;
MObject TestObjectDataNode::aObjectDataOut;
MObject TestObjectDataNode::aObjectDataOutCount;

TestObjectDataNode::TestObjectDataNode()
{
}

TestObjectDataNode::~TestObjectDataNode()
{
}

void *TestObjectDataNode::creator()
{
	return new TestObjectDataNode;
}

void TestObjectDataNode::postConstructor()
{
	MStatus s;
	MFnPluginData fnData;
	MObject plugData = fnData.create( ObjectData::id );
	assert( plugData != MObject::kNullObj );
	
	s = fnData.setObject( plugData );
	assert(s);
	
	ObjectData* data = dynamic_cast<ObjectData *>( fnData.data(&s) );
	assert(s);
	assert(data);
	
	V3fVectorDataPtr points = new V3fVectorData();
	for (unsigned i = 0; i < 100; i++)
	{
		points->writable().push_back( Imath::V3f( i, i, i ) );
	}
	
	PointsPrimitivePtr pointsPrimitive = new PointsPrimitive( points );
	
	data->setObject( pointsPrimitive );
	
	MPlug plug( thisMObject(), aObjectDataIn );
	
	plug.setValue( plugData );
}

MStatus TestObjectDataNode::initialize()
{
	MFnTypedAttribute tAttr;
	MFnNumericAttribute nAttr;
	MStatus s;
	
	aObjectDataIn = tAttr.create("objectDataIn", "odi", ObjectData::id );
	tAttr.setReadable(true);
	tAttr.setWritable(true);
	tAttr.setStorable(false);
	s = addAttribute( aObjectDataIn );
	assert(s);
	
	aObjectDataOut = tAttr.create("objectDataOut", "odo", ObjectData::id );
	tAttr.setReadable(true);
	tAttr.setWritable(false);
	tAttr.setStorable(false);
	s = addAttribute( aObjectDataOut );	
	assert(s);
	
	aObjectDataOutCount = nAttr.create("objectDataOutCount", "odor", MFnNumericData::kInt, true);
	nAttr.setReadable(true);
	nAttr.setWritable(false);
	nAttr.setStorable(false);
	nAttr.setDefault( 0 );
	s = addAttribute( aObjectDataOutCount );	
	assert(s);
	
	s = attributeAffects( aObjectDataIn, aObjectDataOut );
	s = attributeAffects( aObjectDataIn, aObjectDataOutCount );
	
	return MS::kSuccess;
}

MStatus TestObjectDataNode::compute( const MPlug &plug, MDataBlock &block )
{
	if (plug != aObjectDataOut && plug != aObjectDataOutCount)
	{
		return MS::kUnknownParameter;
	}
	
	/// Simple pass-through
	
	MDataHandle inH = block.inputValue( aObjectDataIn );
	MObject inData = inH.data();
	
	MFnPluginData fnData( inData );
	const ObjectData* objectData = dynamic_cast<const ObjectData*>(fnData.constData() );
	assert(objectData);
	ConstPointsPrimitivePtr points = boost::dynamic_pointer_cast<const PointsPrimitive>( objectData->getObject() );
	assert(points);
	
	MDataHandle outH;
	outH = block.outputValue( aObjectDataOut );
	outH.set( inData );
	outH.setClean();
	
	outH = block.outputValue( aObjectDataOutCount );
	outH.set( (int)(points->getNumPoints()) );
	outH.setClean();
	
	return MS::kSuccess;
}

