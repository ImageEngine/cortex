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
#include "IECore/CompoundFrameList.h"

using namespace IECore;

FrameList::Parser< CompoundFrameList > CompoundFrameList::g_parserRegistrar;

CompoundFrameList::CompoundFrameList()
{
}

CompoundFrameList::CompoundFrameList( const std::vector< FrameListPtr > frameLists ) : m_frameLists( frameLists )
{
	for ( std::vector< FrameListPtr >::const_iterator it = m_frameLists.begin(); it != m_frameLists.end(); ++it )
	{
		assert( *it );
	}
}

CompoundFrameList::~CompoundFrameList()
{
}

std::vector< FrameListPtr > &CompoundFrameList::getFrameLists()
{
	return m_frameLists;
}

const std::vector< FrameListPtr > &CompoundFrameList::getFrameLists() const
{
	return m_frameLists;
}


void CompoundFrameList::setFrameLists( const std::vector< FrameListPtr > &frameLists )
{
	m_frameLists = frameLists;
}

void CompoundFrameList::asList( std::vector<Frame> &frames ) const 
{
	frames.clear();	
	
	std::set<Frame> frameSet;
	
	for ( std::vector< FrameListPtr >::const_iterator it = m_frameLists.begin(); it != m_frameLists.end(); ++it )
	{
		if ( !(*it) )
		{
			throw Exception( "CompoundFrameList contains invalid frame list" );
		}
		
		std::vector<Frame> subFrames;
		(*it)->asList( subFrames );
		
		for ( std::vector<Frame>::const_iterator it = subFrames.begin(); it != subFrames.end(); ++it )
		{
			if ( frameSet.find( *it ) == frameSet.end() )
			{
				frameSet.insert( *it );
				frames.push_back( *it );
			}
		}
	}
}

std::string CompoundFrameList::asString() const 
{
	std::string s;
	for ( std::vector< FrameListPtr >::const_iterator it = m_frameLists.begin(); it != m_frameLists.end(); ++it )
	{
		if ( !(*it) )
		{
			throw Exception( "CompoundFrameList contains invalid frame list" );
		}
		if ( it != m_frameLists.begin() )
		{
			s += ",";
		}
		
		s += (*it)->asString();
	}
	return s;
}

bool CompoundFrameList::isEqualTo( ConstFrameListPtr other ) const
{
	if ( !FrameList::isEqualTo( other ) )
	{
		return false;
	}
	
	ConstCompoundFrameListPtr otherF = assertedStaticCast< const CompoundFrameList >( other );
	
	if ( getFrameLists().size() != otherF->getFrameLists().size() )
	{
		return false;
	}
	
	for ( std::vector< FrameListPtr >::size_type i = 0; i < getFrameLists().size(); i++ )
	{
		if ( !getFrameLists()[i]->isEqualTo( otherF->getFrameLists()[i] ) )
		{
			return false;
		}
	}
	
	return true;
}

FrameListPtr CompoundFrameList::copy() const
{
	return new CompoundFrameList( m_frameLists );
}

FrameListPtr CompoundFrameList::parse( const std::string &frameList )
{	
	if ( frameList.find_first_of( ',' ) != std::string::npos )
	{
		std::vector< FrameListPtr > frameLists;
		boost::tokenizer<boost::char_separator<char> > t( frameList, boost::char_separator<char>( "," ) );
		try 
		{
			for (
				boost::tokenizer<boost::char_separator<char> >::iterator it = t.begin();
				it != t.end();
				++it
			)
			{
				FrameListPtr f = FrameList::parse( *it );
				if ( !f )
				{
					return 0;
				}
				
				frameLists.push_back( f );
			}
			
			return new CompoundFrameList( frameLists );
		} 
		catch ( Exception & )
		{
			return 0;
		}
	}

	return 0;
}
