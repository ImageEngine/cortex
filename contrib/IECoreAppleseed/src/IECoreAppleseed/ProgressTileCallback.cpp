//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Esteban Tovagliari. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//	 * Redistributions of source code must retain the above copyright
//	   notice, this list of conditions and the following disclaimer.
//
//	 * Redistributions in binary form must reproduce the above copyright
//	   notice, this list of conditions and the following disclaimer in the
//	   documentation and/or other materials provided with the distribution.
//
//	 * Neither the name of Image Engine Design nor the names of any
//	   other contributors to this software may be used to endorse or
//	   promote products derived from this software without specific prior
//	   written permission.
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

#include "foundation/image/image.h"
#include "foundation/utility/string.h"
#include "renderer/api/frame.h"
#include "renderer/api/log.h"

namespace asf = foundation;
namespace asr = renderer;

IECoreAppleseed::ProgressTileCallback::ProgressTileCallback()
	: m_renderedPixels( 0 )
{
}

void IECoreAppleseed::ProgressTileCallback::release()
{
	// We don't need to do anything here.
	// The tile callback factory deletes this instance.
}

void IECoreAppleseed::ProgressTileCallback::on_tile_end( const asr::Frame* frame, const size_t tileX, const size_t tileY )
{
	boost::mutex::scoped_lock lock( m_mutex );
	log_progress( frame, tileX, tileY );
}

void IECoreAppleseed::ProgressTileCallback::log_progress( const asr::Frame *frame, const size_t tileX, const size_t tileY )
{
	const size_t totalPixels = frame->image().properties().m_pixel_count;

	const asf::Tile& tile = frame->image().tile( tileX, tileY );
	m_renderedPixels += tile.get_pixel_count();

	RENDERER_LOG_INFO( "rendering, %s done", asf::pretty_percent( m_renderedPixels, totalPixels).c_str() );

	// Reset progress when rendering is finished for multi-pass renders.
	if( m_renderedPixels == totalPixels )
	{
		m_renderedPixels = 0;
	}
}

IECoreAppleseed::ProgressTileCallbackFactory::ProgressTileCallbackFactory()
	: m_callback( new ProgressTileCallback() )
{
}

void IECoreAppleseed::ProgressTileCallbackFactory::release()
{
	delete this;
}

asr::ITileCallback* IECoreAppleseed::ProgressTileCallbackFactory::create()
{
	return m_callback.get();
}
