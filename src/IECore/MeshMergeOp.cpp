//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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
	:	MeshPrimitiveOp( staticTypeName(), "Merges one mesh with another." )
{
	m_meshParameter = new MeshPrimitiveParameter(
		"mesh",
		"The mesh to be merged with the input.",
		new MeshPrimitive
	);

	parameters()->addParameter( m_meshParameter );
}

MeshMergeOp::~MeshMergeOp()
{
}

MeshPrimitiveParameterPtr MeshMergeOp::meshParameter()
{
	return m_meshParameter;
}

ConstMeshPrimitiveParameterPtr MeshMergeOp::meshParameter() const
{
	return m_meshParameter;
}

struct MeshMergeOp::AppendPrimVars
{
	typedef void ReturnType;

	AppendPrimVars( ConstMeshPrimitivePtr mesh2, const std::string &name )
		:	m_mesh2( mesh2 ), m_name( name )
	{
	}

	template<typename T>
	ReturnType operator()( typename T::Ptr data )
	{

		PrimitiveVariableMap::const_iterator it = m_mesh2->variables.find( m_name );
		if( it!=m_mesh2->variables.end() )
		{
			typename T::ConstPtr data2 = runTimeCast<const T>( it->second.data );
			if( data2 )
			{
				data->writable().insert( data->writable().end(), data2->readable().begin(), data2->readable().end() );
			}
		}
	}

	private :

		ConstMeshPrimitivePtr m_mesh2;
		std::string m_name;

};

void MeshMergeOp::modifyTypedPrimitive( MeshPrimitivePtr mesh, ConstCompoundObjectPtr operands )
{
	ConstMeshPrimitivePtr mesh2 = boost::static_pointer_cast<const MeshPrimitive>( m_meshParameter->getValue() );

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

	PrimitiveVariableMap::iterator pvIt;
	for( pvIt=mesh->variables.begin(); pvIt!=mesh->variables.end(); pvIt++ )
	{
		if( pvIt->second.interpolation!=PrimitiveVariable::Constant )
		{
			AppendPrimVars f( mesh2, pvIt->first );
			despatchTypedData<AppendPrimVars, TypeTraits::IsVectorTypedData, DespatchTypedDataIgnoreError>( pvIt->second.data, f );
		}
	}
}
