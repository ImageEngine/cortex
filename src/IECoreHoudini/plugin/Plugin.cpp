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

// Boost
#include <boost/python.hpp>

// Houdini
#include <UT/UT_DSOVersion.h>
#include <UT/UT_IOTable.h>
#include <UT/UT_Version.h>
#include <OP/OP_OperatorTable.h>
#include <GR/GR_RenderTable.h>

// IECoreHoudini
#include "IECoreHoudini/SOP_OpHolder.h"
#include "IECoreHoudini/SOP_ParameterisedHolder.h"
#include "IECoreHoudini/SOP_ProceduralHolder.h"
#include "IECoreHoudini/SOP_ToHoudiniConverter.h"
#include "IECoreHoudini/SOP_InterpolatedCacheReader.h"
#include "IECoreHoudini/GEO_CobIOTranslator.h"
#include "IECoreHoudini/GR_Cortex.h"

using namespace IECoreHoudini;

/// Tell Houdini that this plugin should be loaded with RTLD_GLOBAL
extern "C"
{
	DLLEXPORT void HoudiniDSOInit( UT_DSOInfo &dsoinfo )
	{
		dsoinfo.loadGlobal = true;
	}
}

/// Declare our new SOPs
void newSopOperator(OP_OperatorTable *table)
{
	OP_Operator *opHolder = new OP_Operator(
		"ieOpHolder", "Cortex Op",
		SOP_OpHolder::create, SOP_ParameterisedHolder::parameters, 0, 4,
		SOP_ParameterisedHolder::variables, OP_FLAG_GENERATOR
	);
	opHolder->setIconName( "SOP_ieOpHolder" );
	
	OP_Operator *proceduralHolder = new OP_Operator(
		"ieProceduralHolder", "Cortex Procedural",
		SOP_ProceduralHolder::create, SOP_ParameterisedHolder::parameters, 0, 4,
    		SOP_ParameterisedHolder::variables, OP_FLAG_GENERATOR
	);
	proceduralHolder->setIconName( "SOP_ieProceduralHolder" );
	
	OP_Operator *converter = new OP_Operator(
		"ieToHoudiniConverter", "Cortex To Houdini",
		SOP_ToHoudiniConverter::create, SOP_ToHoudiniConverter::parameters, 1,	1,
		SOP_ToHoudiniConverter::variables, OP_FLAG_GENERATOR
	);
	converter->setIconName( "SOP_ieToHoudiniConverter" );
	
	OP_Operator *cacheReader = new OP_Operator(
		"ieInterpolatedCacheReader", "Interpolated Cache Reader",
		SOP_InterpolatedCacheReader::create, SOP_InterpolatedCacheReader::parameters, 1, 1, 0
	);
	cacheReader->setIconName( "SOP_ieInterpolatedCacheReader" );
	
	table->addOperator( proceduralHolder );
	table->addOperator( opHolder );
	table->addOperator( converter );
	table->addOperator( cacheReader );
	
	table->addOpHidden( opHolder->getName() );
	table->addOpHidden( proceduralHolder->getName() );
	table->addOpHidden( converter->getName() );
	table->addOpHidden( cacheReader->getName() );
}

/// Declare our new Render Hooks
void newRenderHook( GR_RenderTable *table )
{
	GR_Cortex *hook = new GR_Cortex;
#if UT_MAJOR_VERSION_INT >= 11
	table->addHook( hook, GR_RENDER_HOOK_VERSION );
#else
	table->addHook( hook );
#endif
}

/// Declare our new IO Translators
void newGeometryIO( void * )
{
	GU_Detail::registerIOTranslator( new GEO_CobIOTranslator() );
	
	UT_ExtensionList *geoextension = UTgetGeoExtensions();
	if ( !geoextension->findExtension( "cob" ) )
	{
		geoextension->addExtension( "cob" );
	}
	if ( !geoextension->findExtension( "pdc" ) )
	{
		geoextension->addExtension( "pdc" );
	}
}
