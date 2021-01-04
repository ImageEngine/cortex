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

#include "IECoreScene/PointsPrimitive.h"

#include "Alembic/AbcGeom/IPoints.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreAlembic;
using namespace Alembic::AbcGeom;

namespace
{

class PointsReader : public PrimitiveReader
{

	public :

		PointsReader( const IPoints &points )
			:	m_points( points )
		{
		}

		const Alembic::Abc::IObject &object() const override
		{
			return m_points;
		}

		Alembic::Abc::IBox3dProperty readBoundProperty() const override
		{
			return m_points.getSchema().getSelfBoundsProperty();
		}

		size_t readNumSamples() const override
		{
			return m_points.getSchema().getNumSamples();
		}

		Alembic::AbcCoreAbstract::TimeSamplingPtr readTimeSampling() const override
		{
			return m_points.getSchema().getTimeSampling();
		}

		IECore::ObjectPtr readSample( const Alembic::Abc::ISampleSelector &sampleSelector ) const override
		{
			const IPointsSchema &pointsSchema = m_points.getSchema();
			const IPointsSchema::Sample sample = pointsSchema.getValue( sampleSelector );

			V3fVectorDataPtr p = new V3fVectorData();
			p->writable().insert( p->writable().end(), sample.getPositions()->get(), sample.getPositions()->get() + sample.getPositions()->size() );

			PointsPrimitivePtr result = new PointsPrimitive( p );

			UInt64VectorDataPtr id = new UInt64VectorData;
			id->writable().insert( id->writable().end(), sample.getIds()->get(), sample.getIds()->get() + sample.getIds()->size() );
			result->variables["id"] = PrimitiveVariable( PrimitiveVariable::Vertex, id );

			if( Alembic::Abc::V3fArraySamplePtr velocities = sample.getVelocities() )
			{
				V3fVectorDataPtr velocityData = new V3fVectorData;
				velocityData->writable().insert( velocityData->writable().begin(), velocities->get(), velocities->get() + velocities->size() );
				velocityData->setInterpretation( GeometricData::Vector );
				result->variables["velocity"] = PrimitiveVariable( PrimitiveVariable::Vertex, velocityData );
			}

			if( Alembic::AbcGeom::IFloatGeomParam widthsParam = pointsSchema.getWidthsParam() )
			{
				readGeomParam( widthsParam, sampleSelector, result.get(), "width" );
			}

			ICompoundProperty arbGeomParams = pointsSchema.getArbGeomParams();
			readArbGeomParams( arbGeomParams, sampleSelector, result.get() );

			return result;
		}

	private :

		const IPoints m_points;

		static Description<PointsReader, IPoints> g_description;
};

IECoreAlembic::ObjectReader::Description<PointsReader, IPoints> PointsReader::g_description( PointsPrimitive::staticTypeId() );

} // namespace
