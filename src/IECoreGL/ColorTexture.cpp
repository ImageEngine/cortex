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

#include "IECoreGL/ColorTexture.h"

#include "IECoreGL/Exception.h"
#include "IECoreGL/NumericTraits.h"

#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"

using namespace IECoreGL;
using namespace IECore;
using namespace Imath;
using namespace std;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( ColorTexture );

ColorTexture::ColorTexture( unsigned int width, unsigned int height )
{
	glGenTextures( 1, &m_texture );
	ScopedBinding binding( *this );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
		GL_FLOAT, nullptr );
}

ColorTexture::ColorTexture( unsigned int width, unsigned int height, const IECore::Data *r,
	const IECore::Data *g, const IECore::Data *b, const IECore::Data *a )
{
	construct( width, height, r, g, b, a, true );
}

static const Data *findChannel( const IECoreImage::ImagePrimitive::ChannelMap &channels, const char **names )
{
	while( *names != nullptr )
	{
		const auto it = channels.find( *names );
		if( it != channels.end() )
		{
			return it->second.get();
		}
		names++;
	}
	return nullptr;
}

ColorTexture::ColorTexture( const IECoreImage::ImagePrimitive *image )
{
	construct( image, true );
}

ColorTexture::ColorTexture( const IECoreImage::ImagePrimitive *image, bool mipMap )
{
	construct( image, mipMap );
}

ColorTexture::~ColorTexture()
{
}

void ColorTexture::construct( const IECoreImage::ImagePrimitive *image, bool mipMap )
{
	static const char *redNames[] = { "r", "R", "red", nullptr };
	static const char *greenNames[] = { "g", "G", "green", nullptr };
	static const char *blueNames[] = { "b", "B", "blue", nullptr };
	static const char *alphaNames[] = { "a", "A", "alpha", nullptr };

	const IECore::Data *r = findChannel( image->channels, redNames );
	const IECore::Data *g = findChannel( image->channels, greenNames );
	const IECore::Data *b = findChannel( image->channels, blueNames );
	const IECore::Data *a = findChannel( image->channels, alphaNames );

	if( !(r && g && b) )
	{
		throw IECore::Exception( "Unsupported color format." );
	}

	int width = image->getDataWindow().size().x + 1;
	int height = image->getDataWindow().size().y + 1;
	construct( width, height, r, g, b, a, mipMap );
}

void ColorTexture::construct( unsigned int width, unsigned int height, const IECore::Data *r,
	const IECore::Data *g, const IECore::Data *b, const IECore::Data *a, bool mipMap )
{
	if( r->typeId() != g->typeId() ||
		r->typeId() != b->typeId() ||
		( a && (r->typeId() != a->typeId()) ) )
	{
		throw IECore::Exception( "Channel types do not match." );
	}

	if( r->typeId()==UCharVectorData::staticTypeId() )
	{
		castConstruct<UCharVectorData>( width, height, r, g, b, a, mipMap );
	}
	else if( r->typeId()==CharVectorData::staticTypeId() )
	{
		castConstruct<CharVectorData>( width, height, r, g, b, a, mipMap );
	}
	else if( r->typeId()==UIntVectorData::staticTypeId() )
	{
		castConstruct<UIntVectorData>( width, height, r, g, b, a, mipMap );
	}
	else if( r->typeId()==IntVectorData::staticTypeId() )
	{
		castConstruct<IntVectorData>( width, height, r, g, b, a, mipMap );
	}
	else if( r->typeId()==HalfVectorData::staticTypeId() )
	{
		castConstruct<HalfVectorData>( width, height, r, g, b, a, mipMap );
	}
	else if( r->typeId()==FloatVectorData::staticTypeId() )
	{
		castConstruct<FloatVectorData>( width, height, r, g, b, a, mipMap );
	}
	else if( r->typeId()==DoubleVectorData::staticTypeId() )
	{
		castConstruct<DoubleVectorData>( width, height, r, g, b, a, mipMap );
	}
	else {
		throw IECore::Exception( boost::str( boost::format( "Unsupported channel type \"%s\"." ) % r->typeName() ) );
	}
}

template<class T>
void ColorTexture::castConstruct( unsigned int width, unsigned int height, const IECore::Data *r,
	const IECore::Data *g, const IECore::Data *b, const IECore::Data *a, bool mipMap )
{
	templateConstruct( width, height,
		static_cast<const T *>( r ),
		static_cast<const T *>( g ),
		static_cast<const T *>( b ),
		static_cast<const T *>( a ),
		mipMap
	);
}

