//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_STREAMINDEXEDIO_H
#define IE_CORE_STREAMINDEXEDIO_H

#include <map>
#include <iostream>
#include <fstream>
#include "tbb/recursive_mutex.h"
#include "boost/optional.hpp"
#include "boost/iostreams/filtering_stream.hpp"

#include "IECore/Export.h"
#include "IECore/IndexedIO.h"
#include "IECore/Exception.h"
#include "IECore/VectorTypedData.h"

namespace IECore
{
/// Abstract base class implementation of IndexedIO which operates with a stream file handle.
/// It handles data instancing transparently for compact file sizes.
/// Read operations are thread safe on read-only opened files.
/// \ingroup ioGroup
class IECORE_API StreamIndexedIO : public IndexedIO
{
	public:

		IE_CORE_DECLARERUNTIMETYPED( StreamIndexedIO, IndexedIO );

		virtual ~StreamIndexedIO();

		virtual IndexedIO::OpenMode openMode() const;

		void path( IndexedIO::EntryIDList &result ) const;

		bool hasEntry( const IndexedIO::EntryID &name ) const;

		const IndexedIO::EntryID &currentEntryId() const;

		void entryIds( IndexedIO::EntryIDList &names ) const;

		void entryIds( IndexedIO::EntryIDList &names, IndexedIO::EntryType type ) const;

		IndexedIOPtr subdirectory( const IndexedIO::EntryID &name, IndexedIO::MissingBehaviour missingBehaviour = IndexedIO::ThrowIfMissing );

		ConstIndexedIOPtr subdirectory( const IndexedIO::EntryID &name, IndexedIO::MissingBehaviour missingBehaviour = IndexedIO::ThrowIfMissing ) const;

		IndexedIO::Entry entry( const IndexedIO::EntryID &name ) const;

		IndexedIOPtr createSubdirectory( const IndexedIO::EntryID &name );

		void remove( const IndexedIO::EntryID &name );

		void removeAll();

		IndexedIOPtr parentDirectory();

		ConstIndexedIOPtr parentDirectory() const;

		IndexedIOPtr directory( const IndexedIO::EntryIDList &path, IndexedIO::MissingBehaviour missingBehaviour = IndexedIO::ThrowIfMissing );

		ConstIndexedIOPtr directory( const IndexedIO::EntryIDList &path, IndexedIO::MissingBehaviour missingBehaviour = IndexedIO::ThrowIfMissing ) const;

		void commit();

		void write(const IndexedIO::EntryID &name, const float *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const double *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const half *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const int *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const int64_t *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const uint64_t *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const unsigned int *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const char *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const unsigned char *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const std::string *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const short *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const unsigned short *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const InternedString *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const float &x);
		void write(const IndexedIO::EntryID &name, const double &x);
		void write(const IndexedIO::EntryID &name, const half &x);
		void write(const IndexedIO::EntryID &name, const int &x);
		void write(const IndexedIO::EntryID &name, const int64_t &x);
		void write(const IndexedIO::EntryID &name, const uint64_t &x);
		void write(const IndexedIO::EntryID &name, const std::string &x);
		void write(const IndexedIO::EntryID &name, const unsigned int &x);
		void write(const IndexedIO::EntryID &name, const char &x);
		void write(const IndexedIO::EntryID &name, const unsigned char &x);
		void write(const IndexedIO::EntryID &name, const short &x);
		void write(const IndexedIO::EntryID &name, const unsigned short &x);

		void read(const IndexedIO::EntryID &name, float *&x, unsigned long arrayLength) const;
		void read(const IndexedIO::EntryID &name, double *&x, unsigned long arrayLength) const;
		void read(const IndexedIO::EntryID &name, half *&x, unsigned long arrayLength) const;
		void read(const IndexedIO::EntryID &name, int *&x, unsigned long arrayLength) const;
		void read(const IndexedIO::EntryID &name, int64_t *&x, unsigned long arrayLength) const;
		void read(const IndexedIO::EntryID &name, uint64_t *&x, unsigned long arrayLength) const;
		void read(const IndexedIO::EntryID &name, unsigned int *&x, unsigned long arrayLength) const;
		void read(const IndexedIO::EntryID &name, char *&x, unsigned long arrayLength) const;
		void read(const IndexedIO::EntryID &name, unsigned char *&x, unsigned long arrayLength) const;
		void read(const IndexedIO::EntryID &name, std::string *&x, unsigned long arrayLength) const;
		void read(const IndexedIO::EntryID &name, short *&x, unsigned long arrayLength) const;
		void read(const IndexedIO::EntryID &name, unsigned short *&x, unsigned long arrayLength) const;
		void read(const IndexedIO::EntryID &name, InternedString *&x, unsigned long arrayLength) const;
		void read(const IndexedIO::EntryID &name, float &x) const;
		void read(const IndexedIO::EntryID &name, double &x) const;
		void read(const IndexedIO::EntryID &name, half &x) const;
		void read(const IndexedIO::EntryID &name, int &x) const;
		void read(const IndexedIO::EntryID &name, int64_t &x) const;
		void read(const IndexedIO::EntryID &name, uint64_t &x) const;
		void read(const IndexedIO::EntryID &name, std::string &x) const;
		void read(const IndexedIO::EntryID &name, unsigned int &x) const;
		void read(const IndexedIO::EntryID &name, char &x) const;
		void read(const IndexedIO::EntryID &name, unsigned char &x) const;
		void read(const IndexedIO::EntryID &name, short &x) const;
		void read(const IndexedIO::EntryID &name, unsigned short &x) const;

