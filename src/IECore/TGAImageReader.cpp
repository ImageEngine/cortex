//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include <fstream>
#include <iostream>
#include <cassert>

#include "boost/filesystem/convenience.hpp"
#include "boost/format.hpp"

#include "IECore/TGAImageReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/BoxOps.h"

#include "IECore/ScaledDataConversion.h"

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

struct TGAImageReader::Header
{
	char idLength;
	char colorMapType;
	char imageType;

	/// Color Map Specification
	uint16_t firstEntryIndex;
	uint16_t colorMapLength;
	char colorMapEntrySize;

	/// Image Specification
	uint16_t xOrigin;
	uint16_t yOrigin;
	uint16_t imageWidth;
	uint16_t imageHeight;
	char pixelDepth;
	char imageDescriptor;
};

IE_CORE_DEFINERUNTIMETYPED( TGAImageReader );
const Reader::ReaderDescription<TGAImageReader> TGAImageReader::m_readerDescription( "tga" );

TGAImageReader::TGAImageReader() :
		ImageReader( "TGAImageReader", "Reads version 1 Truevision Targa files." ),
		m_header()
{
}

TGAImageReader::TGAImageReader( const string &fileName ) :
		ImageReader( "TGAImageReader", "Reads version 1 Truevision Targa files." ),
		m_header()
{
	m_fileNameParameter->setTypedValue( fileName );
}

TGAImageReader::~TGAImageReader()
{
}

bool TGAImageReader::canRead( const string &fileName )
{
	// Attempt to open the file
	ifstream in( fileName.c_str() );
	if ( !in.is_open() || in.fail() )
	{
		return false;
	}
	
	/// No magic number for v1 files, so just check the extension
	return boost::filesystem::extension( fileName ) == ".tga";
}

void TGAImageReader::channelNames( vector<string> &names )
{
	names.clear();

	open( true );

	assert( m_header );

	names.push_back( "B" );
	names.push_back( "G" );
	names.push_back( "R" );

	int alphaChannelBits = m_header->imageDescriptor & 0xf;

	if ( m_header->pixelDepth == 32 && alphaChannelBits )
	{
		names.push_back( "A" );
	}

}

bool TGAImageReader::isComplete()
{
	return open( false );
}

Box2i TGAImageReader::dataWindow()
{
	open( true );
	assert( m_header );

	return m_dataWindow;
}

Box2i TGAImageReader::displayWindow()
{
	open( true );

	return dataWindow();
}

std::string TGAImageReader::sourceColorSpace() const
{
	return "srgb";
}

DataPtr TGAImageReader::readChannel( const string &name, const Imath::Box2i &dataWindow )
{
	if ( !open() )
	{
		return 0;
	}

	readBuffer();

	assert( m_header );

	FloatVectorDataPtr dataContainer = new FloatVectorData();

	FloatVectorData::ValueType &data = dataContainer->writable();

	std::vector<std::string> names;
	channelNames( names );

	std::vector<std::string>::iterator it = find( names.begin(), names.end(), name );
	if ( it == names.end() )
	{
		throw IOException(( boost::format( "TGAImageReader: Could not find channel \"%s\" while reading %s" ) % name % fileName() ).str() );
	}

	int channelOffset = std::distance( names.begin(), it );
	assert( channelOffset >= 0 );
	assert( channelOffset < ( int )names.size() );

	int area = ( dataWindow.size().x + 1 ) * ( dataWindow.size().y + 1 );
	assert( area >= 0 );
	data.resize( area );

	int dataWidth = 1 + dataWindow.size().x;
	int bufferDataWidth = 1 + m_dataWindow.size().x;

	ScaledDataConversion<uint8_t, float> converter;

	const int samplesPerPixel = m_header->pixelDepth == 24 ? 3 : 4 ;

	int dataY = 0;
	int dataYinc = 1;

	if ( !(m_header->imageDescriptor & 1<<5) )
	{
		// bottom-up order.
		dataYinc = -1;
		dataY = dataWindow.size().y;
	}

	int dataXstart = 0;
	int dataXinc = 1;
	if ( m_header->imageDescriptor & 1<<4 )
	{
		// right-left order.
		dataXstart = dataWindow.size().x;
		dataXinc = -1;
	}

	for ( int y = dataWindow.min.y - m_dataWindow.min.y ; y <= dataWindow.max.y - m_dataWindow.min.y ; ++y, dataY += dataYinc )
	{
		int dataX = dataXstart;

		for ( int x = dataWindow.min.x - m_dataWindow.min.x;  x <= dataWindow.max.x - m_dataWindow.min.x ; ++x, dataX += dataXinc )
		{
			const uint8_t* buf = reinterpret_cast< uint8_t* >( & m_buffer[0] );
			assert( buf );

			FloatVectorData::ValueType::size_type dataOffset = dataY * dataWidth + dataX;
			assert( dataOffset < data.size() );

			data[dataOffset] = converter( buf[ samplesPerPixel * ( y * bufferDataWidth + x ) + channelOffset ] );
		}
	}

	return dataContainer;
}

template<typename T>
static void readLittleEndian( std::istream &f, T &n )
{
	f.read(( char* ) &n, sizeof( T ) );

	if ( bigEndian() )
	{
		n = reverseBytes<>( n );
	}
	else
	{
		/// Already little endian
	}
}

