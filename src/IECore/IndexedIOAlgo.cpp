//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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

#include "IECore/IndexedIOAlgo.h"

#include "tbb/task_scheduler_init.h"
#include "tbb/task.h"

#include <atomic>

using namespace IECore;
using namespace IECore::IndexedIOAlgo;

namespace
{

template<typename T, typename Callback>
class Copier
{
	public:
		void handleValue( const IndexedIO *src, IndexedIO *dst, const IndexedIO::Entry &entry, Callback &callback )
		{
			T value;
			src->read( entry.id(), value );
			dst->write( entry.id(), value );
		}

		void handleArray( const IndexedIO *src, IndexedIO *dst, const IndexedIO::Entry &entry, Callback &callback )
		{
			std::vector<T> array( entry.arrayLength() );
			T *ptr = array.data();
			src->read( entry.id(), ptr, entry.arrayLength() );
			dst->write( entry.id(), ptr, entry.arrayLength() );
		}
};

template<typename T, typename Callback>
class Reader
{
	public:
		void handleValue( const IndexedIO *src, IndexedIO *dst, const IndexedIO::Entry &entry, Callback &callback )
		{
			T value;
			src->read( entry.id(), value );

			callback( sizeof( T ) );
		}

		void handleArray( const IndexedIO *src, IndexedIO *dst, const IndexedIO::Entry &entry, Callback &callback )
		{
			std::vector<T> array( entry.arrayLength() );
			T *ptr = array.data();
			src->read( entry.id(), ptr, entry.arrayLength() );

			callback( sizeof( T ) * entry.arrayLength() );
		}
};

template<typename Callback>
class Reader<std::string, Callback>
{
	public:
		void handleValue( const IndexedIO *src, IndexedIO *dst, const IndexedIO::Entry &entry, Callback &callback )
		{
			std::string value;
			src->read( entry.id(), value );

			callback( value.size() );
		}

		void handleArray( const IndexedIO *src, IndexedIO *dst, const IndexedIO::Entry &entry, Callback &callback )
		{

			std::vector<std::string> array( entry.arrayLength() );
			std::string *ptr = array.data();
			src->read( entry.id(), ptr, entry.arrayLength() );

			size_t numBytes = 0;
			for( const auto &str : array )
			{
				numBytes += str.size();
			}

			callback( numBytes );
		}
};

template<template<typename, typename> class Handler, typename Callback>
void handleFile( const IndexedIO *src, IndexedIO *dst, IndexedIO::EntryID fileName, Callback &c )
{
	IndexedIO::Entry entry = src->entry( fileName );

	switch( entry.dataType() )
	{
		case IndexedIO::Invalid:
			break;
		case IndexedIO::Float:
			Handler<float, Callback>().handleValue( src, dst, entry, c );
			break;
		case IndexedIO::FloatArray:
			Handler<float, Callback>().handleArray( src, dst, entry, c );
			break;
		case IndexedIO::Double:
			Handler<double, Callback>().handleValue( src, dst, entry, c );
			break;
		case IndexedIO::DoubleArray:
			Handler<double, Callback>().handleArray( src, dst, entry, c );
			break;
		case IndexedIO::Int:
			Handler<int, Callback>().handleValue( src, dst, entry, c );
			break;
		case IndexedIO::IntArray:
			Handler<int, Callback>().handleArray( src, dst, entry, c );
			break;
		case IndexedIO::Long:
			Handler<long, Callback>().handleValue( src, dst, entry, c );
			break;
		case IndexedIO::LongArray:
			Handler<long, Callback>().handleArray( src, dst, entry, c );
			break;
		case IndexedIO::String:
			Handler<std::string, Callback>().handleValue( src, dst, entry, c );
			break;
		case IndexedIO::StringArray:
			Handler<std::string, Callback>().handleArray( src, dst, entry, c );
			break;
		case IndexedIO::UInt:
			Handler<unsigned int, Callback>().handleValue( src, dst, entry, c );
			break;
		case IndexedIO::UIntArray:
			Handler<unsigned int, Callback>().handleArray( src, dst, entry, c );
			break;
		case IndexedIO::Char:
			Handler<char, Callback>().handleValue( src, dst, entry, c );
			break;
		case IndexedIO::CharArray:
			Handler<char, Callback>().handleArray( src, dst, entry, c );
			break;
		case IndexedIO::UChar:
			Handler<unsigned char, Callback>().handleValue( src, dst, entry, c );
			break;
		case IndexedIO::UCharArray:
			Handler<unsigned char, Callback>().handleArray( src, dst, entry, c );
			break;
		case IndexedIO::Half:
			Handler<half, Callback>().handleValue( src, dst, entry, c );
			break;
		case IndexedIO::HalfArray:
			Handler<half, Callback>().handleArray( src, dst, entry, c );
			break;
		case IndexedIO::Short:
			Handler<short, Callback>().handleValue( src, dst, entry, c );
			break;
		case IndexedIO::ShortArray:
			Handler<short, Callback>().handleArray( src, dst, entry, c );
			break;
		case IndexedIO::UShort:
			Handler<unsigned short, Callback>().handleValue( src, dst, entry, c );
			break;
		case IndexedIO::UShortArray:
			Handler<unsigned short, Callback>().handleArray( src, dst, entry, c );
			break;
		case IndexedIO::Int64:
			Handler<int64_t, Callback>().handleValue( src, dst, entry, c );
			break;
		case IndexedIO::Int64Array:
			Handler<int64_t, Callback>().handleArray( src, dst, entry, c );
			break;
		case IndexedIO::UInt64:
			Handler<uint64_t, Callback>().handleValue( src, dst, entry, c );
			break;
		case IndexedIO::UInt64Array:
			Handler<uint64_t, Callback>().handleArray( src, dst, entry, c );
			break;
		case IndexedIO::InternedStringArray:
			Handler<InternedString, Callback>().handleArray( src, dst, entry, c );
			break;
	}
}

void recursiveCopy( const IndexedIO *src, IndexedIO *dst )
{
	IndexedIO::EntryIDList fileNames;
	src->entryIds( fileNames, IndexedIO::EntryType::File );

	for( const auto &fileName : fileNames )
	{
		int dummy = 0;
		handleFile<Copier, int>( src, dst, fileName, dummy );
	}

	IndexedIO::EntryIDList directoryNames;
	src->entryIds( directoryNames, IndexedIO::EntryType::Directory );

	for( const auto &directoryName : directoryNames )
	{
		IndexedIOPtr childDst = dst->subdirectory( directoryName, IndexedIO::CreateIfMissing );
		recursiveCopy( src->subdirectory( directoryName, IndexedIO::ThrowIfMissing ).get(), childDst.get() );
	}
}

//! Task for traversing all files in parallel. New tasks are spawned for each directory
template<template<typename, typename> class FileHandler, typename FileCallback>
class FileTask : public tbb::task
{

