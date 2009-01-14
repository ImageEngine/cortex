//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include <iostream>
#include <vector>
#include <string>

#include "boost/multi_array.hpp"

#include "IECore/CompoundParameter.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/DataConvert.h"
#include "IECore/ScaledDataConversion.h"
#include "IECore/TypedData.h"
#include "IECore/VectorTypedData.h"

#include "IECoreMaya/ToMayaImageConverter.h"
#include "IECoreMaya/MImageAccessor.h"

using namespace IECoreMaya;
using namespace IECore;

using std::string;
using std::vector;

ToMayaImageConverter::ToMayaImageConverter( ConstObjectPtr object )
	:	ToMayaConverter( "ToMayaImageConverter", "Converts image types.", IECore::ImagePrimitiveTypeId )
{
	srcParameter()->setValue( boost::const_pointer_cast<Object>( object ) );
	
	IntParameter::PresetsMap typePresets;
	typePresets["Float"] = Float;
	typePresets["Byte"] = Byte;	
	
	m_typeParameter = new IntParameter(
		"type",		
		"Type of image to convert to",
		Byte,
		typePresets );	
		
	parameters()->addParameter( m_typeParameter );	
}

ToMayaImageConverterPtr ToMayaImageConverter::create( const IECore::ObjectPtr src )
{
	return new ToMayaImageConverter( src );
}

IntParameterPtr ToMayaImageConverter::typeParameter()
{
	return m_typeParameter;
}

ConstIntParameterPtr ToMayaImageConverter::typeParameter() const
{
	return m_typeParameter;
}

template<typename C>
struct ToMayaImageConverter::ChannelConverter
{
	typedef typename TypedData< std::vector<C> >::Ptr  ReturnType;
	
	const std::string &m_channelName;
	
	ChannelConverter( const std::string &channelName ) : m_channelName( channelName )
	{
	}

	template<typename T>
	ReturnType operator()( typename T::Ptr data )
	{
		assert( data );
			
		return DataConvert < T, TypedData< std::vector<C> >, ScaledDataConversion< typename T::ValueType::value_type, C> >()
		(
			boost::static_pointer_cast<const T>( data )
		);
	};
	
	struct ErrorHandler
	{
		template<typename T, typename F>
		void operator()( typename T::ConstPtr data, const F& functor )
		{
			assert( data );
		
			throw InvalidArgumentException( ( boost::format( "ToMayaImageConverter: Invalid data type \"%s\" for channel \"%s\"." ) % Object::typeNameFromTypeId( data->typeId() ) % functor.m_channelName ).str() );		
		}
	};
};

template<typename T>
void ToMayaImageConverter::writeChannel( MImage &image, typename TypedData< std::vector<T> >::Ptr channelData, unsigned channelOffset, unsigned numChannels ) const
{
	assert( channelOffset < numChannels );
	
	unsigned width, height;
	image.getSize( width, height );
		
	boost::multi_array_ref< const T, 2 > src( &channelData->readable()[0], boost::extents[height][width] );	
	boost::multi_array_ref< T, 3 > dst( MImageAccessor<T>::getPixels( image ), boost::extents[ height ][ width ][ numChannels ] );
	
	for ( unsigned x = 0; x < width; x++ )
	{
		for ( unsigned y = 0; y < height; y++ )
		{			
			/// Vertical flip, to match Maya	
			dst[height - y  - 1][x][channelOffset] = src[y][x];
		}
	}	
}

MStatus ToMayaImageConverter::convert( MImage &image ) const
{
	ConstImagePrimitivePtr toConvert = runTimeCast<const ImagePrimitive>( srcParameter()->getValidatedValue() );
	assert( toConvert );
	
	
	/// Get the channels RGBA at the front, in that order, if they exist
	vector<string> desiredChannelOrder;
	desiredChannelOrder.push_back( "R" );
	desiredChannelOrder.push_back( "G" );
	desiredChannelOrder.push_back( "B" );
	desiredChannelOrder.push_back( "A" );

	vector<string> channelNames;
	for  ( PrimitiveVariableMap::const_iterator it = toConvert->variables.begin(); it != toConvert->variables.end(); ++it )
	{
		channelNames.push_back( it->first );
	}
	
	vector<string> filteredNames;

	int rgbChannelsFound = 0;
	bool haveAlpha = false;
	for ( vector<string>::const_iterator it = desiredChannelOrder.begin(); it != desiredChannelOrder.end(); ++it )
	{
		vector<string>::iterator res = find( channelNames.begin(), channelNames.end(), *it );
		if ( res != channelNames.end() )
		{
			if ( *it == "A" )
			{
				haveAlpha = true;
			}
			else
			{
				rgbChannelsFound ++;
			}
			channelNames.erase( res );
			filteredNames.push_back( *it );
		}
	}
	channelNames = filteredNames;
	
	if ( rgbChannelsFound != 3 )
	{
		return MS::kFailure;
	}
			
	unsigned int width = toConvert->getDataWindow().size().x + 1;
	unsigned int height = toConvert->getDataWindow().size().y + 1;	
	
	unsigned numChannels = rgbChannelsFound + ( haveAlpha ? 1 : 0 );
	
	MImage::MPixelType pixelType = MImage::kUnknown;
	switch( typeParameter()->getNumericValue() )
	{
		case Float :
			pixelType = MImage::kFloat;
			break;
		case Byte:
			pixelType = MImage::kByte;		
			break;
		default :
		
			assert( false );
	}
	
	MStatus s = image.create( width, height, numChannels, pixelType );
	if ( !s )
	{		
		return s;
	}
	
	image.setRGBA( true );
	
	unsigned channelOffset = 0;
	for ( vector<string>::const_iterator it = channelNames.begin(); it != channelNames.end(); ++it, ++channelOffset )
	{					
		DataPtr dataContainer = toConvert->variables.find( *it )->second.data;
		assert( dataContainer );
		
		switch( pixelType )
		{
			case MImage::kFloat :
			{
				ChannelConverter<float> converter( *it );
				FloatVectorDataPtr channelData = despatchTypedData<
					ChannelConverter<float>,
					TypeTraits::IsNumericVectorTypedData,
					ChannelConverter<float>::ErrorHandler
				>( dataContainer, converter );
	
				writeChannel<float>( image, channelData, channelOffset, numChannels );	
			}
				
				break;
			case MImage::kByte:
			{
				ChannelConverter<unsigned char> converter( *it );
				UCharVectorDataPtr channelData = despatchTypedData<
					ChannelConverter<unsigned char>,
					TypeTraits::IsNumericVectorTypedData,
					ChannelConverter<unsigned char>::ErrorHandler
				>( dataContainer, converter );
	
				writeChannel<unsigned char>( image, channelData, channelOffset, numChannels );	
			}
				
				break;
			default :

				assert( false );
		}
		
	
			
	}
	
	
	return MS::kSuccess;
}
