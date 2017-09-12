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

#include <assert.h>
#include <algorithm>

#include "boost/mpl/and.hpp"
#include "OpenEXR/ImathVec.h"

#include "IECore/DespatchTypedData.h"
#include "IECore/MeshAlgo.h"
#include "IECore/FaceVaryingPromotionOp.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TypeTraits.h"
#include "IECore/PolygonIterator.h"
#include "IECore/private/PrimitiveAlgoUtils.h"

using namespace IECore;
using namespace Imath;

//////////////////////////////////////////////////////////////////////////
// Delete Faces
//////////////////////////////////////////////////////////////////////////

namespace
{

template< typename U>
class DeleteFlaggedUniformFunctor
{
	public:
		typedef DataPtr ReturnType;

		DeleteFlaggedUniformFunctor( typename IECore::TypedData<std::vector<U> >::ConstPtr flagData ) : m_flagData( flagData )
		{
		}

		template<typename T>
		ReturnType operator()( const T *data )
		{
			const typename T::ValueType &inputs = data->readable();
			const std::vector<U> &flags = m_flagData->readable();

			T *filteredResultData = new T();
			ReturnType result(filteredResultData);

			typename T::ValueType &filteredResult = filteredResultData->writable();

			filteredResult.reserve( inputs.size() );

			for( size_t i = 0; i < inputs.size(); ++i )
			{
				if( !flags[i] )
				{
					filteredResult.push_back( inputs[i] );
				}
			}

			return result;
		}

	private:
		typename IECore::TypedData<std::vector<U> >::ConstPtr m_flagData;
};

template<typename U>
class DeleteFlaggedFaceVaryingFunctor
{
	public:
		typedef DataPtr ReturnType;

		DeleteFlaggedFaceVaryingFunctor( typename IECore::TypedData<std::vector<U> >::ConstPtr flagData, ConstIntVectorDataPtr verticesPerFaceData ) : m_flagData( flagData ), m_verticesPerFaceData( verticesPerFaceData )
		{
		}

		template<typename T>
		ReturnType operator()( const T *data )
		{
			const typename T::ValueType &inputs = data->readable();
			const std::vector<int> &verticesPerFace = m_verticesPerFaceData->readable();
			const std::vector<U> &flags = m_flagData->readable();

			T *filteredResultData = new T();
			typename T::ValueType &filteredResult = filteredResultData->writable();

			filteredResult.reserve( inputs.size() );

			size_t offset = 0;
			for( size_t f = 0; f < verticesPerFace.size(); ++f )
			{
				int numVerts = verticesPerFace[f];
				if( !flags[f] )
				{
					for( int v = 0; v < numVerts; ++v )
					{
						filteredResult.push_back( inputs[offset + v] );
					}
				}
				offset += numVerts;
			}

			return filteredResultData;
		}

	private:
		typename IECore::TypedData<std::vector<U> >::ConstPtr m_flagData;
		ConstIntVectorDataPtr m_verticesPerFaceData;

};

template<typename U>
class DeleteFlaggedVertexFunctor
{
	public:
		typedef DataPtr ReturnType;

		DeleteFlaggedVertexFunctor( size_t maxVertexId, ConstIntVectorDataPtr vertexIdsData, ConstIntVectorDataPtr verticesPerFaceData, typename IECore::TypedData<std::vector<U> >::ConstPtr flagData )
			: m_flagData( flagData ), m_verticesPerFaceData( verticesPerFaceData ), m_vertexIdsData( vertexIdsData )
		{
			const std::vector<int> &vertexIds = m_vertexIdsData->readable();
			const std::vector<int> &verticesPerFace = m_verticesPerFaceData->readable();
			const std::vector<U> &flags = m_flagData->readable();

			m_usedVerticesData = new BoolVectorData();
			std::vector<bool> &usedVertices = m_usedVerticesData->writable();

			usedVertices.resize( maxVertexId, false );

			size_t offset = 0;
			for( size_t f = 0; f < verticesPerFace.size(); ++f )
			{
				int numVerts = verticesPerFace[f];
				if( !flags[f] )
				{
					for( int v = 0; v < numVerts; ++v )
					{
						usedVertices[vertexIds[offset + v]] = true;
					}
				}
				offset += numVerts;
			}


			m_remappingData = new IntVectorData();
			std::vector<int> &remapping = m_remappingData->writable();

			// again this array is too large but large enough
			remapping.resize(maxVertexId, -1 );

			size_t newIndex = 0;
			for (size_t i = 0; i < usedVertices.size(); ++i)
			{
				if( usedVertices[i] )
				{
					remapping[i] = newIndex;
					newIndex++;
				}
			}
		}

