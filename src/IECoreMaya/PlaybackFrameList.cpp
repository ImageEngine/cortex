//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "maya/MAnimControl.h"
#include "maya/MTime.h"

#include "IECoreMaya/PlaybackFrameList.h"

using namespace IECore;
using namespace IECoreMaya;

IE_CORE_DEFINERUNTIMETYPED( PlaybackFrameList );

FrameList::Parser< PlaybackFrameList > PlaybackFrameList::g_parserRegistrar;

PlaybackFrameList::PlaybackFrameList( PlaybackFrameList::Range r ) : m_range( r )
{
}

PlaybackFrameList::~PlaybackFrameList()
{
}

PlaybackFrameList::Range PlaybackFrameList::getRange() const
{
	return m_range;
}

void PlaybackFrameList::setRange( Range r )
{
	m_range = r;
}

void PlaybackFrameList::asList( std::vector<Frame> &frames ) const
{
	frames.clear();

	MTime start, end;

	if ( m_range == Animation )
	{
		start = MAnimControl::animationStartTime();
		end = MAnimControl::animationEndTime();
	}
	else if ( m_range == Playback )
	{
		start = MAnimControl::minTime();
		end = MAnimControl::maxTime();
	}
	else
	{
		assert( false );
	}

	Frame startFrame = static_cast<Frame>( start.as( MTime::uiUnit() ) );
	Frame endFrame = static_cast<Frame>( end.as( MTime::uiUnit() ) );

	for ( Frame i = startFrame; i <= endFrame; i++ )
	{
		frames.push_back( i );
	}
}

std::string PlaybackFrameList::asString() const
{
	if ( m_range == Animation )
	{
		return "animation";
	}
	else
	{
		assert( m_range == Playback );
		return "playback";
	}
}

FrameListPtr PlaybackFrameList::copy() const
{
	return new PlaybackFrameList( m_range );
}

bool PlaybackFrameList::isEqualTo( ConstFrameListPtr other ) const
{
	if ( !FrameList::isEqualTo( other ) )
	{
		return false;
	}

	ConstPlaybackFrameListPtr otherF = assertedStaticCast< const PlaybackFrameList >( other );

	return m_range == otherF->m_range;
}

FrameListPtr PlaybackFrameList::parse( const std::string &frameList )
{
	if ( frameList == "animation" )
	{
		return new PlaybackFrameList( Animation );
	}
	else if ( frameList == "playback" )
	{
		return new PlaybackFrameList( Playback );
	}
	return 0;
}
