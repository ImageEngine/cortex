//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2025, Cinesite VFX Ltd. All rights reserved.
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

#include "IECoreUSD/DataAlgo.h"
#include "IECoreUSD/ObjectAlgo.h"

#include "IECoreScene/ExternalProcedural.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/usd/usdGeom/gprim.h"
IECORE_POP_DEFAULT_VISIBILITY

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

//////////////////////////////////////////////////////////////////////////
// Reading
//////////////////////////////////////////////////////////////////////////

namespace
{

const pxr::TfToken g_arnold( "arnold" );
const pxr::TfToken g_arnoldAlembic( "ArnoldAlembic" );
const pxr::TfToken g_arnoldNodeEntry( "arnold:node_entry" );
const pxr::TfToken g_arnoldProceduralCustom( "ArnoldProceduralCustom" );
const pxr::TfToken g_arnoldUsd( "ArnoldUsd" );

// Arnold's procedural schemas are a mishmash of parameters specific to the procedural
// and generic Arnold-specific node parameters that have no place in USD because
// they have USD equivalents already. There is no way of querying which is which, so
// we manually list the ones we need to ignore.
const std::unordered_set<std::string> g_parameterSkipList = {
	"visibility", "sidedness", "receive_shadows", "self_shadows", "invert_normals",
	"ray_bias", "matrix", "transform_type", "shader", "opaque", "matte", "use_light_group",
	"light_group", "use_shadow_group", "shadow_group", "trace_sets", "motion_start", "motion_end",
	"id", "override_nodes", "operator", "name", "node_entry"
};

IECore::ObjectPtr readArnoldProcedural( pxr::UsdGeomGprim &gprim, pxr::UsdTimeCode time, const Canceller *canceller )
{
	ExternalProceduralPtr result = new ExternalProcedural();

	// Get procedural type.

	if( gprim.GetPrim().IsA( g_arnoldAlembic ) )
	{
		// For historical reasonss, the "filename" is actually the Arnold node
		// type.
		result->setFileName( "alembic" );
	}
	else if( gprim.GetPrim().IsA( g_arnoldProceduralCustom ) )
	{
		std::string nodeEntry;
		gprim.GetPrim().GetAttribute( g_arnoldNodeEntry ).Get( &nodeEntry, time );
		result->setFileName( nodeEntry );
	}
	else if( gprim.GetPrim().IsA( g_arnoldUsd ) )
	{
		result->setFileName( "usd" );
	}

	// Get bound.

	pxr::VtVec3fArray extent;
	gprim.ComputeExtent( time, &extent );
	if( extent.size() == 2 )
	{
		result->setBound(
			Imath::Box3f( DataAlgo::fromUSD( extent[0] ), DataAlgo::fromUSD( extent[1] ) )
		);
	}

	// Get parameters.

	for( const auto &attribute : gprim.GetPrim().GetAuthoredAttributes() )
	{
		if( attribute.GetNamespace() != g_arnold )
		{
			continue;
		}

		const std::string parameterName = attribute.GetBaseName().GetString();
		if( g_parameterSkipList.count( parameterName ) )
		{
			continue;
		}

		if( auto data = DataAlgo::fromUSD( attribute, time ) )
		{
			result->parameters()->writable()[parameterName] = data;
		}
	}

	return result;
}

bool arnoldProceduralMightBeTimeVarying( pxr::UsdGeomGprim &gprim )
{
	if( gprim.GetExtentAttr().ValueMightBeTimeVarying() )
	{
		return true;
	}

	for( const auto &attribute : gprim.GetPrim().GetAuthoredAttributes() )
	{
		if( attribute.GetNamespace() != g_arnold )
		{
			continue;
		}

		const std::string parameterName = attribute.GetBaseName().GetString();
		if( g_parameterSkipList.count( parameterName ) )
		{
			continue;
		}

		if( attribute.ValueMightBeTimeVarying() )
		{
			return true;
		}
	}

	return false;
}

ObjectAlgo::ReaderDescription<pxr::UsdGeomGprim> g_arnoldAlembicReaderDescription( g_arnoldAlembic, readArnoldProcedural, arnoldProceduralMightBeTimeVarying );
ObjectAlgo::ReaderDescription<pxr::UsdGeomGprim> g_arnoldProceduralCustomReaderDescription( g_arnoldProceduralCustom, readArnoldProcedural, arnoldProceduralMightBeTimeVarying );
ObjectAlgo::ReaderDescription<pxr::UsdGeomGprim> g_arnoldUsdReaderDescription( g_arnoldUsd, readArnoldProcedural, arnoldProceduralMightBeTimeVarying );

} // namespace
