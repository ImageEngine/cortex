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

#include "IECore/DPXImageWriter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"

#include "IECore/dpx.h"
#include "IECore/BoxOperators.h"

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

DPXImageWriter::DPXImageWriter(ObjectPtr image, const string &fileName) : 
		ImageWriter("DPXImageWriter", "Serializes images to Digital Picture eXchange 10-bit log image format")
{
	m_objectParameter->setValue( image );
	m_fileNameParameter->setTypedValue( fileName );
}

DPXImageWriter::~DPXImageWriter()
{
}

void DPXImageWriter::writeImage(vector<string> &names, ConstImagePrimitivePtr image, const Box2i &dw)
{
	// write the dpx in the standard 10bit log format
	std::ofstream out;
	out.open(fileName().c_str());
	if(!out.is_open())
	{
		throw Exception("could not open '" + fileName() + "' for writing");
	}
	
	// assume an 8-bit RGB image
	int width  = 1 + dw.max.x - dw.min.x;
	int height = 1 + dw.max.y - dw.min.y;
	
	//
	// FileInformation
	//
	
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


	// XPDS / swap bytes.
	// although unswapped bytes would probably be much faster, it doesn't seem to be
	// too common and one wonders if many applications detect and use it.  our DPX reader
	// expects to swap, so let's stay with that.
	/// \todo generalize and handle the non-byteswap case
	fi.magic = 0x58504453;

	// compute data offsets
	fi.gen_hdr_size = sizeof(fi) + sizeof(ii) + sizeof(ioi);
	fi.gen_hdr_size = reverseBytes(fi.gen_hdr_size);

	fi.ind_hdr_size = sizeof(mpf) + sizeof(th);
	fi.ind_hdr_size = reverseBytes(fi.ind_hdr_size);
	
	int header_size = sizeof(fi) + sizeof(ii) + sizeof(ioi) + sizeof(mpf) + sizeof(th);
	fi.image_data_offset = header_size;
	fi.image_data_offset = reverseBytes(fi.image_data_offset);
	
	strcpy((char *) fi.vers, "V2.0");
	strcpy((char *) fi.file_name, "image-engine.dpx");

	// compute the current date and time
	time_t t;
	time(&t);
	struct tm gmt;
	localtime_r(&t, &gmt);

	sprintf((char *) fi.create_time, "%04d:%02d:%02d:%02d:%02d:%02d:PST",
			  1900 + gmt.tm_year, gmt.tm_mon, gmt.tm_mday,
			  gmt.tm_hour, gmt.tm_min, gmt.tm_sec);
	sprintf((char *) fi.creator, "image engine vfx for film");
	sprintf((char *) fi.project, "IECore");
	sprintf((char *) fi.copyright, "image engine vfx for film");
	
	
	//
	// ImageInformation;
	//
	ii.orientation = 0;    // left-to-right, top-to-bottom
	ii.element_number = 1;
	ii.pixels_per_line = width;
	ii.lines_per_image_ele = height;

 	// reverse byte ordering
 	ii.element_number      = reverseBytes(ii.element_number);
 	ii.pixels_per_line     = reverseBytes(ii.pixels_per_line);
 	ii.lines_per_image_ele = reverseBytes(ii.lines_per_image_ele);

 	for(int c = 0; c < 8; ++c) {
		
		DPXImageInformation::_image_element &ie = ii.image_element[c];
		ie.data_sign = 0;

		ie.ref_low_data = 0;
		ie.ref_low_quantity = 0.0;
		ie.ref_high_data = 1023;
		ie.ref_high_quantity = 2.046;

		// swap
		ie.ref_low_data = reverseBytes(ie.ref_low_data);
		ie.ref_low_quantity = reverseBytes(ie.ref_low_quantity);
		ie.ref_high_data = reverseBytes(ie.ref_high_data);
		ie.ref_high_quantity = reverseBytes(ie.ref_high_quantity);
		
		
		ie.transfer = 1;
		ie.packing = 256;
		ie.bit_size = 10;
		ie.descriptor = 50;

		ie.data_offset = fi.image_data_offset;
 	}
	
	//
	// ImageOrientation
	//
	ioi.x_offset = 0;                  // could be dataWindow min.x
	ioi.y_offset = 0;                  // could be dataWindow min.y
	// other items left out for now

	// write the header
	
	// compute total file size
	int image_data_size = 4 * width * height;
	fi.file_size = header_size + image_data_size;
	fi.file_size = reverseBytes(fi.file_size);
	
	out.write(reinterpret_cast<char *>(&fi),  sizeof(fi));
	out.write(reinterpret_cast<char *>(&ii),  sizeof(ii));
	out.write(reinterpret_cast<char *>(&ioi), sizeof(ioi));
	out.write(reinterpret_cast<char *>(&mpf), sizeof(mpf));
	out.write(reinterpret_cast<char *>(&th),  sizeof(th));
	
	// write the data
	unsigned int *image_buffer = new unsigned int[width * height]();
	
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
			cerr << "warning: channel '" << *i << "' not encoded by DPXImageWriter" << endl;
			++i;
			continue;
		}
		
		const char *name = (*i).c_str();

		int offset = *i == "R" ? 0 : *i == "G" ? 1 : 2;
		int bpp = 10;
		unsigned int shift = (32 - bpp) - (offset*bpp);
		
		// get the image channel
		DataPtr channelp = image->variables.find(name)->second.data;
		
		switch(channelp->typeId())
		{
			
		case FloatVectorDataTypeId:
		{
			vector<float> channel = static_pointer_cast<FloatVectorData>(channelp)->readable();

			// convert the linear float value to 10-bit log
			for(int i = 0; i < width*height; ++i) {

				where = lower_bound(range.begin(), range.end(), channel[i]);
				unsigned int log_value = distance(range.begin(), where);
				image_buffer[i] |= log_value << shift;
			}
		}
		break;
	
		case UIntVectorDataTypeId:
		{
			delete [] image_buffer;
			throw Exception("DPXImageWriter: no unsigned int input channel supported for write");
		}
		break;
			
		case HalfVectorDataTypeId:
		{
			vector<half> channel = static_pointer_cast<HalfVectorData>(channelp)->readable();
			
			// convert the linear half value to 10-bit log
			for(int i = 0; i < width*height; ++i)
			{
				where = lower_bound(range.begin(), range.end(), channel[i]);
				unsigned int log_value = distance(range.begin(), where);
				image_buffer[i] |= log_value << shift;
			}
		}
		break;
			
		default:
			throw "invalid data type for DPX writer, channel type is: " +
				Object::typeNameFromTypeId(channelp->typeId());
		}
		
		++i;
	}
	
	// write the buffer
	for(int i = 0; i < width*height; ++i)
	{
		image_buffer[i] = reverseBytes(image_buffer[i]);
		out.write((const char *) (&image_buffer[i]), sizeof(unsigned int));
	}

	delete [] image_buffer;	
}
