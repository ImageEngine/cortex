//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/VectorTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TypeTraits.h"

using namespace IECoreGL;
using namespace IECore;
using namespace Imath;
using namespace std;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( LuminanceTexture );

LuminanceTexture::LuminanceTexture( unsigned int width, unsigned int height, IECore::ConstDataPtr y, IECore::ConstDataPtr a, bool mipMap )
{
	construct( width, height, y, a, mipMap );
}

LuminanceTexture::LuminanceTexture( IECore::ConstImagePrimitivePtr image, bool mipMap )
{
	IECore::ConstDataPtr y = image->channelValid( "Y" ) ? image->variables.find( "Y" )->second.data : 0;
	IECore::ConstDataPtr a = image->channelValid( "A" ) ? image->variables.find( "A" )->second.data : 0;
	
	if( !y )
	{
		throw Exception( "Image must have at least a \"Y\" channel." );
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
	typedef GLuint ReturnType;
	
	template<typename T>
	GLuint operator()( typename T::ConstPtr y )
	{			
		typedef typename T::ValueType::value_type ElementType;
	
		const std::vector<ElementType> &ry = y->readable();
		const std::vector<ElementType> *ra = alpha ? &(static_pointer_cast<const T>( alpha )->readable()) : 0;
	
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
			const ElementType *da = ra ? &(*ra)[y*width] : 0;

			for( unsigned int x=0; x<width; x++ )
			{
				interleaved[i++] = dy[x];
				if( da )
				{
					interleaved[i++] = da[x];
				}
			}
		}

		GLuint result = 0;
		glGenTextures( 1, &result );
		glBindTexture( GL_TEXTURE_2D, result );
	
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
		
		return result;
	}
	
	unsigned width;
	unsigned height;
	bool mipMap;
	ConstDataPtr alpha;
};

void LuminanceTexture::construct( unsigned int width, unsigned int height, IECore::ConstDataPtr y, IECore::ConstDataPtr a, bool mipMap )
{
	if( a && (y->typeId() != a->typeId()) )
	{
		throw Exception( "Channel types do not match." );
	}
		
	Constructor c;
	c.alpha = a;
	c.width = width;
	c.height = height;
	c.mipMap = mipMap;
	m_texture = IECore::despatchTypedData<Constructor, IECore::TypeTraits::IsNumericVectorTypedData>( const_pointer_cast<Data>( y ), c );
}


ImagePrimitivePtr LuminanceTexture::imagePrimitive() const
{
	glPushAttrib( mask() );
		
		bind();
	
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
		
		FloatVectorDataPtr ad = 0;
		vector<float> *a = 0;
		if( aSize )
		{
			ad = new FloatVectorData();
			a = &ad->writable(); a->resize( width * height );
		}
		
		unsigned int i = 0;
		for( int yy=height-1; yy>=0; yy-- )
		{
			float *ry = &y[yy*width];
			float *ra = a ? &(*a)[yy*width] : 0;
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
		ImagePrimitivePtr image = new ImagePrimitive( imageExtents, imageExtents );
		image->variables["Y"] = PrimitiveVariable( PrimitiveVariable::Vertex, yd );
		if( a )
		{
			image->variables["A"] = PrimitiveVariable( PrimitiveVariable::Vertex, ad );
		}
	
	glPopAttrib();

	IECoreGL::Exception::throwIfError();
	
	return image;
}

