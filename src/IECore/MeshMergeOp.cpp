//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MeshMergeOp.h"
#include "IECore/CompoundParameter.h"
#include "IECore/NullObject.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TypeTraits.h"

#include <algorithm>

using namespace IECore;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( MeshMergeOp );

MeshMergeOp::MeshMergeOp()
	:	MeshPrimitiveOp( "Merges one mesh with another." )
{
	m_meshParameter = new MeshPrimitiveParameter(
		"mesh",
		"The mesh to be merged with the input.",
		new MeshPrimitive
	);
	
	m_removePrimVarsParameter = new BoolParameter(
		"removeNonMatchingPrimVars",
		"If true, PrimitiveVariables that exist on one mesh and not the other will be removed. If false, the PrimitiveVariable data will be expanded using a default value.",
		false
	);
	
	parameters()->addParameter( m_meshParameter );
	parameters()->addParameter( m_removePrimVarsParameter );
}

MeshMergeOp::~MeshMergeOp()
{
}

MeshPrimitiveParameter * MeshMergeOp::meshParameter()
{
	return m_meshParameter.get();
}

const MeshPrimitiveParameter * MeshMergeOp::meshParameter() const
{
	return m_meshParameter.get();
}

template<class T>
struct MeshMergeOp::DefaultValue
{
	T operator()()
	{
		return T();
	}
};

template<class T>
struct MeshMergeOp::DefaultValue<Imath::Vec3<T> >
{
	Imath::Vec3<T> operator()()
	{
		return Imath::Vec3<T>( 0 );
	}
};

template<class T>
struct MeshMergeOp::DefaultValue<Imath::Vec2<T> >
{
	Imath::Vec2<T> operator()()
	{
		return Imath::Vec2<T>( 0 );
	}
};

struct MeshMergeOp::AppendPrimVars
{
	typedef void ReturnType;

	AppendPrimVars( MeshPrimitive *mesh, const MeshPrimitive *mesh2, const std::string &name, const PrimitiveVariable::Interpolation interpolation, const bool remove, std::set<DataPtr> &visitedData )
		:	m_mesh( mesh ), m_mesh2( mesh2 ), m_name( name ), m_interpolation( interpolation ), m_remove( remove ), m_visitedData( visitedData )
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

		const T *data2 = m_mesh2->variableData<T>( m_name, m_interpolation );
		if ( data2 )
		{
			data->writable().insert( data->writable().end(), data2->readable().begin(), data2->readable().end() );
		}
		else if ( m_remove )
		{
			m_mesh->variables.erase( m_name );
		}
		else
		{
			typedef typename T::ValueType::value_type ValueType;
 			ValueType defaultValue = DefaultValue<ValueType>()();
 			size_t size = m_mesh2->variableSize( m_interpolation );
			
 			data->writable().insert( data->writable().end(), size, defaultValue );
		}
	}

	private :

		MeshPrimitive *m_mesh;
		const MeshPrimitive *m_mesh2;
		const std::string m_name;
		const PrimitiveVariable::Interpolation m_interpolation;
		const bool m_remove;
		std::set<DataPtr> &m_visitedData;

};

struct MeshMergeOp::PrependPrimVars
{
	typedef void ReturnType;
	
	PrependPrimVars( MeshPrimitive *mesh, const std::string &name, const PrimitiveVariable::Interpolation interpolation, const bool remove, std::map<ConstDataPtr, DataPtr> &visitedData )
		:	m_mesh( mesh ), m_name( name ), m_interpolation( interpolation ), m_remove( remove ), m_visitedData( visitedData )
	{
	}
	
