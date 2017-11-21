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

#include <cassert>

#include "boost/format.hpp"

#include "IECore/DespatchTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECoreScene/MeshPrimitive.h"
#include "IECoreScene/TriangulateOp.h"
#include "IECoreScene/MeshNormalsOp.h"
#include "IECoreScene/FaceVaryingPromotionOp.h"

#include "IECoreGL/ToGLMeshConverter.h"
#include "IECoreGL/MeshPrimitive.h"

using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( ToGLMeshConverter );

ToGLConverter::ConverterDescription<ToGLMeshConverter> ToGLMeshConverter::g_description;

ToGLMeshConverter::ToGLMeshConverter( IECoreScene::ConstMeshPrimitivePtr toConvert )
	:	ToGLConverter( "Converts IECoreScene::MeshPrimitive objects to IECoreGL::MeshPrimitive objects.", IECore::MeshPrimitiveTypeId )
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

	IECoreScene::TriangulateOpPtr op = new IECoreScene::TriangulateOp();
	op->inputParameter()->setValue( mesh );
	op->throwExceptionsParameter()->setTypedValue( false ); // it's better to see something than nothing
	op->copyParameter()->setTypedValue( false );
	op->operate();

	IECoreScene::FaceVaryingPromotionOpPtr faceVaryingOp = new IECoreScene::FaceVaryingPromotionOp;
	faceVaryingOp->inputParameter()->setValue( mesh );
	faceVaryingOp->copyParameter()->setTypedValue( false );
	faceVaryingOp->operate();

	MeshPrimitivePtr glMesh = new MeshPrimitive( mesh->numFaces() );

	for ( IECoreScene::PrimitiveVariableMap::iterator pIt = mesh->variables.begin(); pIt != mesh->variables.end(); ++pIt )
	{
		if( pIt->second.data )
		{
			glMesh->addPrimitiveVariable( pIt->first, pIt->second );
		}
		else
		{
			IECore::msg( IECore::Msg::Warning, "ToGLMeshConverter", boost::format( "No data given for primvar \"%s\"" ) % pIt->first );
		}
	}

	return glMesh;
}
