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
#include "renderer/api/version.h"

#include <vector>

using namespace std;
using namespace boost;
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

		void init_display( const asr::Frame *frame, const Box2i &displayWindow, const Box2i &dataWindow )
		{
			const asf::CanvasProperties &frameProps = frame->image().properties();
			m_tile_width = frameProps.m_tile_width;
			m_tile_height = frameProps.m_tile_height;
			m_data_window = dataWindow;

			std::vector<std::string> channelNames;

			if( m_layerName == "beauty" )
			{
				m_image = &frame->image();
				m_channel_count = 4;
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
				m_channel_count = aov->get_channel_count();

				string aovChannelNamePrefix = m_layerName + ".";
				for( size_t i = 0; i < m_channel_count; ++i )
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
			try
			{
				m_driver = IECoreImage::DisplayDriver::create( m_params.get( "driverType" ), displayWindow, m_data_window, channelNames, parameters );
			}
			catch( const std::exception &e )
			{
				msg( Msg::Error, "ieDisplay:init layer display", e.what() );
			}

			// reserve space for one tile
			m_buffer.reserve( m_tile_width * m_tile_height * m_channel_count );
		}

		void hightlight_region( const size_t x, const size_t y, const size_t width, const size_t height )
		{
			const int inset = 0;
			Box2i bucketBox( box_inside_data_window( x + inset, y + inset, width - inset, height - inset ) );

			if( bucketBox.size().x == 0 || bucketBox.size().y == 0)
				return;

			m_buffer.clear();

			for( int i = bucketBox.min.x; i <= bucketBox.max.x; ++i )
			{
				for( size_t k = 0; k < m_channel_count; ++k )
				{
					m_buffer.push_back( 1.0f );
				}
			}

			write_buffer( Box2i( V2i( bucketBox.min.x, bucketBox.min.y ), V2i( bucketBox.max.x, bucketBox.min.y ) ) );
			write_buffer( Box2i( V2i( bucketBox.min.x, bucketBox.max.y ), V2i( bucketBox.max.x, bucketBox.max.y ) ) );

			m_buffer.clear();

			for( int i = bucketBox.min.y; i <= bucketBox.max.y; ++i )
			{
				for( size_t k = 0; k < m_channel_count; ++k )
				{
					m_buffer.push_back( 1.0f );
				}
			}

			write_buffer( Box2i( V2i( bucketBox.min.x, bucketBox.min.y ), V2i( bucketBox.min.x, bucketBox.max.y ) ) );
			write_buffer( Box2i( V2i( bucketBox.max.x, bucketBox.min.y ), V2i( bucketBox.max.x, bucketBox.max.y ) ) );
		}

		void write_tile( const size_t tileX, const size_t tileY )
		{
			const asf::Tile &tile = m_image->tile( tileX, tileY );

			int x0 = tileX * m_tile_width;
			int y0 = tileY * m_tile_height;
			Box2i bucketBox( box_inside_data_window( x0, y0, m_tile_width, m_tile_height ) );

			m_buffer.clear();

			for( int j = bucketBox.min.y; j <= bucketBox.max.y; ++j )
			{
				int y = j - y0;

				for( int i = bucketBox.min.x; i <= bucketBox.max.x; ++i )
				{
					int x = i - x0;

					for( size_t k = 0; k < m_channel_count; ++k )
					{
						m_buffer.push_back( tile.get_component<float>( x, y, k ) );
					}
				}
			}

			write_buffer( bucketBox );
		}

	private :

		void write_buffer( const Box2i &bucketBox ) const
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

		Box2i box_inside_data_window( int x, int y, int w, int h ) const
		{
			int x1 = x + w - 1;
			int y1 = y + h - 1;

			return Box2i( V2i( std::max( x , m_data_window.min.x ), std::max( y , m_data_window.min.y ) ),
						  V2i( std::min( x1, m_data_window.max.x ), std::min( y1, m_data_window.max.y ) ) );
		}

		DisplayDriverPtr m_driver;
		asf::Image *m_image;
		Box2i m_data_window;
		vector<float> m_buffer;
		size_t m_tile_width;
		size_t m_tile_height;
		size_t m_channel_count;
		string m_layerName;
		asf::Dictionary m_params;

};

