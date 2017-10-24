//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#include <algorithm>

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#include "IECore/BoxOps.h"

#include "IECoreImage/Font.h"
#include "IECoreImage/ImagePrimitive.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreImage;

IE_CORE_DEFINERUNTIMETYPED( Font );

////////////////////////////////////////////////////////////////////////////////
// Font::Implementation
////////////////////////////////////////////////////////////////////////////////

class Font::Implementation : public IECore::RefCounted
{

	public :

		Implementation( const std::string &fontFile )
			:	m_fileName( fontFile ), m_kerning( 1.0f ), m_pixelsPerEm( 0 )
		{
			FT_Error e = FT_New_Face( library(), fontFile.c_str(), 0, &m_face );
			if( e )
			{
				throw Exception( "Error creating new FreeType face." );
			}
			setResolution( 100 );
			m_images.resize( 128 );
		}

		~Implementation() override
		{
			FT_Done_Face( m_face );
		}

		const std::string &fileName() const
		{
			return m_fileName;
		}

		void setKerning( float kerning )
		{
			m_kerning = kerning;
		}

		float getKerning() const
		{
			return m_kerning;
		}

		void setResolution( float pixelsPerEm )
		{
			if( pixelsPerEm!=m_pixelsPerEm )
			{
				m_pixelsPerEm = pixelsPerEm;
				for( size_t i = 0, e = m_images.size(); i < e; i++ )
				{
					m_images[i] = nullptr;
				}
				FT_Set_Pixel_Sizes( m_face, (FT_UInt)pixelsPerEm, (FT_UInt)pixelsPerEm );
			}
		}

		float getResolution() const
		{
			return m_pixelsPerEm;
		}

		const ImagePrimitive *image( char c ) const
		{
			return cachedImage( c );
		}

		ImagePrimitivePtr image() const
		{
			Box2i charDisplayWindow = boundingWindow();
			int charWidth = charDisplayWindow.size().x + 1;
			int charHeight = charDisplayWindow.size().y + 1;

			int width = charWidth * 16;
			int height = charHeight * 8;
			Box2i window( V2i( 0 ), V2i( width-1, height-1 ) );

			ImagePrimitivePtr result = new ImagePrimitive( window, window );
			FloatVectorDataPtr luminanceData = result->createChannel<float>( "Y" );
			float *luminance = &*(luminanceData->writable().begin());

			for( unsigned c=0; c<128; c++ )
			{
				ConstImagePrimitivePtr charImage = image( (char)c );
				ConstFloatVectorDataPtr charLuminanceData = charImage->getChannel<float>( "Y" );
				assert( charLuminanceData );
				const float *charLuminance = &*(charLuminanceData->readable().begin());
				const Box2i &charDataWindow = charImage->getDataWindow();
				assert( charDisplayWindow == charImage->getDisplayWindow() );

				V2i dataOffset = charDataWindow.min - charDisplayWindow.min;

				int cx = c % 16;
				int cy = c / 16;
				float *outBase= luminance + (cy * charHeight + dataOffset.y ) * width + cx * charWidth + dataOffset.x;
				for( int y=charDataWindow.min.y; y<=charDataWindow.max.y; y++ )
				{
					float *rowOut = outBase + (y-charDataWindow.min.y) * width;
					for( int x=charDataWindow.min.x; x<=charDataWindow.max.x; x++ )
					{
						*rowOut++ = *charLuminance++;
					}
				}
			}
			return result;
		}

	private :

		std::string m_fileName;

		FT_Face m_face;
		float m_kerning;
		float m_pixelsPerEm;

		mutable std::vector<ConstImagePrimitivePtr> m_images;

		const ImagePrimitive *cachedImage( char c ) const
		{

			// see if we have it cached
			if( m_images[c] )
			{
				return m_images[c].get();
			}

			// not in cache, so load it
			FT_Load_Char( m_face, c, FT_LOAD_RENDER );

			// convert to ImagePrimitive
			FT_Bitmap &bitmap = m_face->glyph->bitmap;
			assert( bitmap.num_grays==256 );
			assert( bitmap.pixel_mode==FT_PIXEL_MODE_GRAY );

			Box2i displayWindow = boundingWindow();

			// datawindow is the bitmap bound, but adjusted to account for the y transformation described
			// in boundingWindow.
			Box2i dataWindow(
				V2i( m_face->glyph->bitmap_left, -m_face->glyph->bitmap_top ),
				V2i( m_face->glyph->bitmap_left + bitmap.width - 1, -m_face->glyph->bitmap_top + bitmap.rows - 1 )
			);

			ImagePrimitivePtr image = new ImagePrimitive( dataWindow, displayWindow );

			FloatVectorDataPtr luminanceData = image->createChannel<float>( "Y" );
			float *luminance = &*(luminanceData->writable().begin());
			for( int y=0; y<(int)bitmap.rows; y++ )
			{
				unsigned char *row = bitmap.buffer + y * bitmap.pitch;
				/// \todo Do we have to reverse gamma correction to get a linear image?
				for( int x=0; x<(int)bitmap.width; x++ )
				{
					*luminance++ = *row++ / 255.0f;
				}
			}

			// put it in the cache
			m_images[c] = image;

			// return it
			return image.get();
		}

		Imath::Box2i boundingWindow() const
		{
			// display window is the maximum possible character bound. unfortunately the ImagePrimitive
			// defines it's windows with y increasing from top to bottom, whereas the freetype coordinate
			// systems have y increasing in the bottom to top direction. there's not an ideal way of mapping
			// between these two - what we choose to do here is map the 0 of our displayWindow to the baseline of the
			// freetype coordinate system.
			float scale = (float)m_face->size->metrics.x_ppem / (float)m_face->units_per_EM;
			return Box2i(
				V2i( (int)roundf( m_face->bbox.xMin * scale ), (int)roundf( -m_face->bbox.yMax * scale ) ),
				V2i( (int)roundf( m_face->bbox.xMax * scale ) - 1, (int)roundf( -m_face->bbox.yMin * scale ) - 1)
			);
		}

		static FT_Library library()
		{
			static FT_Library l;
			static bool init = false;
			if( !init )
			{
				FT_Error e = FT_Init_FreeType( &l );
				if( e )
				{
					throw Exception( "Error initialising FreeType library." );
				}
				init = true;
			}
			return l;
		}

};

//////////////////////////////////////////////////////////////////////////////////////////
// Font
//////////////////////////////////////////////////////////////////////////////////////////

Font::Font( const std::string &fontFile )
	:	m_implementation( new Implementation( fontFile ) )
{
}

Font::~Font()
{
}

const std::string &Font::fileName() const
{
	return m_implementation->fileName();
}

void Font::setKerning( float kerning )
{
	m_implementation->setKerning( kerning );
}

float Font::getKerning() const
{
	return m_implementation->getKerning();
}

void Font::setResolution( float pixelsPerEm )
{
	m_implementation->setResolution( pixelsPerEm );
}

float Font::getResolution() const
{
	return m_implementation->getResolution();
}

const ImagePrimitive *Font::image( char c ) const
{
	return m_implementation->image( c );
}

ImagePrimitivePtr Font::image() const
{
	return m_implementation->image();
}

