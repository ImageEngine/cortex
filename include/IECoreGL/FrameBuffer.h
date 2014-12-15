//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_FRAMEBUFFER_H
#define IECOREGL_FRAMEBUFFER_H

#include <vector>

#include "IECore/RunTimeTyped.h"

#include "IECoreGL/Export.h"
#include "IECoreGL/GL.h"
#include "IECoreGL/TypeIds.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Texture );
IE_CORE_FORWARDDECLARE( DepthTexture );

/// The FrameBuffer object provides a nice reference counted wrapper
/// around the OpenGL framebuffer object extension. It uses the Texture
/// classes to set the components of the framebuffer.
class IECOREGL_API FrameBuffer : public IECore::RunTimeTyped
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::FrameBuffer, FrameBufferTypeId, IECore::RunTimeTyped );

		/// Makes a new framebuffer. At this point the buffer is empty - you must use
		/// the set*() functions below to provide locations to draw to before using
		/// it.
		FrameBuffer();
		virtual ~FrameBuffer();

		/// Returns the GL handle for the framebuffer. Note that this is
		/// owned by the FrameBuffer class and will be destroyed in the
		/// destructor - you must therefore not call glDeleteFramebuffers()
		/// yourself.
		GLuint frameBuffer() const;

		/// Returns the maximum number of color attachments available
		/// in the calls below (the maximum allowable value for index).
		static unsigned int maxColors();
		/// Sets the texture to render colour output to. Multiple color outputs
		/// may be specified by specifying several indices
		void setColor( TexturePtr texture, unsigned int index = 0 );
		/// Returns the texture being used for the specified color channel, or
		/// 0 if no such texture has been specified.
		TexturePtr getColor( unsigned int index = 0 );
		/// Returns the texture being used for the specified color channel, or
		/// 0 if no such texture has been specified.
		ConstTexturePtr getColor( unsigned int index = 0 ) const;
		/// Sets the texture to be used as the depth buffer.
		void setDepth( DepthTexturePtr depthTexture );
		/// Returns the texture being used for the depth buffer, or 0 if
		/// none has been specified.
		DepthTexturePtr getDepth();
		/// Returns the texture being used for the depth buffer, or 0 if
		/// none has been specified.
		ConstDepthTexturePtr getDepth() const;

		/// Throws a descriptive Exception if there is any problem with the
		/// framebuffer.
		void validate() const;

		/// The ScopedBinding class allows the FrameBuffer to be bound to a target
		/// for a specific duration, without worrying about remembering to
		/// unbind it.
		class ScopedBinding
		{
			
			public :
			
				/// Binds the specified FrameBuffer to the specified target.
				ScopedBinding( const FrameBuffer &frameBuffer, GLenum target = GL_FRAMEBUFFER  );
				/// Rebinds the previously bound FrameBuffer.
				~ScopedBinding();
				
			private :
			
				GLenum m_target;
				GLint m_prevDrawBuffer;
				GLint m_prevReadBuffer;
			
		};

	private :

		GLuint m_frameBuffer;
		std::vector<TexturePtr> m_colorAttachments;
		DepthTexturePtr m_depthAttachment;

};

IE_CORE_DECLAREPTR( FrameBuffer );

} // namespace IECoreGL

#endif // IECOREGL_FRAMEBUFFER_H
