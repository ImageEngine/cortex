//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

#include <cassert>
#include <algorithm>

#include "boost/format.hpp"

#include "IECore/DataCastOp.h"
#include "IECore/Convert.h"
#include "IECore/MeshTangentsOp.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/CompoundParameter.h"

using namespace IECore;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( MeshTangentsOp );

MeshTangentsOp::MeshTangentsOp() : MeshPrimitiveOp( "Calculates mesh tangents with respect to texture coordinates." )
{
	/// \todo Add this parameter to a member variable and update pPrimVarNameParameter() functions.
	StringParameterPtr pPrimVarNameParameter = new StringParameter(
		"pPrimVarName",	
		"pPrimVarName description",
		"P"
	);

	m_uPrimVarNameParameter = new StringParameter(
		"uPrimVarName",
		"uPrimVarName description",
		"s"
	);

	m_vPrimVarNameParameter = new StringParameter(
		"vPrimVarName",
		"vPrimVarName description",
		"t"
	);
	
	StringParameterPtr uvIndicesPrimVarNameParameter = new StringParameter(
		"uvIndicesPrimVarName",
		"This primitive variable must be FaceVarying IntVectorData, and store indices for the uvs referenced "
		"in the u and v primvars. See IECoreMaya::FromMayaMeshConverter for an example of getting such indices. "
		"You may set this to the empty string if no indices are available, but this will mean that the tangents will "
		"be incorrect across uv boundaries.",
		"stIndices"
	);

	m_uTangentPrimVarNameParameter = new StringParameter(
		"uTangentPrimVarName",
		"uTangentPrimVarName description",
		"uTangent"
	);

	m_vTangentPrimVarNameParameter = new StringParameter(
		"vTangentPrimVarName",
		"vTangentPrimVarName description",
		"vTangent"
	);

	/// \todo add this parameter on a member variable, update orthogonalizeTangentsParameter() and set default value to true.
	BoolParameterPtr orthogonalizeTangentsParameter = new BoolParameter(
		"orthogonalizeTangents",
		"Make sure tangent and bitangent are orthogonal.",
		false
	);

	parameters()->addParameter( orthogonalizeTangentsParameter );
	parameters()->addParameter( pPrimVarNameParameter );
	parameters()->addParameter( m_uPrimVarNameParameter );
	parameters()->addParameter( m_vPrimVarNameParameter );
	parameters()->addParameter( uvIndicesPrimVarNameParameter );
	parameters()->addParameter( m_uTangentPrimVarNameParameter );
	parameters()->addParameter( m_vTangentPrimVarNameParameter );
}

MeshTangentsOp::~MeshTangentsOp()
{
}

StringParameter * MeshTangentsOp::pPrimVarNameParameter()
{
	return parameters()->parameter<StringParameter>( "pPrimVarName" );
}

const StringParameter * MeshTangentsOp::pPrimVarNameParameter() const
{
	return parameters()->parameter<StringParameter>( "pPrimVarName" );
}

BoolParameter * MeshTangentsOp::orthogonalizeTangentsParameter()
{
	return parameters()->parameter<BoolParameter>( "orthogonalizeTangents" );
}

const BoolParameter * MeshTangentsOp::orthogonalizeTangentsParameter() const
{
	return parameters()->parameter<BoolParameter>( "orthogonalizeTangents" );
}


StringParameter * MeshTangentsOp::uPrimVarNameParameter()
{
	return m_uPrimVarNameParameter.get();
}

const StringParameter * MeshTangentsOp::uPrimVarNameParameter() const
{
	return m_uPrimVarNameParameter.get();
}

StringParameter * MeshTangentsOp::vPrimVarNameParameter()
{
	return m_vPrimVarNameParameter.get();
}

const StringParameter * MeshTangentsOp::vPrimVarNameParameter() const
{
	return m_vPrimVarNameParameter.get();
}

StringParameter * MeshTangentsOp::uvIndicesPrimVarNameParameter()
{
	return parameters()->parameter<StringParameter>( "uvIndicesPrimVarName" );
}

const StringParameter * MeshTangentsOp::uvIndicesPrimVarNameParameter() const
{
	return parameters()->parameter<StringParameter>( "uvIndicesPrimVarName" );
}
		
StringParameter * MeshTangentsOp::uTangentPrimVarNameParameter()
{
	return m_uTangentPrimVarNameParameter.get();
}

const StringParameter * MeshTangentsOp::uTangentPrimVarNameParameter() const
{
	return m_uTangentPrimVarNameParameter.get();
}

StringParameter * MeshTangentsOp::vTangentPrimVarNameParameter()
{
	return m_vTangentPrimVarNameParameter.get();
}

