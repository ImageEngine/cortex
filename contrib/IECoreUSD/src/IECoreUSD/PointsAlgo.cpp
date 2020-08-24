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

#include "DataAlgo.h"
#include "ObjectAlgo.h"
#include "PrimitiveAlgo.h"

#include "IECoreScene/PointsPrimitive.h"

#include "IECore/SimpleTypedData.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/usd/usdGeom/points.h"
IECORE_POP_DEFAULT_VISIBILITY

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

//////////////////////////////////////////////////////////////////////////
// Reading
//////////////////////////////////////////////////////////////////////////

namespace
{

IECore::ObjectPtr readPoints( pxr::UsdGeomPoints &points, pxr::UsdTimeCode time )
{
	IECoreScene::PointsPrimitivePtr newPoints = new IECoreScene::PointsPrimitive();
	PrimitiveAlgo::readPrimitiveVariables( points, time, newPoints.get() );
	if( auto *p = newPoints->variableData<V3fVectorData>( "P" ) )
	{
		newPoints->setNumPoints( p->readable().size() );
	}

	if( auto i = boost::static_pointer_cast<Int64VectorData>( DataAlgo::fromUSD( points.GetIdsAttr(), time ) ) )
	{
		newPoints->variables["id"] = IECoreScene::PrimitiveVariable( IECoreScene::PrimitiveVariable::Vertex, i );
	}

	if( auto w = boost::static_pointer_cast<FloatVectorData>( DataAlgo::fromUSD( points.GetWidthsAttr(), time ) ) )
	{
		IECoreScene::PrimitiveVariable pv( PrimitiveAlgo::fromUSD( points.GetWidthsInterpolation() ), w );
		if( pv.interpolation == PrimitiveVariable::Constant && w->readable().size() == 1 )
		{
			// USD uses arrays even for constant data, but we use single values.
			pv.data = new FloatData( w->readable()[0] );
		}
		newPoints->variables["width"] = pv;
	}

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

ObjectAlgo::ReaderDescription<pxr::UsdGeomPoints> g_curvesReaderDescription( pxr::TfToken( "Points" ), readPoints, pointsMightBeTimeVarying );

} // namespace


//////////////////////////////////////////////////////////////////////////
// Writing
//////////////////////////////////////////////////////////////////////////

namespace
{

void writePoints( const IECoreScene::PointsPrimitive *points, const pxr::UsdStagePtr &stage, const pxr::SdfPath &path, pxr::UsdTimeCode time )
{
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
			auto floatData = runTimeCast<const FloatData>( p.second.data.get() );
			if( p.second.interpolation == PrimitiveVariable::Constant && floatData )
			{
				// USD requires an array even for constant data.
				widthsAttr.Set( pxr::VtArray<float>( 1, floatData->readable() ), time );
			}
			else
			{
				widthsAttr.Set( PrimitiveAlgo::toUSDExpanded( p.second ), time );
			}
			usdPoints.SetWidthsInterpolation( PrimitiveAlgo::toUSD( p.second.interpolation ) );
		}
		else
		{
			PrimitiveAlgo::writePrimitiveVariable( p.first, p.second, usdPoints, time );
		}
	}
}

ObjectAlgo::WriterDescription<PointsPrimitive> g_pointsWriterDescription( writePoints );

} // namespace