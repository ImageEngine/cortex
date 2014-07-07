//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2010, Image Engine Design Inc. All rights reserved.
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
#include <math.h>

#include "boost/version.hpp"
#include "boost/format.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/regex.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/convenience.hpp"
#include "boost/algorithm/string.hpp"

#include "IECore/Exception.h"
#include "IECore/FileSequence.h"
#include "IECore/FileSequenceFunctions.h"
#include "IECore/CompoundFrameList.h"
#include "IECore/EmptyFrameList.h"
#include "IECore/FrameRange.h"
#include "IECore/ReversedFrameList.h"

#if BOOST_VERSION < 103400

	// Boost versions prior to 1.34.0 performed unwanted file name checking. Disabling.
	static bool boostFilesystemDefaultNameCheckOverride()
	{
		boost::filesystem::path::default_name_check( boost::filesystem::no_check );
		return true;
	}
	static bool boostFilesystemDefaultNameCheckOverrideResult = boostFilesystemDefaultNameCheckOverride();

#endif

#if BOOST_VERSION < 104400

	// Boost 1.44.0 introduced Filesystem v3, which we use by defining BOOST_FILESYSTEM_VERSION=3 via
	// the build process. Prior versions of boost didn't have this version, so we need this define
	// to help write code suitable for both.

	#define PATH_TO_STRING filename()

#else

	#define PATH_TO_STRING filename().string()

#endif

using namespace IECore;

