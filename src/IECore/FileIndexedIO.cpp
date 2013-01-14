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

using namespace IECore;

namespace fs = boost::filesystem;

///////////////////////////////////////////////
//
// FileIndexedIO::StreamFile (begin)
//
///////////////////////////////////////////////

class FileIndexedIO::StreamFile : public StreamIndexedIO::StreamFile
{
	public:

		std::string m_filename;

		StreamFile( const std::string &filename, IndexedIO::OpenMode mode, StreamIndexedIO::Index *index = 0 );

		virtual ~StreamFile();

		static bool canRead( const std::string &path );
};

FileIndexedIO::StreamFile::StreamFile( const std::string &filename, IndexedIO::OpenMode mode, StreamIndexedIO::Index *index ) : StreamIndexedIO::StreamFile(mode,index), m_filename( filename )
{
	if (mode & IndexedIO::Write)
	{
		std::fstream *f = new std::fstream(filename.c_str(), std::ios::trunc | std::ios::binary | std::ios::in | std::ios::out);

		if (! f->is_open() )
		{
			throw IOException( "FileIndexedIO: Cannot open '" + filename + "' for writing" );
		}
		setDevice( f );
		
		newIndex();
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
			setDevice( f );
			newIndex();
		}
		else
		{
			/// Read existing file
			std::fstream *f = new std::fstream(filename.c_str(), std::ios::binary | std::ios::in | std::ios::out );

			if (! f->is_open() )
			{
				throw IOException( "FileIndexedIO: Cannot open '" + filename + "' for read " );
			}
			setDevice( f );

			if ( !index )
			{
				/// Read index
				try
				{
					readIndex();
				}
				catch ( Exception &e )
				{
					e.prepend( "Opening file \"" + filename + "\" : " );
					throw;
				}
				catch (...)
				{
					throw IOException( "FileIndexedIO: Caught error reading index in file '" + filename + "'" );
				}
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

		setDevice( f );

		if ( !index )
		{
			/// Read index
			try
			{
				readIndex();
			}
			catch ( Exception &e )
			{
				e.prepend( "Opening file \"" + filename + "\" : " );
				throw;
			}
			catch (...)
			{
				throw IOException( "FileIndexedIO: Caught error while reading index in '" + filename + "'");
			}
		}
	}

	assert( m_device );
	assert( m_stream );
	assert( m_stream->is_complete() );
	assert( m_index );
}

FileIndexedIO::StreamFile::~StreamFile()
{
	boost::optional<Imf::Int64> indexEnd = flush();

	if ( indexEnd && ( m_openmode == IndexedIO::Write || m_openmode == IndexedIO::Append ) )
	{
		std::fstream *f = static_cast< std::fstream * >( m_device );

		f->seekg( 0, std::ios::end );
		Imf::Int64 fileEnd = f->tellg();

		// .. and the length of that file extends beyond the end of the index
		if ( fileEnd > *indexEnd )
		{
			/// Close the file before truncation
			delete m_stream;
			delete m_device;
			m_stream = 0;
			m_device = 0;

			/// Truncate the file at the end of the index
			int err = truncate( m_filename.c_str(), *indexEnd );
			if ( err != 0 )
			{
				msg( Msg::Error, "FileIndexedIO::StreamFile", boost::format ( "Error truncating file '%s' to %d bytes: %s" ) % m_filename % (*indexEnd) % strerror( err ) );
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

	FilteredStream f;
	f.push<>( d );

	return StreamIndexedIO::StreamFile::canRead( f );
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
	const std::string filename = p.native_file_string();

	if (! fs::exists(filename) && (mode & IndexedIO::Read))
	{
		throw FileNotFoundIOException(filename);
	}

	open( new StreamFile( filename, mode ), root, mode );
}

FileIndexedIO::FileIndexedIO( const FileIndexedIO *other ) : StreamIndexedIO( other )
{
}

FileIndexedIO::~FileIndexedIO()
{
}

IndexedIO * FileIndexedIO::duplicate(Node *rootNode) const
{
	// duplicate the IO interface changing the current node
	FileIndexedIO *other = new FileIndexedIO( this );
	assert( rootNode );
	other->m_node = rootNode;
	return other;
}
