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

#include <iosfwd>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/positioning.hpp>

using namespace IECore;

IE_CORE_DEFINERUNTIMETYPEDDESCRIPTION( MemoryIndexedIO )

//////////////////////////////////////////////////////
//
// FileIndexedIO::StreamFile subclass implementations
//
//////////////////////////////////////////////////////

class MemoryIndexedIO::CharVectorDataDevice
{
	public:

		typedef char char_type;
		typedef boost::iostreams::seekable_device_tag  category;

		CharVectorDataDevice( CharVectorData *container ) 
			: m_container(container), m_pos(0)
		{}

		std::streamsize read(char* s, std::streamsize n)
		{
			std::streamsize amt = static_cast<std::streamsize>(m_container->readable().size() - m_pos);
			std::streamsize result = std::min(n, amt);
			if (result != 0)
			{
				std::copy( m_container->readable().begin() + m_pos, m_container->readable().begin() + m_pos + result, s );
				m_pos += result;
				return result;
			}
			else
			{
				return -1; // EOF
			}
		}

		std::streamsize write(const char* s, std::streamsize n)
		{
			using namespace std;
			streamsize result = 0;
			if (m_pos != m_container->writable().size())
			{
				streamsize amt = static_cast<streamsize>(m_container->writable().size() - m_pos);
				result = (min)(n, amt);
				std::copy(s, s + result, m_container->writable().begin() + m_pos);
				m_pos += result;
			}
			if (result < n)
			{
				m_container->writable().insert(m_container->writable().end(), s, s + n);
				m_pos = m_container->writable().size();
			}
			return n;
		}

		boost::iostreams::stream_offset seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way )
		{
			using namespace std;
			// Determine new value of m_pos
			boost::iostreams::stream_offset next;
			if (way == std::ios_base::beg)
			{
				next = off;
			}
			else if (way == std::ios_base::cur)
			{
				next = m_pos + off;
			}
			else if (way == std::ios_base::end)
			{
				next = m_container->readable().size() + off;
			}
			else
			{
				throw ios_base::failure("bad seek direction");
			}

			// Check for errors
			if (next < 0 || (size_t)next > m_container->readable().size())
			{
				throw ios_base::failure("bad seek offset");
			}

			m_pos = next;
			return m_pos;
		}

	private:

		CharVectorData* m_container;
		size_t   m_pos;
};

class MemoryIndexedIO::StreamFile : public StreamIndexedIO::StreamFile
{
	public:
		StreamFile( IndexedIO::OpenMode mode ) : StreamIndexedIO::StreamFile(mode)
		{
		}

		virtual CharVectorDataPtr buffer() = 0;

		virtual ~StreamFile()
		{
		}

};

class MemoryIndexedIO::IStreamFile : public MemoryIndexedIO::StreamFile
{
	public:
		IStreamFile( ConstCharVectorDataPtr buffer, IndexedIO::OpenMode mode ) : MemoryIndexedIO::StreamFile(mode), m_buffer( buffer )
		{
			setStream( new boost::iostreams::stream<CharVectorDataDevice>( const_cast<CharVectorData*>( buffer.get() ) ), buffer->readable().empty() );
		}

		virtual CharVectorDataPtr buffer()
		{
			return m_buffer->copy();
		}

		virtual ~IStreamFile()
		{
		}

	private:
		ConstCharVectorDataPtr m_buffer;

};

class MemoryIndexedIO::OStreamFile : public MemoryIndexedIO::StreamFile
{
	public:
		OStreamFile( CharVectorDataPtr buffer, IndexedIO::OpenMode mode ) : MemoryIndexedIO::StreamFile(mode), m_buffer( buffer )
		{
			setStream( new boost::iostreams::stream<CharVectorDataDevice>( buffer.get() ), buffer->readable().empty() );
		}

		virtual CharVectorDataPtr buffer()
		{
			return m_buffer;
		}

		virtual ~OStreamFile()
		{
		}

	private:
		CharVectorDataPtr m_buffer;

};

//////////////////////////////////////////////////////
//
// FileIndexedIO::StreamFile subclasses end
//
//////////////////////////////////////////////////////


MemoryIndexedIO::MemoryIndexedIO( ConstCharVectorDataPtr buf, const IndexedIO::EntryIDList &root, IndexedIO::OpenMode mode)
{
	if (mode & IndexedIO::Write)
	{
		open( new OStreamFile( new CharVectorData, mode ), root );
	}
	else if (mode & IndexedIO::Append)
	{
		if ( !buf )
		{
			/// Create new file
			open( new OStreamFile( new CharVectorData, mode ), root );
		}
		else
		{
			/// Read existing file
			open( new OStreamFile( buf->copy(), mode ), root );
		}
	}
	else
	{
		assert( buf );
		assert( mode & IndexedIO::Read );
		open( new IStreamFile( buf, mode ), root );
	}
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
