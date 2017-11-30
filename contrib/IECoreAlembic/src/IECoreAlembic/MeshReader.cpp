//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "Alembic/AbcGeom/IPolyMesh.h"
#include "Alembic/AbcGeom/ISubD.h"

#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/MeshAlgo.h"

#include "IECoreAlembic/PrimitiveReader.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreAlembic;
using namespace Alembic::AbcGeom;

namespace
{

class MeshReader : public PrimitiveReader
{

	protected :

		template<typename Schema>
		IECoreScene::MeshPrimitivePtr readTypedSample( const Schema &schema, const Alembic::Abc::ISampleSelector &sampleSelector ) const
		{
			Abc::Int32ArraySamplePtr faceCountsSample;
			schema.getFaceCountsProperty().get( faceCountsSample, sampleSelector );

			IntVectorDataPtr verticesPerFace = new IntVectorData();
			verticesPerFace->writable().insert(
				verticesPerFace->writable().begin(),
				faceCountsSample->get(),
				faceCountsSample->get() + faceCountsSample->size()
			);

			Abc::Int32ArraySamplePtr faceIndicesSample;
			schema.getFaceIndicesProperty().get( faceIndicesSample, sampleSelector );

			IntVectorDataPtr vertexIds = new IntVectorData();
			vertexIds->writable().insert(
				vertexIds->writable().begin(),
				faceIndicesSample->get(),
				faceIndicesSample->get() + faceIndicesSample->size()
			);

			Abc::P3fArraySamplePtr positionsSample;
			schema.getPositionsProperty().get( positionsSample, sampleSelector );

			V3fVectorDataPtr points = new V3fVectorData();
			points->writable().resize( positionsSample->size() );
			memcpy( &(points->writable()[0]), positionsSample->get(), positionsSample->size() * sizeof( Imath::V3f ) );

			MeshPrimitivePtr result = new IECoreScene::MeshPrimitive( verticesPerFace, vertexIds, "linear", points );

			Alembic::AbcGeom::IV2fGeomParam uvs = schema.getUVsParam();
			readUVs( uvs, sampleSelector, result.get() );

			ICompoundProperty arbGeomParams = schema.getArbGeomParams();
			readArbGeomParams( arbGeomParams, sampleSelector, result.get() );

			return result;
		}

	private :

		void readUVs( const Alembic::AbcGeom::IV2fGeomParam &uvs, const Alembic::Abc::ISampleSelector &sampleSelector, IECoreScene::Primitive *primitive ) const
		{
			if( !uvs.valid() )
			{
				return;
			}

			typedef IV2fArrayProperty::sample_ptr_type SamplePtr;
			auto uvParam = uvs.getIndexedValue( sampleSelector );
			SamplePtr sample = uvParam.getVals();
			size_t uvSize = sample->size();
			V2fVectorDataPtr uvData = new V2fVectorData;
			uvData->setInterpretation( GeometricData::UV );
			std::vector<Imath::V2f> &uvValues = uvData->writable();
			uvValues.reserve( uvSize );
			for( size_t i=0; i<uvSize; ++i )
			{
				uvValues.push_back( (*sample)[i] );
			}

			IntVectorDataPtr indexData = nullptr;
			if( uvParam.isIndexed() )
			{
				UInt32ArraySamplePtr indices = uvParam.getIndices();
				size_t indexSize = indices->size();
				indexData = new IntVectorData;
				std::vector<int> &indexValues = indexData->writable();
				indexValues.reserve( indexSize );
				for( size_t i = 0; i < indexSize; ++i )
				{
					indexValues.push_back( (*indices)[i] );
				}
			}

			PrimitiveVariable::Interpolation interpolation = PrimitiveReader::interpolation( uvs.getScope() );
			primitive->variables["uv"] = PrimitiveVariable( interpolation, uvData, indexData );
		}

};

class PolyMeshReader : public MeshReader
{

	public :

		PolyMeshReader( const IPolyMesh &polyMesh )
			:	m_polyMesh( polyMesh )
		{
		}

		const Alembic::Abc::IObject &object() const override
		{
			return m_polyMesh;
		}

		Alembic::Abc::IBox3dProperty readBoundProperty() const override
		{
			return m_polyMesh.getSchema().getSelfBoundsProperty();
		}

		size_t readNumSamples() const override
		{
			return m_polyMesh.getSchema().getNumSamples();
		}

		Alembic::AbcCoreAbstract::TimeSamplingPtr readTimeSampling() const override
		{
			return m_polyMesh.getSchema().getTimeSampling();
		}

		IECore::ObjectPtr readSample( const Alembic::Abc::ISampleSelector &sampleSelector ) const override
		{
			const IPolyMeshSchema &schema = m_polyMesh.getSchema();
			MeshPrimitivePtr result = readTypedSample( schema, sampleSelector );

			IN3fGeomParam normals = schema.getNormalsParam();
			if( normals.valid() )
			{
				readGeomParam( normals, sampleSelector, result.get() );
			}

			IECoreScene::MeshAlgo::reverseWinding( result.get() );
			return result;
		}

	private :

		const IPolyMesh m_polyMesh;

		static Description<PolyMeshReader, IPolyMesh> g_description;
};

IECoreAlembic::ObjectReader::Description<PolyMeshReader, IPolyMesh> PolyMeshReader::g_description( MeshPrimitive::staticTypeId() );

class SubDReader : public MeshReader
{

	public :

		SubDReader( const ISubD &subD )
			:	m_subD( subD )
		{
		}

		const Alembic::Abc::IObject &object() const override
		{
			return m_subD;
		}

		Alembic::Abc::IBox3dProperty readBoundProperty() const override
		{
			return m_subD.getSchema().getSelfBoundsProperty();
		}

		size_t readNumSamples() const override
		{
			return m_subD.getSchema().getNumSamples();
		}

		Alembic::AbcCoreAbstract::TimeSamplingPtr readTimeSampling() const override
		{
			return m_subD.getSchema().getTimeSampling();
		}

		IECore::ObjectPtr readSample( const Alembic::Abc::ISampleSelector &sampleSelector ) const override
		{
			const ISubDSchema &schema = m_subD.getSchema();
			MeshPrimitivePtr result = readTypedSample( schema, sampleSelector );

			std::string interpolation = "catmullClark";
			if( IStringProperty p = schema.getSubdivisionSchemeProperty() )
			{
				interpolation = p.getValue();
				if( interpolation == "catmull-clark" )
				{
					interpolation = "catmullClark";
				}
			}
			result->setInterpolation( interpolation );

			IECoreScene::MeshAlgo::reverseWinding( result.get() );
			return result;
		}

	private :

		const ISubD m_subD;

		static Description<SubDReader, ISubD> g_description;
};

IECoreAlembic::ObjectReader::Description<SubDReader, ISubD> SubDReader::g_description( MeshPrimitive::staticTypeId() );

} // namespace
