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

#include "IECoreUSD/DataAlgo.h"
#include "IECoreUSD/ObjectAlgo.h"
#include "IECoreUSD/PrimitiveAlgo.h"

#include "IECoreScene/PointsPrimitive.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/usd/usdGeom/pointInstancer.h"
IECORE_POP_DEFAULT_VISIBILITY

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

//////////////////////////////////////////////////////////////////////////
// Reading
//////////////////////////////////////////////////////////////////////////

namespace
{

bool checkEnvFlag( const char *envVar, bool def )
{
	const char *value = getenv( envVar );
	if( value )
	{
		return std::string( value ) != "0";
	}
	else
	{
		return def;
	}
}

IECore::ObjectPtr readPointInstancer( pxr::UsdGeomPointInstancer &pointInstancer, pxr::UsdTimeCode time, const Canceller *canceller )
{
	pxr::VtVec3fArray pointsData;
	pointInstancer.GetPositionsAttr().Get( &pointsData, time );
	Canceller::check( canceller );
	IECore::V3fVectorDataPtr positionData = DataAlgo::fromUSD( pointsData );

	positionData->setInterpretation( GeometricData::Point );
	IECoreScene::PointsPrimitivePtr newPoints = new IECoreScene::PointsPrimitive( positionData );

	// Per point attributes

	Canceller::check( canceller );
	PrimitiveAlgo::readPrimitiveVariable( pointInstancer.GetProtoIndicesAttr(), time, newPoints.get(), "prototypeIndex" );

	Canceller::check( canceller );
	PrimitiveAlgo::readPrimitiveVariable( pointInstancer.GetIdsAttr(), time, newPoints.get(), "instanceId" );

	Canceller::check( canceller );
	PrimitiveAlgo::readPrimitiveVariable( pointInstancer.GetOrientationsAttr(), time, newPoints.get(), "orientation" );

	Canceller::check( canceller );
	PrimitiveAlgo::readPrimitiveVariable( pointInstancer.GetScalesAttr(), time, newPoints.get(), "scale" );

	Canceller::check( canceller );
	PrimitiveAlgo::readPrimitiveVariable( pointInstancer.GetVelocitiesAttr(), time, newPoints.get(), "velocity" );

	Canceller::check( canceller );
	PrimitiveAlgo::readPrimitiveVariable( pointInstancer.GetAccelerationsAttr(), time, newPoints.get(), "acceleration" );

	Canceller::check( canceller );
	PrimitiveAlgo::readPrimitiveVariable( pointInstancer.GetAngularVelocitiesAttr(), time, newPoints.get(), "angularVelocity" );

	if( pointInstancer.GetInvisibleIdsAttr().HasAuthoredValue() )
	{
		DataPtr cortexInvisIds = DataAlgo::fromUSD( pointInstancer.GetInvisibleIdsAttr(), time, true );
		if( cortexInvisIds )
		{
			newPoints->variables["invisibleIds"] = IECoreScene::PrimitiveVariable(
				PrimitiveVariable::Constant, cortexInvisIds
			);
		}
	}

	pxr::SdfInt64ListOp inactiveIdsListOp;
	if( pointInstancer.GetPrim().GetMetadata( pxr::UsdGeomTokens->inactiveIds, &inactiveIdsListOp ) )
	{
		newPoints->variables["inactiveIds"] = IECoreScene::PrimitiveVariable(
			PrimitiveVariable::Constant,
			new IECore::Int64VectorData( inactiveIdsListOp.GetExplicitItems() )
		);
	}

	// Prototype paths

	const static bool g_relativePrototypes = checkEnvFlag( "IECOREUSD_POINTINSTANCER_RELATIVE_PROTOTYPES", false );

	pxr::SdfPathVector targets;
	Canceller::check( canceller );
	pointInstancer.GetPrototypesRel().GetForwardedTargets( &targets );

	const pxr::SdfPath &primPath = pointInstancer.GetPath();

	IECore::StringVectorDataPtr prototypeRootsData = new IECore::StringVectorData();
	auto &prototypeRoots = prototypeRootsData->writable();
	prototypeRoots.reserve( targets.size() );
	for( const auto &t : targets )
	{
		if( !g_relativePrototypes || !t.HasPrefix( primPath ) )
		{
			prototypeRoots.push_back( t.GetString() );
		}
		else
		{
			// The ./ prefix shouldn't be necessary - we want to just use the absence of a leading
			// slash to indicate relative paths. We can remove the prefix here once we deprecate the
			// GAFFERSCENE_INSTANCER_EXPLICIT_ABSOLUTE_PATHS env var and have Gaffer always require a leading
			// slash for absolute paths.
			prototypeRoots.push_back( "./" + t.MakeRelativePath( primPath ).GetString() );
		}
	}

	newPoints->variables["prototypeRoots"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Constant, prototypeRootsData );

	// Primitive variables

	PrimitiveAlgo::readPrimitiveVariables( pxr::UsdGeomPrimvarsAPI( pointInstancer ), time, newPoints.get(), canceller );

	return newPoints;
}

bool pointInstancerMightBeTimeVarying( pxr::UsdGeomPointInstancer &instancer )
{
	return
		instancer.GetPositionsAttr().ValueMightBeTimeVarying() ||
		instancer.GetProtoIndicesAttr().ValueMightBeTimeVarying() ||
		instancer.GetIdsAttr().ValueMightBeTimeVarying() ||
		instancer.GetOrientationsAttr().ValueMightBeTimeVarying() ||
		instancer.GetScalesAttr().ValueMightBeTimeVarying() ||
		instancer.GetVelocitiesAttr().ValueMightBeTimeVarying() ||
		instancer.GetAccelerationsAttr().ValueMightBeTimeVarying() ||
		instancer.GetAngularVelocitiesAttr().ValueMightBeTimeVarying() ||
		instancer.GetInvisibleIdsAttr().ValueMightBeTimeVarying() ||
		PrimitiveAlgo::primitiveVariablesMightBeTimeVarying(
			pxr::UsdGeomPrimvarsAPI( instancer )
		)
	;
}

ObjectAlgo::ReaderDescription<pxr::UsdGeomPointInstancer> g_pointInstancerReaderDescription( pxr::TfToken( "PointInstancer" ), readPointInstancer, pointInstancerMightBeTimeVarying );

} // namespace
