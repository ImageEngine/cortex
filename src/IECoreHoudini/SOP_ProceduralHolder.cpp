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
#include "UT/UT_Interrupt.h"
#include "PRM/PRM_Parm.h"

#include "IECore/ParameterisedProcedural.h"
#include "IECore/SimpleTypedData.h"

#include "IECorePython/ScopedGILLock.h"

#include "IECoreGL/Renderer.h"
#include "IECoreGL/Camera.h"

#include "IECoreHoudini/NodePassData.h"
#include "IECoreHoudini/SOP_ProceduralHolder.h"

using namespace boost::python;
using namespace IECoreHoudini;

OP_Node *SOP_ProceduralHolder::create( OP_Network *net, const char *name, OP_Operator *op )
{
	return new SOP_ProceduralHolder( net, name, op );
}

SOP_ProceduralHolder::SOP_ProceduralHolder( OP_Network *net, const char *name, OP_Operator *op ) : SOP_ParameterisedHolder( net, name, op ), m_scene( 0 )
{
	getParm( pParameterisedSearchPathEnvVar.getToken() ).setValue( 0, "IECORE_PROCEDURAL_PATHS", CH_STRING_LITERAL );
}

SOP_ProceduralHolder::~SOP_ProceduralHolder()
{
}

/// Redraws the OpenGL Scene if the procedural is marked as having changed (aka dirty)
IECoreGL::ConstScenePtr SOP_ProceduralHolder::scene()
{
	IECore::ParameterisedProceduralPtr procedural = IECore::runTimeCast<IECore::ParameterisedProcedural>( getParameterised() );
	if ( !procedural )
	{
		return 0;
	}
	
	if ( m_dirty || !m_scene )
	{
		IECorePython::ScopedGILLock gilLock;
		try
		{
			IECoreGL::RendererPtr renderer = new IECoreGL::Renderer();
			renderer->setOption( "gl:mode", new IECore::StringData( "deferred" ) );
			renderer->worldBegin();
			procedural->render( renderer );
			renderer->worldEnd();
			m_scene = renderer->scene();
			m_scene->setCamera( 0 ); // houdini will be providing the camera when we draw the scene
		}
		catch( const std::exception &e )
		{
			std::cerr << e.what() << std::endl;
		}
		catch( ... )
		{
			std::cerr << "Unknown!" << std::endl;
		}

		m_dirty = false;
	}
	
	return m_scene;
}

/// Cook the SOP! This method does all the work
OP_ERROR SOP_ProceduralHolder::cookMySop( OP_Context &context )
{
	IECore::MessageHandler::Scope handlerScope( getMessageHandler() );
	
	// some defaults and useful variables
	float now = context.getTime();

	// force eval of our nodes parameters with our hidden parameter expression
	evalInt( "__evaluateParameters", 0, now );

	// update parameters on procedural from our Houdini parameters
	IECore::ParameterisedProceduralPtr procedural = IECore::runTimeCast<IECore::ParameterisedProcedural>( getParameterised() );

	// check for a valid parameterised on this SOP
	if ( !procedural )
	{
		UT_String msg( "Procedural Holder has no parameterised class to operate on!" );
		addError( SOP_MESSAGE, msg );
		return error();
	}
	
	if( lockInputs(context) >= UT_ERROR_ABORT )
	{
		return error();
	}
	
	// start our work
	UT_Interrupt *boss = UTgetInterrupt();
	boss->opStart("Building ProceduralHolder Geometry...");
	gdp->clearAndDestroy();
	
	setParameterisedValues( now );
	
	try
	{
		// put our cortex passdata on our gdp as a detail attribute
		IECoreHoudini::NodePassData data( this, IECoreHoudini::NodePassData::CORTEX_PROCEDURALHOLDER );
		GA_RWAttributeRef attrRef = gdp->createAttribute( GA_ATTRIB_DETAIL, GA_SCOPE_PRIVATE, "IECoreHoudiniNodePassData", NULL, NULL, "blinddata" );
		GA_Attribute *attr = attrRef.getAttribute();
		const GA_AIFBlindData *blindData = attr->getAIFBlindData();
		blindData->setDataSize( attr, sizeof(IECoreHoudini::NodePassData), &data );

		// calculate our bounding box
		Imath::Box3f bbox = procedural->bound();
		gdp->cube( bbox.min.x, bbox.max.x, bbox.min.y, bbox.max.y, bbox.min.z, bbox.max.z, 0, 0, 0, 1, 1 );
	}
	catch( boost::python::error_already_set )
	{
		addError( SOP_MESSAGE, "Error raised during Python evaluation!" );
		IECorePython::ScopedGILLock lock;
		PyErr_Print();
	}
	catch( const std::exception &e )
	{
		addError( SOP_MESSAGE, e.what() );
	}
	catch( ... )
	{
		addError( SOP_MESSAGE, "Procedural::bound() Caught unknown exception!" );
	}

	// tidy up & go home!
	boss->opEnd();
	unlockInputs();
	return error();
}
