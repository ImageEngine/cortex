//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2012, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

#include "DDImage/ViewerContext.h"
#include "DDImage/Knob.h"
#include "DDImage/Knobs.h"

#include "IECore/WorldBlock.h"
#include "IECore/ParameterisedProcedural.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/Convert.h"

#include "IECoreGL/Renderer.h"
#include "IECoreGL/Scene.h"
#include "IECoreGL/Camera.h"
#include "IECoreGL/BoxPrimitive.h"

#include "IECorePython/ScopedGILLock.h"

#include "IECoreNuke/ProceduralHolder.h"
#include "IECoreNuke/Convert.h"

using namespace IECoreNuke;

const DD::Image::Op::Description ProceduralHolder::g_description( "ieProcedural", build );

ProceduralHolder::ProceduralHolder( Node *node )
	:	ParameterisedHolderOp( node ),
		m_drawContents( true ),
		m_drawBound( true ),
		m_drawCoordinateSystems( true ),
		m_scene( 0 ),
		m_transform( DD::Image::Matrix4::identity() ),
		m_transformKnob( 0 )
{
}

ProceduralHolder::~ProceduralHolder()
{
}

void ProceduralHolder::knobs( DD::Image::Knob_Callback f )
{
	ParameterisedHolderOp::knobs( f );

	DD::Image::Tab_knob( f, "Transform" );

	m_transformKnob = DD::Image::Axis_knob( f, &m_transform, "transform", "Transform" );

	DD::Image::Tab_knob( f, "Display" );

	DD::Image::Bool_knob( f, &m_drawContents, "drawContents", "Draw Contents" );
	DD::Image::Tooltip( f,
		"When this is on, the contents of the procedural are drawn. "
		"If you have very heavy procedurals then turning this off can "
		"greatly improve drawing speed."
	);
	DD::Image::Newline( f );

	DD::Image::Bool_knob( f, &m_drawBound, "drawBound", "Draw Bound" );
	DD::Image::Tooltip( f,
		"When this is on, the bounding box of the procedural is drawn. "
	);
	DD::Image::Newline( f );

	DD::Image::Bool_knob( f, &m_drawCoordinateSystems, "drawCoordinateSystems", "Draw Coordinate Systems" );
	DD::Image::Tooltip( f,
		"When this is on, coordinate systems the procedural generates are drawn. "
	);
	DD::Image::Newline( f );

}

#if kDDImageVersionInteger >= 70000

DD::Image::Op::HandlesMode ProceduralHolder::doAnyHandles( DD::Image::ViewerContext *ctx )
{
	HandlesMode result = ParameterisedHolderOp::doAnyHandles( ctx );

	if ( panel_visible() )
	{
		result |= eHandlesCooked;
	}

	if ( ctx->connected() == DD::Image::SHOW_OBJECT )
	{
		result |= eHandlesCooked;
	}

	return result;
}

#elif kDDImageVersionInteger >= 62000

// Op::doAnyHandles() was introduced in Nuke 6.2.
bool ProceduralHolder::doAnyHandles( DD::Image::ViewerContext *ctx )
{
	if( ParameterisedHolderOp::doAnyHandles( ctx ) )
	{
		return true;
	}

	if( panel_visible() )
	{
		return true;
	}

	if( ctx->connected()==DD::Image::SHOW_OBJECT )
	{
		return true;
	}

	return false;
}

#endif

void ProceduralHolder::build_handles( DD::Image::ViewerContext *ctx )
{
	if( ctx->transform_mode() == DD::Image::VIEWER_2D )
	{
		return;
	}

	if( m_transformKnob->build_handle( ctx ) )
	{
		m_transformKnob->add_draw_handle( ctx );
	}

	DD::Image::Matrix4 parentMatrix = ctx->modelmatrix;
	ctx->modelmatrix *= m_transform;

		buildParameterKnobHandles( ctx );

		Imath::Box3f b = bound();
		if( b.hasVolume() )
		{
			ctx->expand_bbox( node_selected(), IECore::convert<DD::Image::Box3>( b ) );
		}

		add_draw_handle( ctx );

	ctx->modelmatrix = parentMatrix;
}