class DisplayTileCallback : public ProgressTileCallback
{

	public :

		explicit DisplayTileCallback( const asr::ParamArray &params ) : ProgressTileCallback(), m_displays_initialized( false )
		{
			// Create display layers.
			m_layers.reserve( params.dictionaries().size() );
			for( auto it( params.dictionaries().begin() ), eIt( params.dictionaries().end() ); it != eIt; ++it )
			{
				m_layers.push_back( new DisplayLayer( it.key(), it.value() ) );
			}
		}

		~DisplayTileCallback() override
		{
			for( DisplayLayer *layer : m_layers )
			{
				delete layer;
			}
		}

		void release() override
		{
			// We don't need to do anything here.
			// The tile callback factory deletes this instance.
		}

		void on_tile_begin(const asr::Frame *frame, const size_t tileX, const size_t tileY) override
		{
			const asf::CanvasProperties &props = frame->image().properties();
			const size_t x = tileX * props.m_tile_width;
			const size_t y = tileY * props.m_tile_height;

			boost::lock_guard<boost::mutex> guard( m_mutex );

			if( m_displays_initialized )
			{
				for( DisplayLayer *layer : m_layers )
				{
					layer->hightlight_region( x, y, props.m_tile_width, props.m_tile_height );
				}
			}
		}

		void on_tile_end(const asr::Frame *frame, const size_t tileX, const size_t tileY) override
		{
			boost::lock_guard<boost::mutex> guard( m_mutex );

			init_layer_displays( frame );

			for( DisplayLayer *layer : m_layers )
			{
				layer->write_tile( tileX, tileY );
			}

			log_progress( frame, tileX, tileY );
		}

		void on_progressive_frame_update( const asr::Frame* frame ) override
		{
			boost::lock_guard<boost::mutex> guard( m_mutex );

			init_layer_displays( frame );

			const asf::CanvasProperties &frame_props = frame->image().properties();

			for( size_t ty = 0; ty < frame_props.m_tile_count_y; ++ty )
			{
				for( size_t tx = 0; tx < frame_props.m_tile_count_x; ++tx )
				{
					for( DisplayLayer *layer : m_layers )
					{
						layer->write_tile( tx, ty );
					}
				}
			}
		}

	private :

		void init_layer_displays( const asr::Frame* frame )
		{
			if( !m_displays_initialized )
			{
				const asf::CanvasProperties &frameProps = frame->image().properties();

				Box2i displayWindow( V2i( 0, 0 ), V2i( frameProps.m_canvas_width - 1, frameProps.m_canvas_height - 1 ) );

				const asf::AABB2u &cropWindow = frame->get_crop_window();
				Box2i dataWindow = Box2i( V2i( cropWindow.min[0], cropWindow.min[1] ), V2i( cropWindow.max[0], cropWindow.max[1] ) );

				for( DisplayLayer *layer : m_layers )
				{
					layer->init_display( frame, displayWindow, dataWindow );
				}

				m_displays_initialized = true;
			}
		}

		std::vector<DisplayLayer*> m_layers;
		bool m_displays_initialized;
};

class DisplayTileCallbackFactory : public asr::ITileCallbackFactory
{

	public:

		explicit DisplayTileCallbackFactory( const asr::ParamArray &params )
		{
			m_callback = new DisplayTileCallback( params );
		}

		~DisplayTileCallbackFactory() override
		{
			delete m_callback;
		}

		// Delete this instance.
		void release() override
		{
			delete this;
		}

		// Return a new tile callback instance.
		asr::ITileCallback *create() override
		{
			return m_callback;
		}

	private:

		DisplayTileCallback *m_callback;

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
