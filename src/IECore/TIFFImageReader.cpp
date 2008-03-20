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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <cassert>
#include <iterator>
#include <sstream>

#include "IECore/TIFFImageReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/ScopedTIFFExceptionTranslator.h"
#include "IECore/BoxOps.h"

#include "boost/static_assert.hpp"
#include "boost/format.hpp"

#include "tiffio.h"



using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

const Reader::ReaderDescription<TIFFImageReader> TIFFImageReader::m_readerDescription("tiff tif");

TIFFImageReader::TIFFImageReader()
		:	ImageReader("TIFFImageReader", "Reads Tagged Image File Format (TIFF) files" ),
		m_tiffImage(0)
{
}

TIFFImageReader::TIFFImageReader( const string &fileName )
		:	ImageReader("TIFFImageReader", "Reads Tagged Image File Format (TIFF) files" ),
		m_tiffImage(0)
{
	m_fileNameParameter->setTypedValue(fileName);
}

TIFFImageReader::~TIFFImageReader()
{
	if ( m_tiffImage )
	{
		TIFFClose( m_tiffImage );
		m_tiffImage = 0;
	}
}

bool TIFFImageReader::canRead( const string &fileName )
{
	// attempt to open the file
	ifstream in( fileName.c_str() );
	if ( !in.is_open() || in.fail() )
	{
		return false;
	}

	// check the magic number of the input file
	in.seekg( 0, ios_base::beg );
	if ( in.fail() )
	{
		return false;
	}
	unsigned int magic;
	in.read((char *) &magic, sizeof(unsigned int));
	if ( in.fail() )
	{
		return false;
	}

	in.seekg( 0, ios_base::beg );
	if ( in.fail() )
	{
		return false;
	}

	return magic == 0x002a4949 || magic == reverseBytes<unsigned int>(0x002a4949) 		
		|| magic == 0x4d4d002a || magic == reverseBytes<unsigned int>(0x4d4d002a) ;
}

void TIFFImageReader::channelNames( vector<string> &names )
{
	names.clear();

	open( true );

	if ( m_photometricInterpretation == PHOTOMETRIC_RGB )
	{
		if ( m_samplesPerPixel >= 3 )
		{
			names.push_back("R");
			names.push_back("G");
			names.push_back("B");

			bool haveAlpha = false;

			if ( m_extraSamples.size() )
			{
				if ( m_extraSamples[0] == EXTRASAMPLE_ASSOCALPHA || m_extraSamples[0] == EXTRASAMPLE_UNASSALPHA )
				{
					names.push_back("A");
					haveAlpha = true;
				}
			}
			else if ( !haveAlpha && m_samplesPerPixel >= 4 )
			{
				names.push_back("A");
			}
		}
	}
	else
	{
		assert( m_photometricInterpretation == PHOTOMETRIC_MINISBLACK );

		/// Interpret first channel as luminance
		names.push_back("Y");
	}

	int unknownChannelIdx = 1;
	while ( (int)names.size() < m_samplesPerPixel )
	{
		names.push_back( ( boost::format( "Data%d" ) % unknownChannelIdx ).str() );

		unknownChannelIdx++;
	}
}

bool TIFFImageReader::isComplete()
{
	if ( !open( false ) )
	{
		return false;
	}

	try
	{
		/// Ideally we'd read the last scanline here, but we're unable to do that in all cases because not all
		/// compression methods support random access to the image data.
		ScopedTIFFExceptionTranslator errorHandler;

		readBuffer();

		return true;
	}
	catch (...)
	{
		return false;
	}
}

Imath::Box2i TIFFImageReader::dataWindow()
{
	open( true );

	return m_dataWindow;
}

Imath::Box2i TIFFImageReader::displayWindow()
{
	open( true );

	return m_displayWindow;
}

template<typename T>
T TIFFImageReader::tiffField( unsigned int t, T def )
{
	BOOST_STATIC_ASSERT( sizeof( unsigned int ) >= sizeof( ttag_t ) );
	assert( m_tiffImage );

	T value;
	if ( TIFFGetField( m_tiffImage, (ttag_t)t, &value ) )
	{
		return value;
	}
	else
	{
		return def;
	}
}

template<typename T>
T TIFFImageReader::tiffFieldDefaulted( unsigned int t )
{
	BOOST_STATIC_ASSERT( sizeof( unsigned int ) >= sizeof( ttag_t ) );
	assert( m_tiffImage );

	T value;
	TIFFGetFieldDefaulted( m_tiffImage, (ttag_t)t, &value );
	return value;
}

template<typename T>
float toFloat( T t )
{
	static const double normalizer = 1.0 / Imath::limits<T>::max();

	return normalizer * t;
}

template<>
float toFloat( float t )
{
	return t;
}

template<>
float toFloat( double t )
{
	return static_cast<float>( t );
}

