//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_FILEINDEXEDIO_H
#define IE_CORE_FILEINDEXEDIO_H

#include <map>
#include <iostream>
#include <fstream>

#include "boost/optional.hpp"

#include "IndexedIO.h"
#include "Exception.h"
#include "VectorTypedData.h"

namespace IECore
{
/// An implementation of IndexedIO which operates within a single file on disk.
/// \todo Most of the implementation of this class would be better of in a "StreamIndexedIO" class which
/// FileIndexedIO and MemoryIndexedIO derive from. MemoryIndexedIO wasn't implemented that cleanly in the first
/// place due to the necessity to maintain binary compatibility.
/// \ingroup ioGroup
class FileIndexedIO : public IndexedIO
{
	public:

		IE_CORE_DECLAREMEMBERPTR( FileIndexedIO );

		static IndexedIOPtr create(const std::string &path, const std::string &root, IndexedIO::OpenMode mode);

		static bool canRead( const std::string &path );

		/// Open an existing device or create a new one
		FileIndexedIO(const std::string &path, const std::string &root, IndexedIO::OpenMode mode);

		virtual ~FileIndexedIO();

		virtual IndexedIO::OpenMode openMode() const;

		void path( IndexedIO::EntryIDList &result ) const;

		bool hasEntry( const IndexedIO::EntryID &name ) const;

		void entryIds( IndexedIO::EntryIDList &names ) const;

		void entryIds( IndexedIO::EntryIDList &names, IndexedIO::EntryType type ) const;

		IndexedIOPtr subdirectory( const IndexedIO::EntryID &name, IndexedIO::MissingBehavior missingBehavior = IndexedIO::ThrowIfMissing );

		ConstIndexedIOPtr subdirectory( const IndexedIO::EntryID &name, IndexedIO::MissingBehavior missingBehavior = IndexedIO::ThrowIfMissing ) const;

		IndexedIO::Entry entry( const IndexedIO::EntryID &name ) const;

		void remove( const IndexedIO::EntryID &name );

		void removeAll();

		IndexedIOPtr parentDirectory();

		ConstIndexedIOPtr parentDirectory() const;

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

		ConstCharVectorDataPtr buf();

	protected:

		class Index;
		IE_CORE_DECLAREPTR( Index );

		class IndexedFile;
		IE_CORE_DECLAREPTR( IndexedFile );

		class Node;
		IE_CORE_DECLAREPTR( Node );

		/// The mode this device was opened with
		IndexedIO::OpenMode m_mode;

		/// Variant of "removeChild" which allows exceptions to be optionally thrown
		/// if the entry to remove does not exist.
		void remove( const IndexedIO::EntryID &name, bool throwIfNonExistent );

		// Write an array of POD types
		template<typename T>
		void write(const IndexedIO::EntryID &name, const T *x, unsigned long arrayLength);

		// Read an array of POD types
		template<typename T>
		void read(const IndexedIO::EntryID &name, T *&x, unsigned long arrayLength) const;

		// Write an instance of a type which is able to flatten itself.
		template<typename T>
		void write(const IndexedIO::EntryID &name, const T &x);

		// Read an instance of a type which is able to unflatten itself.
		template<typename T>
		void read(const IndexedIO::EntryID &name, T &x) const;

		IndexedFilePtr m_indexedFile;

		NodePtr m_node;

		/// \todo Should be virtual
		boost::optional<Imf::Int64> flush();

		/// \todo Add virtual method to obtain device name ( e.g filename, "memory", etc )

		std::iostream *device();

		void open( std::iostream *device, const std::string &root, IndexedIO::OpenMode mode, bool newStream = false );

		FileIndexedIO();

		FileIndexedIO( const FileIndexedIO *other, Node *newRoot );

		// duplicates this object by mapping it to a different root node.
		virtual IndexedIOPtr duplicate(Node *rootNode) const;

	private :

		bool setRoot( const std::string &root );

};

IE_CORE_DECLAREPTR( FileIndexedIO )

}

#endif // IE_CORE_FILEINDEXEDIO_H
