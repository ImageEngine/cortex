//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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
#include "IECoreScene/private/PrimitiveVariableAlgos.h"

#include "IECore/DespatchTypedData.h"
#include "IECore/DataAlgo.h"

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

//////////////////////////////////////////////////////////////////////////
// Delete Faces
//////////////////////////////////////////////////////////////////////////

namespace
{

void deleteCorners( MeshPrimitive *out, const MeshPrimitive *in, const std::vector<int> &remapping )
{
	const auto &ids = in->cornerIds()->readable();
	if( ids.empty() )
	{
		return;
	}

	const auto &sharpnesses = in->cornerSharpnesses()->readable();

	IntVectorDataPtr outIdData = new IntVectorData;
	auto &outIds = outIdData->writable();

	FloatVectorDataPtr outSharpnessData = new FloatVectorData;
	auto &outSharpnesses = outSharpnessData->writable();

	for( size_t i = 0; i < ids.size(); ++i )
	{
		int id = remapping[ ids[i] ];
		if( id != -1 )
		{
			outIds.push_back( id );
			outSharpnesses.push_back( sharpnesses[i] );
		}
	}

	out->setCorners( outIdData.get(), outSharpnessData.get() );
};

void deleteCreases( MeshPrimitive *out, const MeshPrimitive *in, const std::vector<int> &remapping )
{
	const auto &lengths = in->creaseLengths()->readable();
	if( lengths.empty() )
	{
		return;
	}

	const auto &ids = in->creaseIds()->readable();
	const auto &sharpnesses = in->creaseSharpnesses()->readable();

	IntVectorDataPtr outLengthData = new IntVectorData;
	auto &outLengths = outLengthData->writable();

	IntVectorDataPtr outIdData = new IntVectorData;
	auto &outIds = outIdData->writable();

	FloatVectorDataPtr outSharpnessData = new FloatVectorData;
	auto &outSharpnesses = outSharpnessData->writable();

	int creaseIdOffset = 0;
	for( size_t i = 0; i < lengths.size(); ++i )
	{
		int outLength = 0;
		for( int j = 0; j < lengths[i]; ++j )
		{
			int id = remapping[ ids[creaseIdOffset + j] ];
			if( id != -1 )
			{
				outIds.push_back( id );
				++outLength;
			}

		}

		if( outLength > 0 )
		{
			outLengths.push_back( outLength );
			outSharpnesses.push_back( sharpnesses[i] );
		}

		creaseIdOffset += lengths[i];
	}

	out->setCreases( outLengthData.get(), outIdData.get(), outSharpnessData.get() );
};

template<typename T>
MeshPrimitivePtr deleteFaces( const MeshPrimitive *meshPrimitive, PrimitiveVariable::IndexedView<T> &deleteFlagView, bool invert )
{
	// construct 3 functors for deleting (uniform, vertex & face varying) primvars
	IECoreScene::PrimitiveVariableAlgos::DeleteFlaggedUniformFunctor<T> uniformFunctor( deleteFlagView, invert );
	IECoreScene::PrimitiveVariableAlgos::DeleteFlaggedFaceVaryingFunctor<T> faceVaryingFunctor( deleteFlagView, meshPrimitive->verticesPerFace(), invert );
	IECoreScene::PrimitiveVariableAlgos::DeleteFlaggedVertexFunctor<T> vertexFunctor(
		meshPrimitive->variableSize( PrimitiveVariable::Vertex ), meshPrimitive->vertexIds(), meshPrimitive->verticesPerFace(), deleteFlagView, invert );

	// filter verticesPerFace using DeleteFlaggedUniformFunctor
	const IECore::Data *inputVerticesPerFace = IECore::runTimeCast<const IECore::Data>( meshPrimitive->verticesPerFace() );
	uniformFunctor.setIndices( nullptr );
	IECoreScene::PrimitiveVariableAlgos::IndexedData outputVerticesPerFace = dispatch( inputVerticesPerFace, uniformFunctor );
	IntVectorDataPtr verticesPerFace = IECore::runTimeCast<IECore::IntVectorData>( outputVerticesPerFace.data );

	// filter VertexIds using DeleteFlaggedFaceVaryingFunctor
	const IECore::Data *inputVertexIds = IECore::runTimeCast<const IECore::Data>( meshPrimitive->vertexIds() );
	faceVaryingFunctor.setIndices( nullptr );
	IECoreScene::PrimitiveVariableAlgos::IndexedData outputVertexIds = dispatch( inputVertexIds, faceVaryingFunctor );

	// remap the indices also
	ConstIntVectorDataPtr remappingData = vertexFunctor.getRemapping();
	const std::vector<int> &remapping = remappingData->readable();
	IntVectorDataPtr vertexIdsData = IECore::runTimeCast<IECore::IntVectorData>( outputVertexIds.data );
	IntVectorData::ValueType &vertexIds = vertexIdsData->writable();
	for( size_t i = 0; i < vertexIds.size(); ++i )
	{
		vertexIds[i] = remapping[vertexIds[i]];
	}

	// construct mesh without positions as they'll be set when filtering the primvars
	MeshPrimitivePtr outMeshPrimitive = new MeshPrimitive( verticesPerFace, vertexIdsData, meshPrimitive->interpolation() );

	deleteCorners( outMeshPrimitive.get(), meshPrimitive, remapping );
	deleteCreases( outMeshPrimitive.get(), meshPrimitive, remapping );

	for( PrimitiveVariableMap::const_iterator it = meshPrimitive->variables.begin(), e = meshPrimitive->variables.end(); it != e; ++it )
	{
		switch( it->second.interpolation )
		{
			case PrimitiveVariable::Uniform:
			{
				const IECore::Data *inputData = it->second.data.get();
				uniformFunctor.setIndices( it->second.indices.get() );
				IECoreScene::PrimitiveVariableAlgos::IndexedData outputData = dispatch( inputData, uniformFunctor );
				outMeshPrimitive->variables[it->first] = PrimitiveVariable( it->second.interpolation, outputData.data, outputData.indices );
				break;
			}
			case PrimitiveVariable::Vertex:
			case PrimitiveVariable::Varying:
			{
				const IECore::Data *inputData = it->second.data.get();
				vertexFunctor.setIndices( it->second.indices.get() );
				IECoreScene::PrimitiveVariableAlgos::IndexedData outputData = dispatch( inputData, vertexFunctor );
				outMeshPrimitive->variables[it->first] = PrimitiveVariable( it->second.interpolation, outputData.data, outputData.indices );
				break;
			}

			case PrimitiveVariable::FaceVarying:
			{
				const IECore::Data *inputData = it->second.data.get();
				faceVaryingFunctor.setIndices( it->second.indices.get() );
				IECoreScene::PrimitiveVariableAlgos::IndexedData outputData = dispatch(inputData, faceVaryingFunctor );
				outMeshPrimitive->variables[it->first] = PrimitiveVariable( it->second.interpolation, outputData.data, outputData.indices );
				break;
			}
			case PrimitiveVariable::Constant:
			case PrimitiveVariable::Invalid:
			{
				outMeshPrimitive->variables[it->first] = it->second;
				break;
			}
		}
	}
	return outMeshPrimitive;
}

} // namespace

