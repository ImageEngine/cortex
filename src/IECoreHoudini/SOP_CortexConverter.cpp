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

#include "OP/OP_NodeInfoParms.h"
#include "PRM/PRM_ChoiceList.h"
#include "PRM/PRM_Default.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_StringMMPattern.h"

#include "IECore/CapturingRenderer.h"
#include "IECore/Group.h"
#include "IECore/ParameterisedProcedural.h"
#include "IECore/WorldBlock.h"
#include "IECorePython/ScopedGILLock.h"
#include "IECorePython/ScopedGILRelease.h"

#include "IECoreHoudini/DetailSplitter.h"
#include "IECoreHoudini/FromHoudiniGeometryConverter.h"
#include "IECoreHoudini/GU_CortexPrimitive.h"
#include "IECoreHoudini/ToHoudiniCortexObjectConverter.h"
#include "IECoreHoudini/ToHoudiniGeometryConverter.h"
#include "IECoreHoudini/SOP_CortexConverter.h"

using namespace IECoreHoudini;

const char *SOP_CortexConverter::typeName = "ieCortexConverter";

PRM_Name SOP_CortexConverter::pNameFilter( "nameFilter", "Name Filter" );
PRM_Name SOP_CortexConverter::pAttributeFilter( "attributeFilter", "Attribute Filter" );
PRM_Name SOP_CortexConverter::pResultType( "resultType", "Result Type" );
PRM_Name SOP_CortexConverter::pConvertStandardAttributes( "convertStandardAttributes", "Convert Standard Attributes" );

PRM_Default SOP_CortexConverter::convertStandardAttributesDefault( true );
PRM_Default SOP_CortexConverter::filterDefault( 0, "*" );
PRM_Default SOP_CortexConverter::resultTypeDefault( Houdini );

static PRM_Name resultTypes[] = {
	PRM_Name( "0", "Cortex" ),
	PRM_Name( "1", "Houdini" ),
	PRM_Name( 0 ) // sentinal
};

PRM_ChoiceList SOP_CortexConverter::resultTypeList( PRM_CHOICELIST_SINGLE, resultTypes );

PRM_Template SOP_CortexConverter::parameters[] = {
	PRM_Template(
		PRM_STRING, 1, &pNameFilter, &filterDefault, 0, 0, 0, 0, 0,
		"A list of named shapes to convert. Uses Houdini matching syntax."
	),
	PRM_Template(
		PRM_STRING, 1, &pAttributeFilter, &filterDefault, 0, 0, 0, 0, 0,
		"A list of attribute names to load, if they exist on each shape. Uses Houdini matching syntax. "
		"P will always be loaded."
	),
	PRM_Template(
		PRM_INT, 1, &pResultType, &resultTypeDefault, &resultTypeList, 0, 0, 0, 0,
		"The type of geometry to output. Shapes matching the name filter will be converted to this type. "
		"Shapes that do not match will be passed through."
	),
	PRM_Template(
		PRM_TOGGLE, 1, &pConvertStandardAttributes, &convertStandardAttributesDefault, 0, 0, 0, 0, 0,
		"Performs automated conversion of standard PrimitiveVariables to Houdini Attributes and vice versa (i.e. Pref->rest ; Cs->Cd ; s,t->uv)"
	),
	PRM_Template()
};

CH_LocalVariable SOP_CortexConverter::variables[] = {
	{ 0, 0, 0 },
};

OP_Node *SOP_CortexConverter::create( OP_Network *net, const char *name, OP_Operator *op )
{
	return new SOP_CortexConverter( net, name, op );
}

SOP_CortexConverter::SOP_CortexConverter( OP_Network *net, const char *name, OP_Operator *op )
	: SOP_Node(net, name, op)
{
}

SOP_CortexConverter::~SOP_CortexConverter()
{
}

