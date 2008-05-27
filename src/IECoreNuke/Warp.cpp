//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreNuke/Warp.h"

#include "DDImage/Knobs.h"
#include "DDImage/Pixel.h"
#include "DDImage/Row.h"
#include "DDImage/ARRAY.h"
#include "DDImage/Tile.h"

using namespace IECoreNuke;
using namespace DD::Image;
using namespace Imath;

Warp::Warp( Node *node )
	:	Iop( node ), m_channels( Mask_RGBA )
{
}

Warp::~Warp()
{
}

void Warp::knobs( DD::Image::Knob_Callback f )
{
	Iop::knobs( f );
	ChannelSet_knob( f, &m_channels, "channels", "Channels" );
	Tooltip( f, "The channels to apply the warp to - other channels are passed through unchanged." );
}

void Warp::_validate( bool forReal )
{
	set_out_channels( m_channels );
	Iop::_validate( forReal );
}

void Warp::_request( int x, int y, int r, int t, const DD::Image::ChannelSet &channels, int count )
{
	// request the whole damn thing - we've no idea what the warp will do.	
	input( 0 )->request( channels, count );
}

void Warp::engine( int y, int x, int r, const DD::Image::ChannelSet &channels, DD::Image::Row &out )
{
	// get the data for anything we don't plan on warping into the output row
	ChannelSet unchangingChannels = channels; unchangingChannels -= out_channels();
	input( 0 )->get( y, x, r, unchangingChannels, out );
	if( Op::aborted() )
	{
		return;
	}
	
	// now write into the warp channels
	///////////////////////////////////
	
	// storing an array of pointers to the writable data seems marginally faster
	// than calling Row::writable() in the inner loop.
	ChannelSet changingChannels = out_channels(); changingChannels &= channels;
	ARRAY( float *, writableChannels, (changingChannels.last() + 1) );
	for( Channel c=changingChannels.first(); c; c=changingChannels.next( c ) )
	{
		writableChannels[c] = out.writable( c );
	}
		
	int xEnd = x + (r - x);
	Pixel pixel( changingChannels );
	for( ; x<xEnd; x++ )
	{
		V2f p = warp( V2f( x, y ) );
	
		/// \todo Figure out a better filter area using adjacent warp() results.
		input0().sample( p.x + 0.5, p.y + 0.5, 1.0f, 1.0f, pixel );
		
		for( Channel c=changingChannels.first(); c; c=changingChannels.next( c ) )
		{
			writableChannels[c][x] = pixel[c];
		}
	}
}

