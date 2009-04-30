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

#include <algorithm>
#include <cassert>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <setjmp.h>

#include "IECore/JPEGImageReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/BoxOps.h"

#include "boost/format.hpp"

extern "C"
{
#include "jpeglib.h"
}

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( JPEGImageReader );

const Reader::ReaderDescription <JPEGImageReader> JPEGImageReader::m_readerDescription ("jpeg jpg");

JPEGImageReader::JPEGImageReader() :
		ImageReader( "JPEGImageReader", "Reads Joint Photographic Experts Group (JPEG) files" )
{
}

JPEGImageReader::JPEGImageReader(const string & fileName) :
		ImageReader( "JPEGImageReader", "Reads Joint Photographic Experts Group (JPEG) files" )
{
	m_fileNameParameter->setTypedValue( fileName );
}

JPEGImageReader::~JPEGImageReader()
{
}

bool JPEGImageReader::canRead( const string &fileName )
{
	// attempt to open the file
	ifstream in(fileName.c_str());
	if (!in.is_open())
	{
		return false;
	}

	// check the magic number of the input file
	// a jpeg should have 0xffd8ffe0 from offset 0
	unsigned int magic;
	in.seekg(0, ios_base::beg);
	in.read((char *) &magic, sizeof(unsigned int));

	return magic == 0xe0ffd8ff || magic == 0xffd8ffe0 || magic == 0xe1ffd8ff || magic == 0xffd8ffe1 ;
}

void JPEGImageReader::channelNames( vector<string> &names )
{
	open( true );
	names.clear();

	if ( m_numChannels == 3 )
	{
		names.push_back("R");
		names.push_back("G");
		names.push_back("B");
	}
	else
	{
		assert ( m_numChannels == 1 );
		names.push_back("Y");
	}
}

bool JPEGImageReader::isComplete()
{
	return open( false );
}

Imath::Box2i JPEGImageReader::dataWindow()
{
	open( true );

	return Box2i( V2i( 0, 0 ), V2i( m_bufferWidth - 1, m_bufferHeight - 1 ) );
}

Imath::Box2i JPEGImageReader::displayWindow()
{
	return dataWindow();
}

std::string JPEGImageReader::defaultColorSpace() const
{
	return "srgb";
}

DataPtr JPEGImageReader::readChannel( const std::string &name, const Imath::Box2i &dataWindow )
{
	open( true );

	int channelOffset = 0;
	if ( name == "R" )
	{
		channelOffset = 0;
	}
	else if ( name == "G" )
	{
		channelOffset = 1;
	}
	else if ( name == "B" )
	{
		channelOffset = 2;
	}
	else if ( name == "Y" )
	{
		channelOffset = 0;
	}
	else
	{
		throw IOException( ( boost::format( "JPEGImageReader: Could not find channel \"%s\" while reading %s" ) % name % m_bufferFileName ).str() );
	}

	assert( channelOffset < m_numChannels );

	HalfVectorDataPtr dataContainer = new HalfVectorData();
	HalfVectorData::ValueType &data = dataContainer->writable();
	int area = ( dataWindow.size().x + 1 ) * ( dataWindow.size().y + 1 );
	assert( area >= 0 );
	data.resize( area );

	int dataWidth = 1 + dataWindow.size().x;

	int dataY = 0;

	for ( int y = dataWindow.min.y; y <= dataWindow.max.y; ++y, ++dataY )
	{
		HalfVectorData::ValueType::size_type dataOffset = dataY * dataWidth;

		for ( int x = dataWindow.min.x; x <= dataWindow.max.x; ++x, ++dataOffset )
		{
			assert( dataOffset < data.size() );

			data[dataOffset] = m_buffer[ m_numChannels * ( y * m_bufferWidth + x ) + channelOffset ] / 255.0f;
		}
	}

	return dataContainer;
}

struct JPEGReaderErrorHandler : public jpeg_error_mgr
{
	jmp_buf m_jmpBuffer;
	char m_errorMessage[JMSG_LENGTH_MAX];