const StringParameter * MeshTangentsOp::vTangentPrimVarNameParameter() const
{
	return m_vTangentPrimVarNameParameter.get();
}

struct MeshTangentsOp::CalculateTangents
{
	typedef void ReturnType;

	CalculateTangents( const vector<int> &vertsPerFace, const vector<int> &vertIds, const vector<float> &u, const vector<float> &v, const vector<int> &uvIndices, bool orthoTangents )
		:	m_vertsPerFace( vertsPerFace ), m_vertIds( vertIds ), m_u( u ), m_v( v ), m_uvIds( uvIndices ), m_orthoTangents( orthoTangents )
	{

	}

	template<typename T>
	ReturnType operator()( T * data )
	{
		typedef typename T::ValueType VecContainer;
		typedef typename VecContainer::value_type Vec;

		const VecContainer &points = data->readable();
		
		// the uvIndices array is indexed as with any other facevarying data. the values in the
		// array specify the connectivity of the uvs - where two facevertices have the same index
		// they are known to be sharing a uv. for each one of these unique indices we compute
		// the tangents and normal, by accumulating all the tangents and normals for the faces
		// that reference them. we then take this data and shuffle it back into facevarying
		// primvars for the mesh.
		int numUniqueTangents = 1 + *max_element( m_uvIds.begin(), m_uvIds.end() );
		
		VecContainer uTangents( numUniqueTangents, Vec( 0 ) );
		VecContainer vTangents( numUniqueTangents, Vec( 0 ) );
		VecContainer normals( numUniqueTangents, Vec( 0 ) );
		
		for( size_t faceIndex = 0; faceIndex < m_vertsPerFace.size() ; faceIndex++ )
		{
			
			assert( m_vertsPerFace[faceIndex] == 3 );

			// indices into the facevarying data for this face 
			size_t fvi0 = faceIndex * 3;
			size_t fvi1 = fvi0 + 1;
			size_t fvi2 = fvi1 + 1;
			assert( fvi2 < m_vertIds.size() );
			assert( fvi2 < m_u.size() );
			assert( fvi2 < m_v.size() );
			
			// positions for each vertex of this face
			const Vec &p0 = points[ m_vertIds[ fvi0 ] ];
			const Vec &p1 = points[ m_vertIds[ fvi1 ] ];
			const Vec &p2 = points[ m_vertIds[ fvi2 ] ];

			// uv coordinates for each vertex of this face
			const Imath::V2f uv0( m_u[ fvi0 ], m_v[ fvi0 ] );
			const Imath::V2f uv1( m_u[ fvi1 ], m_v[ fvi1 ] );
			const Imath::V2f uv2( m_u[ fvi2 ], m_v[ fvi2 ] );

			// compute tangents and normal for this face
			const Vec e0 = p1 - p0;
			const Vec e1 = p2 - p0;

			const Imath::V2f e0uv = uv1 - uv0;
			const Imath::V2f e1uv = uv2 - uv0;

			Vec tangent   = ( e0 * -e1uv.y + e1 * e0uv.y ).normalized();
			Vec bitangent = ( e0 * -e1uv.x + e1 * e0uv.x ).normalized();

			Vec normal = (p2-p1).cross(p0-p1);
			normal.normalize();

			// and accumlate them into the computation so far
			uTangents[ m_uvIds[fvi0] ] += tangent;
			uTangents[ m_uvIds[fvi1] ] += tangent;
			uTangents[ m_uvIds[fvi2] ] += tangent;
			
			vTangents[ m_uvIds[fvi0] ] += bitangent;
			vTangents[ m_uvIds[fvi1] ] += bitangent;
			vTangents[ m_uvIds[fvi2] ] += bitangent;
			
			normals[ m_uvIds[fvi0] ] += normal;
			normals[ m_uvIds[fvi1] ] += normal;
			normals[ m_uvIds[fvi2] ] += normal;
			
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

			if ( m_orthoTangents )
			{
				vTangents[i] -= uTangents[i] * vTangents[i].dot( uTangents[i] );
				vTangents[i].normalize();
			}
		
			// make things less sinister
			if( uTangents[i].cross( vTangents[i] ).dot( normals[i] ) < 0.0f )
			{
				uTangents[i] *= -1.0f;
			}
		}
		
		// convert the tangents back to facevarying data and add that to the mesh
		typename T::Ptr fvUD = new T();
		typename T::Ptr fvVD = new T();
		fvUTangentsData = fvUD;
		fvVTangentsData = fvVD;
		
		VecContainer &fvUTangents = fvUD->writable();
		VecContainer &fvVTangents = fvVD->writable();
		fvUTangents.resize( m_uvIds.size() );
		fvVTangents.resize( m_uvIds.size() );
		
		for( unsigned i=0; i<m_uvIds.size(); i++ )
		{
			fvUTangents[i] = uTangents[m_uvIds[i]];
			fvVTangents[i] = vTangents[m_uvIds[i]];
		}

	}
	
