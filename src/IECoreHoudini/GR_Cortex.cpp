//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

/// GR_RenderHooks are used in Houdini 12.0 and cause problems in 12.5.
/// Check GR_CortexPrimitive.cpp for Cortex viewport rendering in Houdini 12.5.
#include "UT/UT_Version.h"
#if UT_MAJOR_VERSION_INT == 12 && UT_MINOR_VERSION_INT <= 1

#include <glew.h>

#include "GA/GA_AIFBlindData.h"
#include "RE/RE_Render.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_Version.h"

#include "IECore/Op.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VisibleRenderable.h"

#include "IECoreGL/BoxPrimitive.h"
#include "IECoreGL/Camera.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/Group.h"
#include "IECoreGL/NameStateComponent.h"
#include "IECoreGL/Scene.h"
#include "IECoreGL/State.h"
#include "IECoreGL/StateComponent.h"
#include "IECoreGL/TypedStateComponent.h"

#include "IECoreHoudini/GR_Cortex.h"
#include "IECoreHoudini/NodePassData.h"
#include "IECoreHoudini/SOP_OpHolder.h"
#include "IECoreHoudini/SOP_ProceduralHolder.h"

using namespace IECoreHoudini;

GR_Cortex::GR_Cortex()
{
	IECoreGL::init( true );
}

GR_Cortex::~GR_Cortex()
{
}

// Tell Houdini to only render GU_ProceduralDetails with this render hook.
GA_PrimCompat::TypeMask GR_Cortex::getWireMask( GU_Detail *gdp, const GR_DisplayOption *dopt ) const
{
	const GA_ROAttributeRef attrRef = gdp->findAttribute( GA_ATTRIB_DETAIL, GA_SCOPE_PRIVATE, "IECoreHoudiniNodePassData" );
	if ( attrRef.isValid() )
	{
		return GA_PrimCompat::TypeMask( 0 );
	}
	else
	{
		return GEO_PrimTypeCompat::GEOPRIMALL;
	}
}

// Tell Houdini to only render GU_ProceduralDetails with this render hook.
GA_PrimCompat::TypeMask GR_Cortex::getShadedMask( GU_Detail *gdp, const GR_DisplayOption *dopt ) const
{
	const GA_ROAttributeRef attrRef = gdp->findAttribute( GA_ATTRIB_DETAIL, GA_SCOPE_PRIVATE, "IECoreHoudiniNodePassData" );
	if ( attrRef.isValid() )
	{
		return GA_PrimCompat::TypeMask( 0 );
	}
	else
	{
		return GEO_PrimTypeCompat::GEOPRIMALL;
	}
}

// Render our ParameterisedProcedural in wireframe
void GR_Cortex::renderWire( GU_Detail *gdp, RE_Render &ren, const GR_AttribOffset &ptinfo, const GR_DisplayOption *dopt, float lod, const GU_PrimGroupClosure *hidden_geometry )
{
	// our render state
	IECoreGL::ConstStatePtr displayState = getDisplayState( dopt, true );
	render( gdp, displayState );
}

// Render our ParameterisedProcedural in shaded
void GR_Cortex::renderShaded( GU_Detail *gdp, RE_Render &ren, const GR_AttribOffset &ptinfo, const GR_DisplayOption *dopt, float lod, const GU_PrimGroupClosure *hidden_geometry )
{
	// our render state
	IECoreGL::ConstStatePtr displayState = getDisplayState( dopt, false );
	render( gdp, displayState );
}

// Get a Cortex display state based on the Houdini display options
IECoreGL::ConstStatePtr GR_Cortex::getDisplayState( const GR_DisplayOption *dopt, bool wireframe )
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

// Renders an object directly (normally from an opHolder)
void GR_Cortex::renderObject( const IECore::Object *object, const IECoreGL::State *displayState )
{
	// try and cast this to a visible renderable
	IECore::ConstVisibleRenderablePtr renderable = IECore::runTimeCast<const IECore::VisibleRenderable>( object );
	if ( !renderable )
	{
		return;
	}

	// render our object into a buffer
	IECoreGL::RendererPtr renderer = new IECoreGL::Renderer();
	renderer->setOption( "gl:mode", new IECore::StringData( "deferred" ) );
	renderer->worldBegin();
	renderable->render( renderer );
	renderer->worldEnd();
	IECoreGL::ScenePtr scene = renderer->scene();
	scene->setCamera( 0 ); // houdini will be providing the camera

	// now render
	scene->render( const_cast<IECoreGL::State *>( displayState ) );
}

// general cortex render function, takes a gu_detail and uses the NodePassData attribute
// to call the required render method
void GR_Cortex::render( GU_Detail *gdp, const IECoreGL::State *displayState )
{
	// gl scene from a parameterised procedural
	const GA_ROAttributeRef attrRef = gdp->findAttribute( GA_ATTRIB_DETAIL, GA_SCOPE_PRIVATE, "IECoreHoudiniNodePassData" );
	if ( attrRef.isInvalid() )
	{
		return;
	}

	const GA_Attribute *attr = attrRef.getAttribute();
	const GA_AIFBlindData *blindData = attr->getAIFBlindData();
	const NodePassData passData = blindData->getValue<NodePassData>( attr, 0 );

	switch( passData.type() )
	{
		case IECoreHoudini::NodePassData::CORTEX_OPHOLDER :
		{
			SOP_OpHolder *sop = dynamic_cast<SOP_OpHolder*>( const_cast<OP_Node*>( passData.nodePtr() ) );
			if ( !sop )
			{
				return;
			}

			IECore::OpPtr op = IECore::runTimeCast<IECore::Op>( sop->getParameterised() );
			if ( !op )
			{
				return;
			}

			const IECore::Parameter *result_parameter = op->resultParameter();
			const IECore::Object *result_object = result_parameter->getValue();
			renderObject( result_object, displayState );
			break;
		}
		case IECoreHoudini::NodePassData::CORTEX_PROCEDURALHOLDER :
		{
			SOP_ProceduralHolder *sop = dynamic_cast<SOP_ProceduralHolder*>( const_cast<OP_Node*>( passData.nodePtr() ) );
			if ( !sop )
			{
				return;
			}

			IECoreGL::ConstScenePtr scene = sop->scene();
			if ( !scene )
			{
				return;
			}

			scene->render( const_cast<IECoreGL::State *>( displayState ) );
			break;
		}
		default :
		{
			break;
		}
	}
}

#endif // 12.1 or earlier
