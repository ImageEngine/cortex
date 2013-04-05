//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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
#include "IECore/CompoundParameter.h"

#include "boost/format.hpp"

using namespace IECore;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( MeshNormalsOp );

MeshNormalsOp::MeshNormalsOp() : MeshPrimitiveOp( "Calculates vertex normals for a mesh." )
{
	/// \todo Add this parameter to a member variable and update pPrimVarNameParameter() functions.
	StringParameterPtr pPrimVarNameParameter = new StringParameter(
		"pPrimVarName",	
		"Input primitive variable name.",
		"P"
	);

	/// \todo Add this parameter to a member variable and update nPrimVarNameParameter() functions.
	StringParameterPtr nPrimVarNameParameter = new StringParameter(
		"nPrimVarName",
		"Output primitive variable name.",
		"N"
	);

	parameters()->addParameter( pPrimVarNameParameter );
	parameters()->addParameter( nPrimVarNameParameter );
}

MeshNormalsOp::~MeshNormalsOp()
{
}

StringParameter * MeshNormalsOp::pPrimVarNameParameter()
{
	return parameters()->parameter<StringParameter>( "pPrimVarName" );
}

const StringParameter * MeshNormalsOp::pPrimVarNameParameter() const
{
	return parameters()->parameter<StringParameter>( "pPrimVarName" );
}

StringParameter * MeshNormalsOp::nPrimVarNameParameter()
{
	return parameters()->parameter<StringParameter>( "nPrimVarName" );
}

const StringParameter * MeshNormalsOp::nPrimVarNameParameter() const
{
	return parameters()->parameter<StringParameter>( "nPrimVarName" );
}

struct MeshNormalsOp::CalculateNormals
{
	typedef DataPtr ReturnType;

	CalculateNormals( const IntVectorData * vertsPerFace, const IntVectorData * vertIds )
		:	m_vertsPerFace( vertsPerFace ), m_vertIds( vertIds )
	{
	}

	template<typename T>
	ReturnType operator()( T * data )
	{
		typedef typename T::ValueType VecContainer;
		typedef typename VecContainer::value_type Vec;

		const typename T::ValueType &points = data->readable();
		const vector<int> &vertsPerFace = m_vertsPerFace->readable();
		const vector<int> &vertIds = m_vertIds->readable();

		typename T::Ptr normalsData = new T;
		normalsData->setInterpretation( GeometricData::Normal );
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
	void operator()( const T *d, const F &f )
	{
		string e = boost::str( boost::format( "MeshNormalsOp : pPrimVarName parameter has unsupported data type \"%s\"." ) % d->typeName() );
		throw InvalidArgumentException( e );
	}
};

void MeshNormalsOp::modifyTypedPrimitive( MeshPrimitive * mesh, const CompoundObject * operands )
{
	const std::string &pPrimVarName = pPrimVarNameParameter()->getTypedValue();
	PrimitiveVariableMap::const_iterator pvIt = mesh->variables.find( pPrimVarName );
	if( pvIt==mesh->variables.end() || !pvIt->second.data )
	{
		string e = boost::str( boost::format( "MeshNormalsOp : MeshPrimitive has no \"%s\" primitive variable." ) % pPrimVarName );
		throw InvalidArgumentException( e );
	}

	if( !mesh->isPrimitiveVariableValid( pvIt->second ) )
	{
		string e = boost::str( boost::format( "MeshNormalsOp : \"%s\" primitive variable is invalid." ) % pPrimVarName );
		throw InvalidArgumentException( e );
	}

	CalculateNormals f( mesh->verticesPerFace(), mesh->vertexIds() );
	DataPtr n = despatchTypedData<CalculateNormals, TypeTraits::IsVec3VectorTypedData, HandleErrors>( pvIt->second.data, f );

	mesh->variables[ nPrimVarNameParameter()->getTypedValue() ] = PrimitiveVariable( PrimitiveVariable::Vertex, n );
}
