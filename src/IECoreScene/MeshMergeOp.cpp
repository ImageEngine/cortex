//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/MeshAlgo.h"
#include "IECoreScene/MeshMergeOp.h"

#include "IECore/CompoundParameter.h"

using namespace IECore;
using namespace IECoreScene;

IE_CORE_DEFINERUNTIMETYPED( MeshMergeOp );

MeshMergeOp::MeshMergeOp()
	:	MeshPrimitiveOp( "Merges one mesh with another." )
{
	m_meshParameter = new MeshPrimitiveParameter(
		"mesh",
		"The mesh to be merged with the input.",
		new MeshPrimitive
	);

	m_removePrimVarsParameter = new BoolParameter(
		"removeNonMatchingPrimVars",
		"If true, PrimitiveVariables that exist on one mesh and not the other will be removed. If false, the PrimitiveVariable data will be expanded using a default value.",
		false
	);

	parameters()->addParameter( m_meshParameter );
	parameters()->addParameter( m_removePrimVarsParameter );
}

MeshMergeOp::~MeshMergeOp()
{
}

MeshPrimitiveParameter * MeshMergeOp::meshParameter()
{
	return m_meshParameter.get();
}

const MeshPrimitiveParameter * MeshMergeOp::meshParameter() const
{
	return m_meshParameter.get();
}

void MeshMergeOp::modifyTypedPrimitive( MeshPrimitive * mesh, const CompoundObject * operands )
{
	std::vector<const MeshPrimitive *> meshes( { mesh } );

	const MeshPrimitive *mesh2 = static_cast<const MeshPrimitive *>( m_meshParameter->getValue() );

	MeshPrimitivePtr mesh3 = nullptr;
	if( operands->member<BoolData>( "removeNonMatchingPrimVars" )->readable() )
	{
		mesh3 = mesh2->copy();

		std::vector<const char *> toErase;
		for( auto &pv : mesh3->variables )
		{
			auto other = mesh->variables.find( pv.first );
			if(
				other == mesh->variables.end() ||
				!other->second.data->isInstanceOf( pv.second.data->typeId() ) ||
				( other->second.interpolation != pv.second.interpolation )
			)
			{
				toErase.push_back( pv.first.c_str() );
			}
		}
		for( auto name : toErase )
		{
			mesh3->variables.erase( name );
		}

		toErase.clear();
		for( auto &pv : mesh->variables )
		{
			auto other = mesh3->variables.find( pv.first );
			if(
				other == mesh3->variables.end() ||
				!other->second.data->isInstanceOf( pv.second.data->typeId() ) ||
				( other->second.interpolation != pv.second.interpolation )
			)
			{
				toErase.push_back( pv.first.c_str() );
			}
		}
		for( auto name : toErase )
		{
			mesh->variables.erase( name );
		}

		meshes.push_back( mesh3.get() );
	}
	else
	{
		meshes.push_back( mesh2 );
	}

	IECoreScene::MeshPrimitivePtr result = IECoreScene::MeshAlgo::merge( meshes );
	if( result )
	{
		mesh->setTopology( result->verticesPerFace(), result->vertexIds(), result->interpolation() );
		mesh->variables = result->variables;
	}
}
