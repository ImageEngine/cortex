//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#include "PRM/PRM_Parm.h"
#include "UT/UT_Interrupt.h"

#include "IECorePython/ScopedGILLock.h" 

#include "IECoreHoudini/DetailSplitter.h"
#include "IECoreHoudini/SOP_OpHolder.h"
#include "IECoreHoudini/ToHoudiniCortexObjectConverter.h"

using namespace boost::python;
using namespace IECoreHoudini;

OP_Node *SOP_OpHolder::create( OP_Network *net, const char *name, OP_Operator *op )
{
    return new SOP_OpHolder( net, name, op );
}

SOP_OpHolder::SOP_OpHolder( OP_Network *net, const char *name, OP_Operator *op ) : SOP_ParameterisedHolder( net, name, op )
{
	getParm( pParameterisedSearchPathEnvVar.getToken() ).setValue( 0, "IECORE_OP_PATHS", CH_STRING_LITERAL );
}

SOP_OpHolder::~SOP_OpHolder()
{
}

/// Cook the SOP! This method does all the work
OP_ERROR SOP_OpHolder::cookMySop( OP_Context &context )
{
	IECore::MessageHandler::Scope handlerScope( getMessageHandler() );
	
	// some defaults and useful variables
	Imath::Box3f bbox( Imath::V3f(-1,-1,-1), Imath::V3f(1,1,1) );
	float now = context.getTime();

	// force eval of our nodes parameters with our hidden parameter expression
	evalInt( "__evaluateParameters", 0, now );

	// get our op
	IECore::OpPtr op = IECore::runTimeCast<IECore::Op>( getParameterised() );
	
	// check for a valid parameterised on this SOP
	if ( !op )
	{
		UT_String msg( "Op Holder has no parameterised class to operate on!" );
		addError( SOP_MESSAGE, msg );
		return error();
	}

	if( lockInputs(context)>=UT_ERROR_ABORT )
	{
		return error();
	}

	// start our work
	UT_Interrupt *boss = UTgetInterrupt();
	boss->opStart("Building OpHolder Geometry...");
	gdp->clearAndDestroy();
	
	// update the op parameters
	setParameterisedValues( now );
	
	// main input is reserved for splitting by name when the filter is enabled
	UT_StringMMPattern nameFilter;
	if ( !m_inputParameters.empty() && getNameFilter( m_inputParameters[0].get(), nameFilter ) ) 
	{
		DetailSplitterPtr splitter = new DetailSplitter( inputGeoHandle( 0 ) );
		std::vector<std::string> names;
		splitter->values( names );
		for ( std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); ++it )
		{
			const std::string &name = *it;
			// we want match all to also match no-name
			if ( UT_String( name ).multiMatch( nameFilter ) || ( name == "" && UT_String( "*" ).multiMatch( nameFilter ) ) )
			{
				doOperation( op.get(), splitter->split( name ), name );
			}
			else
			{
				doPassThrough( splitter->split( name ), name );
			}
		}
	}
	else
	{
		doOperation( op.get(), GU_DetailHandle(), "" );
	}
	
	boss->opEnd();
	unlockInputs();
	return error();
}

void SOP_OpHolder::doOperation( IECore::Op *op, const GU_DetailHandle &handle, const std::string &name )
{
	if ( !m_inputParameters.empty() )
	{
		SOP_ParameterisedHolder::setInputParameterValue( m_inputParameters[0].get(), handle, 0 );
	}
	
	IECore::ConstObjectPtr result = 0;
	
	try
	{
		result = op->operate();
	}
	catch( boost::python::error_already_set )
	{
		addError( SOP_MESSAGE, "Error raised during Python evaluation!" );
		IECorePython::ScopedGILLock lock;
		PyErr_Print();
	}
	catch( const IECore::Exception &e )
	{
		addError( SOP_MESSAGE, e.what() );
	}
	catch( const std::exception &e )
	{
		addError( SOP_MESSAGE, e.what() );
	}
	catch( ... )
	{
		addError( SOP_MESSAGE, "Caught unknown exception!" );
	}
	
	if ( result )
	{
		ToHoudiniCortexObjectConverterPtr converter = new ToHoudiniCortexObjectConverter( result.get() );
		converter->nameParameter()->setTypedValue( name );
		if ( !converter->convert( myGdpHandle ) )
		{
			addError( SOP_MESSAGE, "Unable to store op result on gdp" );
		}
	}
}

void SOP_OpHolder::doPassThrough( const GU_DetailHandle &handle, const std::string &name )
{
	if ( handle.isNull() )
	{
		addError( SOP_MESSAGE, ( "Could not pass through the geometry named " + name ).c_str() );
		return;
	}
	
	GU_DetailHandleAutoReadLock readHandle( handle );
	const GU_Detail *inputGeo = readHandle.getGdp();
	if ( !inputGeo )
	{
		addError( SOP_MESSAGE, ( "Could not pass through the geometry named " + name ).c_str() );
		return;
	}
	
	gdp->merge( *inputGeo );
}

void SOP_OpHolder::setInputParameterValue( IECore::Parameter *parameter, const GU_DetailHandle &handle, unsigned inputIndex )
{
	if ( inputIndex == 0 )
	{
		return;
	}
	
	SOP_ParameterisedHolder::setInputParameterValue( parameter, handle, inputIndex );
}
