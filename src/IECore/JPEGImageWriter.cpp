//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include <setjmp.h>
#include <stdio.h>

#include "IECore/BoxOps.h"
#include "IECore/JPEGImageWriter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/DataConvert.h"
#include "IECore/ScaledDataConversion.h"
#include "IECore/DespatchTypedData.h"

#include "boost/format.hpp"

#include <fstream>

extern "C"
{
#include "jpeglib.h"
}

using namespace IECore;
using namespace std;
using namespace boost;
using namespace Imath;

const Writer::WriterDescription<JPEGImageWriter> JPEGImageWriter::m_writerDescription("jpeg jpg");

JPEGImageWriter::JPEGImageWriter() :
		ImageWriter("JPEGImageWriter", "Serializes images to the Joint Photographic Experts Group (JPEG) format")
{
	constructParameters();
}

JPEGImageWriter::JPEGImageWriter(ObjectPtr image, const string &fileName) :
		ImageWriter("JPEGImageWriter", "Serializes images to the Joint Photographic Experts Group (JPEG) format")
{
	constructParameters();
	m_objectParameter->setValue( image );
	m_fileNameParameter->setTypedValue( fileName );
}

void JPEGImageWriter::constructParameters()
{
	m_qualityParameter = new IntParameter(
	        "quality",
	        "The quality at which to compress the JPEG. 100 yields the largest file size, but best quality image.",
	        100,
	        0,
	        100
	);

	parameters()->addParameter( m_qualityParameter );
}

JPEGImageWriter::~JPEGImageWriter()
{
}

IntParameterPtr JPEGImageWriter::qualityParameter()
{
	return m_qualityParameter;
}

ConstIntParameterPtr JPEGImageWriter::qualityParameter() const
{
	return m_qualityParameter;
}


struct JPEGWriterErrorHandler : public jpeg_error_mgr
{
	jmp_buf m_jmpBuffer;
	char m_errorMessage[JMSG_LENGTH_MAX];

	static void errorExit ( j_common_ptr cinfo )
	{
		assert( cinfo );
		assert( cinfo->err );

		JPEGWriterErrorHandler* errorHandler = static_cast< JPEGWriterErrorHandler* >( cinfo->err );
		( *cinfo->err->format_message )( cinfo, errorHandler->m_errorMessage );
		longjmp( errorHandler->m_jmpBuffer, 1 );
	}
};

struct JPEGImageWriter::ChannelConverter
{
	typedef void ReturnType;

	std::string m_channelName;
	Box2i m_displayWindow;
	Box2i m_dataWindow;
	int m_numChannels;
	int m_channelOffset;
	std::vector<unsigned char> &m_imageBuffer;

	ChannelConverter( const std::string &channelName, const Box2i &displayWindow, const Box2i &dataWindow, int numChannels, int channelOffset, std::vector<unsigned char> &imageBuffer  ) 
	: m_channelName( channelName ), m_displayWindow( displayWindow ), m_dataWindow( dataWindow ), m_numChannels( numChannels ), m_channelOffset( channelOffset ), m_imageBuffer( imageBuffer )
	{
	}

	template<typename T>
	ReturnType operator()( typename T::Ptr dataContainer )
	{
		assert( dataContainer );

		const typename T::ValueType &data = dataContainer->readable();
		ScaledDataConversion<typename T::ValueType::value_type, unsigned char> converter;

		int displayWidth = m_displayWindow.size().x + 1;
		int dataWidth = m_dataWindow.size().x + 1;

		int dataY = 0;

		for ( int y = m_dataWindow.min.y; y <= m_dataWindow.max.y; y++, dataY++ )
		{
			int dataOffset = dataY * dataWidth;
			assert( dataOffset >= 0 );

			for ( int x = m_dataWindow.min.x; x <= m_dataWindow.max.x; x++, dataOffset++ )
			{		
				int pixelIdx = ( y - m_displayWindow.min.y ) * displayWidth + ( x - m_displayWindow.min.x );

				assert( pixelIdx >= 0 );
				assert( m_numChannels*pixelIdx + m_channelOffset < (int)m_imageBuffer.size() );
				assert( dataOffset < (int)data.size() );

				// convert to unsigned 8-bit integer				
				m_imageBuffer[ m_numChannels*pixelIdx + m_channelOffset ] = converter( data[dataOffset] );
			}
		}
	};
	
	struct ErrorHandler
	{
		template<typename T, typename F>
		void operator()( typename T::ConstPtr data, const F& functor )
		{
			throw InvalidArgumentException( ( boost::format( "JPEGImageWriter: Invalid data type \"%s\" for channel \"%s\"." ) % Object::typeNameFromTypeId( data->typeId() ) % functor.m_channelName ).str() );		
		}
	};
};

