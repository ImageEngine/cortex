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

// OpenGL
#include <glew.h>

// Houdini
#include <UT/UT_Version.h>
#include <RE/RE_Render.h>
#include <UT/UT_Interrupt.h>

// Cortex
#include <IECoreGL/Scene.h>
#include <IECoreGL/State.h>
#include <IECoreGL/StateComponent.h>
#include <IECoreGL/TypedStateComponent.h>
#include <IECoreGL/NameStateComponent.h>
#include <IECoreGL/BoxPrimitive.h>
#include <IECoreGL/Exception.h>
#include <IECoreGL/Group.h>
#include <IECoreGL/Camera.h>
#include <IECore/SimpleTypedData.h>

// IECoreHoudini
#include "GR_Procedural.h"
#include "SOP_ProceduralHolder.h"
using namespace IECoreHoudini;

// ctor
GR_Procedural::GR_Procedural()
{
	IECoreGL::init( true );
}
// dtor
GR_Procedural::~GR_Procedural()
{
}

// Tell Houdini to only render GU_ProceduralDetails with this
// render hook.
int GR_Procedural::getWireMask( GU_Detail *gdp,
		const GR_DisplayOption *dopt
		) const
{
	if ( gdp->attribs().find("IECoreHoudini::SOP_ProceduralHolder", GB_ATTRIB_MIXED) )
		return 0;
	else
    	return GEOPRIMALL;
}

// Tell Houdini to only render GU_ProceduralDetails with this
// render hook.
int GR_Procedural::getShadedMask( GU_Detail *gdp,
		const GR_DisplayOption *dopt
		) const
{
	if ( gdp->attribs().find("IECoreHoudini::SOP_ProceduralHolder", GB_ATTRIB_MIXED) )
        return 0;
    else
    	return GEOPRIMALL;
}

// Get a Cortex display state based on the Houdini display options
IECoreGL::ConstStatePtr GR_Procedural::getDisplayState(
		const GR_DisplayOption *dopt,
		bool wireframe
		)
{
	// default is good for shaded
	IECoreGL::StatePtr state = new IECoreGL::State( true );

	// add some properties for wireframe rendering
	if ( wireframe )
	{
		state->add( new IECoreGL::Primitive::DrawSolid( false ) );
		state->add( new IECoreGL::Primitive::DrawWireframe( true ) );
		UT_Color wire_col;
		wire_col = dopt->wireColor();
		float r,g,b;
		wire_col.getValue( r, g, b );
		state->add( new IECoreGL::WireframeColorStateComponent( Imath::Color4f( r, g, b, 1 ) ) );
	}
	return state;
}

// Render our ParameterisedProcedural in wireframe
void GR_Procedural::renderWire( GU_Detail *gdp,
    RE_Render &ren,
    const GR_AttribOffset &ptinfo,
    const GR_DisplayOption *dopt,
    float lod,
    const GU_PrimGroupClosure *hidden_geometry
    )
{
	if ( !gdp->attribs().find("IECoreHoudini::SOP_ProceduralHolder", GB_ATTRIB_MIXED) )
		return;

#if UT_MAJOR_VERSION_INT >= 11
	GB_AttributeRef attr_offset = gdp->attribs().getOffset( "IECoreHoudini::SOP_ProceduralHolder", GB_ATTRIB_MIXED );
	SOP_ProceduralPassStruct *sop = gdp->attribs().castAttribData<SOP_ProceduralPassStruct>(attr_offset);
#else
	int attr_offset = gdp->attribs().getOffset( "IECoreHoudini::SOP_ProceduralHolder", GB_ATTRIB_MIXED );
	SOP_ProceduralPassStruct *sop = gdp->attribs().castAttribData<SOP_ProceduralPassStruct>(attr_offset);
#endif
	if ( !sop )
		return;

    // our render state
    IECoreGL::ConstStatePtr displayState = getDisplayState( dopt, true );

	// our render scene
	IECoreGL::ConstScenePtr scene = sop->ptr()->scene();
    if ( !scene )
    	return;

    // render our scene
	GLint prevProgram;
	glGetIntegerv( GL_CURRENT_PROGRAM, &prevProgram );
	scene->root()->render( displayState );
	glUseProgram( prevProgram );
}

// Render our ParameterisedProcedural in shaded
void GR_Procedural::renderShaded( GU_Detail *gdp,
		RE_Render &ren,
		const GR_AttribOffset &ptinfo,
		const GR_DisplayOption *dopt,
		float lod,
		const GU_PrimGroupClosure *hidden_geometry
		)
{
	if ( !gdp->attribs().find("IECoreHoudini::SOP_ProceduralHolder", GB_ATTRIB_MIXED) )
		return;

#if UT_MAJOR_VERSION_INT >= 11
	GB_AttributeRef attr_offset = gdp->attribs().getOffset( "IECoreHoudini::SOP_ProceduralHolder", GB_ATTRIB_MIXED );
	SOP_ProceduralPassStruct *sop = gdp->attribs().castAttribData<SOP_ProceduralPassStruct>(attr_offset);
#else
	int attr_offset = gdp->attribs().getOffset( "IECoreHoudini::SOP_ProceduralHolder", GB_ATTRIB_MIXED );
	SOP_ProceduralPassStruct *sop = gdp->attribs().castAttribData<SOP_ProceduralPassStruct>(attr_offset);
#endif
	if ( !sop )
		return;

    // our render state
    IECoreGL::ConstStatePtr displayState = getDisplayState( dopt, false );

	// our render scene
	IECoreGL::ConstScenePtr scene = sop->ptr()->scene();
    if ( !scene )
    	return;

    // render our scene
	GLint prevProgram;
	glGetIntegerv( GL_CURRENT_PROGRAM, &prevProgram );
	scene->root()->render( displayState );
	glUseProgram( prevProgram );
}
