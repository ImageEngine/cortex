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

#include "IECoreScene/PointInstancer.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/usd/usdGeom/pointInstancer.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "boost/algorithm/string/predicate.hpp"
#include "boost/container/flat_set.hpp"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

//////////////////////////////////////////////////////////////////////////
// Reading
//////////////////////////////////////////////////////////////////////////

namespace
{

IECore::ObjectPtr readPointInstancer( pxr::UsdGeomPointInstancer &pointInstancer, pxr::UsdTimeCode time, const Canceller *canceller )
{
	pxr::VtVec3fArray pointsData;
	pointInstancer.GetPositionsAttr().Get( &pointsData, time );
	Canceller::check( canceller );
	IECore::V3fVectorDataPtr positionData = DataAlgo::fromUSD( pointsData );

	IECoreScene::PointInstancerPtr newPoints = new IECoreScene::PointInstancer( positionData->readable().size() );
	newPoints->setPosition( positionData );

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

	// Inactive and invisible IDs. These both do the same thing - prevent specific instances
	// from rendering. Inactive IDs are metadata-based and therefore can't be animated.
	// Invisible IDs are attribute-based and therefore can be animated. Since we're converting
	// to Cortex PrimitiveVariables, the distinction is irrelevant - all PrimitiveVariables can
	// be animated. Our PointInstancer therefore just has invisible IDs, which we merge both
	// USD properties into.

	IECore::Int64VectorDataPtr invisibleIds;
	if( pointInstancer.GetInvisibleIdsAttr().HasAuthoredValue() )
	{
		invisibleIds = IECore::runTimeCast<IECore::Int64VectorData>(
			DataAlgo::fromUSD( pointInstancer.GetInvisibleIdsAttr(), time, true )
		);
	}

	pxr::SdfInt64ListOp inactiveIdsListOp;
	if( pointInstancer.GetPrim().GetMetadata( pxr::UsdGeomTokens->inactiveIds, &inactiveIdsListOp ) )
	{
		const std::vector<int64_t> &inactiveIds = inactiveIdsListOp.GetExplicitItems();
		if( inactiveIds.size() )
		{
			if( invisibleIds )
			{
				invisibleIds->writable().insert(
					invisibleIds->writable().end(),
					inactiveIds.begin(), inactiveIds.end()
				);
				std::sort( invisibleIds->writable().begin(), invisibleIds->writable().end() );
				invisibleIds->writable().erase(
					std::unique( invisibleIds->writable().begin(), invisibleIds->writable().end() ),
					invisibleIds->writable().end()
				);
			}
			else
			{
				invisibleIds = new Int64VectorData( inactiveIds );
			}
		}
	}

	newPoints->setInvisibleIDs( invisibleIds );

	// Prototype paths

	pxr::SdfPathVector targets;
	Canceller::check( canceller );
	pointInstancer.GetPrototypesRel().GetForwardedTargets( &targets );

	const pxr::SdfPath &primPath = pointInstancer.GetPath();

	IECore::StringVectorDataPtr prototypeRootsData = new IECore::StringVectorData();
	auto &prototypeRoots = prototypeRootsData->writable();
	prototypeRoots.reserve( targets.size() );
	for( const auto &t : targets )
	{
		if( !t.HasPrefix( primPath ) )
		{
			prototypeRoots.push_back( t.GetString() );
		}
		else
		{
			prototypeRoots.push_back( t.MakeRelativePath( primPath ).GetString() );
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

//////////////////////////////////////////////////////////////////////////
// Writing
//////////////////////////////////////////////////////////////////////////

namespace
{

const boost::container::flat_set<std::string> g_exportedAsAttributes = {
	"prototypeRoots", "prototypeIndex", "P", "scale", "orientation",
	"id", "invisibleIds"
};

bool writePointInstancer( const IECoreScene::PointInstancer *instancer, const pxr::UsdStagePtr &stage, const pxr::SdfPath &path, pxr::UsdTimeCode time )
{
	auto usdInstancer = pxr::UsdGeomPointInstancer::Define( stage, path );

	// Export primitive variables with special meaning to attributes
	// of the UsdGeomPointInstancer.

	if( auto prototypes = instancer->getPrototypes() )
	{
		pxr::SdfPathVector targets;
		targets.reserve( prototypes.size() );
		for( const auto &prototype : prototypes )
		{
			pxr::SdfPath prototypePath;
			if( boost::starts_with( prototype, "./" ) )
			{
				prototypePath = pxr::SdfPath( prototype.substr( 2 ) );
			}
			else
			{
				prototypePath = pxr::SdfPath( prototype );
			}
			if( !prototypePath.IsAbsolutePath() )
			{
				prototypePath = prototypePath.MakeAbsolutePath( path );
			}
			targets.push_back( prototypePath );
		}
		usdInstancer.CreatePrototypesRel().SetTargets( targets );
	}

	if( auto prototypeIndex = instancer->getPrototypeIndex() )
	{
		usdInstancer.CreateProtoIndicesAttr().Set( PrimitiveAlgo::toUSDExpanded( prototypeIndex ), time );
	}

	if( auto position = instancer->getPosition() )
	{
		usdInstancer.CreatePositionsAttr().Set( PrimitiveAlgo::toUSDExpanded( position ), time );
	}

	if( auto orientation = instancer->getOrientation() )
	{
		// USD uses `half` for orientation, but Cortex only has a data type for
		// `float` quaternions, so convert.
		pxr::VtArray<pxr::GfQuath> usdOrientation;
		usdOrientation.reserve( orientation.size() );
		for( const auto &o : orientation )
		{
			usdOrientation.push_back( pxr::GfQuath( DataAlgo::toUSD( o ) ) );
		}
		usdInstancer.CreateOrientationsAttr().Set( usdOrientation, time );
	}

	if( auto scale = instancer->getScale() )
	{
		usdInstancer.CreateScalesAttr().Set( PrimitiveAlgo::toUSDExpanded( scale ), time );
	}

	if( auto id = instancer->getID() )
	{
		usdInstancer.CreateIdsAttr().Set( PrimitiveAlgo::toUSDExpanded( id ), time );
	}

	if( auto invisibleIds = instancer->getInvisibleIDs() )
	{
		usdInstancer.CreateInvisibleIdsAttr().Set( PrimitiveAlgo::toUSDExpanded( invisibleIds ), time );
	}

	for( const auto &[name, primitiveVariable] : instancer->variables )
	{
		if( g_exportedAsAttributes.count( name ) )
		{
			continue;
		}
		PrimitiveAlgo::writePrimitiveVariable( name, primitiveVariable, pxr::UsdGeomPrimvarsAPI( usdInstancer ), time );
	}

	return true;
}

ObjectAlgo::WriterDescription<PointInstancer> g_pointInstancerWriterDescription( writePointInstancer );

} // namespace
