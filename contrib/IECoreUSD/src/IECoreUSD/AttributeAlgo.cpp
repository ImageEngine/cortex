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

#include "IECoreUSD/AttributeAlgo.h"
#include "boost/algorithm/string/erase.hpp"
#include "boost/algorithm/string/replace.hpp"
#include "boost/algorithm/string/predicate.hpp"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/usd/usdGeom/primvar.h"
#include "pxr/usd/usdGeom/primvarsAPI.h"

#include "pxr/usd/usdLux/lightAPI.h"
IECORE_POP_DEFAULT_VISIBILITY

using namespace IECoreUSD;
using namespace pxr;

namespace
{

static const pxr::TfToken g_cortexPrimitiveVariableMetadataToken( "cortex_isConstantPrimitiveVariable" );
static const pxr::TfToken g_cortexPrimitiveVariableMetadataTokenDeprecated( "IECOREUSD_CONSTANT_PRIMITIVE_VARIABLE" );
static const std::string g_primVarPrefix = "primvars:";
static const std::string g_primVarUserPrefix = "primvars:user:";
static const std::string g_renderPrefix = "render:";
static const std::string g_userPrefix = "user:";

}

bool IECoreUSD::AttributeAlgo::isCortexAttribute( const pxr::UsdGeomPrimvar &primVar )
{
	if( primVar.GetInterpolation() != pxr::UsdGeomTokens->constant )
	{
		return false;
	}

	// We have a constant primvar. Check the metadata to see if it has been
	// tagged as a true primvar and not an attribute. If the metadata exists,
	// that is the final word on the matter.

	pxr::VtValue metadataValue;
	if( primVar.GetAttr().GetMetadata( AttributeAlgo::cortexPrimitiveVariableMetadataToken(), &metadataValue ) )
	{
		return !metadataValue.Get<bool>();
	}
	else if( primVar.GetAttr().GetMetadata( AttributeAlgo::cortexPrimitiveVariableMetadataTokenDeprecated(), &metadataValue ) )
	{
		return !metadataValue.Get<bool>();
	}

	// Check for companion `<name>:lengths` primvar. This is a convention
	// Houdini uses to store varying-length-array primvars per-vertex or
	// per-face. We want to load the two primvars side by side as primitive
	// variables.

	const TfToken lengthsName( primVar.GetName().GetString() + ":lengths" );
	if( auto lengthsPrimVar = UsdGeomPrimvarsAPI( primVar.GetAttr().GetPrim() ).GetPrimvar( lengthsName ) )
	{
		if( lengthsPrimVar.GetInterpolation() != pxr::UsdGeomTokens->constant )
		{
			return false;
		}
	}

	// Check for `arnold:*` primvars on lights. These will be loaded as
	// parameters in `ShaderAlgo::readLight()`.

	if(
		boost::starts_with( primVar.GetPrimvarName().GetString(), "arnold:" ) &&
		pxr::UsdLuxLightAPI( primVar.GetAttr().GetPrim() )
	)
	{
		return false;
	}

	// Everything else should be loaded as a Cortex attribute.

	return true;
}

pxr::TfToken IECoreUSD::AttributeAlgo::cortexPrimitiveVariableMetadataToken()
{
	return g_cortexPrimitiveVariableMetadataToken;
}

pxr::TfToken IECoreUSD::AttributeAlgo::cortexPrimitiveVariableMetadataTokenDeprecated()
{
	return g_cortexPrimitiveVariableMetadataTokenDeprecated;
}