OP_ERROR SOP_CortexConverter::cookMySop( OP_Context &context )
{
	if( lockInputs( context ) >= UT_ERROR_ABORT )
	{
		return error();
	}
	
	UT_Interrupt *boss = UTgetInterrupt();
	boss->opStart("Building CortexConverter Geometry...");
	gdp->clearAndDestroy();
	
	UT_String nameFilterStr;
	evalString( nameFilterStr, pNameFilter.getToken(), 0, 0 );
	UT_StringMMPattern nameFilter;
	nameFilter.compile( nameFilterStr );
	
	UT_String p( "P" );
	UT_String attributeFilter;
	evalString( attributeFilter, pAttributeFilter.getToken(), 0, 0 );
	if ( !p.match( attributeFilter ) )
	{
		attributeFilter += " P";
	}
	const std::string attributeFilterStr = attributeFilter.toStdString();
	
	ResultType type = (ResultType)this->evalInt( pResultType.getToken(), 0, 0 );
	bool convertStandardAttributes = evalInt( pConvertStandardAttributes.getToken(), 0, 0 );
	
	DetailSplitterPtr splitter = new DetailSplitter( inputGeoHandle( 0 ) );
	std::vector<std::string> names;
	splitter->values( names );
	for ( std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); ++it )
	{
		const std::string &name = *it;
		
		// we want match all to also match no-name
		if ( UT_String( name ).multiMatch( nameFilter ) || ( name == "" && UT_String( "*" ).multiMatch( nameFilter ) ) )
		{
			doConvert( splitter->split( name ), name, type, attributeFilterStr, convertStandardAttributes );
		}
		else
		{
			doPassThrough( splitter->split( name ), name );
		}
	}
	
	boss->opEnd();
	unlockInputs();
	return error();
}

void SOP_CortexConverter::doConvert( const GU_DetailHandle &handle, const std::string &name, ResultType type, const std::string &attributeFilter, bool convertStandardAttributes )
{
	if ( handle.isNull() )
	{
		addError( SOP_MESSAGE, ( "Could not extract the geometry named " + name ).c_str() );
		return;
	}
	
	FromHoudiniGeometryConverterPtr fromConverter = FromHoudiniGeometryConverter::create( handle );
	if ( !fromConverter )
	{
		addError( SOP_MESSAGE, ( "Could not convert the geometry named " + name ).c_str() );
		return;
	}
	
	IECore::ObjectPtr result = fromConverter->convert();
	if ( !result )
	{
		addError( SOP_MESSAGE, ( "Could not find Cortex Object named " + name + " on input geometry" ).c_str() );
		return;
	}
	
	if ( IECore::ParameterisedProcedural *procedural = IECore::runTimeCast<IECore::ParameterisedProcedural>( result ) )
	{
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
		
		result = IECore::constPointerCast<IECore::Object>( IECore::runTimeCast<const IECore::Object>( renderer->world() ) );
	}
	
	ToHoudiniGeometryConverterPtr converter = ( type == Cortex ) ? new ToHoudiniCortexObjectConverter( result ) : ToHoudiniGeometryConverter::create( result );
	converter->nameParameter()->setTypedValue( name );
	converter->attributeFilterParameter()->setTypedValue( attributeFilter );
	converter->convertStandardAttributesParameter()->setTypedValue( convertStandardAttributes );
	
	if ( !converter->convert( myGdpHandle ) )
	{
		addError( SOP_MESSAGE, ( "Could not convert the Cortex Object named " + name + " to Houdini geometry" ).c_str() );
	}
}

void SOP_CortexConverter::doPassThrough( const GU_DetailHandle &handle, const std::string &name )
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

void SOP_CortexConverter::getNodeSpecificInfoText( OP_Context &context, OP_NodeInfoParms &parms )
{
	SOP_Node::getNodeSpecificInfoText( context, parms );
	
	GU_CortexPrimitive::infoText( getCookedGeo( context ), context, parms );
	
	if ( !evalInt( pConvertStandardAttributes.getToken(), 0, 0 ) )
	{
		return;
	}
	
	UT_String p( "P" );
	UT_String filter;
	evalString( filter, pAttributeFilter.getToken(), 0, 0 );
	if ( !p.match( filter ) )
	{
		filter += " P";
	}
	UT_StringMMPattern attributeFilter;
	attributeFilter.compile( filter );
	
	/// \todo: this text could come from a static method on a class that manages these name relations (once that exists)
	parms.append( "Converting standard Cortex PrimitiveVariables:\n" );
	if ( UT_String( "s" ).multiMatch( attributeFilter ) && UT_String( "t" ).multiMatch( attributeFilter ) )
	{
		parms.append( "  s,t <--> uv\n" );
	}
	
	if ( UT_String( "Cs" ).multiMatch( attributeFilter ) )
	{
		parms.append( "  Cs <--> Cd\n" );
	}
	
	if ( UT_String( "Pref" ).multiMatch( attributeFilter ) )
	{
		parms.append( "  Pref <--> rest\n" );
	}
	
	if ( UT_String( "width" ).multiMatch( attributeFilter ) )
	{
		parms.append( "  width <--> pscale\n" );
	}
	
	if ( UT_String( "Os" ).multiMatch( attributeFilter ) )
	{
		parms.append( "  Os <--> Alpha\n" );
	}
}
