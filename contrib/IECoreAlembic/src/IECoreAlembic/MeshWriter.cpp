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

#include "boost/variant.hpp"

#include "Alembic/AbcGeom/OPolyMesh.h"
#include "Alembic/AbcGeom/OSubD.h"

#include "IECore/MeshPrimitive.h"
#include "IECore/MeshAlgo.h"
#include "IECore/MessageHandler.h"

#include "IECoreAlembic/PrimitiveWriter.h"

using namespace std;
using namespace Imath;
using namespace Alembic::AbcGeom;
using namespace IECore;
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

		virtual void writeSample( const IECore::Object *object )
		{
			ConstMeshPrimitivePtr meshPrimitive;
			if( const MeshPrimitive *typedObject = runTimeCast<const MeshPrimitive>( object ) )
			{
				MeshPrimitivePtr meshPrimitiveCopy = typedObject->copy();
				IECore::MeshAlgo::reverseWinding( meshPrimitiveCopy.get() );
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

				if( const V3fVectorData *n = meshPrimitive->variableData<V3fVectorData>( "N" ) )
				{
					sample.setNormals(
						ON3fGeomParam::Sample( n->readable(), geometryScope( meshPrimitive->variables.find( "N" )->second.interpolation ) )
					);
				}

				writeSampleInternal( meshPrimitive.get(), sample, boost::get<OPolyMesh>( m_state ).getSchema() );
			}
			else
			{
				OSubDSchema::Sample sample;
				sample.setSubdivisionScheme( "catmull-clark" );
				writeSampleInternal( meshPrimitive.get(), sample, boost::get<OSubD>( m_state ).getSchema() );
			}

		}

		virtual void writeTimeSampling( const Alembic::AbcCoreAbstract::TimeSamplingPtr &timeSampling )
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
		void writeSampleInternal( const IECore::MeshPrimitive *meshPrimitive, typename Schema::Sample &sample, Schema &schema )
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

			const FloatVectorData *sData = meshPrimitive->variableData<FloatVectorData>( "s" );
			const FloatVectorData *tData = meshPrimitive->variableData<FloatVectorData>( "t" );
			vector<V2f> st;
			if( sData && tData )
			{
				const vector<float> &s = sData->readable();
				const vector<float> &t = tData->readable();
				if( s.size() == t.size() )
				{
					st.reserve( s.size() );
					for( vector<float>::const_iterator sIt = s.begin(), tIt = t.begin(), eIt = s.end(); sIt != eIt; ++sIt, ++tIt )
					{
						st.push_back( V2f( *sIt, *tIt ) );
					}
					sample.setUVs(
						OV2fGeomParam::Sample( st, geometryScope( meshPrimitive->variables.find( "s" )->second.interpolation ) )
					);
				}
				else
				{
					IECore::msg( IECore::Msg::Warning, "MeshWriter", "s and t have different sizes" );
				}
			}

			const char *namesToIgnore[] = { "P", "N", "s", "t", nullptr };
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

IECoreAlembic::ObjectWriter::Description<MeshWriter> MeshWriter::g_description( IECore::MeshPrimitive::staticTypeId() );

} // namespace
