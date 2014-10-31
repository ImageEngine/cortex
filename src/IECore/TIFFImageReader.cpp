//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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
#include "IECore/private/ScopedTIFFErrorHandler.h"
#include "IECore/BoxOps.h"
#include "IECore/ScaledDataConversion.h"

#include "boost/static_assert.hpp"
#include "boost/format.hpp"
#include "boost/algorithm/string/predicate.hpp"

#include "tiffio.h"

using namespace IECore;
using namespace IECore::Detail;
using namespace boost;
using namespace Imath;
using namespace std;

IE_CORE_DEFINERUNTIMETYPED( TIFFImageReader );

const Reader::ReaderDescription<TIFFImageReader> TIFFImageReader::m_readerDescription("tiff tif tdl");

TIFFImageReader::TIFFImageReader()
		:	ImageReader( "Reads Tagged Image File Format (TIFF) files" ),
		m_tiffImage( 0 ), m_currentDirectoryIndex( 0 ), m_numDirectories( 1 ), m_haveDirectory( false )
{
}

TIFFImageReader::TIFFImageReader( const string &fileName )
		:	ImageReader( "Reads Tagged Image File Format (TIFF) files" ),
		m_tiffImage( 0 ), m_currentDirectoryIndex( 0 ), m_numDirectories( 1 ), m_haveDirectory( false )
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

	readCurrentDirectory( true );

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
		assert( m_photometricInterpretation == PHOTOMETRIC_MINISBLACK || m_photometricInterpretation == PHOTOMETRIC_MINISWHITE );

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
	if ( !readCurrentDirectory( false ) )
	{
		return false;
	}

	try
	{
		/// Ideally we'd read the last scanline here, but we're unable to do that in all cases because not all
		/// compression methods support random access to the image data.
		ScopedTIFFErrorHandler errorHandler;

		readBuffer();

		return !errorHandler.hasError();
	}
	catch (...)
	{
		return false;
	}
}

Imath::Box2i TIFFImageReader::dataWindow()
{
	readCurrentDirectory( true );

	return m_dataWindow;
}

Imath::Box2i TIFFImageReader::displayWindow()
{
	readCurrentDirectory( true );

	return m_displayWindow;
}

std::string TIFFImageReader::sourceColorSpace() const
{
	readCurrentDirectory( true );

	// Handle 3delight tdls specially - they store the sourceColorSpace in the imageDescription
	if( boost::starts_with( m_software, "tdlmake" ) )
	{
		if( m_imageDescription.find( "InputSpace:BT.709" ) != std::string::npos ) return "rec709";
		else if( m_imageDescription.find( "InputSpace:sRGB" ) != std::string::npos ) return "srgb";
		else return "linear";
	}

	if ( m_sampleFormat == SAMPLEFORMAT_IEEEFP )
	{
		// Usually the tiffs are in linear colorspace if the channels are float.
		return "linear";
	}
	return "srgb";
}

unsigned int TIFFImageReader::numDirectories()
{
	open( true );

	return m_numDirectories;
}

