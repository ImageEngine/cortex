//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MeshNormalsOp.h"
#include "IECore/DespatchTypedData.h"

#include "boost/format.hpp"

using namespace IECore;
using namespace std;

MeshNormalsOp::MeshNormalsOp() : MeshPrimitiveOp( staticTypeName(), "Calculates vertex normals for a mesh." )
{
}

MeshNormalsOp::~MeshNormalsOp()
{
}

struct MeshNormalsOp::CalculateNormals
{
	typedef DataPtr ReturnType;
	
	CalculateNormals( ConstIntVectorDataPtr vertsPerFace, ConstIntVectorDataPtr vertIds )
		:	m_vertsPerFace( vertsPerFace ), m_vertIds( vertIds )
	{
	}
	
	template<typename T>
	ReturnType operator()( typename T::Ptr data )
	{
		typedef typename T::ValueType VecContainer;
		typedef typename VecContainer::value_type Vec;
		
		const typename T::ValueType &points = data->readable();
		const vector<int> &vertsPerFace = m_vertsPerFace->readable();
		const vector<int> &vertIds = m_vertIds->readable();

		typename T::Ptr normalsData = new T;
		VecContainer &normals = normalsData->writable();
		normals.resize( points.size(), Vec( 0 ) );

		// for each face, calculate its normal, and accumulate that normal onto
		// the normal for each of its vertices.
		const int *vertId = &(vertIds[0]);
		for( vector<int>::const_iterator it = vertsPerFace.begin(); it!=vertsPerFace.end(); it++ )
		{
			const Vec &p0 = points[*vertId];
			const Vec &p1 = points[*(vertId+1)];
			const Vec &p2 = points[*(vertId+2)];
			
			Vec normal = (p2-p1).cross(p0-p1);
			normal.normalize();
			for( int i=0; i<*it; i++ )
			{
				normals[*vertId] += normal;
				vertId++;
			}
		}
		
		// normalize each of the vertex normals
		for( typename VecContainer::iterator it=normals.begin(); it!=normals.end(); it++ )
		{
			it->normalize();
		}
		
		return normalsData;
	}
	
	private :
	
		ConstIntVectorDataPtr m_vertsPerFace;
		ConstIntVectorDataPtr m_vertIds;
		
};

struct MeshNormalsOp::HandleErrors
{
	template<typename T, typename F>
	void operator()( typename T::ConstPtr d, const F &f )
	{
		string e = boost::str( boost::format( "MeshNormalsOp : \"P\" has unsupported data type \"%s\"." ) % d->typeName() );
		throw InvalidArgumentException( e );
	}
};

void MeshNormalsOp::modifyTypedPrimitive( MeshPrimitivePtr mesh, ConstCompoundObjectPtr operands )
{
	PrimitiveVariableMap::const_iterator pvIt = mesh->variables.find( "P" );
	if( pvIt==mesh->variables.end() || !pvIt->second.data )
	{
		throw InvalidArgumentException( "MeshNormalsOp : MeshPrimitive has no \"P\" primitive variable." );
	}
	
	if( !mesh->isPrimitiveVariableValid( pvIt->second ) )
	{
		throw InvalidArgumentException( "MeshNormalsOp : \"P\" primitive variable is invalid." );
	}

	CalculateNormals f( mesh->verticesPerFace(), mesh->vertexIds() );
	DataPtr n = despatchTypedData<CalculateNormals, TypeTraits::IsVec3VectorTypedData, HandleErrors>( pvIt->second.data, f );

	mesh->variables["N"] = PrimitiveVariable( PrimitiveVariable::Vertex, n );
}
