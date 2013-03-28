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

#include "GA/GA_AIFBlindData.h"
#include "PRM/PRM_Parm.h"
#include "UT/UT_Interrupt.h"

#include "IECore/Op.h"
#include "IECore/VisibleRenderable.h"

#include "IECorePython/ScopedGILLock.h" 

#include "IECoreHoudini/SOP_OpHolder.h"
#include "IECoreHoudini/NodePassData.h"

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
	
	setParameterisedValues( now );
	
	try
	{
		// make our Cortex op do it's thing...
		op->operate();

		// pass ourselves onto the GR_Cortex render hook
		IECoreHoudini::NodePassData data( this, IECoreHoudini::NodePassData::CORTEX_OPHOLDER );
		GA_RWAttributeRef attrRef = gdp->createAttribute( GA_ATTRIB_DETAIL, GA_SCOPE_PRIVATE, "IECoreHoudiniNodePassData", NULL, NULL, "blinddata" );
		GA_Attribute *attr = attrRef.getAttribute();
		const GA_AIFBlindData *blindData = attr->getAIFBlindData();
		blindData->setDataSize( attr, sizeof(IECoreHoudini::NodePassData), &data );

		// if our result is a visible renderable then set our bounds on our output gdp
		const IECore::Object *result = op->resultParameter()->getValue();
		IECore::ConstVisibleRenderablePtr renderable = IECore::runTimeCast<const IECore::VisibleRenderable>( result );
		if ( renderable )
		{
			Imath::Box3f bbox = renderable->bound();
			gdp->cube( bbox.min.x, bbox.max.x, bbox.min.y, bbox.max.y, bbox.min.z, bbox.max.z, 0, 0, 0, 1, 1 );
		}
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

	// tidy up & go home!
	boss->opEnd();
	unlockInputs();
	return error();
}
