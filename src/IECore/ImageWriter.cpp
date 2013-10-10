//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECore/ImageWriter.h"
#include "IECore/Exception.h"
#include "IECore/MessageHandler.h"
#include "IECore/TypedParameter.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/CompoundParameter.h"
#include "IECore/FileNameParameter.h"
#include "IECore/ColorSpaceTransformOp.h"

using namespace std;
using namespace IECore;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( ImageWriter )

ImageWriter::ImageWriter( const std::string &description ) :
		Writer( description, ImagePrimitiveTypeId)
{
	m_channelsParameter = new StringVectorParameter("channels", "The list of channels to write.  No list causes all channels to be written." );

	std::vector< std::string > colorSpaces;
	ColorSpaceTransformOp::outputColorSpaces( colorSpaces );
	StringParameter::PresetsContainer colorSpacesPresets;
	colorSpacesPresets.push_back( StringParameter::Preset( "Auto Detect", "autoDetect" ) );
	for ( std::vector< std::string >::const_iterator it = colorSpaces.begin(); it != colorSpaces.end(); it++ )
	{
		colorSpacesPresets.push_back( StringParameter::Preset( *it, *it ) );
	}

	m_colorspaceParameter = new StringParameter(
		"colorSpace",
		"Specifies color space that the given image will be when stored in the file. "
		"The writer always assumes the input image is in linear color space and it will"
		"convert the image to the target color space before saving it to a file. "
		"So if you don't want color maniputation select 'linear'. "
		"The Auto Detect option will make the appropriate conversions depending on the "
		"choosen file format.",
		"autoDetect",
		colorSpacesPresets,
		true
	);

	m_rawChannelsParameter = new BoolParameter(
		"rawChannels",
		"Specifies if the image channels should be written as is to the file, keeping the same data type if possible. "
		"Color space settings will not take effect when this parameter is on.",
		false
	);

	parameters()->addParameter( m_channelsParameter );
	parameters()->addParameter( m_colorspaceParameter );
	parameters()->addParameter( m_rawChannelsParameter );
}

StringVectorParameter * ImageWriter::channelNamesParameter()
{
	return m_channelsParameter;
}

const StringVectorParameter * ImageWriter::channelNamesParameter() const
{
	return m_channelsParameter;
}

StringParameter * ImageWriter::colorspaceParameter()
{
	return m_colorspaceParameter;
}

const StringParameter * ImageWriter::colorspaceParameter() const
{
	return m_colorspaceParameter;
}

BoolParameter * ImageWriter::rawChannelsParameter()
{
	return m_rawChannelsParameter;
}

const BoolParameter * ImageWriter::rawChannelsParameter() const
{
	return m_rawChannelsParameter;
}

bool ImageWriter::canWrite( ConstObjectPtr image, const string &fileName )
{
	return runTimeCast<const ImagePrimitive>( image );
}

/// get the user-requested channel names
void ImageWriter::imageChannels( vector<string> &names ) const
{
	ConstImagePrimitivePtr image = getImage();
	assert( image );

	vector<string> allNames;
	image->channelNames( allNames );

	const vector<string> &d = m_channelsParameter->getTypedValue();

	// give all channels when no list is provided
	if ( !d.size() )
	{
		names = allNames;
		return;
	}

	// otherwise, copy in the requested names from the parameter set.
	// this is intersection(A, D)
	names.clear();
	for ( vector<string>::const_iterator it = d.begin(); it != d.end(); it++ )
	{
		if ( find( allNames.begin(), allNames.end(), *it ) != allNames.end() &&
			find( names.begin(), names.end(), *it ) == names.end() )
		{
			names.push_back( *it );
		}
	}
}

const ImagePrimitive *ImageWriter::getImage() const
{
	return static_cast<const ImagePrimitive *>( object() );
}

void ImageWriter::doWrite( const CompoundObject *operands )
{
	// write the image channel data
	vector<string> channels;
	imageChannels(channels);

	ConstImagePrimitivePtr image = getImage();
	assert( image );

	if ( !image->arePrimitiveVariablesValid() )
	{
		throw InvalidArgumentException( "ImageWriter: Invalid primitive variables on image" );
	}

	Box2i dataWindow = image->getDataWindow();

	std::string colorspace = operands->member< StringData >( "colorSpace" )->readable();
	if ( colorspace == "autoDetect" )
	{
		colorspace = destinationColorSpace();
	}

	bool rawChannels = operands->member< BoolData >( "rawChannels" )->readable();

	if ( colorspace != "linear" && !rawChannels )
	{
		// Make sure A is not in the list of channels
		vector<string> channelNames;
		for( vector<string>::const_iterator it = channels.begin(); it != channels.end(); it++ )
		{
			if( *it != "A" )
			{
				channelNames.push_back( *it );
			}
		}
		
		image = image->copy();
		// color convert the image from linear colorspace creating a temporary copy.
		ColorSpaceTransformOpPtr transformOp = new ColorSpaceTransformOp();
		transformOp->inputColorSpaceParameter()->setTypedValue( "linear" );
		transformOp->outputColorSpaceParameter()->setTypedValue( colorspace );
		transformOp->inputParameter()->setValue( constPointerCast< ImagePrimitive >(image) );
		transformOp->copyParameter()->setTypedValue( false );
		transformOp->channelsParameter()->setTypedValue( channelNames );
		transformOp->operate();
 	}

	writeImage( channels, image, dataWindow );
}
