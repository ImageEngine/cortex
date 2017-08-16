//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2017, Image Engine Design Inc. All rights reserved.
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

#include <sys/utsname.h>

#include "boost/filesystem.hpp"
#include "boost/static_assert.hpp"

#include "boost/mpl/or.hpp"
#include "boost/mpl/and.hpp"
#include "boost/mpl/if.hpp"
#include "boost/mpl/not.hpp"
#include "boost/mpl/eval_if.hpp"
#include "boost/mpl/identity.hpp"

#include "boost/type_traits.hpp"

#include "OpenImageIO/imageio.h"
OIIO_NAMESPACE_USING

#include "IECore/DataInterleaveOp.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/Exception.h"
#include "IECore/MessageHandler.h"
#include "IECore/ObjectVector.h"
#include "IECore/TypedParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/FileNameParameter.h"

#include "IECoreImage/ImagePrimitive.h"
#include "IECoreImage/ImageWriter.h"
#include "IECoreImage/OpenImageIOAlgo.h"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreImage;

IE_CORE_DEFINERUNTIMETYPED( ImageWriter )

namespace
{

void channelsToWrite( const ImagePrimitive *image, const ImageOutput *out, const CompoundObject *operands, std::vector<std::string> &channels )
{
	channels.clear();

	const bool supportsNChannels = (bool)out->supports( "nchannels" );
	const bool supportsAlpha = (bool)out->supports( "alpha" );

	const std::vector<std::string> &requestedChannels = operands->member<const StringVectorData>( "channels" )->readable();

	std::vector<std::string> existingChannels;
	image->channelNames( existingChannels );

	// some formats require RGB listed in order, so lets find them first
	auto rIt = find( existingChannels.begin(), existingChannels.end(), "R" );
	if( rIt != existingChannels.end() && rIt != existingChannels.begin() )
	{
		std::iter_swap( existingChannels.begin(), rIt );
	}
	auto gIt = find( existingChannels.begin(), existingChannels.end(), "G" );
	if( gIt != existingChannels.end() && gIt != existingChannels.begin() + 1 )
	{
		std::iter_swap( existingChannels.begin() + 1, gIt );
	}
	auto bIt = find( existingChannels.begin(), existingChannels.end(), "B" );
	if( bIt != existingChannels.end() && bIt != existingChannels.begin() + 2 )
	{
		std::iter_swap( existingChannels.begin() + 2, bIt );
	}

	for( const auto &channel : existingChannels )
	{
		if (
			// channel was requested
			(
				requestedChannels.empty() ||
				find( requestedChannels.begin(), requestedChannels.end(), channel ) != requestedChannels.end()
			) &&
			// channel hasn't been added yet
			find( channels.begin(), channels.end(), channel ) == channels.end() &&
			// only allow arbitrary channels if the format supports it
			( supportsNChannels || channel == "R" || channel == "G" || channel == "B" || channel == "A" || channel == "Y" ) &&
			// only allow alpha if the format supports it
			( supportsAlpha || channel != "A" )
		)
		{
			channels.push_back( channel );
		}
	}
}

void metadataToImageSpecAttributes( const CompoundData *metadata, ImageSpec *spec, const std::string &prefix = "" )
{
	const auto &members = metadata->readable();
	for( const auto &item : members )
	{
		std::string thisName = ( (bool)prefix.size() ? prefix + "." : std::string("") ) + item.first.value();

		if( const CompoundData *compound = runTimeCast<const CompoundData>( item.second.get() ) )
		{
			metadataToImageSpecAttributes( compound, spec, thisName );
		}
		else
		{
			const OpenImageIOAlgo::DataView dataView( item.second.get() );
			if( (bool)dataView.rawData )
			{
				spec->attribute( thisName, dataView.type, dataView.rawData );
			}
		}
	}
}

void setImageSpecFormatOptions( const CompoundObject *operands, ImageSpec *spec, const std::string &fileFormatName )
{
	const CompoundObject *settings = operands->member<const CompoundObject>( fileFormatName );
	if( settings == nullptr )
	{
		return;
	}

	std::string dataType;
	if( const StringData *dataTypeHolder = settings->member<const StringData>( "dataType" ) )
	{
		dataType = dataTypeHolder->readable();

		/// \todo: This is copied from GafferImage::ImageWriter.
		/// Can it be consolidated in OpenImageIOAlgo?
		if( dataType == "int8" )
		{
			spec->set_format( TypeDesc::INT8 );
		}
		else if( dataType == "int16" )
		{
			spec->set_format( TypeDesc::INT16 );
		}
		else if( dataType == "int32" )
		{
			spec->set_format( TypeDesc::INT32 );
		}
		else if( dataType == "int64" )
		{
			spec->set_format( TypeDesc::INT64 );
		}
		else if( dataType == "uint8" )
		{
			spec->set_format( TypeDesc::UINT8 );
		}
		else if( dataType == "uint16" )
		{
			spec->set_format( TypeDesc::UINT16 );
		}
		else if( dataType == "uint32" )
		{
			spec->set_format( TypeDesc::UINT32 );
		}
		else if( dataType == "uint64" )
		{
			spec->set_format( TypeDesc::UINT64 );
		}
		else if( dataType == "half" )
		{
			spec->set_format( TypeDesc::HALF );
		}
		else if( dataType == "float" )
		{
			spec->set_format( TypeDesc::FLOAT );
		}
		else if( dataType == "double" )
		{
			spec->set_format( TypeDesc::DOUBLE );
		}
	}

	if( const StringData *compression = settings->member<const StringData>( "compression" ) )
	{
		spec->attribute( "compression", compression->readable() );
	}

	if( fileFormatName == "jpeg" )
	{
		spec->attribute( "CompressionQuality", settings->member<const IntData>( "quality" )->readable() );
	}
	else if( fileFormatName == "dpx" )
	{
		if( dataType == "uint10" )
		{
			spec->set_format( TypeDesc::UINT16 );
			spec->attribute( "oiio:BitsPerSample", 10 );
		}
		else if( dataType == "uint12" )
		{
			spec->set_format( TypeDesc::UINT16 );
			spec->attribute( "oiio:BitsPerSample", 12 );
		}
	}
}

} // namespace

