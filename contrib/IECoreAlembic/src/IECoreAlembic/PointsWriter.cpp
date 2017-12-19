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

#include <numeric>

#include "IECoreScene/PointsPrimitive.h"

#include "Alembic/AbcGeom/OPoints.h"

#include "IECoreAlembic/PrimitiveWriter.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreAlembic;
using namespace Alembic::AbcGeom;

namespace
{

class PointsWriter : public PrimitiveWriter
{

	public :

		PointsWriter( Alembic::Abc::OObject &parent, const std::string &name )
			:	m_points( parent, name )
		{
		}

		void writeSample( const IECore::Object *object ) override
		{
			const PointsPrimitive *pointsPrimitive = runTimeCast<const PointsPrimitive>( object );

			// declare a pointer to generated id data so it stays in scope until we set the sample
			// on the schema
			std::unique_ptr< std::vector<uint64_t> >  inventedIds;

			if( !pointsPrimitive )
			{
				throw IECore::Exception( "PointsWriter expected a PointsPrimitive" );
			}

			OPointsSchema::Sample sample;
			if( const V3fVectorData *p = pointsPrimitive->variableData<V3fVectorData>( "P" ) )
			{
				sample.setPositions(
					Abc::P3fArraySample( p->readable() )
				);
			}

			if( const V3fVectorData *v = pointsPrimitive->variableData<V3fVectorData>( "velocity" ) )
			{
				sample.setVelocities(
					Abc::V3fArraySample( v->readable() )
				);
			}

			if( const UInt64VectorData *ids = pointsPrimitive->variableData<UInt64VectorData>( "id" ) )
			{
				sample.setIds(
					Abc::UInt64ArraySample( ids->readable() )
				);
			}
			else
			{
				// Alembic _requires_ ids to be provided, so we must invent some
				// even if we don't have them.

				inventedIds.reset( new std::vector<uint64_t>( pointsPrimitive->variableSize( PrimitiveVariable::Vertex ) ) );
				std::iota( inventedIds->begin(), inventedIds->end(), 0 );
				sample.setIds( Abc::UInt64ArraySample( *inventedIds ) );
			}

			const char *namesToIgnore[] = { "P", "velocity", "id", nullptr };
			OCompoundProperty geomParams = m_points.getSchema().getArbGeomParams();
			writeArbGeomParams( pointsPrimitive, geomParams, namesToIgnore );

			m_points.getSchema().set( sample );
		}

		void writeTimeSampling( const Alembic::AbcCoreAbstract::TimeSamplingPtr &timeSampling ) override
		{
			m_points.getSchema().setTimeSampling( timeSampling );
		}

	private :

		Alembic::AbcGeom::OPoints m_points;

		static Description<PointsWriter> g_description;

};

IECoreAlembic::ObjectWriter::Description<PointsWriter> PointsWriter::g_description( IECoreScene::PointsPrimitive::staticTypeId() );

} // namespace