void IECore::findSequences( const std::vector< std::string > &names, std::vector< FileSequencePtr > &sequences, size_t minSequenceSize )
{
	sequences.clear();

	/// this matches names of the form $prefix$frameNumber$suffix
	/// placing each of those in a group of the resulting match.
	/// both $prefix and $suffix may be the empty string and $frameNumber
	/// may be preceded by a minus sign.
	/// It also matches file extensions with 3 or 4 characters that contain numbers (for example: CR2, MP3 )
	boost::regex matchExpression( std::string( "^([^#]*?)(-?[0-9]+)([^0-9#]*|[^0-9#]*\\.[a-zA-Z]{2,3}[0-9])$" ) );

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
		// todo: could be more efficient by writing a custom comparison function that uses indexes 
		//	 into the const Frames vector rather than duplicating the strings and sorting them directly
		Frames sortedFrames = frames;
		std::sort( sortedFrames.begin(), sortedFrames.end() );
		
		/// in diabolical cases the elements of frames may not all have the same padding
		/// so we'll sort them out into padded and unpadded frame sequences here, by creating
		/// a map of padding->list of frames. unpadded things will be considered to have a padding
		/// of 1.
		typedef std::vector< FrameList::Frame > NumericFrames;
		typedef std::map< unsigned int, NumericFrames > PaddingToFramesMap;
		PaddingToFramesMap paddingToFrames;
		for ( Frames::const_iterator fIt = sortedFrames.begin(); fIt != sortedFrames.end(); ++fIt )
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

			/// remove any sequences with less than the given minimum.
			if ( expandedFrameList.size() >= minSequenceSize )
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

void IECore::findSequences( const std::vector< std::string > &names, std::vector< FileSequencePtr > &sequences )
{
	/// ignore any sequences with less than two files
	findSequences( names, sequences, 2 );
}

void IECore::ls( const std::string &path, std::vector< FileSequencePtr > &sequences, size_t minSequenceSize )
{
	sequences.clear();

	if ( boost::filesystem::is_directory( path ) )
	{
		boost::filesystem::directory_iterator end;
	 	std::vector< std::string > files;
		for ( boost::filesystem::directory_iterator it( path ); it != end; ++it )
		{
			files.push_back( it->path().PATH_TO_STRING );
		}

		findSequences( files, sequences, minSequenceSize );
	}
}

void IECore::ls( const std::string &sequencePath, FileSequencePtr &sequence, size_t minSequenceSize )
{
	sequence = 0;
	boost::smatch matches;
	bool m = boost::regex_match( sequencePath, matches, FileSequence::fileNameValidator() );
	if ( !m )
	{
		return;
	}

	const std::string paddingStr( matches[2].first, matches[2].second );
	const unsigned padding = paddingStr.size();

 	std::vector< std::string > files;

	boost::filesystem::path dir = boost::filesystem::path( sequencePath ).parent_path();

	std::string baseSequencePath = boost::filesystem::path( sequencePath ).PATH_TO_STRING;

	const std::string::size_type first = baseSequencePath.find_first_of( '#' );
	assert( first != std::string::npos );
	const std::string prefix = baseSequencePath.substr( 0, first );

	const std::string::size_type last = baseSequencePath.find_last_of( '#' );
	assert( last != std::string::npos );
	const std::string suffix = baseSequencePath.substr( last + 1, baseSequencePath.size() - last - 1 );

	boost::filesystem::path dirToCheck( dir );
	if ( dirToCheck.string() == "" )
	{
		dirToCheck = ".";
	}

	boost::filesystem::directory_iterator end;

	for ( boost::filesystem::directory_iterator it( dirToCheck ); it != end; ++it )
	{
		const std::string fileName = it->path().PATH_TO_STRING;

		if ( fileName.size() >= std::min( prefix.size(), suffix.size() ) && fileName.substr( 0, prefix.size() ) == prefix && fileName.substr( fileName.size() - suffix.size(), suffix.size() ) == suffix )
		{
			files.push_back( ( dir / boost::filesystem::path( fileName ) ).string() );
		}
	}

	std::vector< FileSequencePtr > sequences;
	findSequences( files, sequences, minSequenceSize );

	for ( std::vector< FileSequencePtr >::iterator it = sequences.begin(); it != sequences.end() ; ++it )
	{
		if ( (*it)->getPadding() == padding )
		{
			sequence = *it;
			return;
		}
		// Also accept frame ranges with number of digits greater than the padding.
		if ( (*it)->getPadding() == 1 )
		{
			FrameList::Frame startFrame, endFrame;
			
			if( FrameRange *fr = runTimeCast<FrameRange>( (*it)->getFrameList().get() ) )
			{
				// Can do a quick validation if the range is a FrameRange, by looking at the start and end only
				startFrame = fr->getStart();
				endFrame = fr->getEnd();
			}
			else
			{
				// Otherwise, have to get the full list
				std::vector<FrameList::Frame> frames;
				(*it)->getFrameList()->asList(frames);
				// I know that there will be at least one frame, since the file sequence was found
				startFrame = frames[ 0 ];
				endFrame = frames[ 0 ];
				for ( unsigned i=0; i < frames.size(); i++ )
				{
					if ( frames[ i ] < startFrame )
					{
						startFrame = frames[ i ];
					}
					else if ( frames[ i ] > endFrame )
					{
						endFrame = frames[ i ];
					}
				}
			}
			
			// if goes through 0 then this is a no.
			if ( !(startFrame < 0 && endFrame > 0) )
			{
				unsigned int startDigits = 1;
				if ( startFrame > 0 )
					startDigits = static_cast<unsigned int>(floor(log10( static_cast<double>(abs(startFrame)) ))) + 1;

				if ( startDigits >= padding )
				{
					sequence = *it;
					sequence->setPadding( padding );
					return;
				}
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

	while ( rangeEnd <= (FrameList::Frame)frames.size() )
	{
		if ( rangeEnd == (FrameList::Frame)frames.size() || frames[ rangeEnd ] - frames[ rangeEnd -1 ] != rangeStep )
		{
			/// we've come to the end of a run
			if ( rangeEnd - 1 == rangeStart )
			{
				frameLists.push_back( new FrameRange( frames[ rangeStart ], frames[ rangeStart ] ) );
			}
			else
			{
				if ( rangeStep > 0)
				{
					frameLists.push_back( new FrameRange( frames[ rangeStart ], frames[ rangeEnd -1 ], rangeStep ) );
				}
				else
				{
					frameLists.push_back( new ReversedFrameList( new FrameRange(frames[ rangeEnd -1 ], frames[ rangeStart ], -rangeStep ) ));
				}
			}

			rangeStart = rangeEnd;
			rangeEnd = rangeStart + 1;

			if ( rangeEnd < (FrameList::Frame)frames.size() )
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
