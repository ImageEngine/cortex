//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2020, Cinesite VFX Ltd. All rights reserved.
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

#ifndef IECOREUSD_PRIMITIVEALGO_H
#define IECOREUSD_PRIMITIVEALGO_H

#include "IECoreScene/Primitive.h"
#include "IECoreScene/PrimitiveVariable.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/usd/usdGeom/pointBased.h"
#include "pxr/usd/usdGeom/primvarsAPI.h"
IECORE_POP_DEFAULT_VISIBILITY

namespace IECoreUSD
{

namespace PrimitiveAlgo
{

/// From Cortex to USD
/// ==================

/// Writes a PrimitiveVariable to USD, creating a primvar via `primvarsAPI`.
void writePrimitiveVariable( const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable, const pxr::UsdGeomPrimvarsAPI &primvarsAPI, pxr::UsdTimeCode time );
/// As above, but redirects "P", "N" etc to the relevant attributes of `pointBased`.
void writePrimitiveVariable( const std::string &name, const IECoreScene::PrimitiveVariable &primitiveVariable, pxr::UsdGeomPointBased &pointBased, pxr::UsdTimeCode time );
/// Equivalent to `DataAlgo::toUSD( primitiveVariable.expandedData() )`, but avoiding
/// the creation of the temporary expanded data.
pxr::VtValue toUSDExpanded( const IECoreScene::PrimitiveVariable &primitiveVariable );
/// Converts interpolation to USD.
pxr::TfToken toUSD( IECoreScene::PrimitiveVariable::Interpolation interpolation );

/// From USD to Cortex
/// ==================

/// Reads all primvars from `primvarsAPI`, adding them to `primitive`.
void readPrimitiveVariables( const pxr::UsdGeomPrimvarsAPI &primvarsAPI, pxr::UsdTimeCode timeCode, IECoreScene::Primitive *primitive );
/// As above, but also reads "P", "N" etc from `pointBased`.
void readPrimitiveVariables( const pxr::UsdGeomPointBased &pointBased, pxr::UsdTimeCode timeCode, IECoreScene::Primitive *primitive );
/// Returns true if any of the primitive variables might be animated.
bool primitiveVariablesMightBeTimeVarying( const pxr::UsdGeomPrimvarsAPI &primvarsAPI );
/// Returns true if any of the primitive variables might be animated, including the
/// "P", "N" etc that `readPrimitiveVariables()` creates.
bool primitiveVariablesMightBeTimeVarying( const pxr::UsdGeomPointBased &pointBased );
/// Converts interpolation from USD.
IECoreScene::PrimitiveVariable::Interpolation fromUSD( pxr::TfToken interpolationToken );

} // namespace PrimitiveAlgo

} // namespace IECoreUSD

#endif // IECOREUSD_PRIMITIVEALGO_H
