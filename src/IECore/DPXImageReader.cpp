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

#include "IECore/DPXImageReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"

#include "IECore/private/dpx.h"

#include "IECore/BoxOperators.h"

#include "boost/format.hpp"

#include <algorithm>

#include <fstream>
#include <iostream>
#include <cassert>

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

const Reader::ReaderDescription<DPXImageReader> DPXImageReader::m_readerDescription("dpx");

DPXImageReader::DPXImageReader() : 
		ImageReader("DPXImageReader", "Reads Digital Picture eXchange (DPX) files."),
		m_buffer(0)
{
}

DPXImageReader::DPXImageReader(const string & fileName) : 
		ImageReader("DPXImageReader", "Reads Digital Picture eXchange (DPX) files."),
		m_buffer(0)
{
	m_fileNameParameter->setTypedValue(fileName);
}

DPXImageReader::~DPXImageReader()
{
	delete [] m_buffer;
}

// partial validity check: assert that the file begins with the DPX magic number
bool DPXImageReader::canRead(const string & fileName)
{
	// attempt to open the file
	ifstream in(fileName.c_str());
	if(!in.is_open())
	{
		return false;
	}

	// seek to start
	in.seekg(0, ios_base::beg);
	
	// check magic number
	unsigned int magic;
	in.read((char *) &magic, sizeof(unsigned int));
	return magic == 0x53445058 || magic == 0x58504453;
}

void DPXImageReader::channelNames(vector<string> & names)
{
	names.clear();
	
	/// \todo read the channel names from the DPX header
	// form channel names - hardcoded for now, assume RGB (in 10bit log)
	names.push_back("R");
	names.push_back("G");
	names.push_back("B");
}

/// \todo
/// we assume here DPX coding in the 'typical' configuration (output by film dumps, nuke, etc).
/// this is RGB 10bit log for film, pixel-interlaced data.  we convert this to a linear 16-bit (half)
/// format in the ImagePrimitive.
void DPXImageReader::readChannel(string name, ImagePrimitivePtr image, const Box2i & dataWindow)
{	
	if(!open())
	{
		return;
	}

	// compute image extents box
	Box2i dw;
	dw.min.x = 0;
	dw.min.y = 0;
	dw.max.x = m_bufferWidth-1;
	dw.max.y = m_bufferHeight-1;

	// determine the image data window
	Box2i idw = dataWindow.isEmpty() ? dw : dataWindow;
	image->setDataWindow(idw);
	image->setDisplayWindow(idw);

	// compute read box
	Box2i readbox = intersection(dw, idw);
	
	// kinda useless here, we have implicitly assumed log 10 bit in the surrounding code
	int bpp = 10;
	
	// figure out the offset into the bitstream for the given channel
	int boffset = name == "R" ? 0 : name == "G" ? 1 : 2;
	
	// form the mask for this channel
	unsigned int mask = 0;
	for(int pi = 0; pi < bpp; ++pi) { mask = 1 + (mask << 1); }
	mask <<= ((32 - bpp) - boffset * bpp);

	// copy in the corresponding channel
	// (convert to half; this datatype has enough room/structure to hold the equivalent
	//  of the 10bit log values in linear space)
 	vector<half> &ic = image->createChannel<half>(name)->writable();
 	unsigned short cv;

	// compute distance from the read box origin
	V2i d = readbox.min - dw.min;
	
	// read in the buffer
	for(int y = readbox.min.y; y <= readbox.max.y; ++y)
	{
		for(int x = readbox.min.x; x <= readbox.max.x; ++x)
		{
			// i is the index of the pixel on the output image channel
			int i = (y - idw.min.y) * boxwidth(idw) + (x - idw.min.x);

			// di is the index of the pixel in the input image buffer
			int di = (y - d.y) * boxwidth(dw) + (x - d.x);
			
			// get the cell.  for efficiency, we may wish to swap the bytes in the buffer reader
 			unsigned int cell = reverseBytes(m_buffer[di]);
			
 			// assume we have 10bit log, two wasted bits aligning to 32 longword
 			cv = (unsigned short) ((mask & cell) >> (2 + (2 - boffset)*bpp));
			
			// convert to a linear floating-point value
 			ic[i] = m_LUT[cv];
		}
	}
}