MeshPrimitivePtr IECoreScene::MeshAlgo::deleteFaces( const MeshPrimitive *meshPrimitive, const PrimitiveVariable &facesToDelete, bool invert )
{

	if( facesToDelete.interpolation != PrimitiveVariable::Uniform )
	{
		throw InvalidArgumentException( "MeshAlgo::deleteFaces requires an Uniform [Int|Bool|Float]VectorData primitiveVariable " );
	}

	const IntVectorData *intDeleteFlagData = runTimeCast<const IntVectorData>( facesToDelete.data.get() );

	if( intDeleteFlagData )
	{
		PrimitiveVariable::IndexedView<int> deleteFlagView( facesToDelete );
		return ::deleteFaces( meshPrimitive, deleteFlagView, invert );
	}

	const BoolVectorData *boolDeleteFlagData = runTimeCast<const BoolVectorData>( facesToDelete.data.get() );

	if( boolDeleteFlagData )
	{
		PrimitiveVariable::IndexedView<bool> deleteFlagView( facesToDelete );
		return ::deleteFaces( meshPrimitive, deleteFlagView, invert );
	}

	const FloatVectorData *floatDeleteFlagData = runTimeCast<const FloatVectorData>( facesToDelete.data.get() );

	if( floatDeleteFlagData )
	{
		PrimitiveVariable::IndexedView<float> deleteFlagView( facesToDelete );
		return ::deleteFaces( meshPrimitive, deleteFlagView, invert );
	}

	throw InvalidArgumentException( "MeshAlgo::deleteFaces requires an Uniform [Int|Bool|Float]VectorData primitiveVariable " );

}
