//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, J3P LLC. All rights reserved.
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "IECoreNuke/DisplayIop.h"

#include "IECoreNuke/TypeIds.h"

#include "IECoreImage/ImageDisplayDriver.h"

#include "IECore/LRUCache.h"

#include "DDImage/Knobs.h"
#include "DDImage/Row.h"

#include "boost/bind.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/signal.hpp"

using namespace IECore;
using namespace IECoreImage;
using namespace IECoreNuke;
using namespace DD::Image;
using namespace Imath;

//////////////////////////////////////////////////////////////////////////
// DisplayDriverServer cache. Many nodes may all want to use a server on
// the same port. We therefore use an LRUCache to manage the lifetime of
// the servers and provide them to the nodes.
//////////////////////////////////////////////////////////////////////////

typedef LRUCache<int, DisplayDriverServerPtr> ServerCache;

// key is the port number for the server.
static DisplayDriverServerPtr serverCacheGetter( int key, size_t cost )
{
	cost = 1;
	return new DisplayDriverServer( key );
}

// max cost of 4 means we will never have more than 4 unused servers at any one time.
static ServerCache g_servers( serverCacheGetter, 4 );

//////////////////////////////////////////////////////////////////////////
// NukeDisplayDriver implementation
//////////////////////////////////////////////////////////////////////////

namespace IECoreNuke
{

class NukeDisplayDriver : public IECoreImage::ImageDisplayDriver
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( NukeDisplayDriver, NukeDisplayDriverTypeId, IECoreImage::ImageDisplayDriver );

		NukeDisplayDriver( const Imath::Box2i &displayWindow, const Imath::Box2i &dataWindow, const std::vector<std::string> &channelNames, ConstCompoundDataPtr parameters )
			:	ImageDisplayDriver( displayWindow, dataWindow, channelNames, parameters )
		{
			if( parameters )
			{
				m_parameters = parameters->copy();
			}
			else
			{
				m_parameters = new CompoundData;
			}

			instanceCreatedSignal( this );
		}

		virtual ~NukeDisplayDriver()
		{
		}

		/// Returns a copy of the parameters used in creating this instance. This
		/// is useful in recognising relevant instances in the instanceCreatedSignal.
		ConstCompoundDataPtr parameters() const
		{
			return m_parameters;
		}

		/// Updates the current image, and then emits the dataReceivedSignal.
		virtual void imageData( const Imath::Box2i &box, const float *data, size_t dataSize )
		{
			ImageDisplayDriver::imageData( box, data, dataSize );
			dataReceivedSignal( this, box );
		}

		/// This signal is emitted when a new NukeDisplayDriver has been created.
		/// This allows nuke nodes to pick up the new DisplayDrivers even when they're
		/// created in some other code, such as a DisplayDriverServer.
		typedef boost::signal<void( NukeDisplayDriver * )> InstanceCreatedSignal;
		static InstanceCreatedSignal instanceCreatedSignal;

		/// This signal is emitted when this NukeDisplayDriver instance receives new data.
		typedef boost::signal<void( NukeDisplayDriver *, const Imath::Box2i &box )> DataReceivedSignal;
		DataReceivedSignal dataReceivedSignal;

	private :

		static const DisplayDriverDescription<NukeDisplayDriver> g_description;

		ConstCompoundDataPtr m_parameters;

};

NukeDisplayDriver::InstanceCreatedSignal NukeDisplayDriver::instanceCreatedSignal;
const DisplayDriver::DisplayDriverDescription<NukeDisplayDriver> NukeDisplayDriver::g_description;

} // namespace IECoreNuke

//////////////////////////////////////////////////////////////////////////
// DisplayIop implementation
//////////////////////////////////////////////////////////////////////////

const DD::Image::Iop::Description DisplayIop::g_description( "ieDisplay", DisplayIop::build );

DisplayIop::DisplayIop( Node *node )
	:	Iop( node ), m_portNumber( 1559 ), m_server( g_servers.get( m_portNumber ) ), m_updateCount( 0 ), m_driver( 0 )
{
	inputs( 0 );
	slowness( 0 ); // disable caching as we're buffering everything internally ourselves
	NukeDisplayDriver::instanceCreatedSignal.connect( boost::bind( &DisplayIop::driverCreated, this, _1 ) );
}

DisplayIop::~DisplayIop()
{
	NukeDisplayDriver::instanceCreatedSignal.disconnect( boost::bind( &DisplayIop::driverCreated, this, _1 ) );
	connectToDriver( 0 );
}

const char *DisplayIop::Class() const
{
	return "ieDisplay";
}

const char *DisplayIop::node_help() const
{
	return "Acts as a framebuffer for external renderers.";
}

