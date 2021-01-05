//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

//////////////////////////////////////////////////////////////////////////
// ScopedBinding implementation
//////////////////////////////////////////////////////////////////////////

FrameBuffer::ScopedBinding::ScopedBinding( const FrameBuffer &frameBuffer, GLenum target )
	:	m_target( target ), m_prevDrawBuffer( -1 ), m_prevReadBuffer( -1 )
{
	switch( target )
	{
		case GL_DRAW_FRAMEBUFFER :
			glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &m_prevDrawBuffer );
			break;
		case GL_READ_FRAMEBUFFER :
			glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &m_prevReadBuffer );
			break;
		case GL_FRAMEBUFFER :
			glGetIntegerv( GL_DRAW_FRAMEBUFFER_BINDING, &m_prevDrawBuffer );
			glGetIntegerv( GL_READ_FRAMEBUFFER_BINDING, &m_prevReadBuffer );
			break;
		default :
			throw IECore::Exception( "IECoreGL::FrameBuffer::ScopedBinding : Unknown target type" );
	}

	glBindFramebuffer( m_target, frameBuffer.m_frameBuffer );
}

FrameBuffer::ScopedBinding::~ScopedBinding()
{
	if( m_prevDrawBuffer >= 0 )
	{
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_prevDrawBuffer );
	}
	if( m_prevReadBuffer >= 0 )
	{
		glBindFramebuffer( GL_READ_FRAMEBUFFER, m_prevReadBuffer );
	}
}

//////////////////////////////////////////////////////////////////////////
// FrameBuffer implementation
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( FrameBuffer );

FrameBuffer::FrameBuffer()
	:	m_depthAttachment( nullptr )
{
	glGenFramebuffers( 1, &m_frameBuffer );
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers( 1, &m_frameBuffer );
}

GLuint FrameBuffer::frameBuffer() const
{
	return m_frameBuffer;
}

unsigned int FrameBuffer::maxColors()
{
	GLint m;
	glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS, &m );
	return m;
}

void FrameBuffer::setColor( TexturePtr texture, unsigned int index )
{
	if( index >= maxColors() )
	{
		throw IECore::Exception( "Attachment index exceeds GL_MAX_COLOR_ATTACHMENTS." );
	}

	ScopedBinding binding( *this );

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, texture->m_texture, 0 );
	m_colorAttachments.resize( std::max( (unsigned int)m_colorAttachments.size(), (unsigned int)index + 1 ) );
	m_colorAttachments[index] = texture;
}

TexturePtr FrameBuffer::getColor( unsigned int index )
{
	if( index >= m_colorAttachments.size() )
	{
		return nullptr;
	}
	return m_colorAttachments[index];
}

ConstTexturePtr FrameBuffer::getColor( unsigned int index ) const
{
	if( index >= m_colorAttachments.size() )
	{
		return nullptr;
	}
	return m_colorAttachments[index];
}

void FrameBuffer::setDepth( DepthTexturePtr depthTexture )
{
	ScopedBinding binding( *this );

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture->m_texture, 0 );
	m_depthAttachment = depthTexture;
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
	ScopedBinding binding( *this );

	GLenum status;
	status = glCheckFramebufferStatus( GL_FRAMEBUFFER );

    switch( status )
	{
		case GL_FRAMEBUFFER_COMPLETE :
			return;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT :
			throw IECoreGL::Exception( "Framebuffer incomplete - incomplete attachment." );
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT :
			throw IECoreGL::Exception( "Framebuffer incomplete - missing attachment." );
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER :
			throw IECoreGL::Exception( "Framebuffer incomplete - missing draw buffer." );
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER :
			throw IECoreGL::Exception( "Framebuffer incomplete - missing read buffer." );
		case GL_FRAMEBUFFER_UNSUPPORTED :
			throw IECoreGL::Exception( "Unsupported framebuffer format." );
		default :
			throw IECoreGL::Exception( "Unknown framebuffer error." );
    }
}
