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

#include "renderer/api/rendering.h"

#include "IECore/DisplayDriver.h"
#include "IECore/BoxAlgo.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/MessageHandler.h"

#include "foundation/image/image.h"
#include "foundation/utility/autoreleaseptr.h"
#include "renderer/api/frame.h"
#include "renderer/api/utility.h"

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include <vector>

using namespace IECore;
using namespace Imath;
using namespace boost;
using namespace std;

namespace asf = foundation;
namespace asr = renderer;

namespace IECoreAppleseed
{

class DisplayTileCallback : public asr::TileCallbackBase
{

	public :

		explicit DisplayTileCallback( const asr::ParamArray &params ) : m_params( params )
		{
			m_displayInitialized = false;
		}

		virtual ~DisplayTileCallback()
		{
			if( m_driver )
			{
				try
				{
					m_driver->imageClose();
				}
				catch( const std::exception &e )
				{
					// we have to catch and report exceptions because letting them out into pure c land
					// just causes aborts.
					msg( Msg::Error, "ieOutputDriver:driverClose", e.what() );
				}
			}
		}

		virtual void release()
		{
			// we don't need to do anything here.
			// the tile callback factory deletes this instance.
		}

		/// This method is called after a tile is rendered (final rendering).
		virtual void post_render_tile( const asr::Frame *frame, const size_t tileX, const size_t tileY )
		{
			boost::lock_guard<boost::mutex> guard( m_mutex );

			if( !m_displayInitialized )
			{
				initDisplay( frame );
			}

			write_tile( frame, tileX, tileY );
		}

		/// This method is called after a whole frame is rendered (progressive rendering).
		virtual void post_render( const asr::Frame *frame )
		{
			boost::lock_guard<boost::mutex> guard( m_mutex );

			if( !m_displayInitialized )
			{
				initDisplay( frame );
			}

			const asf::CanvasProperties &frame_props = frame->image().properties();

			for( size_t ty = 0; ty < frame_props.m_tile_count_y; ++ty )
			{
				for( size_t tx = 0; tx < frame_props.m_tile_count_x; ++tx )
				{
					write_tile( frame, tx, ty );
				}
			}
		}

	private:

		void initDisplay( const asr::Frame *frame )
		{
			assert( !m_displayInitialized );

			Box2i displayWindow( V2i( 0, 0 ), V2i( frame->image().properties().m_canvas_width - 1, frame->image().properties().m_canvas_height - 1 ) );

			m_dataWindow = Box2i(  V2i( frame->get_crop_window().min[0], frame->get_crop_window().min[1] ),
								   V2i( frame->get_crop_window().max[0], frame->get_crop_window().max[1] ) );

			std::vector<std::string> channelNames;

			if( frame->image().properties().m_channel_count >= 1 )
			{
				channelNames.push_back( "R" );
			}

			if( frame->image().properties().m_channel_count >= 3 )
			{
				channelNames.push_back( "G" );
				channelNames.push_back( "B" );
			}

			if( frame->image().properties().m_channel_count == 4 )
			{
				channelNames.push_back( "A" );
			}

			CompoundDataPtr parameters = new CompoundData();
			CompoundDataMap &p = parameters->writable();
			p["displayHost"] = new StringData( m_params.get( "displayHost" ) );
			p["displayPort"] = new StringData( m_params.get( "displayPort" ) );
			p["driverType"] = new StringData( m_params.get( "driverType" ) );
			p["remoteDisplayType"] = new StringData( m_params.get( "remoteDisplayType" ) );
			p["type"] = new StringData( m_params.get( "type" ) );

			try
			{
				m_driver = IECore::DisplayDriver::create( m_params.get( "driverType" ), displayWindow, m_dataWindow, channelNames, parameters );
			}
			catch( const std::exception &e )
			{
				// we have to catch and report exceptions because letting them out into pure c land
				// just causes aborts.
				msg( Msg::Error, "ieOutputDriver:driverOpen", e.what() );
			}

			m_displayInitialized = true;
		}

		void write_tile( const asr::Frame *frame, const size_t tileX, const size_t tileY )
		{
			const asf::CanvasProperties &frameProps = frame->image().properties();
			const asf::Tile &tile = frame->image().tile( tileX, tileY );

			int x0 = tileX * frameProps.m_tile_width;
			int y0 = tileY * frameProps.m_tile_height;
			int x1 = x0 + frameProps.m_tile_width - 1;
			int y1 = y0 + frameProps.m_tile_height - 1;

			Box2i bucketBox( V2i( std::max( x0, m_dataWindow.min.x ), std::max( y0, m_dataWindow.min.y ) ),
							 V2i( std::min( x1, m_dataWindow.max.x ), std::min( y1, m_dataWindow.max.y ) ) );

			try
			{
				m_buffer.clear();
				m_buffer.reserve( bucketBox.size().x * bucketBox.size().y * frameProps.m_channel_count );

				for( int j = bucketBox.min.y; j <= bucketBox.max.y; ++j )
				{
					int y = j - y0;

					for( int i = bucketBox.min.x; i <= bucketBox.max.x; ++i )
					{
						int x = i - x0;

						for( size_t k = 0; k < frameProps.m_channel_count; ++k )
						{
							m_buffer.push_back( tile.get_component<float>( x, y, k ) );
						}
					}
				}

				m_driver->imageData( bucketBox, &m_buffer.front(), m_buffer.size() );
			}
			catch( const std::exception &e )
			{
				// we have to catch and report exceptions because letting them out into pure c land
				// just causes aborts.
				msg( Msg::Error, "ieOutputDriver:driverWriteBucket", e.what() );
			}
		}

		asr::ParamArray m_params;
		mutex m_mutex;
		bool m_displayInitialized;
		DisplayDriverPtr m_driver;
		Box2i m_dataWindow;
		vector<float> m_buffer;
};

class DisplayTileCallbackFactory : public asr::ITileCallbackFactory
{

	public:

		explicit DisplayTileCallbackFactory( const asr::ParamArray &params )
		{
			m_callback = new DisplayTileCallback( params );
		}

		virtual ~DisplayTileCallbackFactory()
		{
			delete m_callback;
		}

		// Delete this instance.
		virtual void release()
		{
			delete this;
		}

		// Return a new tile callback instance.
		virtual asr::ITileCallback *create()
		{
			return m_callback;
		}

	private:

		DisplayTileCallback *m_callback;

};

}

extern "C"
{

/// Display plugin entry point.
asr::ITileCallbackFactory* create_tile_callback_factory( const asr::ParamArray *params )
{
	return new IECoreAppleseed::DisplayTileCallbackFactory( *params );
}

}
