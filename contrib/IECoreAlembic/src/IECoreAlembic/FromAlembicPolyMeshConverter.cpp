//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

#include "IECore/MeshPrimitive.h"

#include "IECoreAlembic/FromAlembicPolyMeshConverter.h"

using namespace IECore;
using namespace IECoreAlembic;
using namespace Alembic::AbcGeom;

FromAlembicPolyMeshConverter::ConverterDescription<FromAlembicPolyMeshConverter> FromAlembicPolyMeshConverter::g_description;

IE_CORE_DEFINERUNTIMETYPED( FromAlembicPolyMeshConverter );

FromAlembicPolyMeshConverter::FromAlembicPolyMeshConverter( Alembic::Abc::IObject iPolyMesh )
	:	FromAlembicGeomBaseConverter( "Converts AbcGeom::IPolyMesh objects to IECore::MeshPrimitive objects", iPolyMesh )
{
}

IECore::ObjectPtr FromAlembicPolyMeshConverter::doAlembicConversion( const Alembic::Abc::IObject &iObject, const Alembic::Abc::ISampleSelector &sampleSelector, const IECore::CompoundObject *operands ) const
{
	IPolyMesh iPolyMesh( iObject, kWrapExisting );
	IPolyMeshSchema &iPolyMeshSchema = iPolyMesh.getSchema();

	IPolyMeshSchema::Sample sample = iPolyMeshSchema.getValue( sampleSelector );

	IntVectorDataPtr verticesPerFace = new IntVectorData();
	verticesPerFace->writable().insert(
		verticesPerFace->writable().begin(),
		sample.getFaceCounts()->get(),
		sample.getFaceCounts()->get() + sample.getFaceCounts()->size()
	);

	IntVectorDataPtr vertexIds = new IntVectorData();
	vertexIds->writable().insert(
		vertexIds->writable().begin(),
		sample.getFaceIndices()->get(),
		sample.getFaceIndices()->get() + sample.getFaceIndices()->size()
	);

	V3fVectorDataPtr points = new V3fVectorData();
	points->writable().resize( sample.getPositions()->size() );
	memcpy( &(points->writable()[0]), sample.getPositions()->get(), sample.getPositions()->size() * sizeof( Imath::V3f ) );

	MeshPrimitivePtr result = new IECore::MeshPrimitive( verticesPerFace, vertexIds, "linear", points );

	IN3fGeomParam normals = iPolyMeshSchema.getNormalsParam();
	if( normals.valid() )
	{
		convertGeomParam( normals, sampleSelector, result.get() );
	}

	Alembic::AbcGeom::IV2fGeomParam uvs = iPolyMeshSchema.getUVsParam();
	convertUVs( uvs, sampleSelector, result.get() );

	ICompoundProperty arbGeomParams = iPolyMeshSchema.getArbGeomParams();
	convertArbGeomParams( arbGeomParams, sampleSelector, result.get() );

	return result;
}
