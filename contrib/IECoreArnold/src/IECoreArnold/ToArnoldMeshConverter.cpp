//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "IECoreArnold/ToArnoldMeshConverter.h"

using namespace IECoreArnold;
using namespace IECore;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( ToArnoldMeshConverter );

ToArnoldMeshConverter::ToArnoldMeshConverter( IECore::MeshPrimitivePtr toConvert )
	:	ToArnoldConverter( "Converts IECore::MeshPrimitives to arnold polymesh nodes", IECore::MeshPrimitive::staticTypeId() )
{
	srcParameter()->setValue( toConvert );
}

ToArnoldMeshConverter::~ToArnoldMeshConverter()
{
}

AtNode *ToArnoldMeshConverter::doConversion( IECore::ConstObjectPtr from, IECore::ConstCompoundObjectPtr operands ) const
{
	const MeshPrimitive *mesh = static_cast<const MeshPrimitive *>( from.get() );

	// make the result mesh and add points
	
	AtNode *result = AiNode( "polymesh" );

	const std::vector<int> verticesPerFace = mesh->verticesPerFace()->readable();
	AiNodeSetArray(
		result,
		"nsides",
		AiArrayConvert( verticesPerFace.size(), 1, AI_TYPE_INT, (void *)&( verticesPerFace[0] ) )
	);

	const std::vector<int> vertexIds = mesh->vertexIds()->readable();
	AiNodeSetArray(
		result,
		"vidxs",
		AiArrayConvert( vertexIds.size(), 1, AI_TYPE_INT, (void *)&( vertexIds[0] ) )
	);
	
	const V3fVectorData *p = mesh->variableData<V3fVectorData>( "P", PrimitiveVariable::Vertex );
	if( !p )
	{
		AiNodeDestroy( result );
		throw Exception( "MeshPrimitive does not have \"P\" primitive variable of interpolation type Vertex." );
	}
	AiNodeSetArray(
		result,
		"vlist",
		AiArrayConvert( p->readable().size(), 1, AI_TYPE_POINT, (void *)&( p->readable()[0] ) )
	);
	
	// set subdivision
	
	if( mesh->interpolation()=="catmullClark" )
	{
		AiNodeSetStr( result, "subdiv_type", "catclark" );
		AiNodeSetBool( result, "smoothing", true );
	}
	
	/// \todo Normals, uvs
	
	return result;
}
