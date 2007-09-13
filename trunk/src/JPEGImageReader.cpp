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
//        other contributors to this software may be used to endorse or
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

#include "IECore/JPEGImageReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"

#include "boost/format.hpp"

extern "C"
{
#include "jpeglib.h"
}

#include <algorithm>
#include <cassert>
#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

const Reader::ReaderDescription <JPEGImageReader>
JPEGImageReader::m_readerDescription ("jpeg jpg");

JPEGImageReader::JPEGImageReader() : 
	ImageReader( "JPEGImageReader", "Reads Joint Photographic Experts Group (JPEG) files" ),
	m_buffer(0)
{
}

JPEGImageReader::JPEGImageReader(const string & fileName) : 
	ImageReader( "JPEGImageReader", "Reads Joint Photographic Experts Group (JPEG) files" ),
	m_buffer(0)
{
	m_fileNameParameter->setTypedValue( fileName );
}

JPEGImageReader::~JPEGImageReader()
{
	delete [] m_buffer;
}

bool JPEGImageReader::canRead(const string & fileName)
{
	// attempt to open the file
	ifstream in(fileName.c_str());
	if(!in.is_open())
	{
		return false;
	}

	// check the magic number of the input file
	// a jpeg should have 0xffd8ffe0 from offset 0
	unsigned int magic;
	in.seekg(0, ios_base::beg);
	in.read((char *) &magic, sizeof(unsigned int));
	return magic == 0xe0ffd8ff || magic == 0xffd8ffe0;
}

void JPEGImageReader::channelNames(vector<string> & names)
{
	names.clear();
	
	// form channel names - hardcoded for now 'til i learn more about JPEG
	names.push_back("R");
	names.push_back("G");
	names.push_back("B");
}

void JPEGImageReader::readChannel(string name, ImagePrimitivePtr image,
											 const Box2i & dataWindow)
{
	
	if(!open())
	{
		return;
	}

	int width = m_bufferWidth;
	int datawidth = m_bufferWidth;
	int height = m_bufferHeight;

	// compute the data window
	Box2i dw;
	if(dataWindow.isEmpty())
	{
		// compute image box
		dw.min.x = 0;
		dw.min.y = 0;
		dw.max.x = width - 1;
		dw.max.y = height - 1;
	}
	else
	{
		dw = dataWindow;
	}

	image->setDataWindow(dw);
	image->setDisplayWindow(dw);
	
	int boffset = name == "R" ? 0 : name == "G" ? 1 : 2;

	// copy in the corresponding channel
	vector<half> &ic = image->createChannel<half>(name)->writable();

	int low_x = max(dw.min.x, 0);
	int high_x = min(dw.max.x, width-1);

	width = 1 + dw.max.x - dw.min.x;
	
	// adjust height bounds so that they intersect the image's data window
	int low_y = max(dw.min.y, 0);
	int high_y = min(dw.max.y, height-1);

	// adjust cl for the difference between min.y, low_y
	int cl = low_y - dw.min.y;
	int dx = low_x - dw.min.x;
	
	unsigned int ini = 0;
	
  	for(int sl = low_y; sl <= high_y; ++sl, ++cl) {

		ini = cl * width + dx;
		for(int i = low_x; i <= high_x; ++i, ++ini)
 		{
			ic[ini] = m_buffer[3*(sl*datawidth + i) + boffset] / 255.0f;
		}
	}
		
}

bool JPEGImageReader::open()
{
	if(fileName() != m_bufferFileName)
	{
		m_bufferFileName = fileName();
	
		delete [] m_buffer;
		m_buffer = 0;
		
		// open the file
		FILE *inFile = fopen(fileName().c_str(), "rb");
		if(!inFile)
		{
			return false;
		}

		// open the image
		struct jpeg_decompress_struct cinfo;
		struct jpeg_error_mgr jerr;
		
		cinfo.err = jpeg_std_error(&jerr);

		// initialize decompressor
		jpeg_create_decompress(&cinfo);
		jpeg_stdio_src(&cinfo, inFile);
		jpeg_read_header(&cinfo, TRUE);
		
		// start decompression
		jpeg_start_decompress(&cinfo);

		// create buffer
		int row_stride = cinfo.output_width * cinfo.output_components;
		m_buffer = new unsigned char[row_stride * cinfo.output_height]();
		unsigned char *row_pointer[1];
		m_bufferWidth = cinfo.output_width;
		m_bufferHeight = cinfo.output_height;

		// read scanlines one at a time.
		// \todo: optimize this, probably based on image dimensions
		while (cinfo.output_scanline < cinfo.output_height)
		{
			row_pointer[0] = m_buffer + row_stride * cinfo.output_scanline;
			jpeg_read_scanlines(&cinfo, row_pointer, 1);
		}

		// finish decompression
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);

		fclose(inFile);		
	}
	
	return m_buffer;
}
