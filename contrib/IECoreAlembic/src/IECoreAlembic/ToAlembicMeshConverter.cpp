//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2012, Image Engine Design Inc. All rights reserved.
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
#include "IECore/Exception.h"
#include "IECore/MessageHandler.h"

#include "IECoreAlembic/ToAlembicMeshConverter.h"

using namespace IECoreAlembic;
using namespace IECore;
using namespace std;

using namespace Alembic::Abc;
using namespace Alembic::AbcGeom;

IE_CORE_DEFINERUNTIMETYPED( ToAlembicMeshConverter );

ToAlembicMeshConverter::ConverterDescription<ToAlembicMeshConverter> ToAlembicMeshConverter::g_description;

ToAlembicMeshConverter::ToAlembicMeshConverter( OObject transform )
	:	ToAlembicConverter( "Converts IECore::MeshPrimitives to alembic polymesh nodes", IECore::MeshPrimitive::staticTypeId(), transform )
{
}

ToAlembicMeshConverter::~ToAlembicMeshConverter()
{
}

void ToAlembicMeshConverter::ensureAlembicObject( Alembic::Abc::OObject &transform )
{
	const MeshPrimitive *mesh = runTimeCast<MeshPrimitive>( srcParameter()->getValue() );
	if( mesh->interpolation() == "catmullClark" )
	{
		if( m_polyMesh )
		{
			throw IECore::Exception( "ToAlembicMeshConverter::ensureAlembicObject(): trying to write a subdiv after writing a poly mesh" );
		}

		// create a subdiv object
		if( !m_subD )
		{
			m_subD = OSubD( transform, "subdiv" );
		}
	}
	else
	{
		if( m_subD )
		{
			throw IECore::Exception( "ToAlembicMeshConverter::ensureAlembicObject(): trying to write a poly mesh after writing a subdiv" );
		}

		// create a polyMesh object
		if( !m_polyMesh )
		{
			m_polyMesh = OPolyMesh( transform, "mesh" );
		}
	}
}

template<class Sample>
static void convertSample( Sample &sample, const MeshPrimitive *mesh )
{
	const V3fVectorData *pData = mesh->variableData<V3fVectorData>( "P", PrimitiveVariable::Vertex );
	if( !pData )
	{
		throw IECore::Exception( "ToAlembicMeshConverter::writeAlembicObject(): mesh has no 'P' data" );
	}
	
	// set topology:
	sample.setFaceCounts( mesh->verticesPerFace()->readable() );
	sample.setFaceIndices( mesh->vertexIds()->readable() );

	// set vertex positions:
	sample.setPositions( P3fArraySample( pData->readable().data(), pData->readable().size() ) );
}

void ToAlembicMeshConverter::writeAlembicObject()
{
	const MeshPrimitive *mesh = runTimeCast<MeshPrimitive>( srcParameter()->getValue() );
	if( m_subD )
	{
		OSubDSchema::Sample sample;
		convertSample( sample, mesh );
		sample.setSubdivisionScheme( "catmull-clark" );
		m_subD.getSchema().set( sample );
	}
	else
	{
		OPolyMeshSchema::Sample sample;
		convertSample( sample, mesh );
		m_polyMesh.getSchema().set( sample );
	}
}

void ToAlembicMeshConverter::updateTimeSampling( Alembic::Abc::TimeSamplingPtr timeSampling )
{
	if( m_subD )
	{
		m_subD.getSchema().setTimeSampling( timeSampling );
	}
	else
	{
		m_polyMesh.getSchema().setTimeSampling( timeSampling );
	}
}
