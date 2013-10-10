//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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
#include <cassert>
#include <stdio.h>
#include <iostream>
#include <fstream>
// already included in libpng #include <setjmp.h>

#include "IECore/PNGImageReader.h"


#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/BoxOps.h"
#include "IECore/ScaledDataConversion.h"

#include "IECore/CompoundParameter.h"

#include "boost/format.hpp"

#include "png.h"


using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

struct PNGImageData
{
	/// PNG Header info
	int 			m_width, m_height;
	int				m_numChannels;
	png_byte 		m_colorType;
	png_byte 		m_bitDepth;
	
	/// PNG Data
	png_structp 	m_pngPtr;
	png_infop 		m_infoPtr;
	int 			m_numberOfPasses;
	png_bytep* 		m_rowPointers;
	
	PNGImageData()
	{
		m_rowPointers = NULL;
		m_pngPtr = NULL;
		m_infoPtr = NULL;
	}
	
	~PNGImageData()
	{
		if (m_rowPointers)
		{
			/* cleanup heap allocation */
			for (int y = 0; y < m_height; y++)
					free(m_rowPointers[y]);
			free(m_rowPointers);
		}
	
		if (m_pngPtr)
		{
			if (m_infoPtr)
				png_destroy_read_struct(&m_pngPtr, &m_infoPtr, (png_infopp)NULL);
			else
				png_destroy_read_struct(&m_pngPtr, (png_infopp)NULL, (png_infopp)NULL);		
		}		
	}
};


IE_CORE_DEFINERUNTIMETYPED( PNGImageReader );

const Reader::ReaderDescription <PNGImageReader> PNGImageReader::m_readerDescription ("png");

PNGImageReader::PNGImageReader() :
		ImageReader( "Reads Portable Network Graphics (PNG) files" )
{
	constructParameters();
	m_pngImageData = NULL;
}

PNGImageReader::PNGImageReader(const string & fileName) :
		ImageReader( "Reads Portable Network Graphics (PNG) files" )
{
	m_fileNameParameter->setTypedValue( fileName );
	constructParameters();
	m_pngImageData = NULL;
}

PNGImageReader::~PNGImageReader()
{
	if (m_pngImageData)
	{
		delete m_pngImageData;
	}
}

void PNGImageReader::constructParameters()
{
	m_convertGreyToRGB = new BoolParameter(
	        "convertGreyToRGB",
	        "Automatically convert greyscale images to RGB.",
	        false);

	parameters()->addParameter( m_convertGreyToRGB );
}


bool PNGImageReader::canRead( const string &fileName )
{
	// attempt to open the file
	ifstream in(fileName.c_str());
	if (!in.is_open())
	{
		return false;
	}

	png_byte header[8];    // 8 is the maximum size that can be checked
	
	// read in the header
	in.seekg(0, ios_base::beg);
	in.read((char*)header, 8);

	// check the header
	if (png_sig_cmp(header, 0, 8))
	{
		return false;
	}

	return true;
}

void PNGImageReader::channelNames(  vector<string> &names )
{
	open( true );
	names.clear();

	if ( (m_bufferFileName.size() < 1) || (!m_pngImageData) )
	{
		// no file loaded, bail out
		return;
	}

	if ( m_pngImageData->m_colorType == PNG_COLOR_TYPE_GRAY )
	{
		names.push_back("Y");
	}
	else if ( m_pngImageData->m_colorType == PNG_COLOR_TYPE_RGB )
	{
		names.push_back("R");
		names.push_back("G");
		names.push_back("B");
	}
	else if ( m_pngImageData->m_colorType == PNG_COLOR_TYPE_RGB_ALPHA )
	{
		names.push_back("R");
		names.push_back("G");
		names.push_back("B");
		names.push_back("A");
	}
	else if ( m_pngImageData->m_colorType == PNG_COLOR_TYPE_GRAY_ALPHA )
	{
		names.push_back("Y");
		names.push_back("A");
	}
	else if ( m_pngImageData->m_colorType == PNG_COLOR_TYPE_PALETTE )
	{
		throw IOException( std::string("PNGImageReader: PNG_COLOR_TYPE_PALETTE not supported") );
	}
	else
	{
		throw IOException( std::string("PNGImageReader: Unknown color type.") );
	}
}

