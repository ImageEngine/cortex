//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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
#include "IECoreScene/private/PrimitiveAlgoUtils.h"

#include "IECore/DataAlgo.h"
#include "IECore/DespatchTypedData.h"

#include "boost/format.hpp"

#include <algorithm>

using namespace Imath;
using namespace IECore;
using namespace IECoreScene;

namespace
{

template<class T>
struct DefaultValue
{
	T operator()()
	{
		return T();
	}
};

template<class T>
struct DefaultValue<Imath::Vec3<T> >
{
	Imath::Vec3<T> operator()()
	{
		return Imath::Vec3<T>( 0 );
	}
};

template<class T>
struct DefaultValue<Imath::Vec2<T> >
{
	Imath::Vec2<T> operator()()
	{
		return Imath::Vec2<T>( 0 );
	}
};

struct AppendPrimVars
{
		typedef void ReturnType;

		AppendPrimVars( MeshPrimitive *mesh, const MeshPrimitive *mesh2, const std::string &name, const PrimitiveVariable::Interpolation interpolation, IntVectorData *indices, std::set<DataPtr> &visitedData )
			:	m_mesh( mesh ), m_mesh2( mesh2 ), m_name( name ), m_interpolation( interpolation ), m_indices( indices ), m_visitedData( visitedData )
		{
		}

		template<typename T>
		ReturnType operator()( T *data )
		{
			if ( m_visitedData.find( data ) != m_visitedData.end() )
			{
				return;
			}
			m_visitedData.insert( data );

			PrimitiveVariableMap::const_iterator it = m_mesh2->variables.find( m_name );
			if( it != m_mesh2->variables.end() && it->second.data->isInstanceOf( data->staticTypeId() ) && it->second.interpolation == m_interpolation )
			{
				if( m_indices )
				{
					const int offset = data->readable().size();

					const T *data2 = runTimeCast<const T>( it->second.data.get() );
					data->writable().insert( data->writable().end(), data2->readable().begin(), data2->readable().end() );

					if( it->second.indices )
					{
						/// Re-index to fit on the end of the existing data
						/// \todo: the data would be more compact if we search
						/// existing values rather than blindly insert.
						std::vector<int> &indices = m_indices->writable();
						const std::vector<int> &indices2 = it->second.indices->readable();
						indices.reserve( indices.size() + indices2.size() );
						for( const auto &index : indices2 )
						{
							indices.push_back( offset + index );
						}
					}
					else
					{
						/// Append new indices for the second mesh
						/// \todo: the data would be more compact if we search
						/// existing values rather than blindly insert.
						std::vector<int> &indices = m_indices->writable();
						const size_t data2Size = data2->readable().size();
						for( size_t i = 0; i < data2Size; ++i )
						{
							indices.push_back( offset + i );
						}
					}
				}
				else
				{
					/// The first mesh dictates whether the PrimitiveVariable should
					/// be indexed. If the second mesh has indices, we must expand them.
					typename T::Ptr expandedData2 = runTimeCast<T>( it->second.expandedData() );
					data->writable().insert( data->writable().end(), expandedData2->readable().begin(), expandedData2->readable().end() );
				}
			}
			else
			{
				typedef typename T::ValueType::value_type ValueType;
				ValueType defaultValue = DefaultValue<ValueType>()();
				size_t size = m_mesh2->variableSize( m_interpolation );
				if( !size )
				{
					/// mesh2 may have an empty variableSize if it contains
					/// no topology, so early out rather than appending.
					return;
				}

				if( m_indices )
				{
					/// \todo: the data would be more compact if we search for defaultValue
					/// in the existing data rather than blindly insert.
					m_indices->writable().insert( m_indices->writable().end(), size, data->writable().size() );
					data->writable().push_back( defaultValue );
				}
				else
				{
					data->writable().insert( data->writable().end(), size, defaultValue );
				}
			}
		}

	private :

		MeshPrimitive *m_mesh;
		const MeshPrimitive *m_mesh2;
		const std::string m_name;
		const PrimitiveVariable::Interpolation m_interpolation;
		IntVectorData *m_indices;
		std::set<DataPtr> &m_visitedData;

};

struct PrependPrimVars
{
		typedef void ReturnType;

