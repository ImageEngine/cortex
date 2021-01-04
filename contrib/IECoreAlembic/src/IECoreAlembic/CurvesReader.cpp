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

#include "IECoreAlembic/PrimitiveReader.h"

#include "IECoreScene/CurvesPrimitive.h"

#include "IECore/MessageHandler.h"

#include "Alembic/AbcGeom/ICurves.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreAlembic;
using namespace Alembic::AbcGeom;

namespace
{

CubicBasisf convertBasis( const ICurvesSchema::Sample &sample )
{
	switch( sample.getType() )
	{
		case kLinear :
			return CubicBasisf::linear();
		case kCubic :
			switch( sample.getBasis() )
			{
				case kNoBasis :
					return CubicBasisf::linear();
				case kBezierBasis :
					return CubicBasisf::bezier();
				case kCatmullromBasis :
					return CubicBasisf::catmullRom();
				case kBsplineBasis :
					return CubicBasisf::bSpline();
				case kHermiteBasis :
				case kPowerBasis :
					IECore::msg( IECore::Msg::Warning, "CurvesAlgo::convert", "Unsupported basis" );
					return CubicBasisf::bSpline();
			}
		default :
			return CubicBasisf::bSpline();
	}
}

class CurvesReader : public PrimitiveReader
{

	public :

		CurvesReader( const ICurves &curves )
			:	m_curves( curves )
		{
		}

		const Alembic::Abc::IObject &object() const override
		{
			return m_curves;
		}

		Alembic::Abc::IBox3dProperty readBoundProperty() const override
		{
			return m_curves.getSchema().getSelfBoundsProperty();
		}

		size_t readNumSamples() const override
		{
			return m_curves.getSchema().getNumSamples();
		}

		Alembic::AbcCoreAbstract::TimeSamplingPtr readTimeSampling() const override
		{
			return m_curves.getSchema().getTimeSampling();
		}

		IECore::ObjectPtr readSample( const Alembic::Abc::ISampleSelector &sampleSelector ) const override
		{
			const ICurvesSchema curvesSchema = m_curves.getSchema();
			const ICurvesSchema::Sample sample = curvesSchema.getValue( sampleSelector );

			IntVectorDataPtr vertsPerCurve = new IntVectorData;
			vertsPerCurve->writable().insert(
				vertsPerCurve->writable().end(),
				sample.getCurvesNumVertices()->get(),
				sample.getCurvesNumVertices()->get() + sample.getCurvesNumVertices()->size()
			);

			V3fVectorDataPtr points = new V3fVectorData();
			points->writable().insert( points->writable().end(), sample.getPositions()->get(), sample.getPositions()->get() + sample.getPositions()->size() );

			CurvesPrimitivePtr result = new CurvesPrimitive(
				vertsPerCurve,
				convertBasis( sample ),
				sample.getWrap() == kPeriodic,
				points
			);

			if( Alembic::Abc::V3fArraySamplePtr velocities = sample.getVelocities() )
			{
				V3fVectorDataPtr velocityData = new V3fVectorData;
				velocityData->writable().insert( velocityData->writable().end(), velocities->get(), velocities->get() + velocities->size() );
				velocityData->setInterpretation( GeometricData::Vector );
				result->variables["velocity"] = PrimitiveVariable( PrimitiveVariable::Vertex, velocityData );
			}

			if( Alembic::AbcGeom::IFloatGeomParam widthsParam = curvesSchema.getWidthsParam() )
			{
				readGeomParam( widthsParam, sampleSelector, result.get() );
			}

			if( Alembic::AbcGeom::IV2fGeomParam uvsParam = curvesSchema.getUVsParam() )
			{
				readGeomParam( uvsParam, sampleSelector, result.get() );
				if( auto uvData = result->variableData<V2fVectorData>( "uv" ) )
				{
					uvData->setInterpretation( GeometricData::UV );
				}
			}

			if( Alembic::AbcGeom::IN3fGeomParam nParam = curvesSchema.getNormalsParam() )
			{
				readGeomParam( nParam, sampleSelector, result.get() );
			}

			ICompoundProperty arbGeomParams = curvesSchema.getArbGeomParams();
			readArbGeomParams( arbGeomParams, sampleSelector, result.get() );

			return result;
		}

	private :

		const ICurves m_curves;

		static Description<CurvesReader, ICurves> g_description;
};

IECoreAlembic::ObjectReader::Description<CurvesReader, ICurves> CurvesReader::g_description( CurvesPrimitive::staticTypeId() );

} // namespace
