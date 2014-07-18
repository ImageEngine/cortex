//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#include "IECoreNuke/OpHolder.h"

using namespace IECoreNuke;

static IECore::ObjectPtr g_lastExecuteResult = 0;

const DD::Image::Op::Description OpHolder::g_description( "ieOp", build );

OpHolder::OpHolder( Node *node )
	:	ParameterisedHolderOp( node ),
		DD::Image::Executable( this ),
		m_result( 0 )
{
}

OpHolder::~OpHolder()
{
}

IECore::ObjectPtr OpHolder::engine()
{
	if( m_result && hash()==m_resultHash )
	{
		return m_result;
	}
	
	IECore::ConstOpPtr constOp = IECore::runTimeCast<const IECore::Op>( parameterised() );
	if( !constOp )
	{
		return 0;
	}

	/// \todo operate() should be const, then we wouldn't need this cast.
	IECore::OpPtr op = boost::const_pointer_cast<IECore::Op>( constOp );

	setParameterValues(); /// \todo is this really needed?? didn't we do that in validate()?
	setParameterValuesFromInputs(); /// \todo Should this be done by an engine() call on the base class?
	
	m_result = op->operate();
	m_resultHash = hash();
	
	return m_result;
}

DD::Image::Op *OpHolder::build( Node *node )
{
	return new OpHolder( node );
}

const char *OpHolder::Class() const
{
	return g_description.name;
}

const char *OpHolder::node_help() const
{
	return "Executes Cortex Ops.";
}

DD::Image::Executable *OpHolder::executable()
{
	return this;
}

bool OpHolder::isExecuteThreadSafe() const
{
	return false;
}

void OpHolder::execute()
{
	IECore::ObjectPtr result = engine();
	g_lastExecuteResult = result;
}

bool OpHolder::isWrite()
{
	return false;
}

IECore::ObjectPtr OpHolder::executeResult()
{
	IECore::ObjectPtr result = g_lastExecuteResult;
	g_lastExecuteResult = 0;
	return result;
}