template<typename T>
DataPtr TIFFImageReader::readTypedChannel( const std::string &name, const Box2i &dataWindow )
{
	FloatVectorDataPtr dataContainer = new FloatVectorData();

	typename FloatVectorData::ValueType &data = dataContainer->writable();

	std::vector<std::string> names;
	channelNames( names );

	std::vector<std::string>::iterator it = find( names.begin(), names.end(), name );
	if ( it == names.end() )
	{
		throw IOException( (boost::format( "TIFFImageReader: Could not find channel \"%s\" while reading %s") % name % fileName() ).str() );
	}

	int channelOffset = std::distance( names.begin(), it );
	assert( channelOffset >= 0 );
	assert( channelOffset < (int)names.size() );

	if ( channelOffset >= m_samplesPerPixel )
	{
		throw IOException( (boost::format( "TIFFImageReader: Insufficient samples-per-pixel (%d) for reading channel \"%s\"") % m_samplesPerPixel % name).str() );
	}

	int area = ( dataWindow.size().x + 1 ) * ( dataWindow.size().y + 1 );
	assert( area >= 0 );
	data.resize( area );

	int dataWidth = 1 + dataWindow.size().x;
	int bufferDataWidth = 1 + m_dataWindow.size().x;

	int dataY = 0;
	for ( int y = dataWindow.min.y - m_dataWindow.min.y ; y <= dataWindow.max.y - m_dataWindow.min.y ; ++y, ++dataY )
	{
		int dataX = 0;

		for ( int x = dataWindow.min.x - m_dataWindow.min.x;  x <= dataWindow.max.x - m_dataWindow.min.x ; ++x, ++dataX  )
		{
			const T* buf = reinterpret_cast< T* >( & m_buffer[0] );
			assert( buf );

			// \todo Currently, we only support PLANARCONFIG_CONTIG for TIFFTAG_PLANARCONFIG.
			/// \todo Use a DataConversion object instead of toFloat<> ?

			FloatVectorData::ValueType::size_type dataOffset = dataY * dataWidth + dataX;
			assert( dataOffset < data.size() );

			data[dataOffset] = toFloat<T>( buf[ m_samplesPerPixel * ( y * bufferDataWidth + x ) + channelOffset ] );
		}
	}

	return dataContainer;
}

DataPtr TIFFImageReader::readChannel( const std::string &name, const Imath::Box2i &dataWindow )
{
	ScopedTIFFExceptionTranslator errorHandler;

	open( true );

	if ( m_buffer.size() == 0 )
	{
		readBuffer();
	}

	if ( m_sampleFormat == SAMPLEFORMAT_IEEEFP )
	{
		return readTypedChannel<float>( name, dataWindow );
	}
	else
	{
		assert( m_sampleFormat == SAMPLEFORMAT_UINT ) ;

		switch ( m_bitsPerSample )
		{
		case 8:
			return readTypedChannel<unsigned char>( name, dataWindow );

		case 16:
			return readTypedChannel<uint16>( name, dataWindow );

		case 32:
			return readTypedChannel<uint32>( name, dataWindow );

		default:
			assert( false );
			return 0;
		}
	}
}

void TIFFImageReader::readBuffer()
{
	/// readChannel should already have opened the image by now
	assert( m_tiffImage );

	/// \todo Support tiled images!
	if ( TIFFIsTiled( m_tiffImage ) )
	{
		throw IOException( "TIFFImageReader: Tiled images unsupported" );
	}

	int width = boxSize( m_dataWindow ).x + 1;
	int height = boxSize( m_dataWindow ).y + 1;

	tsize_t stripSize = TIFFStripSize(m_tiffImage);

	// \todo Currently, we only support PLANARCONFIG_CONTIG for TIFFTAG_PLANARCONFIG.
	std::vector<unsigned char>::size_type bufSize = (size_t)( (float)m_bitsPerSample / 8 * m_samplesPerPixel * width * height );
	assert( bufSize );
	m_buffer.resize( bufSize, 0 );

	// read the image
	tsize_t imageOffset = 0;
	tstrip_t numStrips = TIFFNumberOfStrips( m_tiffImage );
	for ( tstrip_t strip = 0; strip < numStrips; strip++ )
	{
		tsize_t result = TIFFReadEncodedStrip( m_tiffImage, strip, &m_buffer[0] + imageOffset, stripSize);

		if ( result == -1 )
		{
			throw IOException( (boost::format( "TIFFImageReader: Read error on strip number %d") % strip).str() );
		}

		imageOffset += result;
	}
}

