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
#include <iostream>

#include "boost/format.hpp"
#include "boost/regex.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"

#include "IECore/Exception.h"
#include "IECore/FileSequence.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( FileSequence );

FileSequence::FileSequence( const std::string &fileName, FrameListPtr frameList )
{
	setFileName( fileName );
	setFrameList( frameList );
}

FileSequence::~FileSequence()
{
}

const std::string &FileSequence::getFileName() const
{
	return m_fileName;
}

void FileSequence::setFileName( const std::string &fileName )
{
	if ( ! boost::regex_match( fileName, fileNameValidator() ) )
	{
		throw InvalidArgumentException( "FileSequence fileName must contain a single sequence of at least one # to denote frame number." );
	}
	m_fileName = fileName;
}

FrameListPtr FileSequence::getFrameList()
{
	return m_frameList;
}

void FileSequence::setFrameList( FrameListPtr frameList )
{
	assert( frameList );
	m_frameList = frameList;
}

std::string FileSequence::asString() const
{
	return m_fileName + " " + m_frameList->asString();
}

unsigned FileSequence::getPadding() const
{
	unsigned count = 0;

	for ( std::string::const_iterator it = m_fileName.begin(); it != m_fileName.end(); ++it )
	{
		if ( *it == '#' )
		{
			++count;
		}
	}
	return count;
}

void FileSequence::setPadding( unsigned padding )
{
	unsigned oldPadding = getPadding();
	std::string oldPaddingStr = "";
	for ( unsigned i = 0; i < oldPadding; i++)
	{
		oldPaddingStr += "#";
	}

	std::string newPaddingStr = "";
	for ( unsigned i = 0; i < padding; i++)
	{
		newPaddingStr += "#";
	}

	std::string newFileName = m_fileName;
	boost::replace_all( newFileName, oldPaddingStr, newPaddingStr );
	setFileName( newFileName );
}

std::string FileSequence::getPrefix() const
{
	std::string::size_type p = m_fileName.find_first_of( '#' );
	assert( p != std::string::npos );

	return m_fileName.substr( 0, p );
}

void FileSequence::setPrefix( const std::string &prefix )
{
	std::string::size_type p = m_fileName.find_first_of( '#' );
	assert( p != std::string::npos );
	setFileName( prefix + m_fileName.substr( p, m_fileName.size() - p ) );
}

std::string FileSequence::getSuffix() const
{
	std::string::size_type p = m_fileName.find_last_of( '#' );
	assert( p != std::string::npos );
	return m_fileName.substr( p+1, m_fileName.size() - p - 1 );
}

void FileSequence::setSuffix( const std::string &suffix )
{
	std::string::size_type p = m_fileName.find_last_of( '#' );
	assert( p != std::string::npos );
	setFileName( m_fileName.substr( 0, p + 1 ) + suffix );
}

std::string FileSequence::fileNameForFrame( FrameList::Frame frameNumber ) const
{
	return ( fileNameTemplate( frameNumber < 0 ) % frameNumber ).str();
}

FrameList::Frame FileSequence::frameForFileName( const std::string &fileName ) const
{
	std::string prefix = getPrefix();
	std::string suffix = getSuffix();

	if ( fileName.substr( 0, prefix.size() ) != prefix || fileName.substr( fileName.size() - suffix.size(), suffix.size() ) != suffix )
	{
		throw InvalidArgumentException( ( boost::format( "Filename \"%s\" is not a part of sequence \"%s\"." ) % fileName % asString() ).str() );
	}

	std::string frameStr = fileName.substr( prefix.size(), fileName.size() - suffix.size() - prefix.size() );

	if ( frameStr.size() == 0 )
	{
		throw InvalidArgumentException( ( boost::format( "Filename \"%s\" is not a part of sequence \"%s\"." ) % fileName % asString() ).str() );
	}

	return boost::lexical_cast< FrameList::Frame >( frameStr );
}

void FileSequence::fileNames( std::vector< std::string > &f ) const
{
	f.clear();

	boost::format posFmt = fileNameTemplate( false );
	boost::format negFmt = fileNameTemplate( true );

	std::vector< FrameList::Frame > frames;

	m_frameList->asList( frames );

	for ( std::vector< FrameList::Frame >::const_iterator it = frames.begin(); it != frames.end(); ++it )
	{
		boost::format &fmt = *it < 0 ? negFmt : posFmt ;
		f.push_back( ( fmt % *it ).str() );
	}
}

void FileSequence::clumpedFileNames( unsigned clumpSize, std::vector< std::vector < std::string > > &f ) const
{
	f.clear();

	boost::format posFmt = fileNameTemplate( false );
	boost::format negFmt = fileNameTemplate( true );

	std::vector< std::vector< FrameList::Frame > > clumpedFrames;

	m_frameList->asClumpedList( clumpedFrames, clumpSize );

	for ( std::vector< std::vector< FrameList::Frame > >::const_iterator it = clumpedFrames.begin(); it != clumpedFrames.end(); ++it )
	{
		const std::vector< FrameList::Frame > &clump = *it;

		f.push_back( std::vector< std::string >() );

		for ( std::vector< FrameList::Frame > ::const_iterator cit = clump.begin(); cit != clump.end(); ++cit )
		{
			boost::format &fmt = *cit < 0 ? negFmt : posFmt ;
			f.back().push_back( ( fmt % *cit ).str() );
		}
	}
}

FileSequencePtr FileSequence::copy() const
{
	return new FileSequence( m_fileName, m_frameList->copy() );
}

void FileSequence::mapTo( ConstFileSequencePtr other, std::vector< std::pair< std::string, std::string > > &result ) const
{
	assert( other );
	result.clear();

	std::vector< std::string > names, otherNames;

	fileNames( names );
	other->fileNames( otherNames );

	if ( names.size() != otherNames.size() )
	{
		throw InvalidArgumentException( "FileSequence.mapTo() : FileSequence objects contain different numbers of frames." );
	}

	size_t numNames = names.size();

	result.reserve( numNames );
	for ( size_t i = 0; i < numNames; i++)
	{
		result.push_back( std::pair< std::string, std::string >( names[i], otherNames[i] ) );
	}
}

void FileSequence::mapTo( ConstFileSequencePtr other, std::map< std::string, std::string > &result ) const
{
	assert( other );
	result.clear();

	std::vector< std::string > names, otherNames;

	fileNames( names );
	other->fileNames( otherNames );

	if ( names.size() != otherNames.size() )
	{
		throw InvalidArgumentException( "FileSequence.mapTo() : FileSequence objects contain different numbers of frames." );
	}

	size_t numNames = names.size();

	for ( size_t i = 0; i < numNames; i++)
	{
		result[ names[i] ] = otherNames[i] ;
	}
}

boost::regex FileSequence::fileNameValidator()
{
	return boost::regex( "^([^#]*)(#+)([^#]*)$" );
}

bool FileSequence::operator ==( const FileSequence &other ) const
{
	return m_fileName == other.m_fileName && m_frameList->isEqualTo( other.m_frameList );
}

boost::format FileSequence::fileNameTemplate( bool negativeFrame ) const
{
	unsigned padding = getPadding();
	std::string paddingStr = "";
	for ( unsigned i = 0; i < padding; i++)
	{
		paddingStr += "#";
	}

	std::string f = m_fileName;
	boost::replace_all( f, "%", "%%" );
	boost::replace_all( f, paddingStr, ( boost::format( "%%0%dd" ) % ( negativeFrame ? padding + 1 : padding ) ).str() );

	return boost::format( f );
}
