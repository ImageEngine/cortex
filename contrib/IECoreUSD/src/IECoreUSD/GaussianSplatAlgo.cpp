//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2026, Cinesite VFX Ltd. All rights reserved.
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

#if PXR_VERSION >= 2603

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/usd/usdVol/particleField3DGaussianSplat.h"
IECORE_POP_DEFAULT_VISIBILITY

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

//////////////////////////////////////////////////////////////////////////
// Reading
//////////////////////////////////////////////////////////////////////////

namespace
{

IECore::ObjectPtr readGaussianSplat( pxr::UsdVolParticleField3DGaussianSplat &splats, pxr::UsdTimeCode time, const Canceller *canceller )
{
	pxr::UsdAttribute positionAttr;
	splats.UsesFloatPositions( &positionAttr );
	auto positionData = runTimeCast<V3fVectorData>( DataAlgo::fromUSD( positionAttr, time ) );

	if( !positionData )
	{
		IECore::msg(
			IECore::Msg::Warning,
			"GaussianSplatAlgo::readGaussianSplat",
			"Position primitive variable not found at location \"{}\". Accepted USD primitive variable names are \"positions\" and \"positionsh\".",
			splats.GetPrim().GetPath().GetAsString()
		);
		return nullptr;
	}

	IECoreScene::PointsPrimitivePtr newPoints = new IECoreScene::PointsPrimitive( positionData );

	newPoints->variables["type"] = PrimitiveVariable( PrimitiveVariable::Interpolation::Constant, new StringData( "gaussianSplat" ) );
	PrimitiveAlgo::readPrimitiveVariable( splats.GetProjectionModeHintAttr(), time, newPoints.get(), "projectionModeHint", PrimitiveVariable::Interpolation::Constant );
	PrimitiveAlgo::readPrimitiveVariable( splats.GetSortingModeHintAttr(), time, newPoints.get(), "sortingModeHint", PrimitiveVariable::Interpolation::Constant );

	Canceller::check( canceller );
	PrimitiveAlgo::readPrimitiveVariables( pxr::UsdGeomPrimvarsAPI( splats.GetPrim() ), time, newPoints.get(), canceller );

	Canceller::check( canceller );
	pxr::UsdAttribute orientationsAttr;
	splats.UsesFloatOrientations( &orientationsAttr );
	PrimitiveAlgo::readPrimitiveVariable( orientationsAttr, time, newPoints.get(), "orientations" );

	Canceller::check( canceller );
	pxr::UsdAttribute scalesAttr;
	splats.UsesFloatScales( &scalesAttr );
	PrimitiveAlgo::readPrimitiveVariable( scalesAttr, time, newPoints.get(), "scales" );

	Canceller::check( canceller );
	pxr::UsdAttribute opacitiesAttr;
	splats.UsesFloatOpacities( &opacitiesAttr );
	PrimitiveAlgo::readPrimitiveVariable( opacitiesAttr, time, newPoints.get(), "opacities" );

	Canceller::check( canceller );
	pxr::UsdAttribute degreeAttr = splats.GetRadianceSphericalHarmonicsDegreeAttr();
	auto degreeData = boost::static_pointer_cast<IntData>( DataAlgo::fromUSD( degreeAttr, time, false ) );
	newPoints->variables["radiance:sphericalHarmonicsDegree"] = PrimitiveVariable( PrimitiveVariable::Interpolation::Constant, degreeData );

	const int degree = degreeData->readable();
	const int coefficientCount = ( degree + 1 ) * ( degree + 1 );

	pxr::UsdAttribute coefficientsAttr;
	splats.UsesFloatRadianceCoefficients( &coefficientsAttr );
	auto allCoefficientsData = runTimeCast<V3fVectorData>( DataAlgo::fromUSD( coefficientsAttr, time ) );

	if( allCoefficientsData && allCoefficientsData->readable().size() >= coefficientCount * newPoints->getNumPoints() )
	{
		const std::vector<Imath::V3f> &allCoefficients = allCoefficientsData->readable();

		for( int i = 0; i < coefficientCount; ++i )
		{
			Canceller::check( canceller );

			V3fVectorDataPtr coefficientsData = new V3fVectorData();
			std::vector<Imath::V3f> &coefficients = coefficientsData->writable();
			coefficients.resize( newPoints->getNumPoints() );
			for( size_t j = 0, eJ = newPoints->getNumPoints(); j < eJ; ++j )
			{
				coefficients[j] = allCoefficients[j * coefficientCount + i];
			}

			newPoints->variables[fmt::format( "radiance:sphericalHarmonicsCoefficients:{}", i )] = PrimitiveVariable( PrimitiveVariable::Interpolation::Vertex, coefficientsData );
		}
	}

	return newPoints;
}

bool splatMightBeTimeVarying( pxr::UsdVolParticleField3DGaussianSplat &splats )
{
	return
		splats.GetPositionsAttr().ValueMightBeTimeVarying() ||
		splats.GetPositionshAttr().ValueMightBeTimeVarying() ||
		splats.GetOrientationsAttr().ValueMightBeTimeVarying() ||
		splats.GetOrientationshAttr().ValueMightBeTimeVarying() ||
		splats.GetScalesAttr().ValueMightBeTimeVarying() ||
		splats.GetScaleshAttr().ValueMightBeTimeVarying() ||
		splats.GetOpacitiesAttr().ValueMightBeTimeVarying() ||
		splats.GetOpacitieshAttr().ValueMightBeTimeVarying() ||
		splats.GetRadianceSphericalHarmonicsCoefficientsAttr().ValueMightBeTimeVarying() ||
		splats.GetRadianceSphericalHarmonicsCoefficientshAttr().ValueMightBeTimeVarying() ||
		PrimitiveAlgo::primitiveVariablesMightBeTimeVarying( pxr::UsdGeomPrimvarsAPI( splats ) )
	;
}

ObjectAlgo::ReaderDescription<pxr::UsdVolParticleField3DGaussianSplat> g_splatReaderDescription( pxr::TfToken( "ParticleField3DGaussianSplat" ), readGaussianSplat, splatMightBeTimeVarying );

} // namespace

#endif
