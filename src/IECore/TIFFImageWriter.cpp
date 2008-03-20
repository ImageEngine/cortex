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

#include "boost/static_assert.hpp"
#include "boost/format.hpp"
#include "boost/mpl/eval_if.hpp"

#include "boost/type_traits/is_same.hpp"

#include "OpenEXR/half.h"
#include "OpenEXR/ImathLimits.h"

#include "IECore/TIFFImageWriter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/Parameter.h"
#include "IECore/NumericParameter.h"
#include "IECore/ScopedTIFFExceptionTranslator.h"
#include "IECore/DataConvert.h"
#include "IECore/ScaledDataConversion.h"

#include "IECore/BoxOperators.h"

#include "tiffio.h"

using namespace IECore;
using namespace std;
using namespace Imath;
using namespace boost;

const Writer::WriterDescription<TIFFImageWriter> TIFFImageWriter::m_writerDescription("tiff tif");

TIFFImageWriter::TIFFImageWriter()
		: 	ImageWriter("TIFFImageWriter", "Serializes images to the Tagged Image File Format (TIFF) format")
{
	constructParameters();
}

TIFFImageWriter::TIFFImageWriter( ObjectPtr image, const string &fileName )
		: 	ImageWriter("TIFFImageWriter", "Serializes images to the Tagged Image File Format (TIFF) format")
{
	constructParameters();
	m_objectParameter->setValue( image );
	m_fileNameParameter->setTypedValue( fileName );
}

void TIFFImageWriter::constructParameters()
{
	IntParameter::PresetsMap bitDepthPresets;
	bitDepthPresets["8"] = 8;
	bitDepthPresets["16"] = 16;
	bitDepthPresets["32"] = 32;

	m_bitDepthParameter = new IntParameter(
	        "bitdepth",
	        "Output TIFF bit depth",
	        16,
	        8,
	        32,
	        bitDepthPresets,
	        true);

	parameters()->addParameter( m_bitDepthParameter );

	// compression parameter
	IntParameter::PresetsMap compressionPresets;
	compressionPresets["none"]    = COMPRESSION_NONE;
	compressionPresets["lzw"]     = COMPRESSION_LZW;
	compressionPresets["jpeg"]    = COMPRESSION_JPEG;
	compressionPresets["deflate"] = COMPRESSION_DEFLATE;

	/// Verify min
	BOOST_STATIC_ASSERT( COMPRESSION_NONE < COMPRESSION_LZW );
	BOOST_STATIC_ASSERT( COMPRESSION_NONE < COMPRESSION_JPEG );
	BOOST_STATIC_ASSERT( COMPRESSION_NONE < COMPRESSION_DEFLATE );

	/// Verify max
	BOOST_STATIC_ASSERT( COMPRESSION_DEFLATE > COMPRESSION_NONE );
	BOOST_STATIC_ASSERT( COMPRESSION_DEFLATE > COMPRESSION_LZW );
	BOOST_STATIC_ASSERT( COMPRESSION_DEFLATE > COMPRESSION_JPEG );

	m_compressionParameter = new IntParameter(
	        "compression",
	        "TIFF compression method",
	        compressionPresets["lzw"],
	        COMPRESSION_NONE,
	        COMPRESSION_DEFLATE,
	        compressionPresets,
	        true
	);

	parameters()->addParameter( m_compressionParameter );
}

TIFFImageWriter::~TIFFImageWriter()
{
}

