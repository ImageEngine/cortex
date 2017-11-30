//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2012, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "boost/format.hpp"

#include "IECore/TypedData.h"
#include "IECore/VectorTypedData.h"

#include "IECore/CompoundData.h"

#include "IECore/DespatchTypedData.h"
#include "IECore/MessageHandler.h"
#include "IECoreScene/PrimitiveVariable.h"

#include "IECoreImage/ImagePrimitive.h"

#include "IECoreGL/ToGLTextureConverter.h"
#include "IECoreGL/ColorTexture.h"
#include "IECoreGL/LuminanceTexture.h"

using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( ToGLTextureConverter );

ToGLTextureConverter::ConverterDescription<ToGLTextureConverter> ToGLTextureConverter::g_description;
ToGLTextureConverter::ConverterDescription<ToGLTextureConverter> ToGLTextureConverter::g_compoundDataDescription( IECore::CompoundData::staticTypeId(), IECoreGL::Texture::staticTypeId() );

ToGLTextureConverter::ToGLTextureConverter( IECore::ConstObjectPtr toConvert, bool createMissingRGBChannels )
	:	ToGLConverter( "Converts IECoreImage::ImagePrimitive objects to IECoreGL::Texture objects.", IECore::ObjectTypeId ),
	m_createMissingRGBChannels( createMissingRGBChannels )
{
	srcParameter()->setValue( boost::const_pointer_cast<IECore::Object>( toConvert ) );
}

ToGLTextureConverter::~ToGLTextureConverter()
{
}

IECore::RunTimeTypedPtr ToGLTextureConverter::doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const
{

	TexturePtr t = nullptr;
	IECoreImage::ImagePrimitive::ConstPtr image;

	image = IECore::runTimeCast<const IECoreImage::ImagePrimitive>( src );

	if ( ! image )
	{

		IECore::CompoundData::ConstPtr data = IECore::runTimeCast<const IECore::CompoundData>( src );
		if ( !data ) {
			throw IECore::Exception( "Invalid object supplied. ToGLTextureConverter takes an ImagePrimitive or its CompoundData representation." );
		}

		image = imageFromCompoundData( data );

	}

	bool r = image->channelValid( "R" );
	bool g = image->channelValid( "G" );
	bool b = image->channelValid( "B" );
	bool y = image->channelValid( "Y" );

	if ( !y && r && g && b )
	{
		t = new ColorTexture( image.get() );
	}
	else if ( y && !r && !g && !b )
	{
		t = new LuminanceTexture( image.get() );
	}
	else
	{
		if( m_createMissingRGBChannels )
		{
			image = createMissingChannels( image.get() );
			t = new ColorTexture( image.get() );
		}
		else
		{
			throw IECore::Exception( "Invalid image format, ToGLTextureConverter supports RGB[A] and Y[A]." );
		}
	}

	if ( ! t )
	{
		throw IECore::Exception( "Failed to create IECoreGL Texture." );
	}


	return t;
}

IECoreImage::ImagePrimitivePtr ToGLTextureConverter::createMissingChannels( const IECoreImage::ImagePrimitive *image ) const
{
	IECoreImage::ImagePrimitivePtr newImage = image->copy();
	if( newImage->getChannel<float>( "R" ) == nullptr)
	{
		newImage->createChannel<float>( "R" );
	}
	if( newImage->getChannel<float>( "G" ) == nullptr)
	{
		newImage->createChannel<float>( "G" );
	}
	if( newImage->getChannel<float>( "B" ) == nullptr)
	{
		newImage->createChannel<float>( "B" );
	}
	return newImage;
}

IECoreImage::ImagePrimitivePtr ToGLTextureConverter::imageFromCompoundData( IECore::CompoundData::ConstPtr data ) const
{

	if ( ! data )
	{
		throw IECore::Exception( "No CompoundData supplied." );
	}

	IECore::Box2iDataPtr dataWindow = nullptr;

	IECore::CompoundDataMap::const_iterator itData = data->readable().find( "dataWindow" );
	if ( itData == data->readable().end() )
	{
		throw IECore::Exception( "Invalid CompoundData supplied. ImagePrimitive representations need a dataWindow (Box2i)." );
	}
	dataWindow = IECore::runTimeCast<IECore::Box2iData>( itData->second );

	IECore::Box2iDataPtr screenWindow = nullptr;
	itData = data->readable().find( "displayWindow" );
	if ( itData == data->readable().end() )
	{
		throw IECore::Exception( "Invalid CompoundData supplied. ImagePrimitive representations need a screenWindow (Box2i)." );
	}
	screenWindow = IECore::runTimeCast<IECore::Box2iData>( itData->second );

	IECore::CompoundDataPtr channels = nullptr;
	itData = data->readable().find( "channels" );
	if ( itData == data->readable().end() )
	{
		throw IECore::Exception( "Invalid CompoundData supplied. ImagePrimitive representations need a CompoundDataMap of channels." );
	}
	channels = IECore::runTimeCast<IECore::CompoundData>( itData->second );


	if ( !dataWindow || !screenWindow || !channels )
	{
		throw IECore::Exception( "Invalid CompoundData representation supplied. Some data is of the wrong type" );
	}


	IECoreImage::ImagePrimitivePtr newImage = new IECoreImage::ImagePrimitive( dataWindow->readable(), screenWindow->readable() );
	for (
			IECore::CompoundDataMap::const_iterator itChannels = channels->readable().begin();
			itChannels != channels->readable().end();
			itChannels++
	)
	{
		IECore::FloatVectorDataPtr channelData = IECore::runTimeCast<IECore::FloatVectorData>( itChannels->second );
		if ( ! channelData )
		{
			throw IECore::Exception( "Invalid channel data found in ImagePrimitive representation, only 32bit float data is supported. Please check texture.");
		}

		newImage->channels.insert( IECoreImage::ImagePrimitive::ChannelMap::value_type( itChannels->first, channelData ) );

	}

	return newImage;

}

