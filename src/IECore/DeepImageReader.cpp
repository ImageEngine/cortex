//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/DeepImageReader.h"
#include "IECore/FileNameParameter.h"
#include "IECore/ImagePrimitive.h"
#include "IECore/NullObject.h"
#include "IECore/ObjectParameter.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( DeepImageReader );

DeepImageReader::DeepImageReader( const std::string &description )
	: Reader( description, new ObjectParameter( "result", "The composited image", new NullObject, ImagePrimitive::staticTypeId() ) )
{
}

ObjectPtr DeepImageReader::doOperation( const CompoundObject *operands )
{
	Imath::Box2i displayWind = displayWindow();
	Imath::Box2i dataWind = dataWindow();

	// create our ImagePrimitive
	ImagePrimitivePtr image = new ImagePrimitive( dataWind, displayWind );

	std::vector<std::string> channels;
	channelNames( channels );
	
	std::vector<std::vector<float> *> primVarData;
	
	Imath::V2i pixelDimensions = dataWind.size() + Imath::V2i( 1 );
	unsigned numPixels = pixelDimensions.x * pixelDimensions.y;
	unsigned numChannels = channels.size();
	primVarData.reserve( numChannels );
	
	for ( std::vector<std::string>::const_iterator cIt = channels.begin(); cIt != channels.end(); ++cIt )
	{
		FloatVectorDataPtr data = new FloatVectorData( std::vector<float>( numPixels ) );
		primVarData.push_back( &data->writable() );
		
		image->variables[*cIt] = PrimitiveVariable( PrimitiveVariable::Vertex, data );
	}

	DeepPixelPtr pixel = 0;
	float *channelData = new float [channels.size()];
	
	unsigned p = 0;
	for ( int y=dataWind.min.y; y < dataWind.max.y + 1; ++y )
	{
		for ( int x=dataWind.min.x; x < dataWind.max.x + 1; ++x, ++p )
		{
			pixel = readPixel( x, y );
			
			if ( !pixel )
			{
				continue;
			}
			
			pixel->composite( channelData );
			
			for ( unsigned c=0; c < numChannels; ++c )
			{
				(*primVarData[c])[p] = channelData[c];
			}
		}
	}
	delete [] channelData;
	return image;
}

DeepPixelPtr DeepImageReader::readPixel( int x, int y )
{
	// validate that requested pixel is inside the available data window
	if( !dataWindow().intersects( Imath::V2i( x, y ) ) )
	{
		throw Exception( "Requested pixel not in available data window." );
	}
	
	return doReadPixel( x, y );
}

CompoundObjectPtr DeepImageReader::readHeader()
{
	std::vector<std::string> names;
	channelNames( names );
	
	CompoundObjectPtr header = Reader::readHeader();
	header->members()["displayWindow"] = new Box2iData( displayWindow() );
	header->members()["dataWindow"] = new Box2iData( dataWindow() );
	header->members()["channelNames"] = new StringVectorData( names );
	header->members()["worldToCameraMatrix"] = new M44fData( worldToCameraMatrix() );
	header->members()["worldToNDCMatrix"] = new M44fData( worldToNDCMatrix() );
	
	return header;
}
