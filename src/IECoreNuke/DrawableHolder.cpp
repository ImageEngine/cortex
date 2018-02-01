//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "IECoreNuke/DrawableHolder.h"

#include "IECoreNuke/Convert.h"

#include "IECoreGL/Camera.h"
#include "IECoreGL/Group.h"
#include "IECoreGL/Renderer.h"
#include "IECoreGL/Scene.h"

#include "IECoreScene/WorldBlock.h"

#include "IECorePython/ScopedGILLock.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "DDImage/Knob.h"
#include "DDImage/Knobs.h"
#include "DDImage/ViewerContext.h"

using namespace IECoreNuke;

const DD::Image::Op::Description DrawableHolder::g_description( "ieDrawable", build );

DrawableHolder::DrawableHolder( Node *node )
	:	ParameterisedHolderOp( node ),
		m_scene( 0 ),
		m_transform( DD::Image::Matrix4::identity() ),
		m_transformKnob( 0 )
{
	inputs( 0 );
}

DrawableHolder::~DrawableHolder()
{
}

void DrawableHolder::knobs( DD::Image::Knob_Callback f )
{
	ParameterisedHolderOp::knobs( f );

	DD::Image::Tab_knob( f, "Transform" );

	m_transformKnob = DD::Image::Axis_knob( f, &m_transform, "transform", "Transform" );

}

void DrawableHolder::build_handles( DD::Image::ViewerContext *ctx )
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

		IECoreGL::ConstScenePtr s = scene();
		if( s )
		{
			Imath::Box3f b = s->root()->bound();
			if( b.hasVolume() )
			{
				ctx->expand_bbox( node_selected(), IECore::convert<DD::Image::Box3>( b ) );
			}
		}

		add_draw_handle( ctx );

	ctx->modelmatrix = parentMatrix;
}

void DrawableHolder::draw_handle( DD::Image::ViewerContext *ctx )
{
	if( ctx->draw_solid() )
	{
		IECoreGL::ConstScenePtr s = scene();
		if( s )
		{

			// nuke apparently uses the name stack to determine which handle is under
			// the mouse. the IECoreGL::NameStateComponent will ruin this by overwriting
			// the current name. we work around this by pushing another name onto the stack.
			// the NameStateComponent will overwrite this name, but nuke will still detect
			// hits on the drawable using the original name one level lower in the stack.
			glPushName( 0 );

				try
				{
					s->render();
				}
				catch( const std::exception &e )
				{
					IECore::msg( IECore::Msg::Error, "DrawableHolder::draw_handle", e.what() );
				}

			glPopName();

		}
	}
}

IECoreGL::ConstScenePtr DrawableHolder::scene()
{
	if( m_scene && hash()==m_sceneHash )
	{
		return m_scene;
	}

	m_scene = 0;

	IECore::ConstRunTimeTypedPtr drawable = parameterised();
	if( drawable )
	{
		setParameterValues();

		try
		{

			IECoreGL::RendererPtr renderer = new IECoreGL::Renderer();
			renderer->setOption( "gl:mode", new IECore::StringData( "deferred" ) );

			{
				IECoreScene::WorldBlock worldBlock( renderer );
				{
					IECorePython::ScopedGILLock gilLock;
					boost::python::object pythonDrawable( boost::const_pointer_cast<IECore::RunTimeTyped>( drawable ) );
					pythonDrawable.attr( "draw" )( IECoreScene::RendererPtr( renderer ) );
				}
			}

			m_scene = renderer->scene();
			m_scene->setCamera( 0 );

		}
		catch( boost::python::error_already_set )
		{
			IECorePython::ScopedGILLock gilLock;
			PyErr_Print();
		}
		catch( const std::exception &e )
		{
			IECore::msg( IECore::Msg::Error, "DrawableHolder::scene", e.what() );
		}
		catch( ... )
		{
			IECore::msg( IECore::Msg::Error, "DrawableHolder::scene", "Caught unknown exception" );
		}

	}

	m_sceneHash = hash();
	return m_scene;
}

Imath::M44f DrawableHolder::transform()
{
	return IECore::convert<Imath::M44f>( m_transform );
}

DD::Image::Op *DrawableHolder::build( Node *node )
{
	return new DrawableHolder( node );
}

const char *DrawableHolder::Class() const
{
	return g_description.name;
}

const char *DrawableHolder::node_help() const
{
	return "Displays drawable things.";
}