	static void errorExit ( j_common_ptr cinfo )
	{
		assert( cinfo );
		assert( cinfo->err );

		JPEGReaderErrorHandler* errorHandler = static_cast< JPEGReaderErrorHandler* >( cinfo->err );
		( *cinfo->err->format_message )( cinfo, errorHandler->m_errorMessage );
		longjmp( errorHandler->m_jmpBuffer, 1 );
	}
};

bool JPEGImageReader::open( bool throwOnFailure )
{
	if ( fileName() == m_bufferFileName )
	{
		return true;
	}

	m_bufferFileName = fileName();
	m_buffer.clear();

	FILE *inFile = 0;
	try
	{
		// open the file
		inFile = fopen( m_bufferFileName.c_str(), "rb" );
		if ( !inFile )
		{
			throw IOException( ( boost::format( "JPEGImageReader: Could not open file %s" ) % m_bufferFileName ).str() );
		}

		struct jpeg_decompress_struct cinfo;

		try
		{
			JPEGReaderErrorHandler errorHandler;

			/// Setup error handler
			cinfo.err = jpeg_std_error( &errorHandler );

			/// Override fatal error and warning handlers
			errorHandler.error_exit = JPEGReaderErrorHandler::errorExit;
			errorHandler.output_message = JPEGReaderErrorHandler::errorExit;

			/// If we reach here then libjpeg has called our error handler, in which we've saved a copy of the
			/// error such that we might throw it as an exception.
			if ( setjmp( errorHandler.m_jmpBuffer ) )
			{
				throw IOException( std::string( "JPEGImageReader: " ) + errorHandler.m_errorMessage );
			}

			/// Initialize decompressor to read from "inFile"
			jpeg_create_decompress( &cinfo );
			jpeg_stdio_src( &cinfo, inFile );
			jpeg_read_header( &cinfo, TRUE );

			/// Start decompression
			jpeg_start_decompress( &cinfo );

			m_numChannels = cinfo.output_components;

			if ( m_numChannels != 1 && m_numChannels != 3 )
			{
				throw IOException( ( boost::format( "JPEGImageReader: Unsupported number of channels (%d) while opening file %s" ) % m_numChannels % m_bufferFileName ).str() );
			}

			if ( cinfo.out_color_space == JCS_GRAYSCALE )
			{
				assert( m_numChannels == 1 );
			}
			else if ( cinfo.out_color_space != JCS_RGB )
			{
				throw IOException( ( boost::format( "JPEGImageReader: Unsupported colorspace (%d) while opening file %s" ) % cinfo.out_color_space % m_bufferFileName ).str() );
			}			
			/// \todo Add support for JCS_YCbCr encoding, and make sure that we add a parameter that
			/// species whether we should rescale values 0-255 to 16-235 (Y) and 16-239 (Cb/Cr) - default 
			/// should be true. See http://www.fourcc.org/fccyvrgb.php for further info.
			
			/// Create buffer
			int rowStride = cinfo.output_width * cinfo.output_components;
			m_buffer.resize( rowStride * cinfo.output_height, 0 );
			;
			m_bufferWidth = cinfo.output_width;
			m_bufferHeight = cinfo.output_height;

			/// Read scanlines one at a time.
			while (cinfo.output_scanline < cinfo.output_height)
			{
				unsigned char *rowPointer[1] = { &m_buffer[0] + rowStride * cinfo.output_scanline };
				jpeg_read_scanlines( &cinfo, rowPointer, 1 );
			}

			/// Finish decompression
			jpeg_finish_decompress( &cinfo );
			jpeg_destroy_decompress( &cinfo );
		}
		catch ( Exception &e )
		{
			jpeg_destroy_decompress( &cinfo );

			throw;
		}
		catch ( std::exception &e )
		{
			jpeg_destroy_decompress( &cinfo );

			throw IOException( ( boost::format( "JPEGImageReader : %s" ) % e.what() ).str() );
		}
		catch ( ... )
		{
			jpeg_destroy_decompress( &cinfo );

			throw IOException( "JPEGImageReader: Unexpected error" );
		}

		fclose( inFile );
	}
	catch (...)
	{
		if ( inFile )
		{
			fclose( inFile );
		}

		if ( throwOnFailure )
		{
			throw;
		}
		else
		{
			return false;
		}
	}

	return true;
}
