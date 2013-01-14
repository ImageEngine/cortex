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

#include "boost/optional.hpp"
#include "boost/iostreams/filtering_stream.hpp"

#include "IndexedIO.h"
#include "Exception.h"
#include "VectorTypedData.h"

namespace IECore
{
/// Abstract base class implementation of IndexedIO which operates with a stream file handle.
/// It handles data instancing transparently for compact file sizes.
/// \ingroup ioGroup
class StreamIndexedIO : public IndexedIO
{
	public:

		IE_CORE_DECLAREMEMBERPTR( StreamIndexedIO );

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

		void remove( const IndexedIO::EntryID &name );

		void removeAll();

		IndexedIOPtr parentDirectory();

		ConstIndexedIOPtr parentDirectory() const;

		IndexedIOPtr directory( const IndexedIO::EntryIDList &path, IndexedIO::MissingBehaviour missingBehaviour = IndexedIO::ThrowIfMissing );

		ConstIndexedIOPtr directory( const IndexedIO::EntryIDList &path, IndexedIO::MissingBehaviour missingBehaviour = IndexedIO::ThrowIfMissing ) const;

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
		void write(const IndexedIO::EntryID &name, const IndexedIO::EntryIDList &x);

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
		void read(const IndexedIO::EntryID &name, IndexedIO::EntryIDList &x) const;

	protected:

		class Index;
		IE_CORE_DECLAREPTR( Index );

		class Node;
		IE_CORE_DECLAREPTR( Node );

		/// Class that provides access to the stream file
		class StreamFile : public RefCounted
		{
			public:

				typedef boost::iostreams::filtering_stream< boost::iostreams::bidirectional_seekable > FilteredStream;

				FilteredStream *m_stream;
				std::iostream *m_device;
				IndexedIO::OpenMode m_openmode;

				virtual ~StreamFile();

				/// Obtain the index for this file
				Index* index() const;

				/// Seek to a particular node within the file for reading
				void seekg( Node* node );

				/// Write some data to the file. Its position is automatically allocated within the file, and the node
				/// is updated to record this offset along with its size (or it's turned into a hardlink if the data is already stored by other node).
				/// The hardlinks will only be created for nodes that have been saved on the same session. So edit mode will not be that great.
				void write(Node* node, const char *data, Imf::Int64 size);

				/// Saves to the file changes to the index. This function is used by the destructor.
				/// If the index has changed, than it returns the offset where the file ends.
				boost::optional<Imf::Int64> flush();

				std::iostream *device();

				static bool canRead( FilteredStream &stream );

			protected:

				StreamFile( IndexedIO::OpenMode mode, Index *index );
				void setDevice( std::iostream *device );
				void newIndex();
				void readIndex();

				IndexPtr m_index;

		};
		IE_CORE_DECLAREPTR( StreamFile );

		/// Create an instance with unnitialized state. Must call open() method.
		StreamIndexedIO();

		/// Copy constructor used when duplicating the file
		StreamIndexedIO( const StreamIndexedIO *other );

		/// Opens a file using the given IndexedFile accessor
		void open( StreamFilePtr file, const IndexedIO::EntryIDList &root, IndexedIO::OpenMode mode );

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

		boost::optional<Imf::Int64> flush();

		// duplicates this object by mapping it to a different root node.
		virtual IndexedIO *duplicate(Node *rootNode) const = 0;


		/// The mode this device was opened with
		IndexedIO::OpenMode m_mode;

		StreamFilePtr m_streamFile;

		Node * m_node;

	private :

		void setRoot( const IndexedIO::EntryIDList &root );

		char *ioBuffer( unsigned long size ) const;

		mutable unsigned long m_ioBufferLen;
		mutable char *m_ioBuffer;

};

IE_CORE_DECLAREPTR( StreamIndexedIO )

}

#endif // IE_CORE_STREAMINDEXEDIO_H
