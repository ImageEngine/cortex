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

#include <algorithm>

#include "boost/algorithm/string/predicate.hpp"

#include "ai.h"

#include "IECore/Exception.h"
#include "IECore/MessageHandler.h"
#include "IECore/MeshPrimitive.h"

#include "IECoreArnold/NodeAlgo.h"
#include "IECoreArnold/ShapeAlgo.h"
#include "IECoreArnold/MeshAlgo.h"
#include "IECoreArnold/ParameterAlgo.h"

using namespace std;
using namespace IECore;
using namespace IECoreArnold;

//////////////////////////////////////////////////////////////////////////
// Internal utilities
//////////////////////////////////////////////////////////////////////////

namespace
{

AtArray *identityIndices( size_t size )
{
	AtArray *result = AiArrayAllocate( size, 1, AI_TYPE_UINT );
	for( size_t i=0; i < size; ++i )
	{
		AiArraySetInt( result, i, i );
	}
	return result;
}

template<typename T>
const T *variableData( const PrimitiveVariableMap &variables, const std::string &name, PrimitiveVariable::Interpolation interpolation = PrimitiveVariable::Invalid )
{
	PrimitiveVariableMap::const_iterator it = variables.find( name );
	if( it==variables.end() )
	{
		return NULL;
	}
	if( interpolation != PrimitiveVariable::Invalid && it->second.interpolation != interpolation )
	{
		return NULL;
	}
	return runTimeCast<T>( it->second.data.get() );
}

void convertUVSet( PrimitiveVariableMap::const_iterator uvIt, const vector<int> &vertexIds, AtNode *node )
{
	const V2fVectorData *uvData = runTimeCast<V2fVectorData>( uvIt->second.data.get() );

	if( !uvData )
	{
		return;
	}

	if( uvIt->second.interpolation != PrimitiveVariable::Varying && uvIt->second.interpolation != PrimitiveVariable::Vertex && uvIt->second.interpolation != PrimitiveVariable::FaceVarying )
	{
		msg(
			Msg::Warning, "ToArnoldMeshConverter::doConversion",
			boost::format( "Variable \"%s\" has an invalid interpolation type - not generating uvs." ) % uvIt->first
		);
		return;
	}

	const vector<Imath::V2f> &uvs = uvData->readable();

	AtArray *uvsArray = AiArrayAllocate( uvs.size(), 1, AI_TYPE_POINT2 );
	for( size_t i = 0, e = uvs.size(); i < e; ++i )
	{
		AtPoint2 uv = { uvs[i][0], uvs[i][1] };
		AiArraySetPnt2( uvsArray, i, uv );
	}

	AtArray *indicesArray = nullptr;
	if( uvIt->second.indices )
	{
		const vector<int> &indices = uvIt->second.indices->readable();
		indicesArray = AiArrayAllocate( indices.size(), 1, AI_TYPE_UINT );
		for( size_t i = 0, e = indices.size(); i < e; ++i )
		{
			AiArraySetUInt( indicesArray, i, indices[i] );
		}
	}
	else if( uvIt->second.interpolation == PrimitiveVariable::FaceVarying )
	{
		indicesArray = identityIndices( vertexIds.size() );
	}
	else
	{
		indicesArray = AiArrayAllocate( vertexIds.size(), 1, AI_TYPE_UINT );
		for( size_t i = 0, e = vertexIds.size(); i < e; ++i )
		{
			AiArraySetUInt( indicesArray, i, vertexIds[i] );
		}
	}

	if( uvIt->first == "uv" )
	{
		AiNodeSetArray( node, "uvlist", uvsArray );
		AiNodeSetArray( node, "uvidxs", indicesArray );
	}
	else
	{
		AiNodeDeclare( node, uvIt->first.c_str(), "indexed POINT2" );
		AiNodeSetArray( node, uvIt->first.c_str(), uvsArray );
		AiNodeSetArray( node, (uvIt->first + "idxs").c_str(), indicesArray );
	}
}

AtNode *convertCommon( const IECore::MeshPrimitive *mesh )
{

	// Make the result mesh and add topology

	AtNode *result = AiNode( "polymesh" );

	const std::vector<int> &verticesPerFace = mesh->verticesPerFace()->readable();
	AiNodeSetArray(
		result,
		"nsides",
		AiArrayConvert( verticesPerFace.size(), 1, AI_TYPE_INT, (void *)&( verticesPerFace[0] ) )
	);

	const std::vector<int> &vertexIds = mesh->vertexIds()->readable();
	AiNodeSetArray(
		result,
		"vidxs",
		AiArrayConvert( vertexIds.size(), 1, AI_TYPE_INT, (void *)&( vertexIds[0] ) )
	);

	// Set subdivision

	if( mesh->interpolation()=="catmullClark" )
	{
		AiNodeSetStr( result, "subdiv_type", "catclark" );
		AiNodeSetBool( result, "smoothing", true );
	}

	// Convert primitive variables. We start with indexed uvs,
	// since those require matching sets of three variables.
	// Each successful conversion removes the used variables
	// so they are not used by the next potential conversion.

	PrimitiveVariableMap variablesToConvert = mesh->variables;
	variablesToConvert.erase( "P" ); // These will be converted
	variablesToConvert.erase( "N" ); // outside of this function.

	// Find all UV sets. We must perform the iteration to find the
	// names separately to the iteration to convert them, because
	// convertUVSet() removes items from variablesToConvert, and
	// would therefore invalidate the interators we were using if
	// we were to do it in one loop.
	vector<PrimitiveVariableMap::iterator> uvSets;
	for( PrimitiveVariableMap::iterator it = variablesToConvert.begin(), eIt = variablesToConvert.end(); it != eIt; ++it )
	{
		/// \todo: add a role enum to PrimitiveVariable, so we can distinguish between UVs and
		///  things that just happen to hold V2fVectorData.
		if( it->second.data->typeId() == V2fVectorDataTypeId )
		{
			uvSets.push_back( it );
		}
	}
	for( auto &it : uvSets )
	{
		::convertUVSet( it, vertexIds, result );
		variablesToConvert.erase( it );
	}

	// Finally, do a generic conversion of anything that remains.
	for( PrimitiveVariableMap::iterator it = variablesToConvert.begin(), eIt = variablesToConvert.end(); it != eIt; ++it )
	{
		ShapeAlgo::convertPrimitiveVariable( mesh, it->second, result, it->first.c_str() );
	}

	return result;
}

const V3fVectorData *normal( const IECore::MeshPrimitive *mesh, PrimitiveVariable::Interpolation &interpolation )
{
	PrimitiveVariableMap::const_iterator it = mesh->variables.find( "N" );
	if( it == mesh->variables.end() )
	{
		return NULL;
	}

	const V3fVectorData *n = runTimeCast<const V3fVectorData>( it->second.data.get() );
	if( !n )
	{
		msg( Msg::Warning, "MeshAlgo", boost::format( "Variable \"N\" has unsupported type \"%s\" (expected V3fVectorData)." ) % it->second.data->typeName() );
		return NULL;
	}

	const PrimitiveVariable::Interpolation thisInterpolation = it->second.interpolation;
	if( interpolation != PrimitiveVariable::Invalid && thisInterpolation != interpolation )
	{
		msg( Msg::Warning, "MeshAlgo", "Variable \"N\" has inconsistent interpolation types - not generating normals." );
		return NULL;
	}

	if( thisInterpolation != PrimitiveVariable::Varying && thisInterpolation != PrimitiveVariable::Vertex && thisInterpolation != PrimitiveVariable::FaceVarying )
	{
		msg( Msg::Warning, "MeshAlgo", "Variable \"N\" has unsupported interpolation type - not generating normals." );
		return NULL;
	}

	interpolation = thisInterpolation;
	return n;
}

void convertNormalIndices( const IECore::MeshPrimitive *mesh, AtNode *node, PrimitiveVariable::Interpolation interpolation )
{
	if( interpolation == PrimitiveVariable::FaceVarying )
	{
		AiNodeSetArray(
			node,
			"nidxs",
			identityIndices( mesh->variableSize( PrimitiveVariable::FaceVarying ) )
		);
	}
	else
	{
		const std::vector<int> &vertexIds = mesh->vertexIds()->readable();
		AiNodeSetArray(
			node,
			"nidxs",
			AiArrayConvert( vertexIds.size(), 1, AI_TYPE_INT, (void *)&( vertexIds[0] ) )
		);
	}
}

NodeAlgo::ConverterDescription<MeshPrimitive> g_description( MeshAlgo::convert, MeshAlgo::convert );

} // namespace

