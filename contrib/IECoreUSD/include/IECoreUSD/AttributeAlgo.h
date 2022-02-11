//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2021, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREUSD_ATTRIBUTEALGO_H
#define IECOREUSD_ATTRIBUTEALGO_H

#include "IECoreUSD/Export.h"

#include "IECoreScene/SceneInterface.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/base/tf/token.h"
#include "pxr/usd/usd/attribute.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usdGeom/primvar.h"
IECORE_POP_DEFAULT_VISIBILITY

// AttributeAlgo is suite of utilities for loading/writing Cortex/USD Attributes.

namespace IECoreUSD
{

namespace AttributeAlgo
{

// Find a UsdAttribute under the given prim which matches the given cortex name.  This UsdAttribute
// could be either a constant primvar or a custom attribute with an appropriate name.  If no matching
// UsdAttribute is found, returns an invalid UsdAttribute
IECOREUSD_API pxr::UsdAttribute findUSDAttribute( const pxr::UsdPrim &prim, std::string cortexName );

// Return the cortex attribute corresponding to a UsdAttribute.  There will be a corresponding Cortex
// name if the UsdAttribute corresponds to a constant primvar which should be loaded as an attribute,
// or a custom UsdAttribute.  If the UsdAttribute corresponds to a primvar that we load as a primvar,
// or a non-custom primvar, then an empty string is returned.
IECOREUSD_API IECore::InternedString cortexAttributeName( const pxr::UsdAttribute &attribute );

// Returns true if the primvar should be loaded as a Cortex attribute rather than
// a PrimitiveVariable.
IECOREUSD_API bool isCortexAttribute( const pxr::UsdGeomPrimvar &primVar );

struct Name
{
	pxr::TfToken name;
	bool isPrimvar;
};

IECOREUSD_API Name nameToUSD( std::string name );
IECOREUSD_API IECore::InternedString nameFromUSD( Name name );

IECOREUSD_API pxr::TfToken cortexPrimitiveVariableMetadataToken();
IECOREUSD_API pxr::TfToken cortexPrimitiveVariableMetadataTokenDeprecated();

} // namespace AttributeAlgo

} // namespace IECoreUSD

#endif // IECOREUSD_ATTRIBUTEALGO_H
