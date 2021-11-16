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

using namespace IECoreUSD;
using namespace pxr;


namespace
{
static const pxr::TfToken g_cortexPrimitiveVariableMetadataToken( "IECOREUSD_CONSTANT_PRIMITIVE_VARIABLE" );
static const std::string g_primVarPrefix = "primvars:";
static const std::string g_primVarUserPrefix = "primvars:user:";
static const std::string g_renderPrefix = "render:";
static const std::string g_userPrefix = "user:";
}

pxr::TfToken IECoreUSD::AttributeAlgo::cortexPrimitiveVariableMetadataToken()
{
	return g_cortexPrimitiveVariableMetadataToken;
}

pxr::TfToken IECoreUSD::AttributeAlgo::toUSD( const std::string& name )
{
	// user:foo -> primvars:user:foo
	if ( boost::starts_with( name, g_userPrefix ) )
	{
		return TfToken( g_primVarPrefix + name );
	}
	// render:foo -> primvars:foo
	else if ( boost::starts_with( name, g_renderPrefix ) )
	{
		auto attributeName = name;
		boost::replace_first( attributeName, g_renderPrefix, g_primVarPrefix );
		return TfToken( attributeName );
	}
	// studio:foo -> studio:foo
	return TfToken( name );
}

IECore::InternedString IECoreUSD::AttributeAlgo::fromUSD( const std::string& name )
{
	// primvars:user:foo -> user:foo
	if ( boost::starts_with( name, g_primVarUserPrefix ) )
	{
		auto attributeName = name;
		boost::erase_first( attributeName, g_primVarPrefix );
		return IECore::InternedString( attributeName );
	}
	// replace first `primvars:` with `render:`
	else if ( boost::starts_with( name, g_primVarPrefix ) )
	{
		// primvars:foo -> render:foo
		auto attributeName = name;
		boost::replace_first( attributeName, g_primVarPrefix, g_renderPrefix );
		return IECore::InternedString( attributeName );
	}

	return IECore::InternedString( name );
}

IECore::InternedString IECoreUSD::AttributeAlgo::primVarPrefix()
{
	return g_primVarPrefix;
}
std::string IECoreUSD::AttributeAlgo::userPrefix()
{
	return g_userPrefix;
}
std::string IECoreUSD::AttributeAlgo::renderPrefix()
{
	return g_renderPrefix;
}