		PrependPrimVars( MeshPrimitive *mesh, const std::string &name, const PrimitiveVariable &primVar, std::map<ConstDataPtr, DataPtr> &visitedData )
			:	m_mesh( mesh ), m_name( name ), m_primVar( primVar ), m_visitedData( visitedData )
		{
		}

		template<typename T>
		ReturnType operator()( const T *data )
		{
			PrimitiveVariableMap::iterator it = m_mesh->variables.find( m_name );
			if( it == m_mesh->variables.end() )
			{
				typename T::Ptr data2 = nullptr;

				std::map<ConstDataPtr, DataPtr>::iterator dataIt = m_visitedData.find( data );
				if ( dataIt != m_visitedData.end() )
				{
					data2 = runTimeCast<T>( dataIt->second );
				}

				if ( !data2 )
				{
					typedef typename T::ValueType::value_type ValueType;
					ValueType defaultValue = DefaultValue<ValueType>()();

					typename T::Ptr expandedData = runTimeCast<T>( m_primVar.expandedData() );
					size_t size = m_mesh->variableSize( m_primVar.interpolation ) - expandedData->readable().size();
					data2 = new T();
					data2->writable().insert( data2->writable().end(), size, defaultValue );

					/// The first mesh dictates whether the PrimitiveVariable should
					/// be indexed. If the second mesh has indices, we must expand them.
					data2->writable().insert( data2->writable().end(), expandedData->readable().begin(), expandedData->readable().end() );
				}

				m_mesh->variables[m_name] = PrimitiveVariable( m_primVar.interpolation, data2 );

				m_visitedData[data] = data2;
			}
		}

	private :

