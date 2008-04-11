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

#include "IECoreMaya/ConverterHolder.h"
#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/MayaTypeIds.h"

#include "IECore/Object.h"

#include "maya/MFnMessageAttribute.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MFnStringData.h"

using namespace IECoreMaya;

MTypeId ConverterHolder::id = ConverterHolderId;
MObject ConverterHolder::aIn;
MObject ConverterHolder::aFileName;

ConverterHolder::ConverterHolder()
{
}

ConverterHolder::~ConverterHolder()
{
}

void *ConverterHolder::creator()
{
	return new ConverterHolder;
}

MStatus ConverterHolder::initialize()
{
	MStatus s = inheritAttributesFrom( ParameterisedHolderNode::typeName );
	if( !s )
	{
		return s;
	}
	
	MFnMessageAttribute fnMAttr;
	aIn = fnMAttr.create( "input", "in", &s );
	assert( s );
	s = addAttribute( aIn );
	assert( s );
	
	MFnTypedAttribute fnTAttr;
	MFnStringData fnSData;
	aFileName = fnTAttr.create( "fileName", "fn", MFnData::kString, fnSData.create( "" ) , &s );
	assert( s );
	s = addAttribute( aFileName );
	assert( s );
	
	return MS::kSuccess;
}

MStatus ConverterHolder::connectionMade( const MPlug &plug, const MPlug &otherPlug, bool asSrc )
{
	if( plug==aIn )
	{
		MObject otherNode = otherPlug.node();
		FromMayaConverterPtr converter = FromMayaObjectConverter::create( otherNode );
		setParameterised( converter );
	}
	return MS::kSuccess;
}

MStatus ConverterHolder::connectionBroken( const MPlug &plug, const MPlug &otherPlug, bool asSrc )
{
	if( plug==aIn )
	{
		setParameterised( 0 );
	}
	return MS::kSuccess;
}
