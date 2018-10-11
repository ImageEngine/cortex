//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/TriangulateOp.h"

#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/MeshAlgo.h"

#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"

using namespace IECore;
using namespace IECoreScene;

IE_CORE_DEFINERUNTIMETYPED( TriangulateOp );

TriangulateOp::TriangulateOp() : MeshPrimitiveOp( "A MeshPrimitiveOp to triangulate a mesh" )
{
	m_toleranceParameter = new FloatParameter(
		"tolerance",
		"The floating point tolerance to use for various operations, such as determining planarity of faces",
		1.e-6f,
		0.0f
	);

	m_throwExceptionsParameter = new BoolParameter(
		"throwExceptions",
		"When enabled, exceptions are thrown when invalid geometry is encountered (e.g. non-planar or concave faces).",
		true
	);


	parameters()->addParameter( m_toleranceParameter );
	parameters()->addParameter( m_throwExceptionsParameter );
}

TriangulateOp::~TriangulateOp()
{
}

FloatParameter * TriangulateOp::toleranceParameter()
{
	return m_toleranceParameter.get();
}

const FloatParameter * TriangulateOp::toleranceParameter() const
{
	return m_toleranceParameter.get();
}

BoolParameter * TriangulateOp::throwExceptionsParameter()
{
	return m_throwExceptionsParameter.get();
}

const BoolParameter * TriangulateOp::throwExceptionsParameter() const
{
	return m_throwExceptionsParameter.get();
}

void TriangulateOp::modifyTypedPrimitive( MeshPrimitive * mesh, const CompoundObject * operands )
{
	// already triangulated
	if ( mesh->maxVerticesPerFace() == 3 )
	{
		return;
	}

	const float tolerance = toleranceParameter()->getNumericValue();
	bool throwExceptions = static_cast<const BoolData *>(throwExceptionsParameter()->getValue())->readable();

	IECoreScene::MeshPrimitivePtr triangulatedMesh = IECoreScene::MeshAlgo::triangulate( mesh, tolerance, throwExceptions );

	if ( triangulatedMesh  )
	{
		// todo replace with setTopologyUnchecked
		mesh->setTopology( triangulatedMesh->verticesPerFace(), triangulatedMesh->vertexIds(), triangulatedMesh->interpolation() );
		mesh->variables = triangulatedMesh->variables;
	}
}
