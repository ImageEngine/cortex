//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2012, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/ToGLMeshConverter.h"

#include "IECoreGL/CachedConverter.h"
#include "IECoreGL/MeshPrimitive.h"

#include "IECoreScene/FaceVaryingPromotionOp.h"
#include "IECoreScene/MeshAlgo.h"
#include "IECoreScene/MeshNormalsOp.h"
#include "IECoreScene/MeshPrimitive.h"

#include "IECore/DespatchTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include <cassert>

using namespace IECoreGL;

namespace {

void triangulateMeshIndices(
    const IECoreScene::MeshPrimitive *mesh,
    std::vector<int> &newIds
)
{
	const std::vector<int> &verticesPerFace = mesh->verticesPerFace()->readable();

	newIds.clear();

	int numTris = 0;
	for( auto n : verticesPerFace )
	{
		numTris += n - 2;
	}

	newIds.reserve( numTris * 3 );

	int faceVertexIdStart = 0;
	for( int faceIdx = 0; faceIdx < (int)verticesPerFace.size(); faceIdx++ )
	{
		int numFaceVerts = verticesPerFace[ faceIdx ];

		const int i0 = faceVertexIdStart + 0;

		for( int i = 1; i < numFaceVerts - 1; i++ )
		{
			const int i1 = faceVertexIdStart + i;
			const int i2 = faceVertexIdStart + i + 1;

			/// Store the indices required to rebuild the facevarying primvars
			newIds.push_back( i0 );
			newIds.push_back( i1 );
			newIds.push_back( i2 );
		}

		faceVertexIdStart += numFaceVerts;
	}
}

} // namespace

IE_CORE_DEFINERUNTIMETYPED( ToGLMeshConverter );

ToGLConverter::ConverterDescription<ToGLMeshConverter> ToGLMeshConverter::g_description;

ToGLMeshConverter::ToGLMeshConverter( IECoreScene::ConstMeshPrimitivePtr toConvert )
	:	ToGLConverter( "Converts IECoreScene::MeshPrimitive objects to IECoreGL::MeshPrimitive objects.", IECoreScene::MeshPrimitive::staticTypeId() )
{
	srcParameter()->setValue( boost::const_pointer_cast<IECoreScene::MeshPrimitive>( toConvert ) );
}

ToGLMeshConverter::~ToGLMeshConverter()
{
}

IECore::RunTimeTypedPtr ToGLMeshConverter::doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const
{
	IECoreScene::MeshPrimitivePtr mesh = boost::static_pointer_cast<IECoreScene::MeshPrimitive>( src->copy() ); // safe because the parameter validated it for us

	if( !mesh->variableData<IECore::V3fVectorData>( "P", IECoreScene::PrimitiveVariable::Vertex ) )
	{
		throw IECore::Exception( "Must specify primitive variable \"P\", of type V3fVectorData and interpolation type Vertex." );
	}

	if( mesh->variables.find( "N" )==mesh->variables.end() )
	{
		// \todo - this is a weird place for this - you don't always need normals in order to render a mesh.
		// Pretty wasteful to be running a normalsOp on a mesh if you're using a shader that doesn't need normals.

		// the mesh has no normals - we need to explicitly add some. if it's a polygon
		// mesh (interpolation==linear) then we add per-face normals for a faceted look
		// and if it's a subdivision mesh we add smooth per-vertex normals.
		IECoreScene::MeshNormalsOpPtr normalOp = new IECoreScene::MeshNormalsOp();
		normalOp->inputParameter()->setValue( mesh );
		normalOp->copyParameter()->setTypedValue( false );
		normalOp->interpolationParameter()->setNumericValue(
			mesh->interpolation() == "linear" ? IECoreScene::PrimitiveVariable::Uniform : IECoreScene::PrimitiveVariable::Vertex
		);
		normalOp->operate();
	}

	IECoreScene::FaceVaryingPromotionOpPtr faceVaryingOp = new IECoreScene::FaceVaryingPromotionOp;
	faceVaryingOp->inputParameter()->setValue( mesh );
	faceVaryingOp->copyParameter()->setTypedValue( false );
	faceVaryingOp->operate();

	IECore::IntVectorDataPtr meshIndices = new IECore::IntVectorData();
	triangulateMeshIndices( mesh.get(), IECoreScene::PrimitiveVariable::FaceVarying, meshIndices->writable(), nullptr );

	MeshPrimitivePtr glMesh = new MeshPrimitive( meshIndices );

	for ( IECoreScene::PrimitiveVariableMap::iterator pIt = mesh->variables.begin(); pIt != mesh->variables.end(); ++pIt )
	{
		if( pIt->second.data )
		{
			glMesh->addPrimitiveVariable( pIt->first, pIt->second );
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "ToGLMeshConverter", "No data given for primvar \"{}\"", pIt->first );
		}
	}

	return glMesh;
}
