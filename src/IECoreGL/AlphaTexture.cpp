//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/AlphaTexture.h"

#include "IECoreGL/Exception.h"
#include "IECoreGL/NumericTraits.h"

#include "IECore/DespatchTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECore/TypeTraits.h"
#include "IECore/VectorTypedData.h"

using namespace IECoreGL;
using namespace IECore;
using namespace Imath;
using namespace std;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( AlphaTexture );

AlphaTexture::AlphaTexture( unsigned int width, unsigned int height, const IECore::Data *a, bool mipMap )
{
	construct( width, height, a, mipMap );
}

AlphaTexture::AlphaTexture( const IECoreImage::ImagePrimitive *image, bool mipMap )
{
	const IECore::Data *a = image->channelValid( "A" ) ? image->channels.find( "A" )->second.get() : nullptr;

	if( !a )
	{
		throw IECore::Exception( "Image must have at least an \"A\" channel." );
	}

	int width = image->getDataWindow().size().x + 1;
	int height = image->getDataWindow().size().y + 1;
	construct( width, height, a, mipMap );
}

AlphaTexture::~AlphaTexture()
{
}

struct AlphaTexture::Constructor
{
	typedef bool ReturnType;

	template<typename T>
	GLuint operator()( typename T::ConstPtr a )
	{
		typedef typename T::ValueType::value_type ElementType;

		const std::vector<ElementType> &ra = a->readable();

		unsigned int n = width * height;
		if( ra.size()!=n )
		{
			throw IECore::Exception( "Channel data has wrong size." );
		}

		std::vector<ElementType> reordered( n );

		unsigned int i = 0;
		for( int y=height-1; y>=0; y-- )
		{
			const ElementType *da = &(ra[y*width]);
			for( unsigned int x=0; x<width; x++ )
			{
				reordered[i++] = da[x];
			}
		}

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		if( mipMap )
		{
			gluBuild2DMipmaps( GL_TEXTURE_2D, GL_ALPHA, width, height, GL_ALPHA,
				NumericTraits<ElementType>::glType(), &reordered[0] );
		}
		else
		{
			glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA,
				NumericTraits<ElementType>::glType(), &reordered[0] );
		}

		IECoreGL::Exception::throwIfError();

		return true;
	}

	unsigned width;
	unsigned height;
	bool mipMap;
};

void AlphaTexture::construct( unsigned int width, unsigned int height, const IECore::Data *a, bool mipMap )
{
	glGenTextures( 1, &m_texture );
	ScopedBinding binding( *this );

	Constructor c;
	c.width = width;
	c.height = height;
	c.mipMap = mipMap;
	IECore::despatchTypedData<Constructor, IECore::TypeTraits::IsNumericVectorTypedData>( const_cast<Data *>( a ), c );
}


IECoreImage::ImagePrimitivePtr AlphaTexture::imagePrimitive() const
{
	ScopedBinding binding( *this );

	GLint width = 0;
	GLint height = 0;
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height );

	vector<float> data( width * height );

	glGetTexImage( GL_TEXTURE_2D, 0, GL_ALPHA, GL_FLOAT, &data[0] );

	FloatVectorDataPtr ad = new FloatVectorData();
	vector<float> &a = ad->writable(); a.resize( width * height );

	unsigned int i = 0;
	for( int y=height-1; y>=0; y-- )
	{
		float *ra = &a[y*width];
		for( int x=0; x<width; x++ )
		{
			ra[x] = data[i++];
		}
	}

	Box2i imageExtents( V2i( 0, 0 ), V2i( width-1, height-1 ) );
	IECoreImage::ImagePrimitivePtr image = new IECoreImage::ImagePrimitive( imageExtents, imageExtents );
	image->channels["A"] = ad;

	return image;
}

