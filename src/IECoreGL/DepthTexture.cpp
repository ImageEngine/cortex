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

#include "IECoreGL/DepthTexture.h"

#include "IECoreGL/Exception.h"

#include "IECoreImage/ImagePrimitive.h"

#include "IECore/MessageHandler.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( DepthTexture );

DepthTexture::DepthTexture( unsigned int width, unsigned height, const IECore::Data *z )
{
	glGenTextures( 1, &m_texture );
	ScopedBinding binding( *this );

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	if( !z )
	{
		glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT,
			GL_FLOAT, nullptr );
	}
	else
	{
		IECore::msg( IECore::Msg::Warning, "DepthTexture::DepthTexture", "Construction from z data not yet implemented." );
	}

	IECoreGL::Exception::throwIfError();
}

DepthTexture::~DepthTexture()
{
}

IECoreImage::ImagePrimitivePtr DepthTexture::imagePrimitive() const
{
	ScopedBinding binding( *this );

	GLint width = 0;
	GLint height = 0;
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height );

	vector<float> data( width * height );

	glGetTexImage( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &data[0] );

	FloatVectorDataPtr zd = new FloatVectorData();
	vector<float> &z = zd->writable();
	z.resize( width * height );

	unsigned int i = 0;
	for( int y=height-1; y>=0; y-- )
	{
		float *rz = &z[y*width];
		for( int x=0; x<width; x++ )
		{
			rz[x] = data[i++];
		}
	}

	Box2i imageExtents( V2i( 0, 0 ), V2i( width-1, height-1 ) );
	IECoreImage::ImagePrimitivePtr image = new IECoreImage::ImagePrimitive( imageExtents, imageExtents );
	image->channels["Z"] = zd;

	return image;
}