		MeshPrimitive *m_mesh;
		const std::string m_name;
		const PrimitiveVariable &m_primVar;
		std::map<ConstDataPtr, DataPtr> &m_visitedData;

};

void merge( MeshPrimitive *a, const MeshPrimitive *b )
{
	const auto &vertexIdsA = a->vertexIds()->readable();
	const auto &verticesPerFaceA = a->verticesPerFace()->readable();

	const auto &vertexIdsB = b->vertexIds()->readable();
	const auto &verticesPerFaceB = b->verticesPerFace()->readable();

	IntVectorDataPtr verticesPerFaceData = new IntVectorData;
	auto &verticesPerFace = verticesPerFaceData->writable();
	verticesPerFace.resize( verticesPerFaceA.size() + verticesPerFaceB.size() );
	auto it = std::copy( verticesPerFaceA.begin(), verticesPerFaceA.end(), verticesPerFace.begin() );
	std::copy( verticesPerFaceB.begin(), verticesPerFaceB.end(), it );

	IntVectorDataPtr vertexIdsData = new IntVectorData;
	auto &vertexIds = vertexIdsData->writable();
	vertexIds.resize( vertexIdsA.size() + vertexIdsB.size() );
	it = std::copy( vertexIdsA.begin(), vertexIdsA.end(), vertexIds.begin() );
	int vertexIdOffset = a->variableSize( PrimitiveVariable::Vertex );
	std::transform( vertexIdsB.begin(), vertexIdsB.end(), it, std::bind2nd( std::plus<int>(), vertexIdOffset ) );

	a->setTopologyUnchecked( verticesPerFaceData, vertexIdsData, a->variableSize( PrimitiveVariable::Vertex ) + b->variableSize( PrimitiveVariable::Vertex ), a->interpolation() );

	const auto &bCornerIds = b->cornerIds()->readable();
	if( !bCornerIds.empty() )
	{
		const auto &aCornerIds = a->cornerIds()->readable();
		IntVectorDataPtr idData = new IntVectorData;
		auto &ids = idData->writable();
		ids.resize( aCornerIds.size() + bCornerIds.size() );
		it = std::copy( aCornerIds.begin(), aCornerIds.end(), ids.begin() );
		std::transform( bCornerIds.begin(), bCornerIds.end(), it, std::bind2nd( std::plus<int>(), vertexIdOffset ) );

		const auto &aSharpnesses = a->cornerSharpnesses()->readable();
		const auto &bSharpnesses = b->cornerSharpnesses()->readable();
		FloatVectorDataPtr sharpnessData = new FloatVectorData;
		auto &sharpnesses = sharpnessData->writable();
		sharpnesses.resize( aSharpnesses.size() + bSharpnesses.size() );
		auto fIt = std::copy( aSharpnesses.begin(), aSharpnesses.end(), sharpnesses.begin() );
		std::copy( bSharpnesses.begin(), bSharpnesses.end(), fIt );

		a->setCorners( idData.get(), sharpnessData.get() );
	}

	const auto &bCreaseIds = b->creaseIds()->readable();
	if( !bCreaseIds.empty() )
	{
		const auto &aLengths = a->creaseLengths()->readable();
		const auto &bLengths = b->creaseLengths()->readable();
		IntVectorDataPtr lengthData = new IntVectorData;
		auto &lengths = lengthData->writable();
		lengths.resize( aLengths.size() + bLengths.size() );
		it = std::copy( aLengths.begin(), aLengths.end(), lengths.begin() );
		std::copy( bLengths.begin(), bLengths.end(), it );

		const auto &aCreaseIds = a->creaseIds()->readable();
		IntVectorDataPtr idData = new IntVectorData;
		auto &ids = idData->writable();
		ids.resize( aCreaseIds.size() + bCreaseIds.size() );
		it = std::copy( aCreaseIds.begin(), aCreaseIds.end(), ids.begin() );
		std::transform( bCreaseIds.begin(), bCreaseIds.end(), it, std::bind2nd( std::plus<int>(), vertexIdOffset ) );

		const auto &aSharpnesses = a->creaseSharpnesses()->readable();
		const auto &bSharpnesses = b->creaseSharpnesses()->readable();
		FloatVectorDataPtr sharpnessData = new FloatVectorData;
		auto &sharpnesses = sharpnessData->writable();
		sharpnesses.resize( aSharpnesses.size() + bSharpnesses.size() );
		auto fIt = std::copy( aSharpnesses.begin(), aSharpnesses.end(), sharpnesses.begin() );
		std::copy( bSharpnesses.begin(), bSharpnesses.end(), fIt );

		a->setCreases( lengthData.get(), idData.get(), sharpnessData.get() );
	}

	/// \todo: can this be parallelized?
	std::set<DataPtr> visitedData;
	for( auto &pv : a->variables )
	{
		if( pv.second.interpolation != PrimitiveVariable::Constant )
		{
			IntVectorData *indices = pv.second.indices ? pv.second.indices.get() : nullptr;
			AppendPrimVars f( a, b, pv.first, pv.second.interpolation, indices, visitedData );
			despatchTypedData<AppendPrimVars, TypeTraits::IsVectorTypedData, DespatchTypedDataIgnoreError>( pv.second.data.get(), f );
		}
	}

	/// \todo: can this be parallelized?
	std::map<ConstDataPtr, DataPtr> visitedData2;
	for( auto &pv : b->variables )
	{
		if( pv.second.interpolation != PrimitiveVariable::Constant )
		{
			PrependPrimVars f( a, pv.first, pv.second, visitedData2 );
			despatchTypedData<PrependPrimVars, TypeTraits::IsVectorTypedData, DespatchTypedDataIgnoreError>( pv.second.data.get(), f );
		}
	}
}

} // namespace

MeshPrimitivePtr IECoreScene::MeshAlgo::merge( const std::vector<const MeshPrimitive *> &meshes )
{
	if( meshes.empty() )
	{
		throw IECore::InvalidArgumentException( "IECoreScene::MeshAlgo::merge : No Mesh Primitives were provided." );
	}

	/// \todo: can this be parallelized?
	MeshPrimitivePtr result = meshes[0]->copy();
	auto it = meshes.begin() + 1;
	for( ; it != meshes.end(); ++it )
	{
		::merge( result.get(), *it );
	}

	return result;
}