void DisplayIop::knobs( DD::Image::Knob_Callback f )
{
	Iop::knobs( f );

	Int_knob( f, &m_portNumber, "portNumber", "Port Number" );
	// we must have KNOB_CHANGED_RECURSIVE set otherwise nuke doesn't give us knob_changed()
	// calls when the knob value is changed from a knobChanged method of a PythonPanel.
	SetFlags( f, Knob::KNOB_CHANGED_ALWAYS | Knob::KNOB_CHANGED_RECURSIVE | Knob::NO_ANIMATION );
	Tooltip(
		f,
		"The port on which to receive images. This must match "
		"the port being used by the renderer to send images."
	);

}

int DisplayIop::knob_changed( DD::Image::Knob *knob )
{
	if( knob->is( "portNumber" ) )
	{
	 	int portNumber = (int)this->knob( "portNumber" )->get_value();
		m_server = g_servers.get( portNumber );
		return 1;
	}

	return Iop::knob_changed( knob );
}

void DisplayIop::append( DD::Image::Hash &hash )
{
	Iop::append( hash );

	hash.append( __DATE__ );
	hash.append( __TIME__ );
	hash.append( firstDisplayIop()->m_updateCount );
}

void DisplayIop::_validate( bool forReal )
{
	Box2i displayWindow( V2i( 0, 0 ), V2i( 255, 255 ) );

	if( firstDisplayIop()->m_driver )
	{
		displayWindow = firstDisplayIop()->m_driver->image()->getDisplayWindow();
	}

	m_format = m_fullSizeFormat = Format( displayWindow.size().x + 1, displayWindow.size().y + 1 );
	// these set function don't copy the format, but instead reference its address.
	// we therefore have to store the format as member data.
	info_.format( m_format );
	info_.full_size_format( m_fullSizeFormat );
	info_.set( m_format );

	info_.channels( Mask_RGBA );
}

void DisplayIop::engine( int y, int x, int r, const DD::Image::ChannelSet &channels, DD::Image::Row &row )
{
	Channel outputChannels[4] = { Chan_Red, Chan_Green, Chan_Blue, Chan_Alpha };
	const char *inputChannels[] = { "R", "G", "B", "A", nullptr };

	const ImagePrimitive *image = nullptr;
	Box2i inputDataWindow;
	Box2i inputDisplayWindow;
	if( firstDisplayIop()->m_driver )
	{
		image = firstDisplayIop()->m_driver->image().get();
		inputDataWindow = image->getDataWindow();
		inputDisplayWindow = image->getDisplayWindow();
	}

	int i = 0;
	while( inputChannels[i] )
	{
		const FloatVectorData *inputData = image ? image->getChannel<float>( inputChannels[i] ) : nullptr;
		if( inputData )
		{
			float *output = row.writable( outputChannels[i] ) + x;
			// remap coordinate relative to our data window. nuke images have pixel origin at bottom,
			// cortex images have pixel origin at top.
			V2i pDataWindow = V2i( x, inputDisplayWindow.max.y - y ) - inputDataWindow.min;
			const float *input = &((inputData->readable())[pDataWindow.y*(inputDataWindow.size().x+1) + pDataWindow.x]);
			memcpy( output, input, (r-x) * sizeof( float ) );
		}
		else
		{
			row.erase( outputChannels[i] );
		}
		i++;
	}
}

DD::Image::Op *DisplayIop::build( Node *node )
{
	return new DisplayIop( node );
}

DisplayIop *DisplayIop::firstDisplayIop()
{
	return static_cast<DisplayIop *>( firstOp() );
}

void DisplayIop::driverCreated( NukeDisplayDriver *driver )
{
	ConstStringDataPtr portNumber = driver->parameters()->member<StringData>( "displayPort" );
	if( portNumber && boost::lexical_cast<int>( portNumber->readable() ) == m_portNumber )
	{
		firstDisplayIop()->connectToDriver( driver );
	}
}

void DisplayIop::connectToDriver( NukeDisplayDriver *driver )
{
	assert( this == firstDisplayIop() );

	if( m_driver )
	{
		m_driver->dataReceivedSignal.disconnect( boost::bind( &DisplayIop::driverDataReceived, this, _1, _2 ) );
	}

	m_driver = driver;
	if( m_driver )
	{
		m_driver->dataReceivedSignal.connect( boost::bind( &DisplayIop::driverDataReceived, this, _1, _2 ) );
	}

	m_updateCount++;
	asapUpdate();
}

void DisplayIop::driverDataReceived( NukeDisplayDriver *driver, const Imath::Box2i &box )
{
	assert( this == firstDisplayIop() );
	m_updateCount++;
	asapUpdate();
}
