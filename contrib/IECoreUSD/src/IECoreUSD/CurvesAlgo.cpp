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

#include "IECoreScene/CurvesPrimitive.h"

#include "IECore/SimpleTypedData.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/usd/usdGeom/basisCurves.h"
#include "pxr/usd/usdGeom/nurbsCurves.h"
IECORE_POP_DEFAULT_VISIBILITY

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

//////////////////////////////////////////////////////////////////////////
// Reading
//////////////////////////////////////////////////////////////////////////

namespace
{

IECore::ObjectPtr readCurves( pxr::UsdGeomCurves &curves, pxr::UsdTimeCode time, const IECore::CubicBasisf &basis, bool periodic, const Canceller *canceller )
{
	Canceller::check( canceller );
	pxr::VtIntArray vertexCountsArray;
	curves.GetCurveVertexCountsAttr().Get( &vertexCountsArray, time );
	IECore::IntVectorDataPtr countData = DataAlgo::fromUSD( vertexCountsArray );

	Canceller::check( canceller );
	IECoreScene::CurvesPrimitivePtr newCurves = new IECoreScene::CurvesPrimitive( countData, basis, periodic );
	PrimitiveAlgo::readPrimitiveVariables( curves, time, newCurves.get(), canceller );

	Canceller::check( canceller );
	PrimitiveAlgo::readPrimitiveVariable(
		curves.GetWidthsAttr(), time, newCurves.get(), "width", PrimitiveAlgo::fromUSD( curves.GetWidthsInterpolation() )
	);

	return newCurves;
}

bool curvesMightBeTimeVarying( pxr::UsdGeomCurves &curves )
{
	return
		curves.GetCurveVertexCountsAttr().ValueMightBeTimeVarying() ||
		curves.GetWidthsAttr().ValueMightBeTimeVarying() ||
		PrimitiveAlgo::primitiveVariablesMightBeTimeVarying( curves )
	;
}

IECore::ObjectPtr readBasisCurves( pxr::UsdGeomBasisCurves &curves, pxr::UsdTimeCode time, const Canceller *canceller )
{

	// Basis
	Canceller::check( canceller );
	IECore::CubicBasisf basis = CubicBasisf::linear();
	pxr::TfToken type;
	curves.GetTypeAttr().Get( &type, time );
	if( type == pxr::UsdGeomTokens->cubic )
	{
		pxr::TfToken usdBasis;
		curves.GetBasisAttr().Get( &usdBasis, time );
		if( usdBasis == pxr::UsdGeomTokens->bezier )
		{
			basis = CubicBasisf::bezier();
		}
		else if( usdBasis == pxr::UsdGeomTokens->bspline )
		{
			basis = CubicBasisf::bSpline();
		}
		else if( usdBasis == pxr::UsdGeomTokens->catmullRom )
		{
			basis = CubicBasisf::catmullRom();
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "USDScene", "Unsupported basis \"{}\"", usdBasis.GetString() );
		}
	}

	// Wrap

	bool periodic = false;
	pxr::TfToken wrap;
	curves.GetWrapAttr().Get( &wrap, time );
	if( wrap == pxr::UsdGeomTokens->periodic )
	{
		periodic = true;
	}
	else if( wrap != pxr::UsdGeomTokens->nonperiodic )
	{
		IECore::msg( IECore::Msg::Warning, "USDScene", "Unsupported wrap \"{}\"", wrap.GetString() );
	}

	return readCurves( curves, time, basis, periodic, canceller );
}

bool basisCurvesMightBeTimeVarying( pxr::UsdGeomBasisCurves &curves )
{
	return
		curvesMightBeTimeVarying( curves ) ||
		curves.GetTypeAttr().ValueMightBeTimeVarying() ||
		curves.GetBasisAttr().ValueMightBeTimeVarying() ||
		curves.GetWrapAttr().ValueMightBeTimeVarying()
	;
}

IECore::ObjectPtr readNurbsCurves( pxr::UsdGeomNurbsCurves &curves, pxr::UsdTimeCode time, const Canceller *canceller )
{
	IECore::CubicBasisf basis = IECore::CubicBasisf::linear();

	Canceller::check( canceller );
	pxr::VtIntArray order;
	curves.GetOrderAttr().Get( &order, time );
	if( std::all_of( order.begin(), order.end(), [] ( int o ) { return o == 4; } ) )
	{
		basis = CubicBasisf::bSpline();
	}

	return readCurves( curves, time, basis, false, canceller );
}

bool nurbsCurvesMightBeTimeVarying( pxr::UsdGeomNurbsCurves &curves )
{
	return
		curvesMightBeTimeVarying( curves ) ||
		curves.GetOrderAttr().ValueMightBeTimeVarying()
	;
}

ObjectAlgo::ReaderDescription<pxr::UsdGeomBasisCurves> g_curvesReaderDescription( pxr::TfToken( "BasisCurves" ), readBasisCurves, basisCurvesMightBeTimeVarying );
ObjectAlgo::ReaderDescription<pxr::UsdGeomNurbsCurves> g_nurbsCurvesReaderDescription( pxr::TfToken( "NurbsCurves" ), readNurbsCurves, nurbsCurvesMightBeTimeVarying );

} // namespace


//////////////////////////////////////////////////////////////////////////
// Writing
//////////////////////////////////////////////////////////////////////////

namespace
{

bool writeCurves( const IECoreScene::CurvesPrimitive *curves, const pxr::UsdStagePtr &stage, const pxr::SdfPath &path, pxr::UsdTimeCode time )
{
	auto usdCurves = pxr::UsdGeomBasisCurves::Define( stage, path );

	// Topology, wrap, basis

	usdCurves.CreateCurveVertexCountsAttr().Set( DataAlgo::toUSD( curves->verticesPerCurve() ), time );

	usdCurves.CreateWrapAttr().Set(
		curves->periodic() ? pxr::UsdGeomTokens->periodic : pxr::UsdGeomTokens->nonperiodic,
		time
	);

	pxr::TfToken basis;
	if( curves->basis() == CubicBasisf::bezier() )
	{
		basis = pxr::UsdGeomTokens->bezier;
	}
	else if( curves->basis() == CubicBasisf::bSpline() )
	{
		basis = pxr::UsdGeomTokens->bspline;
	}
	else if( curves->basis() == CubicBasisf::catmullRom() )
	{
		basis = pxr::UsdGeomTokens->catmullRom;
	}
	else if ( curves->basis() != CubicBasisf::linear() )
	{
		IECore::msg( IECore::Msg::Warning, "USDScene", "Unsupported basis" );
	}

	if( !basis.IsEmpty() )
	{
		usdCurves.CreateTypeAttr().Set( pxr::UsdGeomTokens->cubic, time );
		usdCurves.CreateBasisAttr().Set( basis, time );
	}
	else
	{
		usdCurves.CreateTypeAttr().Set( pxr::UsdGeomTokens->linear, time );
	}

	// Primvars

	for( const auto &p : curves->variables )
	{
		if( p.first == "width" )
		{
			auto widthsAttr = usdCurves.CreateWidthsAttr();
			widthsAttr.Set( PrimitiveAlgo::toUSDExpanded( p.second, /* arrayRequired = */ true ) );
			usdCurves.SetWidthsInterpolation( PrimitiveAlgo::toUSD( p.second.interpolation ) );
		}
		else
		{
			PrimitiveAlgo::writePrimitiveVariable( p.first, p.second, usdCurves, time );
		}
	}

	return true;
}

ObjectAlgo::WriterDescription<CurvesPrimitive> g_curvesWriterDescription( writeCurves );

} // namespace
