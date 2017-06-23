//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MessageHandler.h"

#include "IECoreAlembic/ObjectAlgo.h"
#include "IECoreAlembic/CurvesAlgo.h"
#include "IECoreAlembic/GeomBaseAlgo.h"

using namespace IECore;
using namespace IECoreAlembic;
using namespace Alembic::AbcGeom;

namespace IECoreAlembic
{

namespace CurvesAlgo
{

IECOREALEMBIC_API IECore::CurvesPrimitivePtr convert( const Alembic::AbcGeom::ICurves &curves, const Alembic::Abc::ISampleSelector &sampleSelector )
{
	const ICurvesSchema curvesSchema = curves.getSchema();
	const ICurvesSchema::Sample sample = curvesSchema.getValue( sampleSelector );

	IntVectorDataPtr vertsPerCurve = new IntVectorData;
	vertsPerCurve->writable().insert(
		vertsPerCurve->writable().end(),
		sample.getCurvesNumVertices()->get(),
		sample.getCurvesNumVertices()->get() + sample.getCurvesNumVertices()->size()
	);

	CubicBasisf basis = CubicBasisf::linear();
	switch( sample.getType() )
	{
		case kLinear :
			basis = CubicBasisf::linear();
			break;
		case kCubic :
			switch( sample.getBasis() )
			{
				case kNoBasis :
					basis = CubicBasisf::linear();
					break;
				case kBezierBasis :
					basis = CubicBasisf::bezier();
					break;
				case kCatmullromBasis :
					basis = CubicBasisf::catmullRom();
					break;
				case kBsplineBasis :
					basis = CubicBasisf::bSpline();
					break;
				case kHermiteBasis :
				case kPowerBasis :
					IECore::msg( IECore::Msg::Warning, "CurvesAlgo::convert", "Unsupported basis" );
					basis = CubicBasisf::bSpline();
					break;
			}
			break;
		default :
			basis = CubicBasisf::bSpline();
			break;
	}

	V3fVectorDataPtr points = new V3fVectorData();
	points->writable().resize( sample.getPositions()->size() );
	memcpy( &(points->writable()[0]), sample.getPositions()->get(), sample.getPositions()->size() * sizeof( Imath::V3f ) );

	CurvesPrimitivePtr result = new CurvesPrimitive(
		vertsPerCurve,
		basis,
		sample.getWrap() == kPeriodic,
		points
	);

	ICompoundProperty arbGeomParams = curvesSchema.getArbGeomParams();
	GeomBaseAlgo::convertArbGeomParams( arbGeomParams, sampleSelector, result.get() );

	return result;
}

} // namespace CurvesAlgo

} // namespace IECoreAlembic

static ObjectAlgo::ConverterDescription<ICurves, CurvesPrimitive> g_Description( &IECoreAlembic::CurvesAlgo::convert );

