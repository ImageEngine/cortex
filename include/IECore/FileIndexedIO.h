//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
#include <fstream>

#include "IndexedIOInterface.h"
#include "Exception.h"

namespace IECore
{

/// An implementation of IndexedIOInterface which operates within a single file on disk.
class FileIndexedIO : public IndexedIOInterface
{
	public:
		
		static IndexedIOInterfacePtr create(const std::string &path, const IndexedIO::EntryID &root, IndexedIO::OpenMode mode);
		
		static bool canRead( const std::string &path );
	
		/// Open an existing device or create a new one
		FileIndexedIO(const std::string &path, const IndexedIO::EntryID &root, IndexedIO::OpenMode mode);
		
		virtual ~FileIndexedIO();

		IndexedIOInterfacePtr resetRoot() const;

		void chdir(const IndexedIO::EntryID &name);

		void mkdir(const IndexedIO::EntryID &name);

		IndexedIO::EntryID pwd();

		IndexedIO::EntryList ls(IndexedIOFilterPtr f=0);

		IndexedIO::Entry ls(const IndexedIO::EntryID &name);

		unsigned long rm(const IndexedIO::EntryID &name);

		void write(const IndexedIO::EntryID &name, const float *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const double *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const half *x, unsigned long arrayLength);		
		void write(const IndexedIO::EntryID &name, const int *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const long *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const unsigned int *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const char *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const unsigned char *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const std::string *x, unsigned long arrayLength);
		void write(const IndexedIO::EntryID &name, const float &x);
		void write(const IndexedIO::EntryID &name, const double &x);
		void write(const IndexedIO::EntryID &name, const half &x);		
		void write(const IndexedIO::EntryID &name, const int &x);
		void write(const IndexedIO::EntryID &name, const long &x);
		void write(const IndexedIO::EntryID &name, const std::string &x);
		void write(const IndexedIO::EntryID &name, const unsigned int &x);
		void write(const IndexedIO::EntryID &name, const char &x);
		void write(const IndexedIO::EntryID &name, const unsigned char &x);

		void read(const IndexedIO::EntryID &name, float *&x, unsigned long arrayLength);
		void read(const IndexedIO::EntryID &name, double *&x, unsigned long arrayLength);
		void read(const IndexedIO::EntryID &name, half *&x, unsigned long arrayLength);		
		void read(const IndexedIO::EntryID &name, int *&x, unsigned long arrayLength);
		void read(const IndexedIO::EntryID &name, long *&x, unsigned long arrayLength);
		void read(const IndexedIO::EntryID &name, unsigned int *&x, unsigned long arrayLength);
		void read(const IndexedIO::EntryID &name, char *&x, unsigned long arrayLength);
		void read(const IndexedIO::EntryID &name, unsigned char *&x, unsigned long arrayLength);
		void read(const IndexedIO::EntryID &name, std::string *&x, unsigned long arrayLength);
		void read(const IndexedIO::EntryID &name, float &x);
		void read(const IndexedIO::EntryID &name, double &x);
		void read(const IndexedIO::EntryID &name, half &x);		
		void read(const IndexedIO::EntryID &name, int &x);
		void read(const IndexedIO::EntryID &name, long &x);
		void read(const IndexedIO::EntryID &name, std::string &x);
		void read(const IndexedIO::EntryID &name, unsigned int &x);
		void read(const IndexedIO::EntryID &name, char &x);
		void read(const IndexedIO::EntryID &name, unsigned char &x);
	
	protected:
	
		FileIndexedIO(const FileIndexedIO &other, const IndexedIO::EntryID &root, IndexedIO::OpenMode mode);
		
		/// Check for existence of the entry with specified type
		bool exists(const IndexedIOPath &path, IndexedIO::EntryType e) const;
		
		/// Check for existence of the entry with any type		
		bool exists(const IndexedIO::EntryID &name) const;	
		
		/// Variant of "rm" which allows exceptions to be optionally thrown
		/// if the entry to remove does not exist.	
		unsigned long rm(const IndexedIO::EntryID &name, bool throwIfNonExistent);
	
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
		
		class Index;
		IE_CORE_DECLAREPTR( Index );
		
		class IndexedFile;
		IE_CORE_DECLAREPTR( IndexedFile );		
		
		IndexedFilePtr m_indexedFile;		
		
		class Node;
		IE_CORE_DECLAREPTR( Node );
		
		NodePtr m_currentDirectoryNode, m_rootDirectoryNode;
		
		bool find( const IndexedIO::EntryID &name, NodePtr &node ) const;
		NodePtr insert( const IndexedIO::EntryID &name );
};	

IE_CORE_DECLAREPTR( FileIndexedIO )

}

#endif // IE_CORE_FILEINDEXEDIO_H
