//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2015, Image Engine Design Inc. All rights reserved.
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

/// GR_Primitives are used in Houdini 12.5, but do not exist in earlier versions.
/// Check GR_Cortex.cpp for Cortex viewport rendering in Houdini 12.0.
#include "UT/UT_Version.h"
#if UT_MAJOR_VERSION_INT > 12 || UT_MINOR_VERSION_INT >= 5

#include "IECore/MeshPrimitive.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreGL/IECoreGL.h"
#include "IECoreGL/Camera.h"
#include "IECoreGL/CurvesPrimitive.h"
#include "IECoreGL/PointsPrimitive.h"
#include "IECoreGL/Primitive.h"
#include "IECoreGL/Renderer.h"
#include "IECoreGL/Scene.h"
#include "IECoreGL/ShaderLoader.h"
#include "IECoreGL/ShaderStateComponent.h"
#include "IECoreGL/State.h"
#include "IECoreGL/TextureLoader.h"

// this needs to come after IECoreGL so gl and glew don't fight
#include "RE/RE_Render.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/GR_CortexPrimitive.h"
#include "IECoreHoudini/GEO_CortexPrimitive.h"

using namespace IECoreHoudini;

#if UT_MAJOR_VERSION_INT >= 14

typedef GEO_CortexPrimitive CortexPrimitive;

#else

#include "IECoreHoudini/GU_CortexPrimitive.h"

typedef GU_CortexPrimitive CortexPrimitive;

#endif

GR_CortexPrimitive::GR_CortexPrimitive( const GR_RenderInfo *info, const char *cache_name, const GEO_Primitive *prim )
	: GR_Primitive( info, cache_name, GA_PrimCompat::TypeMask(0) )
{
	IECoreGL::init( true );
	
	if ( prim->getTypeDef().getId() == CortexPrimitive::typeId().get() )
	{

#if UT_MAJOR_VERSION_INT < 14

		m_primId = prim->getNum();

#else

		m_primId = prim->getMapIndex();
#endif

	}

}

GR_CortexPrimitive::~GR_CortexPrimitive()
{
	m_renderable = 0;
}

GR_PrimAcceptResult GR_CortexPrimitive::acceptPrimitive( GT_PrimitiveType t, int geo_type, const GT_PrimitiveHandle &ph, const GEO_Primitive *prim )
{

	if ( geo_type == CortexPrimitive::typeId().get() )
	{

#if UT_MAJOR_VERSION_INT < 14

		m_primId = prim->getNum();

#else

		m_primId = prim->getMapIndex();

#endif

		return GR_PROCESSED;
	}
	
	return GR_NOT_PROCESSED;
}

void GR_CortexPrimitive::resetPrimitives()
{
	m_primId = -1;
	m_renderable = 0;
}

void GR_CortexPrimitive::update( RE_Render *r, const GT_PrimitiveHandle &primh, const GR_UpdateParms &p )
{
	GA_Offset offset = p.geometry.primitiveOffset( m_primId );

	const CortexPrimitive *prim = dynamic_cast<const CortexPrimitive *>( p.geometry.getGEOPrimitive( offset ) );

	if ( !prim )
	{
		m_scene = 0;
		m_renderable = 0;
		return;
	}
	
	m_renderable = IECore::runTimeCast<const IECore::Renderable>( prim->getObject() );
	if ( !m_renderable )
	{
		m_scene = 0;
		return;
	}
	
	IECoreGL::RendererPtr renderer = new IECoreGL::Renderer();
	renderer->setOption( "gl:mode", new IECore::StringData( "deferred" ) );
	renderer->setOption( "gl:drawCoordinateSystems", new IECore::BoolData( true ) );
	renderer->worldBegin();
	
	if ( p.dopts.boundBox() )
	{
		const IECore::VisibleRenderable *visible = IECore::runTimeCast<const IECore::VisibleRenderable>( m_renderable );
		if ( visible )
		{
			IECore::MeshPrimitive::createBox( visible->bound() )->render( renderer.get() );
		}
	}
	else
	{
		m_renderable->render( renderer.get() );
	}
	
	renderer->worldEnd();
	
	m_scene = renderer->scene();
	m_scene->setCamera( 0 ); // houdini will be providing the camera
}

void GR_CortexPrimitive::render( RE_Render *r, GR_RenderMode render_mode, GR_RenderFlags flags, const GR_DisplayOption *opt, const RE_MaterialList *materials )
{
	if ( !m_scene )
	{
		return;
	}
	
	UT_Matrix4D transform;
	memcpy( transform.data(), r->getUniform( RE_UNIFORM_OBJECT_MATRIX )->getValue(), sizeof(double) * 16 );
	
	GLint currentProgram = 0;
	glGetIntegerv( GL_CURRENT_PROGRAM, &currentProgram );
	
	IECoreGL::State *state = getState( render_mode, flags, opt );
	
	if ( render_mode == GR_RENDER_OBJECT_PICK )
	{
		const IECoreGL::Shader *shader = state->get<IECoreGL::ShaderStateComponent>()->shaderSetup()->shader();
		glUseProgram( shader->program() );
		glUniform1i( shader->uniformParameter( "objectPickId" )->location, r->getObjectPickID() );
	}
	
	r->pushMatrix();
		
		r->multiplyMatrix( transform );
		m_scene->render( state );
	
	r->popMatrix();
	
	if ( render_mode == GR_RENDER_OBJECT_PICK )
	{
		glUseProgram( currentProgram );
	}
}

