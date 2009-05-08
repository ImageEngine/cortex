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

#include <cassert>

#include "boost/format.hpp"

#include "IECore/DataCastOp.h"
#include "IECore/Convert.h"
#include "IECore/MeshTangentsOp.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/CompoundParameter.h"

using namespace IECore;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( MeshTangentsOp );

MeshTangentsOp::MeshTangentsOp() : MeshPrimitiveOp( staticTypeName(), "Calculates vertex normals for a mesh." )
{
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

	parameters()->addParameter( m_uPrimVarNameParameter );
	parameters()->addParameter( m_vPrimVarNameParameter );
	parameters()->addParameter( m_uTangentPrimVarNameParameter );
	parameters()->addParameter( m_vTangentPrimVarNameParameter );
}

MeshTangentsOp::~MeshTangentsOp()
{
}

StringParameterPtr MeshTangentsOp::uPrimVarNameParameter()
{
	return m_uPrimVarNameParameter;
}

ConstStringParameterPtr MeshTangentsOp::uPrimVarNameParameter() const
{
	return m_uPrimVarNameParameter;
}

StringParameterPtr MeshTangentsOp::vPrimVarNameParameter()
{
	return m_vPrimVarNameParameter;
}

ConstStringParameterPtr MeshTangentsOp::vPrimVarNameParameter() const
{
	return m_vPrimVarNameParameter;
}

StringParameterPtr MeshTangentsOp::uTangentPrimVarNameParameter()
{
	return m_uTangentPrimVarNameParameter;
}

ConstStringParameterPtr MeshTangentsOp::uTangentPrimVarNameParameter() const
{
	return m_uTangentPrimVarNameParameter;
}

StringParameterPtr MeshTangentsOp::vTangentPrimVarNameParameter()
{
	return m_vTangentPrimVarNameParameter;
}

ConstStringParameterPtr MeshTangentsOp::vTangentPrimVarNameParameter() const
{
	return m_vTangentPrimVarNameParameter;
}

struct MeshTangentsOp::CalculateTangents
{
	typedef void ReturnType;

	CalculateTangents( ConstIntVectorDataPtr vertsPerFace, ConstIntVectorDataPtr vertIds, ConstFloatVectorDataPtr uData, ConstFloatVectorDataPtr vData )
		:	m_vertsPerFace( vertsPerFace ), m_vertIds( vertIds ), m_uData( uData ), m_vData( vData )
	{

	}

	template<typename T>
	ReturnType operator()( typename T::Ptr data )
	{
		typedef typename T::ValueType VecContainer;
		typedef typename VecContainer::value_type Vec;

		const IntVectorData::ValueType::size_type numFaces = m_vertsPerFace->readable().size();

		const typename T::ValueType &points = data->readable();
		const IntVectorData::ValueType &vertIds = m_vertIds->readable();

		typename T::Ptr uTangentData = new T();
		m_uTangentData = uTangentData;
		typename T::Ptr vTangentData = new T();
		m_vTangentData = vTangentData;

		VecContainer &uTangent = uTangentData->writable();
		VecContainer &vTangent = vTangentData->writable();

		uTangent.resize( points.size(), Vec( 0 ) );
		vTangent.resize( points.size(), Vec( 0 ) );

		typename T::Ptr normalsData = new T;
		VecContainer &normals = normalsData->writable();
		normals.resize( points.size(), Vec( 0 ) );

		const int *vertId = &(vertIds[0]);
		for ( IntVectorData::ValueType::size_type f = 0; f < numFaces; f++ )
		{
			assert( m_vertsPerFace->readable()[f] == 3 );

			assert( f*3 + 2 < m_vertIds->readable().size() );
			const int &v0 = m_vertIds->readable()[ f*3 + 0 ];
			const int &v1 = m_vertIds->readable()[ f*3 + 1 ];
			const int &v2 = m_vertIds->readable()[ f*3 + 2 ];

			const Vec &p0 = data->readable()[ v0 ];
			const Vec &p1 = data->readable()[ v1 ];
			const Vec &p2 = data->readable()[ v2 ];

			assert( f*3 + 2 < m_uData->readable().size() );
			assert( f*3 + 2 < m_vData->readable().size() );
			const Imath::V2f uv0( m_uData->readable()[ f*3 + 0 ], m_vData->readable()[ f*3 + 0 ] );
			const Imath::V2f uv1( m_uData->readable()[ f*3 + 1 ], m_vData->readable()[ f*3 + 1 ] );
			const Imath::V2f uv2( m_uData->readable()[ f*3 + 2 ], m_vData->readable()[ f*3 + 2 ] );

			const Vec e0 = p1 - p0;
			const Vec e1 = p2 - p0;

			const Imath::V2f e0uv = uv1 - uv0;
			const Imath::V2f e1uv = uv2 - uv0;

			Vec tangent   = ( e0 * -e1uv.y + e1 * e0uv.y ).normalized();
			Vec bitangent = ( e0 * -e1uv.x + e1 * e0uv.x ).normalized();

			Vec normal = (p2-p1).cross(p0-p1);
			normal.normalize();

			for ( unsigned i = 0; i < 3; i++ )
			{
				uTangent[*vertId] += tangent;
				vTangent[*vertId] += bitangent;

				normals[*vertId] += normal;

				vertId++;
			}
		}

		for ( typename T::ValueType::size_type i = 0; i < points.size(); i++ )
		{
			assert( i < uTangentData->readable().size() );
			assert( i < vTangentData->readable().size() );

			normals[i].normalize();

			uTangent[i].normalize();
			vTangent[i].normalize();

			/// Make uTangent/vTangent orthogonal to normal
			uTangent[i] -= normals[i] * uTangent[i].dot( normals[i] );
			vTangent[i] -= normals[i] * vTangent[i].dot( normals[i] );
		}
	}

