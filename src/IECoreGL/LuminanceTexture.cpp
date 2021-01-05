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

#include "IECoreGL/LuminanceTexture.h"

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

IE_CORE_DEFINERUNTIMETYPED( LuminanceTexture );

LuminanceTexture::LuminanceTexture( unsigned int width, unsigned int height, const IECore::Data *y, const IECore::Data *a, bool mipMap )
{
	construct( width, height, y, a, mipMap );
}

LuminanceTexture::LuminanceTexture( const IECoreImage::ImagePrimitive *image, bool mipMap )
{
	const IECore::Data *y = image->channelValid( "Y" ) ? image->channels.find( "Y" )->second.get() : nullptr;
	const IECore::Data *a = image->channelValid( "A" ) ? image->channels.find( "A" )->second.get() : nullptr;

	if( !y )
	{
		throw IECore::Exception( "Image must have at least a \"Y\" channel." );
	}

	int width = image->getDataWindow().size().x + 1;
	int height = image->getDataWindow().size().y + 1;
	construct( width, height, y, a, mipMap );
}

LuminanceTexture::~LuminanceTexture()
{
}

struct LuminanceTexture::Constructor
{
	typedef bool ReturnType;

	template<typename T>
	GLuint operator()( typename T::ConstPtr y )
	{
		typedef typename T::ValueType::value_type ElementType;

		const std::vector<ElementType> &ry = y->readable();
		const std::vector<ElementType> *ra = alpha ? &(boost::static_pointer_cast<const T>( alpha )->readable()) : nullptr;

		unsigned int n = width * height;
		if( ry.size()!=n || (ra && ra->size()!=n) )
		{
			throw IECore::Exception( "Image data has wrong size." );
		}

		std::vector<ElementType> interleaved( n * (ra ? 2 : 1) );

		unsigned int i = 0;
		for( int y=height-1; y>=0; y-- )
		{
			const ElementType *dy = &ry[y*width];
			const ElementType *da = ra ? &(*ra)[y*width] : nullptr;

			for( unsigned int x=0; x<width; x++ )
			{
				interleaved[i++] = dy[x];
				if( da )
				{
					interleaved[i++] = da[x];
				}
			}
		}

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

		if( mipMap )
		{
			gluBuild2DMipmaps( GL_TEXTURE_2D, ra ? GL_LUMINANCE_ALPHA : GL_LUMINANCE, width, height, ra ? GL_LUMINANCE_ALPHA : GL_LUMINANCE,
				NumericTraits<ElementType>::glType(), &interleaved[0] );
		}
		else
		{
			glTexImage2D( GL_TEXTURE_2D, 0, ra ? GL_LUMINANCE_ALPHA : GL_LUMINANCE, width, height, 0, ra ? GL_LUMINANCE_ALPHA : GL_LUMINANCE,
				NumericTraits<ElementType>::glType(), &interleaved[0] );
		}

		IECoreGL::Exception::throwIfError();

		return true;
	}

	unsigned width;
	unsigned height;
	bool mipMap;
	ConstDataPtr alpha;
};

void LuminanceTexture::construct( unsigned int width, unsigned int height, const IECore::Data *y, const IECore::Data *a, bool mipMap )
{
	if( a && (y->typeId() != a->typeId()) )
	{
		throw IECore::Exception( "Channel types do not match." );
	}

	glGenTextures( 1, &m_texture );
	ScopedBinding binding( *this );

	Constructor c;
	c.alpha = a;
	c.width = width;
	c.height = height;
	c.mipMap = mipMap;
	IECore::despatchTypedData<Constructor, IECore::TypeTraits::IsNumericVectorTypedData>( const_cast<Data *>( y ), c );
}

IECoreImage::ImagePrimitivePtr LuminanceTexture::imagePrimitive() const
{
	ScopedBinding binding( *this );

	GLint width = 0;
	GLint height = 0;
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height );

	GLint aSize = 0;
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &aSize );

	unsigned int numChannels = aSize ? 2 : 1;

	vector<float> data( width * height * numChannels );

	glGetTexImage( GL_TEXTURE_2D, 0, numChannels==2 ? GL_LUMINANCE_ALPHA : GL_LUMINANCE, GL_FLOAT, &data[0] );

	FloatVectorDataPtr yd = new FloatVectorData();
	vector<float> &y = yd->writable(); y.resize( width * height );

	FloatVectorDataPtr ad = nullptr;
	vector<float> *a = nullptr;
	if( aSize )
	{
		ad = new FloatVectorData();
		a = &ad->writable(); a->resize( width * height );
	}

	unsigned int i = 0;
	for( int yy=height-1; yy>=0; yy-- )
	{
		float *ry = &y[yy*width];
		float *ra = a ? &(*a)[yy*width] : nullptr;
		for( int x=0; x<width; x++ )
		{
			ry[x] = data[i++];
			if( ra )
			{
				ra[x] = data[i++];
			}
		}
	}

	Box2i imageExtents( V2i( 0, 0 ), V2i( width-1, height-1 ) );
	IECoreImage::ImagePrimitivePtr image = new IECoreImage::ImagePrimitive( imageExtents, imageExtents );
	image->channels["Y"] = yd;
	if( a )
	{
		image->channels["A"] = ad;
	}

	IECoreGL::Exception::throwIfError();

	return image;
}

