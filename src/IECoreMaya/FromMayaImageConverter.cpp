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
#include <cassert>

#include "boost/static_assert.hpp"
#include "boost/format.hpp"
#include "boost/multi_array.hpp"

#include "IECore/ImagePrimitive.h"
#include "IECore/Exception.h"
#include "IECore/ScaledDataConversion.h"

#include "IECoreMaya/FromMayaImageConverter.h"
#include "IECoreMaya/MImageAccessor.h"

#include "OpenEXR/ImathBox.h"

using namespace IECoreMaya;
using namespace IECore;

FromMayaImageConverter::FromMayaImageConverter( MImage &image ) : FromMayaConverter( "name", "description" ), m_image( image )
{
}

const MImage &FromMayaImageConverter::image() const
{
	return m_image;
}

template<typename T> 
void FromMayaImageConverter::writeChannels( ImagePrimitivePtr target, const std::vector< std::string > &channelNames ) const
{
	assert( target );
	unsigned numChannels = m_image.depth() / ( m_image.pixelType() == MImage::kFloat ? sizeof(float) : sizeof(unsigned char) );
	unsigned width, height;
	MStatus s = m_image.getSize( width, height );	
	assert( width );
	assert( height );
	
	std::vector< boost::multi_array_ref< float, 2 > > channelArrays;

	for ( std::vector< std::string >::const_iterator it = channelNames.begin(); it != channelNames.end(); ++it )
	{
		float *dataArray = &runTimeCast<FloatVectorData>( target->variables[*it].data )->writable()[0];
		assert( dataArray );
		channelArrays.push_back( boost::multi_array_ref< float, 2 >( dataArray, boost::extents[height][width] ) );
	}

	boost::multi_array_ref< T, 3 > pixels( MImageAccessor<T>::getPixels( m_image ), boost::extents[height][width][numChannels] );

	ScaledDataConversion< T, float > converter;
	for ( unsigned x = 0; x < width; x++ )
	{
		for ( unsigned y = 0; y < height; y++ )
		{				
			for ( unsigned c = 0; c < channelNames.size(); c++ )
			{	
				/// Vertical flip, to match Maya					
				channelArrays[c][height - 1 - y][x] = converter( pixels[y][x][c] );										
			}				
		}
	}	
}

void FromMayaImageConverter::writeDepth( ImagePrimitivePtr target, const float *depth ) const
{
	assert( target );
	unsigned width, height;
	MStatus s = m_image.getSize( width, height );	
	assert( width );
	assert( height );
	
	boost::multi_array_ref< const float, 2 > depthArray( depth, boost::extents[height][width] );
	
	FloatVectorDataPtr targetDepth = new FloatVectorData();
	targetDepth->writable().resize( width * height );
	
	boost::multi_array_ref< float, 2 > targetDepthArray( &(targetDepth->writable()[0]), boost::extents[height][width] );

	for ( unsigned x = 0; x < width; x++ )
	{
		for ( unsigned y = 0; y < height; y++ )
		{				
			/// Vertical flip, to match Maya					
			targetDepthArray[height - 1 - y][x] = depthArray[y][x];													
		}
	}
	
	target->variables["Z"] = PrimitiveVariable( PrimitiveVariable::Vertex, targetDepth );	
}

ObjectPtr FromMayaImageConverter::doConversion( ConstCompoundObjectPtr operands ) const
{
	assert( operands );
	
	unsigned width, height;
	MStatus s = m_image.getSize( width, height );		
	
	if ( !s || width * height == 0 )
	{
		return new ImagePrimitive();
	}
		
	Imath::Box2i dataWindow( Imath::V2i( 0, 0 ),Imath::V2i( width - 1 , height - 1 ) );
	assert( !dataWindow.isEmpty() );
	
	ImagePrimitivePtr img = new ImagePrimitive( dataWindow, dataWindow );
	
	std::vector< std::string > channels;
	if ( m_image.isRGBA() )
	{
		channels.push_back( "R" );
		channels.push_back( "G" );
		channels.push_back( "B" );
		channels.push_back( "A" );								
	}
	else
	{
		channels.push_back( "B" );
		channels.push_back( "G" );
		channels.push_back( "R" );
		channels.push_back( "A" );
	}
	
	unsigned numChannels = m_image.depth() / ( m_image.pixelType() == MImage::kFloat ? sizeof(float) : sizeof(unsigned char) );
	while ( channels.size() > numChannels )
	{
		channels.pop_back();
	}
	
	if ( channels.size() < 3 )
	{
		throw InvalidArgumentException( "FromMayaImageConverter: MImage has unsupported channel count" );
	}
	assert( channels.size() == 3 || channels.size() == 4 );
	assert( channels.size() <= numChannels );
			
	for ( std::vector< std::string >::const_iterator it = channels.begin(); it != channels.end(); ++it )
	{	
		FloatVectorDataPtr data = new FloatVectorData();
		assert( data );
		
		data->writable().resize( width * height );
	
		img->variables[ *it ] = PrimitiveVariable( PrimitiveVariable::Vertex, data ) ;
	}
		
	switch ( m_image.pixelType() )
	{
		case MImage::kFloat :
			writeChannels<float>( img, channels );
			break;
		case MImage::kByte :
			writeChannels<unsigned char>( img, channels );		
			break;
		default :		
			throw InvalidArgumentException( "FromMayaImageConverter: MImage has unknown pixel type" );
	}
	
	float *depth = m_image.depthMap();
	if ( depth )
	{	
		unsigned depthWidth = 0, depthHeight = 0;
		
		s = m_image.getDepthMapSize( depthWidth, depthHeight );
		assert( s );

		if ( depthWidth != width || depthHeight != height )
		{
			throw InvalidArgumentException( "FromMayaImageConverter: Different color/depth resolutions" );
		}
		
		writeDepth( img, depth );		
	}
	
	assert( img->arePrimitiveVariablesValid() );
	
	return img;
}