	DataPtr m_uTangentData;
	DataPtr m_vTangentData;

	private :

		ConstIntVectorDataPtr m_vertsPerFace;
		ConstIntVectorDataPtr m_vertIds;
		ConstFloatVectorDataPtr m_uData;
		ConstFloatVectorDataPtr m_vData;
};

struct MeshTangentsOp::HandleErrors
{
	template<typename T, typename F>
	void operator()( typename T::ConstPtr d, const F &f )
	{
		string e = boost::str( boost::format( "MeshTangentsOp : \"P\" has unsupported data type \"%s\"." ) % d->typeName() );
		throw InvalidArgumentException( e );
	}
};

void MeshTangentsOp::modifyTypedPrimitive( MeshPrimitivePtr mesh, ConstCompoundObjectPtr operands )
{
	PrimitiveVariableMap::const_iterator pvIt = mesh->variables.find( "P" );
	if( pvIt==mesh->variables.end() || !pvIt->second.data )
	{
		throw InvalidArgumentException( "MeshTangentsOp : MeshPrimitive has no \"P\" primitive variable." );
	}

	if( !mesh->arePrimitiveVariablesValid( ) )
	{
		throw InvalidArgumentException( "MeshTangentsOp : MeshPrimitive variables are invalid." );
	}

	DataPtr pData = pvIt->second.data;

	ConstIntVectorDataPtr vertsPerFace = mesh->verticesPerFace();

	for ( IntVectorData::ValueType::const_iterator it = vertsPerFace->readable().begin(); it != vertsPerFace->readable().end(); ++it )
	{
		if ( *it != 3 )
		{
			throw InvalidArgumentException( "MeshTangentsOp : MeshPrimitive has non-triangular faces." );
		}
	}

	const std::string &uPrimVarName = uPrimVarNameParameter()->getTypedValue();
	const std::string &vPrimVarName = vPrimVarNameParameter()->getTypedValue();

	PrimitiveVariableMap::const_iterator uIt = mesh->variables.find( uPrimVarName );
	if ( uIt == mesh->variables.end() )
	{
		throw InvalidArgumentException( ( boost::format( "MeshTangentsOp : MeshPrimitive has no \"%s\" primitive variable."  ) % ( uPrimVarName ) ).str() );
	}

	PrimitiveVariableMap::const_iterator vIt = mesh->variables.find( vPrimVarName );
	if ( vIt == mesh->variables.end() )
	{
		throw InvalidArgumentException( ( boost::format( "MeshTangentsOp : MeshPrimitive has no \"%s\" primitive variable."  ) % ( vPrimVarName ) ).str() );
	}

	FloatVectorDataPtr uData = runTimeCast< FloatVectorData > ( uIt->second.data );
	if ( !uData )
	{
		throw InvalidArgumentException( ( boost::format( "MeshTangentsOp : MeshPrimitive primitive variable \"%s\" is not of type FloatVectorData."  ) % ( uPrimVarName ) ).str() );
	}

	FloatVectorDataPtr vData = runTimeCast< FloatVectorData > ( vIt->second.data );
	if ( !uData )
	{
		throw InvalidArgumentException( ( boost::format( "MeshTangentsOp : MeshPrimitive primitive variable \"%s\" is not of type FloatVectorData."  ) % ( vPrimVarName ) ).str() );
	}

	if ( uIt->second.interpolation != PrimitiveVariable::FaceVarying )
	{
		throw InvalidArgumentException( ( boost::format( "MeshTangentsOp : MeshPrimitive primitive variable \"%s\" must have FaceVarying interpolation."  ) % ( uPrimVarName ) ).str() );
	}

	if ( vIt->second.interpolation != PrimitiveVariable::FaceVarying )
	{
		throw InvalidArgumentException( ( boost::format( "MeshTangentsOp : MeshPrimitive primitive variable \"%s\" must have FaceVarying interpolation."  ) % ( vPrimVarName ) ).str() );
	}

	DataCastOpPtr dco = new DataCastOp();
	dco->targetTypeParameter()->setNumericValue( FloatVectorDataTypeId );

	dco->objectParameter()->setValue( uData );
	FloatVectorDataPtr uFloatData = runTimeCast< FloatVectorData >( dco->operate() );
	if ( uFloatData->readable().size() != mesh->variableSize( PrimitiveVariable::FaceVarying  ) )
	{
		throw InvalidArgumentException( ( boost::format( "MeshTangentsOp : MeshPrimitive primitive variable \"%s\" must have be of simple numeric type."  ) % ( uPrimVarName ) ).str() );
	}

	dco->objectParameter()->setValue( vData );
	FloatVectorDataPtr vFloatData = runTimeCast< FloatVectorData >( dco->operate() );
	if ( vFloatData->readable().size() != mesh->variableSize( PrimitiveVariable::FaceVarying  ) )
	{
		throw InvalidArgumentException( ( boost::format( "MeshTangentsOp : MeshPrimitive primitive variable \"%s\" must have be of simple numeric type."  ) % ( vPrimVarName ) ).str() );
	}

	CalculateTangents f( vertsPerFace, mesh->vertexIds(), uFloatData, vFloatData );

	despatchTypedData<CalculateTangents, TypeTraits::IsVec3VectorTypedData, HandleErrors>( pData, f );

	mesh->variables[ uTangentPrimVarNameParameter()->getTypedValue() ] = PrimitiveVariable( PrimitiveVariable::Varying, f.m_uTangentData->copy() );
	mesh->variables[ vTangentPrimVarNameParameter()->getTypedValue() ] = PrimitiveVariable( PrimitiveVariable::Varying, f.m_vTangentData->copy() );

	assert( mesh->arePrimitiveVariablesValid() );
}


