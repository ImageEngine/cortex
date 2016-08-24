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

// Cortex represents UV sets horribly, with the u and v stored as separate primitive variables.
// RenderMan's FaceVarying data format is also horrible for UV sets, because it throws away all
// connectivity information, duplicating UVs so that each face vertex has its own value. To work
// around this, Cortex stores a third variable holding the indices into the original data, so we
// have a record of the connectivity. This is all done by convention, and because that's what the
// FromMayaMeshConverter has evolved to do.
//
// All this plays poorly with Arnold, because it stores UVs sanely, as an array of 2d vectors with
// an indices array to index into them per face-vertex. So this function converts the Cortex
// representation into the Arnold form, so that Arnold can use the connectivity to correctly
// interpolate the uvs.
//
// We should really improve the Cortex representation as follows :
//
// - Use V2fVectorData for UVs.
// - Add an optional IntVectorData indices array into the PrimitiveVariable class to store
//   indices into the UV array, and associating the indices directly with the data.
// - Use the above to avoid the duplication of UVs when storing FaceVarying UVs, so we can
//   do a straightforward conversion to Arnold, and only do the dumb duplication on conversion
///  to RenderMan.
// - Perhaps also add a role enum to PrimitiveVariable, so we can distinguish between UVs and
///  things that just happen to hold V2fVectorData.
void convertIndexedUVSet( const std::string &setName, PrimitiveVariableMap &variables, AtNode *node )
{
	const std::string sName = setName == "" ? "s" : setName + "_s";
	const std::string tName = setName == "" ? "t" : setName + "_t";
	const std::string indicesName = setName == "" ? "stIndices" : setName + "Indices";

	const FloatVectorData *sData = variableData<FloatVectorData>( variables, sName, PrimitiveVariable::FaceVarying );
	const FloatVectorData *tData = variableData<FloatVectorData>( variables, tName, PrimitiveVariable::FaceVarying );
	const IntVectorData *indicesData = variableData<IntVectorData>( variables, indicesName, PrimitiveVariable::FaceVarying );

	if( !( sData && tData && indicesData ) )
	{
		return;
	}

	const vector<float> &s = sData->readable();
	const vector<float> &t = tData->readable();
	const vector<int> &indices = indicesData->readable();

	int numUVs = 1 + *max_element( indices.begin(), indices.end() );
	AtArray *uvsArray = AiArrayAllocate( numUVs, 1, AI_TYPE_POINT2 );
	AtArray *indicesArray = AiArrayAllocate( indices.size(), 1, AI_TYPE_UINT );

	for( size_t i = 0, e = indices.size(); i < e; ++i )
	{
		AtPoint2 uv = { s[i], 1.0f - t[i] };
		AiArraySetPnt2( uvsArray, indices[i], uv );
		AiArraySetUInt( indicesArray, i, indices[i] );
	}

	if( setName == "" )
	{
		AiNodeSetArray( node, "uvlist", uvsArray );
		AiNodeSetArray( node, "uvidxs", indicesArray );
	}
	else
	{
		AiNodeDeclare( node, setName.c_str(), "indexed POINT2" );
		AiNodeSetArray( node, setName.c_str(), uvsArray );
		AiNodeSetArray( node, (setName + "idxs").c_str(), indicesArray );
	}

	variables.erase( sName );
	variables.erase( tName );
	variables.erase( indicesName );
}

// Version of the above for when we have no indices available.
void convertUVSet( const std::string &setName, PrimitiveVariableMap &variables, const vector<int> &vertexIds, AtNode *node )
{
	const std::string sName = setName == "" ? "s" : setName + "_s";
	const std::string tName = setName == "" ? "t" : setName + "_t";

	const FloatVectorData *sData = variableData<FloatVectorData>( variables, sName );
	const FloatVectorData *tData = variableData<FloatVectorData>( variables, tName );

	if( !( sData && tData ) )
	{
		return;
	}

	PrimitiveVariable::Interpolation sInterpolation = variables.find( "s" )->second.interpolation;
	PrimitiveVariable::Interpolation tInterpolation = variables.find( "t" )->second.interpolation;
	if( sInterpolation != tInterpolation )
	{
		msg(
			Msg::Warning, "ToArnoldMeshConverter::doConversion",
			boost::format( "Variables \"%s\" and \"%s\" have different interpolation - not generating uvs." ) % sName % tName
		);
		return;
	}

	if( sInterpolation != PrimitiveVariable::Varying && sInterpolation != PrimitiveVariable::Vertex && sInterpolation != PrimitiveVariable::FaceVarying )
	{
		msg(
			Msg::Warning, "ToArnoldMeshConverter::doConversion",
			boost::format( "Variables \"%s\" and \"%s\" have different interpolation types - not generating uvs." ) % sName % tName
		);
		return;
	}

	const vector<float> &s = sData->readable();
	const vector<float> &t = tData->readable();

	AtArray *uvsArray = AiArrayAllocate( s.size(), 1, AI_TYPE_POINT2 );
	for( size_t i = 0, e = s.size(); i < e; ++i )
	{
		AtPoint2 uv = { s[i], 1.0f - t[i] };
		AiArraySetPnt2( uvsArray, i, uv );
	}

	AtArray *indicesArray = NULL;
	if( sInterpolation == PrimitiveVariable::FaceVarying )
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

	if( setName == "" )
	{
		AiNodeSetArray( node, "uvlist", uvsArray );
		AiNodeSetArray( node, "uvidxs", indicesArray );
	}
	else
	{
		AiNodeDeclare( node, setName.c_str(), "indexed POINT2" );
		AiNodeSetArray( node, setName.c_str(), uvsArray );
		AiNodeSetArray( node, (setName + "idxs").c_str(), indicesArray );
	}

	variables.erase( sName );
	variables.erase( tName );
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

	// Convert and remove indexed UVs first. We must perform
	// the iteration to find the names separately to the iteration
	// to convert them, because convertIndexedUVSet() removes items
	// from variablesToConvert, and would therefore invalidate
	// the interators we were using if we were to do it in one loop.
	vector<string> uvSetNames;
	for( PrimitiveVariableMap::iterator it = variablesToConvert.begin(), eIt = variablesToConvert.end(); it != eIt; ++it )
	{
		if( boost::ends_with( it->first, "Indices" ) )
		{
			uvSetNames.push_back( it->first == "stIndices" ? "" : it->first.substr( 0, it->first.size() - 7 ) );
		}
	}
	for( vector<string>::const_iterator it = uvSetNames.begin(), eIt = uvSetNames.end(); it != eIt; ++it )
	{
		convertIndexedUVSet( *it, variablesToConvert, result );
	}

	// Then convert and remove non-indexed uvs. As above, we must
	// do this in two phases.
	uvSetNames.clear();
	for( PrimitiveVariableMap::iterator it = variablesToConvert.begin(), eIt = variablesToConvert.end(); it != eIt; ++it )
	{
		if( it->first == "s" || boost::ends_with( it->first, "_s" ) )
		{
			uvSetNames.push_back( it->first == "s" ? "" : it->first.substr( 0, it->first.size() - 2 ) );
		}
	}
	for( vector<string>::const_iterator it = uvSetNames.begin(), eIt = uvSetNames.end(); it != eIt; ++it )
	{
		convertUVSet( *it, variablesToConvert, vertexIds, result );
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

