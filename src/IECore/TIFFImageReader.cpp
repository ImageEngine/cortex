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

#include "IECore/TIFFImageReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"

#include "boost/static_assert.hpp"
#include "boost/format.hpp"

#include "IECore/BoxOperators.h"

#include "tiffio.h"

#include <algorithm>

#include <fstream>
#include <iostream>
#include <cassert>

using namespace IECore;
using namespace boost;
using namespace Imath;
using namespace std;

const Reader::ReaderDescription<TIFFImageReader> TIFFImageReader::m_readerDescription("tiff tif");

TIFFImageReader::TIFFImageReader()
		:	ImageReader("TIFFImageReader", "Reads Tagged Image File Format (TIFF) files" ),
		m_tiffImage(0), m_buffer(0)
{
}

TIFFImageReader::TIFFImageReader(const string & fileName)
		:	ImageReader("TIFFImageReader", "Reads Tagged Image File Format (TIFF) files" ),
		m_tiffImage(0), m_buffer(0)
{
	m_fileNameParameter->setTypedValue(fileName);
}

TIFFImageReader::~TIFFImageReader()
{
	if (m_tiffImage)
	{
		TIFFClose(m_tiffImage);
	}
	delete [] m_buffer;
}

bool TIFFImageReader::canRead(const string & fileName)
{
	// attempt to open the file
	ifstream in(fileName.c_str());
	if (!in.is_open())
	{
		return false;
	}

	// check the magic number of the input file
	// a tiff should have 0x49492a00 / 0x002a4949 from offset 0

	// attempt to open the file
	in.seekg(0, ios_base::beg);

	// check magic number
	unsigned int magic;
	in.read((char *) &magic, sizeof(unsigned int));
	
	/// \todo Why the 3 variations here? Surely only 2 are necessary?
	return magic == 0x002a4949 || magic == 0x49492a00 || magic == 0x2a004d4d;
}

void TIFFImageReader::channelNames(vector<string> & names)
{
	names.clear();

	if (!open())
	{
		return;
	}

	uint16 spp;
	TIFFGetField(m_tiffImage, TIFFTAG_SAMPLESPERPIXEL, &spp);

	/// \todo
	// form channel names - hardcoded for now 'til i learn more about TIFF
	names.push_back("R");
	names.push_back("G");
	names.push_back("B");
	if (spp == 4)
	{
		names.push_back("A");
	}
}