bool DPXImageReader::open()
{
	if(m_bufferFileName != fileName())
	{
		m_bufferFileName = fileName();
		delete [] m_buffer;
		m_buffer = 0;

		ifstream in(m_bufferFileName.c_str());
		
		// open the file
		if(!in.is_open())
		{
			throw Exception("could not open " + fileName());
		}
		
		// read the header
		DPXFileInformation fi;
		in.read(reinterpret_cast<char*>(&fi), sizeof(fi));

		// read the image information
		DPXImageInformation ii;
		in.read(reinterpret_cast<char*>(&ii), sizeof(ii));
		
		// read the data format information
		DPXImageOrientation io;
		in.read(reinterpret_cast<char*>(&io), sizeof(io));

		// make sure we have a valid file
		// 'proper' endianness should have 802A5FD7
		if(fi.magic != 0x53445058 && fi.magic != 0x58504453)
		{
			throw Exception("invalid DPX magic number");
		}
		
		// reverse some bytes on the pertinent fields
		fi.image_data_offset   = reverseBytes(fi.image_data_offset);
		ii.element_number      = reverseBytes(ii.element_number);
		ii.pixels_per_line     = reverseBytes(ii.pixels_per_line);
		ii.lines_per_image_ele = reverseBytes(ii.lines_per_image_ele);
		
		//
		// image information
		//

		// dataWindow: use the convention that the image begins at the origin,
		// and that it extends to (width, height)
		Box2i dw;
		dw.min.x = 0;
		dw.min.y = 0;

		dw.max.x = ii.pixels_per_line - 1;
		dw.max.y = ii.lines_per_image_ele - 1;

		m_bufferWidth = ii.pixels_per_line;
		m_bufferHeight = ii.lines_per_image_ele;

		/// \todo should verify that it is a code 50 RGB single image dpx.
		
		// seek to the image data offset
		in.seekg(fi.image_data_offset, ios_base::beg);		
		
		// 
		// build a LUT based on the transfer enum.  we handle only the
		// log/printing density and linear
		//

// 		// ugg, assume that there is only one image and it is specified in ii.image_element[0]
// 		if(ii.image_element[0].transfer == 2)
// 		{
// 			// i have noticed that our lone example of linear DPX encoded files has specified
// 			// ref_low_data, ref_high_data points.  i am under the impression that these specify
// 			// the black and white points for linear transfer.
// 			//
// 			// for example, the SNO plates have values 64 and 965.  however, not having 0 and 1023 
// 			// as points is just wasteful.
// 			//
// 			// here instead we will mimic shake and nuke and simply use a ramp over 0 to 1023

// 			// build a different lut for linear transfer
// 			int ref_black_val = 0;       // ii.image_element[0].ref_low_data;
// 			int ref_white_val = 1023;    // ii.image_element[0].ref_high_data;
// 			float spread = ref_white_val - ref_black_val;
// 			for(int i = 0; i < 1024; ++i)
// 			{
// 				float cvf = i <= ref_black_val ? 0.0f : i >= ref_white_val ? 1.0 : (1.0*i - ref_black_val) / spread;
// 				m_LUT[i] = cvf;
// 			}
// 		}
// 		else
// 		{

		// build a LUT for 10bit log -> 16bit linear conversion
		
		// get reference white
		double film_gamma = 0.6;
		int ref_black_val = 95;
		int ref_white_val = 685;
		double quantization_step = 0.002;
		double ref_mult = quantization_step / film_gamma;
		
		// compute black offset
		double black_offset = pow(10.0, (ref_black_val - ref_white_val) * ref_mult);
		
		// standard lut
		for(int i = 0; i < 1024; ++i)
		{
			float cvf = (pow(10.0, (i - ref_white_val) * ref_mult) - black_offset) / (1.0 - black_offset);
			m_LUT[i] = cvf;
		}

		//
		// read in the buffer
		//
		
		// determine the size
		int buffersize = 3 * m_bufferWidth * m_bufferHeight;

		// assume data is interlaced, just read the whole thing,
		// then stripe off the channel
		m_buffer = new unsigned int[buffersize];

		// read the data into the buffer
		in.read((char *) m_buffer, sizeof(unsigned int) * buffersize);
	}
	
	return m_buffer;
}
