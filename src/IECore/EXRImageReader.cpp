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

#include "IECore/EXRImageReader.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/MessageHandler.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/BoxOperators.h"

#include "boost/format.hpp"

#include "OpenEXR/ImfInputFile.h"
#include "OpenEXR/ImfChannelList.h"
#include "OpenEXR/Iex.h"
#include "OpenEXR/ImfTestFile.h"

#include <algorithm>

#include <fstream>
#include <iostream>
#include <cassert>

using namespace IECore;
using namespace boost;

// ILM ns
using namespace Imath;
using namespace Imf;

using namespace std;

const Reader::ReaderDescription<EXRImageReader> EXRImageReader::m_readerDescription("exr");

EXRImageReader::EXRImageReader() :
		ImageReader( "EXRImageReader", "Reads ILM OpenEXR file formats" ),
		m_inputFile(0)
{
}

EXRImageReader::EXRImageReader(const string & fileName) :
		ImageReader( "EXRImageReader", "Reads ILM OpenEXR file formats" ),
		m_inputFile(0)
{
	m_fileNameParameter->setTypedValue(fileName);
}

EXRImageReader::~EXRImageReader()
{
	delete m_inputFile;
}

bool EXRImageReader::canRead(const string &fileName)
{
	// attempt to open the file
	ifstream in(fileName.c_str());
	if (!in.is_open())
	{
		return false;
	}

	// use the IlmImf library test function for the magic number
	// see also bool IlmImf::InputFile::isComplete(), which will return true iff the exr has all
	// pixel data written out
	return isOpenExrFile(fileName.c_str());
}

bool EXRImageReader::isComplete() const
{
	return m_inputFile && m_inputFile->isComplete();
}

void EXRImageReader::channelNames(vector<string> & names)
{
	names.clear();
	if (!open())
	{
		return;
	}

	// query the channels
	const ChannelList &channels = m_header.channels();

	// copy in the channel names
	for (ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i)
	{
		names.push_back(i.name());
	}
}

void EXRImageReader::readChannel(string name, ImagePrimitivePtr image, const Box2i & dataWindow)
{
	if (!open())
	{
		return;
	}

	// compute the data window to read
	Box2i dw = dataWindow.isEmpty() ? m_header.dataWindow() : dataWindow;
	image->setDataWindow(dw);

	// honor the display window from the header
	if (image->getDisplayWindow().isEmpty())
	{
		image->setDisplayWindow(m_header.displayWindow());
	}

	const ChannelList &channels = m_header.channels();
	const Channel &channel = channels[name.c_str()];

	// get the channel from our Image, compute the size of the channel type
	// see ImfPixelType.h
	switch (channel.type)
	{

	case UINT:  // unsigned int (32 bit)
		readTypedChannel<unsigned int>(name, image, dw, channel);
		break;

	case HALF:  // half (16 bit floating point)
		readTypedChannel<half>        (name, image, dw, channel);
		break;

	case FLOAT: // float (32 bit floating point)
		readTypedChannel<float>       (name, image, dw, channel);
		break;

	default:
		throw Exception(string("EXRImageReader::readChannel: channel '" + name + "' is of unknown type"));
	}
}

template <typename T>
void EXRImageReader::readTypedChannel(string name, ImagePrimitivePtr image,
                                      const Box2i & dataWindow, const Channel & channel)
{
	intrusive_ptr<TypedData<vector<T> > > image_channel = image->template createChannel<T>(name);
	vector<T> &ic = image_channel->writable();

	// compute the size of the sample values, the stride, and the width
	int size = sizeof(T);
	int width = 1 + dataWindow.max.x - dataWindow.min.x;

	Box2i dw = m_header.dataWindow();
	int datawidth = 1 + dw.max.x - dw.min.x;

	// copy cropped scanlines into the buffer
	vector<T> scanline;
	scanline.resize(datawidth);

	// determine the image data window
	Box2i idw = dataWindow.isEmpty() ? dw : dataWindow;
	image->setDataWindow(idw);
	image->setDisplayWindow(idw);

	// compute read box
	Box2i readbox = intersection(dw, idw);

	// read as scanlines

	// x-shift for the ImagePrimitive array
	int dx = readbox.min.x - dataWindow.min.x;

	// y-shift for the ImagePrimitive array
	int cl = readbox.min.y - dataWindow.min.y;

	int readWidth = 1 + readbox.max.x - readbox.min.x;

	for (int sl = readbox.min.y; sl <= readbox.max.y; ++sl, ++cl)
	{

		FrameBuffer fb;

		char * scanlinestart = (char *) (&scanline[0] - (dw.min.x + sl*datawidth));
		fb.insert(name.c_str(), Slice(channel.type, scanlinestart, size, size * datawidth));

		// read the line from the input file
		m_inputFile->setFrameBuffer(fb);
		m_inputFile->readPixels(sl, sl);

		// crop the scanline horizontally
		unsigned int ini = cl * width + dx;

		// i varies over the intersection of the datawindow x-range and the image x-range
		for (int i = 0; i < readWidth; ++i, ++ini)
		{
			ic[ini] = scanline[i];
		}
	}
}

bool EXRImageReader::open()
{
	bool valid = true;

	if (!m_inputFile || fileName() != m_inputFile->fileName())
	{
		delete m_inputFile;

		valid = isOpenExrFile(fileName().c_str());
		if (valid)
		{
			m_inputFile = new Imf::InputFile(fileName().c_str());
			m_header = m_inputFile->header();
		}
	}

	//return true; // not good enough but matches previous behaviour
	return valid;
}
