//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CapturingRenderer.h"
#include "IECore/Group.h"
#include "IECore/Op.h"
#include "IECore/ParameterisedProcedural.h"
#include "IECore/WorldBlock.h"
#include "IECorePython/ScopedGILLock.h"
#include "IECorePython/ScopedGILRelease.h"

#include "IECoreHoudini/CoreHoudini.h"
#include "IECoreHoudini/NodePassData.h"
#include "IECoreHoudini/ToHoudiniGeometryConverter.h"
#include "IECoreHoudini/SOP_ParameterisedHolder.h"
#include "IECoreHoudini/SOP_ToHoudiniConverter.h"

using namespace IECoreHoudini;

PRM_Template SOP_ToHoudiniConverter::parameters[] = {
	PRM_Template()
};

CH_LocalVariable SOP_ToHoudiniConverter::variables[] = {
	{ 0, 0, 0 },
};

OP_Node *SOP_ToHoudiniConverter::create( OP_Network *net, const char *name, OP_Operator *op )
{
	return new SOP_ToHoudiniConverter( net, name, op );
}

SOP_ToHoudiniConverter::SOP_ToHoudiniConverter( OP_Network *net, const char *name, OP_Operator *op )
	: SOP_Node(net, name, op)
{
}

SOP_ToHoudiniConverter::~SOP_ToHoudiniConverter()
{
}

OP_ERROR SOP_ToHoudiniConverter::cookMySop( OP_Context &context )
{
	if( lockInputs( context ) >= UT_ERROR_ABORT )
	{
		return error();
	}

	UT_Interrupt *boss = UTgetInterrupt();
	boss->opStart("Building ToHoudiniConverter Geometry...");
	gdp->clearAndDestroy();

	GU_DetailHandleAutoReadLock readHandle( inputGeoHandle(0) );
	const GU_Detail *inputGeo = readHandle.getGdp();
	if ( !inputGeo )
	{
		addError( SOP_MESSAGE, "Input Geo was not readable" );
	    	boss->opEnd();
	    	return error();
	}
	
	const GA_ROAttributeRef attrRef = inputGeo->findAttribute( GA_ATTRIB_DETAIL, GA_SCOPE_PRIVATE, "IECoreHoudiniNodePassData" );
	if ( attrRef.isInvalid() )
	{
		addError( SOP_MESSAGE, "Could not find Cortex Object on input geometry!" );
		boss->opEnd();
		return error();
	}
	
	const GA_Attribute *attr = attrRef.getAttribute();
	const GA_AIFBlindData *blindData = attr->getAIFBlindData();
	const NodePassData passData = blindData->getValue<NodePassData>( attr, 0 );
	
	IECore::ConstVisibleRenderablePtr renderable = 0;
	SOP_ParameterisedHolder *sop = dynamic_cast<SOP_ParameterisedHolder*>( const_cast<OP_Node*>( passData.nodePtr() ) );
	
	if ( passData.type() == IECoreHoudini::NodePassData::CORTEX_OPHOLDER )
	{
		IECore::Op *op = IECore::runTimeCast<IECore::Op>( sop->getParameterised() );
		renderable = IECore::runTimeCast<const IECore::VisibleRenderable>( op->resultParameter()->getValue() );
	}
	else if ( passData.type() == IECoreHoudini::NodePassData::CORTEX_PROCEDURALHOLDER )
	{
		IECore::ParameterisedProcedural *procedural = IECore::runTimeCast<IECore::ParameterisedProcedural>( sop->getParameterised() );
		IECore::CapturingRendererPtr renderer = new IECore::CapturingRenderer();
		// We are acquiring and releasing the GIL here to ensure that it is released when we render. This has
		// to be done because a procedural might jump between c++ and python a few times (i.e. if it spawns
		// subprocedurals that are implemented in python). In a normal call to cookMySop, this wouldn't be an
		// issue, but if cookMySop was called from HOM, hou.Node.cook appears to be holding onto the GIL.
		IECorePython::ScopedGILLock gilLock;
		{
			IECorePython::ScopedGILRelease gilRelease;
			{
				IECore::WorldBlock worldBlock( renderer );
				procedural->render( renderer );
			}
		}
		renderable = renderer->world();
	}
	else
	{
		addError( SOP_MESSAGE, "Input node is not a recognized Cortex type" );
		boss->opEnd();
		return error();
	}
	
	if ( !renderable )
	{
		addError( SOP_MESSAGE, "Input Cortex data could not be converted to Houdini Geo" );
		boss->opEnd();
		return error();
	}

	ToHoudiniGeometryConverterPtr converter = ToHoudiniGeometryConverter::create( renderable );
	if ( !converter->convert( myGdpHandle ) )
	{
		addError( SOP_MESSAGE, "Input Cortex data could not be converted to Houdini Geo" );
		boss->opEnd();
		return error();
	}

	boss->opEnd();
	unlockInputs();
	return error();
}

const char *SOP_ToHoudiniConverter::inputLabel( unsigned pos ) const
{
	return "Cortex Primitive";
}
