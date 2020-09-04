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

IECore::ObjectPtr readPointInstancer( pxr::UsdGeomPointInstancer &pointInstancer, pxr::UsdTimeCode time )
{
	pxr::VtVec3fArray pointsData;
	pointInstancer.GetPositionsAttr().Get( &pointsData, time );
	IECore::V3fVectorDataPtr positionData = DataAlgo::fromUSD( pointsData );
	positionData->setInterpretation( GeometricData::Point );
	IECoreScene::PointsPrimitivePtr newPoints = new IECoreScene::PointsPrimitive( positionData );

	// Per point attributes

	if( auto protoIndicesData = DataAlgo::fromUSD( pointInstancer.GetProtoIndicesAttr(), time ) )
	{
		newPoints->variables["prototypeIndex"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, protoIndicesData );
	}

	if( auto idsData = DataAlgo::fromUSD( pointInstancer.GetIdsAttr(), time ) )
	{
		newPoints->variables["instanceId"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, idsData );
	}

	if( auto orientationData = DataAlgo::fromUSD( pointInstancer.GetOrientationsAttr(), time ) )
	{
		newPoints->variables["orientation"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, orientationData );
	}

	if( auto scaleData = DataAlgo::fromUSD( pointInstancer.GetScalesAttr(), time ) )
	{
		newPoints->variables["scale"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, scaleData );
	}

	if( auto velocityData = DataAlgo::fromUSD( pointInstancer.GetVelocitiesAttr(), time ) )
	{
		newPoints->variables["velocity"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, velocityData );
	}

#if USD_VERSION >= 1911
	if( auto accelerationData = DataAlgo::fromUSD( pointInstancer.GetAccelerationsAttr(), time ) )
	{
		newPoints->variables["acceleration"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, accelerationData );
	}
#endif

	if( auto angularVelocityData = DataAlgo::fromUSD( pointInstancer.GetAngularVelocitiesAttr(), time ) )
	{
		newPoints->variables["angularVelocity"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, angularVelocityData );
	}

	// Prototype paths

	pxr::SdfPathVector targets;
	pointInstancer.GetPrototypesRel().GetTargets( &targets );

	IECore::StringVectorDataPtr prototypeRootsData = new IECore::StringVectorData();
	auto &prototypeRoots = prototypeRootsData->writable();
	prototypeRoots.reserve( targets.size() );
	for( const auto &t : targets )
	{
		prototypeRoots.push_back( t.GetString() );
	}

	newPoints->variables["prototypeRoots"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Constant, prototypeRootsData );

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
#if USD_VERSION >= 1911
		instancer.GetAccelerationsAttr().ValueMightBeTimeVarying() ||
#endif
		instancer.GetAngularVelocitiesAttr().ValueMightBeTimeVarying()
	;
}

ObjectAlgo::ReaderDescription<pxr::UsdGeomPointInstancer> g_curvesReaderDescription( pxr::TfToken( "PointInstancer" ), readPointInstancer, pointInstancerMightBeTimeVarying );

} // namespace