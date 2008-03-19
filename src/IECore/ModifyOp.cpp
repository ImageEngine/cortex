//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ModifyOp.h"
#include "IECore/CompoundObject.h"
#include "IECore/CompoundParameter.h"

using namespace IECore;

ModifyOp::ModifyOp( const std::string name, const std::string description, ParameterPtr resultParameter, ParameterPtr inputParameter )
	:	Op( name, description, resultParameter )
{
	parameters()->addParameter( inputParameter );
	m_inputParameter = inputParameter;
	
	m_copyParameter = new BoolParameter(
		"copyInput",
		"When this is on the input is copied before being modified. When off the object is modified in place.",
		true
	);
	parameters()->addParameter( m_copyParameter );
	
	m_enableParameter = new BoolParameter(
		"enable",
		"When this is off the input is passed through unchanged.",
		true
	
	);
	parameters()->addParameter( m_enableParameter );
}

ModifyOp::~ModifyOp()
{
}

ParameterPtr ModifyOp::inputParameter()
{
	return m_inputParameter;
}

ConstParameterPtr ModifyOp::inputParameter() const
{
	return m_inputParameter;
}

BoolParameterPtr ModifyOp::copyParameter()
{
	return m_copyParameter;
}

BoolParameterPtr ModifyOp::copyParameter() const
{
	return m_copyParameter;
}

BoolParameterPtr ModifyOp::enableParameter()
{
	return m_enableParameter;
}

BoolParameterPtr ModifyOp::enableParameter() const
{
	return m_enableParameter;
}

ObjectPtr ModifyOp::doOperation( ConstCompoundObjectPtr operands )
{
	ObjectPtr object = m_inputParameter->getValue();
	if( m_copyParameter->getTypedValue() )
	{
		object = object->copy();
	}
	if( m_enableParameter->getTypedValue() )
	{
		modify( object, operands );
	}
	return object;
}
