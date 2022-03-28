//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2021, Image Engine Design. All rights reserved.
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

#ifndef IECOREUSD_SHADERALGO_H
#define IECOREUSD_SHADERALGO_H

#include "IECoreUSD/Export.h"

#include "IECoreScene/ShaderNetwork.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/usd/usdShade/material.h"
#include "pxr/usd/usdShade/output.h"
#if PXR_VERSION >= 2111
#include "pxr/usd/usdLux/lightAPI.h"
#endif
IECORE_POP_DEFAULT_VISIBILITY

namespace IECoreUSD
{

namespace ShaderAlgo
{

/// Write ShaderNetwork to USD, placing the shaders under the Prim `shaderContainer`
IECOREUSD_API pxr::UsdShadeOutput writeShaderNetwork( const IECoreScene::ShaderNetwork *shaderNetwork, pxr::UsdPrim shaderContainer );

/// Reads a ShaderNetwork from a material output, typically obtained from `UsdShadeMaterial::GetOutput()`.
IECoreScene::ShaderNetworkPtr readShaderNetwork( const pxr::UsdShadeOutput &output );

#if PXR_VERSION >= 2111
/// Reads a ShaderNetwork from a light.
IECoreScene::ShaderNetworkPtr readShaderNetwork( const pxr::UsdLuxLightAPI &light );
#endif

} // namespace ShaderAlgo

} // namespace IECoreUSD

#endif // IECOREUSD_SHADERALGO_H
