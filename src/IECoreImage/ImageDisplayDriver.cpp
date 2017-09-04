//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#include "tbb/mutex.h"

#include "boost/algorithm/string/predicate.hpp"

#include "IECoreImage/ImageDisplayDriver.h"

using namespace std;
using namespace boost;
using namespace Imath;
using namespace IECore;
using namespace IECoreImage;

IE_CORE_DEFINERUNTIMETYPED( ImageDisplayDriver );

const DisplayDriver::DisplayDriverDescription<ImageDisplayDriver> ImageDisplayDriver::g_description;

typedef std::map<std::string, ConstImagePrimitivePtr> ImagePool;
static ImagePool g_pool;
static tbb::mutex g_poolMutex;

ImageDisplayDriver::ImageDisplayDriver( const Box2i &displayWindow, const Box2i &dataWindow, const vector<string> &channelNames, ConstCompoundDataPtr parameters ) :
		DisplayDriver( displayWindow, dataWindow, channelNames, parameters ),
		m_image( new ImagePrimitive( dataWindow, displayWindow ) )
{
	for ( vector<string>::const_iterator it = channelNames.begin(); it != channelNames.end(); it++ )
	{
		m_image->createChannel<float>( *it );
	}
	if( parameters )
	{
		// Add all entries that follow our 'header:' metadata convention to the blindData.
		// Other entries are omitted.
		CompoundDataMap &xData = m_image->blindData()->writable();
		const CompoundDataMap &yData = parameters->readable();
		CompoundDataMap::const_iterator iterY = yData.begin();
		for ( ; iterY != yData.end(); iterY++ )
		{
			if( starts_with( iterY->first.string(), "header:" ) )
			{
				xData[ iterY->first.string().substr( 7 ) ] = iterY->second->copy();
			}
		}

		ConstStringDataPtr handle = parameters->member<StringData>( "handle" );
		if( handle )
		{
			tbb::mutex::scoped_lock lock( g_poolMutex );
			g_pool[handle->readable()] = m_image;
		}
	}
}

ImageDisplayDriver::~ImageDisplayDriver()
{
}

bool ImageDisplayDriver::scanLineOrderOnly() const
{
	return false;
}

bool ImageDisplayDriver::acceptsRepeatedData() const
{
	return true;
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

	int channelId, targetX, targetY, sourceWidth, sourceHeight, targetWidth;
	sourceWidth = box.max.x - box.min.x + 1;
	sourceHeight = box.max.y - box.min.y + 1;
	targetWidth = dataWindow.max.x - dataWindow.min.x + 1;
	channelId = 0;
	targetX = box.min.x - dataWindow.min.x;
	targetY = box.min.y - dataWindow.min.y;

	for( const auto &name : channelNames() )
	{
		vector< float > &target = boost::static_pointer_cast< FloatVectorData >( m_image->channels[name] )->writable();
		vector< float >::iterator targetIt;
		const float *sourceIt = data+channelId;
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

		++channelId;
	}
}

void ImageDisplayDriver::imageClose()
{
}

ConstImagePrimitivePtr ImageDisplayDriver::image() const
{
	return m_image;
}

ConstImagePrimitivePtr ImageDisplayDriver::storedImage( const std::string &handle )
{
	tbb::mutex::scoped_lock lock( g_poolMutex );
	ImagePool::const_iterator it = g_pool.find( handle );
	if( it != g_pool.end() )
	{
		return it->second;
	}
	return 0;
}

ConstImagePrimitivePtr ImageDisplayDriver::removeStoredImage( const std::string &handle )
{
	ConstImagePrimitivePtr result = 0;
	tbb::mutex::scoped_lock lock( g_poolMutex );
	ImagePool::iterator it = g_pool.find( handle );
	if( it != g_pool.end() )
	{
		result = it->second;
		g_pool.erase( it );
	}
	return result;
}
