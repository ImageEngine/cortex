//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/IFFFile.h"

#include "IECore/ByteOrder.h"
#include "IECore/Exception.h"
#include "IECore/TestTypedData.h"

#include <fstream>

using namespace IECore;

IFFFile::IFFFile( const std::string &fileName ) : m_iStream( nullptr ), m_streamFileName( fileName ), m_root( nullptr )
{
}

IFFFile::~IFFFile()
{
	delete m_iStream;
}

bool IFFFile::open()
{
	if( !m_iStream || !m_root )
	{
		delete m_iStream;
		m_iStream = new std::ifstream( m_streamFileName.c_str(), std::ios_base::binary | std::ios_base::in);
		if( !m_iStream->is_open() || !m_iStream->good() )
		{
			return false;
		}

		delete m_root;
		std::streampos begin = m_iStream->tellg();

		char id[4];
		m_iStream->read( id, 4 );
		if( !m_iStream->good() )
		{
			return false;
		}

		IFFFile::Tag testTag( id );
		if( !testTag.isGroup() )
		{
			return false;
		}

		m_iStream->seekg( 0, std::ios_base::end );
		std::streampos end = m_iStream->tellg();
		m_root = new IFFFile::Chunk( "FOR4", end - begin, this, begin, 4 );
		m_iStream->seekg( 0, std::ios_base::beg );
	}
	return m_iStream->good() && m_root;
}

IFFFile::Chunk::Chunk()
	: m_type(), m_dataSize( 0 ), m_file( nullptr ), m_filePosition( 0 ), m_groupName(), m_alignmentQuota( 0 ), m_children()
{
}

IFFFile::Chunk::Chunk( std::string type, unsigned int dataSize, IFFFilePtr file, std::streampos filePosition, int alignmentQuota )
	: m_type( type ), m_dataSize( dataSize ), m_file( file ), m_filePosition( filePosition ), m_groupName(), m_alignmentQuota( alignmentQuota ), m_children()
{
}

IFFFile::Chunk *IFFFile::root()
{
	if ( !open() )
	{
		throw Exception( ( boost::format( "Failed to load \"%s\"." ) % m_streamFileName ).str() );
	}

	return m_root;
}

IFFFile::Tag IFFFile::Chunk::type()
{
	return m_type;
}

unsigned int IFFFile::Chunk::dataSize()
{
	return m_dataSize;
}

bool IFFFile::Chunk::isGroup()
{
	return m_type.isGroup();
}

IFFFile::Tag IFFFile::Chunk::groupName()
{
	return m_groupName;
}

IFFFile::Chunk::ChunkIterator IFFFile::Chunk::childrenBegin()
{
	if ( isGroup() && !m_children.size() )
	{
		ls();
	}

	return m_children.begin();
}

IFFFile::Chunk::ChunkIterator IFFFile::Chunk::childrenEnd()
{
	if ( isGroup() && !m_children.size() )
	{
		ls();
	}

	return m_children.end();
}

void IFFFile::Chunk::ls()
{
	std::streampos currentPosition = m_filePosition;

	while ( currentPosition < m_filePosition + (std::streampos)m_dataSize )
	{
		IFFFile::Chunk child( IFFFile::Tag().name(), 0, m_file, currentPosition, m_alignmentQuota );

		child.readHeader( &currentPosition );

		m_children.push_back( child );

		currentPosition += child.dataSize() + child.skippableBytes();
	}
}

void IFFFile::Chunk::readHeader( std::streampos *pos )
{
	m_file->m_iStream->seekg( *pos, std::ios_base::beg );

	// read type
	char tagBuffer[IFFFile::Tag::TagSize];
	m_file->m_iStream->read( tagBuffer, IFFFile::Tag::TagSize );
	m_type = IFFFile::Tag( tagBuffer );

	// read dataSize
	m_file->m_iStream->read( (char *)&m_dataSize, sizeof(m_dataSize) );
	m_dataSize = asBigEndian( m_dataSize );

	if ( isGroup() )
	{
		// read groupName
		m_file->m_iStream->read( tagBuffer, IFFFile::Tag::TagSize );
		m_groupName = IFFFile::Tag( tagBuffer );

		// modify dataSize
		m_dataSize -= IFFFile::Tag::TagSize;

		// calculate alignment quota
		m_alignmentQuota = alignmentQuota();
	}

	// set the file position of the data
	m_filePosition = m_file->m_iStream->tellg();
	*pos = m_filePosition;
}

void IFFFile::Chunk::read( std::string &data )
{
	m_file->m_iStream->seekg( m_filePosition, std::ios_base::beg );

	std::vector<char> buffer( m_dataSize );
	m_file->m_iStream->read( buffer.data(), m_dataSize );

	data.clear();
	data = buffer.data();
}

int IFFFile::Chunk::alignmentQuota()
{
	if ( !isGroup() )
	{
		return 0;
	}
	else if ( m_type.alignmentByte() == '8' )
	{
		return 8;
	}
	else if ( m_type.alignmentByte() == '4' )
	{
		return 4;
	}
	else
	{
		return 2;
	}
}

int IFFFile::Chunk::skippableBytes()
{
	int modResult = m_dataSize % m_alignmentQuota;

	if ( modResult )
	{
		return m_alignmentQuota - modResult;
	}
	else
	{
		return 0;
	}
}

IFFFile::Tag::Tag() : m_a( '\0' ), m_b( '\0' ), m_c( '\0' ), m_d( '\0' ), m_id( 0 )
{
}

IFFFile::Tag::Tag( const char *buffer ) : m_a( buffer[0] ), m_b( buffer[1] ), m_c( buffer[2] ), m_d( buffer[3] )
{
	int intBuffer[1];

	IFFFile::readData( buffer, intBuffer, 1 );
	m_id = intBuffer[0];
}

IFFFile::Tag::Tag( std::string str ) : m_a( str[0] ), m_b( str[1] ), m_c( str[2] ), m_d( str[3] )
{
	int intBuffer[1];
	char buffer[IFFFile::Tag::TagSize];
	buffer[0] = m_a;
	buffer[1] = m_b;
	buffer[2] = m_c;
	buffer[3] = m_d;

	IFFFile::readData( buffer, intBuffer, 1 );
	m_id = intBuffer[0];
}

std::string IFFFile::Tag::name()
{
	std::string s;
	s = s + m_a + m_b + m_c + m_d + '\0';
	return s;
}

int IFFFile::Tag::id()
{
	return m_id;
}

char IFFFile::Tag::alignmentByte()
{
	return m_d;
}

bool IFFFile::Tag::isGroup()
{
	if ( m_id == IFFFile::Tag::kFORM || m_id == IFFFile::Tag::kFOR4 || m_id == IFFFile::Tag::kFOR8 )
	{
		return true;
	}
	else
	{
		return false;
	}
}
