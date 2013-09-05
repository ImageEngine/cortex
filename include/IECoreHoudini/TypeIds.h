//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_TYPEIDS_H
#define IECOREHOUDINI_TYPEIDS_H

namespace IECoreHoudini
{

	/// Define Cortex Type Ids for our converter class.
	enum TypeId
	{
		FromHoudiniConverterTypeId = 111000,
		FromHoudiniGeometryConverterTypeId = 111001,
		FromHoudiniPointsConverterTypeId = 111002,
		FromHoudiniPolygonsConverterTypeId = 111003,
		ToHoudiniConverterTypeId = 111004,
		ToHoudiniGeometryConverterTypeId = 111005,
		ToHoudiniPointsConverterTypeId = 111006,
		ToHoudiniPolygonsConverterTypeId = 111007,
		FromHoudiniCurvesConverterTypeId = 111008,
		ToHoudiniCurvesConverterTypeId = 111009,
		ToHoudiniAttribConverterTypeId = 111010,
		ToHoudiniFloatDetailAttribConverterTypeId = 111011,
		ToHoudiniV2fDetailAttribConverterTypeId = 111012,
		ToHoudiniV3fDetailAttribConverterTypeId = 111013,
		ToHoudiniColor3fDetailAttribConverterTypeId = 111014,
		ToHoudiniIntDetailAttribConverterTypeId = 111015,
		ToHoudiniV2iDetailAttribConverterTypeId = 111016,
		ToHoudiniV3iDetailAttribConverterTypeId = 111017,
		ToHoudiniFloatVectorAttribConverterTypeId = 111018,
		ToHoudiniV2fVectorAttribConverterTypeId = 111019,
		ToHoudiniV3fVectorAttribConverterTypeId = 111020,
		ToHoudiniColor3fVectorAttribConverterTypeId = 111021,
		ToHoudiniIntVectorAttribConverterTypeId = 111022,
		ToHoudiniV2iVectorAttribConverterTypeId = 111023,
		ToHoudiniV3iVectorAttribConverterTypeId = 111024,
		FromHoudiniGroupConverterTypeId = 111025,
		ToHoudiniGroupConverterTypeId = 111026,
		ToHoudiniStringDetailAttribConverterTypeId = 111027,
		ToHoudiniStringVectorAttribConverterTypeId = 111028,
		RATDeepImageReaderTypeId = 111029,
		RATDeepImageWriterTypeId = 111030,
		HoudiniSceneTypeId = 111031,
		FromHoudiniCortexObjectConverterTypeId = 111032,
		ToHoudiniCortexObjectConverterTypeId = 111033,
		// remember to update TypeIdBinding.cpp
		LastTypeId = 111999,
	};

} // namespace IECoreHoudini

#endif /* IECOREHOUDINI_TYPEIDS_H */
