//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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
#include "IECoreScene/MeshVertexReorderOp.h"

#include "IECore/CompoundParameter.h"
#include "IECore/DespatchTypedData.h"

#include "boost/format.hpp"

using namespace IECore;
using namespace IECoreScene;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( MeshVertexReorderOp );

MeshVertexReorderOp::MeshVertexReorderOp() : MeshPrimitiveOp( "Calculates vertex normals for a mesh." )
{
	m_startingVerticesParameter = new V3iParameter(
	        "startingVertices",
	        "startingVertices description",
	        Imath::V3i( 0, 1, 2 )
	);

	parameters()->addParameter( m_startingVerticesParameter );
}

MeshVertexReorderOp::~MeshVertexReorderOp()
{
}

V3iParameter * MeshVertexReorderOp::startingVerticesParameter()
{
	return m_startingVerticesParameter.get();
}

const V3iParameter * MeshVertexReorderOp::startingVerticesParameter() const
{
	return m_startingVerticesParameter.get();
}

void MeshVertexReorderOp::modifyTypedPrimitive( MeshPrimitive * mesh, const CompoundObject * operands )
{
	PrimitiveVariableMap::const_iterator pvIt = mesh->variables.find( "P" );
	if ( pvIt==mesh->variables.end() || !pvIt->second.data )
	{
		throw InvalidArgumentException( "MeshVertexReorderOp : MeshPrimitive has no \"P\" primitive variable." );
	}

	if ( !mesh->isPrimitiveVariableValid( pvIt->second ) )
	{
		throw InvalidArgumentException( "MeshVertexReorderOp : \"P\" primitive variable is invalid." );
	}

	Imath::V3i faceVtxSrc = m_startingVerticesParameter->getTypedValue();

	MeshAlgo::reorderVertices( mesh, faceVtxSrc[0], faceVtxSrc[1], faceVtxSrc[2] );
}