void ProceduralHolder::draw_handle( DD::Image::ViewerContext *ctx )
{
	if( ctx->draw_solid() )
	{

		// nuke apparently uses the name stack to determine which handle is under
		// the mouse. the IECoreGL::NameStateComponent will ruin this by overwriting
		// the current name. we work around this by pushing another name onto the stack.
		// the NameStateComponent will overwrite this name, but nuke will still detect
		// hits on the procedural using the original name one level lower in the stack.
		glPushName( 0 );

			try
			{
				if( m_drawContents )
				{
					IECoreGL::ConstScenePtr s = scene();
					if( s )
					{
						s->render();
					}
				}
			}
			catch( const std::exception &e )
			{
				IECore::msg( IECore::Msg::Error, "ProceduralHolder::draw_handle", e.what() );
			}

		glPopName();

	}
	else if( ctx->draw_lines() )
	{
		if( m_drawBound )
		{
			Imath::Box3f b = bound();
			if( b.hasVolume() )
			{
				IECoreGL::BoxPrimitive::renderWireframe( b );
			}
		}
	}
}

const char *ProceduralHolder::node_shape() const
{
	return "()";
}

IECore::ConstParameterisedProceduralPtr ProceduralHolder::procedural()
{
	return IECore::runTimeCast<const IECore::ParameterisedProcedural>( parameterised() );
}

IECoreGL::ConstScenePtr ProceduralHolder::scene()
{
	if( m_scene && hash()==m_sceneHash )
	{
		return m_scene;
	}

	IECore::ConstParameterisedProceduralPtr proc = procedural();
	if( !proc )
	{
		return 0;
	}

	try
	{

		setParameterValues();

		IECoreGL::RendererPtr renderer = new IECoreGL::Renderer();
		renderer->setOption( "gl:mode", new IECore::StringData( "deferred" ) );
		renderer->setOption( "gl:drawCoordinateSystems", new IECore::BoolData( m_drawCoordinateSystems ) );

		{
			IECore::WorldBlock worldBlock( renderer );
			proc->render( renderer.get(), false, true, true, true );
		}

		m_scene = renderer->scene();
		m_scene->setCamera( 0 );

	}
	/// \todo I think python errors should be handled in the python wrappers - why should C++
	/// code have to catch boost::python exceptions?
	catch( boost::python::error_already_set )
	{
		IECorePython::ScopedGILLock gilLock;
		PyErr_Print();
	}
	catch( const std::exception &e )
	{
		IECore::msg( IECore::Msg::Error, "ProceduralHolder::scene", e.what() );
	}
	catch( ... )
	{
		IECore::msg( IECore::Msg::Error, "ProceduralHolder::scene", "Caught unknown exception" );
	}

	m_sceneHash = hash();
	return m_scene;
}

Imath::Box3f ProceduralHolder::bound()
{
	if( m_boundHash == hash() )
	{
		return m_bound;
	}

	IECore::ConstParameterisedProceduralPtr proc = procedural();
	if( !proc )
	{
		m_bound = Imath::Box3f();
	}
	else
	{
		try
		{
			setParameterValues();
			m_bound = proc->bound();
		}
		/// \todo I think python errors should be handled in the python wrappers - why should C++
		/// code have to catch boost::python exceptions?
		catch( boost::python::error_already_set )
		{
			IECorePython::ScopedGILLock gilLock;
			PyErr_Print();
		}
		catch( const std::exception &e )
		{
			IECore::msg( IECore::Msg::Error, "ProceduralHolder::bound", e.what() );
		}
		catch( ... )
		{
			IECore::msg( IECore::Msg::Error, "ProceduralHolder::bound", "Caught unknown exception" );
		}
	}

	m_boundHash = hash();

	return m_bound;
}

Imath::M44f ProceduralHolder::transform()
{
	return IECore::convert<Imath::M44f>( m_transform );
}

DD::Image::Op *ProceduralHolder::build( Node *node )
{
	return new ProceduralHolder( node );
}

const char *ProceduralHolder::Class() const
{
	return g_description.name;
}

const char *ProceduralHolder::node_help() const
{
	return "Displays Cortex procedurals.";
}