////////////////////////////////////////////////////////////////////////////////
// ImageWriter
////////////////////////////////////////////////////////////////////////////////

const Writer::WriterDescription<ImageWriter> ImageWriter::g_writerDescription( OpenImageIOAlgo::extensions() );

ImageWriter::ImageWriter()
	: Writer( "Serializes images to disk using OpenImageIO", (IECore::TypeId)ImagePrimitiveTypeId )
{
	constructCommon();
}

ImageWriter::ImageWriter( IECore::ObjectPtr object, const std::string &fileName )
	: Writer( "Serializes images to disk using OpenImageIO", (IECore::TypeId)ImagePrimitiveTypeId )
{
	constructCommon();

	m_objectParameter->setValue( object );
	m_fileNameParameter->setTypedValue( fileName );
}

void ImageWriter::constructCommon()
{
	m_channelsParameter = new StringVectorParameter( "channels", "The list of channels to write. No list causes all channels to be written." );

	m_formatSettingsParameter = new CompoundParameter( "formatSettings", "Settings specific to various file formats" );

	CompoundParameterPtr exrSettings = new CompoundParameter( "openexr", "OpenEXR specific settings" );
	m_formatSettingsParameter->addParameter( exrSettings );
	StringParameter::PresetsContainer exrCompressionPresets;
	exrCompressionPresets.emplace_back( StringParameter::Preset( "none", "none" ) );
	exrCompressionPresets.emplace_back( StringParameter::Preset( "rle", "rle" ) );
	exrCompressionPresets.emplace_back( StringParameter::Preset( "zips", "zips" ) );
	exrCompressionPresets.emplace_back( StringParameter::Preset( "zip", "zip" ) );
	exrCompressionPresets.emplace_back( StringParameter::Preset( "piz", "piz" ) );
	exrCompressionPresets.emplace_back( StringParameter::Preset( "pxr24", "pxr24" ) );
	exrCompressionPresets.emplace_back( StringParameter::Preset( "b44", "b44" ) );
	exrCompressionPresets.emplace_back( StringParameter::Preset( "b44a", "b44a" ) );
	exrCompressionPresets.emplace_back( StringParameter::Preset( "dwaa", "dwaa" ) );
	exrCompressionPresets.emplace_back( StringParameter::Preset( "dwab", "dwab" ) );
	exrSettings->addParameter(
		new StringParameter(
			"compression",
			"OpenEXR compression",
			"zips",
			exrCompressionPresets,
			/* presetsOnly */ true
		)
	);
	StringParameter::PresetsContainer exrDataTypePresets;
	exrDataTypePresets.emplace_back( StringParameter::Preset( "half", "half" ) );
	exrDataTypePresets.emplace_back( StringParameter::Preset( "float", "float" ) );
	exrDataTypePresets.emplace_back( StringParameter::Preset( "double", "double" ) );
	exrSettings->addParameter(
		new StringParameter(
			"dataType",
			"Format of the data to write. OpenImageIO will convert the PrimitiveVariables to this format automatically",
			"half",
			exrDataTypePresets,
			/* presetsOnly */ true
		)
	);

	CompoundParameterPtr dpxSettings = new CompoundParameter( "dpx", "dpx specific settings" );
	m_formatSettingsParameter->addParameter( dpxSettings );
	StringParameter::PresetsContainer dpxDataTypePresets;
	dpxDataTypePresets.emplace_back( StringParameter::Preset( "8-bit", "uint8" ) );
	dpxDataTypePresets.emplace_back( StringParameter::Preset( "10-bit", "uint10" ) );
	dpxDataTypePresets.emplace_back( StringParameter::Preset( "12-bit", "uint12" ) );
	dpxDataTypePresets.emplace_back( StringParameter::Preset( "16-bit", "uint16" ) );
	dpxSettings->addParameter(
		new StringParameter(
			"dataType",
			"Format of the data to write. OpenImageIO will convert the PrimitiveVariables to this format automatically",
			"uint10",
			dpxDataTypePresets,
			/* presetsOnly */ true
		)
	);

	CompoundParameterPtr tifSettings = new CompoundParameter( "tiff", "tiff specific settings" );
	m_formatSettingsParameter->addParameter( tifSettings );
	StringParameter::PresetsContainer tifCompressionPresets;
	tifCompressionPresets.emplace_back( StringParameter::Preset( "none", "none" ) );
	tifCompressionPresets.emplace_back( StringParameter::Preset( "lzw", "lzw" ) );
	tifCompressionPresets.emplace_back( StringParameter::Preset( "jpeg", "jpeg" ) );
	tifCompressionPresets.emplace_back( StringParameter::Preset( "zip", "zip" ) );
	tifCompressionPresets.emplace_back( StringParameter::Preset( "deflate", "deflate" ) );
	tifCompressionPresets.emplace_back( StringParameter::Preset( "packbits", "packbits" ) );
	tifSettings->addParameter(
		new StringParameter(
			"compression",
			"tif compression",
			"zip",
			tifCompressionPresets,
			/* presetsOnly */ true
		)
	);
	StringParameter::PresetsContainer tifDataTypePresets;
	tifDataTypePresets.emplace_back( StringParameter::Preset( "8-bit", "uint8" ) );
	tifDataTypePresets.emplace_back( StringParameter::Preset( "16-bit", "uint16" ) );
	tifDataTypePresets.emplace_back( StringParameter::Preset( "float", "float" ) );
	tifSettings->addParameter(
		new StringParameter(
			"dataType",
			"Format of the data to write. OpenImageIO will convert the PrimitiveVariables to this format automatically",
			"uint8",
			tifDataTypePresets,
			/* presetsOnly */ true
		)
	);

	CompoundParameterPtr jpgSettings = new CompoundParameter( "jpeg", "jpeg specific settings" );
	m_formatSettingsParameter->addParameter( jpgSettings );
	jpgSettings->addParameter(
		new IntParameter(
			"quality",
			"A value between 0 (low quality, high compression) and 100 (high quality, low compression).",
			100,
			/* min */ 0,
			/* max */ 100
		)
	);

	parameters()->addParameter( m_channelsParameter );
	parameters()->addParameter( m_formatSettingsParameter );
}

