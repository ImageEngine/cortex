//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_ALPHATEXTURE_H
#define IECOREGL_ALPHATEXTURE_H

#include "IECoreGL/Texture.h"

namespace IECoreGL
{

class AlphaTexture : public Texture
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::AlphaTexture, AlphaTextureTypeId, Texture );

		/// Constructs a texture using the specified data.
		AlphaTexture( unsigned int width, unsigned int height, const IECore::Data *a, bool mipMap=true );

		/// Creates an AlphaTexture from the specified image. Currently ignores the display window and uses only
		/// the data window. Image must have an "A" channel.
		AlphaTexture( const IECore::ImagePrimitive *image, bool mipMap=true );

		virtual ~AlphaTexture();

		virtual IECore::ImagePrimitivePtr imagePrimitive() const;

	private :

		struct Constructor;

		void construct( unsigned int width, unsigned int height, const IECore::Data *a, bool mipMap );

};

IE_CORE_DECLAREPTR( AlphaTexture );

} // namespace IECoreGL

#endif // IECOREGL_ALPHATEXTURE_H
