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

#include "maya/MFnNumericAttribute.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MTime.h"
#include "maya/MGlobal.h"

#include "typeIds/TypeIds.h"

#include "IECore/Exception.h"
#include "IECore/OversamplesCalculator.h"
#include "IECoreMaya/CacheSet.h"
#include "IECoreMaya/MayaTime.h"

using namespace IECore;
using namespace IECoreMaya;

MObject CacheSet::aActive;
MObject CacheSet::aFrameRate;
MObject CacheSet::aOversamples;		
MObject CacheSet::aActualOversamples;		
MObject CacheSet::aOutFrameMel;

MTypeId CacheSet::id( CacheSetId );

CacheSet::CacheSet()
{
}

CacheSet::~CacheSet()
{
}

void *CacheSet::creator()
{
	return new CacheSet;
}

bool CacheSet::isAbstractClass() const
{
	return true;
}

MStatus CacheSet::initialize()
{
	MStatus s;
	MFnNumericAttribute nAttr;
	MFnTypedAttribute tAttr;
	
	aActive = nAttr.create("active", "a", MFnNumericData::kBoolean, true, &s);
	nAttr.setReadable(true);
	nAttr.setWritable(true);	
	nAttr.setStorable(true);	
	nAttr.setKeyable(true);	

	aFrameRate = nAttr.create("frameRate", "fr", MFnNumericData::kDouble, 24.0, &s);
	nAttr.setReadable(true);
	nAttr.setWritable(true);
	nAttr.setStorable(true);
	nAttr.setMin(1.0);
	
	aOversamples = nAttr.create("oversamples", "os", MFnNumericData::kInt, 1, &s);
	nAttr.setReadable(true);
	nAttr.setWritable(true);
	nAttr.setStorable(true);
	nAttr.setMin(1);

	aActualOversamples = nAttr.create("actualOversamples", "aos", MFnNumericData::kInt, 1, &s);
	nAttr.setReadable(true);
	nAttr.setWritable(false);
	nAttr.setStorable(true);
	
	aOutFrameMel = tAttr.create("outFrameMel", "ofc", MFnData::kString);
	tAttr.setWritable(false);
	tAttr.setReadable(true);
	
	
	s = addAttribute(aActive);
	assert(s);

	s = addAttribute(aFrameRate);
	assert(s);

	s = addAttribute(aOversamples);
	assert(s);

	s = addAttribute(aActualOversamples);
	assert(s);
	
	s = addAttribute(aOutFrameMel);
	assert(s);
	
	s = attributeAffects(aActive, aOutFrameMel);
	assert(s);

	s = attributeAffects(aFrameRate, aActualOversamples);
	assert(s);

	s = attributeAffects(aOversamples, aActualOversamples);
	assert(s);
	
	return MS::kSuccess;
}

MStatus CacheSet::compute(const MPlug &plug, MDataBlock &block)
{
	if (plug != aActualOversamples)
	{
		return MS::kUnknownParameter;
	}
	
	MStatus s;
		
	MDataHandle frameRateH = block.inputValue( aFrameRate );
	double frameRate = frameRateH.asDouble();
	
	MDataHandle oversamplesH = block.inputValue( aOversamples );
	int oversamples = oversamplesH.asInt();

	MDataHandle actualOversamplesH = block.outputValue( aActualOversamples, &s );

	// frameRate should match UI time units, 
	// so we can use the mel command currentTime for caching very easily.
	if (frameRate != MayaTime::fps( MTime::uiUnit() ) )
	{
		MGlobal::displayError( "The frame rate attribute does not match current time unit. Caching will not save the expected frames." );
		return MS::kFailure;
	}

	try {
		OversamplesCalculator6kFPS oversamplesCalc( frameRate, oversamples );
		oversamples = oversamplesCalc.actualOversamples();
		actualOversamplesH.set( oversamples );
		s = MStatus::kSuccess;
	}
	catch (IECore::Exception &e)
	{
		MString err = e.type();
		err += ": ";
		err += e.what();
			
		MGlobal::displayError(err);
		return MS::kFailure;	
	}	
	catch (...)
	{
		MString err = "Unknown error computing actual oversamples.";

		MGlobal::displayError(err);
		return MS::kFailure;	
	}
	
	return s;
}

MString CacheSet::melFromStringArray(const MStringArray &a) const
{
	MString mel = "{";
	for (unsigned i = 0; i < a.length(); i++)
	{
		if (i != 0)
		{
			mel += ", ";	
		}	
		mel += "\"";	
		mel += a[i];
		mel += "\"";
	}
	mel += "}";
	
	return mel;
}
