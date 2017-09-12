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

#include "IECore/DespatchTypedData.h"
#include "IECore/MeshAlgo.h"

using namespace Imath;
using namespace IECore;

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
