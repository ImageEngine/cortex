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

#include "IECore/FrameRange.h"

#include "IECore/Exception.h"

#include "boost/format.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"

#include <algorithm>

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( FrameRange );

FrameList::Parser< FrameRange > FrameRange::g_parserRegistrar;

FrameRange::FrameRange( Frame start, Frame end, Frame step ) : m_start( start ), m_end( end ), m_step( step )
{
	if ( start > end )
	{
		throw Exception( "FrameRange start must be less than or equal to end." );
	}
	if ( step == 0 )
	{
		throw Exception( "FrameRange step cannot be zero" );
	}
}

FrameRange::~FrameRange()
{
}

FrameList::Frame FrameRange::getStart()
{
	return m_start;
}
void FrameRange::setStart( Frame start )
{
	if ( start > m_end )
	{
		throw Exception( "FrameRange start must be less than or equal to end." );
	}
	m_start = start;
}

FrameList::Frame FrameRange::getEnd()
{
	return m_end;
}

void FrameRange::setEnd( Frame end )
{
	if ( end < m_start )
	{
		throw Exception( "FrameRange end must be greater than or equal to start." );
	}
	m_end = end;
}

FrameList::Frame FrameRange::getStep()
{
	return m_step;
}

void FrameRange::setStep( Frame step )
{
	if ( step == 0 )
	{
		throw Exception( "FrameRange step cannot be zero" );
	}
	m_step = step;
}

void FrameRange::asList( std::vector<Frame> &frames ) const
{
	frames.clear();
	for ( Frame f = m_start; f <= m_end; f += m_step )
	{
		frames.push_back( f );
	}
}

std::string FrameRange::asString() const
{
	if ( m_step != 1 )
	{
		return ( boost::format( "%d-%dx%d") % m_start % m_end % m_step ).str();
	}
	else if ( m_start != m_end )
	{
		return ( boost::format( "%d-%d") % m_start % m_end ).str();
	}
	else
	{
		return ( boost::format( "%d") % m_start ).str();
	}
}

bool FrameRange::isEqualTo( ConstFrameListPtr other ) const
{
	if ( !FrameList::isEqualTo( other ) )
	{
		return false;
	}

	ConstFrameRangePtr otherF = assertedStaticCast< const FrameRange >( other );

	return m_start == otherF->m_start && m_end == otherF->m_end && m_step == otherF->m_step ;
}

FrameListPtr FrameRange::copy() const
{
	return new FrameRange( m_start, m_end, m_step );
}

FrameListPtr FrameRange::parse( const std::string &frameList )
{
	try
	{
		Frame i = boost::lexical_cast<Frame>( frameList );
		return new FrameRange( i, i );
	}
	catch( const boost::bad_lexical_cast & )
	{
	}

	boost::regex re( "^(-?[0-9]+)(-)(-?[0-9]+)(x-?[0-9]+)?$" );
	boost::smatch matches;
	if ( boost::regex_match( frameList, matches, re ) )
	{
		Frame start = boost::lexical_cast<Frame>( std::string( matches[1].first, matches[1].second ) );
		Frame end = boost::lexical_cast<Frame>( std::string( matches[3].first, matches[3].second ) );
		Frame step = 1;

		if ( matches.size() > 4 && matches[4].matched )
		{
			/// The +1 is there because we want to skip the "x"
			step = boost::lexical_cast<Frame>( std::string( matches[4].first + 1, matches[4].second ) );
		}

		return new FrameRange( start, end, step );
	}

	return nullptr;
}

