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

#include "IECoreAlembic/PrimitiveWriter.h"

#include "IECoreScene/MeshAlgo.h"
#include "IECoreScene/MeshPrimitive.h"

#include "IECore/MessageHandler.h"

#include "Alembic/AbcGeom/OPolyMesh.h"
#include "Alembic/AbcGeom/OSubD.h"

#include "boost/variant.hpp"

using namespace std;
using namespace Imath;
using namespace Alembic::AbcGeom;
using namespace IECore;
using namespace IECoreScene;
using namespace IECoreAlembic;

namespace
{

class MeshWriter : public PrimitiveWriter
{

	public :

		MeshWriter( Alembic::Abc::OObject &parent, const std::string &name )
			:	m_state( ConstructorArguments{ parent, name } )
		{
		}

		void writeSample( const IECore::Object *object ) override
		{
			ConstMeshPrimitivePtr meshPrimitive;
			if( const MeshPrimitive *typedObject = runTimeCast<const MeshPrimitive>( object ) )
			{
				MeshPrimitivePtr meshPrimitiveCopy = typedObject->copy();
				IECoreScene::MeshAlgo::reverseWinding( meshPrimitiveCopy.get() );
				meshPrimitive = meshPrimitiveCopy;
			}
			else
			{
				throw IECore::Exception( "MeshWriter expected a MeshPrimitive" );
			}

			// If this is the first sample, construct an OPolyMesh or OSubD depending on the mesh
			// interpolation.

			if( m_state.which() == PendingConstructionState )
			{
				const ConstructorArguments &constructorArguments = boost::get<ConstructorArguments>( m_state );
				if( meshPrimitive->interpolation() == "linear" )
				{
					m_state = OPolyMesh( constructorArguments.parent, constructorArguments.name );
				}
				else
				{
					m_state = OSubD( constructorArguments.parent, constructorArguments.name );
				}
			}

			// Now write the sample

			if( m_state.which() == PolyMeshState )
			{
				OPolyMeshSchema::Sample sample;

				std::vector<unsigned int> indices;
				PrimitiveVariableMap::const_iterator nIt = meshPrimitive->variables.find( "N" );
				if( nIt != meshPrimitive->variables.end() )
				{
					const V3fVectorData *nData = runTimeCast<V3fVectorData>( nIt->second.data.get() );
					if( nData )
					{
						ON3fGeomParam::Sample nSample( nData->readable(), geometryScope( nIt->second.interpolation ) );

						if( nIt->second.indices )
						{
							const std::vector<int> &indexValues = nIt->second.indices->readable();
							indices = std::vector<unsigned int>( indexValues.begin(), indexValues.end() );
							nSample.setIndices( indices );
						}

						sample.setNormals( nSample );
					}
				}

				writeSampleInternal( meshPrimitive.get(), sample, boost::get<OPolyMesh>( m_state ).getSchema() );
			}
			else
			{
				OSubDSchema::Sample sample;
				sample.setSubdivisionScheme( "catmull-clark" );

				if( meshPrimitive->cornerIds()->readable().size() )
				{
					sample.setCorners(
						Abc::Int32ArraySample( meshPrimitive->cornerIds()->readable() ),
						Abc::FloatArraySample( meshPrimitive->cornerSharpnesses()->readable() )
					);
				}

				if( meshPrimitive->creaseLengths()->readable().size() )
				{
					sample.setCreases(
						Abc::Int32ArraySample( meshPrimitive->creaseIds()->readable() ),
						Abc::Int32ArraySample( meshPrimitive->creaseLengths()->readable() ),
						Abc::FloatArraySample( meshPrimitive->creaseSharpnesses()->readable() )
					);
				}

				writeSampleInternal( meshPrimitive.get(), sample, boost::get<OSubD>( m_state ).getSchema() );
			}

		}

