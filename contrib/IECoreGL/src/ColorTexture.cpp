//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/VectorTypedData.h"
#include "IECore/MessageHandler.h"

using namespace IECoreGL;
using namespace IECore;
using namespace Imath;
using namespace std;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( ColorTexture );

ColorTexture::ColorTexture( unsigned int width, unsigned int height )
{
	glGenTextures( 1, &m_texture );
	glBindTexture( GL_TEXTURE_2D, m_texture );
		
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
		GL_FLOAT, 0 );
}

ColorTexture::ColorTexture( unsigned int width, unsigned int height, IECore::ConstDataPtr r,
	IECore::ConstDataPtr g, IECore::ConstDataPtr b, IECore::ConstDataPtr a )
{
	construct( width, height, r, g, b, a );
}

static IECore::ConstDataPtr findChannel( const IECore::PrimitiveVariableMap &variables, const char **names )
{
	while( *names!=0 )
	{
		IECore::PrimitiveVariableMap::const_iterator it = variables.find( *names );
		if( it!=variables.end() )
		{
			PrimitiveVariable::Interpolation interpolation = it->second.interpolation;
			if( interpolation==PrimitiveVariable::Vertex ||
				interpolation==PrimitiveVariable::Varying ||
				interpolation==PrimitiveVariable::FaceVarying )
			{
				return it->second.data;
			}
		}
		names++;
	}
	return 0;
}

ColorTexture::ColorTexture( IECore::ConstImagePrimitivePtr image )
{
	static const char *redNames[] = { "r", "R", "red", 0 };
	static const char *greenNames[] = { "g", "G", "green", 0 };
	static const char *blueNames[] = { "b", "B", "blue", 0 };
	static const char *alphaNames[] = { "a", "A", "alpha", 0 };
	
	IECore::ConstDataPtr r = findChannel( image->variables, redNames );
	IECore::ConstDataPtr g = findChannel( image->variables, greenNames );
	IECore::ConstDataPtr b = findChannel( image->variables, blueNames );
	IECore::ConstDataPtr a = findChannel( image->variables, alphaNames );
	
	if( !(r && g && b) )
	{
		throw Exception( "Unsupported color format." );
	}
	
	int width = image->getDataWindow().size().x + 1;
	int height = image->getDataWindow().size().y + 1;
	construct( width, height, r, g, b, a );
}
				
ColorTexture::~ColorTexture()
{
}

void ColorTexture::construct( unsigned int width, unsigned int height, IECore::ConstDataPtr r,
	IECore::ConstDataPtr g, IECore::ConstDataPtr b, IECore::ConstDataPtr a )
{
	if( r->typeId() != g->typeId() ||
		r->typeId() != b->typeId() ||
		( a && (r->typeId() != a->typeId()) ) )
	{
		throw Exception( "Channel types do not match." );
	}
		
	if( r->typeId()==UCharVectorData::staticTypeId() )
	{
		castConstruct<UCharVectorData>( width, height, r, g, b, a );
	}
	else if( r->typeId()==CharVectorData::staticTypeId() )
	{
		castConstruct<CharVectorData>( width, height, r, g, b, a );
	}
	else if( r->typeId()==UIntVectorData::staticTypeId() )
	{
		castConstruct<UIntVectorData>( width, height, r, g, b, a );
	}
	else if( r->typeId()==IntVectorData::staticTypeId() )
	{
		castConstruct<IntVectorData>( width, height, r, g, b, a );
	}
	else if( r->typeId()==HalfVectorData::staticTypeId() )
	{
		castConstruct<HalfVectorData>( width, height, r, g, b, a );
	}
	else if( r->typeId()==FloatVectorData::staticTypeId() )
	{
		castConstruct<FloatVectorData>( width, height, r, g, b, a );
	}
	else if( r->typeId()==DoubleVectorData::staticTypeId() )
	{
		castConstruct<DoubleVectorData>( width, height, r, g, b, a );
	}
	else {
		throw Exception( boost::str( boost::format( "Unsupported channel type \"%s\"." ) % r->typeName() ) );
	}
}

template<class T>
void ColorTexture::castConstruct( unsigned int width, unsigned int height, IECore::ConstDataPtr r,
	IECore::ConstDataPtr g, IECore::ConstDataPtr b, IECore::ConstDataPtr a )
{
	templateConstruct( width, height,
		static_pointer_cast<const T>( r ),
		static_pointer_cast<const T>( g ),
		static_pointer_cast<const T>( b ),
		static_pointer_cast<const T>( a )	);
}

template<typename T>
void ColorTexture::templateConstruct( unsigned int width, unsigned int height, boost::intrusive_ptr<const T> r,
	boost::intrusive_ptr<const T> g,  boost::intrusive_ptr<const T> b, boost::intrusive_ptr<const T> a )
{
	typedef typename T::ValueType::value_type ElementType;
	
	const std::vector<ElementType> &rr = r->readable();
	const std::vector<ElementType> &rg = g->readable();
	const std::vector<ElementType> &rb = b->readable();
	const std::vector<ElementType> *ra = a ? &a->readable() : 0;
	
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
		const ElementType *da = ra ? &(*ra)[y*width] : 0;
		
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
	glBindTexture( GL_TEXTURE_2D, m_texture );
	
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	
	glTexImage2D( GL_TEXTURE_2D, 0, ra ? GL_RGBA : GL_RGB, width, height, 0, ra ? GL_RGBA : GL_RGB,
		NumericTraits<ElementType>::glType(), &interleaved[0] );
		
	Exception::throwIfError();
	
}

ImagePrimitivePtr ColorTexture::imagePrimitive() const
{
	glPushAttrib( mask() );
		
		bind();
	
		GLint width = 0;
		GLint height = 0;
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height );
			
		GLint internalFormat = 0;
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat );

		unsigned int numChannels = 4;
		vector<float> data( width * height * numChannels );
		
		glGetTexImage( GL_TEXTURE_2D, 0, internalFormat, GL_FLOAT, &data[0] );
		
		FloatVectorDataPtr rd = new FloatVectorData();
		vector<float> &r = rd->writable(); r.resize( width * height );
		
		FloatVectorDataPtr gd = new FloatVectorData();
		vector<float> &g = gd->writable(); g.resize( width * height );
		
		FloatVectorDataPtr bd = new FloatVectorData();
		vector<float> &b = bd->writable(); b.resize( width * height );
		
		FloatVectorDataPtr ad = 0;
		vector<float> *a = 0;
		if( internalFormat==GL_RGBA )
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
			float *ra = a ? &(*a)[y*width] : 0;
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
		ImagePrimitivePtr image = new ImagePrimitive( imageExtents, imageExtents );
		image->variables["R"] = PrimitiveVariable( PrimitiveVariable::Vertex, rd );
		image->variables["G"] = PrimitiveVariable( PrimitiveVariable::Vertex, gd );
		image->variables["B"] = PrimitiveVariable( PrimitiveVariable::Vertex, bd );
		if( a )
		{
			image->variables["A"] = PrimitiveVariable( PrimitiveVariable::Vertex, ad );
		}
	
	glPopAttrib();
	
	return image;
}
