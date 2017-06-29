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

#include "IECoreAlembic/ObjectAlgo.h"
#include "IECoreAlembic/MeshAlgo.h"
#include "IECoreAlembic/GeomBaseAlgo.h"

using namespace IECore;
using namespace IECoreAlembic;
using namespace Alembic::AbcGeom;

namespace
{

template<typename Schema>
IECore::MeshPrimitivePtr convertCommon( const Schema &schema, const Alembic::Abc::ISampleSelector &sampleSelector )
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

	MeshPrimitivePtr result = new IECore::MeshPrimitive( verticesPerFace, vertexIds, "linear", points );

	Alembic::AbcGeom::IV2fGeomParam uvs = schema.getUVsParam();
	GeomBaseAlgo::convertUVs( uvs, sampleSelector, result.get() );

	ICompoundProperty arbGeomParams = schema.getArbGeomParams();
	GeomBaseAlgo::convertArbGeomParams( arbGeomParams, sampleSelector, result.get() );

	return result;
}

} // namespace

namespace IECoreAlembic
{

namespace MeshAlgo
{

IECore::MeshPrimitivePtr convert( const Alembic::AbcGeom::IPolyMesh &mesh, const Alembic::Abc::ISampleSelector &sampleSelector )
{
	const IPolyMeshSchema &iPolyMeshSchema = mesh.getSchema();
	MeshPrimitivePtr result = convertCommon( iPolyMeshSchema, sampleSelector );

	IN3fGeomParam normals = iPolyMeshSchema.getNormalsParam();
	if( normals.valid() )
	{
		GeomBaseAlgo::convertGeomParam( normals, sampleSelector, result.get() );
	}

	return result;
}

IECore::MeshPrimitivePtr convert( const Alembic::AbcGeom::ISubD &mesh, const Alembic::Abc::ISampleSelector &sampleSelector )
{
	const ISubDSchema &iSubDSchema = mesh.getSchema();
	MeshPrimitivePtr result = convertCommon( iSubDSchema, sampleSelector );

	std::string interpolation = iSubDSchema.getSubdivisionSchemeProperty().getValue();
	if( interpolation == "catmull-clark" )
	{
		interpolation = "catmullClark";
	}
	result->setInterpolation( interpolation );

	return result;
}

} // namespace MeshAlgo

} // namespace IECoreAlembic

static ObjectAlgo::ConverterDescription<IPolyMesh, MeshPrimitive> g_polyMeshDescription( &IECoreAlembic::MeshAlgo::convert );
static ObjectAlgo::ConverterDescription<ISubD, MeshPrimitive> g_subDDescription( &IECoreAlembic::MeshAlgo::convert );