	template<typename T>
	ReturnType operator()( const T *data )
	{
		PrimitiveVariableMap::iterator it = m_mesh->variables.find( m_name );
		if ( it == m_mesh->variables.end() && !m_remove )
		{
			typename T::Ptr data2 = 0;

			std::map<ConstDataPtr, DataPtr>::iterator dataIt = m_visitedData.find( data );
			if ( dataIt != m_visitedData.end() )
			{
				data2 = runTimeCast<T>( dataIt->second );
			}
			
			if ( !data2 )
			{
				typedef typename T::ValueType::value_type ValueType;
				ValueType defaultValue = DefaultValue<ValueType>()();
				size_t size = m_mesh->variableSize( m_interpolation ) - data->readable().size();
				
				data2 = new T();
				data2->writable().insert( data2->writable().end(), size, defaultValue );
				data2->writable().insert( data2->writable().end(), data->readable().begin(), data->readable().end() );
			}
			
			m_mesh->variables[m_name] = PrimitiveVariable( m_interpolation, data2 );
			
			m_visitedData[data] = data2;
		}
	}
	
	private :

		MeshPrimitive *m_mesh;
		const std::string m_name;
		const PrimitiveVariable::Interpolation m_interpolation;
		const bool m_remove;
		std::map<ConstDataPtr, DataPtr> &m_visitedData;

};

void MeshMergeOp::modifyTypedPrimitive( MeshPrimitive * mesh, const CompoundObject * operands )
{
	const MeshPrimitive *mesh2 = static_cast<const MeshPrimitive *>( m_meshParameter->getValue() );

	const vector<int> &verticesPerFace1 = mesh->verticesPerFace()->readable();
	const vector<int> &vertexIds1 = mesh->vertexIds()->readable();

	const vector<int> &verticesPerFace2 = mesh2->verticesPerFace()->readable();
	const vector<int> &vertexIds2 = mesh2->vertexIds()->readable();

	IntVectorDataPtr verticesPerFaceData = new IntVectorData;
	vector<int> &verticesPerFace = verticesPerFaceData->writable();
	verticesPerFace.resize( verticesPerFace1.size() + verticesPerFace2.size() );
	vector<int>::iterator it = copy( verticesPerFace1.begin(), verticesPerFace1.end(), verticesPerFace.begin() );
	copy( verticesPerFace2.begin(), verticesPerFace2.end(), it );

	IntVectorDataPtr vertexIdsData = new IntVectorData;
	vector<int> &vertexIds = vertexIdsData->writable();
	vertexIds.resize( vertexIds1.size() + vertexIds2.size() );
	it = copy( vertexIds1.begin(), vertexIds1.end(), vertexIds.begin() );
	int vertexIdOffset = mesh->variableSize( PrimitiveVariable::Vertex );
	transform( vertexIds2.begin(), vertexIds2.end(), it, bind2nd( plus<int>(), vertexIdOffset ) );

	mesh->setTopology( verticesPerFaceData, vertexIdsData, mesh->interpolation() );
	
	std::set<DataPtr> visitedData;
	PrimitiveVariableMap::iterator pvIt;
	for( pvIt=mesh->variables.begin(); pvIt!=mesh->variables.end(); pvIt++ )
	{
		if( pvIt->second.interpolation!=PrimitiveVariable::Constant )
		{
			AppendPrimVars f( mesh, mesh2, pvIt->first, pvIt->second.interpolation, m_removePrimVarsParameter->getTypedValue(), visitedData );
			despatchTypedData<AppendPrimVars, TypeTraits::IsVectorTypedData, DespatchTypedDataIgnoreError>( pvIt->second.data.get(), f );
		}
	}
	
	std::map<ConstDataPtr, DataPtr> visitedData2;
	PrimitiveVariableMap::const_iterator pvIt2;
	for ( pvIt2=mesh2->variables.begin(); pvIt2 != mesh2->variables.end(); pvIt2++ )
	{
		if ( pvIt2->second.interpolation != PrimitiveVariable::Constant )
		{
			PrependPrimVars f( mesh, pvIt2->first, pvIt2->second.interpolation, m_removePrimVarsParameter->getTypedValue(), visitedData2 );
			despatchTypedData<PrependPrimVars, TypeTraits::IsVectorTypedData, DespatchTypedDataIgnoreError>( pvIt2->second.data.get(), f );
		}
	}
}