void TIFFImageReader::setDirectory( unsigned int directoryIndex )
{
	unsigned int numDirs = numDirectories();
	if ( directoryIndex >= numDirs )
	{
		throw InvalidArgumentException( ( boost::format( "TIFFImageReader: Cannot read directory %s of %s in \"%s\"" ) % (directoryIndex+1) % numDirs % fileName() ).str() );
	}

	m_currentDirectoryIndex = directoryIndex;
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

template<typename T, typename V>
DataPtr TIFFImageReader::readTypedChannel( const std::string &name, const Box2i &dataWindow )
{
	typedef TypedData< std::vector< V > > TargetVector;

	typename TargetVector::Ptr dataContainer = new TargetVector();

	typename TargetVector::ValueType &data = dataContainer->writable();

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

	ScaledDataConversion<T, V> converter;

	int dataY = 0;
	for ( int y = dataWindow.min.y - m_dataWindow.min.y ; y <= dataWindow.max.y - m_dataWindow.min.y ; ++y, ++dataY )
	{
		int dataX = 0;

		for ( int x = dataWindow.min.x - m_dataWindow.min.x;  x <= dataWindow.max.x - m_dataWindow.min.x ; ++x, ++dataX  )
		{
			const T* buf = reinterpret_cast< T* >( & m_buffer[0] );
			assert( buf );

			// \todo Currently, we only support PLANARCONFIG_CONTIG for TIFFTAG_PLANARCONFIG.
			assert( m_planarConfig ==  PLANARCONFIG_CONTIG );
			typename TargetVector::ValueType::size_type dataOffset = dataY * dataWidth + dataX;
			assert( dataOffset < data.size() );

			data[dataOffset] = converter( buf[ m_samplesPerPixel * ( y * bufferDataWidth + x ) + channelOffset ] );
		}
	}

	return dataContainer;
}

DataPtr TIFFImageReader::readChannel( const std::string &name, const Imath::Box2i &dataWindow, bool raw )
{
	readCurrentDirectory( true );

	if ( m_buffer.size() == 0 )
	{
		readBuffer();
	}

	if ( m_sampleFormat == SAMPLEFORMAT_IEEEFP )
	{
		return readTypedChannel<float, float>( name, dataWindow );
	}
	else if ( m_sampleFormat == SAMPLEFORMAT_INT )
	{
		switch ( m_bitsPerSample )
		{
		case 8:
			if ( raw )
				return readTypedChannel<char, char>( name, dataWindow );
			else
				return readTypedChannel<char, float>( name, dataWindow );

		case 16:
			if ( raw )
				return readTypedChannel<int16, int16>( name, dataWindow );
			else
				return readTypedChannel<int16, float>( name, dataWindow );

		case 32:
			if ( raw )
				return readTypedChannel<int32, int32>( name, dataWindow );
			else
				return readTypedChannel<int32, float>( name, dataWindow );

		default:
			assert( false );
			return 0;
		}
	}
	else
	{
		assert( m_sampleFormat == SAMPLEFORMAT_UINT ) ;

		switch ( m_bitsPerSample )
		{
		case 8:
			if ( raw )
				return readTypedChannel<unsigned char, unsigned char>( name, dataWindow );
			else
				return readTypedChannel<unsigned char, float>( name, dataWindow );

		case 16:
			if ( raw )
				return readTypedChannel<uint16, uint16>( name, dataWindow );
			else
				return readTypedChannel<uint16, float>( name, dataWindow );

		case 32:
			if ( raw )
				return readTypedChannel<uint32, uint32>( name, dataWindow );
			else
				return readTypedChannel<uint32, float>( name, dataWindow );

		default:
			assert( false );
			return 0;
		}
	}
}

void TIFFImageReader::readBuffer()
{
	assert( m_tiffImage );
	assert( m_haveDirectory );

	int width = boxSize( m_dataWindow ).x + 1;
	int height = boxSize( m_dataWindow ).y + 1;

	// \todo Currently, we only support PLANARCONFIG_CONTIG for TIFFTAG_PLANARCONFIG.
	assert( m_planarConfig ==  PLANARCONFIG_CONTIG );
	std::vector<unsigned char>::size_type bufLineSize = (size_t)( (float)m_bitsPerSample / 8 * m_samplesPerPixel * width );
	std::vector<unsigned char>::size_type bufSize = bufLineSize * height;
	assert( bufSize );
	m_buffer.resize( bufSize, 0 );

	if ( TIFFIsTiled( m_tiffImage ) )
	{
		tsize_t tileSize = TIFFTileSize( m_tiffImage );
		ttile_t numTiles = TIFFNumberOfTiles( m_tiffImage );

		/// Create a buffer to hold an individual tile
		int tileWidth = tiffField<uint32>( TIFFTAG_TILEWIDTH );
		if ( tileWidth == 0 )
		{
			throw IOException( ( boost::format("TIFFImageReader: Unsupported value (%d) for TIFFTAG_TILEWIDTH while reading %s") % tileWidth % fileName() ).str() );
		}

		int tileLength = tiffField<uint32>( TIFFTAG_TILELENGTH );
		if ( tileLength == 0 )
		{
			throw IOException( ( boost::format("TIFFImageReader: Unsupported value (%d) for TIFFTAG_TILELENGTH while reading %s") % tileLength % fileName() ).str() );
		}

		std::vector<unsigned char>::size_type pixelSize = (size_t)( (float)m_bitsPerSample / 8 * m_samplesPerPixel );
		std::vector<unsigned char>::size_type tileLineSize = pixelSize * tileWidth;
		std::vector<unsigned char>::size_type tileBufSize = tileLineSize * tileLength;
		std::vector<unsigned char> tileBuffer;
		tileBuffer.resize( tileBufSize, 0 );

		int x = 0;
		int y = 0;

		/// Read each tile
		for ( ttile_t tile = 0; tile < numTiles; tile++ )
		{
			tsize_t imageOffset = y * bufLineSize + x * ( bufLineSize / width );
			int result = TIFFReadEncodedTile( m_tiffImage, tile, &tileBuffer[0], tileSize );

			if ( result == -1 )
			{
				throw IOException( (boost::format( "TIFFImageReader: Error on tile number %d while reading %s") % tile % fileName() ).str() );
			}

			/// Copy the tile into its rightful place in the image buffer.
			/// We have to be careful here as the image might not be an exact
			/// multiple of tiles, in which case we can't copy the tiles round the
			/// edges in their entirety as that would give us buffer overruns.
			int rowsToCopy = min( tileLength, height - y );
			int columnsToCopy = min( (int)tileWidth, width - x );
			tsize_t tileOffset = 0;
			for ( int l = 0; l < rowsToCopy; l ++)
			{
				memcpy( &m_buffer[0] + imageOffset, &tileBuffer[0] + tileOffset, pixelSize * columnsToCopy );
				imageOffset += bufLineSize;
				tileOffset += tileLineSize;
			}

			x = x + tileWidth;

			if ( x >= width )
			{
				x = 0;
				y += tileLength;
			}
		}
	}
	else
	{
		tsize_t stripSize = TIFFStripSize( m_tiffImage );
		tsize_t imageOffset = 0;
		tstrip_t numStrips = TIFFNumberOfStrips( m_tiffImage );
		for ( tstrip_t strip = 0; strip < numStrips; strip++ )
		{
			tsize_t result = TIFFReadEncodedStrip( m_tiffImage, strip, &m_buffer[0] + imageOffset, stripSize );

			if ( result == -1 )
			{
				throw IOException( (boost::format( "TIFFImageReader: Error on strip number %d while reading %s") % strip % fileName() ).str() );
			}

			imageOffset += result;
		}
	}
}

bool TIFFImageReader::open( bool throwOnFailure )
{
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

	try
	{
	
		ScopedTIFFErrorHandler errorHandler;

		m_tiffImage = TIFFOpen( fileName().c_str(), "r" );
		errorHandler.throwIfError();

		m_numDirectories = 1;
		TIFFSetDirectory( m_tiffImage, 0 );
		while ( !TIFFLastDirectory( m_tiffImage ) )
		{
			TIFFReadDirectory( m_tiffImage );
			if( errorHandler.hasError() )
			{
				if( errorHandler.errorMessage().find( "SMinSampleValue" ) != string::npos ||
					errorHandler.errorMessage().find( "SMaxSampleValue" ) != string::npos
				)
				{
					// 3delight makes tiff files which have SMinSampleValue and SMaxSampleValue
					// tags where the values differ per sample, and libtiff currently doesn't support that.
					// fortunately 3delight only puts those tags on the last directory, so we
					// just ignore the problem and end up with one less directory than we should.
					errorHandler.clear();
				}
			}
			else
			{
				m_numDirectories ++;
			}
		}
		TIFFSetDirectory( m_tiffImage, 0 );

		errorHandler.throwIfError();
		
	}
	catch( ... )
	{
		if( throwOnFailure )
		{
			throw;
		}
		return false;
	}

	assert( m_tiffImage );
	m_tiffImageFileName = fileName();
	
	return true;
}

bool TIFFImageReader::readCurrentDirectory( bool throwOnFailure ) const
{
	return const_cast< TIFFImageReader * >(this)->readCurrentDirectory(throwOnFailure);
}

bool TIFFImageReader::readCurrentDirectory( bool throwOnFailure )
{
	if ( !open( throwOnFailure ) )
	{
		return false;
	}

	assert ( m_tiffImage );


	try
	{
		ScopedTIFFErrorHandler errorHandler;

		if ( m_haveDirectory && m_currentDirectoryIndex == TIFFCurrentDirectory( m_tiffImage ) )
		{
			return true;
		}
		else
		{
			int res = TIFFSetDirectory( m_tiffImage, m_currentDirectoryIndex );
			if ( res != 1 )
			{
				return false;
			}
		}

		m_buffer.clear();

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
				m_photometricInterpretation == PHOTOMETRIC_MINISWHITE ||
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
		                m_sampleFormat == SAMPLEFORMAT_IEEEFP ||
		                m_sampleFormat == SAMPLEFORMAT_INT
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

		// The pointer returned by TIFFGetField will be invalidated if we change the directory index.
		// Copy it to a string to be safe
		m_software = std::string( tiffField< char* >( TIFFTAG_SOFTWARE, "" ) );
		m_imageDescription = std::string( tiffField< char* >( TIFFTAG_IMAGEDESCRIPTION, "" ) );

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

		errorHandler.throwIfError();

		m_haveDirectory = true;
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

	return m_haveDirectory;
}

