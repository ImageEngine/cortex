//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "boost/filesystem/operations.hpp"

#include "IECore/MessageHandler.h"
#include "IECore/FileIndexedIO.h"

#ifdef _WIN32
#include <Windows.h>
#include <tchar.h>
int truncate(const char* filename, long offset)
{
	TCHAR tfilename[255];
	wsprintf(tfilename,_T("%s"),filename);
	HANDLE filet = CreateFile(tfilename,FILE_WRITE_DATA,FILE_SHARE_READ,NULL,OPEN_ALWAYS,0,NULL);
	if (filet == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	SetFilePointer(filet,offset,NULL,FILE_BEGIN);
	if (SetEndOfFile(filet))
	{
		CloseHandle(filet);
		return 0;
	}
	else
	{
		CloseHandle(filet);
		return -1;
	}

}
#endif

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif


/// \todo Describe what each item actually means!

/// FileFormat ::= Data Index Version MagicNumber
/// Data ::= DataEntry*
/// Index ::= StringCache Nodes FreePages IndexOffset

/// StringCache ::= NumStrings String*
/// NumStrings ::= int64
/// String ::= StringLength char*
/// StringLength ::= int64

/// Nodes ::= NumNodes Node*
/// NumNodes ::= int64
/// Node ::= EntryType EntryStringCacheID DataType ArrayLength NodeID ParentNodeID DataOffset DataSize
/// EntryType ::= char
/// EntryStringCacheID ::= int64
/// DataType ::= char
/// ArrayLength ::= int64
/// NodeID ::= int64
/// ParentNodeID ::= ParentNodeID
/// DataOffset ::= int64
/// DataSize ::= int64

/// FreePages ::= NumFreePages FreePage*
/// NumFreePages ::= int64
/// FreePage ::= FreePageOffset FreePageSize
/// FreePageOffset ::= int64
/// FreePageSize ::= int64
/// IndexOffset ::= int64

/// Version ::= int64
/// MagicNumber ::= int64

using namespace IECore;

namespace fs = boost::filesystem;

IE_CORE_DEFINERUNTIMETYPEDDESCRIPTION( FileIndexedIO )


///////////////////////////////////////////////
//
// FileIndexedIO::StreamFile (begin)
//
///////////////////////////////////////////////

class FileIndexedIO::StreamFile : public StreamIndexedIO::StreamFile
{
	public:

		std::string m_filename;

		size_t m_endPosition;

		StreamFile( const std::string &filename, IndexedIO::OpenMode mode );

		virtual ~StreamFile();

		static bool canRead( const std::string &path );

		void flush( size_t endPosition );

};

FileIndexedIO::StreamFile::StreamFile( const std::string &filename, IndexedIO::OpenMode mode ) : StreamIndexedIO::StreamFile(mode), m_filename( filename ), m_endPosition(0)
{
	if (mode & IndexedIO::Write)
	{
		std::fstream *f = new std::fstream(filename.c_str(), std::ios::trunc | std::ios::binary | std::ios::in | std::ios::out);

		if (! f->is_open() )
		{
			throw IOException( "FileIndexedIO: Cannot open '" + filename + "' for writing" );
		}
		setStream( f, true );
	}
	else if (mode & IndexedIO::Append)
	{
		if (!fs::exists( filename.c_str() ) )
		{
			/// Create new file
			std::fstream *f = new std::fstream(filename.c_str(), std::ios::trunc | std::ios::binary | std::ios::in | std::ios::out );

			if (! f->is_open() )
			{
				throw IOException( "FileIndexedIO: Cannot open '" + filename + "' for append" );
			}
			setStream( f, true );
		}
		else
		{
			/// Read existing file
			std::fstream *f = new std::fstream(filename.c_str(), std::ios::binary | std::ios::in | std::ios::out );

			if (! f->is_open() )
			{
				throw IOException( "FileIndexedIO: Cannot open '" + filename + "' for read " );
			}

			try
			{
				setStream( f, false );
			}
			catch ( Exception &e )
			{
				e.prepend( "Opening file \"" + filename + "\" : " );
				throw;
			}
			catch (...)
			{
				throw IOException( "FileIndexedIO: Caught error reading file '" + filename + "'" );
			}

		}
	}
	else
	{
		assert( mode & IndexedIO::Read );
		std::fstream *f = new std::fstream(filename.c_str(), std::ios::binary | std::ios::in );

		if (! f->is_open() )
		{
			throw IOException( "FileIndexedIO: Cannot open file '" + filename + "' for read" );
		}

		try
		{
			setStream( f, false );
		}
		catch ( Exception &e )
		{
			e.prepend( "Opening file \"" + filename + "\" : " );
			throw;
		}
		catch (...)
		{
			throw IOException( "FileIndexedIO: Caught error reading file '" + filename + "'" );
		}

	}
}

void FileIndexedIO::StreamFile::flush( size_t endPosition )
{
	m_endPosition = endPosition;
}

FileIndexedIO::StreamFile::~StreamFile()
{
	if ( m_openmode == IndexedIO::Write || m_openmode == IndexedIO::Append )
	{
		std::fstream *f = static_cast< std::fstream * >( m_stream );

		f->seekg( 0, std::ios::end );
		Imf::Int64 fileEnd = f->tellg();

		// .. and the length of that file extends beyond the end of the index
		if ( fileEnd > m_endPosition )
		{
			/// Close the file before truncation
			delete m_stream;
			m_stream = 0;

			/// Truncate the file at the end of the index
			int err = truncate( m_filename.c_str(), m_endPosition );
			if ( err != 0 )
			{
				msg( Msg::Error, "FileIndexedIO::StreamFile", boost::format ( "Error truncating file '%s' to %d bytes: %s" ) % m_filename % m_endPosition % strerror( err ) );
			}
		}
	}
}

bool FileIndexedIO::StreamFile::canRead( const std::string &path )
{
	std::fstream d( path.c_str(), std::ios::binary | std::ios::in);

	if ( !d.is_open() )
	{
		return false;
	}

	return StreamIndexedIO::StreamFile::canRead( d );
}

///////////////////////////////////////////////
//
// FileIndexedIO::StreamFile (end)
//
///////////////////////////////////////////////

///////////////////////////////////////////////
//
// FileIndexedIO (begin)
//
///////////////////////////////////////////////

static IndexedIO::Description<FileIndexedIO> registrar(".fio");

IndexedIOPtr FileIndexedIO::create(const std::string &path, const IndexedIO::EntryIDList &root, IndexedIO::OpenMode mode)
{
	return new FileIndexedIO(path, root, mode);
}

bool FileIndexedIO::canRead( const std::string &path )
{
	return StreamFile::canRead( path );
}

FileIndexedIO::FileIndexedIO()
{
}

FileIndexedIO::FileIndexedIO(const std::string &path, const IndexedIO::EntryIDList &root, IndexedIO::OpenMode mode)
{
	const fs::path p = fs::path(path);
	const std::string filename = p.string();

	if (! fs::exists(filename) && (mode & IndexedIO::Read))
	{
		throw FileNotFoundIOException(filename);
	}
	open( new StreamFile( filename, mode ), root );
}

FileIndexedIO::FileIndexedIO( StreamIndexedIO::Node &rootNode ) : StreamIndexedIO( rootNode )
{
}

FileIndexedIO::~FileIndexedIO()
{
}

IndexedIO * FileIndexedIO::duplicate(Node &rootNode) const
{
	// duplicate the IO interface changing the current node
	return new FileIndexedIO( rootNode );
}

const std::string &FileIndexedIO::fileName() const
{
	StreamFile &stream = static_cast<StreamFile&>( streamFile() );
	return stream.m_filename;
}
