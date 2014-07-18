//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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
#include "IECore/CompoundParameter.h"
#include "IECore/BoxOps.h"
#include "IECore/TimeCodeData.h"

#include "OpenEXR/ImfFloatAttribute.h"
#include "OpenEXR/ImfDoubleAttribute.h"
#include "OpenEXR/ImfIntAttribute.h"
#include "OpenEXR/ImfBoxAttribute.h"
#include "OpenEXR/ImfVecAttribute.h"
#include "OpenEXR/ImfMatrixAttribute.h"
#include "OpenEXR/ImfStringAttribute.h"
#include "OpenEXR/ImfTimeCodeAttribute.h"

#include "boost/format.hpp"

#include <fstream>

using namespace IECore;

using std::string;
using std::vector;

using Imath::Box2i;
using namespace Imf;

IE_CORE_DEFINERUNTIMETYPED( EXRImageWriter )

const Writer::WriterDescription<EXRImageWriter> EXRImageWriter::m_writerDescription("exr");

EXRImageWriter::EXRImageWriter()
		: ImageWriter( "Serializes images to the OpenEXR HDR image format")
{
	constructCommon();
}

EXRImageWriter::EXRImageWriter(ObjectPtr image, const string &fileName)
		: ImageWriter( "Serializes images to the OpenEXR HDR image format" )
{

	constructCommon();
	
	assert( m_objectParameter );
	assert( m_fileNameParameter );

	m_objectParameter->setValue( image );
	m_fileNameParameter->setTypedValue( fileName );
	
}

void EXRImageWriter::constructCommon()
{

	// compression parameter
	IntParameter::PresetsContainer compressionPresets;
	compressionPresets.push_back( IntParameter::Preset( "none", NO_COMPRESSION ) );
	compressionPresets.push_back( IntParameter::Preset( "rle", RLE_COMPRESSION ) );
	compressionPresets.push_back( IntParameter::Preset( "zips", ZIPS_COMPRESSION ) );
	compressionPresets.push_back( IntParameter::Preset( "zip", ZIP_COMPRESSION ) );
	compressionPresets.push_back( IntParameter::Preset( "piz", PIZ_COMPRESSION ) );
	compressionPresets.push_back( IntParameter::Preset( "pxr24", PXR24_COMPRESSION ) );
	compressionPresets.push_back( IntParameter::Preset( "b44", B44_COMPRESSION ) );
	compressionPresets.push_back( IntParameter::Preset( "b44a", B44A_COMPRESSION ) );

	IntParameterPtr compressionParameter = new IntParameter(
	        "compression",
	        "EXR compression method",
	        PIZ_COMPRESSION,
	        NO_COMPRESSION,
	        NUM_COMPRESSION_METHODS - 1,
	        compressionPresets,
	        true
	);

	parameters()->addParameter( compressionParameter );

}

std::string EXRImageWriter::destinationColorSpace() const
{
	return "linear";
}

IntParameter * EXRImageWriter::compressionParameter()
{
	return parameters()->parameter< IntParameter >( "compression" );
}

const IntParameter * EXRImageWriter::compressionParameter() const
{
	return parameters()->parameter< IntParameter >( "compression" );
}

