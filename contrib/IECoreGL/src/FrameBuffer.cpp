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

#include "IECoreGL/FrameBuffer.h"
#include "IECoreGL/ColorTexture.h"
#include "IECoreGL/DepthTexture.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/GL.h"

#include "IECore/MessageHandler.h"

using namespace IECoreGL;

FrameBuffer::FrameBuffer()
	:	m_depthAttachment( 0 ), m_savedFrameBuffer( 0 )
{
	if( !GLEW_EXT_framebuffer_object )
	{
		throw Exception( "FrameBuffers not supported by this OpenGL implementation.");
	}
	glGenFramebuffersEXT( 1, &m_frameBuffer ); 
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffersEXT( 1, &m_frameBuffer );
}

unsigned int FrameBuffer::maxColors()
{
	GLint m;
	glGetIntegerv( GL_MAX_DRAW_BUFFERS, &m );
	return m;
}

void FrameBuffer::setColor( ColorTexturePtr texture, unsigned int index )
{
	saveAndBind();
	
		if( index )
		{
			IECore::msg( IECore::Msg::Warning, "FrameBuffer::setColor", "Attachment points other than 0 not implemented yet." );
		}
		glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture->m_texture, 0 );
		m_colorAttachments.resize( std::max( (unsigned int)m_colorAttachments.size(), (unsigned int)index + 1 ) );
		m_colorAttachments[index] = texture;
		
	restore();
}

ColorTexturePtr FrameBuffer::getColor( unsigned int index )
{
	if( index >= m_colorAttachments.size() )
	{
		return 0;
	}
	return m_colorAttachments[index];
}

ConstColorTexturePtr FrameBuffer::getColor( unsigned int index ) const
{
	if( index >= m_colorAttachments.size() )
	{
		return 0;
	}
	return m_colorAttachments[index];
}
	
void FrameBuffer::setDepth( DepthTexturePtr depthTexture )
{
	saveAndBind();

		glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depthTexture->m_texture, 0 );
		m_depthAttachment = depthTexture;

	restore();
}

DepthTexturePtr FrameBuffer::getDepth()
{
	return m_depthAttachment;
}

ConstDepthTexturePtr FrameBuffer::getDepth() const
{
	return m_depthAttachment;
}

void FrameBuffer::validate() const
{
	saveAndBind();
		GLenum status;
		status = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );
	restore();
	
    switch( status )
	{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			throw Exception( "Framebuffer incomplete - incomplete attachment." );
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			throw Exception( "Framebuffer incomplete - missing attachment." );
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			throw Exception( "Framebuffer incomplete - attachments don't have same dimensions." );
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			throw Exception( "Framebuffer incomplete - color attachments must have same format." );
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			throw Exception( "Framebuffer incomplete - missing draw buffer." );
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			throw Exception( "Framebuffer incomplete - missing read buffer." );
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			throw Exception( "Unsupported framebuffer format." );
    }
}

void FrameBuffer::bind() const
{
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_frameBuffer );
	/// \todo do the drawbuffers thing.
}

GLbitfield FrameBuffer::mask() const
{
	return 0;
}

void FrameBuffer::saveAndBind() const
{
	glGetIntegerv( GL_FRAMEBUFFER_BINDING_EXT, &m_savedFrameBuffer );
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_frameBuffer );
}

void FrameBuffer::restore() const
{
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_savedFrameBuffer );
}
