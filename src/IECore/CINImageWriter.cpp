//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CINImageWriter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/BoxOperators.h"

#include "IECore/private/cineon.h"

#include "boost/format.hpp"

#include <fstream>
#include <time.h>

using namespace IECore;
using namespace std;
using namespace boost;
using namespace Imath;

const Writer::WriterDescription<CINImageWriter> CINImageWriter::m_writerDescription("cin");

CINImageWriter::CINImageWriter() : 
		ImageWriter("CINImageWriter", "Serializes images to the Kodak Cineon 10-bit log image format")
{
}

CINImageWriter::CINImageWriter(ObjectPtr image, const string &fileName) : 
		ImageWriter("CINImageWriter", "Serializes images to the Kodak Cineon 10-bit log image format")
{
	m_objectParameter->setValue( image );
	m_fileNameParameter->setTypedValue( fileName );
}

CINImageWriter::~CINImageWriter()
{
}

void CINImageWriter::writeImage(vector<string> &names, ConstImagePrimitivePtr image, const Box2i &dw)
{
	// write the cineon in the standard 10bit log format
	std::ofstream out;
	out.open(fileName().c_str());//, "wb");
	if(!out.is_open())
	{
		throw IOException("Could not open '" + fileName() + "' for writing.");
	}
	
	// assume an 8-bit RGB image
	int width  = 1 + dw.max.x - dw.min.x;
	int height = 1 + dw.max.y - dw.min.y;
	
	//
	// FileInformation
	//
	
	// build the header
	FileInformation fi;
	fi.magic = 0xd75f2a80;

	fi.section_header_length = 0;
	fi.industry_header_length = 0;
	fi.variable_header_length = 0;
	
	strcpy(fi.version, "V4.5");
	
	/// \todo What is the purpose of this?
	strcpy(fi.file_name, "image-engine.cin");

	// compute the current date and time
	time_t t;
	time(&t);
	struct tm gmt;
	localtime_r(&t, &gmt);

	sprintf(fi.creation_date, "%04d-%02d-%02d", 1900 + gmt.tm_year, gmt.tm_mon, gmt.tm_mday);
	sprintf(fi.creation_time, "%02d:%02d:%02d", gmt.tm_hour, gmt.tm_min, gmt.tm_sec);
		
	//
	// ImageInformation;
	//
	ImageInformation ii;
	ii.orientation = 0;    // left-to-right, top-to-bottom
	ii.channel_count = 3;  // rgb

	for(int c = 0; c < 8; ++c) {
		ImageInformationChannelInformation &ci = ii.channel_information[c];
		ci.byte_0 = 0;
		ci.byte_1 = 0;
		ci.bpp = 10;

		ci.pixels_per_line = 0;
		ci.lines_per_image = 0;
	}
	
	for(int c = 0; c < 3; ++c) {
		ImageInformationChannelInformation &ci = ii.channel_information[c];
		ci.byte_0 = 0;
		ci.byte_1 = c+1;
		ci.bpp = 10;

		/// \todo Establish why we're not calling asLittleEndian or asBigEndian here
		ci.pixels_per_line = reverseBytes(width);
		ci.lines_per_image = reverseBytes(height);
		
		/// \todo Document these constants
		ci.min_data_value = 0.0;
		ci.min_quantity = 0.0;
		ci.max_data_value = 1023.0;
		ci.max_quantity = 2.046;
		
		/// \todo Establish why we're not calling asLittleEndian or asBigEndian here
		ci.min_data_value = reverseBytes(ci.min_data_value);
		ci.min_quantity   = reverseBytes(ci.min_quantity);
		ci.max_data_value = reverseBytes(ci.max_data_value);
		ci.min_quantity   = reverseBytes(ci.max_quantity);
	}

	//
	// ImageDataFormatInformation
	// 
	ImageDataFormatInformation idfi;
	idfi.interleave = 0;               // pixel interleave
	idfi.packing = 5;                  // 32 bit left-aligned with 2 waste bits
	idfi.data_signed = 0;              // unsigned data
	idfi.sense = 0;                    // positive image sense
	idfi.eol_padding = 0;              // no end-of-line padding
	idfi.eoc_padding = 0;              // no end-of-channel padding

	//
	// ImageOriginationInformation
	//
	ImageOriginationInformation ioi;
	ioi.x_offset = 0;                  // could be dataWindow min.x
	ioi.y_offset = 0;                  // could be dataWindow min.y
	// other items left out for now
	ioi.gamma = 0x7f800000;
	ioi.gamma = reverseBytes(ioi.gamma);
	
	// write the header

	// compute data offsets
	fi.image_data_offset = 1024;
	fi.image_data_offset = reverseBytes(fi.image_data_offset);

	// file size is 1024 (header) + image data size
	// image data size is 32 bits (4 bytes) times width*height
	int image_data_size = 4 * width * height;
	fi.total_file_size = 1024 + image_data_size;
	fi.total_file_size = reverseBytes(fi.total_file_size);
	
	out.write(reinterpret_cast<char *>(&fi),   sizeof(fi));
	out.write(reinterpret_cast<char *>(&ii),   sizeof(ii));
	out.write(reinterpret_cast<char *>(&idfi), sizeof(idfi));
	out.write(reinterpret_cast<char *>(&ioi),  sizeof(ioi));

	// write the data
	std::vector<unsigned int> image_buffer( width*height, 0 );
	
	// build a LUT
	double film_gamma = 0.6;
	int ref_white_val = 685;
	int ref_black_val = 95;
	double ref_mult = 0.002 / film_gamma;
	double black_offset = pow(10.0, (ref_black_val - ref_white_val) * ref_mult);

	// build a reverse LUT (linear to logarithmic)
	vector<double> range(1024);
	for(int i = 0; i < 1024; ++i) {
		double v = i + 0.5;
		range[i] = (pow(10.0, (v - ref_white_val) * ref_mult) - black_offset) / (1.0 - black_offset);
	}
	vector<double>::iterator where;
	
	// add the channels into the header with the appropriate types
	// channel data is RGB interlaced
	vector<string>::const_iterator i = names.begin();
	while(i != names.end())
	{		
		if(!(*i == "R" || *i == "G" || *i == "B"))
		{
			msg( Msg::Warning, "CINImageWriter::write", format( "Channel \"%s\" was not encoded." ) % *i );
			++i;
			continue;
		}
		
		int offset = *i == "R" ? 0 : *i == "G" ? 1 : 2;
		int bpp = 10;
		unsigned int shift = (32 - bpp) - (offset*bpp);
		
		// get the image channel
		DataPtr channelp = image->variables.find( *i )->second.data;
		
		switch(channelp->typeId())
		{
			
		case FloatVectorDataTypeId:
		{
			const vector<float> &channel = static_pointer_cast<FloatVectorData>(channelp)->readable();

			// convert the linear float value to 10-bit log
			for(int i = 0; i < width*height; ++i)
			{
				/// \todo Examine the performance of this!
				where = lower_bound(range.begin(), range.end(), channel[i]);
				unsigned int log_value = distance(range.begin(), where);
				image_buffer[i] |= log_value << shift;
			}
		}
		break;
			
		case HalfVectorDataTypeId:
		{
			const vector<half> &channel = static_pointer_cast<HalfVectorData>(channelp)->readable();
			
			// convert the linear half value to 10-bit log
			for(int i = 0; i < width*height; ++i)
			{
				/// \todo Examine the performance of this!
				where = lower_bound(range.begin(), range.end(), channel[i]);
				unsigned int log_value = distance(range.begin(), where);
				image_buffer[i] |= log_value << shift;
			}
		}
		break;
		
		/// \todo Deal with other channel types, preferably using templates!
			
		default:
			throw InvalidArgumentException( (format( "CINImageWriter: Invalid data type \"%s\" for channel \"%s\"." ) % Object::typeNameFromTypeId(channelp->typeId()) % *i).str() );
		}
		
		++i;
	}
	
	// write the buffer
	for(int i = 0; i < width*height; ++i)
	{
		/// \todo Why is this call to reverseBytes here? If we want to write in either little endian or big endian format there
		/// are calls specifically do this, which work regardless of which architecture the code is running on
		image_buffer[i] = reverseBytes(image_buffer[i]);
		out.write((const char *) (&image_buffer[i]), sizeof(unsigned int));
	}
}
