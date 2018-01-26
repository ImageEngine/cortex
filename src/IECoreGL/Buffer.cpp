//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/Buffer.h"

#include "IECore/Exception.h"

using namespace IECoreGL;

//////////////////////////////////////////////////////////////////////////
// ScopedBinding implementation
//////////////////////////////////////////////////////////////////////////

Buffer::ScopedBinding::ScopedBinding( const Buffer &buffer, GLenum target )
	:	m_target( target ), m_buffer( buffer.m_buffer ), m_prevBuffer( 0 )
{
	switch( target )
	{
		case GL_ARRAY_BUFFER :
			glGetIntegerv( GL_ARRAY_BUFFER_BINDING, &m_prevBuffer );
			break;
		case GL_ELEMENT_ARRAY_BUFFER :
			glGetIntegerv( GL_ELEMENT_ARRAY_BUFFER_BINDING, &m_prevBuffer );
			break;
		case GL_PIXEL_PACK_BUFFER :
			glGetIntegerv( GL_PIXEL_PACK_BUFFER_BINDING, &m_prevBuffer );
			break;
		case GL_PIXEL_UNPACK_BUFFER :
			glGetIntegerv( GL_PIXEL_UNPACK_BUFFER_BINDING, &m_prevBuffer );
			break;
		default :
			throw IECore::Exception( "IECoreGL::Buffer::ScopedBinding : Unknown target type" );
	}

	glBindBuffer( m_target, m_buffer );
}

Buffer::ScopedBinding::~ScopedBinding()
{
	glBindBuffer( m_target, m_prevBuffer );
}

//////////////////////////////////////////////////////////////////////////
// Buffer implementation
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( Buffer )

Buffer::Buffer( GLuint buffer )
	:	m_buffer( buffer )
{
}

Buffer::Buffer( const void *data, size_t sizeInBytes, GLenum target, GLenum usage )
{
	glGenBuffers( 1, &m_buffer );
	ScopedBinding binding( *this, target );
	glBufferData( target, sizeInBytes, data, usage );
}

Buffer::~Buffer()
{
	glDeleteBuffers( 1, &m_buffer );
}

size_t Buffer::size() const
{
	ScopedBinding binding( *this, GL_ARRAY_BUFFER );
	int result = 0;
	glGetBufferParameteriv( GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &result );
	return result;
}

