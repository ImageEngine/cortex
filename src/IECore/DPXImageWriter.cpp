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

#include "IECore/DPXImageWriter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/BoxOperators.h"
#include "IECore/DataConvert.h"
#include "IECore/ScaledDataConversion.h"

#include "IECore/private/dpx.h"

#include "boost/format.hpp"

#include <fstream>
#include <time.h>

using namespace IECore;
using namespace std;
using namespace boost;
using namespace Imath;

const Writer::WriterDescription<DPXImageWriter> DPXImageWriter::m_writerDescription("dpx");

DPXImageWriter::DPXImageWriter() :
		ImageWriter("DPXImageWriter", "Serializes images to Digital Picture eXchange 10-bit log image format")
{
}

DPXImageWriter::DPXImageWriter( ObjectPtr image, const string &fileName ) :
		ImageWriter("DPXImageWriter", "Serializes images to Digital Picture eXchange 10-bit log image format")
{
	m_objectParameter->setValue( image );
	m_fileNameParameter->setTypedValue( fileName );
}

DPXImageWriter::~DPXImageWriter()
{
}


template<typename T>
void DPXImageWriter::encodeChannel( ConstDataPtr dataContainer, const Box2i &displayWindow, const Box2i &dataWindow, int bitShift, vector<unsigned int> &imageBuffer )
{
	const typename T::ValueType &data = static_pointer_cast<const T>( dataContainer )->readable();
	ScaledDataConversion<typename T::ValueType::value_type, float> converter;

	int displayWidth = displayWindow.size().x + 1;
	int dataWidth = dataWindow.size().x + 1;

	int dataX = 0;
	int dataY = 0;

	for ( int y = dataWindow.min.y; y <= dataWindow.max.y; y++, dataY++ )
	{
		int dataOffset = dataY * dataWidth + dataX;
		assert( dataOffset >= 0 );

		for ( int x = dataWindow.min.x; x <= dataWindow.max.x; x++, dataOffset++ )
		{
			int pixelIdx = ( y - displayWindow.min.y ) * displayWidth + ( x - displayWindow.min.x );

			assert( pixelIdx >= 0 );
			assert( pixelIdx < (int)imageBuffer.size() );
			assert( dataOffset < (int)data.size() );

			vector<double>::iterator where = lower_bound(m_LUT.begin(), m_LUT.end(), converter( data[dataOffset] ) );
			unsigned int logValue = distance(m_LUT.begin(), where);
			imageBuffer[ pixelIdx ] |= logValue << bitShift;
		}
	}
}