		template<typename T>
		ReturnType operator()( const T *data )
		{
			const std::vector<bool> &usedVertices = m_usedVerticesData->readable();

			const typename T::ValueType &vertices = data->readable();

			T *filteredVerticesData = new T();
			typename T::ValueType &filteredVertices = filteredVerticesData->writable();

			filteredVertices.reserve( vertices.size() );

			for( size_t v = 0; v < vertices.size(); ++v )
			{
				if( usedVertices[v] )
				{
					filteredVertices.push_back( vertices[v] );
				}
			};

			return filteredVerticesData;
		}

		ConstIntVectorDataPtr getRemapping() const { return m_remappingData; }

	private:
		typename IECore::TypedData<std::vector<U> >::ConstPtr m_flagData;
		ConstIntVectorDataPtr m_verticesPerFaceData;
		ConstIntVectorDataPtr m_vertexIdsData;

		BoolVectorDataPtr m_usedVerticesData;

		// map from old vertex index to new
		IntVectorDataPtr m_remappingData;
};

template<typename T>
MeshPrimitivePtr deleteFaces( const MeshPrimitive* meshPrimitive, const typename IECore::TypedData<std::vector<T> > *deleteFlagData)
{
	// construct 3 functors for deleting (uniform, vertex & face varying) primvars
	DeleteFlaggedUniformFunctor<T> uniformFunctor(deleteFlagData);
	DeleteFlaggedFaceVaryingFunctor<T> faceVaryingFunctor(deleteFlagData, meshPrimitive->verticesPerFace());
	DeleteFlaggedVertexFunctor<T> vertexFunctor( meshPrimitive->variableSize(PrimitiveVariable::Vertex), meshPrimitive->vertexIds(), meshPrimitive->verticesPerFace(), deleteFlagData );

	// filter verticesPerFace using DeleteFlaggedUniformFunctor
	IECore::Data *inputVerticesPerFace = const_cast< IECore::Data * >( IECore::runTimeCast<const IECore::Data>( meshPrimitive->verticesPerFace() ) );
	IECore::DataPtr outputVerticesPerFace = despatchTypedData<DeleteFlaggedUniformFunctor<T>, TypeTraits::IsVectorTypedData>( inputVerticesPerFace, uniformFunctor );
	IntVectorDataPtr verticesPerFace = IECore::runTimeCast<IECore::IntVectorData>(outputVerticesPerFace);

	// filter VertexIds using DeleteFlaggedFaceVaryingFunctor
	IECore::Data *inputVertexIds = const_cast< IECore::Data * >( IECore::runTimeCast<const IECore::Data>( meshPrimitive->vertexIds() ) );
	IECore::DataPtr outputVertexIds = despatchTypedData<DeleteFlaggedFaceVaryingFunctor<T>, TypeTraits::IsVectorTypedData>( inputVertexIds, faceVaryingFunctor );

	// remap the indices also
	ConstIntVectorDataPtr remappingData = vertexFunctor.getRemapping();
	const std::vector<int>& remapping = remappingData->readable();

	IntVectorDataPtr vertexIdsData = IECore::runTimeCast<IECore::IntVectorData>(outputVertexIds);
	IntVectorData::ValueType& vertexIds = vertexIdsData->writable();

	for (size_t i = 0; i < vertexIds.size(); ++i)
	{
		vertexIds[i] = remapping[vertexIds[i]];
	}

	// construct mesh without positions as they'll be set when filtering the primvars
	MeshPrimitivePtr outMeshPrimitive = new MeshPrimitive(verticesPerFace, vertexIdsData, meshPrimitive->interpolation());

	for (PrimitiveVariableMap::const_iterator it = meshPrimitive->variables.begin(), e = meshPrimitive->variables.end(); it != e; ++it)
	{
		switch(it->second.interpolation)
		{
			case PrimitiveVariable::Uniform:
			{
				IECore::Data *inputData = const_cast< IECore::Data * >( it->second.data.get() );
				IECore::DataPtr outputData = despatchTypedData<DeleteFlaggedUniformFunctor<T>, TypeTraits::IsVectorTypedData>( inputData, uniformFunctor );
				outMeshPrimitive->variables[it->first] = PrimitiveVariable( it->second.interpolation, outputData );
				break;
			}
			case PrimitiveVariable::Vertex:
			case PrimitiveVariable::Varying:
			{
				IECore::Data *inputData = const_cast< IECore::Data * >( it->second.data.get() );
				IECore::DataPtr ouptputData = despatchTypedData<DeleteFlaggedVertexFunctor<T>, TypeTraits::IsVectorTypedData>( inputData, vertexFunctor );
				outMeshPrimitive->variables[it->first] = PrimitiveVariable( it->second.interpolation, ouptputData );
				break;
			}

			case PrimitiveVariable::FaceVarying:
			{
				IECore::Data *inputData = const_cast< IECore::Data * >( it->second.data.get() );
				IECore::DataPtr outputData = despatchTypedData<DeleteFlaggedFaceVaryingFunctor<T>, TypeTraits::IsVectorTypedData>( inputData, faceVaryingFunctor );
				outMeshPrimitive->variables[it->first] = PrimitiveVariable ( it->second.interpolation, outputData);
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

MeshPrimitivePtr IECore::MeshAlgo::deleteFaces( const MeshPrimitive *meshPrimitive, const PrimitiveVariable& facesToDelete )
{

	if( facesToDelete.interpolation != PrimitiveVariable::Uniform )
	{
		throw InvalidArgumentException( "MeshAlgo::deleteFaces requires an Uniform [Int|Bool|Float]VectorData primitiveVariable " );
	}

	const IntVectorData *intDeleteFlagData = runTimeCast<const IntVectorData>( facesToDelete.data.get() );

	if( intDeleteFlagData )
	{
		return ::deleteFaces( meshPrimitive, intDeleteFlagData );
	}

	const BoolVectorData *boolDeleteFlagData = runTimeCast<const BoolVectorData>( facesToDelete.data.get() );

	if( boolDeleteFlagData )
	{
		return ::deleteFaces( meshPrimitive, boolDeleteFlagData );
	}

	const FloatVectorData *floatDeleteFlagData = runTimeCast<const FloatVectorData>( facesToDelete.data.get() );

	if( floatDeleteFlagData )
	{
		return ::deleteFaces( meshPrimitive, floatDeleteFlagData );
	}

	throw InvalidArgumentException( "MeshAlgo::deleteFaces requires an Uniform [Int|Bool|Float]VectorData primitiveVariable " );

}

//////////////////////////////////////////////////////////////////////////
// Resample Primitive Variables
//////////////////////////////////////////////////////////////////////////

namespace
{

struct MeshVertexToUniform
{
	typedef DataPtr ReturnType;

	MeshVertexToUniform( const MeshPrimitive *mesh )	:	m_mesh( mesh )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		const typename From::ValueType &src = data->readable();

		// TODO sipmlify this code by using polygon face & vertex iterators
		trg.reserve( m_mesh->numFaces() );

		std::vector<int>::const_iterator vId = m_mesh->vertexIds()->readable().begin();
		const std::vector<int> &verticesPerFace = m_mesh->verticesPerFace()->readable();
		for( std::vector<int>::const_iterator it = verticesPerFace.begin(); it < verticesPerFace.end(); ++it )
		{
			// initialize with the first value to avoid
			// ambiguitity during default construction
			typename From::ValueType::value_type total = src[ *vId ];
			++vId;

			for( int j = 1; j < *it; ++j, ++vId )
			{
				total += src[ *vId ];
			}

			trg.push_back( total / *it );
		}

		return result;
	}

	const MeshPrimitive *m_mesh;
};

struct MeshUniformToVertex
{
	typedef DataPtr ReturnType;

	MeshUniformToVertex( const MeshPrimitive *mesh )	:	m_mesh( mesh )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		const typename From::ValueType &src = data->readable();

		size_t numVerts = m_mesh->variableSize( PrimitiveVariable::Vertex );
		std::vector<int> count( numVerts, 0 );
		trg.resize( numVerts );

		typename From::ValueType::const_iterator srcIt = src.begin();
		std::vector<int>::const_iterator vId = m_mesh->vertexIds()->readable().begin();
		const std::vector<int> &verticesPerFace = m_mesh->verticesPerFace()->readable();
		for( std::vector<int>::const_iterator it = verticesPerFace.begin(); it < verticesPerFace.end(); ++it, ++srcIt )
		{
			for( int j = 0; j < *it; ++j, ++vId )
			{
				trg[ *vId ] += *srcIt;
				++count[ *vId ];
			}
		}

		std::vector<int>::const_iterator cIt = count.begin();
		typename From::ValueType::iterator trgIt = trg.begin(), trgEnd = trg.end();
		for( trgIt = trg.begin(); trgIt != trgEnd ; ++trgIt, ++cIt )
		{
			*trgIt /= *cIt;
		}

		return result;
	}

	const MeshPrimitive *m_mesh;
};

struct MeshFaceVaryingToVertex
{
	typedef DataPtr ReturnType;

	MeshFaceVaryingToVertex( const MeshPrimitive *mesh )	:	m_mesh( mesh )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		const typename From::ValueType &src = data->readable();

		size_t numVerts = m_mesh->variableSize( PrimitiveVariable::Vertex );
		std::vector<int> count( numVerts, 0 );
		trg.resize( numVerts );

		const std::vector<int>& vertexIds = m_mesh->vertexIds()->readable();
		std::vector<int>::const_iterator vertexIdIt = vertexIds.begin();
		typename From::ValueType::const_iterator srcIt = src.begin(), srcEnd = src.end();

		for( ; srcIt != srcEnd ; ++srcIt, ++vertexIdIt )
		{
			trg[ *vertexIdIt ] += *srcIt;
			++count[ *vertexIdIt ];
		}

		std::vector<int>::const_iterator cIt = count.begin();
		typename From::ValueType::iterator trgIt = trg.begin(), trgEnd = trg.end();
		for( trgIt = trg.begin(); trgIt != trgEnd ; ++trgIt, ++cIt )
		{
			*trgIt /= *cIt;
		}

		return result;
	}

	const MeshPrimitive *m_mesh;
};

struct MeshFaceVaryingToUniform
{
	typedef DataPtr ReturnType;

	MeshFaceVaryingToUniform( const MeshPrimitive *mesh )	:	m_mesh( mesh )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{
		typename From::Ptr result = static_cast< From* >( Object::create( data->typeId() ).get() );
		typename From::ValueType &trg = result->writable();
		const typename From::ValueType &src = data->readable();

		trg.reserve( m_mesh->numFaces() );

		typename From::ValueType::const_iterator srcIt = src.begin();

		const std::vector<int> &verticesPerFace = m_mesh->verticesPerFace()->readable();
		for( std::vector<int>::const_iterator it = verticesPerFace.begin(); it < verticesPerFace.end(); ++it )
		{
			// initialize with the first value to avoid
			// ambiguity during default construction
			typename From::ValueType::value_type total = *srcIt;
			++srcIt;

			for( int j = 1; j < *it; ++j, ++srcIt )
			{
				total += *srcIt;
			}

			trg.push_back( total / *it );
		}

		return result;
	}

	const MeshPrimitive *m_mesh;
};

struct MeshAnythingToFaceVarying
{
	typedef DataPtr ReturnType;

	MeshAnythingToFaceVarying( const MeshPrimitive *mesh, PrimitiveVariable::Interpolation srcInterpolation )
		: m_mesh( mesh ), m_srcInterpolation( srcInterpolation )
	{
	}

	template<typename From> ReturnType operator()( const From* data )
	{

		// TODO replace the call to the IECore::FaceVaryingPromotionOpPtr and include the logic in this file.

		// we need to duplicate because the Op expects a primvar to manipulate..
		IECore::MeshPrimitivePtr tmpMesh = m_mesh->copy();
		// cast OK due to read-only access.
		tmpMesh->variables["tmpPrimVar"] = IECore::PrimitiveVariable( m_srcInterpolation, const_cast< From * >(data) );
		IECore::FaceVaryingPromotionOpPtr promoteOp = new IECore::FaceVaryingPromotionOp();
		promoteOp->inputParameter()->setValue( tmpMesh );
		IECore::StringVectorDataPtr names = new StringVectorData();
		names->writable().push_back( "tmpPrimVar" );
		promoteOp->primVarNamesParameter()->setValue( names );
		ReturnType result = runTimeCast< MeshPrimitive >( promoteOp->operate() )->variables["tmpPrimVar"].data;


		return result;
	}

	const MeshPrimitive *m_mesh;
	PrimitiveVariable::Interpolation m_srcInterpolation;
};

} // namespace

void IECore::MeshAlgo::resamplePrimitiveVariable( const MeshPrimitive *mesh, PrimitiveVariable& primitiveVariable, PrimitiveVariable::Interpolation interpolation )
{
	Data *srcData = primitiveVariable.data.get();
	DataPtr dstData;

	PrimitiveVariable::Interpolation srcInterpolation = primitiveVariable.interpolation;

	if ( srcInterpolation == interpolation )
	{
		return;
	}

	// average array to single value
	if ( interpolation == PrimitiveVariable::Constant )
	{
		Detail::AverageValueFromVector fn;
		dstData = despatchTypedData<Detail::AverageValueFromVector, Detail::IsArithmeticVectorTypedData>( srcData, fn );
		primitiveVariable = PrimitiveVariable( interpolation, dstData );
		return;
	}

	if ( primitiveVariable.interpolation == PrimitiveVariable::Constant )
	{
		DataPtr arrayData = Detail::createArrayData(primitiveVariable, mesh, interpolation);
		if (arrayData)
		{
			primitiveVariable = PrimitiveVariable(interpolation, arrayData);
		}
		return;
	}

	if( interpolation == PrimitiveVariable::Uniform )
	{
		if( srcInterpolation == PrimitiveVariable::Varying || srcInterpolation == PrimitiveVariable::Vertex )
		{
			MeshVertexToUniform fn( mesh );
			dstData = despatchTypedData<MeshVertexToUniform, Detail::IsArithmeticVectorTypedData>( srcData, fn );
		}
		else if( srcInterpolation == PrimitiveVariable::FaceVarying )
		{
			MeshFaceVaryingToUniform fn( mesh );
			dstData = despatchTypedData<MeshFaceVaryingToUniform, Detail::IsArithmeticVectorTypedData>( srcData, fn );
		}
	}
	else if( interpolation == PrimitiveVariable::Varying || interpolation == PrimitiveVariable::Vertex )
	{
		if( srcInterpolation == PrimitiveVariable::Uniform )
		{
			MeshUniformToVertex fn( mesh );
			dstData = despatchTypedData<MeshUniformToVertex, Detail::IsArithmeticVectorTypedData>( srcData, fn );
		}
		else if( srcInterpolation == PrimitiveVariable::FaceVarying )
		{
			MeshFaceVaryingToVertex fn( mesh );
			dstData = despatchTypedData<MeshFaceVaryingToVertex, Detail::IsArithmeticVectorTypedData>( srcData, fn );
		}
		else if( srcInterpolation == PrimitiveVariable::Varying || srcInterpolation == PrimitiveVariable::Vertex )
		{
			dstData = srcData;
		}
	}
	else if( interpolation == PrimitiveVariable::FaceVarying )
	{
		MeshAnythingToFaceVarying fn( mesh, srcInterpolation );
		dstData = despatchTypedData<MeshAnythingToFaceVarying, Detail::IsArithmeticVectorTypedData>( srcData, fn );
	}

	primitiveVariable = PrimitiveVariable( interpolation, dstData );
}

//////////////////////////////////////////////////////////////////////////
// Calculate tangents
//////////////////////////////////////////////////////////////////////////

std::pair<PrimitiveVariable, PrimitiveVariable> IECore::MeshAlgo::calculateTangents(
	const MeshPrimitive *mesh,
	const std::string &uvSet, /* = "uv" */
	bool orthoTangents, /* = true */
	const std::string &position /* = "P" */
)
{
	if( mesh->minVerticesPerFace() != 3 || mesh->maxVerticesPerFace() != 3 )
	{
		throw InvalidArgumentException( "MeshAlgo::calculateTangents : MeshPrimitive must only contain triangles" );
	}

	const V3fVectorData *positionData = mesh->variableData<V3fVectorData>( position );
	if( !positionData )
	{
		std::string e = boost::str( boost::format( "MeshAlgo::calculateTangents : MeshPrimitive has no Vertex \"%s\" primitive variable." ) % position );
		throw InvalidArgumentException( e );
	}

	const V3fVectorData::ValueType &points = positionData->readable();

	const IntVectorData *vertsPerFaceData = mesh->verticesPerFace();
	const IntVectorData::ValueType &vertsPerFace = vertsPerFaceData->readable();

	const IntVectorData *vertIdsData = mesh->vertexIds();
	const IntVectorData::ValueType &vertIds = vertIdsData->readable();

	const auto uvIt = mesh->variables.find( uvSet );
	if( uvIt == mesh->variables.end() || uvIt->second.interpolation != PrimitiveVariable::FaceVarying || uvIt->second.data->typeId() != V2fVectorDataTypeId )
	{
		throw InvalidArgumentException( ( boost::format( "MeshAlgo::calculateTangents : MeshPrimitive has no FaceVarying V2fVectorData primitive variable named \"%s\"."  ) % ( uvSet ) ).str() );
	}

	const V2fVectorData *uvData = runTimeCast<V2fVectorData>( uvIt->second.data.get() );
	const V2fVectorData::ValueType &uvs = uvData->readable();

	// I'm a little unsure about using the vertIds as a fallback for the stIndices.
	const IntVectorData::ValueType &uvIndices = uvIt->second.indices ? uvIt->second.indices->readable() : vertIds;

	size_t numUVs = uvs.size();

	std::vector<V3f> uTangents( numUVs, V3f( 0 ) );
	std::vector<V3f> vTangents( numUVs, V3f( 0 ) );
	std::vector<V3f> normals( numUVs, V3f( 0 ) );

	for( size_t faceIndex = 0; faceIndex < vertsPerFace.size(); faceIndex++ )
	{
		assert( vertsPerFace[faceIndex] == 3 );

		// indices into the facevarying data for this face
		size_t fvi0 = faceIndex * 3;
		size_t fvi1 = fvi0 + 1;
		size_t fvi2 = fvi1 + 1;
		assert( fvi2 < vertIds.size() );
		assert( fvi2 < uvIndices.size() );

		// positions for each vertex of this face
		const V3f &p0 = points[vertIds[fvi0]];
		const V3f &p1 = points[vertIds[fvi1]];
		const V3f &p2 = points[vertIds[fvi2]];

		// uv coordinates for each vertex of this face
		const V2f &uv0 = uvs[uvIndices[fvi0]];
		const V2f &uv1 = uvs[uvIndices[fvi1]];
		const V2f &uv2 = uvs[uvIndices[fvi2]];

		// compute tangents and normal for this face
		const V3f e0 = p1 - p0;
		const V3f e1 = p2 - p0;

		const V2f e0uv = uv1 - uv0;
		const V2f e1uv = uv2 - uv0;

		V3f tangent = ( e0 * -e1uv.y + e1 * e0uv.y ).normalized();
		V3f bitangent = ( e0 * -e1uv.x + e1 * e0uv.x ).normalized();

		V3f normal = ( p2 - p1 ).cross( p0 - p1 );
		normal.normalize();

		// and accumlate them into the computation so far
		uTangents[uvIndices[fvi0]] += tangent;
		uTangents[uvIndices[fvi1]] += tangent;
		uTangents[uvIndices[fvi2]] += tangent;

		vTangents[uvIndices[fvi0]] += bitangent;
		vTangents[uvIndices[fvi1]] += bitangent;
		vTangents[uvIndices[fvi2]] += bitangent;

		normals[uvIndices[fvi0]] += normal;
		normals[uvIndices[fvi1]] += normal;
		normals[uvIndices[fvi2]] += normal;

	}

	// normalize and orthogonalize everything
	for( size_t i = 0; i < uTangents.size(); i++ )
	{
		normals[i].normalize();

		uTangents[i].normalize();
		vTangents[i].normalize();

		// Make uTangent/vTangent orthogonal to normal
		uTangents[i] -= normals[i] * uTangents[i].dot( normals[i] );
		vTangents[i] -= normals[i] * vTangents[i].dot( normals[i] );

		uTangents[i].normalize();
		vTangents[i].normalize();

		if( orthoTangents )
		{
			vTangents[i] -= uTangents[i] * vTangents[i].dot( uTangents[i] );
			vTangents[i].normalize();
		}

		// Ensure we have set of basis vectors (n, uT, vT) with the correct handedness.
		if( uTangents[i].cross( vTangents[i] ).dot( normals[i] ) < 0.0f )
		{
			uTangents[i] *= -1.0f;
		}
	}

	// convert the tangents back to facevarying data and add that to the mesh
	V3fVectorDataPtr fvUD = new V3fVectorData();
	V3fVectorDataPtr fvVD = new V3fVectorData();

	std::vector<V3f> &fvU = fvUD->writable();
	std::vector<V3f> &fvV = fvVD->writable();
	fvU.resize( uvIndices.size() );
	fvV.resize( uvIndices.size() );

	for( unsigned i = 0; i < uvIndices.size(); i++ )
	{
		fvU[i] = uTangents[uvIndices[i]];
		fvV[i] = vTangents[uvIndices[i]];
	}

	PrimitiveVariable tangentPrimVar( PrimitiveVariable::FaceVarying, fvUD );
	PrimitiveVariable bitangentPrimVar( PrimitiveVariable::FaceVarying, fvVD );

	return std::make_pair( tangentPrimVar, bitangentPrimVar );
}

//////////////////////////////////////////////////////////////////////////
// Reverse winding
//////////////////////////////////////////////////////////////////////////

namespace
{

template<typename T>
void reverseWinding( MeshPrimitive *mesh, T &values )
{
	for( PolygonIterator it = mesh->faceBegin(), eIt = mesh->faceEnd(); it != eIt; ++it )
	{
		std::reverse( it.faceVaryingBegin( values.begin() ), it.faceVaryingEnd( values.begin() ) );
	}
}

struct ReverseWindingFunctor
{

	typedef void ReturnType;

	ReverseWindingFunctor( MeshPrimitive *mesh ) : m_mesh( mesh )
	{
	}

	template<typename T>
	void operator()( T *data )
	{
		reverseWinding( m_mesh, data->writable() );
	}

	private :

		MeshPrimitive *m_mesh;

};

} // namespace

void IECore::MeshAlgo::reverseWinding( MeshPrimitive *mesh )
{
	IntVectorDataPtr vertexIds = mesh->vertexIds()->copy();
	::reverseWinding( mesh, vertexIds->writable() );
	mesh->setTopologyUnchecked(
		mesh->verticesPerFace(),
		vertexIds,
		mesh->variableSize( PrimitiveVariable::Vertex ),
		mesh->interpolation()
	);

	ReverseWindingFunctor reverseWindingFunctor( mesh );
	for( auto &it : mesh->variables )
	{
		if( it.second.interpolation == PrimitiveVariable::FaceVarying )
		{
			if( it.second.indices )
			{
				::reverseWinding<IntVectorData::ValueType>( mesh, it.second.indices->writable() );
			}
			else
			{
				despatchTypedData<ReverseWindingFunctor, TypeTraits::IsVectorTypedData>( it.second.data.get(), reverseWindingFunctor );
			}
		}
	}
}

