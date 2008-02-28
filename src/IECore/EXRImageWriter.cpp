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

#include "IECore/EXRImageWriter.h"
#include "IECore/MessageHandler.h"
#include "IECore/VectorTypedData.h"
#include "IECore/ByteOrder.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/FileNameParameter.h"
#include "IECore/BoxOps.h"

#include "boost/format.hpp"

#include <fstream>

using namespace IECore;

using std::string;
using std::vector;

using Imath::Box2i;
using namespace Imf;

const Writer::WriterDescription<EXRImageWriter> EXRImageWriter::m_writerDescription("exr");

EXRImageWriter::EXRImageWriter()
		: ImageWriter("EXRImageWriter", "Serializes images to the OpenEXR HDR image format")
{
}

EXRImageWriter::EXRImageWriter(ObjectPtr image, const string &fileName)
		: ImageWriter("EXRImageWriter", "Serializes images to the OpenEXR HDR image format" )
{
	assert( m_objectParameter );
	assert( m_fileNameParameter );

	m_objectParameter->setValue( image );
	m_fileNameParameter->setTypedValue( fileName );
}

// \todo "names" should be const
void EXRImageWriter::writeImage(vector<string> &names, ConstImagePrimitivePtr image, const Box2i &dataWindow)
{
	assert( image );

	// create the header
	int width  = 1 + boxSize( dataWindow ).x;
	int height = 1 + boxSize( dataWindow ).y;

	try
	{
		/// \todo Add parameters for compression, etc
		Header header(width, height, 1, Imath::V2f(0.0, 0.0), 1, INCREASING_Y, PIZ_COMPRESSION);
		header.dataWindow() = dataWindow;
		header.displayWindow() = image->getDisplayWindow();

		// create the framebuffer
		FrameBuffer fb;

		// add the channels into the header with the appropriate types
		for (vector<string>::const_iterator i = names.begin(); i != names.end(); ++i)
		{
			const char *name = (*i).c_str();

			// get the image channel
			PrimitiveVariableMap::const_iterator pit = image->variables.find(name);
			if ( pit == image->variables.end() )
			{
				throw IOException( ( boost::format("EXRImageWriter: Could not find image channel \"%s\"") % name ).str() );
			}

			ConstDataPtr channelData = pit->second.data;
			if (!channelData)
			{
				throw IOException( ( boost::format("EXRImageWriter: Channel \"%s\" has no data") % name ).str() );
			}

			switch (channelData->typeId())
			{
			case FloatVectorDataTypeId:
				writeTypedChannel<float>(name, image, dataWindow,
				                         boost::static_pointer_cast<const FloatVectorData>(channelData)->readable(),
				                         FLOAT, header, fb);
				break;

			case UIntVectorDataTypeId:
				writeTypedChannel<unsigned int>(name, image, dataWindow,
				                                boost::static_pointer_cast<const UIntVectorData>(channelData)->readable(),
				                                UINT, header, fb);
				break;

			case HalfVectorDataTypeId:
				writeTypedChannel<half>(name, image, dataWindow,
				                        boost::static_pointer_cast<const HalfVectorData>(channelData)->readable(),
				                        HALF, header, fb);
				break;

			default:
				throw IOException( ( boost::format("EXRImageWriter: Invalid data type \"%s\" for channel \"%s\"") % channelData->typeName() % name ).str() );
			}
		}

		// create the output file, write, implicitly close
		OutputFile out(fileName().c_str(), header);

		out.setFrameBuffer(fb);
		out.writePixels(height);
	}
	catch ( Exception &e )
	{
		throw;
	}
	catch ( std::exception &e )
	{
		throw IOException( ( boost::format("EXRImageWriter: %s") % e.what() ).str() );
	}
	catch ( ... )
	{
		throw IOException( "EXRImageWriter: Unexpected error" );
	}

}

template<typename T>
void EXRImageWriter::writeTypedChannel(const char *name, ConstImagePrimitivePtr image, const Box2i &dataWindow,
                                       const vector<T> &channel, const Imf::PixelType pixelType, Header &header, FrameBuffer &fb)
{
	assert( name );

	/// \todo Remove this unused parameter
	(void) image;

	int width = 1 + dataWindow.max.x - dataWindow.min.x;

	// update the header
	header.channels().insert( name, Channel(pixelType) );

	// update the framebuffer
	char *offset = (char *) (&channel[0] - (dataWindow.min.x + width * dataWindow.min.y));
	fb.insert(name, Slice(pixelType, offset, sizeof(T), sizeof(T) * width));
}