void GR_CortexPrimitive::renderInstances( RE_Render *r, GR_RenderMode render_mode, GR_RenderFlags flags, const GR_DisplayOption *opt, const RE_MaterialList *materials, int render_instance )
{
	/// \todo: implement this to support instanced rendering.
	/// renderInstances() is for doing instanced drawing of
	/// your primitive, which will be called if it's instanced
	/// at the object level, or contained in a packed primitive
	/// which is copied multiple times. In those cases, update()
	/// will be passed a non-NULL UT_Matrix4DArray and an instance
	/// group to which they belong (in GR_UpdateParms). The
	/// instance_group passed to renderInstances() is used to
	/// indicate the group being rendered.
}

int GR_CortexPrimitive::renderPick( RE_Render *r, const GR_DisplayOption *opt, unsigned int pick_type, GR_PickStyle pick_style, bool has_pick_map )
{
	// return 0 to indicate we don't support component picking
	return 0;
}

IECoreGL::StatePtr GR_CortexPrimitive::g_lit = 0;
IECoreGL::StatePtr GR_CortexPrimitive::g_shaded = 0;
IECoreGL::StatePtr GR_CortexPrimitive::g_wire = 0;
IECoreGL::StatePtr GR_CortexPrimitive::g_wireLit = 0;
IECoreGL::StatePtr GR_CortexPrimitive::g_wireShaded = 0;
IECoreGL::StatePtr GR_CortexPrimitive::g_wireConstGhost = 0;
IECoreGL::StatePtr GR_CortexPrimitive::g_wireConstBG = 0;
IECoreGL::StatePtr GR_CortexPrimitive::g_pick = 0;
IECoreGL::StatePtr GR_CortexPrimitive::g_selected = 0;
IECoreGL::StatePtr GR_CortexPrimitive::g_wireSelected = 0;
IECoreGL::StatePtr GR_CortexPrimitive::g_wireConstBGSelected = 0;
IECoreGL::StatePtr GR_CortexPrimitive::g_wireConstGhostSelected = 0;

