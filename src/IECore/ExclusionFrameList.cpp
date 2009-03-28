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
#include <set>

#include "boost/tokenizer.hpp"

#include "IECore/Exception.h"
#include "IECore/ExclusionFrameList.h"

using namespace IECore;

FrameList::Parser< ExclusionFrameList > ExclusionFrameList::g_parserRegistrar;

ExclusionFrameList::ExclusionFrameList( FrameListPtr frameList, FrameListPtr exclusionFrameList ) : m_frameList( frameList ), m_exclusionFrameList( exclusionFrameList )
{
}

ExclusionFrameList::~ExclusionFrameList()
{
}

void ExclusionFrameList::setFrameList( FrameListPtr frameList )
{
	m_frameList = frameList;
}

FrameListPtr ExclusionFrameList::getFrameList()
{
	return m_frameList;
}

void ExclusionFrameList::setExclusionFrameList( FrameListPtr exclusionFrameList )
{
	m_exclusionFrameList = exclusionFrameList;
}

FrameListPtr ExclusionFrameList::getExclusionFrameList()
{
	return m_exclusionFrameList;
}

void ExclusionFrameList::asList( std::vector<Frame> &frames ) const 
{	
	frames.clear();
	
	std::vector<Frame> l;
	m_frameList->asList( l );
	
	std::vector<Frame> e;
	m_exclusionFrameList->asList( e );	
	
	std::set<Frame> lSet( l.begin(), l.end() );
	std::set<Frame> eSet( e.begin(), e.end() );
	
	set_difference( lSet.begin(), lSet.end(), eSet.begin(), eSet.end(), std::back_inserter( frames ) );
	
	assert( frames.size() <= l.size() );
}

std::string ExclusionFrameList::asString() const
{
	std::string s1 = m_frameList->asString();
	std::string s2 = m_exclusionFrameList->asString();
	
	if ( s1.find_first_of( ',' ) != std::string::npos )
	{
		s1 = "(" + s1 + ")";
	}
	
	if ( s2.find_first_of( ',' ) != std::string::npos )
	{
		s2 = "(" + s2 + ")";
	}
	
	return s1 + "!" + s2;
}

bool ExclusionFrameList::isEqualTo( ConstFrameListPtr other ) const
{
	if ( !FrameList::isEqualTo( other ) )
	{
		return false;
	}
	
	ConstExclusionFrameListPtr otherF = assertedStaticCast< const ExclusionFrameList >( other );
	
	return m_frameList->isEqualTo( otherF->m_frameList ) && m_exclusionFrameList->isEqualTo( otherF->m_exclusionFrameList ) ;
}

FrameListPtr ExclusionFrameList::copy() const
{
	return new ExclusionFrameList( m_frameList, m_exclusionFrameList );
}

FrameListPtr ExclusionFrameList::parse( const std::string &frameList )
{
	boost::tokenizer<boost::char_separator<char> > t( frameList, boost::char_separator<char>( "!" ) );
	std::vector<std::string> tokens;
	std::copy( t.begin(), t.end(), std::back_inserter( tokens ) );
	
	if ( tokens.size() == 2 )
	{
		try
		{		
			FrameListPtr f1 = FrameList::parse( tokens[0] );
			FrameListPtr f2 = FrameList::parse( tokens[1] );			
			
			if ( f1 && f2 )
			{
				return new ExclusionFrameList( f1, f2 );
			}
		}
		catch ( Exception & )
		{
			return 0;
		}
	}
	return 0;
}