void TIFFImageReader::readChannel(string name, ImagePrimitivePtr image, const Box2i & dataWindow)
{
	if (!open())
	{
		return;
	}

	// get the TIFF fields
	uint16 photo, bps, spp, fillorder, sampleformat;
	uint32 width, height;

	TIFFGetField(m_tiffImage, TIFFTAG_BITSPERSAMPLE, &bps);
	TIFFGetField(m_tiffImage, TIFFTAG_PHOTOMETRIC, &photo);
	TIFFGetField(m_tiffImage, TIFFTAG_SAMPLESPERPIXEL, &spp);
	TIFFGetField(m_tiffImage, TIFFTAG_FILLORDER, &fillorder);
	TIFFGetField(m_tiffImage, TIFFTAG_SAMPLEFORMAT, &sampleformat);

	// we handle here 8, 16, and 32 bpp with RGB channels in integer space, and now float
	TIFFGetField(m_tiffImage, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(m_tiffImage, TIFFTAG_IMAGELENGTH, &height);

	// compute the data window
	Box2i dw;
	dw.min.x = 0;
	dw.min.y = 0;
	dw.max.x = width - 1;
	dw.max.y = height - 1;

	// determine the image data window
	Box2i idw = dataWindow.isEmpty() ? dw : dataWindow;
	image->setDataWindow(idw);
	image->setDisplayWindow(idw);

	// compute read box
	Box2i readbox = intersection(dw, idw);

	if (!m_buffer)
	{
		read_buffer();
	}

	// compute offset to image
	int boffset = name == "R" ? 0 : name == "G" ? 1 : name == "B" ? 2 : 3;

	// we form 32 bit float image channel by normalizing the input integer range to [0.0, 1.0]
	double normalizer = 1.0 / ((1 << bps) - 1);

	// copy in the corresponding channel
	// ugg, perhaps we should move/duplicate the loop inside the switch cases for speed
	vector<float> & ic = image->createChannel<float>(name)->writable();

	// check for empty box
	if (readbox.isEmpty())
	{
		return;
	}

	// compute distance from the read box origin
	V2i d = readbox.min - dw.min;

	/// \todo: throw out an exception if the sample format field is mangled.  for now
	/// we shall assume that a non-floating point TIFF is of integer type
	if (sampleformat == SAMPLEFORMAT_IEEEFP)
	{
		// read in the buffer
		for (int y = readbox.min.y; y <= readbox.max.y; ++y)
		{
			for (int x = readbox.min.x; x <= readbox.max.x; ++x)
			{
				// i is the index of the pixel on the output image channel
				int i = (y - idw.min.y) * boxwidth(idw) + (x - idw.min.x);

				// di is the index of the pixel in the input image buffer
				int di = (y - d.y) * boxwidth(dw) + (x - d.x);

				ic[i] = (reinterpret_cast<float *>(m_buffer))[spp * di + boffset];
			}
		}
	}
	else
	{
		for (int y = readbox.min.y; y <= readbox.max.y; ++y)
		{
			for (int x = readbox.min.x; x <= readbox.max.x; ++x)
			{
				// i is the index of the pixel on the output image channel
				int i = (y - idw.min.y) * boxwidth(idw) + (x - idw.min.x);

				// di is the index of the pixel in the input image buffer
				int di = (y - d.y) * boxwidth(dw) + (x - d.x);

				switch (bps)
				{

				case 8:
				{
					// cast to unsigned byte, divide
					ic[i] = normalizer * m_buffer[spp * di + boffset];
				}
				break;

				case 16:
				{
					//// \todo should probably be using uint16_t here instead
					BOOST_STATIC_ASSERT( sizeof( unsigned short ) == 2 );
					// cast to short, divide
					unsigned short v = *((unsigned short *) m_buffer + spp * di + boffset);
					ic[i] = normalizer * v;
				}
				break;

				case 32:
				{
					//// \todo should probably be using uint32_t here instead
					BOOST_STATIC_ASSERT( sizeof( unsigned int ) == 4 );
					// cast to int, divide
					unsigned int v = *((unsigned int *) m_buffer + spp * di + boffset);
					ic[i] = normalizer * v;
				}
				break;

				default:
					throw Exception("unhandled TIFF bit-depth: " + bps);

				}
			}
		}
	}
}

/// read in the data to a buffer
void TIFFImageReader::read_buffer()
{
	uint16 spp, bps;
	uint32 width, height;
	tsize_t stripSize;
	long imageOffset, result;
	int stripMax, stripCount;

	TIFFGetField(m_tiffImage, TIFFTAG_BITSPERSAMPLE, &bps);
	TIFFGetField(m_tiffImage, TIFFTAG_SAMPLESPERPIXEL, &spp);
	TIFFGetField(m_tiffImage, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(m_tiffImage, TIFFTAG_IMAGELENGTH, &height);

	// get strip size
	stripSize = TIFFStripSize(m_tiffImage);
	stripMax = TIFFNumberOfStrips(m_tiffImage);
	imageOffset = 0;

	/// \todo Is this assumption sound?
	// assume data is interlaced, just read the whole thing,
	// then stripe off the channel
	m_buffer = new unsigned char[(bps / 8) * spp * width * height]();

	// read the image
	for (stripCount = 0; stripCount < stripMax; stripCount++)
	{
		if ((result = TIFFReadEncodedStrip( m_tiffImage, stripCount, m_buffer + imageOffset, stripSize)) == -1)
		{
			throw Exception("TIFF read error on strip number " + stripCount);
		}

		imageOffset += result;
	}
}

bool TIFFImageReader::open()
{
	if (!m_tiffImage || m_tiffImageFileName != fileName())
	{
		if (m_tiffImage)
		{
			TIFFClose(m_tiffImage);
		}
		m_tiffImage = TIFFOpen(fileName().c_str(), "r");
		m_tiffImageFileName = fileName();
		delete [] m_buffer;
		m_buffer = 0;
	}

	return m_tiffImage;
}
