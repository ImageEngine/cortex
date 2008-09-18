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

#include "IECore/ImageDisplayDriver.h"

using namespace boost;
using namespace std;
using namespace Imath;
using namespace IECore;

ImageDisplayDriver::ImageDisplayDriver( const Box2i &displayWindow, const Box2i &dataWindow, const vector<string> &channelNames, ConstCompoundDataPtr parameters ) :
		DisplayDriver( displayWindow, dataWindow, channelNames, parameters ), 
		m_image( new ImagePrimitive( dataWindow, displayWindow ) )
{
	for ( vector<string>::const_iterator it = channelNames.begin(); it != channelNames.end(); it++ )
	{
		m_image->createChannel<float>( *it );
	}
}

ImageDisplayDriver::~ImageDisplayDriver()
{
}

bool ImageDisplayDriver::scanLineOrderOnly() const
{
	return false;
}

void ImageDisplayDriver::imageData( const Box2i &box, const float *data, size_t dataSize )
{
	Box2i tmpBox = box;
	Box2i dataWindow = m_image->getDataWindow();
	tmpBox.extendBy( dataWindow );
	if ( tmpBox != dataWindow )
	{
		throw Exception("The box is outside image data window.");
	}

	int pixelSize = channelNames().size();
	if ( dataSize != (box.max.x - box.min.x + 1) * (box.max.y - box.min.y + 1) * channelNames().size() )
	{
		throw Exception("Invalid dataSize value.");
	}

	int channel, targetX, targetY, sourceWidth, sourceHeight, targetWidth;
	sourceWidth = box.max.x - box.min.x + 1;
	sourceHeight = box.max.y - box.min.y + 1;
	targetWidth = dataWindow.max.x - dataWindow.min.x + 1;
	channel = 0;
	targetX = box.min.x - dataWindow.min.x;
	targetY = box.min.y - dataWindow.min.y;

	for ( vector<string>::const_iterator it = channelNames().begin(); it != channelNames().end(); it++, channel++ )
	{
		vector< float > &target = static_pointer_cast< FloatVectorData >(m_image->variables[ *it ].data)->writable();
		vector< float >::iterator targetIt;
		const float *sourceIt = data+channel;
		targetIt = target.begin()+targetWidth*targetY+targetX;

		for ( int y = 0; y < sourceHeight; y++ )
		{
			for ( int x = 0; x < sourceWidth; x++ )
			{
				*targetIt = *sourceIt;
				sourceIt += pixelSize;
				targetIt++;
			}
			targetIt += targetWidth - sourceWidth;
		}
	}
}

void ImageDisplayDriver::imageClose()
{
}

ConstImagePrimitivePtr ImageDisplayDriver::image() const
{
	return m_image;
}