template<typename T>
void TIFFImageWriter::encodeChannels( ConstImagePrimitivePtr image, const vector<string> &names, const Imath::Box2i &dataWindow, tiff *tiffImage, size_t bufSize, unsigned int numStrips )
{
	assert( tiffImage );

	int width  = 1 + dataWindow.max.x - dataWindow.min.x;
	int height = 1 + dataWindow.max.y - dataWindow.min.y;
	int area = width * height;

	int samplesPerPixel = names.size();

	// Build a vector in which we place all the image channels

	typedef TypedData< vector<T> > ChannelData;

	typename ChannelData::Ptr channelData = 0;

	vector<T> imageBuffer( samplesPerPixel * area, 0 );

	// Encode eaech individual channel into the buffer
	int channelOffset = 0;
	for ( vector<string>::const_iterator i = names.begin(); i != names.end(); ++i, ++channelOffset )
	{
		DataPtr dataContainer = image->variables.find(i->c_str())->second.data;
		assert( dataContainer );

		switch (dataContainer->typeId())
		{

		case FloatVectorDataTypeId:
			channelData = VectorDataConvert < FloatVectorData, ChannelData, ScaledDataConversion<float, T> >()(
			                      boost::static_pointer_cast<const FloatVectorData>( dataContainer )
			              );
			break;

		case LongVectorDataTypeId:
			channelData = VectorDataConvert< LongVectorData, ChannelData, ScaledDataConversion<long, T> >()(
			                      boost::static_pointer_cast<const LongVectorData>( dataContainer )
			              );
			break;

		case CharVectorDataTypeId:
			channelData = VectorDataConvert< CharVectorData, ChannelData, ScaledDataConversion<char, T> >()(
			                      boost::static_pointer_cast<const CharVectorData>( dataContainer )
			              );
			break;

		case UCharVectorDataTypeId:
			channelData = VectorDataConvert< UCharVectorData, ChannelData, ScaledDataConversion<unsigned char, T> >()(
			                      boost::static_pointer_cast<const UCharVectorData>( dataContainer )
			              );
			break;

		case DoubleVectorDataTypeId:
			channelData = VectorDataConvert< DoubleVectorData, ChannelData, ScaledDataConversion<double, T> >()(
			                      boost::static_pointer_cast<const DoubleVectorData>( dataContainer )
			              );
			break;

		case HalfVectorDataTypeId:
			channelData = VectorDataConvert< HalfVectorData, ChannelData, ScaledDataConversion<half, T> >()(
			                      boost::static_pointer_cast<const HalfVectorData>( dataContainer )
			              );
			break;

		case IntVectorDataTypeId:
			channelData = VectorDataConvert< IntVectorData, ChannelData, ScaledDataConversion<int, T> >()(
			                      boost::static_pointer_cast<const IntVectorData>( dataContainer )
			              );
			break;

		case UIntVectorDataTypeId:
			channelData = VectorDataConvert< UIntVectorData, ChannelData, ScaledDataConversion<unsigned int, T> >()(
			                      boost::static_pointer_cast<const UIntVectorData>( dataContainer )
			              );
			break;

		case ShortVectorDataTypeId:
			channelData = VectorDataConvert< ShortVectorData, ChannelData, ScaledDataConversion<short, T> >()(
			                      boost::static_pointer_cast<const ShortVectorData>( dataContainer )
			              );
			break;

		case UShortVectorDataTypeId:
			channelData = VectorDataConvert< UShortVectorData, ChannelData, ScaledDataConversion<unsigned short, T> >()(
			                      boost::static_pointer_cast<const UShortVectorData>( dataContainer )
			              );
			break;

		default:
			throw InvalidArgumentException( (boost::format( "TIFFImageWriter: Invalid data type \"%s\" for channel \"%s\"." ) % Object::typeNameFromTypeId(dataContainer->typeId()) % *i).str() );
		}

		assert( channelData );

		if ( channelData->readable().size() == 0)
		{
			throw InvalidArgumentException( (boost::format( "TIFFImageWriter: Invalid data size \"%s\" for channel \"%s\"." ) % Object::typeNameFromTypeId(dataContainer->typeId()) % *i).str() );
		}

		for ( typename ChannelData::ValueType::size_type j = 0; j < channelData->readable().size(); ++j )
		{
			imageBuffer[ samplesPerPixel*j + channelOffset ] = channelData->readable()[j];
		}
	}

	/// Write the image buffer to the TIFF file, strip by strip
	int offset = 0;
	for ( tstrip_t strip = 0; strip < numStrips; ++strip )
	{
		int tss = TIFFStripSize(tiffImage);
		assert( tss >= 0 );

		int remaining = bufSize - offset;
		assert( remaining >= 0 );

		tsize_t lc = TIFFWriteEncodedStrip( tiffImage, strip,  (char *) &imageBuffer[0] + offset, tss < remaining ? tss : remaining );
		if ( lc == -1 )
		{
			throw IOException( ( boost::format( "TIFFImageWriter: Error writing strip %d to %s" ) % strip % fileName() ).str() );
		}

		offset += lc;
	}
}

