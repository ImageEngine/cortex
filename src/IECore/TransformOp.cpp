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

#include "IECore/TransformOp.h"
#include "IECore/MatrixMultiplyOp.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Primitive.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( TransformOp );

TransformOp::TransformOp()
	:	PrimitiveOp( staticTypeName(), "Applies a matrix transformation to primitive variables." )
{
	m_multiplyOp = new MatrixMultiplyOp;
	m_multiplyOp->copyParameter()->setTypedValue( false );

	StringVectorDataPtr pDefault = new StringVectorData;
	pDefault->writable().push_back( "P" );
	m_pointPrimVarsParameter = new StringVectorParameter(
		"pointPrimVars",
		"The names of primitive variables which should be transformed as points.",
		pDefault
	);

	m_vectorPrimVarsParameter = new StringVectorParameter(
		"vectorPrimVars",
		"The names of primitive variables which should be transformed as vectors.",
		new StringVectorData
	);

	StringVectorDataPtr nDefault = new StringVectorData;
	nDefault->writable().push_back( "N" );
	m_normalPrimVarsParameter = new StringVectorParameter(
		"normalPrimVars",
		"The names of primitive variables which should be transformed as normals.",
		nDefault
	);

	parameters()->addParameter( m_multiplyOp->matrixParameter() );
	parameters()->addParameter( m_pointPrimVarsParameter );
	parameters()->addParameter( m_vectorPrimVarsParameter );
	parameters()->addParameter( m_normalPrimVarsParameter );
}

ObjectParameterPtr TransformOp::matrixParameter()
{
	return m_multiplyOp->matrixParameter();
}

ConstObjectParameterPtr TransformOp::matrixParameter() const
{
	return m_multiplyOp->matrixParameter();
}

StringVectorParameterPtr TransformOp::pointPrimVarsParameter()
{
	return m_pointPrimVarsParameter;
}

ConstStringVectorParameterPtr TransformOp::pointPrimVarsParameter() const
{
	return m_pointPrimVarsParameter;
}

StringVectorParameterPtr TransformOp::vectorPrimVarsParameter()
{
	return m_vectorPrimVarsParameter;
}

ConstStringVectorParameterPtr TransformOp::vectorPrimVarsParameter() const
{
	return m_vectorPrimVarsParameter;
}

StringVectorParameterPtr TransformOp::normalPrimVarsParameter()
{
	return m_normalPrimVarsParameter;
}

ConstStringVectorParameterPtr TransformOp::normalPrimVarsParameter() const
{
	return m_normalPrimVarsParameter;
}

void TransformOp::modifyPrimitive( PrimitivePtr primitive, ConstCompoundObjectPtr operands )
{
	m_multiplyOp->modeParameter()->setNumericValue( MatrixMultiplyOp::Point );
	const std::vector<std::string> &p = m_pointPrimVarsParameter->getTypedValue();
	for( std::vector<std::string>::const_iterator it = p.begin(); it!=p.end(); it++ )
	{
		PrimitiveVariableMap::iterator pIt = primitive->variables.find( *it );
		if( pIt!=primitive->variables.end() && pIt->second.data )
		{
			m_multiplyOp->inputParameter()->setValue( pIt->second.data );
			m_multiplyOp->operate();
		}
	}

	m_multiplyOp->modeParameter()->setNumericValue( MatrixMultiplyOp::Vector );
	const std::vector<std::string> &v = m_vectorPrimVarsParameter->getTypedValue();
	for( std::vector<std::string>::const_iterator it = v.begin(); it!=v.end(); it++ )
	{
		PrimitiveVariableMap::iterator pIt = primitive->variables.find( *it );
		if( pIt!=primitive->variables.end() && pIt->second.data )
		{
			m_multiplyOp->inputParameter()->setValue( pIt->second.data );
			m_multiplyOp->operate();
		}
	}

	m_multiplyOp->modeParameter()->setNumericValue( MatrixMultiplyOp::Normal );
	const std::vector<std::string> &n = m_normalPrimVarsParameter->getTypedValue();
	for( std::vector<std::string>::const_iterator it = n.begin(); it!=n.end(); it++ )
	{
		PrimitiveVariableMap::iterator pIt = primitive->variables.find( *it );
		if( pIt!=primitive->variables.end() && pIt->second.data )
		{
			m_multiplyOp->inputParameter()->setValue( pIt->second.data );
			m_multiplyOp->operate();
		}
	}
}
