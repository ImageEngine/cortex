//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2016, Image Engine Design Inc. All rights reserved.
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

#include "ai.h"

#include "IECore/Exception.h"
#include "IECore/MessageHandler.h"
#include "IECore/MeshPrimitive.h"

#include "IECoreArnold/NodeAlgo.h"
#include "IECoreArnold/ShapeAlgo.h"
#include "IECoreArnold/MeshAlgo.h"

using namespace std;
using namespace IECore;
using namespace IECoreArnold;

//////////////////////////////////////////////////////////////////////////
// Internal utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

AtArray *faceVaryingIndices( const IECore::MeshPrimitive *mesh )
{
	vector<int> ids;
	ids.resize( mesh->variableSize( PrimitiveVariable::FaceVarying ) );
	for( size_t i=0, e=ids.size(); i < e; i++ )
	{
		ids[i] = i;
	}
	return AiArrayConvert( ids.size(), 1, AI_TYPE_INT, (void *)&(ids[0]) );
}

NodeAlgo::ConverterDescription<MeshPrimitive> g_description( MeshAlgo::convert );

} // namespace

//////////////////////////////////////////////////////////////////////////
// Implementation of public API
//////////////////////////////////////////////////////////////////////////

AtNode *MeshAlgo::convert( const IECore::MeshPrimitive *mesh )
{

	// make the result mesh and add topology and points

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

	ShapeAlgo::convertP( mesh, result, "vlist" );

	// set subdivision

	if( mesh->interpolation()=="catmullClark" )
	{
		AiNodeSetStr( result, "subdiv_type", "catclark" );
		AiNodeSetBool( result, "smoothing", true );
	}

	// add uvs

	const FloatVectorData *s = mesh->variableData<FloatVectorData>( "s" );
	const FloatVectorData *t = mesh->variableData<FloatVectorData>( "t" );
	if( s && t )
	{
		PrimitiveVariable::Interpolation sInterpolation = mesh->variables.find( "s" )->second.interpolation;
		PrimitiveVariable::Interpolation tInterpolation = mesh->variables.find( "t" )->second.interpolation;
		if( sInterpolation == tInterpolation )
		{
			if( sInterpolation == PrimitiveVariable::Varying || sInterpolation == PrimitiveVariable::Vertex || sInterpolation == PrimitiveVariable::FaceVarying )
			{
				// interleave the uvs and set them
				vector<float> st;
				st.reserve( s->readable().size() * 2 );
				for( vector<float>::const_iterator sIt = s->readable().begin(), tIt = t->readable().begin(), eIt = s->readable().end(); sIt != eIt; sIt++, tIt++ )
				{
					st.push_back( *sIt );
					st.push_back( *tIt );
				}
				AiNodeSetArray(
					result,
					"uvlist",
					AiArrayConvert( s->readable().size(), 1, AI_TYPE_POINT2, (void *)&(st[0]) )
				);
				// build uv indices
				if( sInterpolation == PrimitiveVariable::FaceVarying )
				{
					AiNodeSetArray(
						result,
						"uvidxs",
						faceVaryingIndices( mesh )
					);
				}
				else
				{
					AiNodeSetArray(
						result,
						"uvidxs",
						AiArrayConvert( vertexIds.size(), 1, AI_TYPE_INT, (void *)&( vertexIds[0] ) )
					);
				}
			}
			else
			{
				msg( Msg::Warning, "ToArnoldMeshConverter::doConversion", "Variables s and t have unsupported interpolation type - not generating uvs." );
			}
		}
		else
		{
			msg( Msg::Warning, "ToArnoldMeshConverter::doConversion", "Variables s and t have different interpolation - not generating uvs." );
		}
	}
	else if( s || t )
	{
		msg( Msg::Warning, "ToArnoldMeshConverter::doConversion", "Only one of s and t available - not generating uvs." );
	}

	/// add normals

	PrimitiveVariableMap::const_iterator nIt = mesh->variables.find( "N" );
	if( nIt != mesh->variables.end() )
	{
		const V3fVectorData *n = runTimeCast<const V3fVectorData>( nIt->second.data.get() );
		if( n )
		{
			PrimitiveVariable::Interpolation nInterpolation = nIt->second.interpolation;
			if( nInterpolation == PrimitiveVariable::Varying || nInterpolation == PrimitiveVariable::Vertex || nInterpolation == PrimitiveVariable::FaceVarying )
			{
				AiNodeSetArray(
					result,
					"nlist",
					AiArrayConvert( n->readable().size(), 1, AI_TYPE_VECTOR, (void *)&( n->readable()[0] ) )
				);
				if( nInterpolation == PrimitiveVariable::FaceVarying )
				{
					AiNodeSetArray(
						result,
						"nidxs",
						faceVaryingIndices( mesh )
					);
				}
				else
				{
					AiNodeSetArray(
						result,
						"nidxs",
						AiArrayConvert( vertexIds.size(), 1, AI_TYPE_INT, (void *)&( vertexIds[0] ) )
					);
				}
				AiNodeSetBool( result, "smoothing", true );
			}
			else
			{
				msg( Msg::Warning, "ToArnoldMeshConverter::doConversion", "Variable \"N\" has unsupported interpolation type - not generating normals." );
			}
		}
		else
		{
			msg( Msg::Warning, "ToArnoldMeshConverter::doConversion", boost::format( "Variable \"N\" has unsupported type \"%s\" (expected V3fVectorData)." ) );
		}
	}

	// add arbitrary user parameters

	const char *ignore[] = { "P", "s", "t", "N", 0 };
	ShapeAlgo::convertPrimitiveVariables( mesh, result, ignore );

	return result;
}
