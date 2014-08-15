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

#include "IECore/TransformOp.h"
#include "IECore/MatrixMultiplyOp.h"
#include "IECore/MessageHandler.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Primitive.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( TransformOp );

TransformOp::TransformOp()
	:	PrimitiveOp( "Applies a matrix transformation to primitive variables." )
{
	m_multiplyOp = new MatrixMultiplyOp;
	m_multiplyOp->copyParameter()->setTypedValue( false );

	StringVectorDataPtr defaultPrimVars = new StringVectorData;
	defaultPrimVars->writable().push_back( "P" );
	defaultPrimVars->writable().push_back( "N" );
	m_primVarsParameter = new StringVectorParameter(
		"primVarsToModify",
		"The names of primitive variables which should be transformed according to their Geometric Interpretation.",
		defaultPrimVars
	);
	
	parameters()->addParameter( m_multiplyOp->matrixParameter() );
	parameters()->addParameter( m_primVarsParameter );
}

ObjectParameter * TransformOp::matrixParameter()
{
	return m_multiplyOp->matrixParameter();
}

const ObjectParameter * TransformOp::matrixParameter() const
{
	return m_multiplyOp->matrixParameter();
}

StringVectorParameter * TransformOp::primVarsParameter()
{
	return m_primVarsParameter;
}

const StringVectorParameter * TransformOp::primVarsParameter() const
{
	return m_primVarsParameter;
}

void TransformOp::modifyPrimitive( Primitive * primitive, const CompoundObject * operands )
{
	const std::vector<std::string> &pv = m_primVarsParameter->getTypedValue();
	std::set< DataPtr > visitedData;
	for ( std::vector<std::string>::const_iterator it = pv.begin(); it != pv.end(); ++it )
	{
		PrimitiveVariableMap::iterator pIt = primitive->variables.find( *it );
		if ( pIt == primitive->variables.end() || !pIt->second.data || visitedData.find( pIt->second.data ) != visitedData.end() )
		{
			continue;
		}
		
		Data *data = pIt->second.data;
		visitedData.insert( data );
		
		// fix for old files that don't store Interpretation properly
		V3fVectorData *v3fData = runTimeCast<V3fVectorData>( data );
		if ( v3fData )
		{
			GeometricData::Interpretation interp = v3fData->getInterpretation();
			
			if ( interp == GeometricData::Numeric )
			{
				if ( *it == "P" )
				{
					IECore::msg( IECore::MessageHandler::Warning, "TransformOp", "Primitive contains Numeric data named P. Converting to Point data." );
					v3fData->setInterpretation( GeometricData::Point );
				}
				else if ( *it == "N" )
				{
					IECore::msg( IECore::MessageHandler::Warning, "TransformOp", "Primitive contains Numeric data named N. Converting to Normal data." );
					v3fData->setInterpretation( GeometricData::Normal );
				}
			}
		}
		
		m_multiplyOp->inputParameter()->setValue( pIt->second.data );
		m_multiplyOp->operate();
	}
}