IECoreUSD::AttributeAlgo::Name IECoreUSD::AttributeAlgo::nameToUSD( std::string name )
{
	bool isPrimvar = false;

	// The long term plan is to convert only "render:" prefixed attributes to primvars, and it will
	// be the client's responsibility to ensure everything important gets prefixed with "render:".
	// But for the moment, Gaffer doesn't do this yet, so we support the two most important prefixes
	// for Gaffer currently:  "user:" and "ai:"
	if( boost::starts_with( name, "render:" ) || boost::starts_with( name, "user:" ) || boost::starts_with( name, "ai:" ) )
	{
		isPrimvar = true;

		// Strip the "render:" prefix from when writing attributes as primitive variables
		if( boost::starts_with( name, g_renderPrefix ) )
		{
			name = name.substr( 7 );
		}
	}

	if( name == "ai:disp_map" )
	{
		// Special case where the whole name is different, not just prefix
		name = "arnold:displacement";
	}
	else
	{
		size_t colonPos = name.find( ":" );
		if( colonPos != std::string::npos )
		{
			std::string prefix = name.substr( 0, colonPos );
			std::string newPrefix;
			// Translate prefixes.  Currently ai -> arnold is the only mapping supported
			if( prefix == "ai" )
			{
				newPrefix = "arnold";
			}

			if( newPrefix.size() )
			{
				name = newPrefix + name.substr( colonPos );
			}
		}
	}

	return { TfToken( name ), isPrimvar };
}

IECore::InternedString IECoreUSD::AttributeAlgo::nameFromUSD( IECoreUSD::AttributeAlgo::Name name )
{
	std::string nameStr = name.name;
	if( nameStr == "arnold:displacement" )
	{
		// Special case where the whole name is different, not just prefix
		nameStr = "ai:disp_map";
	}
	else
	{
		size_t colonPos = nameStr.find( ":" );
		if( colonPos != std::string::npos )
		{
			std::string prefix = nameStr.substr( 0, colonPos );
			std::string newPrefix;

			// Translate prefixes.  Currently arnold -> ai is the only mapping supported
			if( prefix == "arnold" )
			{
				newPrefix = "ai";
			}

			if( newPrefix.size() )
			{
				nameStr = newPrefix + nameStr.substr( colonPos );
			}
		}
	}

	// The long term plan is to always prefix primitive variables converted to attributes with "render:".
	// But for the moment, Gaffer doesn't support this, so we skip the prefix for the two most important prefixes
	// for Gaffer currently:  "user:" and "ai:"
	if ( !boost::starts_with( nameStr, g_userPrefix ) && !boost::starts_with( nameStr, "ai:" ) && name.isPrimvar )
	{
		nameStr = "render:" + nameStr;
	}

	return IECore::InternedString( nameStr );
}

UsdAttribute IECoreUSD::AttributeAlgo::findUSDAttribute( const pxr::UsdPrim &prim, std::string cortexName )
{
	AttributeAlgo::Name n = AttributeAlgo::nameToUSD( cortexName );
	if( n.isPrimvar )
	{
		if( pxr::UsdGeomPrimvar primvar = pxr::UsdGeomPrimvarsAPI( prim ).GetPrimvar( n.name ) )
		{
			if( isCortexAttribute( primvar ) )
			{
				return primvar.GetAttr();
			}
		}
	}

	// In theory, this should be able to be an else.  But for the moment, for attributes that should be written
	// to a primvar, we try reading them from an attribute if we can't find them in a primvar.  This provides
	// some backwards compatibility with files from before we started writing to primvars, and might provide
	// compatibility with other USD authors, maybe?
	if( pxr::UsdAttribute attribute = prim.GetAttribute( n.name ) )
	{
		if ( attribute.GetName().GetString().find( ":" ) != std::string::npos && attribute.IsCustom() )
		{
			return attribute;
		}
	}

	return UsdAttribute();
}

IECore::InternedString IECoreUSD::AttributeAlgo::cortexAttributeName( const pxr::UsdAttribute &attribute )
{
	if( pxr::UsdGeomPrimvar primvar = pxr::UsdGeomPrimvar( attribute ) )
	{
		if( isCortexAttribute( primvar ) )
		{
			return AttributeAlgo::nameFromUSD( { primvar.GetPrimvarName(), true } );
		}
	}
	else
	{
		const std::string &n = attribute.GetName().GetString();
		if ( n.find( ":" ) != std::string::npos && attribute.IsCustom() )
		{
			return AttributeAlgo::nameFromUSD( { attribute.GetName(), false } );
		}
	}
	return IECore::InternedString();
}

