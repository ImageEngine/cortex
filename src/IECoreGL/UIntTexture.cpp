//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/UIntTexture.h"
#include "IECoreGL/Exception.h"

using namespace IECoreGL;
using namespace IECore;
using namespace IECoreImage;

IE_CORE_DEFINERUNTIMETYPED( UIntTexture );

UIntTexture::UIntTexture( unsigned int width, unsigned int height )
{
	glGenTextures( 1, &m_texture );
	ScopedBinding binding( *this );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0 );
}

UIntTexture::~UIntTexture()
{
}

ImagePrimitivePtr UIntTexture::imagePrimitive() const
{
	ScopedBinding binding( *this );
	GLint width = 0;
	GLint height = 0;
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height );

	std::vector<unsigned int> data( width * height );
	glGetTexImage( GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &data[0] );

	UIntVectorDataPtr uiData = new UIntVectorData();
	std::vector<unsigned int> &ui = uiData->writable();
	ui.resize( width * height );

	unsigned int i = 0;
	for( int y=height-1; y>=0; y-- )
	{
		unsigned int *ry = &ui[y*width];
		for( int x=0; x<width; x++ )
		{
			ry[x] = data[i++];
		}
	}

	Imath::Box2i imageExtents( Imath::V2i( 0, 0 ), Imath::V2i( width-1, height-1 ) );
	ImagePrimitivePtr image = new ImagePrimitive( imageExtents, imageExtents );
	image->channels["Y"] = uiData;

	IECoreGL::Exception::throwIfError();
	return image;
}
