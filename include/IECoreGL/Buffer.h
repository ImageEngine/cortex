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

#ifndef IECOREGL_BUFFER_H
#define IECOREGL_BUFFER_H

#include "IECore/RunTimeTyped.h"

#include "IECoreGL/Export.h"
#include "IECoreGL/GL.h"
#include "IECoreGL/TypeIds.h"

namespace IECoreGL
{

/// The Buffer class provides a simple reference counted wrapper
/// around an OpenGL buffer object, making the lifetime management
/// of shared buffers straightforward.
class IECOREGL_API Buffer : public IECore::RunTimeTyped
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( Buffer, BufferTypeId, IECore::RunTimeTyped );

		/// Wraps an existing buffer. Ownership of the buffer is taken,
		/// and it will be deleted with glDeleteBuffers() in the destructor.
		Buffer( GLuint buffer );
		/// Creates a buffer from the specified data.
		Buffer( const void *data, size_t sizeInBytes, GLenum target = GL_ARRAY_BUFFER, GLenum usage = GL_STATIC_DRAW );
		/// Deletes the buffer with glDeleteBuffers().
		~Buffer() override;

		/// Returns the size of the buffer in bytes.
		size_t size() const;

		/// The ScopedBinding class allows the buffer to be bound to a target
		/// for a specific duration, without worrying about remembering to
		/// unbind it.
		class ScopedBinding
		{

			public :

				/// Binds the specified buffer to the specified target.
				ScopedBinding( const Buffer &buffer, GLenum target = GL_ARRAY_BUFFER  );
				/// Rebinds the previously bound buffer.
				~ScopedBinding();

			private :

				GLenum m_target;
				GLuint m_buffer;
				GLint m_prevBuffer;

		};

	private :

		GLuint m_buffer;

};

IE_CORE_DECLAREPTR( Buffer );

} // namespace IECoreGL

#endif // IECOREGL_BUFFER_H
