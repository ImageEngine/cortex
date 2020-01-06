//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
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

#include "IECoreAppleseed/ProgressTileCallback.h"

#include "IECoreImage/DisplayDriver.h"

#include "IECore/BoxAlgo.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "boost/thread/locks.hpp"
#include "boost/thread/mutex.hpp"

#include "foundation/image/image.h"
#include "foundation/utility/autoreleaseptr.h"
#include "renderer/api/aov.h"
#include "renderer/api/frame.h"
#include "renderer/api/log.h"
#include "renderer/api/rendering.h"
#include "renderer/api/utility.h"

#include <mutex>
#include <vector>

using namespace std;
using namespace Imath;
using namespace IECore;
using namespace IECoreImage;

namespace asf = foundation;
namespace asr = renderer;

namespace IECoreAppleseed
{

class DisplayLayer
{
	public :

		DisplayLayer( const char *name, const asf::Dictionary& params ) : m_image( nullptr ), m_layerName( name ), m_params( params )
		{
		}

		~DisplayLayer()
		{
			if( m_driver )
			{
				try
				{
					m_driver->imageClose();
				}
				catch( const std::exception &e )
				{
					msg( Msg::Error, "ieDisplay:delete layer", e.what() );
				}
			}
		}

		void initDisplay( const asr::Frame *frame )
		{
			LockGuard lock( m_mutex );

			if( m_image )
			{
				// Already initialised
				return;
			}

			const asf::CanvasProperties &frameProps = frame->image().properties();
			const asf::AABB2u &cropWindow = frame->get_crop_window();

			m_tileWidth = frameProps.m_tile_width;
			m_tileHeight = frameProps.m_tile_height;
			m_dataWindow = Box2i( V2i( cropWindow.min[0], cropWindow.min[1] ), V2i( cropWindow.max[0], cropWindow.max[1] ) );

			std::vector<std::string> channelNames;

			if( m_layerName == "beauty" )
			{
				m_image = &frame->image();
				m_channelCount = 4;
				channelNames.push_back( "R" );
				channelNames.push_back( "G" );
				channelNames.push_back( "B" );
				channelNames.push_back( "A" );
			}
			else
			{
				const asr::AOV *aov = frame->aovs().get_by_name( m_layerName.c_str() );
				assert( aov );

				m_image = &aov->get_image();
				m_channelCount = aov->get_channel_count();

				string aovChannelNamePrefix = m_layerName + ".";
				for( size_t i = 0; i < m_channelCount; ++i )
				{
					channelNames.push_back( aovChannelNamePrefix + aov->get_channel_names()[i] );
				}
			}

			// Convert parameters.
			CompoundDataPtr parameters = new CompoundData();
			CompoundDataMap &p = parameters->writable();

			for( asf::StringDictionary::const_iterator it = m_params.strings().begin(), eIt = m_params.strings().end(); it != eIt; ++it )
			{
				p[ it.key() ] = new StringData( it.value() );
			}

			// Create the driver.
			const Box2i displayWindow( V2i( 0, 0 ), V2i( frameProps.m_canvas_width - 1, frameProps.m_canvas_height - 1 ) );
			try
			{
				m_driver = IECoreImage::DisplayDriver::create( m_params.get( "driverType" ), displayWindow, m_dataWindow, channelNames, parameters );
			}
			catch( const std::exception &e )
			{
				msg( Msg::Error, "ieDisplay:init layer display", e.what() );
			}

			// reserve space for one tile
			m_buffer.reserve( m_tileWidth * m_tileHeight * m_channelCount );
		}

		void highlightRegion( const size_t x, const size_t y, const size_t width, const size_t height )
		{
			LockGuard lock( m_mutex );

			const int inset = 0;
			Box2i bucketBox( boxInsideDataWindow( x + inset, y + inset, width - inset, height - inset ) );

			if( bucketBox.size().x == 0 || bucketBox.size().y == 0)
				return;

			m_buffer.clear();

			for( int i = bucketBox.min.x; i <= bucketBox.max.x; ++i )
			{
				for( size_t k = 0; k < m_channelCount; ++k )
				{
					m_buffer.push_back( 1.0f );
				}
			}

			writeBuffer( Box2i( V2i( bucketBox.min.x, bucketBox.min.y ), V2i( bucketBox.max.x, bucketBox.min.y ) ) );
			writeBuffer( Box2i( V2i( bucketBox.min.x, bucketBox.max.y ), V2i( bucketBox.max.x, bucketBox.max.y ) ) );

			m_buffer.clear();

			for( int i = bucketBox.min.y; i <= bucketBox.max.y; ++i )
			{
				for( size_t k = 0; k < m_channelCount; ++k )
				{
					m_buffer.push_back( 1.0f );
				}
			}

			writeBuffer( Box2i( V2i( bucketBox.min.x, bucketBox.min.y ), V2i( bucketBox.min.x, bucketBox.max.y ) ) );
			writeBuffer( Box2i( V2i( bucketBox.max.x, bucketBox.min.y ), V2i( bucketBox.max.x, bucketBox.max.y ) ) );
		}

