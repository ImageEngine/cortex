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

#include "IECoreScene/MeshAlgo.h"
#include "IECoreScene/MeshNormalsOp.h"

#include "IECore/CompoundParameter.h"
#include "IECore/DespatchTypedData.h"

#include "boost/format.hpp"

using namespace IECore;
using namespace IECoreScene;
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

	IntParameter::PresetsContainer interpolationPresets;
	interpolationPresets.push_back( IntParameter::Preset( "Vertex", PrimitiveVariable::Vertex ) );
	interpolationPresets.push_back( IntParameter::Preset( "Uniform", PrimitiveVariable::Uniform ) );
	IntParameterPtr interpolationParameter;
	interpolationParameter = new IntParameter(
		"interpolation",
		"The primitive variable interpolation type for the calculated normals.",
		PrimitiveVariable::Vertex,
		interpolationPresets
	);

	parameters()->addParameter( pPrimVarNameParameter );
	parameters()->addParameter( nPrimVarNameParameter );
	parameters()->addParameter( interpolationParameter );
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

IntParameter * MeshNormalsOp::interpolationParameter()
{
	return parameters()->parameter<IntParameter>( "interpolation" );
}

const IntParameter * MeshNormalsOp::interpolationParameter() const
{
	return parameters()->parameter<IntParameter>( "interpolation" );
}

void MeshNormalsOp::modifyTypedPrimitive( MeshPrimitive * mesh, const CompoundObject * operands )
{
	const std::string &pPrimVarName = pPrimVarNameParameter()->getTypedValue();
	const PrimitiveVariable::Interpolation interpolation = static_cast<PrimitiveVariable::Interpolation>( operands->member<IntData>( "interpolation" )->readable() );
	mesh->variables[ nPrimVarNameParameter()->getTypedValue() ] = MeshAlgo::calculateNormals( mesh, interpolation, pPrimVarName );
}
