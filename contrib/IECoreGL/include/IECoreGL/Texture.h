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

#ifndef IECOREGL_TEXTURE_H
#define IECOREGL_TEXTURE_H

#include "IECoreGL/GL.h"
#include "IECoreGL/Bindable.h"

#include "IECore/ImagePrimitive.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( FrameBuffer );

/// The Texture class represents a reference counted OpenGL texture.
/// When the Texture object dies, it also removes the associated GL texture.
/// Constructors from various IECore datatypes are provided.
/// \todo Provide some control over resizing (to power of 2), mipmapping,
/// filtering etc.
class Texture : public Bindable
{
	public :
		
		friend class FrameBuffer;
		
		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( Texture, TextureTypeId, Bindable );
		
		/// Creates a texture object to wrap the already created
		/// GL texture specified.
		Texture( GLuint texture );
		virtual ~Texture();
		
		/// Binds the texture as the current GL texture.
		virtual void bind() const;
		virtual GLbitfield mask() const;
		
		/// Creates an ImagePrimitive using the texture contents.
		virtual IECore::ImagePrimitivePtr imagePrimitive() const;
		
	protected :
	
		Texture();
		
		/// Derived classes must set this in their constructor.
		GLuint m_texture;

};

IE_CORE_DECLAREPTR( Texture );

} // namespace IECoreGL

#endif // IECOREGL_TEXTURE_H
