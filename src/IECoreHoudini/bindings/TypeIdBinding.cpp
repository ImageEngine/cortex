//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2014, Image Engine Design Inc. All rights reserved.
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

#include <boost/python.hpp>

#include "IECoreHoudini/TypeIds.h"
#include "IECoreHoudini/bindings/TypeIdBinding.h"

using namespace boost::python;

namespace IECoreHoudini
{

void bindTypeId()
{
	enum_<TypeId>( "TypeId" )
		.value( "FromHoudiniConverter", FromHoudiniConverterTypeId )
		.value( "FromHoudiniGeometryConverter", FromHoudiniGeometryConverterTypeId )
		.value( "FromHoudiniPointsConverter", FromHoudiniPointsConverterTypeId )
		.value( "FromHoudiniPolygonsConverter", FromHoudiniPolygonsConverterTypeId )
		.value( "ToHoudiniConverter", ToHoudiniConverterTypeId )
		.value( "ToHoudiniGeometryConverter", ToHoudiniGeometryConverterTypeId )
		.value( "ToHoudiniPointsConverter", ToHoudiniPointsConverterTypeId )
		.value( "ToHoudiniPolygonsConverter", ToHoudiniPolygonsConverterTypeId )
		.value( "FromHoudiniCurvesConverter", FromHoudiniCurvesConverterTypeId )
		.value( "ToHoudiniCurvesConverter", ToHoudiniCurvesConverterTypeId )
		.value( "ToHoudiniAttribConverter", ToHoudiniAttribConverterTypeId )
		.value( "ToHoudiniFloatDetailAttribConverter", ToHoudiniFloatDetailAttribConverterTypeId )
		.value( "ToHoudiniV2fDetailAttribConverter", ToHoudiniV2fDetailAttribConverterTypeId )
		.value( "ToHoudiniV3fDetailAttribConverter", ToHoudiniV3fDetailAttribConverterTypeId )
		.value( "ToHoudiniColor3fDetailAttribConverter", ToHoudiniColor3fDetailAttribConverterTypeId )
		.value( "ToHoudiniIntDetailAttribConverter", ToHoudiniIntDetailAttribConverterTypeId )
		.value( "ToHoudiniV2iDetailAttribConverter", ToHoudiniV2iDetailAttribConverterTypeId )
		.value( "ToHoudiniV3iDetailAttribConverter", ToHoudiniV3iDetailAttribConverterTypeId )
		.value( "ToHoudiniFloatVectorAttribConverter", ToHoudiniFloatVectorAttribConverterTypeId )
		.value( "ToHoudiniV2fVectorAttribConverter", ToHoudiniV2fVectorAttribConverterTypeId )
		.value( "ToHoudiniV3fVectorAttribConverter", ToHoudiniV3fVectorAttribConverterTypeId )
		.value( "ToHoudiniColor3fVectorAttribConverter", ToHoudiniColor3fVectorAttribConverterTypeId )
		.value( "ToHoudiniIntVectorAttribConverter", ToHoudiniIntVectorAttribConverterTypeId )
		.value( "ToHoudiniV2iVectorAttribConverter", ToHoudiniV2iVectorAttribConverterTypeId )
		.value( "ToHoudiniV3iVectorAttribConverter", ToHoudiniV3iVectorAttribConverterTypeId )
		.value( "FromHoudiniGroupConverter", FromHoudiniGroupConverterTypeId )
		.value( "ToHoudiniGroupConverter", ToHoudiniGroupConverterTypeId )
		.value( "ToHoudiniStringDetailAttribConverter", ToHoudiniStringDetailAttribConverterTypeId )
		.value( "ToHoudiniStringVectorAttribConverter", ToHoudiniStringVectorAttribConverterTypeId )
		.value( "RATDeepImageReader", RATDeepImageReaderTypeId )
		.value( "RATDeepImageWriter", RATDeepImageWriterTypeId )
		.value( "LiveScene", LiveSceneTypeId )
		.value( "FromHoudiniCortexObjectConverter", FromHoudiniCortexObjectConverterTypeId )
		.value( "ToHoudiniCortexObjectConverter", ToHoudiniCortexObjectConverterTypeId )
		.value( "FromHoudiniCompoundObjectConverter", FromHoudiniCompoundObjectConverterTypeId )
		.value( "ToHoudiniCompoundObjectConverter", ToHoudiniCompoundObjectConverterTypeId )
	;
}

}