		void writeTimeSampling( const Alembic::AbcCoreAbstract::TimeSamplingPtr &timeSampling ) override
		{
			switch( m_state.which() )
			{
				case PolyMeshState :
					boost::get<OPolyMesh>( m_state ).getSchema().setTimeSampling( timeSampling );
					break;
				case SubDState :
					boost::get<OSubD>( m_state ).getSchema().setTimeSampling( timeSampling );
					break;
				default :
					throw IECore::Exception( "MeshWriter::writeSample() must be called before MeshWriter::writeTimeSampling()" );
			}
		}

	private :

		template<typename Schema>
		void writeSampleInternal( const IECoreScene::MeshPrimitive *meshPrimitive, typename Schema::Sample &sample, Schema &schema )
		{

			sample.setFaceCounts(
				Abc::Int32ArraySample( meshPrimitive->verticesPerFace()->readable() )
			);

			sample.setFaceIndices(
				Abc::Int32ArraySample( meshPrimitive->vertexIds()->readable() )
			);

			if( const V3fVectorData *p = meshPrimitive->variableData<V3fVectorData>( "P" ) )
			{
				sample.setPositions(
					Abc::P3fArraySample( p->readable() )
				);
			}

			if( const V3fVectorData *v = meshPrimitive->variableData<V3fVectorData>( "velocity" ) )
			{
				sample.setVelocities(
					Abc::V3fArraySample( v->readable() )
				);
			}

			std::vector<unsigned int> indices;
			PrimitiveVariableMap::const_iterator uvIt = meshPrimitive->variables.find( "uv" );
			if( uvIt != meshPrimitive->variables.end() )
			{
				const V2fVectorData *uvData = runTimeCast<V2fVectorData>( uvIt->second.data.get() );
				if( uvData )
				{
					OV2fGeomParam::Sample uvSample( uvData->readable(), geometryScope( uvIt->second.interpolation ) );

					if( uvIt->second.indices )
					{
						const std::vector<int> &indexValues = uvIt->second.indices->readable();
						indices = std::vector<unsigned int>( indexValues.begin(), indexValues.end() );
						uvSample.setIndices( indices );
					}

					sample.setUVs( uvSample );
				}
			}

			const char *namesToIgnore[] = { "P", "N", "uv", "velocity", nullptr };
			OCompoundProperty geomParams = schema.getArbGeomParams();
			writeArbGeomParams( meshPrimitive, geomParams, namesToIgnore );

			schema.set( sample );

		}

		struct ConstructorArguments
		{
			OObject parent;
			std::string name;
		};

		enum WhichState
		{
			PendingConstructionState = 1,
			PolyMeshState = 2,
			SubDState = 3
		};

		// Most ObjectWriters are able to create an Alembic OObject of the right type in their constructor, but
		// we can't because we don't know whether we should create an OPolyMesh or an OSubD until the first call
		// to `writeSample()`. We are therefore always in one of three states :
		//
		// 1. When `writeSample()` has not been called yet, our state contains the ConstructorArguments we will need
		//    to enter the next state.
		// 2. When `writeSample()` has been called with a linear mesh, our state contains the OPolyMesh we created.
		// 3. When `writeSample()` has been called with a subdiv mesh, our state contains the OSubD we created.
		//
		// We use a boost::variant to store our state, ensuring we can only be in one state at a time, and avoiding
		// the cost of storing each state separately (OSubd and OPolyMesh are in the region of a kilobyte
		// each, which quickly adds up for large scenes). We include boost::blank in our state only to enable
		// the "single storage" optimisation as described in the boost documentation.
		typedef boost::variant<boost::blank, ConstructorArguments, Alembic::AbcGeom::OPolyMesh, Alembic::AbcGeom::OSubD> State;
		State m_state;

		static Description<MeshWriter> g_description;

};

IECoreAlembic::ObjectWriter::Description<MeshWriter> MeshWriter::g_description( IECoreScene::MeshPrimitive::staticTypeId() );

} // namespace