IECoreGL::State *GR_CortexPrimitive::getState( GR_RenderMode mode, GR_RenderFlags flags, const GR_DisplayOption *opt )
{
	if ( !g_lit || !g_shaded || !g_wire || !g_wireLit || !g_wireShaded || !g_wireConstGhost || !g_wireConstBG || !g_pick || !g_selected || !g_wireSelected || !g_wireConstBGSelected || !g_wireConstGhostSelected )
	{
		g_shaded = new IECoreGL::State( true );
		g_shaded->add( new IECoreGL::PointsPrimitive::UseGLPoints( IECoreGL::ForAll ) );
		g_shaded->add( new IECoreGL::PointsPrimitive::GLPointWidth( 3. ) );
		g_shaded->add( new IECoreGL::CurvesPrimitive::UseGLLines( true ) );
		
		/// \todo: this doesn't seem to get the lights. maybe they aren't in the gl light list?
		g_lit = new IECoreGL::State( *g_shaded );
		g_lit->add(
			new IECoreGL::ShaderStateComponent(
				IECoreGL::ShaderLoader::defaultShaderLoader(),
				IECoreGL::TextureLoader::defaultTextureLoader(),
				IECoreGL::Shader::defaultVertexSource(),
				IECoreGL::Shader::defaultGeometrySource(),
				IECoreGL::Shader::lambertFragmentSource(),
				new IECore::CompoundObject()
			),
			/// \todo: by setting true here, we are forcing an override of all other
			/// ShaderStateComponents in the hierarhcy. Is this desirable in all cases?
			true
		);
		
		g_wireShaded = new IECoreGL::State( *g_shaded );
		g_wireShaded->add( new IECoreGL::Primitive::DrawWireframe( true ) );
		g_wireShaded->add( new IECoreGL::WireframeColorStateComponent( IECore::convert<Imath::Color4f>( opt->common().getColor( GR_WIREFRAME_COLOR ) ) ) );
		
		g_wire = new IECoreGL::State( *g_shaded );
		g_wire->add( new IECoreGL::Primitive::DrawSolid( false ) );
		g_wire->add( new IECoreGL::Primitive::DrawWireframe( true ) );
		g_wire->add( new IECoreGL::WireframeColorStateComponent( Imath::Color4f( 1 ) ) );
		
		g_wireLit = new IECoreGL::State( *g_lit );
		g_wireLit->add( new IECoreGL::Primitive::DrawWireframe( true ) );
		g_wireLit->add( new IECoreGL::WireframeColorStateComponent( Imath::Color4f( 0.5, 0.5, 0.5, 1 ) ) );
		
		g_wireConstBG = new IECoreGL::State( *g_wireShaded );
		g_wireConstBG->add( new IECoreGL::Color( IECore::convert<Imath::Color4f>( opt->common().getColor( GR_BACKGROUND_COLOR ) ) ) );
		g_wireConstBG->add(
			new IECoreGL::ShaderStateComponent(
				IECoreGL::ShaderLoader::defaultShaderLoader(),
				IECoreGL::TextureLoader::defaultTextureLoader(),
				IECoreGL::Shader::defaultVertexSource(),
				IECoreGL::Shader::defaultGeometrySource(),
				IECoreGL::Shader::constantFragmentSource(),
				new IECore::CompoundObject()
			),
			true
		);
		
		g_wireConstGhost = new IECoreGL::State( *g_wireConstBG );
		g_wireConstGhost->add( new IECoreGL::Color( IECore::convert<Imath::Color4f>( opt->common().getColor( GR_GHOST_FILL_COLOR ) ) ) );
		
		g_pick = new IECoreGL::State( *g_shaded );
		g_pick->add(
			new IECoreGL::ShaderStateComponent(
				IECoreGL::ShaderLoader::defaultShaderLoader(),
				IECoreGL::TextureLoader::defaultTextureLoader(),
				IECoreGL::Shader::defaultVertexSource(),
				IECoreGL::Shader::defaultGeometrySource(),
				pickFragmentSource(),
				new IECore::CompoundObject()
			),
			true
		);
		
		g_selected = new IECoreGL::State( *g_shaded );
		g_selected->add( new IECoreGL::Primitive::DrawWireframe( true ) );
		IECoreGL::WireframeColorStateComponentPtr selectionColor = new IECoreGL::WireframeColorStateComponent( IECore::convert<Imath::Color4f>( opt->common().getColor( GR_OBJECT_SELECT_COLOR ) ) );
		g_selected->add( selectionColor.get() );
		
		g_wireSelected = new IECoreGL::State( *g_wire );
		g_wireSelected->add( selectionColor.get() );
		
		g_wireConstBGSelected = new IECoreGL::State( *g_wireConstBG );
		g_wireConstBGSelected->add( selectionColor.get() );
		
		g_wireConstGhostSelected = new IECoreGL::State( *g_wireConstGhost );
		g_wireConstGhostSelected->add( selectionColor.get() );
	}
	
	switch ( mode )
	{
		case GR_RENDER_BEAUTY :
		case GR_RENDER_MATERIAL :
		case GR_RENDER_MATERIAL_WIREFRAME :
		{
			if ( isObjectSelection() )
			{
				return g_selected.get();
			}

			if ( flags & GR_RENDER_FLAG_WIRE_OVER )
			{
				if ( flags & GR_RENDER_FLAG_UNLIT )
				{
					return g_wireShaded.get();
				}
				
				return g_wireLit.get();
			}
			
			if ( flags & GR_RENDER_FLAG_UNLIT )
			{
				return g_shaded.get();
			}
			
			return g_lit.get();
		}
		case GR_RENDER_WIREFRAME :
		{
			if ( isObjectSelection() )
			{
				return g_wireSelected.get();
			}

			return g_wire.get();
		}
		case GR_RENDER_HIDDEN_LINE :
		{
			if ( isObjectSelection() )
			{
				return g_wireConstBGSelected.get();
			}

			return g_wireConstBG.get();
		}
		case GR_RENDER_GHOST_LINE :
		{
			if ( isObjectSelection() )
			{
				return g_wireConstGhostSelected.get();
			}

			return g_wireConstGhost.get();
		}
		// hovering on CortexPrimitives during GR_RENDER_OBJECT_PICK mode flips the mode
		// to GR_RENDER_MATTE. Since we're not supporting that on its own, we'll consider it
		// a continued pick for now. This avoids strange popping draws on hover.
		case GR_RENDER_MATTE :
		case GR_RENDER_OBJECT_PICK :
		{
			return g_pick.get();
		}
		default :
		{
			break;
		}
	}
	
	return g_shaded.get();
}

const std::string &GR_CortexPrimitive::pickFragmentSource()
{
	static std::string s = 
	
		"#version 150 compatibility\n"
		"#extension GL_EXT_gpu_shader4 : enable\n"
		""
		"uniform int objectPickId;"
		"out ivec4 id;"
		""
		"void main()"
		"{"
		"	id = ivec4( objectPickId, 0, 0, 0 );"
		"}";
	
	return s;
}

#endif // 12.5 or later