template<typename T>
void ColorTexture::templateConstruct( unsigned int width, unsigned int height, const T *r,
	const T *g, const T *b, const T *a, bool mipMap )
{
	typedef typename T::ValueType::value_type ElementType;

	const std::vector<ElementType> &rr = r->readable();
	const std::vector<ElementType> &rg = g->readable();
	const std::vector<ElementType> &rb = b->readable();
	const std::vector<ElementType> *ra = a ? &a->readable() : nullptr;

	unsigned int n = width * height;
	if( rr.size()!=n || rg.size()!=n || rb.size()!=n || (ra && ra->size()!=n) )
	{
		throw IECore::Exception( "Image data has wrong size." );
	}

	std::vector<ElementType> interleaved( n * (ra ? 4 : 3) );

	unsigned int i = 0;
	for( int y=height-1; y>=0; y-- )
	{
		const ElementType *dr = &rr[y*width];
		const ElementType *dg = &rg[y*width];
		const ElementType *db = &rb[y*width];
		const ElementType *da = ra ? &(*ra)[y*width] : nullptr;

		for( unsigned int x=0; x<width; x++ )
		{
			interleaved[i++] = dr[x];
			interleaved[i++] = dg[x];
			interleaved[i++] = db[x];
			if( da )
			{
				interleaved[i++] = da[x];
			}
		}
	}

	glGenTextures( 1, &m_texture );

	ScopedBinding binding( *this );

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	if( mipMap )
	{
		/// \todo Why don't we choose an internal format matching the data provided, rather than using 16
		/// bit integers for everything? Hardcoding GL_RGB16 means that sometimes we're using more memory than
		/// needed, and sometimes we're throwing away information.
		gluBuild2DMipmaps( GL_TEXTURE_2D, ra ? GL_RGBA16 : GL_RGB16, width, height, ra ? GL_RGBA : GL_RGB,
			NumericTraits<ElementType>::glType(), &interleaved[0] );
	}
	else
	{
		glTexImage2D( GL_TEXTURE_2D, 0, ra ? GL_RGBA16 : GL_RGB16, width, height, 0, ra ? GL_RGBA : GL_RGB,
			NumericTraits<ElementType>::glType(), &interleaved[0] );
	}

	IECoreGL::Exception::throwIfError();
}

IECoreImage::ImagePrimitivePtr ColorTexture::imagePrimitive() const
{
	ScopedBinding binding( *this );

	GLint width = 0;
	GLint height = 0;
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height );

	GLint internalFormat = 0;
	glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat );

	unsigned int numChannels = 4;
	vector<float> data( width * height * numChannels );

	glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &data[0] );

	FloatVectorDataPtr rd = new FloatVectorData();
	vector<float> &r = rd->writable(); r.resize( width * height );

	FloatVectorDataPtr gd = new FloatVectorData();
	vector<float> &g = gd->writable(); g.resize( width * height );

	FloatVectorDataPtr bd = new FloatVectorData();
	vector<float> &b = bd->writable(); b.resize( width * height );

	FloatVectorDataPtr ad = nullptr;
	vector<float> *a = nullptr;
	// there are potentially loads of different internal formats which denote alpha.
	// these are the only ones encountered so far, but it's not a great way of testing
	// and i can't find another way of doing it.
	if( internalFormat==GL_RGBA || internalFormat == GL_RGBA32F || internalFormat==GL_RGBA8_EXT || internalFormat==GL_RGBA16 )
	{
		ad = new FloatVectorData();
		a = &ad->writable(); a->resize( width * height );
	}

	unsigned int i = 0;
	for( int y=height-1; y>=0; y-- )
	{
		float *rr = &r[y*width];
		float *rg = &g[y*width];
		float *rb = &b[y*width];
		float *ra = a ? &(*a)[y*width] : nullptr;
		for( int x=0; x<width; x++ )
		{
			rr[x] = data[i++];
			rg[x] = data[i++];
			rb[x] = data[i++];
			if( ra )
			{
				ra[x] = data[i++];
			}
		}
	}

	Box2i imageExtents( V2i( 0, 0 ), V2i( width-1, height-1 ) );
	IECoreImage::ImagePrimitivePtr image = new IECoreImage::ImagePrimitive( imageExtents, imageExtents );
	image->channels["R"] = rd;
	image->channels["G"] = gd;
	image->channels["B"] = bd;
	if( a )
	{
		image->channels["A"] = ad;
	}

	IECoreGL::Exception::throwIfError();

	return image;
}
