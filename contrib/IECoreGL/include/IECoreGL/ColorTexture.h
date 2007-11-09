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

#ifndef IECOREGL_COLORTEXTURE_H
#define IECOREGL_COLORTEXTURE_H

#include "IECoreGL/Texture.h"

namespace IECoreGL
{

/// The ColorTexture class represents a texture with RGB
/// channels and an optional alpha channel.
/// It is suitable for use as the color attachment for
/// a FrameBuffer.
class ColorTexture : public Texture
{
	public :
				
		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ColorTexture, ColorTextureTypeId, Texture );
		
		/// Constructs an empty texture of the specified dimensions.
		ColorTexture( unsigned int width, unsigned int height );
		/// Constructs a new ColorTexture. All channels must be of the same type, and must
		/// be some form of numeric VectorData.
		ColorTexture( unsigned int width, unsigned int height, IECore::ConstDataPtr r,
			IECore::ConstDataPtr g, IECore::ConstDataPtr b, IECore::ConstDataPtr a = 0 );
		
		/// Creates a ColorTexture from the specified image. Accepts channels with names
		/// "r", "R", "red", "g", "G", "green", "b", "B", "blue", "a", "A" and "alpha".
		/// Currently ignores the display window and uses only the data window. 
		/// Image must have at least RGB channels and all channels
		/// must be of the same type.
		ColorTexture( IECore::ConstImagePrimitivePtr image );
					
		virtual ~ColorTexture();

		/// Creates an ImagePrimitive using the texture contents.
		virtual IECore::ImagePrimitivePtr imagePrimitive() const;

	private :
	
		void construct( unsigned int width, unsigned int height, IECore::ConstDataPtr r,
			IECore::ConstDataPtr g, IECore::ConstDataPtr b, IECore::ConstDataPtr a );
	
		template<typename T>
		void castConstruct( unsigned int width, unsigned int height, IECore::ConstDataPtr r,
			IECore::ConstDataPtr g, IECore::ConstDataPtr b, IECore::ConstDataPtr a );
	
		template<typename T>
		void templateConstruct( unsigned int width, unsigned int height, boost::intrusive_ptr<const T> r,
			boost::intrusive_ptr<const T> g,  boost::intrusive_ptr<const T> b, boost::intrusive_ptr<const T> a );
					
};

IE_CORE_DECLAREPTR( ColorTexture );

} // namespace IECoreGL

#endif // IECOREGL_COLORTEXTURE_H