void JPEGImageWriter::writeImage( const vector<string> &names, ConstImagePrimitivePtr image, const Box2i &dataWindow ) const
{
	vector<string>::const_iterator rIt = std::find( names.begin(), names.end(), "R" );
	vector<string>::const_iterator gIt = std::find( names.begin(), names.end(), "G" );	
	vector<string>::const_iterator bIt = std::find( names.begin(), names.end(), "B" );
	vector<string>::const_iterator yIt = std::find( names.begin(), names.end(), "Y" );		
	
	int numChannels = 3;
	J_COLOR_SPACE colorSpace = JCS_RGB;
	if ( rIt != names.end() && gIt != names.end() && bIt != names.end() )
	{
		if ( yIt != names.end() )
		{
			throw IOException("JPEGImageWriter: Unsupported channel names specified");
		}
	} 
	else if ( yIt != names.end() ) 
	{
		if ( rIt != names.end() || gIt != names.end() || bIt != names.end() )
		{
			throw IOException("JPEGImageWriter: Unsupported channel names specified");
		}
		
		colorSpace = JCS_GRAYSCALE;
		numChannels = 1;
	}
	
	assert( numChannels == 1 || numChannels == 3 );

	FILE *outFile = 0;
	if ((outFile = fopen(fileName().c_str(), "wb")) == NULL)
	{
		throw IOException("Could not open '" + fileName() + "' for writing.");
	}
	assert( outFile );

	int displayWidth  = 1 + image->getDisplayWindow().size().x;
	int displayHeight = 1 + image->getDisplayWindow().size().y;

	// compression info
	struct jpeg_compress_struct cinfo;

	try
	{
		JPEGWriterErrorHandler errorHandler;

		/// Setup error handler
		cinfo.err = jpeg_std_error( &errorHandler );

		/// Override fatal error and warning handlers
		errorHandler.error_exit = JPEGWriterErrorHandler::errorExit;
		errorHandler.output_message = JPEGWriterErrorHandler::errorExit;

		/// If we reach here then libjpeg has called our error handler, in which we've saved a copy of the
		/// error such that we might throw it as an exception.
		if ( setjmp( errorHandler.m_jmpBuffer ) )
		{
			throw IOException( std::string( "JPEGImageWriter: " ) + errorHandler.m_errorMessage );
		}

		jpeg_create_compress( &cinfo );

		jpeg_stdio_dest(&cinfo, outFile);

		cinfo.image_width = displayWidth;
		cinfo.image_height = displayHeight;
		cinfo.input_components = numChannels;
		cinfo.in_color_space = colorSpace;

		jpeg_set_defaults( &cinfo );

		int quality = qualityParameter()->getNumericValue();
		assert( quality >= 0 );
		assert( quality <= 100 );		

		// force baseline-JPEG (8bit) values with TRUE
		jpeg_set_quality(&cinfo, quality, TRUE);

		// build the buffer
		std::vector<unsigned char> imageBuffer( displayWidth*displayHeight*numChannels, 0 );

		// add the channels into the header with the appropriate types
		// channel data is RGB interlaced
		for ( vector<string>::const_iterator it = names.begin(); it != names.end(); it++ )
		{
			const string &name = *it;

			if (!( name == "R" || name == "G" || name == "B" || name == "Y" ))
			{
				msg( Msg::Warning, "JPEGImageWriter", format( "Channel \"%s\" was not encoded." ) % name );
				continue;
			}

			int channelOffset = 0;
			if ( name == "R" )
			{
				assert( colorSpace == JCS_RGB );
				channelOffset = 0;
			}
			else if ( name == "G" )
			{
				assert( colorSpace == JCS_RGB );			
				channelOffset = 1;
			}
			else if ( name == "B" )
			{
				assert( colorSpace == JCS_RGB );			
				channelOffset = 2;
			}
			else
			{
				assert( name == "Y" );
				assert( colorSpace == JCS_GRAYSCALE );
				channelOffset = 0;
			}

			// get the image channel
			assert( image->variables.find( name ) != image->variables.end() );			
			DataPtr dataContainer = image->variables.find( name )->second.data;
			assert( dataContainer );
			
			ChannelConverter converter( *it, image->getDisplayWindow(), dataWindow, numChannels, channelOffset, imageBuffer );
		
			despatchTypedData<			
				ChannelConverter, 
				TypeTraits::IsNumericVectorTypedData,
				ChannelConverter::ErrorHandler
			>( dataContainer, converter );	

		}

		// start the compressor
		jpeg_start_compress(&cinfo, TRUE);

		// pass one scanline at a time
		int rowStride = displayWidth * numChannels;
		while (cinfo.next_scanline < cinfo.image_height)
		{
			unsigned char *rowPointer[1] = { &imageBuffer[cinfo.next_scanline * rowStride] };
			jpeg_write_scanlines(&cinfo, rowPointer, 1);
		}

		jpeg_finish_compress( &cinfo );
		jpeg_destroy_compress( &cinfo );
		fclose( outFile );

	}
	catch ( Exception &e )
	{
		jpeg_destroy_compress( &cinfo );
		fclose( outFile );

		throw;
	}
	catch ( std::exception &e )
	{
		throw IOException( ( boost::format( "JPEGImageReader : %s" ) % e.what() ).str() );
	}
	catch ( ... )
	{
		jpeg_destroy_compress( &cinfo );
		fclose( outFile );

		throw IOException( "JPEGImageReader: Unexpected error" );
	}
}