bool PNGImageReader::isComplete()
{
	return open( false );
}

Imath::Box2i PNGImageReader::dataWindow()
{
	open( true );

	assert(m_pngImageData);

	return Box2i( V2i( 0, 0 ), V2i( m_pngImageData->m_width - 1, m_pngImageData->m_height - 1 ) );
}

Imath::Box2i PNGImageReader::displayWindow()
{
	return dataWindow();
}

std::string PNGImageReader::sourceColorSpace() const
{
	return "srgb";
}

template<typename FromType, typename ToType>
DataPtr PNGImageReader::readTypedChannel( const Imath::Box2i &dataWindow, int pixelOffset )
{
	assert(m_pngImageData);
	
	int area = ( dataWindow.size().x + 1 ) * ( dataWindow.size().y + 1 );
	assert( area >= 0 );
	int dataWidth = 1 + dataWindow.size().x;
	int dataY = 0;

	typedef TypedData< std::vector< ToType > > TargetVector;
	
	typename TargetVector::Ptr dataContainer = new TargetVector();
	typename TargetVector::ValueType &data = dataContainer->writable();
	data.resize( area );

	ScaledDataConversion< FromType, ToType > converter;
	
	int pixelSize = m_pngImageData->m_numChannels * (int)sizeof(FromType);
	
	for ( int y = dataWindow.min.y; y <= dataWindow.max.y; ++y, ++dataY )
	{
		png_byte* row = m_pngImageData->m_rowPointers[y];

		typename TargetVector::ValueType::size_type dataOffset = dataY * dataWidth;
		for ( int x = dataWindow.min.x; x <= dataWindow.max.x; ++x, ++dataOffset )
		{
			assert( dataOffset < data.size() );
			
			FromType* ptr = (FromType*) &(row[x * pixelSize]);
			
			data[dataOffset] = converter( reverseBytes(ptr[pixelOffset]) );
		}
	}
	return dataContainer;
}

DataPtr PNGImageReader::readChannel( const std::string &name, const Imath::Box2i &dataWindow, bool raw )
{
	open( true );
	
	assert(m_pngImageData);
	
	vector<string> channelNames;
	this->channelNames( channelNames );
	
	if (std::find(channelNames.begin(), channelNames.end(), name) == channelNames.end())
	{
		throw IOException( ( boost::format( "PNGImageReader: Could not find channel \"%s\" while reading %s" ) % name % m_bufferFileName ).str() );
	}

	// Unsupported PNG types
	if (m_pngImageData->m_colorType & PNG_COLOR_MASK_PALETTE)
	{
		// PALETTE NOT IMPLEMENTED
		throw IOException( ( boost::format( "PNGImageReader: %s is a paletted image. Palette image types are not supported." ) % m_bufferFileName ).str() );
	}
	if (m_pngImageData->m_bitDepth < 8)
	{
		// ONLY SUPPORT 8 AND 16 BIT DEPTHS
		throw IOException( ( boost::format( "PNGImageReader: %s has a bit depth of %d. Only bit depths of 8 and 16 are supported." ) % m_bufferFileName % m_pngImageData->m_bitDepth ).str() );
	}

	// Calculate pixel offset
	int pixelOffset = 0;	
	if ( name == "R" )
	{
		pixelOffset = 0;
	}
	else if ( name == "G" )
	{
		pixelOffset = 1;
	}
	else if ( name == "B" )
	{
		pixelOffset = 2;
	}
	else if ( name == "Y" )
	{
		pixelOffset = 0;
	}
	else if ( name == "A" )
	{
		if (m_pngImageData->m_colorType & PNG_COLOR_MASK_COLOR)
			pixelOffset = 3; // RGBA
		else
			pixelOffset = 1; // YA
	}
	
	DataPtr dataContainer = NULL;
	if ( raw )
	{
		if (m_pngImageData->m_bitDepth == 8)
			dataContainer = readTypedChannel< png_byte, png_byte >( dataWindow, pixelOffset );
		else
			dataContainer = readTypedChannel< png_uint_16, png_uint_16 >( dataWindow, pixelOffset );		
	}
	else
	{
		if (m_pngImageData->m_bitDepth == 8)
			dataContainer = readTypedChannel< png_byte, float >( dataWindow, pixelOffset );
		else
			dataContainer = readTypedChannel< png_uint_16, float >( dataWindow, pixelOffset );		
	}
	return dataContainer;
}


