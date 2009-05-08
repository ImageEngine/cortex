//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/private/ImmediateRendererImplementation.h"
#include "IECoreGL/private/Display.h"
#include "IECoreGL/StateComponent.h"
#include "IECoreGL/Primitive.h"
#include "IECoreGL/FrameBuffer.h"
#include "IECoreGL/ColorTexture.h"
#include "IECoreGL/DepthTexture.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/State.h"
#include "IECoreGL/PerspectiveCamera.h"

#include "IECore/MessageHandler.h"
#include "IECore/Writer.h"

#include <cassert>

using namespace IECoreGL;
using namespace Imath;
using namespace std;

ImmediateRendererImplementation::ImmediateRendererImplementation()
{
	m_stateStack.push( new State( true ) );
}

ImmediateRendererImplementation::~ImmediateRendererImplementation()
{
}

void ImmediateRendererImplementation::addCamera( CameraPtr camera )
{
	m_camera = camera;
}

void ImmediateRendererImplementation::addDisplay( ConstDisplayPtr display )
{
	m_displays.push_back( display );
}

void ImmediateRendererImplementation::worldBegin()
{
	unsigned int width = m_camera->getResolution().x;
	unsigned int height = m_camera->getResolution().y;
	try
	{
		m_frameBuffer = new FrameBuffer;
		m_frameBuffer->setColor( new ColorTexture( width, height ) );
		Exception::throwIfError();
		m_frameBuffer->setDepth( new DepthTexture( width, height ) );
		Exception::throwIfError();
		m_frameBuffer->validate();
		m_frameBuffer->bind();
	}
	catch( const std::exception &e )
	{
		/// \todo how about a fallback?
		IECore::msg( IECore::Msg::Error, "Renderer::worldBegin", boost::format( "Unable to make framebuffer (%s)." ) % e.what() );
	}

	m_camera->render( m_stateStack.top() );

	glViewport( 0, 0, width, height );
	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glClearDepth( 1.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	m_stateStack.push( new State( *(m_stateStack.top()) ) );

	glPushAttrib( GL_ALL_ATTRIB_BITS );
	State::bindBaseState();
	m_stateStack.top()->bind();
}

void ImmediateRendererImplementation::worldEnd()
{
	m_stateStack.pop();
	glPopAttrib();
	if( m_frameBuffer )
	{
		for( unsigned int i=0; i<m_displays.size(); i++ )
		{
			m_displays[i]->display( m_frameBuffer );
		}
	}
}

void ImmediateRendererImplementation::transformBegin()
{
	glPushMatrix();
}

void ImmediateRendererImplementation::transformEnd()
{
	glPopMatrix();
}

void ImmediateRendererImplementation::concatTransform( const Imath::M44f &matrix )
{
	glMultMatrixf( matrix.getValue() );
}

void ImmediateRendererImplementation::attributeBegin()
{
	transformBegin();
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	m_stateStack.push( new State( *(m_stateStack.top()) ) );
}

void ImmediateRendererImplementation::attributeEnd()
{
	m_stateStack.pop();
	glPopAttrib();
	transformEnd();
}

void ImmediateRendererImplementation::addState( StateComponentPtr state )
{
	m_stateStack.top()->add( state );
	state->bind();
}

StateComponentPtr ImmediateRendererImplementation::getState( IECore::TypeId type )
{
	return m_stateStack.top()->get( type );
}

void ImmediateRendererImplementation::addPrimitive( PrimitivePtr primitive )
{
	primitive->render( m_stateStack.top() );
}