void TGAImageReader::readBuffer()
{
	open( true );

	if ( fileName() == m_bufferFileName )
	{
		assert( m_header );

		return;
	}

	m_buffer.clear();

	uint32_t pixelCount = m_header->imageWidth * m_header->imageHeight;
	uint16_t bytesPerPixel = ( int )(( float )m_header->pixelDepth / 8.0 + 0.5 );
	uint32_t bufferSize = pixelCount * bytesPerPixel;
	m_buffer.resize( bufferSize, 0 );

	ifstream in( fileName().c_str() );
	in.seekg( 18, ios_base::beg );

	if ( m_header->imageType == 2 )
	{
		// uncompressed format
		in.read( reinterpret_cast<char*>( &m_buffer[0] ), bufferSize );
		if ( in.fail() )
		{
			throw IOException( "TGAImageReader: Error reading " + fileName() );
		}
	}
	else
	{
		// RLE compression
		char repetitionByte;
		char rleValue[4];
		int counter;
		
		std::vector<char>::iterator bufIt = m_buffer.begin();

		while( bufIt != m_buffer.end() && !in.fail() )
		{
			in.read( &repetitionByte, 1 );
			if ( !in.fail() )
			{
				counter = ( ((unsigned char)repetitionByte) & ((1<<7)-1) ) + 1;
				if ( repetitionByte & (1<<7) )
				{
					// RLE encoded
					in.read( rleValue, bytesPerPixel );
					for ( int c = 0; c < counter; c++ )
					{
						std::copy( rleValue, rleValue + bytesPerPixel, bufIt );
						bufIt += bytesPerPixel;
					}
				}
				else
				{
					// RAW
					in.read( &(*bufIt), bytesPerPixel * counter );
					bufIt += bytesPerPixel * counter;
				}
			}
		}
		if ( in.fail() )
		{
			// so we can read incomplete files
			msg( Msg::Warning, "TGAImageReader::readChannel", "Incomplete file" );
		}
	}
	m_bufferFileName = fileName();
}

bool TGAImageReader::open( bool throwOnFailure )
{
	if ( fileName() == m_headerFileName )
	{
		assert( m_header );

		return true;
	}

	try
	{
		m_header = boost::shared_ptr<Header>( new Header() );

		ifstream in( fileName().c_str() );

		if ( !in.is_open() || in.fail() )
		{
			throw IOException( "TGAImageReader: Could not open " + fileName() );
		}

		/// Can't read header in one go due to alignment issues
		readLittleEndian( in, m_header->idLength );
		readLittleEndian( in, m_header->colorMapType );
		readLittleEndian( in, m_header->imageType );
		readLittleEndian( in, m_header->firstEntryIndex );
		readLittleEndian( in, m_header->colorMapLength );
		readLittleEndian( in, m_header->colorMapEntrySize );
		readLittleEndian( in, m_header->xOrigin );
		readLittleEndian( in, m_header->yOrigin );
		readLittleEndian( in, m_header->imageWidth );
		readLittleEndian( in, m_header->imageHeight );
		readLittleEndian( in, m_header->pixelDepth );
		readLittleEndian( in, m_header->imageDescriptor );
		if ( in.fail() )
		{
			throw IOException( "TGAImageReader: Error reading " + fileName() );
		}

		if ( m_header->colorMapType != 0 )
		{
			throw IOException(( boost::format( "TGAImageReader: Unsupported color map type (%d) in file %s" ) % ( int )m_header->colorMapType % fileName() ).str() );
		}

		if ( m_header->imageType != 2 && m_header->imageType != 10 )
		{
			throw IOException(( boost::format( "TGAImageReader: Unsupported image type (%d) in file %s" ) % ( int )m_header->imageType % fileName() ).str() );
		}

		if ( m_header->pixelDepth != 24 &&  m_header->pixelDepth != 32 )
		{
			throw IOException(( boost::format( "TGAImageReader: Unsupported pixel depth (%d) in file %s" ) % ( int )m_header->pixelDepth % fileName() ).str() );
		}

		int alphaChannelBits = m_header->imageDescriptor & 0xf;

		if ( m_header->pixelDepth == 32 && alphaChannelBits && alphaChannelBits != 8 )
		{
			throw IOException(( boost::format( "TGAImageReader: Unsupported alpha channel bits (%d) for pixel depth %d in file %s" ) % alphaChannelBits % ( int )m_header->pixelDepth % fileName() ).str() );
		}

		if ( m_header->pixelDepth == 24 && alphaChannelBits != 0 )
		{
			throw IOException(( boost::format( "TGAImageReader: Unsupported alpha channel bits (%d) for pixel depth %d in file %s" ) % alphaChannelBits % ( int )m_header->pixelDepth % fileName() ).str() );
		}

		/// Image ID (skip)
		in.seekg( m_header->idLength, ios_base::cur );
		if ( in.fail() )
		{
			throw IOException( "TGAImageReader: Error reading " + fileName() );
		}

		const V2i origin( m_header->xOrigin, m_header->yOrigin );
		const V2i size( m_header->imageWidth, m_header->imageHeight );

		m_dataWindow = Box2i( origin, origin + size - V2i( 1, 1 ) );

		m_headerFileName = fileName();
	}
	catch ( ... )
	{
		m_header.reset();

		if ( throwOnFailure )
		{
			throw;
		}
		else
		{
			return false;
		}
	}

	assert( m_header );
	return true;
}