bool PNGImageReader::open( bool throwOnFailure )
{
	if ( fileName() == m_bufferFileName )
	{
		return true;
	}

	m_bufferFileName = fileName();

	if (m_pngImageData)
	{
		delete m_pngImageData;
	}
	m_pngImageData = new PNGImageData();
	
	png_byte header[8];
	FILE *fp = NULL;
	
	try
	{
		/* open file and test for it being a png */
		fp = fopen(m_bufferFileName.c_str(), "rb");
		if (!fp)
		{
			throw IOException( ( boost::format( "PNGImageReader: File %s could not be opened for reading" ) % m_bufferFileName ).str() );
		}
		size_t sizeRead = fread(header, 1, 8, fp);
		if ( sizeRead != 8 || png_sig_cmp(header, 0, 8))
		{
			throw IOException( ( boost::format( "PNGImageReader: File %s is not recognized as a PNG file" ) % m_bufferFileName ).str() );
		}
	
		// Main png struct
		m_pngImageData->m_pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!m_pngImageData->m_pngPtr)
		{
			throw IOException( std::string("PNGImageReader: png_create_read_struct failed.") );
		}
	
		// PNG Info struct
		m_pngImageData->m_infoPtr = png_create_info_struct(m_pngImageData->m_pngPtr);
		if (!m_pngImageData->m_infoPtr)
		{
			png_destroy_read_struct(&m_pngImageData->m_pngPtr, (png_infopp)NULL, (png_infopp)NULL);
			throw IOException( std::string("PNGImageReader: png_create_info_struct failed.") );
		}
	
		// set location to longjmp back in case of errors
		if (setjmp(png_jmpbuf(m_pngImageData->m_pngPtr)))
		{
			png_destroy_read_struct(&m_pngImageData->m_pngPtr, &m_pngImageData->m_infoPtr, (png_infopp)NULL);
			throw IOException( std::string("PNGImageReader: Error during init_io.") );
		}
	
		// pass our file pointer in and initialize IO
		png_init_io(m_pngImageData->m_pngPtr, fp);
		
		// tell libpng we have already read the sig bytes (8 byte magic number)
		png_set_sig_bytes(m_pngImageData->m_pngPtr, 8);
	
		// Read all info up to the actual image data	
		png_read_info(m_pngImageData->m_pngPtr, m_pngImageData->m_infoPtr);
	
		// Get some of the info we just read	
		m_pngImageData->m_colorType = png_get_color_type(m_pngImageData->m_pngPtr, m_pngImageData->m_infoPtr);
		m_pngImageData->m_bitDepth = png_get_bit_depth(m_pngImageData->m_pngPtr, m_pngImageData->m_infoPtr);
	
		// tell libpng to handle interlacing for us
		m_pngImageData->m_numberOfPasses = png_set_interlace_handling(m_pngImageData->m_pngPtr);

		// Convert paletted to RGB		
		if (m_pngImageData->m_colorType == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(m_pngImageData->m_pngPtr);
			
		// Convert greyscale bitdepths 1,2,4 to 8
		if (m_pngImageData->m_colorType == PNG_COLOR_TYPE_GRAY && m_pngImageData->m_bitDepth < 8)
			png_set_expand_gray_1_2_4_to_8(m_pngImageData->m_pngPtr);
			
		// Convert transparency chunk to an alpha channel
		if (png_get_valid(m_pngImageData->m_pngPtr, m_pngImageData->m_infoPtr, PNG_INFO_tRNS))
			png_set_tRNS_to_alpha(m_pngImageData->m_pngPtr);

		if (m_convertGreyToRGB->getTypedValue())
		{
			printf("convert grey to RGB\n");
			if (m_pngImageData->m_colorType == PNG_COLOR_TYPE_GRAY ||
				m_pngImageData->m_colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
				png_set_gray_to_rgb(m_pngImageData->m_pngPtr);	
		}

		// re-read updated info
		png_read_update_info(m_pngImageData->m_pngPtr, m_pngImageData->m_infoPtr);
	
		// re-get updated info
		m_pngImageData->m_width = png_get_image_width(m_pngImageData->m_pngPtr, m_pngImageData->m_infoPtr);
		m_pngImageData->m_height = png_get_image_height(m_pngImageData->m_pngPtr, m_pngImageData->m_infoPtr);
		m_pngImageData->m_colorType = png_get_color_type(m_pngImageData->m_pngPtr, m_pngImageData->m_infoPtr);
		m_pngImageData->m_bitDepth = png_get_bit_depth(m_pngImageData->m_pngPtr, m_pngImageData->m_infoPtr);
		m_pngImageData->m_numChannels = png_get_channels(m_pngImageData->m_pngPtr, m_pngImageData->m_infoPtr);

		// set location to longjmp back in case of errors
		if (setjmp(png_jmpbuf(m_pngImageData->m_pngPtr)))
		{
			png_destroy_read_struct(&m_pngImageData->m_pngPtr, &m_pngImageData->m_infoPtr, (png_infopp)NULL);
			throw IOException( std::string("PNGImageReader: Error during read_image.") );
		}
	
		// Allocate image data
		m_pngImageData->m_rowPointers = (png_bytep*) malloc(sizeof(png_bytep) * m_pngImageData->m_height);
		for (int y = 0; y < m_pngImageData->m_height; y++)
		{
			m_pngImageData->m_rowPointers[y] = (png_byte*) malloc(png_get_rowbytes(m_pngImageData->m_pngPtr, m_pngImageData->m_infoPtr));
		}
	
		// Read the image
		png_read_image(m_pngImageData->m_pngPtr, m_pngImageData->m_rowPointers);
	
		// Finish reading
		png_read_end(m_pngImageData->m_pngPtr, (png_infop)NULL);
	
		// close the file
		fclose(fp);		
	}
	catch (...)
	{
		if ( fp )
		{
			fclose( fp );
		}

		m_bufferFileName.clear();
		if ( throwOnFailure )
		{
			throw;
		}
		else
		{
			return false;
		}
	}
	
	preMultiplyAlphas();

	return true;
}

// preMultiplyAlphas
//
// Note: This is temporary and will be moved into the base ImageReader class, along with virtual
//       functions to determine whether the image produced from the reader has pre-multiplied alphas
//       or not.
//       Ideally, this would be done in floats to avoid data loss, and will be when cortex is revised
//       to handle alpha pre-multiplication consistently.
//   
void PNGImageReader::preMultiplyAlphas()
{
	assert (m_pngImageData);
	assert (m_pngImageData->m_rowPointers);

	// don't bother with lower bit depths
	if (m_pngImageData->m_bitDepth < 8) return;

	// calculate pixel size
	int pixelSize =m_pngImageData-> m_numChannels * m_pngImageData->m_bitDepth / 8;
	
	// only if we have an alpha channel
	if (m_pngImageData->m_colorType & PNG_COLOR_MASK_ALPHA)
	{
		// walk the rows
		for (int y = 0; y < m_pngImageData->m_height; ++y)
		{
			png_byte* row = m_pngImageData->m_rowPointers[y];

			// walk the pixels in the row	
			for (int x = 0; x < m_pngImageData->m_width; ++x)
			{
				// bit depth 8
				if (m_pngImageData->m_bitDepth == 8)
				{
					// get pixel 
					png_byte* ptr = &(row[x * pixelSize]);
					float  a = (float)ptr[m_pngImageData->m_numChannels - 1] / 255.0f;
					
					// pre-multiply by alpha
					for(int i = 0; i < (m_pngImageData->m_numChannels - 1); ++i)
					{
						*ptr = (png_byte)((float)*ptr * a);
						ptr++;
					}
				}
				// bit depth 16
				else
				{
					// get pixel 
					png_uint_16* ptr = (png_uint_16*) &(row[x * pixelSize]);
					float  a = (float)ptr[m_pngImageData->m_numChannels - 1] / 65535.0f;
					
					// pre-multiply by alpha
					for(int i = 0; i < (m_pngImageData->m_numChannels - 1); ++i)
					{
						*ptr = (png_uint_16)((float)*ptr * a);
						ptr++;
					}
				}
			}
		}		
	}
}