bool TIFFImageReader::open( bool throwOnFailure )
{
	ScopedTIFFExceptionTranslator errorHandler;

	if ( m_tiffImage )

	{
		if ( m_tiffImageFileName == fileName() )
		{
			return true;
		}
		else
		{
			TIFFClose( m_tiffImage );
			m_tiffImage = 0;
			m_buffer.clear();
		}
	}

	assert( m_tiffImage == 0 );
	assert( m_buffer.size() == 0 );

	try
	{
		m_tiffImage = TIFFOpen( fileName().c_str(), "r" );

		if (! m_tiffImage )
		{
			throw IOException( ( boost::format("TIFFImageReader: Could not open %s ") % fileName() ).str() );
		}

		int width = tiffField<uint32>( TIFFTAG_IMAGEWIDTH );
		if ( width == 0 )
		{
			throw IOException( ( boost::format("TIFFImageReader: Unsupported value (%d) for TIFFTAG_IMAGEWIDTH while reading %s") % width % fileName() ).str() );
		}

		int height = tiffField<uint32>( TIFFTAG_IMAGELENGTH );
		if ( height == 0 )
		{
			throw IOException( ( boost::format("TIFFImageReader: Unsupported value (%d) for TIFFTAG_IMAGELENGTH while reading %s") % height % fileName() ).str() );
		}

		m_samplesPerPixel = tiffField<uint16>( TIFFTAG_SAMPLESPERPIXEL );
		if ( m_samplesPerPixel == 0 )
		{
			throw IOException( ( boost::format("TIFFImageReader: Unsupported value (%d) for TIFFTAG_SAMPLESPERPIXEL") % m_samplesPerPixel ).str() );
		}

		m_bitsPerSample = tiffField<uint16>( TIFFTAG_BITSPERSAMPLE );
		if (! (	m_bitsPerSample == 8 ||
		                m_bitsPerSample == 16 ||
		                m_bitsPerSample == 32
		      ) )
		{
			throw IOException( ( boost::format("TIFFImageReader: Unsupported value (%d) for TIFFTAG_BITSPERSAMPLE") % m_bitsPerSample ).str() );
		}

		m_photometricInterpretation = tiffFieldDefaulted<uint16>( TIFFTAG_PHOTOMETRIC );
		if (! ( m_photometricInterpretation == PHOTOMETRIC_MINISBLACK ||
		                m_photometricInterpretation == PHOTOMETRIC_RGB
		      ))
		{
			throw IOException( ( boost::format("TIFFImageReader: Unsupported value (%d) for TIFFTAG_PHOTOMETRIC") % m_photometricInterpretation ).str() );
		}

		m_fillOrder = tiffFieldDefaulted<uint16>( TIFFTAG_FILLORDER );
		if (!( m_fillOrder == FILLORDER_MSB2LSB || m_fillOrder == FILLORDER_LSB2MSB) )
		{
			throw IOException( ( boost::format("TIFFImageReader: Invalid value (%d) for TIFFTAG_FILLORDER") % m_fillOrder ).str() );
		}
		assert( (bool)( TIFFIsMSB2LSB( m_tiffImage) ) == (bool)( m_fillOrder == FILLORDER_MSB2LSB ));

		m_sampleFormat = tiffFieldDefaulted<uint16>( TIFFTAG_SAMPLEFORMAT );
		if (! ( m_sampleFormat == SAMPLEFORMAT_UINT ||
		                m_sampleFormat == SAMPLEFORMAT_IEEEFP
		      ))
		{
			throw IOException( ( boost::format("TIFFImageReader: Unsupported value (%d) for TIFFTAG_SAMPLEFORMAT") % m_sampleFormat ).str() );
		}

		m_orientation = tiffFieldDefaulted<uint16>( TIFFTAG_ORIENTATION );
		if ( m_orientation != ORIENTATION_TOPLEFT )
		{
			throw IOException( ( boost::format("TIFFImageReader: Unsupported value (%d) for TIFFTAG_ORIENTATION") % m_orientation ).str() );
		}

		m_planarConfig = tiffFieldDefaulted<uint16>( TIFFTAG_PLANARCONFIG );
		if ( m_planarConfig !=  PLANARCONFIG_CONTIG )
		{
			throw IOException( ( boost::format("TIFFImageReader: Unsupported value (%d) for TIFFTAG_PLANARCONFIG") % m_orientation ).str() );
		}

		uint16 numExtraSamples;
		uint16 *extraSamples;
		TIFFGetFieldDefaulted( m_tiffImage, TIFFTAG_EXTRASAMPLES, &numExtraSamples, &extraSamples);
		for ( unsigned int i = 0; i < numExtraSamples; i++ )
		{
			m_extraSamples.push_back( extraSamples[i] );
		}

		m_dataWindow = Box2i( V2i( 0, 0 ), V2i( width - 1, height - 1 ) );

		float xPosition = tiffField<float>( TIFFTAG_XPOSITION, 0.0f);
		float yPosition = tiffField<float>( TIFFTAG_YPOSITION, 0.0f);
		m_dataWindow.min += V2i( (int)xPosition, (int)yPosition);
		m_dataWindow.max += V2i( (int)xPosition, (int)yPosition );

		uint32 fullWidth = tiffField<uint32>( TIFFTAG_PIXAR_IMAGEFULLWIDTH, width );
		uint32 fullLength = tiffField<uint32>( TIFFTAG_PIXAR_IMAGEFULLLENGTH, height );

		m_displayWindow = Box2i( V2i( 0, 0 ), V2i( fullWidth - 1, fullLength - 1 ) );

		m_tiffImageFileName = fileName();
	}
	catch ( ... )
	{
		if ( throwOnFailure )
		{
			throw;
		}
		else
		{
			return false;
		}
	}

	return m_tiffImage != 0;
}