	protected:

		class Index;
		IE_CORE_DECLAREPTR( Index );

		class Node;
		IE_CORE_DECLAREPTR( Node );

		class StringCache;

		/// Class that provides access to the stream file.
		class StreamFile : public RefCounted
		{
			public:
				virtual ~StreamFile();

				void seekg( size_t pos, std::ios_base::seekdir dir );
				void seekp( size_t pos, std::ios_base::seekdir dir );
				void read( char *buffer, size_t size );
				void write( const char *buffer, size_t size );
				Imf::Int64 tellg();
				Imf::Int64 tellp();

				IndexedIO::OpenMode openMode() const;

				// returns a read lock, when thread-safety is required.
				typedef tbb::recursive_mutex Mutex;
				typedef Mutex::scoped_lock MutexLock;
				Mutex & mutex();

				// utility function that returns a temporary buffer for io operations (not thread safe).
				char *ioBuffer( unsigned long size );

				/// called after the main index is saved to disk, ready to close the file.
				virtual void flush( size_t endPosition );

				static bool canRead( std::iostream &stream );

			protected:

				StreamFile( IndexedIO::OpenMode mode );
				/// called once after construction. Assigns a stream and tells if the stream is empty.
				// This function allocates and if in read-mode also reads the Index of the file.
				void setStream( std::iostream *stream, bool emptyFile );

				IndexedIO::OpenMode m_openmode;
				std::iostream *m_stream;
				Mutex m_mutex;

				unsigned long m_ioBufferLen;
				char *m_ioBuffer;
		};
		IE_CORE_DECLAREPTR( StreamFile );

		/// Create an instance with unnitialized state. Must call open() method.
		StreamIndexedIO();

		/// Constructor based on the node
		StreamIndexedIO( Node &node );

		/// Opens a file using the given IndexedFile accessor
		void open( StreamFilePtr file, const IndexedIO::EntryIDList &root );

		/// Variant of "removeChild" which allows exceptions to be optionally thrown
		/// if the entry to remove does not exist.
		void remove( const IndexedIO::EntryID &name, bool throwIfNonExistent );

		// Write an array of POD types
		template<typename T>
		void write(const IndexedIO::EntryID &name, const T *x, unsigned long arrayLength);

		// Write an array of POD types (without temporary buffers - used on little endian platforms)
		template<typename T>
		void rawWrite(const IndexedIO::EntryID &name, const T *x, unsigned long arrayLength);

		// Read an array of POD types
		template<typename T>
		void read(const IndexedIO::EntryID &name, T *&x, unsigned long arrayLength) const;

		// Read an array of POD types (without temporary buffers - used on little endian platforms)
		template<typename T>
		void rawRead(const IndexedIO::EntryID &name, T *&x, unsigned long arrayLength) const;

		// Write an instance of a type which is able to flatten itself.
		template<typename T>
		void write(const IndexedIO::EntryID &name, const T &x);

		// Write an instance of a type which is able to flatten itself (without temporary buffers - used on little endian platforms).
		template<typename T>
		void rawWrite(const IndexedIO::EntryID &name, const T &x);

		// Read an instance of a type which is able to unflatten itself.
		template<typename T>
		void read(const IndexedIO::EntryID &name, T &x) const;

		// Read an instance of a type which is able to unflatten itself (without temporary buffers - used on little endian platforms).
		template<typename T>
		void rawRead(const IndexedIO::EntryID &name, T &x) const;

		// Duplicates this object by mapping it to a different root node. Used when the subdirectory functions are called.
		// This function does not duplicate the file handle like the public duplicate does. It works with any openMode.
		virtual IndexedIO *duplicate(Node &rootNode) const = 0;

		/// forces writing the index to the file and preventing any further changes.
		/// this function can be called by derived classes (MemoryIndexedIO).
		void flush();

		StreamFile &streamFile() const;

	private :

		// \todo Either rename Node to MemberData or add it's member here and save one raw pointer.
		Node *m_node;

		void setRoot( const IndexedIO::EntryIDList &root );

};

IE_CORE_DECLAREPTR( StreamIndexedIO )

}

#endif // IE_CORE_STREAMINDEXEDIO_H
