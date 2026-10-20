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

#include "IECore/SimpleTypedData.h"

#include "boost/algorithm/string/predicate.hpp"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/usd/usdGeom/points.h"
#if PXR_VERSION >= 2603
#include "pxr/usd/usdVol/particleField3DGaussianSplat.h"
#endif
IECORE_POP_DEFAULT_VISIBILITY

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

//////////////////////////////////////////////////////////////////////////
// Reading
//////////////////////////////////////////////////////////////////////////

namespace
{

IECore::ObjectPtr readPoints( pxr::UsdGeomPoints &points, pxr::UsdTimeCode time, const Canceller *canceller )
{
	IECoreScene::PointsPrimitivePtr newPoints = new IECoreScene::PointsPrimitive( points.GetPointCount( time ) );
	PrimitiveAlgo::readPrimitiveVariables( points, time, newPoints.get(), canceller );

	Canceller::check( canceller );
	PrimitiveAlgo::readPrimitiveVariable( points.GetIdsAttr(), time, newPoints.get(), "id" );

	Canceller::check( canceller );
	PrimitiveAlgo::readPrimitiveVariable(
		points.GetWidthsAttr(), time, newPoints.get(), "width", PrimitiveAlgo::fromUSD( points.GetWidthsInterpolation() )
	);

	return newPoints;
}

bool pointsMightBeTimeVarying( pxr::UsdGeomPoints &points )
{
	return
		points.GetIdsAttr().ValueMightBeTimeVarying() ||
		points.GetWidthsAttr().ValueMightBeTimeVarying() ||
		PrimitiveAlgo::primitiveVariablesMightBeTimeVarying( points )
	;
}

ObjectAlgo::ReaderDescription<pxr::UsdGeomPoints> g_pointsReaderDescription( pxr::TfToken( "Points" ), readPoints, pointsMightBeTimeVarying );

} // namespace


//////////////////////////////////////////////////////////////////////////
// Writing
//////////////////////////////////////////////////////////////////////////

namespace
{

bool writePoints( const IECoreScene::PointsPrimitive *points, const pxr::UsdStagePtr &stage, const pxr::SdfPath &path, pxr::UsdTimeCode time )
{
#if PXR_VERSION >= 2603
	auto type = points->variableData<const StringData>( "type", PrimitiveVariable::Constant );
	if( type && type->readable() == "gaussianSplat" )
	{
		auto usdSplats = pxr::UsdVolParticleField3DGaussianSplat::Define( stage, path );

		int maxCoefficientsIndex = -1;
		std::unordered_map<int, const V3fVectorData *> coefficientsMap;

		for( const auto &p : points->variables )
		{
			if( p.first == "P" )
			{
				usdSplats.CreatePositionsAttr().Set( PrimitiveAlgo::toUSDExpanded( p.second ), time );
			}
			else if( p.first == "orientations" )
			{
				usdSplats.CreateOrientationsAttr().Set( PrimitiveAlgo::toUSDExpanded( p.second ), time );
			}
			else if( p.first == "scales" )
			{
				usdSplats.CreateScalesAttr().Set( PrimitiveAlgo::toUSDExpanded( p.second ), time );
			}
			else if( p.first == "opacities" )
			{
				usdSplats.CreateOpacitiesAttr().Set( PrimitiveAlgo::toUSDExpanded( p.second ), time );
			}
			else if( p.first == "radiance:sphericalHarmonicsDegree" )
			{
				usdSplats.CreateRadianceSphericalHarmonicsDegreeAttr().Set( DataAlgo::toUSD( p.second.data.get() ), time );
			}
			else if( p.first == "projectionModeHint" )
			{
				usdSplats.CreateProjectionModeHintAttr().Set( DataAlgo::toUSD( p.second.data.get() ), time );
			}
			else if( p.first == "sortingModeHint" )
			{
				usdSplats.CreateSortingModeHintAttr().Set( DataAlgo::toUSD( p.second.data.get() ), time );
			}
			else if( boost::starts_with( p.first, "radiance:sphericalHarmonicsCoefficients:" ) )
			{
				try
				{
					const int coefficientsIndex = std::stoi( p.first.substr( 40 ) );  // 40 = length of `radiance:...` prefix

					if( p.second.data->typeId() != V3fVectorDataTypeId || p.second.interpolation != PrimitiveVariable::Interpolation::Vertex )
					{
						IECore::msg(
							IECore::Msg::Warning,
							"PointsAlgo::writePoints",
							"Invalid primitive variable \"{}\" on location \"{}\". Spherical harmonics coefficients must be of type V3fVectorData with Vertex interpolation.",
							p.first, path.GetAsString()
						);
						continue;
					}

					coefficientsMap[coefficientsIndex] = static_cast<const V3fVectorData *>( p.second.data.get() );
					maxCoefficientsIndex = std::max( maxCoefficientsIndex, coefficientsIndex );
				}
				catch( ... )
				{
					PrimitiveAlgo::writePrimitiveVariable( p.first, p.second, usdSplats, time );
				}
			}
			else if(  p.first == "type" )
			{
				// This is intrinsic to `UsdVolParticleField3DGaussianSplat`
				continue;
			}
			else
			{
				PrimitiveAlgo::writePrimitiveVariable( p.first, p.second, usdSplats, time );
			}
		}

		if( !coefficientsMap.empty() )
		{
			const int coefficientsCount = maxCoefficientsIndex + 1;
			pxr::VtArray<pxr::GfVec3f> allCoefficients( coefficientsCount * points->getNumPoints() );

			for( const auto &[i, coefficientsData] : coefficientsMap )
			{
				const std::vector<Imath::V3f> &coefficients = coefficientsData->readable();

				for( size_t j = 0, eJ = points->getNumPoints(); j < eJ; ++j )
				{
					allCoefficients[j * coefficientsCount + i] = DataAlgo::toUSD( coefficients[j] );
				}
			}

			usdSplats.CreateRadianceSphericalHarmonicsCoefficientsAttr().Set( allCoefficients, time );
		}

		return true;
	}

#endif

	auto usdPoints = pxr::UsdGeomPoints::Define( stage, path );
	for( const auto &p : points->variables )
	{
		if( p.first == "id" )
		{
			usdPoints.CreateIdsAttr().Set( DataAlgo::toUSD( p.second.data.get() ), time );
		}
		else if( p.first == "width" )
		{
			auto widthsAttr = usdPoints.CreateWidthsAttr();
			widthsAttr.Set( PrimitiveAlgo::toUSDExpanded( p.second, /* arrayRequired = */ true ) );
			usdPoints.SetWidthsInterpolation( PrimitiveAlgo::toUSD( p.second.interpolation ) );
		}
		else
		{
			PrimitiveAlgo::writePrimitiveVariable( p.first, p.second, usdPoints, time );
		}
	}

	return true;
}

ObjectAlgo::WriterDescription<PointsPrimitive> g_pointsWriterDescription( writePoints );

} // namespace