ImageWriter::~ImageWriter() = default;

StringVectorParameter *ImageWriter::channelNamesParameter()
{
	return m_channelsParameter.get();
}

const StringVectorParameter *ImageWriter::channelNamesParameter() const
{
	return m_channelsParameter.get();
}

CompoundParameter *ImageWriter::formatSettingsParameter()
{
	return m_formatSettingsParameter.get();
}

const CompoundParameter *ImageWriter::formatSettingsParameter() const
{
	return m_formatSettingsParameter.get();
}

bool ImageWriter::canWrite( ConstObjectPtr object, const string &fileName )
{
	return (bool)runTimeCast<const ImagePrimitive>( object ).get();
}

void ImageWriter::channelsToWrite( vector<string> &channels, const CompoundObject *operands ) const
{
	ImageOutput *out = ImageOutput::create( fileName() );

	const CompoundObject *args = (bool)operands ? operands : parameters()->getTypedValue<CompoundObject>();

	::channelsToWrite( getImage(), out, args, channels );

	out->close();
	ImageOutput::destroy( out );
}

const ImagePrimitive *ImageWriter::getImage() const
{
	return static_cast<const ImagePrimitive *>( object() );
}

void ImageWriter::doWrite( const CompoundObject *operands )
{
	const ImagePrimitive *image = getImage();
	if( !image->channelsValid() )
	{
		throw InvalidArgumentException( "ImageWriter: Invalid channels on image" );
	}

	Box2i dataWindow = image->getDataWindow();
	Box2i displayWindow = image->getDisplayWindow();

	/// \todo: nearly everything below this point is copied from GafferImage::ImageWriter
	/// Can we consolidate some of this into IECoreImage::OpenImageIOAlgo?

	ImageOutput *out = ImageOutput::create( fileName() );
	if( !out )
	{
		throw IECore::Exception( OpenImageIO::geterror() );
	}

	const std::string fileFormatName = out->format_name();
	const bool supportsDisplayWindow = (bool)out->supports( "displaywindow" ) && fileFormatName != "dpx";

	ImageSpec spec( TypeDesc::UNKNOWN );

	// Specify the display window.
	spec.full_x = displayWindow.min.x;
	spec.full_y = displayWindow.min.y;
	spec.full_width = displayWindow.size().x + 1;
	spec.full_height = displayWindow.size().y + 1;

	if( supportsDisplayWindow && dataWindow.hasVolume() )
	{
		spec.x = dataWindow.min.x;
		spec.y = dataWindow.min.y;
		spec.width = dataWindow.size().x + 1;
		spec.height = dataWindow.size().y + 1;
	}
	else
	{
		spec.x = spec.full_x;
		spec.y = spec.full_y;
		spec.width = spec.full_width;
		spec.height = spec.full_height;
	}

	// Cleanse the image blindData and then add it to the spec
	CompoundDataPtr metadata = image->blindData()->copy();
	metadata->writable().erase( "oiio:ColorSpace" );
	metadata->writable().erase( "oiio:Gamma" );
	metadata->writable().erase( "oiio:UnassociatedAlpha" );
	metadata->writable().erase( "fileFormat" );
	metadata->writable().erase( "dataType" );

	metadataToImageSpecAttributes( metadata.get(), &spec );

	setImageSpecFormatOptions( operands->member<const CompoundObject>( "formatSettings" ), &spec, out->format_name() );

	// Add common attribs to the spec
	std::string software = ( boost::format( "Cortex %d.%d.%d" ) % IE_CORE_MAJORVERSION % IE_CORE_MINORVERSION % IE_CORE_PATCHVERSION ).str();
	spec.attribute( "Software", software );
	struct utsname info;
	if ( !(bool)uname( &info ) )
	{
		spec.attribute( "HostComputer", info.nodename );
	}
	if ( const char *artist = getenv( "USER" ) )
	{
		spec.attribute( "Artist", artist );
	}

	std::vector<std::string> channels;
	::channelsToWrite( image, out, operands, channels );
	if( channels.empty() )
	{
		throw IECore::Exception( std::string( "IECoreImage::ImageWriter : No valid channels were specified for the file format \"" ) + out->format_name() + "\"." );
	}

	spec.nchannels = (int)channels.size();
	spec.channelnames.clear();
	spec.channelnames.reserve( channels.size() );

	const Data *firstChannelData = image->channels.begin()->second.get();

	for( auto it = channels.begin(), cEnd = channels.end(); it != cEnd; ++it )
	{
		spec.channelnames.push_back( *it );

		// OpenImageIO claims to handle non-matching types (if the format supports it)
		// but when it comes time to write the scanlines, we must pass a single buffer
		// of interleaved pixels, so we only support a single type for now.
		if( image->channels.find( *it )->second->typeId() != firstChannelData->typeId() )
		{
			throw IECore::Exception( "IECoreImage::ImageWriter : Image must have channels of the same type." );
		}

		// OIIO has a special attribute for the Alpha and Z channels. If we find some, we should tag them...
		if( *it == "A" )
		{
			spec.alpha_channel = (int)(it - channels.begin());
		}
		else if( *it == "Z" )
		{
			spec.z_channel = (int)(it - channels.begin());
		}
	}

	// Create the directory we need and open the file
	boost::filesystem::path directory = boost::filesystem::path( fileName() ).parent_path();
	if( !directory.empty() )
	{
		boost::filesystem::create_directories( directory );
	}

	if ( out->open( fileName(), spec ) )
	{
		IECore::msg( IECore::MessageHandler::Info, "IECoreImage::ImageWriter", "Writing " + fileName() );
	}
	else
	{
		throw IECore::Exception( boost::str( boost::format( "IECoreImage::ImageWriter : Could not open \"%s\", error = %s" ) % fileName() % out->geterror() ) );
	}

	DataInterleaveOpPtr op = new DataInterleaveOp();
	op->targetTypeParameter()->setNumericValue( firstChannelData->typeId() );
	auto &vectors = static_cast<ObjectVector *>( op->dataParameter()->getValue() )->members();
	vectors.reserve( (size_t)spec.nchannels );
	for( const auto &channel : channels )
	{
		vectors.emplace_back( image->channels.find( channel )->second );
	}

	DataPtr buffer = static_pointer_cast<Data>( op->operate() );
	const OpenImageIOAlgo::DataView dataView( buffer.get() );
	if( dataView.type == TypeDesc::UNKNOWN )
	{
		throw IECore::Exception( boost::str( boost::format( "IECoreImage::ImageWriter : Failed to write \"%s\". Unsupported dataType %s." ) % fileName() % buffer->typeName() ) );
	}

	const unsigned char *rawBuffer = (const unsigned char *)dataView.rawData;
	size_t stride = spec.nchannels * spec.width * dataView.type.elementsize();

	for( int y = 0; y < spec.height; ++y )
	{
		bool status = out->write_scanline(
			/* y */ spec.y + y,
			/* z */ 0,
			/* format */ dataView.type,
			/* data */ &rawBuffer[stride * y]
		);

		if( !status )
		{
			throw IECore::Exception( boost::str( boost::format( "IECoreImage::ImageWriter : Failed to write \"%s\", error = %s" ) % fileName() % out->geterror() ) );
		}
	}

	out->close();
	ImageOutput::destroy( out );
}
