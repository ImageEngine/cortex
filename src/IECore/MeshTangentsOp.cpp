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
#include "boost/regex.hpp"

#include "IECore/DataCastOp.h"
#include "IECore/Convert.h"
#include "IECore/MeshTangentsOp.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/CompoundParameter.h"
#include "IECore/MeshAlgo.h"

using namespace IECore;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( MeshTangentsOp );

namespace
{

std::string makeUVSetName( const std::string &uPrimVarName, const std::string &vPrimVarName )
{
	if( uPrimVarName == "s" && vPrimVarName == "t" )
	{
		return "st";
	}

	static const boost::regex expression( "([a-zA-Z0-9_]+)_([st])" );

	boost::cmatch uName;
	boost::cmatch vName;

	boost::regex_match( uPrimVarName.c_str(), uName, expression );
	boost::regex_match( vPrimVarName.c_str(), vName, expression );

	if( uName[0] == vName[0] )
	{
		return uName[0];
	}

	std::string msg = (boost::format( "MeshTangentsOp : Invalid UV coordinate primVar Names '%s', '%s'" ) % uPrimVarName % vPrimVarName ).str();
	throw IECore::InvalidArgumentException( msg );
}

}

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

	if( !IECore::runTimeCast<V3fVectorData>( pData ) )
	{
		string e = boost::str(
			boost::format( "MeshTangentsOp : MeshPrimitive Vertex \"%s\" primitive variable is incorrect type. Only V3fVectorData supported." ) %
			pData->typeName()
		);
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

	std::string uvSetName = makeUVSetName( uPrimVarName, vPrimVarName );
	std::string expecteduvIndicesName = uvSetName + "Indices";

	if (uvIndicesPrimVarName != "" && uvIndicesPrimVarName != expecteduvIndicesName)
	{
		string e = boost::str( boost::format( "MeshTangentsOp : uvIndicesPrimVarName is '%s' but must be set to: '%s' ") % uvIndicesPrimVarName % expecteduvIndicesName );
		throw InvalidArgumentException( e );
	}

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
	if( uvIndicesPrimVarName == "" )
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

	std::pair<IECore::PrimitiveVariable, IECore::PrimitiveVariable> tangentPrimVars = IECore::MeshAlgo::calculateTangents(
		mesh,
		uvSetName,
		orthoTangents,
		pPrimVarName
	);

	mesh->variables[ uTangentPrimVarNameParameter()->getTypedValue() ] = tangentPrimVars.first;
	mesh->variables[ vTangentPrimVarNameParameter()->getTypedValue() ] = tangentPrimVars.second;

	assert( mesh->arePrimitiveVariablesValid() );
}


