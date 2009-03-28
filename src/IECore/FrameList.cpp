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

#include <algorithm>

#include "boost/format.hpp"

#include "IECore/Exception.h"
#include "IECore/FrameList.h"

using namespace IECore;

FrameList::FrameList()
{
}

FrameList::~FrameList()
{
}

void FrameList::asClumpedList( std::vector< std::vector<Frame> > &clumpedFrames, unsigned int clumpSize ) const
{
	clumpedFrames.clear();
	
	std::vector<Frame> frames;
	asList( frames );
	
	size_t idx = 0;
	while ( idx < frames.size() )
	{
		size_t thisClumpSize = std::min( frames.size() - idx, (size_t)clumpSize );

		size_t j = 0;				
		std::vector<Frame> clump;
		while ( j < thisClumpSize )
		{
			clump.push_back( frames[ idx + j ] );
			j++;
		}
		clumpedFrames.push_back( clump );

		idx += thisClumpSize;
	}
}

FrameListPtr FrameList::parse( const std::string &frameList )
{
	std::string s;

	/// Strip whitespace
	for ( std::string::const_iterator it = frameList.begin(); it != frameList.end(); ++it )
	{		
		if ( *it == ' ' || *it == '\n' || *it == '\r' || *it == '\n' )
		{
			continue;
		}
		
		s += *it;
	}

	/// Strip enclosing brackets
	if ( s.size() >= 2 && s[0] == '(' && s[ s.size() - 1 ] == ')' )
	{
		s = s.substr( 1, s.size() - 2 );
	}

	ParserList *l = parserList();
	for ( ParserList::const_iterator it = l->begin(); it != l->end(); ++it )
	{
		FrameListPtr f = (*it)( s );
		if ( f )
		{
			return f;
		}
	}
	
	throw Exception( ( boost::format( "\"%s\" does not define a valid frame list." ) % ( frameList ) ).str() );
}

bool FrameList::isEqualTo( ConstFrameListPtr other ) const
{
	return typeId() == other->typeId();
}
		
bool FrameList::operator ==( const FrameList &other ) const
{
	return this->isEqualTo( ConstFrameListPtr( &other ) );
}

FrameList::ParserList *FrameList::parserList()
{
	static ParserList *l = new ParserList();
	return l;
}

void FrameList::registerParser( ParserFn fn )
{
	ParserList *l = parserList();
	assert( std::find( l->begin(), l->end(), fn ) == l->end() );	
	l->push_back( fn );
}

