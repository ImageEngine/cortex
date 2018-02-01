//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreImage/ImageThinner.h"

#include "IECore/CompoundParameter.h"
#include "IECore/DespatchTypedData.h"
#include "IECore/TypeTraits.h"

#include "boost/multi_array.hpp"

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreImage;

IE_CORE_DEFINERUNTIMETYPED( ImageThinner );

static int g_masks[] = { 0200, 0002, 0040, 0010 };

static unsigned char g_delete[512] = {

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1

};

ImageThinner::ImageThinner()
	:	ChannelOp( "Performs thinning of binary images." )
{
	FloatParameterPtr thresholdParameter = new FloatParameter(
		"threshold",
		"The threshold above which pixels are considered to be part of the foreground.",
		0.5f
	);

	parameters()->addParameter( thresholdParameter );
}

ImageThinner::~ImageThinner()
{
}

FloatParameter * ImageThinner::thresholdParameter()
{
	return parameters()->parameter<FloatParameter>( "threshold" );
}

const FloatParameter * ImageThinner::thresholdParameter() const
{
	return parameters()->parameter<FloatParameter>( "threshold" );
}

void ImageThinner::modifyChannels( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, ChannelVector &channels )
{
	float threshold = thresholdParameter()->getNumericValue();

	const V2i size = dataWindow.size() + V2i( 1 );

	for( unsigned i=0; i<channels.size(); i++ )
	{
		FloatVectorDataPtr floatData = runTimeCast<FloatVectorData>( channels[i] );
		if( !floatData )
		{
			throw Exception( "ImageThinner::modifyChannels : only float channels supported." );
		}
		std::vector<float> &channel = floatData->writable();

		// threshold the image first
		for( std::vector<float>::iterator it=channel.begin(); it!=channel.end(); it++ )
		{
			*it = *it < threshold ? 0.0f : 1.0f;
		}

		// then apply the graphics gems magic that i don't understand in the slightest
		//////////////////////////////////////////////////////////////////////////////

		boost::multi_array_ref<float, 2> image( &channel[0], boost::extents[size.y][size.x] );

		int	p, q; // Neighborhood maps of adjacent cells
		std::vector<unsigned char> qb; // Neighborhood maps of previous scanline
		qb.resize( size.x );
		qb[qb.size()-1] = 0; // Used for lower-right pixel

		int count = 1; // Deleted pixel count
		while( count )
		{
			count = 0;

			for( int i = 0; i < 4 ; i++ )
			{

				int m = g_masks[i];

				// Build initial previous scan buffer

				p = image[0][0] > 0.5f;
				for( int x = 0; x < size.x-1; x++ )
				{
					qb[x] = p = ((p<<1)&0006) | (image[0][x+1] > 0.5f);
				}

				// Scan image for pixel deletion candidates

				for( int y = 0; y < size.y-1; y++ )
				{

					q = qb[0];
					p = ((q<<3)&0110) | (image[y+1][0] > 0.5f);

					for( int x = 0; x < size.x-1; x++ )
					{
						q = qb[x];
						p = ((p<<1)&0666) | ((q<<3)&0110) |
							(image[y+1][x+1] > 0.5f);
						qb[x] = p;
						if( ((p&m) == 0) && g_delete[p] )
						{
							count++;
							image[y][x] = 0.f;
						}
					}

					// Process right edge pixel

					p = (p<<1)&0666;
					if( (p&m) == 0 && g_delete[p] )
					{
						count++;
						image[y][size.x-1] = 0.f;
					}

				}

				// Process bottom scan line

				for( int x = 0 ; x < size.x ; x++ )
				{
					q = qb[x];
					p = ((p<<1)&0666) | ((q<<3)&0110);
					if( (p&m) == 0 && g_delete[p] )
					{
						count++;
						image[size.y-1][x] = 0.f;
					}
				}
			}
		}
	}
}