void DPXImageWriter::writeImage( vector<string> &names, ConstImagePrimitivePtr image, const Box2i &dataWindow )
{
	// write the dpx in the standard 10bit log format
	ofstream out;
	out.open(fileName().c_str());
	if ( !out.is_open() )
	{
		throw IOException( "DPXImageWriter: Error writing to " + fileName() );
	}
	
	/// We'd like RGB to be at the front, in that order, because it seems that not all readers support the channel identifiers!
	vector<string> desiredChannelOrder;
	desiredChannelOrder.push_back( "R" );
	desiredChannelOrder.push_back( "G" );
	desiredChannelOrder.push_back( "B" );

	vector<string> namesCopy = names;
	vector<string> filteredNames;

	for ( vector<string>::const_iterator it = desiredChannelOrder.begin(); it != desiredChannelOrder.end(); ++it )
	{
		vector<string>::iterator res = find( namesCopy.begin(), namesCopy.end(), *it );
		if ( res != namesCopy.end() )
		{
			namesCopy.erase( res );
			filteredNames.push_back( *it );
		}
	}

	for ( vector<string>::const_iterator it = namesCopy.begin(); it != namesCopy.end(); ++it )
	{
		filteredNames.push_back( *it );
	}

	assert( names.size() == filteredNames.size() );
	
	Box2i displayWindow = image->getDisplayWindow();

	int displayWidth  = 1 + displayWindow.size().x;
	int displayHeight = 1 + displayWindow.size().y;

	// build the header
	DPXFileInformation fi;
	memset(&fi, 0, sizeof(fi));

	DPXImageInformation ii;
	memset(&ii, 0, sizeof(ii));

	DPXImageOrientation ioi;
	memset(&ioi, 0, sizeof(ioi));

	DPXMotionPictureFilm mpf;
	memset(&mpf, 0, sizeof(mpf));

	DPXTelevisionHeader th;
	memset(&th, 0, sizeof(th));

	fi.magic = asBigEndian<>( 0x53445058 );

	// compute data offsets
	fi.gen_hdr_size = sizeof(fi) + sizeof(ii) + sizeof(ioi);
	fi.gen_hdr_size = asBigEndian<>(fi.gen_hdr_size);

	fi.ind_hdr_size = sizeof(mpf) + sizeof(th);
	fi.ind_hdr_size = asBigEndian<>(fi.ind_hdr_size);

	int header_size = sizeof(fi) + sizeof(ii) + sizeof(ioi) + sizeof(mpf) + sizeof(th);
	fi.image_data_offset = header_size;
	fi.image_data_offset = asBigEndian<>(fi.image_data_offset);

	strcpy((char *) fi.vers, "V2.0");

	strncpy( (char *) fi.file_name, fileName().c_str(), sizeof( fi.file_name ) );

	// compute the current date and time
	time_t t;
	time(&t);
	struct tm gmt;
	localtime_r(&t, &gmt);
	snprintf((char *) fi.create_time,  sizeof( fi.create_time ), "%04d:%02d:%02d:%02d:%02d:%02d:%s",
	        1900 + gmt.tm_year, gmt.tm_mon, gmt.tm_mday,
	        gmt.tm_hour, gmt.tm_min, gmt.tm_sec, gmt.tm_zone );
	
	snprintf((char *) fi.creator, sizeof( fi.creator ), "cortex");
	snprintf((char *) fi.project, sizeof( fi.project ), "cortex");
	snprintf((char *) fi.copyright, sizeof( fi.copyright ), "Unknown");

	ii.orientation = 0;    // left-to-right, top-to-bottom
	ii.element_number = 1;
	ii.pixels_per_line = displayWidth;
	ii.lines_per_image_ele = displayHeight;

	ii.element_number      = asBigEndian<>(ii.element_number);
	ii.pixels_per_line     = asBigEndian<>(ii.pixels_per_line);
	ii.lines_per_image_ele = asBigEndian<>(ii.lines_per_image_ele);

	for (int c = 0; c < 8; ++c)
	{
		DPXImageInformation::_image_element &ie = ii.image_element[c];
		ie.data_sign = 0;

		/// \todo Dcoument these constants
		ie.ref_low_data = 0;
		ie.ref_low_quantity = 0.0;
		ie.ref_high_data = 1023;
		ie.ref_high_quantity = 2.046;
		
		ie.ref_low_data = asBigEndian<>(ie.ref_low_data);
		ie.ref_low_quantity = asBigEndian<>(ie.ref_low_quantity);
		ie.ref_high_data = asBigEndian<>(ie.ref_high_data);
		ie.ref_high_quantity = asBigEndian<>(ie.ref_high_quantity);

		/// \todo Dcoument these constants
		ie.transfer = 1;
		ie.packing = 256;
		ie.bit_size = 10;
		ie.descriptor = 50;

		ie.data_offset = fi.image_data_offset;
	}

	ioi.x_offset = 0;
	ioi.y_offset = 0;

	// Write the header
	int image_data_size = sizeof( unsigned int ) * displayWidth * displayHeight;
	fi.file_size = header_size + image_data_size;

	fi.file_size = asBigEndian<>(fi.file_size);

	out.write(reinterpret_cast<char *>(&fi),  sizeof(fi));
	if ( out.fail() )
	{
		throw IOException( "DPXImageWriter: Error writing to " + fileName() );
	}
	
	out.write(reinterpret_cast<char *>(&ii),  sizeof(ii));
	if ( out.fail() )
	{
		throw IOException( "DPXImageWriter: Error writing to " + fileName() );
	}
	
	out.write(reinterpret_cast<char *>(&ioi), sizeof(ioi));
	if ( out.fail() )
	{
		throw IOException( "DPXImageWriter: Error writing to " + fileName() );
	}
		
	out.write(reinterpret_cast<char *>(&mpf), sizeof(mpf));
	if ( out.fail() )
	{
		throw IOException( "DPXImageWriter: Error writing to " + fileName() );
	}
		
	out.write(reinterpret_cast<char *>(&th),  sizeof(th));
	if ( out.fail() )
	{
		throw IOException( "DPXImageWriter: Error writing to " + fileName() );
	}	

	// build a reverse LUT (linear to logarithmic)
	double film_gamma = 0.6;
	int ref_white_val = 685;
	int ref_black_val = 95;
	double ref_mult = 0.002 / film_gamma;
	double black_offset = pow(10.0, (ref_black_val - ref_white_val) * ref_mult);

	
	m_LUT.resize(1024);
	for (int i = 0; i < 1024; ++i)
	{
		double v = i + 0.5;
		m_LUT[i] = (pow(10.0, (v - ref_white_val) * ref_mult) - black_offset) / (1.0 - black_offset);
	}
	
	// write the data
	vector<unsigned int> imageBuffer( displayWidth*displayHeight, 0 );
	
	int offset = 0;
	vector<string>::const_iterator i = filteredNames.begin();
	while (i != filteredNames.end())
	{
		if (!(*i == "R" || *i == "G" || *i == "B"))
		{
			msg( Msg::Warning, "DPXImageWriter::write", format( "Channel \"%s\" was not encoded." ) % *i );
			++i;
			continue;
		}
		
		int bpp = 10;
		unsigned int shift = (32 - bpp) - (offset*bpp);

		assert( image->variables.find( *i ) != image->variables.end() );
		DataPtr dataContainer = image->variables.find( *i )->second.data;
		assert( dataContainer );

		switch (dataContainer->typeId())
		{
		case FloatVectorDataTypeId:

			encodeChannel<FloatVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case HalfVectorDataTypeId:

			encodeChannel<HalfVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case DoubleVectorDataTypeId:

			encodeChannel<DoubleVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case LongVectorDataTypeId:

			encodeChannel<LongVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case CharVectorDataTypeId:

			encodeChannel<CharVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case UCharVectorDataTypeId:

			encodeChannel<UCharVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case ShortVectorDataTypeId:

			encodeChannel<ShortVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case UShortVectorDataTypeId:

			encodeChannel<UShortVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case IntVectorDataTypeId:

			encodeChannel<IntVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		case UIntVectorDataTypeId:

			encodeChannel<UIntVectorData>( dataContainer, displayWindow, dataWindow, shift, imageBuffer );
			break;

		default:
			throw InvalidArgumentException( (format( "DPXImageWriter: Invalid data type \"%s\" for channel \"%s\"." ) % Object::typeNameFromTypeId(dataContainer->typeId()) % *i).str() );
		}

		++i;
		++offset;
	}

	// write the buffer
	for (int i = 0; i < displayWidth * displayHeight; ++i)
	{
		imageBuffer[i] = asBigEndian<>(imageBuffer[i]);
		out.write( (const char *) (&imageBuffer[i]), sizeof(unsigned int) );
		if ( out.fail() )
		{
			throw IOException( "DPXImageWriter: Error writing to " + fileName() );
		}
	}
}
