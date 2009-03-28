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

#include "IECore/EmptyFrameList.h"
#include "IECore/ReversedFrameList.h"
#include "IECore/Exception.h"

using namespace IECore;

FrameList::Parser< ReversedFrameList > ReversedFrameList::g_parserRegistrar;

ReversedFrameList::ReversedFrameList( FrameListPtr frameList ) : ReorderedFrameList( frameList ? frameList : new EmptyFrameList() )
{
}

ReversedFrameList::~ReversedFrameList()
{
}

void ReversedFrameList::asList( std::vector<Frame> &frames ) const
{
	frames.clear();
	
	m_frameList->asList( frames );
	
	std::reverse( frames.begin(), frames.end() );
	
#ifndef NDEBUG
	std::vector<Frame> origList;
	m_frameList->asList( origList );	
	assert( frames.size() == origList.size() );
#endif	
}

std::string ReversedFrameList::asString() const
{
	std::string s = m_frameList->asString();
	
	if ( s.find_first_of( ',' ) != std::string::npos )
	{
		return "(" + s + ")" + suffix();
	}
	else
	{
		return s + suffix();
	}
}

bool ReversedFrameList::isEqualTo( ConstFrameListPtr other ) const
{
	return ReorderedFrameList::isEqualTo( other );
}

FrameListPtr ReversedFrameList::copy() const
{
	return new ReversedFrameList( m_frameList );
}

std::string ReversedFrameList::suffix()
{
	return "r";
}

FrameListPtr ReversedFrameList::parse( const std::string &frameList )
{	
	FrameListPtr l = parseForChildList<ReversedFrameList>( frameList );
	if ( l )
	{
		return new ReversedFrameList( l );
	}
	
	return 0;
}
