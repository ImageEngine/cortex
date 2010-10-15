//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

// Houdini
#include "GB/GB_AttributeRef.h"
#include <OP/OP_OperatorTable.h>
#include <OP/OP_Operator.h>
#include <UT/UT_Interrupt.h>
#include <CH/CH_LocalVariable.h>

// Cortex
#include <IECore/Object.h>
#include <IECore/Op.h>
#include <IECore/TypedParameter.h>
#include <IECore/Primitive.h>

// OpenEXR
#include <OpenEXR/ImathBox.h>

// C++
#include <sstream>

// IECoreHoudini
#include "CoreHoudini.h"
#include "SOP_OpHolder.h"
#include "SOP_ToHoudiniConverter.h"
#include "NodePassData.h"
#include "ToHoudiniGeometryConverter.h"
#include "ToHoudiniPointsConverter.h"
#include "ToHoudiniPolygonsConverter.h"
using namespace IECoreHoudini;

/// Add parameters to SOP
PRM_Template SOP_ToHoudiniConverter::myParameters[] = {
		PRM_Template()
};

/// Don't worry about variables today
CH_LocalVariable SOP_ToHoudiniConverter::myVariables[] = {
		{ 0, 0, 0 },
};

/// Houdini's static creator method
OP_Node *SOP_ToHoudiniConverter::myConstructor( OP_Network *net,
										const char *name,
										OP_Operator *op )
{
    return new SOP_ToHoudiniConverter(net, name, op);
}

/// Ctor
SOP_ToHoudiniConverter::SOP_ToHoudiniConverter(OP_Network *net,
		const char *name,
		OP_Operator *op ) :
	SOP_Node(net, name, op)
{
}

/// Dtor
SOP_ToHoudiniConverter::~SOP_ToHoudiniConverter()
{
}

/// Cook the SOP! This method does all the work
OP_ERROR SOP_ToHoudiniConverter::cookMySop(OP_Context &context)
{
    if( lockInputs(context)>=UT_ERROR_ABORT )
    	return error();

	// start our work
	UT_Interrupt *boss = UTgetInterrupt();
	boss->opStart("Building ToHoudiniConverter Geometry...");
	gdp->clearAndDestroy();

	GU_DetailHandle gdp_handle = inputGeoHandle(0);
	const GU_Detail *input_gdp = gdp_handle.readLock();

	const NodePassData *pass_data = 0;
	if ( input_gdp->attribs().find("IECoreHoudini::NodePassData", GB_ATTRIB_MIXED) ) // looks like data passed from another OpHolder
	{
		GB_AttributeRef attrOffset = input_gdp->attribs().getOffset( "IECoreHoudini::NodePassData", GB_ATTRIB_MIXED );
		pass_data = input_gdp->attribs().castAttribData<NodePassData>( attrOffset );
	};

	if ( !pass_data )
	{
    	addError( SOP_MESSAGE, "Could not find Cortex Object on input geometry!" );
    	boss->opEnd();
    	return error();
	}

	if ( pass_data->type()==IECoreHoudini::NodePassData::CORTEX_OPHOLDER )
	{
		SOP_OpHolder *sop = dynamic_cast<SOP_OpHolder*>(const_cast<OP_Node*>(pass_data->nodePtr()));
		IECore::OpPtr op = IECore::runTimeCast<IECore::Op>( sop->getParameterised() );
		const IECore::Parameter *result_parameter = op->resultParameter();
		const IECore::Object *result_ptr = result_parameter->getValue();
		const IECore::Primitive *primitive = IECore::runTimeCast<const IECore::Primitive>( result_ptr );
		if ( !primitive )
		{
	    	addError( SOP_MESSAGE, "Object was not a Cortex Primitive!" );
	    	boss->opEnd();
	    	return error();
		}

		ToHoudiniGeometryConverterPtr converter = ToHoudiniGeometryConverter::create( primitive );
		if ( !converter->convert( myGdpHandle ) )
		{
	    	addError( SOP_MESSAGE, "Conversion Failed!" );
	    	boss->opEnd();
	    	return error();
		}
	}
	gdp_handle.unlock( input_gdp );

	// tidy up & go home!
	boss->opEnd();
	unlockInputs();
	return error();
}

const char *SOP_ToHoudiniConverter::inputLabel( unsigned pos ) const
{
	return "Cortex Primitive";
}