//////////////////////////////////////////////////////////////////////////
// Implementation of public API
//////////////////////////////////////////////////////////////////////////

AtNode *MeshAlgo::convert( const IECore::MeshPrimitive *mesh )
{
	AtNode *result = convertCommon( mesh );

	ShapeAlgo::convertP( mesh, result, "vlist" );

	// add normals

	PrimitiveVariable::Interpolation nInterpolation = PrimitiveVariable::Invalid;
	if( const V3fVectorData *n = normal( mesh, nInterpolation ) )
	{
		AiNodeSetArray(
			result,
			"nlist",
			AiArrayConvert( n->readable().size(), 1, AI_TYPE_VECTOR, &n->readable().front() )
		);
		convertNormalIndices( mesh, result, nInterpolation );
		AiNodeSetBool( result, "smoothing", true );
	}

	return result;
}

AtNode *MeshAlgo::convert( const std::vector<const IECore::MeshPrimitive *> &samples, const std::vector<float> &sampleTimes )
{
	AtNode *result = convertCommon( samples.front() );

	std::vector<const IECore::Primitive *> primitiveSamples( samples.begin(), samples.end() );
	ShapeAlgo::convertP( primitiveSamples, result, "vlist" );

	// add normals

	vector<const Data *> nSamples;
	nSamples.reserve( samples.size() );
	PrimitiveVariable::Interpolation nInterpolation = PrimitiveVariable::Invalid;
	for( vector<const MeshPrimitive *>::const_iterator it = samples.begin(), eIt = samples.end(); it != eIt; ++it )
	{
		if( const V3fVectorData *n = normal( *it, nInterpolation ) )
		{
			nSamples.push_back( n );
		}
		else
		{
			break;
		}
	}

	if( nSamples.size() == samples.size() )
	{
		AiNodeSetArray(
			result,
			"nlist",
			ParameterAlgo::dataToArray( nSamples, AI_TYPE_VECTOR )
		);
		convertNormalIndices( samples.front(), result, nInterpolation );
		AiNodeSetBool( result, "smoothing", true );
	}

	// add time sampling

	AiNodeSetArray( result, "deform_time_samples", AiArrayConvert( sampleTimes.size(), 1, AI_TYPE_FLOAT, &sampleTimes.front() ) );

	return result;
}