	public :

		FileTask( const IndexedIO *src, FileCallback &fileCallback )
		: m_src( src ), m_fileCallback( fileCallback )
		{
		}

		~FileTask() override
		{
		}

		task *execute() override
		{
			IndexedIO::EntryIDList fileNames;
			m_src->entryIds( fileNames, IndexedIO::EntryType::File );

			for( const auto &fileName : fileNames )
			{
				handleFile<FileHandler, FileCallback>( m_src, nullptr, fileName, m_fileCallback );
			}

			IndexedIO::EntryIDList directoryNames;
			m_src->entryIds( directoryNames, IndexedIO::EntryType::Directory );

			set_ref_count( 1 + directoryNames.size() );

			std::vector<ConstIndexedIOPtr> childDirectories;
			childDirectories.reserve(directoryNames.size());
			for( const auto &directoryName : directoryNames )
			{
				childDirectories.push_back( m_src->subdirectory( directoryName, IndexedIO::ThrowIfMissing ) );
			}

			for( const auto &childDirectory : childDirectories )
			{
				FileTask *t = new( allocate_child() ) FileTask( childDirectory.get() , m_fileCallback );
				spawn( *t );
			}

			wait_for_all();

			return nullptr;
		}

	private :
		const IndexedIO *m_src;
		FileCallback &m_fileCallback;
};

} // namespace

namespace IECore
{
namespace IndexedIOAlgo
{

void copy( const IndexedIO *src, IndexedIO *dst )
{
	::recursiveCopy( src, dst );
}

FileStats<size_t> parallelReadAll( const IndexedIO *src )
{
	FileStats<std::atomic<size_t> > fileStats;

	auto fileCallback = [&fileStats]( size_t numBytes )
	{
		fileStats.addBlock( numBytes );
	};

	FileTask<Reader, decltype( fileCallback )> *task = new( tbb::task::allocate_root() ) FileTask<Reader, decltype( fileCallback )>( src, fileCallback );
	tbb::task::spawn_root_and_wait( *task );
	return fileStats;
}

} // IndexedIOAlgo
} // IECore