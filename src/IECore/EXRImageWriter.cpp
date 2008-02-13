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

#include "IECore/EXRImageWriter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"

#include "boost/format.hpp"

#include <fstream>

using namespace IECore;
using namespace std;
using namespace boost;

// ILM
using namespace Imath;
using namespace Imf;

const Writer::WriterDescription<EXRImageWriter> EXRImageWriter::m_writerDescription("exr");

EXRImageWriter::EXRImageWriter()
		: ImageWriter("EXRImageWriter", "Serializes images to the OpenEXR HDR image format")
{
}

EXRImageWriter::EXRImageWriter(ObjectPtr image, const string & fileName)
		: ImageWriter("EXRImageWriter", "Serializes images to the OpenEXR HDR image format" )
{
	m_objectParameter->setValue( image );
	m_fileNameParameter->setTypedValue( fileName );
}

void EXRImageWriter::writeImage(vector<string> & names, ConstImagePrimitivePtr image, const Box2i & dw)
{

	// create the header
	int width  = 1 + dw.max.x - dw.min.x;
	int height = 1 + dw.max.y - dw.min.y;

	Header header(width, height, 1, Imath::V2f(0.0, 0.0), 1, INCREASING_Y, PIZ_COMPRESSION);
	header.dataWindow() = dw;
	header.displayWindow() = image->getDisplayWindow();

	// create the framebuffer
	FrameBuffer fb;

	// add the channels into the header with the appropriate types
	vector<string>::const_iterator i = names.begin();
	while (i != names.end())
	{

		const char *name = (*i).c_str();

		// get the image channel
		DataPtr channelp = image->variables.find(name)->second.data;

		switch (channelp->typeId())
		{

		case FloatVectorDataTypeId:
			writeTypedChannel<float>(name, image, dw,
			                         static_pointer_cast<FloatVectorData>(channelp)->readable(),
			                         FLOAT, header, fb);
			break;

		case UIntVectorDataTypeId:
			writeTypedChannel<unsigned int>(name, image, dw,
			                                static_pointer_cast<UIntVectorData>(channelp)->readable(),
			                                UINT, header, fb);
			break;

		case HalfVectorDataTypeId:
			writeTypedChannel<half>(name, image, dw,
			                        static_pointer_cast<HalfVectorData>(channelp)->readable(),
			                        HALF, header, fb);
			break;

		default:
			throw "invalid data type for EXR writer, channel type is: " +
			Object::typeNameFromTypeId(channelp->typeId());
			break;
		}

		++i;
	}

	// create the output file, write, implicitly close
	OutputFile out(fileName().c_str(), header);

	out.setFrameBuffer(fb);
	out.writePixels(height);

}

template<typename T>
void EXRImageWriter::writeTypedChannel(const char * name, ConstImagePrimitivePtr image, const Box2i & dw,
                                       const vector<T> & channel, const Imf::PixelType TYPE, Header & header, FrameBuffer & fb)
{
	int width  = 1 + dw.max.x - dw.min.x;

	// update the header
	header.channels().insert(name, Channel(TYPE));

	const int size = sizeof(T);

	// update the framebuffer
	char *offset = (char *) (&channel[0] - (dw.min.x + width * dw.min.y));
	fb.insert(name, Slice(TYPE, offset, size, size * width));
}