		void writeTile( const size_t tileX, const size_t tileY )
		{
			LockGuard lock( m_mutex );

			const asf::Tile &tile = m_image->tile( tileX, tileY );

			int x0 = tileX * m_tileWidth;
			int y0 = tileY * m_tileHeight;
			Box2i bucketBox( boxInsideDataWindow( x0, y0, m_tileWidth, m_tileHeight ) );

			m_buffer.clear();

			for( int j = bucketBox.min.y; j <= bucketBox.max.y; ++j )
			{
				int y = j - y0;

				for( int i = bucketBox.min.x; i <= bucketBox.max.x; ++i )
				{
					int x = i - x0;

					for( size_t k = 0; k < m_channelCount; ++k )
					{
						m_buffer.push_back( tile.get_component<float>( x, y, k ) );
					}
				}
			}

			writeBuffer( bucketBox );
		}

	private :

		void writeBuffer( const Box2i &bucketBox ) const
		{
			try
			{
				// Don't send anything to the Driver if there are no pixels.
				if( !m_buffer.empty() )
				{
					m_driver->imageData( bucketBox, &m_buffer.front(), m_buffer.size() );
				}
			}
			catch( const std::exception &e )
			{
				msg( Msg::Error, "ieDisplay:write_buffer", e.what() );
			}
		}

		Box2i boxInsideDataWindow( int x, int y, int w, int h ) const
		{
			int x1 = x + w - 1;
			int y1 = y + h - 1;

			return Box2i( V2i( std::max( x , m_dataWindow.min.x ), std::max( y , m_dataWindow.min.y ) ),
						  V2i( std::min( x1, m_dataWindow.max.x ), std::min( y1, m_dataWindow.max.y ) ) );
		}

		using Mutex = std::mutex;
		using LockGuard = std::lock_guard<Mutex>;

		// Our public methods will be called concurrently,
		// so we use this mutex to protect access to our
		// members.
		Mutex m_mutex;
		DisplayDriverPtr m_driver;
		asf::Image *m_image;
		Box2i m_dataWindow;
		vector<float> m_buffer;
		size_t m_tileWidth;
		size_t m_tileHeight;
		size_t m_channelCount;
		string m_layerName;
		asf::Dictionary m_params;

};

using DisplayLayers = vector<unique_ptr<DisplayLayer>>;
using DisplayLayersPtr = shared_ptr<DisplayLayers>;

class DisplayTileCallback : public ProgressTileCallback
{

	public :

		explicit DisplayTileCallback( const DisplayLayersPtr &layers )
			:	m_layers( layers )
		{
		}

		void release() override
		{
			delete this;
		}

		void on_tile_begin(const asr::Frame *frame, const size_t tileX, const size_t tileY) override
		{
			ProgressTileCallback::on_tile_begin( frame, tileX, tileY );

			const asf::CanvasProperties &props = frame->image().properties();
			const size_t x = tileX * props.m_tile_width;
			const size_t y = tileY * props.m_tile_height;
			for( const auto &layer : *m_layers )
			{
				layer->initDisplay( frame );
				layer->highlightRegion( x, y, props.m_tile_width, props.m_tile_height );
			}
		}

		void on_tile_end(const asr::Frame *frame, const size_t tileX, const size_t tileY) override
		{
			ProgressTileCallback::on_tile_end( frame, tileX, tileY );

			for( const auto &layer : *m_layers )
			{
				layer->initDisplay( frame );
				layer->writeTile( tileX, tileY );
			}
		}

		void on_progressive_frame_update( const asr::Frame* frame ) override
		{
			const asf::CanvasProperties &frame_props = frame->image().properties();

			for( size_t ty = 0; ty < frame_props.m_tile_count_y; ++ty )
			{
				for( size_t tx = 0; tx < frame_props.m_tile_count_x; ++tx )
				{
					for( const auto &layer : *m_layers )
					{
						layer->initDisplay( frame );
						layer->writeTile( tx, ty );
					}
				}
			}
		}

	private :

		DisplayLayersPtr m_layers;

};

class DisplayTileCallbackFactory : public asr::ITileCallbackFactory
{

	public:

		explicit DisplayTileCallbackFactory( const asr::ParamArray &params )
			:	m_layers( make_shared<DisplayLayers>() )
		{
			m_layers->reserve( params.dictionaries().size() );
			for( auto it( params.dictionaries().begin() ), eIt( params.dictionaries().end() ); it != eIt; ++it )
			{
				m_layers->emplace_back( new DisplayLayer( it.key(), it.value() ) );
			}
		}

		~DisplayTileCallbackFactory() override
		{
		}

		// Delete this instance.
		void release() override
		{
			delete this;
		}

		// Appleseed calls this to create tile callbacks for writing
		// out the image. It is called once per-thread at the beginning
		// of a final frame (non-progressive) render, and just once at
		// the beginning of an interactive progressive render. For the
		// per-thread case, we need to ensure that all the callbacks we
		// return are writing to the same `IECoreImage::DisplayDrivers`.
		// So the DisplayTileCallbacks that we return share the `m_layers`
		// that we have created.
		asr::ITileCallback *create() override
		{
			return new DisplayTileCallback( m_layers );
		}

	private:

		const DisplayLayersPtr m_layers;

};

}

extern "C"
{

// Display plugin entry point.
IECOREAPPLESEED_API asr::ITileCallbackFactory* create_tile_callback_factory( const asr::ParamArray *params );
asr::ITileCallbackFactory* create_tile_callback_factory( const asr::ParamArray *params )
{
	return new IECoreAppleseed::DisplayTileCallbackFactory( *params );
}

}
