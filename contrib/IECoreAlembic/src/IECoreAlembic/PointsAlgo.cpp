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

#include "IECoreAlembic/ObjectAlgo.h"
#include "IECoreAlembic/PointsAlgo.h"
#include "IECoreAlembic/GeomBaseAlgo.h"

using namespace IECore;
using namespace IECoreAlembic;
using namespace Alembic::AbcGeom;

namespace IECoreAlembic
{

namespace PointsAlgo
{

IECOREALEMBIC_API IECore::PointsPrimitivePtr convert( const Alembic::AbcGeom::IPoints &points, const Alembic::Abc::ISampleSelector &sampleSelector )
{
	const IPointsSchema &pointsSchema = points.getSchema();
	const IPointsSchema::Sample sample = pointsSchema.getValue( sampleSelector );

	V3fVectorDataPtr p = new V3fVectorData();
	p->writable().resize( sample.getPositions()->size() );
	memcpy( &(p->writable()[0]), sample.getPositions()->get(), sample.getPositions()->size() * sizeof( Imath::V3f ) );

	PointsPrimitivePtr result = new PointsPrimitive( p );

	V3fVectorDataPtr velocity = new V3fVectorData;
	velocity->writable().resize( sample.getVelocities()->size() );
	memcpy( &(velocity->writable()[0]), sample.getVelocities()->get(), sample.getVelocities()->size() * sizeof( Imath::V3f ) );
	velocity->setInterpretation( GeometricData::Vector );
	result->variables["velocity"] = PrimitiveVariable( PrimitiveVariable::Vertex, velocity );

	UInt64VectorDataPtr id = new UInt64VectorData;
	id->writable().resize( sample.getIds()->size() );
	memcpy( &(id->writable()[0]), sample.getIds()->get(), sample.getIds()->size() * sizeof( uint64_t ) );
	result->variables["id"] = PrimitiveVariable( PrimitiveVariable::Vertex, id );

	ICompoundProperty arbGeomParams = pointsSchema.getArbGeomParams();
	GeomBaseAlgo::convertArbGeomParams( arbGeomParams, sampleSelector, result.get() );

	return result;
}

} // namespace PointsAlgo

} // namespace IECoreAlembic

static ObjectAlgo::ConverterDescription<IPoints, PointsPrimitive> g_Description( &IECoreAlembic::PointsAlgo::convert );