	// this is the data filled in by operator() above, ready to be added onto the mesh
	DataPtr fvUTangentsData;
	DataPtr fvVTangentsData;
	
	private :

		const vector<int> &m_vertsPerFace;
		const vector<int> &m_vertIds;
		const vector<float> &m_u;
		const vector<float> &m_v;
		const vector<int> &m_uvIds;
		bool m_orthoTangents;
		
};

struct MeshTangentsOp::HandleErrors
{
	template<typename T, typename F>
	void operator()( const T *d, const F &f )
	{
		string e = boost::str( boost::format( "MeshTangentsOp : pPrimVarName parameter has unsupported data type \"%s\"." ) % d->typeName() );
		throw InvalidArgumentException( e );
	}
};

void MeshTangentsOp::modifyTypedPrimitive( MeshPrimitive * mesh, const CompoundObject * operands )
{
	if( !mesh->arePrimitiveVariablesValid( ) )
	{
		throw InvalidArgumentException( "MeshTangentsOp : MeshPrimitive variables are invalid." );
	}
	
	const std::string &pPrimVarName = pPrimVarNameParameter()->getTypedValue();
	Data * pData = mesh->variableData<Data>( pPrimVarName, PrimitiveVariable::Vertex );
	if( !pData )
	{
		string e = boost::str( boost::format( "MeshTangentsOp : MeshPrimitive has no Vertex \"%s\" primitive variable." ) % pPrimVarName );
		throw InvalidArgumentException( e );
	}

	const IntVectorData * vertsPerFace = mesh->verticesPerFace();
	for ( IntVectorData::ValueType::const_iterator it = vertsPerFace->readable().begin(); it != vertsPerFace->readable().end(); ++it )
	{
		if ( *it != 3 )
		{
			throw InvalidArgumentException( "MeshTangentsOp : MeshPrimitive has non-triangular faces." );
		}
	}

	const std::string &uPrimVarName = uPrimVarNameParameter()->getTypedValue();
	const std::string &vPrimVarName = vPrimVarNameParameter()->getTypedValue();
	const std::string &uvIndicesPrimVarName = uvIndicesPrimVarNameParameter()->getTypedValue();

	FloatVectorDataPtr uData = mesh->variableData<FloatVectorData>( uPrimVarName, PrimitiveVariable::FaceVarying );
	if( !uData )
	{
		throw InvalidArgumentException( ( boost::format( "MeshTangentsOp : MeshPrimitive has no FaceVarying FloatVectorData primitive variable named \"%s\"."  ) % ( uPrimVarName ) ).str() );
	}

	FloatVectorDataPtr vData = mesh->variableData<FloatVectorData>( vPrimVarName, PrimitiveVariable::FaceVarying );
	if( !vData )
	{
		throw InvalidArgumentException( ( boost::format( "MeshTangentsOp : MeshPrimitive has no FaceVarying FloatVectorData primitive variable named \"%s\"."  ) % ( vPrimVarName ) ).str() );
	}

	ConstIntVectorDataPtr uvIndicesData = 0;
	if( uvIndicesPrimVarName=="" )
	{
		uvIndicesData = mesh->vertexIds();
	}
	else
	{
		uvIndicesData = mesh->variableData<IntVectorData>( uvIndicesPrimVarName, PrimitiveVariable::FaceVarying );
		if( !uvIndicesData )
		{
			throw InvalidArgumentException( ( boost::format( "MeshTangentsOp : MeshPrimitive has no FaceVarying IntVectorData primitive variable named \"%s\"."  ) % ( uvIndicesPrimVarName ) ).str() );
		}
	}
	
	DataCastOpPtr dco = new DataCastOp();
	dco->targetTypeParameter()->setNumericValue( FloatVectorDataTypeId );

	bool orthoTangents = orthogonalizeTangentsParameter()->getTypedValue();

	CalculateTangents f( vertsPerFace->readable(), mesh->vertexIds()->readable(), uData->readable(), vData->readable(), uvIndicesData->readable(), orthoTangents );

	despatchTypedData<CalculateTangents, TypeTraits::IsFloatVec3VectorTypedData, HandleErrors>( pData, f );

	mesh->variables[ uTangentPrimVarNameParameter()->getTypedValue() ] = PrimitiveVariable( PrimitiveVariable::FaceVarying, f.fvUTangentsData );
	mesh->variables[ vTangentPrimVarNameParameter()->getTypedValue() ] = PrimitiveVariable( PrimitiveVariable::FaceVarying, f.fvVTangentsData );

	assert( mesh->arePrimitiveVariablesValid() );
}


