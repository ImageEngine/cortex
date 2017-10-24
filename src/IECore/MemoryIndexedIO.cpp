//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/MemoryIndexedIO.h"
#include "IECore/FileIndexedIO.h"
#include "IECore/VectorTypedData.h"

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPEDDESCRIPTION( MemoryIndexedIO )

///////////////////////////////////////////////
//
// FileIndexedIO::StreamFile (begin)
//
///////////////////////////////////////////////

class MemoryIndexedIO::StreamFile : public StreamIndexedIO::StreamFile
{
	public:
		StreamFile( const char *buf, size_t size, IndexedIO::OpenMode mode );

		CharVectorDataPtr buffer();

		~StreamFile() override;

		void flush( size_t endPosition ) override;

	private:

		size_t m_endPosition;
};

MemoryIndexedIO::StreamFile::StreamFile( const char *buf, size_t size, IndexedIO::OpenMode mode ) : StreamIndexedIO::StreamFile(mode), m_endPosition(0)
{
	if (mode & IndexedIO::Write)
	{
		std::stringstream *f = new std::stringstream( std::ios::trunc | std::ios::binary | std::ios::in | std::ios::out );
		setStream( f, true );
	}
	else if (mode & IndexedIO::Append)
	{
		if ( !buf || !size )
		{
			/// Create new file
			std::stringstream *f = new std::stringstream(  std::ios::trunc | std::ios::binary | std::ios::in | std::ios::out );
			setStream( f, true );
		}
		else
		{
			/// Read existing file
			assert( buf );

			/// Read existing file
			std::stringstream *f = new std::stringstream( std::string(buf, size), std::ios::binary | std::ios::in | std::ios::out );
			setStream( f, false );
		}
	}
	else
	{
		assert( buf );
		assert( mode & IndexedIO::Read );
		std::stringstream *f = new std::stringstream( std::string(buf, size), std::ios::binary | std::ios::in | std::ios::out );
		setStream( f, false );
	}
}

void MemoryIndexedIO::StreamFile::flush( size_t endPosition )
{
	m_endPosition = endPosition;
}

CharVectorDataPtr MemoryIndexedIO::StreamFile::buffer()
{
	std::stringstream *s = static_cast< std::stringstream *>( m_stream );
	assert( s );

	CharVectorData::ValueType d;
	const std::string &str = s->str();

	d.assign( str.begin(), str.end() );
	d.resize( m_endPosition );
	assert( d.size() ==  m_endPosition );
	return new CharVectorData( d );
}

MemoryIndexedIO::StreamFile::~StreamFile()
{
}

///////////////////////////////////////////////
//
// MemoryIndexedIO::StreamFile (end)
//
///////////////////////////////////////////////


MemoryIndexedIO::MemoryIndexedIO( ConstCharVectorDataPtr buf, const IndexedIO::EntryIDList &root, IndexedIO::OpenMode mode)
{
	const char *bufPtr = nullptr;
	size_t size = 0;
	if ( buf )
	{
		bufPtr = &(buf->readable()[0]);
		size = buf->readable().size();
	}
	open( new StreamFile( bufPtr, size, mode ), root );
}

MemoryIndexedIO::MemoryIndexedIO( StreamIndexedIO::Node &rootNode ) : StreamIndexedIO( rootNode )
{
}

MemoryIndexedIO::~MemoryIndexedIO()
{
}

CharVectorDataPtr MemoryIndexedIO::buffer()
{
	flush();
	StreamFile &stream = static_cast<StreamFile&>( streamFile() );
	return stream.buffer();
}

IndexedIO * MemoryIndexedIO::duplicate(Node &rootNode) const
{
	// duplicate the IO interface changing the current node
	MemoryIndexedIO *other = new MemoryIndexedIO( rootNode );
	return other;
}