void TIFFImageWriter::writeImage( vector<string> &names, ConstImagePrimitivePtr image, const Box2i &dataWindow )
{
	ScopedTIFFExceptionTranslator errorHandler;

	// create the tiff file
	TIFF *tiffImage;
	if ((tiffImage = TIFFOpen(fileName().c_str(), "w")) == NULL)
	{
		throw IOException("TIFFImageWriter: Could not open '" + fileName() + "' for writing.");
	}

	assert( tiffImage );

	try
	{
		/// Get the channels RGBA at the front, in that order, if they exist
		vector<string> desiredChannelOrder;
		desiredChannelOrder.push_back( "R" );
		desiredChannelOrder.push_back( "G" );
		desiredChannelOrder.push_back( "B" );
		desiredChannelOrder.push_back( "A" );

		vector<string> namesCopy = names;
		vector<string> filteredNames;

		int rgbChannelsFound = 0;
		bool haveAlpha = false;
		for ( vector<string>::const_iterator it = desiredChannelOrder.begin(); it != desiredChannelOrder.end(); ++it )
		{
			vector<string>::iterator res = find( namesCopy.begin(), namesCopy.end(), *it );
			if ( res != namesCopy.end() )
			{
				if ( *it == "A" )
				{
					haveAlpha = true;
				}
				else
				{
					rgbChannelsFound ++;
				}
				namesCopy.erase( res );
				filteredNames.push_back( *it );
			}
		}

		for ( vector<string>::const_iterator it = namesCopy.begin(); it != namesCopy.end(); ++it )
		{
			filteredNames.push_back( *it );
		}

		assert( names.size() == filteredNames.size() );

		if ( rgbChannelsFound == 0 )
		{
			TIFFSetField( tiffImage, TIFFTAG_PHOTOMETRIC, (uint16)PHOTOMETRIC_MINISBLACK );
		}
		else if ( rgbChannelsFound == 3 )
		{
			TIFFSetField( tiffImage, TIFFTAG_PHOTOMETRIC, (uint16)PHOTOMETRIC_RGB );
		}
		else
		{
			throw IOException("TIFFImageWriter: Incorrect number of RGB channels specified while writing " + fileName() );
		}

		// compute the number of channels
		int samplesPerPixel = filteredNames.size();

		TIFFSetField( tiffImage, TIFFTAG_SAMPLESPERPIXEL, (uint16)samplesPerPixel );

		int numExtraSamples = filteredNames.size() - rgbChannelsFound;
		assert( numExtraSamples >= 0 );

		vector<uint16> extraSamples;
		if ( haveAlpha )
		{
			extraSamples.push_back( (uint16)EXTRASAMPLE_UNASSALPHA );
		}

		if ( numExtraSamples )
		{
			while ( (int)extraSamples.size() < numExtraSamples )
			{
				extraSamples.push_back( EXTRASAMPLE_UNSPECIFIED );
			}

			assert( (int)extraSamples.size() == numExtraSamples );

			TIFFSetField( tiffImage, TIFFTAG_EXTRASAMPLES, extraSamples.size(), (uint16*)&extraSamples[0] );
		}

		// compute the writebox
		int width  = 1 + dataWindow.max.x - dataWindow.min.x;
		int height = 1 + dataWindow.max.y - dataWindow.min.y;

		/// \todo different compression methods have a bearing on other attributes, eg. the strip size
		/// handle these issues a bit better and perhaps more explicitly here.  also, we should probably
		/// warn the user in cases where parameter settings are not permitted (eg 16 bit jpeg)
		int compression = parameters()->parameter<IntParameter>("compression")->getNumericValue();
		TIFFSetField( tiffImage, TIFFTAG_COMPRESSION, compression );

		int bitDepth = m_bitDepthParameter->getNumericValue();
		if ( compression == COMPRESSION_JPEG )
		{
			/// \todo Warn
			/// \todo Decide whether to change the compression method or the bitdepth
			bitDepth = 8;
		}

		switch ( bitDepth )
		{
		case 8:
			TIFFSetField( tiffImage, TIFFTAG_SAMPLEFORMAT, (uint16)SAMPLEFORMAT_UINT );
			break;
		case 16:
			TIFFSetField( tiffImage, TIFFTAG_SAMPLEFORMAT, (uint16)SAMPLEFORMAT_UINT );
			break;
		case 32:
			TIFFSetField( tiffImage, TIFFTAG_SAMPLEFORMAT, (uint16)SAMPLEFORMAT_IEEEFP );
			break;
		default:
			assert( 0 );
		}

		// number of strips to write.  TIFF's JPEG compression requires rps to be a multiple of 8
		int rowsPerStrip = 8;

		// now properly compute # of strips
		int strips = height / rowsPerStrip + (height % rowsPerStrip > 0 ? 1 : 0);

		// set the basic values
		TIFFSetField( tiffImage, TIFFTAG_IMAGEWIDTH, (uint32)width );
		TIFFSetField( tiffImage, TIFFTAG_IMAGELENGTH, (uint32)height );

		if ( dataWindow != image->getDisplayWindow() )
		{
			V2i position = dataWindow.min - image->getDisplayWindow().min;

			TIFFSetField( tiffImage, TIFFTAG_XPOSITION, (float) position.x );
			TIFFSetField( tiffImage, TIFFTAG_YPOSITION, (float) position.y );

			int displayWidth = 1 + image->getDisplayWindow().size().x;
			int displayHeight = 1 + image->getDisplayWindow().size().y;
			TIFFSetField( tiffImage, TIFFTAG_PIXAR_IMAGEFULLWIDTH, (uint32)( displayWidth ) );
			TIFFSetField( tiffImage, TIFFTAG_PIXAR_IMAGEFULLLENGTH, (uint32)( displayHeight ) );
		}

		TIFFSetField( tiffImage, TIFFTAG_BITSPERSAMPLE, (uint16)bitDepth );
		TIFFSetField( tiffImage, TIFFTAG_ROWSPERSTRIP, (uint32)rowsPerStrip );

		/// \todo What about files written on big endian platforms?
		TIFFSetField( tiffImage, TIFFTAG_FILLORDER, (uint16)FILLORDER_MSB2LSB );
		TIFFSetField( tiffImage, TIFFTAG_PLANARCONFIG, (uint16)PLANARCONFIG_CONTIG );

		TIFFSetField( tiffImage, TIFFTAG_XRESOLUTION, 1 );
		TIFFSetField( tiffImage, TIFFTAG_YRESOLUTION, 1 );
		TIFFSetField( tiffImage, TIFFTAG_RESOLUTIONUNIT, (uint16)RESUNIT_NONE );

		size_t bufSize = (size_t)( (float)bitDepth / 8 * samplesPerPixel * width * height );
		assert( bufSize );

		switch ( bitDepth )
		{
		case 8:
			encodeChannels<unsigned char>(image, filteredNames, dataWindow, tiffImage, bufSize, strips );
			break;

		case 16:
			encodeChannels<uint16>(image, filteredNames, dataWindow, tiffImage, bufSize, strips );
			break;

		case 32:
			encodeChannels<float>(image, filteredNames, dataWindow, tiffImage, bufSize, strips );
			break;
		}
	}
	catch (...)
	{
		TIFFClose( tiffImage );
		throw;
	}

	TIFFClose( tiffImage );
}
