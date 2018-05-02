//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECore/Timer.h"

#include "IECore/Exception.h"
#include "IECore/MessageHandler.h"

#include <iostream>

using namespace IECore;
using namespace boost;

namespace
{

boost::timer::nanosecond_type time( const boost::timer::cpu_times &cpuTimes, Timer::Mode mode )
{
	switch( mode )
	{
		case Timer::SystemCPU:
			return cpuTimes.system;
		case Timer::UserCPU:
			return cpuTimes.user;
		case Timer::WallClock:
			return cpuTimes.wall;
		default:
			throw( Exception( "Invalid Timer Mode" ) );
	}
}

} // namespace

Timer::Timer( bool start, Timer::Mode mode ) : m_running( start ), m_accumulated( 0.0 ), m_mode( mode )
{
	if( !m_running )
	{
		m_timer.stop();
	}
}

void Timer::start()
{
	if( m_running )
	{
		throw( Exception( "Timer already started." ) );
	}
	m_running = true;
	m_timer.start();
}

double Timer::stop()
{
	if( !m_running )
	{
		throw( Exception( "Timer not started yet." ) );
	}
	double e = time( m_timer.elapsed(), m_mode ) / 1e9;
	m_timer.stop();
	m_accumulated += e;
	m_running = false;
	return e;
}

bool Timer::running()
{
	return m_running;
}

double Timer::currentElapsed() const
{
	if( m_running )
	{
		return time( m_timer.elapsed(), m_mode ) / 1e9;
	}
	return 0.0;
}

double Timer::totalElapsed() const
{
	return m_accumulated + currentElapsed();
}

ScopedTimer::ScopedTimer( const std::string &name, const std::string &channel ) : m_timer( true, IECore::Timer::WallClock ), m_name( name ), m_channel( channel ), m_depth(s_depth)
{
	if( m_channel != "" )
	{
		auto it = s_channelMap.find( m_channel );
		if( it == s_channelMap.end() )
		{
			s_channelMap.emplace( m_channel, 0.0 );
		}
	}

	s_depth++;
}

ScopedTimer::~ScopedTimer()
{
	std::string depthString = std::string( m_depth, ' ' );
	s_depth -= 1;

	if( m_channel != "" )
	{
		s_channelMap[m_channel] += m_timer.currentElapsed();
		return;
	}

	IECore::msg(
		IECore::MessageHandler::Debug,
		"ScopedTimer",
		boost::str( boost::format( "%1%[timed block] name: '%2%' time: %3% ms" ) % depthString % m_name % (m_timer.currentElapsed() * 1000.0) )
	);

}

void ScopedTimer::printChannel( const Channel &channel )
{
	auto it = s_channelMap.find( channel );
	if( it == s_channelMap.end() )
	{
		throw( Exception( boost::str( boost::format( "Channel %1% does not exists." ) % channel ) ) );
	}
	else
	{
		double time = (*it).second;
		IECore::msg(
			IECore::MessageHandler::Debug,
			"ScopedTimer",
			boost::str( boost::format( "[timed block] channel: %1% time: %2% ms" ) % channel % ( time * 1000.0 ) )
		);
	}
}

void ScopedTimer::printAllChannels()
{
	for( const auto& channelPair : s_channelMap )
	{
		IECore::msg(
			IECore::MessageHandler::Debug,
			"ScopedTimer",
			boost::str( boost::format( "[timed block] channel: %1% time: %2% ms" ) % channelPair.first % ( channelPair.second * 1000.0 ) )
		);
	}
}

void ScopedTimer::resetChannel( const Channel &channel )
{
	auto it = s_channelMap.find( channel );
	if( it == s_channelMap.end() )
	{
		throw( Exception( boost::str( boost::format( "Channel %1% does not exists." ) % channel ) ) );
	}
	else
	{
		s_channelMap[channel] = 0.0;
	}
}

void ScopedTimer::resetAllChannels()
{
	for( const auto& channelPair : s_channelMap )
	{
		s_channelMap[channelPair.first] = 0.0;
	}
}

std::map<ScopedTimer::Channel, double> ScopedTimer::s_channelMap;
size_t ScopedTimer::s_depth = 0;
