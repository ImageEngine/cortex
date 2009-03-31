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
#include <cassert>

#include "boost/format.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"

#include "IECore/Exception.h"
#include "IECore/FileSequence.h"
#include "IECore/FileSequenceFunctions.h"
#include "IECore/CompoundFrameList.h"
#include "IECore/EmptyFrameList.h"
#include "IECore/FrameRange.h"

using namespace IECore;

void IECore::findSequences( const std::vector< std::string > &names, std::vector< FileSequencePtr > &sequences )
{
	sequences.clear();
	
	/// this matches names of the form $prefix$frameNumber$suffix
	/// placing each of those in a group of the resulting match.
	/// both $prefix and $suffix may be the empty string and $frameNumber
	/// may be preceded by a minus sign.
	boost::regex matchExpression( std::string( "^([^#]*?)(-?[0-9]+)([^0-9#]*)$" ) );
	
	/// build a mapping from ($prefix, $suffix) to a list of $frameNumbers
	typedef std::vector< std::string > Frames;
	typedef std::map< std::pair< std::string, std::string >, Frames > SequenceMap;
	
	SequenceMap sequenceMap;
	
	for ( std::vector< std::string >::const_iterator it = names.begin(); it != names.end(); ++it )
	{
		boost::smatch matches;
		if ( boost::regex_match( *it, matches, matchExpression ) )
		{
			sequenceMap[ 
				SequenceMap::key_type( 
					std::string( matches[1].first, matches[1].second ),
					std::string( matches[3].first, matches[3].second )
				)
			].push_back( std::string( matches[2].first, matches[2].second ) );				
		}
	}
	
	for ( SequenceMap::const_iterator it = sequenceMap.begin(); it != sequenceMap.end(); ++it )
	{
		const SequenceMap::key_type &fixes = it->first;
		const Frames &frames = it->second;
		
		/// in diabolical cases the elements of frames may not all have the same padding
		/// so we'll sort them out into padded and unpadded frame sequences here, by creating
		/// a map of padding->list of frames. unpadded things will be considered to have a padding
		/// of 1.
		typedef std::vector< FrameList::Frame > NumericFrames;
		typedef std::map< unsigned int, NumericFrames > PaddingToFramesMap;
		PaddingToFramesMap paddingToFrames;
		for ( Frames::const_iterator fIt = frames.begin(); fIt != frames.end(); ++fIt )
		{
			std::string frame = *fIt;
			int sign = 1;
						
			assert( frame.size() );
			if ( *frame.begin() == '-' )
			{
				frame = frame.substr( 1, frame.size() - 1 );
				sign = -1;
			}
			if ( *frame.begin() == '0' || paddingToFrames.find( frame.size() ) != paddingToFrames.end() )
			{
				paddingToFrames[ frame.size() ].push_back( sign * boost::lexical_cast<FrameList::Frame>( frame ) );
			}
			else
			{
				paddingToFrames[ 1 ].push_back( sign * boost::lexical_cast<FrameList::Frame>( frame ) );
			}
		}
		
		for ( PaddingToFramesMap::iterator pIt = paddingToFrames.begin(); pIt != paddingToFrames.end(); ++pIt )
		{
			const PaddingToFramesMap::key_type &padding = pIt->first;
			NumericFrames &numericFrames = pIt->second;
			std::sort( numericFrames.begin(), numericFrames.end() );
			
			FrameListPtr frameList = frameListFromList( numericFrames );
			
			std::vector< FrameList::Frame > expandedFrameList;
			frameList->asList( expandedFrameList );
			
			/// remove any sequences with less than two files
			if ( expandedFrameList.size() >= 2 )
			{
				std::string frameTemplate;
				for ( PaddingToFramesMap::key_type i = 0; i < padding; i++ )
				{
					frameTemplate += "#";
				} 
							
				sequences.push_back( 
					new FileSequence(
						fixes.first + frameTemplate + fixes.second,
						frameList
					)
				);						
			}			
		}
	}
}

FrameListPtr IECore::frameListFromList( const std::vector< FrameList::Frame > &frames )
{
	if ( frames.size() == 0 )
	{
		return new EmptyFrameList();
	}
	else if ( frames.size() == 1 ) 
	{
		return new FrameRange( frames[0], frames[0] );
	}
	
	std::vector< FrameListPtr > frameLists;
	
	FrameList::Frame rangeStart = 0;
	FrameList::Frame rangeEnd = 1;
	FrameList::Frame rangeStep = frames[ rangeEnd ] - frames[ rangeStart ];	
	assert( rangeStep > 0 );
	
	while ( rangeEnd <= frames.size() )
	{
		if ( rangeEnd == frames.size() || frames[ rangeEnd ] - frames[ rangeEnd -1 ] != rangeStep )
		{
			/// we've come to the end of a run
			if ( rangeEnd - 1 == rangeStart )
			{
				frameLists.push_back( new FrameRange( frames[ rangeStart ], frames[ rangeStart ] ) );
			}
			else
			{
				frameLists.push_back( new FrameRange( frames[ rangeStart ], frames[ rangeEnd -1 ], rangeStep ) );
			}
			
			rangeStart = rangeEnd;
			rangeEnd = rangeStart + 1;
			
			if ( rangeEnd < frames.size() )
			{
				rangeStep = frames[ rangeEnd ] - frames[ rangeStart ];
			}
		}
		else
		{
			rangeEnd ++;
		}
	}
	
	if ( frameLists.size() == 1 )
	{
		return frameLists[0];
	}
	else
	{
		return new CompoundFrameList( frameLists );
	}
	
}
