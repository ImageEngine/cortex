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

#include "IECoreHoudini/ToHoudiniNumericAttribConverter.h"

using namespace IECore;

namespace IECoreHoudini
{

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniFloatVectorAttribConverter, ToHoudiniFloatVectorAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniV2fVectorAttribConverter, ToHoudiniV2fVectorAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniV3fVectorAttribConverter, ToHoudiniV3fVectorAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniColor3fVectorAttribConverter, ToHoudiniColor3fVectorAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniIntVectorAttribConverter, ToHoudiniIntVectorAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniV2iVectorAttribConverter, ToHoudiniV2iVectorAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniV3iVectorAttribConverter, ToHoudiniV3iVectorAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniM33fVectorAttribConverter, ToHoudiniM33fVectorAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniM44fVectorAttribConverter, ToHoudiniM44fVectorAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniM44dVectorAttribConverter, ToHoudiniM44dVectorAttribConverterTypeId )

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniFloatDetailAttribConverter, ToHoudiniFloatDetailAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniV2fDetailAttribConverter, ToHoudiniV2fDetailAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniV3fDetailAttribConverter, ToHoudiniV3fDetailAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniColor3fDetailAttribConverter, ToHoudiniColor3fDetailAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniIntDetailAttribConverter, ToHoudiniIntDetailAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniV2iDetailAttribConverter, ToHoudiniV2iDetailAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniV3iDetailAttribConverter, ToHoudiniV3iDetailAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniM33fDetailAttribConverter, ToHoudiniM33fDetailAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniM44fDetailAttribConverter, ToHoudiniM44fDetailAttribConverterTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( ToHoudiniM44dDetailAttribConverter, ToHoudiniM44dDetailAttribConverterTypeId )

// Explicit instantiations for numeric vector classes
template class ToHoudiniNumericVectorAttribConverter<IECore::FloatVectorData>;
template class ToHoudiniNumericVectorAttribConverter<IECore::V2fVectorData>;
template class ToHoudiniNumericVectorAttribConverter<IECore::V3fVectorData>;
template class ToHoudiniNumericVectorAttribConverter<IECore::Color3fVectorData>;
template class ToHoudiniNumericVectorAttribConverter<IECore::IntVectorData>;
template class ToHoudiniNumericVectorAttribConverter<IECore::V2iVectorData>;
template class ToHoudiniNumericVectorAttribConverter<IECore::V3iVectorData>;
template class ToHoudiniNumericVectorAttribConverter<IECore::M33fVectorData>;
template class ToHoudiniNumericVectorAttribConverter<IECore::M44fVectorData>;
template class ToHoudiniNumericVectorAttribConverter<IECore::M44dVectorData>;

// Explicit instantiations for numeric detail classes
template class ToHoudiniNumericDetailAttribConverter<IECore::FloatData>;
template class ToHoudiniNumericDetailAttribConverter<IECore::V2fData>;
template class ToHoudiniNumericDetailAttribConverter<IECore::V3fData>;
template class ToHoudiniNumericDetailAttribConverter<IECore::Color3fData>;
template class ToHoudiniNumericDetailAttribConverter<IECore::IntData>;
template class ToHoudiniNumericDetailAttribConverter<IECore::V2iData>;
template class ToHoudiniNumericDetailAttribConverter<IECore::V3iData>;
template class ToHoudiniNumericDetailAttribConverter<IECore::M33fData>;
template class ToHoudiniNumericDetailAttribConverter<IECore::M44fData>;
template class ToHoudiniNumericDetailAttribConverter<IECore::M44dData>;

} // namespace IECoreHoudini