static void blindDataToHeader( const CompoundData *blindData, Imf::Header &header, std::string prefix = "" )
{
	const CompoundDataMap &map = blindData->readable();
	for ( CompoundDataMap::const_iterator it = map.begin(); it != map.end(); it++ )
	{
		std::string thisName = (prefix.size() ? prefix + "." : std::string("") ) + it->first.value();

		switch ( it->second->typeId() )
		{
			case CompoundDataTypeId:

				blindDataToHeader( static_cast<const CompoundData *>( it->second.get() ), header, thisName );
				break;
			
			case IntDataTypeId:

				header.insert( thisName.c_str(), Imf::IntAttribute( boost::static_pointer_cast< const IntData >( it->second )->readable() ) );
				break;

			case FloatDataTypeId:

				header.insert( thisName.c_str(), Imf::FloatAttribute( boost::static_pointer_cast< FloatData >( it->second )->readable() ) );
				break;

			case DoubleDataTypeId:

				header.insert( thisName.c_str(), Imf::DoubleAttribute( boost::static_pointer_cast< DoubleData >( it->second )->readable() ) );
				break;

			case V2iDataTypeId:

				header.insert( thisName.c_str(), Imf::V2iAttribute( boost::static_pointer_cast< V2iData >( it->second )->readable() ) );
				break;

			case V2fDataTypeId:

				header.insert( thisName.c_str(), Imf::V2fAttribute( boost::static_pointer_cast< V2fData >( it->second )->readable() ) );
				break;

			case V3iDataTypeId:

				header.insert( thisName.c_str(), Imf::V3iAttribute( boost::static_pointer_cast< V3iData >( it->second )->readable() ) );
				break;

			case V3fDataTypeId:

				header.insert( thisName.c_str(), Imf::V3fAttribute( boost::static_pointer_cast< V3fData >( it->second )->readable() ) );
				break;

			case Box2iDataTypeId:

				header.insert( thisName.c_str(), Imf::Box2iAttribute( boost::static_pointer_cast< Box2iData >( it->second )->readable() ) );
				break;

			case Box2fDataTypeId:

				header.insert( thisName.c_str(), Imf::Box2fAttribute( boost::static_pointer_cast< Box2fData >( it->second )->readable() ) );
				break;

			case M33fDataTypeId:

				header.insert( thisName.c_str(), Imf::M33fAttribute( boost::static_pointer_cast< M33fData >( it->second )->readable() ) );
				break;

			case M44fDataTypeId:

				header.insert( thisName.c_str(), Imf::M44fAttribute( boost::static_pointer_cast< M44fData >( it->second )->readable() ) );
				break;

			case StringDataTypeId:

				header.insert( thisName.c_str(), Imf::StringAttribute( boost::static_pointer_cast< StringData >( it->second )->readable() ) );
				break;

			case TimeCodeDataTypeId:

				header.insert( thisName.c_str(), Imf::TimeCodeAttribute( boost::static_pointer_cast< TimeCodeData >( it->second )->readable() ) );
				break;

			default:
				// ignore all the other data types.
				break;
		}
	}
}

void EXRImageWriter::writeImage( const vector<string> &names, const ImagePrimitive * image, const Box2i &dataWindow) const
{
	assert( image );

	// create the header
	int width  = 1 + boxSize( dataWindow ).x;
	int height = 1 + boxSize( dataWindow ).y;

	try
	{
		Header header(width, height, 1, Imath::V2f(0.0, 0.0), 1, INCREASING_Y, 
			static_cast<Compression>(compressionParameter()->getNumericValue()) );
		blindDataToHeader( image->blindData(), header );
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

			const Data *channelData = pit->second.data.get();
			if (!channelData)
			{
				throw IOException( ( boost::format("EXRImageWriter: Channel \"%s\" has no data") % name ).str() );
			}

			switch (channelData->typeId())
			{
			case FloatVectorDataTypeId:
				writeTypedChannel<float>(name, dataWindow,
				                         static_cast<const FloatVectorData *>(channelData)->readable(),
				                         FLOAT, header, fb);
				break;

			case UIntVectorDataTypeId:
				writeTypedChannel<unsigned int>(name, dataWindow,
				                                static_cast<const UIntVectorData *>(channelData)->readable(),
				                                UINT, header, fb);
				break;

			case HalfVectorDataTypeId:
				writeTypedChannel<half>(name, dataWindow,
				                        static_cast<const HalfVectorData *>(channelData)->readable(),
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
void EXRImageWriter::writeTypedChannel(const char *name, const Box2i &dataWindow,
                                       const vector<T> &channel, const Imf::PixelType pixelType, Header &header, FrameBuffer &fb) const
{
	assert( name );

	int width = 1 + dataWindow.max.x - dataWindow.min.x;

	// update the header
	header.channels().insert( name, Channel(pixelType) );

	// update the framebuffer
	char *offset = (char *) (&channel[0] - (dataWindow.min.x + width * dataWindow.min.y));
	fb.insert(name, Slice(pixelType, offset, sizeof(T), sizeof(T) * width));
}
