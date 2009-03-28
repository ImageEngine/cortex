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
#include <list>

#include "IECore/EmptyFrameList.h"
#include "IECore/BinaryFrameList.h"
#include "IECore/Exception.h"

using namespace IECore;

FrameList::Parser< BinaryFrameList > BinaryFrameList::g_parserRegistrar;

BinaryFrameList::BinaryFrameList( FrameListPtr frameList ) : ReorderedFrameList( frameList ? frameList : new EmptyFrameList() )
{
}

BinaryFrameList::~BinaryFrameList()
{
}

void BinaryFrameList::asList( std::vector<Frame> &frames ) const
{
	frames.clear();
	
	std::vector<Frame> l;
	m_frameList->asList( l );
	
	if ( l.size() <= 2 )
	{
		return;
	}
#ifndef NDEBUG
	size_t oldSize = l.size();
#endif	
	
	frames.push_back( *l.begin() );
	l.erase( l.begin() );
	frames.push_back( *l.rbegin() );
	l.pop_back();
	
	assert( l.size() + 2 == oldSize );
	assert( frames.size() == 2 );
	
	std::list< std::vector< Frame > > toVisit;
	toVisit.push_back( l );
	while ( toVisit.size() )
	{
		std::vector< Frame > n = *toVisit.begin();
		toVisit.pop_front();
		
		if ( n.size() > 1 )
		{
			size_t mid = ( n.size() - 1 ) / 2;
			
			std::vector< Frame >::iterator midIt = n.begin();
			size_t i = 0;
			while ( i++ != mid )
			{
				++ midIt;
			}
			
			frames.push_back( *midIt );
			
			std::vector< Frame > head( n.begin(), midIt );
			std::vector< Frame > tail( midIt+1, n.end() );			
			
			toVisit.push_back( head );
			toVisit.push_back( tail );			
			 
		}
		else if ( n.size() )
		{
			frames.push_back( *n.begin() );
		}
	}
	
#ifndef NDEBUG
	std::vector<Frame> origList;
	m_frameList->asList( origList );	
	assert( frames.size() == origList.size() );
#endif	
}


std::string BinaryFrameList::asString() const
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

FrameListPtr BinaryFrameList::copy() const
{
	return new BinaryFrameList( m_frameList );
}

std::string BinaryFrameList::suffix()
{
	return "b";
}

FrameListPtr BinaryFrameList::parse( const std::string &frameList )
{	
	FrameListPtr l = parseForChildList<BinaryFrameList>( frameList );
	if ( l )
	{
		return new BinaryFrameList( l );
	}
	
	return 0;
}
