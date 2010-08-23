//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
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
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Version.h>
#include <OP/OP_OperatorTable.h>
#include <GR/GR_RenderTable.h>

// Boost
#include <boost/python.hpp>

// IECoreHoudini
#include "SOP_ProceduralHolder.h"
#include "GR_Procedural.h"
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
	table->addOperator(
			new OP_Operator("ieProceduralHolder", // Internal name
					"Cortex Procedural", // UI name
					SOP_ProceduralHolder::myConstructor, // How to build the SOP
    				SOP_ProceduralHolder::myParameters, // My parameters
    				0, // Min # of sources
    				0, // Max # of sources
    				SOP_ProceduralHolder::myVariables, // Local variables
    				OP_FLAG_GENERATOR) ); // Flag it as generator
}

/// Declare our new Render Hooks
void newRenderHook( GR_RenderTable *table )
{
    GR_Procedural *hook = new GR_Procedural;
#if UT_MAJOR_VERSION_INT >= 11
    table->addHook(hook,GR_RENDER_HOOK_VERSION);
#else
    table->addHook(hook);
#endif
}
