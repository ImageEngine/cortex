//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2014, Image Engine Design Inc. All rights reserved.
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

#define __STDC_LIMIT_MACROS
#include "IECore/StreamIndexedIO.h"

#include "IECore/ByteOrder.h"
#include "IECore/CompoundData.h"
#include "IECore/MemoryStream.h"
#include "IECore/MessageHandler.h"
#include "IECore/MurmurHash.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

#include "blosc.h"

#include "tbb/spin_rw_mutex.h"

#include "boost/format.hpp"
#include "boost/iostreams/device/file.hpp"
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/stream.hpp"
#include "boost/optional.hpp"
#include "boost/tokenizer.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <list>
#include <map>
#include <set>

#include <fcntl.h>
#ifndef _MSC_VER
	#include <unistd.h>
#endif
#include <stdint.h>

#define HARDLINK				127
#define SUBINDEX_DIR			126

static const Imf::Int64 g_unversionedMagicNumber = 0x0B00B1E5;
static const Imf::Int64 g_versionedMagicNumber = 0xB00B1E50;

/// File format history:
/// Version 4: introduced hard links (automatic data deduplication), also ability to store InternedString data.
/// Version 5: introduced subindex as zipped data blocks (to reduce size of the main index).
///            Hard links are represented as regular data nodes, that points to same data on file (no removal of data ever).
///            Removed the linkCount field on the data nodes.
/// Version 6: compress large (1kb) DataNodes using blosc
/// Version 7: compress index using blosc(lz4) instead of gzip
/// \todo Store SubIndexSize and NodeCount as unsigned 64bit integers
static const Imf::Int64 g_currentVersion = 7;

/// FileFormat ::= Data Index IndexOffset Version MagicNumber
/// Data ::= DataEntry*
/// Index ::= zip(StringCache NodeTree FreePages)

/// DataEntry ::= Stores data from nodes:
///                [Data nodes] binary data indexed by DataOffset/DataSize and
///                [Subindex]   SubIndexSize zip(NodeCount NodeTree*) indexed by SubIndexOffset.
/// SubIndexSize :: = uint32 - number of bytes in the zipped subindex that follows

/// StringCache ::= NumStrings String*
/// NumStrings ::= int64
/// String ::= StringLength char*
/// StringLength ::= int64

/// NodeTree Node* ( A Directory node followed by it's child nodes )
/// Node ::= EntryType EntryStringCacheID NodeCount ( if EntryType == Directory )
///          EntryType EntryStringCacheID DataType ArrayLength DataOffset DataSize ( if EntryType == File )
///			 EntryType EntryStringCacheID SubIndexOffset ( If EntryType == SUBINDEX_DIR )
/// EntryType ::= char ( value from IndexedIO::EntryType )
/// EntryStringCacheID ::= int64 ( index in StringCache )
/// DataType ::= char ( value from IndexedIO::DataType )
/// ArrayLength ::= int64 ( if DataType is array, then this tells how long they are )
/// NodeID ::= int64 ( unique Id of this node in the file )
/// ParentNodeID ::= int64 ( Id for the parent node )
/// DataOffset ::= int64 ( this is offset where the data is located )
/// DataSize ::= int64 ( number of bytes stored in the data section )
/// NodeCount ::= uint32 ( number of child nodes in the directory - stored right after this node leading to recursive definition of a tree )
/// SubIndexOffset :: = int64 ( offset in the Data block where there's a zipped index that contains all the child nodes from this node - and possibly other nodes )

/// FreePages ::= NumFreePages FreePage*
/// NumFreePages ::= int64
/// FreePage ::= FreePageOffset FreePageSize
/// FreePageOffset ::= int64
/// FreePageSize ::= int64

/// IndexOffset ::= int64 ( offset in the file where the Index zipped block starts )
/// Version ::= int64 (file format version)
/// MagicNumber ::= int64

using namespace IECore;
namespace io = boost::iostreams;

IE_CORE_DEFINERUNTIMETYPEDDESCRIPTION( StreamIndexedIO )

//// Templated functions for stream files //////

template<typename F, typename T>
void writeLittleEndian( F &f, const T &n )
{
	const T nl = asLittleEndian<>(n);
	f.write( (const char*) &nl, sizeof(T) );
}

template<typename F, typename T>
void readLittleEndian( F &f, T &n )
{
	f.read( (char*) &n, sizeof(T) );

	if (bigEndian())
	{
		n = reverseBytes<>(n);
	}
	else
	{
		/// Already little endian
	}
}

namespace IECore
{

/// Base class for providing lock free reads
class StreamIndexedIO::PlatformReader
{
	public:
		virtual ~PlatformReader();
		virtual bool read( char *buffer, size_t size, size_t pos ) = 0;
		static std::unique_ptr<PlatformReader> create( const std::string &fileName );
};

#ifndef _MSC_VER

/// Posix Reader for Linux & OSX
class PosixPlatformReader : public StreamIndexedIO::PlatformReader
{
	public:
		~PosixPlatformReader();
		PosixPlatformReader( const std::string &fileName );
		bool read( char *buffer, size_t size, size_t pos ) override;
	private:
		int m_fileHandle;
};

PosixPlatformReader::PosixPlatformReader( const std::string &fileName )
{
	m_fileHandle = ::open( fileName.c_str(), O_RDONLY);
}

PosixPlatformReader::~PosixPlatformReader()
{
	::close(m_fileHandle);
}

bool PosixPlatformReader::read( char *buffer, size_t size, size_t pos )
{
	ssize_t result = pread(m_fileHandle, buffer, size, pos);

	if (result < 0)
	{
		return false;
	}

	return (size_t) result == size;
}

#endif

StreamIndexedIO::PlatformReader::~PlatformReader()
{
}

std::unique_ptr<StreamIndexedIO::PlatformReader> StreamIndexedIO::PlatformReader::create(const std::string& fileName)
{
#ifndef _MSC_VER
	PlatformReader* p = new PosixPlatformReader(fileName);
	return std::unique_ptr<StreamIndexedIO::PlatformReader>(p);
#else
	return nullptr;
#endif
}

}// IECore

class StreamIndexedIO::StringCache
{
	public:

		StringCache() : m_prevId(0), m_ioBuffer(nullptr), m_ioBufferLen(0)
		{
			m_idToStringMap.reserve(100);
		}

		template < typename F >
		StringCache( F &f ) : m_prevId(0), m_ioBuffer(nullptr), m_ioBufferLen(0)
		{
			Imf::Int64 sz;
			readLittleEndian(f,sz);

			m_idToStringMap.reserve(sz + 100);

			for (Imf::Int64 i = 0; i < sz; ++i)
			{
				const char *s = read(f);

				Imf::Int64 id;
				readLittleEndian( f,id );

				m_prevId = std::max( id, m_prevId );

				m_stringToIdMap[s] = id;
				if ( id >= m_idToStringMap.size() )
				{
					m_idToStringMap.resize(id+1, (const char *)"");
				}
				m_idToStringMap[id] = s;
			}
		}

		template < typename F >
		void write( F &f ) const
		{
			Imf::Int64 sz = m_stringToIdMap.size();
			writeLittleEndian( f,sz );

			for (StringToIdMap::const_iterator it = m_stringToIdMap.begin();
				it != m_stringToIdMap.end(); ++it)
			{
				write(f, it->first);

				writeLittleEndian(f,it->second);
			}
		}

		Imf::Int64 find( const IndexedIO::EntryID &s ) const
		{
			StringToIdMap::const_iterator it = m_stringToIdMap.find( s );
			if ( it == m_stringToIdMap.end() )
			{
				throw IOException( (boost::format ( "StringCache: could not find string %s!" ) % s.value() ).str() );
			}
			return it->second;
		}

		Imf::Int64 find( const IndexedIO::EntryID &s, bool errIfNotFound = true )
		{
			StringToIdMap::const_iterator it = m_stringToIdMap.find( s );

			if ( it == m_stringToIdMap.end() )
			{
				if (errIfNotFound)
				{
					throw IOException( (boost::format ( "StringCache: could not find string %s!" ) % s.value() ).str() );
				}

				Imf::Int64 id = ++m_prevId;

				m_stringToIdMap[s] = id;
				if ( id >= m_idToStringMap.size() )
				{
					m_idToStringMap.resize(id+1, (const char *)"");
				}
				m_idToStringMap[id] = s;

				return id;
			}
			else
			{
				return it->second;
			}
		}

		const IndexedIO::EntryID &findById( const Imf::Int64 &id ) const
		{
			if ( id >= m_idToStringMap.size() )
			{
				throw IOException( (boost::format ( "StringCache: invalid string ID %d!" ) % id ).str() );
			}
			return m_idToStringMap[id];
		}

		void add( const IndexedIO::EntryID &s )
		{
			(void)find(s, false);
		}

		Imf::Int64 size() const
		{
			return m_stringToIdMap.size();
		}

	protected:

		template < typename F >
		void write( F &f, const std::string &s ) const
		{
			Imf::Int64 sz = s.size();
			writeLittleEndian( f, sz );

			/// Does not include null terminator
			f.write( s.c_str(), sz * sizeof(char) );
		}

		template < typename F >
		const char *read( F &f ) const
		{
			Imf::Int64 sz;
			readLittleEndian( f, sz );

			if ( m_ioBufferLen < sz + 1 )
			{
				if ( m_ioBuffer )
				{
					delete [] m_ioBuffer;
				}
				m_ioBufferLen = sz+1;
				m_ioBuffer = new char[m_ioBufferLen];
			}
			f.read( m_ioBuffer, sz*sizeof(char));
			m_ioBuffer[sz] = '\0';
			return m_ioBuffer;
		}

		Imf::Int64 m_prevId;

		typedef std::map< IndexedIO::EntryID, Imf::Int64 > StringToIdMap;
		typedef std::vector< IndexedIO::EntryID > IdToStringMap;

		StringToIdMap m_stringToIdMap;
		IdToStringMap m_idToStringMap;

		mutable char *m_ioBuffer;
		mutable unsigned long m_ioBufferLen;
};

namespace
{

const char* indexCompressor = "lz4";
const int indexCompressionLevel = 9;

const static std::map<std::string, int> nameCodeMapping = {{"blosclz", 0}, {"lz4", 1}, {"lz4hc", 2}, {"snappy", 3}, {"zlib", 4}};

//! map blosc compressor name to a int which we can serialise into
//! the indexedIO header. We don't use the blosc header defined values incase they change.
int getCompressionCode( const std::string &compressor )
{
	const auto it = nameCodeMapping.find( compressor );
	if( it != nameCodeMapping.end() )
	{
		return it->second;
	}
	return -1;
}

//! look up compressor name from id.
std::string getCompressor( int code )
{
	for (const auto it : nameCodeMapping)
	{
		if (it.second == code )
		{
			return it.first;
		}
	}
	return "unknown";
}

/// compress 'size' bytes at 'data' into 'outputBuffer'
/// compressionLevel, compressor & threadCount are passed directly to blosc ( see blosc.h )
/// if  'size' is greater than the max buffer blosc can handle we split into a number of independently compressed blocks.
/// returns the number of compression blocks
/// 'outputBuffer' contains the compressed block data and is resized in this function.
/// 'maxBlockSize' is useful for testing the compression block size without using buffers greater than 2GB
size_t compress(
	const char *data,
	size_t size,
	std::vector<char> &outputBuffer,
	int compressionLevel,
	const std::string &compressor,
	int threadCount,
	boost::optional<size_t> maxBlockSize = boost::optional<size_t>(),
	size_t minCompressedBlockSize = 1024U
)
{
	size_t maxCompressedBlockSize = maxBlockSize ? maxBlockSize.get() : BLOSC_MAX_BUFFERSIZE;

	if( size < minCompressedBlockSize )
	{
		return 0;
	}

	size_t bytesToCompress = size;
	const char *currentBlockCompressed = data;

	size_t numBlocks = 0;

	/// this isn't enough space in some edge cases but is sufficient in the common case
	/// and we check if we have enough size in the compression loop
	outputBuffer.resize( size + BLOSC_MAX_OVERHEAD );
	char *writePtr = outputBuffer.data();
	size_t writerBufferBytes = outputBuffer.size();

	size_t totalCompressedSize = 0;

	while ( bytesToCompress )
	{
		size_t currentBlockUncompressedSize = std::min( maxCompressedBlockSize, bytesToCompress );
		size_t compressedBufferMaxSize = currentBlockUncompressedSize + BLOSC_MAX_OVERHEAD;
		if( writerBufferBytes < compressedBufferMaxSize )
		{
			size_t additionalBytes = (size_t) ( compressedBufferMaxSize - writerBufferBytes );
			outputBuffer.resize( outputBuffer.size() + additionalBytes );
		}

		int compressedSize = blosc_compress_ctx(
			compressionLevel,
			true,
			4,
			currentBlockUncompressedSize,
			currentBlockCompressed,
			writePtr,
			compressedBufferMaxSize,
			compressor.c_str(),
			0,
			threadCount
		);

		if ( compressedSize < 0 )
		{
			outputBuffer.clear();
			return 0;
		}

		writerBufferBytes -= compressedSize;
		writePtr += compressedSize;

		totalCompressedSize += compressedSize;

		currentBlockCompressed += currentBlockUncompressedSize;
		bytesToCompress -= currentBlockUncompressedSize;
		numBlocks++;
	}

	// if we've compressed all the input then just set the
	// output buffer
	outputBuffer.resize( totalCompressedSize );

	return numBlocks;
}

/// decompress a memory buffer which is formed by a number of blosc compressed blocks
/// returns the number of compression blocks
/// 'outputBuffer' contains the decompressed data and is resized in this function if not large enough.
size_t decompress( const char *data, size_t size, std::vector<char> &outputBuffer, int threadCount )
{
	std::vector<std::pair<size_t, size_t>> blockSizes;
	size_t totalDecompressedSize = 0;
	size_t compressedBytesRead = 0;

	while( compressedBytesRead < size )
	{
		size_t compressedNumBytes = 0, decompressedNumBytes = 0, blockSize = 0;
		blosc_cbuffer_sizes( &data[compressedBytesRead], &decompressedNumBytes, &compressedNumBytes, &blockSize );

		blockSizes.push_back( std::make_pair( compressedNumBytes, decompressedNumBytes ) );
		totalDecompressedSize += decompressedNumBytes;
		compressedBytesRead += compressedNumBytes;
	}

	if( outputBuffer.size() < totalDecompressedSize )
	{
		std::vector<char> b ( totalDecompressedSize );
		outputBuffer.swap( b );
	}

	compressedBytesRead = 0;
	size_t decompressedBytesWritten = 0;
	for( const auto &blockSize : blockSizes )
	{
		int bloscResult = blosc_decompress_ctx( &data[compressedBytesRead], &outputBuffer[decompressedBytesWritten], blockSize.second, threadCount );

		if( bloscResult <= 0 )
		{
			throw IECore::IOException( "StreamIndexedIO (decompress) - Corrupted compressed archive" );
		}

		compressedBytesRead += blockSize.first;
		decompressedBytesWritten += blockSize.second;
	}

	return blockSizes.size();
}

} // namespace


/// NodeBase is a base class for nodes representing the index
// It's designed to keep the size of nodes to a minimum for dealing with large indexes
// As a result we deliberately not deriving from RefCounted to save the size of the refcount and the vptr (due to the virtual methods)
// Because it has no virtual destructor, we provide the destroy() function that carefully casts the Node pointer to the appropriate derived type before deleting it.
// NodeType enumerates the derived types, and m_nodeType is defined at construction by the derived classes
class NodeBase
{
	public :

		enum NodeType : char
		{
			Base = 0,
			SmallData = 1,
			Data = 2,
			Directory = 3,
			SubIndex = 4
		};

		NodeBase( NodeType type, IndexedIO::EntryID name ) : m_name(name), m_nodeType(type) {}

		inline const IndexedIO::EntryID &name()
		{
			return m_name;
		}

		inline NodeType nodeType()
		{
			return m_nodeType;
		}

		static bool compareNames(const NodeBase* a, const NodeBase* b)
		{
			return a->m_name < b->m_name;
		}

		static void destroy( NodeBase *n );

protected :

		// name of the node in the current directory
		const IndexedIO::EntryID m_name;

		// using char instead of enum to compact members in one word
		const NodeType m_nodeType;

};

/// Class that represents small data nodes
class SmallDataNode : public NodeBase
{
	public :
		typedef uint16_t Length;
		typedef uint32_t Size;

		static const size_t maxArrayLength = UINT16_MAX;
		static const size_t maxSize = UINT32_MAX;

		SmallDataNode( IndexedIO::EntryID name, IndexedIO::DataType dataType, Imf::Int64 arrayLength, Imf::Int64 size, Imf::Int64 offset ) : NodeBase(
			NodeBase::SmallData, name
		), m_dataType( dataType ), m_arrayLength( (Length) arrayLength ), m_size( (Size) size ), m_offset( offset )
		{
		}

		inline IndexedIO::DataType dataType() const
		{
			return static_cast<IndexedIO::DataType>(m_dataType);
		}

		inline Imf::Int64 arrayLength() const
		{
			return m_arrayLength;
		}

		inline Imf::Int64 size() const
		{
			return m_size;
		}

		inline Imf::Int64 offset() const
		{
			return m_offset;
		}

		/// SmallDataNodes are never compressed so we just return the size
		inline Imf::Int64 decompressedSize() const
		{
			return m_size;
		}

		/// SmallDataNodes are never compressed so we have 0 compressed blocks
		inline Imf::Int64 compressedBlocks() const
		{
			return 0;
		}


	protected :

		/// data fields from IndexedIO::Entry
		// using char instead of enum to compact members in one word
		const char m_dataType;

		/// data fields from IndexedIO::Entry
		const Length m_arrayLength;

		/// The size of this node's data chunk within the file
		const Size m_size;

		/// The offset in the file to this node's data
		const Imf::Int64 m_offset;
};

/// Class that represents Data nodes
class DataNode : public NodeBase
{
	public :
		static const size_t maxArrayLength = UINT64_MAX;
		static const size_t maxSize = UINT64_MAX;

		DataNode(
			IndexedIO::EntryID name,
			IndexedIO::DataType dataType,
			Imf::Int64 arrayLength,
			Imf::Int64 size,
			Imf::Int64 offset,
			Imf::Int64 decompressedSize,
			unsigned short numCompressedBlocks
		) : NodeBase(
			NodeBase::Data, name
		),
			m_dataType( dataType ),
			m_arrayLength( arrayLength ),
			m_size( size ),
			m_decompressedSize( decompressedSize ),
			m_numCompressedBlocks( numCompressedBlocks ),
			m_offset( offset )
		{
		}

		inline IndexedIO::DataType dataType()
		{
			return m_dataType;
		}

		inline Imf::Int64 arrayLength()
		{
			return m_arrayLength;
		}

		inline Imf::Int64 size()
		{
			return m_size;
		}

		inline Imf::Int64 offset()
		{
			return m_offset;
		}

		inline Imf::Int64 decompressedSize() const
		{
			return m_decompressedSize;
		}

		inline unsigned short compressedBlocks() const
		{
			return m_numCompressedBlocks;
		}

		void copyFrom( DataNode *other )
		{
			m_dataType = other->m_dataType;
			m_arrayLength = other->m_arrayLength;
			m_offset = other->m_offset;
			m_size = other->m_size;
			m_decompressedSize = other->m_decompressedSize;
			m_numCompressedBlocks = other->m_numCompressedBlocks;
		}

	protected :

		/// data fields from IndexedIO::Entry
		IndexedIO::DataType m_dataType;

		/// data fields from IndexedIO::Entry
		Imf::Int64 m_arrayLength;

		/// The size of this node's data chunk within the file
		Imf::Int64 m_size;

		/// Size of the data chunk after decompression and the number of compressed blocks in the top 8 bits
		Imf::Int64 m_decompressedSize;

		/// Size of the data chunk after decompression and the number of compressed blocks in the top 8 bits
		unsigned short m_numCompressedBlocks;

		/// The offset in the file to this node's data
		Imf::Int64 m_offset;
};


/// A compressed subindex node
class SubIndexNode : public NodeBase
{
	public :
		SubIndexNode(IndexedIO::EntryID name, Imf::Int64 offset) : NodeBase(NodeBase::SubIndex, name), m_offset(offset) {}

		inline Imf::Int64 offset()
		{
			return m_offset;
		}

	protected :
		/// The offset in the file to this node's subindex block if m_subindex is not NoSubIndex.
		const Imf::Int64 m_offset;

};

/// A directory node within an index
/// It also represents subindex directory nodes by setting m_subindex at the root and all it's child nodes to true.
class DirectoryNode : public NodeBase
{
	public :
		/// Directory nodes can save it's children to sub-indexes to free resources and reduce the size of the main index.
		/// Once saved to a subindex, they become read-only.
		enum SubIndexMode {
			NoSubIndex = 0,
			SavedSubIndex,
			LoadedSubIndex,
		};

		typedef std::vector< NodeBase* > ChildMap;

		// regular constructor
		DirectoryNode(IndexedIO::EntryID name, boost::optional<uint32_t> numChildren = boost::optional<uint32_t>()) : NodeBase( NodeBase::Directory, name ),
			m_subindex( NoSubIndex ),
			m_sortedChildren( false ),
			m_subindexChildren( false ),
			m_offset( 0 ),
			m_parent( nullptr )
		{
			if ( numChildren )
			{
				m_children.reserve( numChildren.get() );
			}

		}

		// constructor used when building a directory based on an existing SubIndexNode (because we want to load the contents soon).
		DirectoryNode( SubIndexNode *subindex, DirectoryNode *parent ) : NodeBase(NodeBase::Directory, subindex->name()), m_subindex(SavedSubIndex), m_sortedChildren(false), m_subindexChildren(false), m_offset(subindex->offset()), m_parent(parent) {}

		// returns what's the state of this directory, whether it's contents are in a subindex and whether they have been loaded or not.
		inline SubIndexMode subindex()
		{
			return static_cast<SubIndexMode>(m_subindex);
		}

		inline bool subindexChildren() const
		{
			return m_subindexChildren;
		}

		inline Imf::Int64 offset() const
		{
			return m_offset;
		}

		inline DirectoryNode *parent()
		{
			return m_parent;
		}

		/// Returns the current list of child Nodes.
		// This function is not thread-safe and Index::lockDirectory must be used in read-only access
		// \todo we may want to restrict more the access to the internal children and add the manipulation methods in the class instead.
		inline ChildMap &children()
		{
			return m_children;
		}

		// This function is not thread-safe and Index::lockDirectory must be used in read-only access
		inline void sortChildren()
		{
			if ( !m_sortedChildren )
			{
				std::sort( m_children.begin(), m_children.end(), NodeBase::compareNames );
				m_sortedChildren = true;
			}
		}

		// This function is not thread-safe and Index::lockDirectory must be used in read-only access
		inline ChildMap::iterator findChild( IndexedIO::EntryID name )
		{
			sortChildren();
			NodeBase search(NodeBase::Base, name);
			ChildMap::iterator it = std::lower_bound(m_children.begin(), m_children.end(), &search, NodeBase::compareNames );
			if ( it != m_children.end() )
			{
				if ( (*it)->name() != name )
				{
					return m_children.end();
				}
			}
			return it;
		}

		/// registers a child node in this node
		void registerChild( NodeBase* c );

		void path( IndexedIO::EntryIDList &result ) const;

		// Function that changes the SubIndex Mode to SavedSubIndex and saves memory by deallocating it's children.
		void setSubIndexOffset( Imf::Int64 offset );

		// Indicates to this Directory that it's contents have been retrieved from the subindex.
		void recoveredSubIndex();

	protected :

		char m_subindex;	// using char instead of enum to compact members in one word
		bool m_sortedChildren; // same as above
		bool m_subindexChildren;	// true if one or more children are subindex. Helps avoiding the mutex...

		/// The offset in the file to this node's subindex block if m_subindex is not NoSubIndex.
		Imf::Int64 m_offset;

		/// A pointer to the parent node in the tree - will be NULL for the root node
		DirectoryNode* m_parent;

		/// Sorted list of node's children (DirectoryNode or DataNode/SmallDataNode)
		ChildMap m_children;

};

// holds the private member data for StreamIndexedIO instance and provides high level access to the directory nodes, including thread-safety
class StreamIndexedIO::Node
{
	public :

		// location & size information of data block in a file
		struct Info
		{
			Info() : offset( 0 ), size( 0 ), decompressedSize( 0 )
			{
			}

			size_t offset;
			size_t size;
			size_t decompressedSize;
			size_t numCompressedBlocks;
		};

		/// Construct a new Node in the given index with the given numeric id
		Node(StreamIndexedIO::Index* index, DirectoryNode *dirNode);

		void childNames( IndexedIO::EntryIDList &names ) const;
		void childNames( IndexedIO::EntryIDList &names, IndexedIO::EntryType ) const;

		const IndexedIO::EntryID &name() const;

		bool hasChild( const IndexedIO::EntryID &name ) const;

		// Returns the named child directory node or NULL if not existent. Loads the subindex for the child nodes (if applicable).
		DirectoryNode* directoryChild( const IndexedIO::EntryID &name ) const;

		/// returns information about the Data node
		bool dataChildInfo( const IndexedIO::EntryID &name, Info &info ) const;

		DirectoryNode* addChild( const IndexedIO::EntryID & childName );
		void addDataChild(
			const IndexedIO::EntryID &childName,
			IndexedIO::DataType dataType,
			size_t arrayLen,
			size_t offset,
			size_t size,
			size_t decompressedSize,
			size_t numCompressedBlocks
		);

		void removeChild( const IndexedIO::EntryID &childName, bool throwException = true );

		StreamIndexedIO::IndexPtr m_idx;
		DirectoryNode *m_node;
};

//! Small scoped class to read from a given data block in a file, 
//! decompressing if required.
class StreamIndexedIO::Reader
{
	public:

		//! If an outputBuffer is supplied then it has to be large enough to store info.decompressedSize bytes of data
		//! and if one isn't supplied then a suitably sized buffer is created and freed on destruction.
		Reader( StreamIndexedIO::StreamFile &f, const Node::Info &info, int threadCount = 1, char *outputBuffer = nullptr )
			: m_data( nullptr ),
			m_decompressedData( outputBuffer ),
			m_size( info.size ),
			m_decompressedSize( info.decompressedSize ),
			m_ownDecompressedData( outputBuffer == nullptr )
		{
			if( m_ownDecompressedData )
			{
				m_decompressedData = new char[m_decompressedSize];
			}

			if( info.numCompressedBlocks > 0 )
			{
				m_data = new char[info.size];
				f.read( m_data, info.size, info.offset );

				const char* readPtr = m_data;
				char* writePtr = m_decompressedData;

				size_t writeBufferSize = m_decompressedSize;
				for ( size_t block = 0; block < info.numCompressedBlocks; ++block )
				{
					/// read the blosc header so we can decompress this block
					size_t compresedNumBytes = 0, decompressedNumBytes = 0, blockSize = 0;
					blosc_cbuffer_sizes( readPtr, &decompressedNumBytes , &compresedNumBytes, &blockSize );

					int bloscResult = blosc_decompress_ctx( readPtr, writePtr, decompressedNumBytes, threadCount );

					if( bloscResult <= 0 )
					{
						throw IECore::IOException( "StreamIndexedIO::Reader - Corrupted compressed archive" );
					}

					readPtr += compresedNumBytes;
					writePtr += decompressedNumBytes;
					writeBufferSize -= decompressedNumBytes;
				}
			}
			else
			{
				f.read( m_decompressedData, info.size, info.offset );
			}
		}

		~Reader()
		{
			if ( m_data )
			{
				delete[] m_data;
			}

			if( m_decompressedData && m_ownDecompressedData )
			{
				delete[] m_decompressedData;
			}
		}

		char *data() const
		{
			if( m_decompressedData )
			{
				return m_decompressedData;
			}
			else
			{
				return m_data;
			}
		}

		bool isCompressed() const
		{
			return m_size != m_decompressedSize;
		}

	private:
		char *m_data;
		char *m_decompressedData;
		Imf::Int64 m_size;
		Imf::Int64 m_decompressedSize;
		bool m_ownDecompressedData;
};

/// A tree to represent nodes in a filesystem, along with their locations in a file.
class StreamIndexedIO::Index : public RefCounted
{
	public:

		friend class Node;

		/// Construct an index from reading a file stream.
		Index( StreamIndexedIO::StreamFilePtr stream, const CompoundData *options = nullptr );
		~Index() override;

		/// function called right after construction
		void openStream();

		DirectoryNode *root() const;

		/// Allocate a new chunk of data of the requested size, returning its offset within the file
		Imf::Int64 allocate( Imf::Int64 sz );

		/// Deallocate a Data node's data block from the file.
		template< typename D >
		void deallocate( D* n );

		/// Queries the string cache
		StringCache &stringCache();

		StreamIndexedIO::StreamFile &streamFile() const;

		/// flushes index to the file
		void flush();

		/// Returns the offset after saving the data to file or the offset for a previously saved data (with matching hash)
		/// \param prefixSize If true than it will prepend to the block, the size of it
		Imf::Int64 writeUniqueData( const char *data, size_t size, bool prefixSize = false );

		struct WriteInfo
		{
			WriteInfo() : offset( 0 ), size( 0 ), numCompressedBlocks( 0 )
			{
			}

			Imf::Int64 offset;

			/// number of bytes written. i.e. compressed size if it's compressed
			size_t size;

			/// We split up files into compressed blocks as required by the BLOSC_MAX_BUFFERSIZE define.
			size_t numCompressedBlocks;
		};

		WriteInfo writeUniqueDataCompressed( const char *data, size_t size, bool prefixSize = false );

		/// flushes the children of the given directory node to a subindex in the file
		void commitNodeToSubIndex( DirectoryNode *n );

		/// read the subindex that contains the children of the given node
		void readNodeFromSubIndex( DirectoryNode *n );

		typedef tbb::spin_rw_mutex Mutex;
		typedef Mutex::scoped_lock MutexLock;
		/// Returns an appropriate mutex scoped lock to access the given Directory node.
		/// It selects on mutex from the pool, reducing the changes of blocking other threads that are accessing different locations.
		void lockDirectory( MutexLock &lock, const DirectoryNode *n, bool writeAccess = false ) const;

		int decompressionThreadCount() const { return m_decompressionThreadCount; }

		CompoundDataPtr metadata() const
		{
			CompoundDataPtr meta(new CompoundData());
			auto & writable = meta->writable();
			writable["version"] = new IntData( (int) m_version );
			writable["compressionLevel"] = new IntData( m_compressionLevel);
			writable["compressor"] = new StringData( m_compressor ) ;
			writable["compressionThreadCount"] = new IntData( m_compressionThreadCount );
			writable["decompressionThreadCount"] = new IntData( m_decompressionThreadCount);
			return meta;
		}

	protected:

		static const int MAX_MUTEXES = 11;

		/// defines a pool of mutexes for thread-safe access to the Node hierarchy
		mutable Mutex m_mutexes[ MAX_MUTEXES ];

		DirectoryNode *m_root;

		/// we keep all the removed nodes alive until the Index destruction
		std::vector< NodeBase * > m_removedNodes;

		Imf::Int64 m_version;

		bool m_hasChanged;

		Imf::Int64 m_offset;
		Imf::Int64 m_next;

		// only used on Version <= 4
		typedef std::vector< NodeBase* > IndexToNodeMap;
		IndexToNodeMap m_indexToNodeMap;

		typedef std::map< std::pair<MurmurHash,unsigned int>, Imf::Int64 > HashToDataMap;
		HashToDataMap m_hashToDataMap;

		StringCache m_stringCache;

		StreamIndexedIO::StreamFilePtr m_stream;

		struct FreePage;

		typedef std::map< Imf::Int64, FreePage* > FreePagesOffsetMap;
		typedef std::multimap< Imf::Int64, FreePage* > FreePagesSizeMap;

		FreePagesOffsetMap m_freePagesOffset;
		FreePagesSizeMap m_freePagesSize;

		int m_compressionLevel;
		int m_compressionThreadCount;
		int m_decompressionThreadCount;
		boost::optional<size_t> m_maxCompressedBlockSize;
		std::string m_compressor;

		struct FreePage
		{
			FreePage( Imf::Int64 offset, Imf::Int64 sz ) : m_offset(offset), m_size(sz) {}

			Imf::Int64 m_offset;
			Imf::Int64 m_size;

			FreePagesOffsetMap::iterator m_offsetIterator;
			FreePagesSizeMap::iterator m_sizeIterator;
		};

		void addFreePage( Imf::Int64 offset, Imf::Int64 sz );

		void deallocateWalk( NodeBase* n );

		/// Write the index to the file stream
		Imf::Int64 write();

		/// Write the node (and all child nodes) to a stream
		template < typename F >
		void writeNode( DirectoryNode *n, F &f );

		/// Write the subindex node to a stream
		template < typename F >
		void writeNode( SubIndexNode *n, F &f );

		/// Write the data node to a stream
		template < typename F, typename D >
		void writeDataNode( D *n, F &f );

		/// Serialize all the node's children to a stream
		template < typename F >
		void writeNodeChildren( DirectoryNode *n, F &f );

		template < typename F >
		void read( F &f );

		/// Read method used on previous file format versions up to version 4
		/// Returns a newly created Node.
		template < typename F >
		NodeBase *readNodeV4( F &f );

		/// Read method used on V5 IndexedIO
		/// Returns a newly created Node.
		template<typename F>
		NodeBase *readNodeV5( F &f );

		/// Replace the contents of this node with data read from a stream.
		/// Returns a newly created Node.
		template < typename F >
		NodeBase *readNode( F &f );
};

///////////////////////////////////////////////
//
// NodeBase
//
///////////////////////////////////////////////

void NodeBase::destroy( NodeBase *n )
{
	if ( !n )
	{
		return;
	}
	switch( n->nodeType() )
	{
		case NodeBase::Directory :
			{
				DirectoryNode *dn = static_cast< DirectoryNode *>(n);
				for (DirectoryNode::ChildMap::const_iterator it = dn->children().begin(); it != dn->children().end(); ++it)
				{
					destroy( *it );
				}
				delete dn;
				break;
			}
		case NodeBase::Data :
			{
				DataNode *dn = static_cast< DataNode *>(n);
				delete dn;
				break;
			}
		case NodeBase::SmallData :
			{
				SmallDataNode *dn = static_cast< SmallDataNode *>(n);
				delete dn;
				break;
			}
		case NodeBase::SubIndex :
			{
				SubIndexNode *dn = static_cast< SubIndexNode *>(n);
				delete dn;
				break;
			}
		default:
			throw Exception("Unknown node type!");
	}
}


///////////////////////////////////////////////
//
// DirectoryNode
//
///////////////////////////////////////////////

void DirectoryNode::registerChild( NodeBase* c )
{
	if ( !c )
	{
		throw Exception("Invalid pointer for child node!!");
	}

	if ( m_children.size() >= UINT32_MAX )
	{
		// we currently save childCount as a uint32... so we prevent new children by construction.
		throw IOException("StreamIndexedIO: Too many children under the same node!");
	}

	if ( c->nodeType() == NodeBase::Directory )
	{
		DirectoryNode *childNode = static_cast< DirectoryNode *>(c);
		if (childNode->m_parent)
		{
			throw IOException("StreamIndexedIO: Node already has parent!");
		}
		childNode->m_parent = this;
	}
	else if ( c->nodeType() == NodeBase::SubIndex )
	{
		m_subindexChildren = true;
	}
	m_children.push_back( c );
	m_sortedChildren = false;
}

void DirectoryNode::path( IndexedIO::EntryIDList &result ) const
{
	if ( m_parent )
	{
		m_parent->path( result );
		result.push_back( m_name );
	}
}

void DirectoryNode::setSubIndexOffset( Imf::Int64 offset )
{
	m_offset = offset;

	// mark this node as a saved in a subindex
	m_subindex = DirectoryNode::SavedSubIndex;

	// dealloc all the child nodes
	for (DirectoryNode::ChildMap::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
	{
		NodeBase::destroy( *it );
	}

	// remove all children from this node
	m_children.clear();
}

void DirectoryNode::recoveredSubIndex()
{
	m_subindex = DirectoryNode::LoadedSubIndex;
}


///////////////////////////////////////////////
//
// StreamIndexedIO::Node (begin)
//
///////////////////////////////////////////////

StreamIndexedIO::Node::Node(Index* index, DirectoryNode *dirNode) : m_idx(index), m_node(dirNode)
{
}

bool StreamIndexedIO::Node::hasChild( const IndexedIO::EntryID &name ) const
{
	Index::MutexLock lock;
	m_idx->lockDirectory( lock, m_node );
	DirectoryNode::ChildMap::const_iterator cit = m_node->findChild( name );
	return cit != m_node->children().end();
}

DirectoryNode* StreamIndexedIO::Node::directoryChild( const IndexedIO::EntryID &name ) const
{
	Index::MutexLock lock;
	m_idx->lockDirectory( lock, m_node );

	DirectoryNode::ChildMap::iterator it = m_node->findChild( name );
	if ( it != m_node->children().end() )
	{
		if ( (*it)->nodeType() == NodeBase::Directory )
		{
			DirectoryNode *dir = static_cast< DirectoryNode *>( (*it) );

			if ( dir->subindex() == DirectoryNode::SavedSubIndex )
			{
				if ( m_node->subindexChildren() )
				{
					lock.release();		/// we will not change the children dictionary, so we release the lock!
				}

				// this can occur when the user flushed a directory and right after tries to access it.
				m_idx->readNodeFromSubIndex( dir );
			}
			return dir;
		}
		else if ( (*it)->nodeType() == NodeBase::SubIndex )
		{
			SubIndexNode *subIndex = static_cast< SubIndexNode *>( (*it) );

			// build a Directory that knows it's flushed to a subindex.
			DirectoryNode *newDir = new DirectoryNode( subIndex, m_node );

			lock.release();		/// we release the lock while loading data..

			m_idx->readNodeFromSubIndex( newDir );

			// now that we loaded the whole thing, lock our Index for writing
			m_idx->lockDirectory( lock, m_node, true );

			// there's a chance that someone else already replaced the pointer...
			if ( (*it)->nodeType() == NodeBase::Directory )
			{
				lock.release();		/// we release the lock because we won't change the children anyways..
				NodeBase::destroy( newDir );
				return static_cast< DirectoryNode *>(*it);
			}

			// replace SubIndex by Directory node.
			(*it) = newDir;

			// and now we are ok to delete the SubIndexNode..
			delete subIndex;

			return newDir;
		}
	}
	return nullptr;
}

bool StreamIndexedIO::Node::dataChildInfo( const IndexedIO::EntryID &name, Info &info ) const
{
	Index::MutexLock lock;
	m_idx->lockDirectory( lock, m_node );

	DirectoryNode::ChildMap::const_iterator cit = m_node->findChild( name );
	if ( cit != m_node->children().end() )
	{
		NodeBase *p = *cit;

		if ( p->nodeType() == NodeBase::Data )
		{
			DataNode *n = static_cast< DataNode *>( p );
			info.offset = n->offset();
			info.size = n->size();
			info.decompressedSize = n->decompressedSize();
			info.numCompressedBlocks = n->compressedBlocks();
			return true;
		}
		else if ( p->nodeType() == NodeBase::SmallData )
		{
			SmallDataNode *n = static_cast< SmallDataNode *>( p );
			info.offset = n->offset();
			info.size = n->size();
			info.decompressedSize = n->decompressedSize();
			info.numCompressedBlocks = n->compressedBlocks();
			return true;
		}
	}
	return false;
}

DirectoryNode* StreamIndexedIO::Node::addChild( const IndexedIO::EntryID &childName )
{
	if ( m_node->subindex() )
	{
		throw Exception( "Cannot modify the file at current location! It was already committed to the file." );
	}

	if ( hasChild(childName) )
	{
		return nullptr;
	}

	DirectoryNode* child = new DirectoryNode(childName);
	if ( !child )
	{
		throw Exception( "Failed to allocate node!" );
	}
	m_idx->m_stringCache.add( childName );

	m_node->registerChild( child );

	m_idx->m_hasChanged = true;

	return child;
}

void StreamIndexedIO::Node::addDataChild(
	const IndexedIO::EntryID &childName,
	IndexedIO::DataType dataType,
	size_t arrayLen,
	size_t offset,
	size_t size,
	size_t decompressedSize,
	size_t numCompressedBlocks
)
{
	if ( m_node->subindex() )
	{
		throw Exception( "Cannot modify the file at current location! It was already committed to the file." );
	}

	if ( hasChild(childName) )
	{
		throw IOException( "StreamIndexedIO: Could not insert node '" + childName.value() + "' into index" );
	}

	m_idx->m_stringCache.add( childName );

	// SmallDataNodes should not be compressed.
	if( arrayLen <= SmallDataNode::maxArrayLength && size <= SmallDataNode::maxSize && ( size == decompressedSize ) && (numCompressedBlocks == 0) )
	{
		SmallDataNode* child = new SmallDataNode(childName, dataType, arrayLen, size, offset);
		if ( !child )
		{
			throw Exception( "Failed to allocate node!" );
		}
		m_node->registerChild( child );
	}
	else
	{
		if( numCompressedBlocks > std::numeric_limits<unsigned short>::max() )
		{
			throw IECore::Exception(
				boost::str(
					boost::format( "StreamIndexedIO::Node::addDataChild - Unable to store file with more than %1% compressed blocks " ) %
						std::numeric_limits<unsigned short>::max()
				)
			);
		}

		DataNode *child = new DataNode( childName, dataType, arrayLen, size, offset, decompressedSize, numCompressedBlocks );
		if ( !child )
		{
			throw Exception( "Failed to allocate node!" );
		}
		m_node->registerChild( child );
	}
	m_idx->m_hasChanged = true;
}

const IndexedIO::EntryID &StreamIndexedIO::Node::name() const
{
	return m_node->name();
}

void StreamIndexedIO::Node::childNames( IndexedIO::EntryIDList &names ) const
{
	names.clear();
	names.reserve( m_node->children().size() );

	Index::MutexLock lock;
	m_idx->lockDirectory( lock, m_node );

	for ( DirectoryNode::ChildMap::const_iterator cit = m_node->children().begin(); cit != m_node->children().end(); cit++ )
	{
		names.push_back( (*cit)->name() );
	}
}

void StreamIndexedIO::Node::childNames( IndexedIO::EntryIDList &names, IndexedIO::EntryType type ) const
{
	names.clear();
	names.reserve( m_node->children().size() );

	bool typeIsDirectory = ( type == IndexedIO::Directory );

	Index::MutexLock lock;
	m_idx->lockDirectory( lock, m_node );

	for ( DirectoryNode::ChildMap::const_iterator cit = m_node->children().begin(); cit != m_node->children().end(); cit++ )
	{
		NodeBase *cc = *cit;
		bool childIsDirectory = ( cc->nodeType() == NodeBase::Directory || cc->nodeType() == NodeBase::SubIndex );
		if ( typeIsDirectory == childIsDirectory )
		{
			names.push_back( (*cit)->name() );
		}
	}
}

void StreamIndexedIO::Node::removeChild( const IndexedIO::EntryID &childName, bool throwException )
{
	DirectoryNode::ChildMap::iterator it = m_node->findChild( childName );
	if ( it == m_node->children().end() )
	{
		if (throwException)
		{
			throw IOException( "StreamIndexedIO::Node::removeChild: Entry not found '" + childName.value() + "'" );
		}
		return;
	}

	NodeBase *child = *it;

	m_idx->deallocateWalk(child);

	m_node->children().erase( it );
}

///////////////////////////////////////////////
//
// StreamIndexedIO::Node (end)
//
///////////////////////////////////////////////

///////////////////////////////////////////////
//
// StreamIndexedIO::Index (begin)
//
///////////////////////////////////////////////

StreamIndexedIO::Index::Index( StreamIndexedIO::StreamFilePtr stream, const CompoundData *options )
	: m_root( nullptr ),
	m_version( g_currentVersion ),
	m_hasChanged( false ),
	m_offset( 0 ),
	m_next( 0 ),
	m_stream( stream ), m_compressionLevel( 0 ),
	m_compressionThreadCount(1),
	m_decompressionThreadCount(1), m_compressor( "lz4" )

{
	m_stringCache.add(IndexedIO::rootName);

	const char *compressionLevelEnvVar = getenv( "IECORE_STREAMINDEXEDIO_COMPRESSION" );
	if ( compressionLevelEnvVar )
	{
		char buffer[1024];
		if ( sscanf( compressionLevelEnvVar, "%s %i %i %i", &buffer[0], &m_compressionLevel, &m_compressionThreadCount, &m_decompressionThreadCount ) == 4 )
		{
			m_compressor = std::string( buffer );
		}
	}

	if ( options )
	{
		if ( const StringData* compressor = options->member<StringData>("compressor", false) )
		{
			m_compressor = compressor->readable();
		}

		if ( const IntData* compressionLevel = options->member<IntData>("compressionLevel", false) )
		{
			m_compressionLevel = compressionLevel->readable();
		}

		if ( const IntData* compressionThreadCount = options->member<IntData>("compressionThreadCount", false) )
		{
			m_compressionThreadCount = compressionThreadCount->readable();
		}

		if ( const IntData* decompressionThreadCount = options->member<IntData>("decompressionThreadCount", false) )
		{
			m_decompressionThreadCount = decompressionThreadCount->readable();
		}

		if ( const UIntData* maxCompressedBlockSize = options->member<UIntData>("maxCompressedBlockSize", false) )
		{
			m_maxCompressedBlockSize = maxCompressedBlockSize->readable();
		}
	}

	// validate our parameters
	m_compressionLevel = std::min( std::max( 0, m_compressionLevel ), 9 ); // todo replace with std::clamp in C++17
	m_compressionThreadCount = std::min( std::max( 1, m_compressionThreadCount ), 32 );
	m_decompressionThreadCount = std::min( std::max( 1, m_decompressionThreadCount ), 32 );

	if ( getCompressionCode( m_compressor ) == -1)
	{
		m_compressor = "lz4";
	}

}

StreamIndexedIO::Index::~Index()
{
	flush();

	assert( m_freePagesOffset.size() == m_freePagesSize.size() );

	for (FreePagesOffsetMap::iterator it = m_freePagesOffset.begin(); it != m_freePagesOffset.end(); ++it)
	{
		assert(it->second);
		delete it->second;
	}

	// dealloc all nodes, starting from root
	NodeBase::destroy( m_root );

	// dealloc removed nodes
	for ( std::vector< NodeBase * >::const_iterator it = m_removedNodes.begin(); it != m_removedNodes.end(); it++ )
	{
		NodeBase::destroy( *it );
	}
}

void StreamIndexedIO::Index::flush()
{
	if ( m_hasChanged )
	{
		Imf::Int64 end = write();
		assert( m_stream.get() );
		assert( m_hasChanged == false );
		m_stream->flush( end );
	}
}

void StreamIndexedIO::Index::openStream()
{
	if ( m_stream->openMode() & (IndexedIO::Append|IndexedIO::Read) )
	{
		StreamIndexedIO::StreamFile &f = *m_stream;

		m_hasChanged = false;

		f.seekg( 0, std::ios::end );
		Imf::Int64 fileLen;
		Imf::Int64 end = fileLen = f.tellg();
		f.seekg( end-1*sizeof(Imf::Int64), std::ios::beg );

		Imf::Int64 magicNumber = 0;
		readLittleEndian( f, magicNumber );

		if ( magicNumber == g_versionedMagicNumber )
		{
			end -= 3*sizeof(Imf::Int64);
			f.seekg( end, std::ios::beg );
			readLittleEndian( f, m_offset );
			readLittleEndian( f, m_version );
		}
		else if (magicNumber == g_unversionedMagicNumber )
		{
			m_version = 0;
			end -= 2*sizeof(Imf::Int64);
			f.seekg( end, std::ios::beg );
			readLittleEndian( f, m_offset );
		}
		else
		{
			throw IOException("Not a StreamIndexedIO file");
		}

		if (m_version >= 6)
		{
			end -= 2 * sizeof(int);
			f.seekg( end, std::ios::beg );

			int compressorCode;
			readLittleEndian( f, compressorCode );
			readLittleEndian( f, m_compressionLevel );
			m_compressor = getCompressor( compressorCode );
		}

		f.seekg( m_offset, std::ios::beg );

		if (m_version >= 2 )
		{
			// construct a memory stream source from the decompressed index data
			io::filtering_istream indexInStream;

			if( m_version >= 7 )
			{
				// read the compressed Index
				Imf::Int64 indexCompressedSize = end - m_offset;
				std::vector<char> compressedIndex( indexCompressedSize );
				f.read( &compressedIndex[0], indexCompressedSize );

				std::vector<char> decompressedIndex;
				decompress( &compressedIndex[0], indexCompressedSize, decompressedIndex, 1 );

				MemoryStreamSource source( &decompressedIndex[0], decompressedIndex.size(), false );
				indexInStream.push( source );
				assert( indexInStream.is_complete() );

				read( indexInStream );
			}
			else
			{
				char *compressedIndex = new char[end - m_offset];
				f.read( compressedIndex, end - m_offset );
				MemoryStreamSource source( compressedIndex, end - m_offset, true );
				indexInStream.push( io::gzip_decompressor() );
				indexInStream.push( source );
				assert( indexInStream.is_complete() );

				read( indexInStream );
			}
		}
		else
		{
			read( f );
		}
	}
	else
	{
		// creating a new empty Index
		m_root = new DirectoryNode(IndexedIO::rootName);
		m_hasChanged = true;
	}
}

DirectoryNode *StreamIndexedIO::Index::root() const
{
	return m_root;
}

StreamIndexedIO::StringCache &StreamIndexedIO::Index::stringCache()
{
	return m_stringCache;
}

StreamIndexedIO::StreamFile &StreamIndexedIO::Index::streamFile() const
{
	return *m_stream;
}

template < typename F >
NodeBase *StreamIndexedIO::Index::readNodeV4( F &f )
{
	char t;
	f.read( &t, sizeof(char) );

	IndexedIO::EntryType entryType = (IndexedIO::EntryType)t;

	bool isLink = false;

	if ( t == HARDLINK ) /// only at version = 4
	{
		entryType = IndexedIO::File;
		isLink = true;
	}

	const IndexedIO::EntryID *id;
	if (m_version >= 1)
	{
		Imf::Int64 stringId;
		readLittleEndian(f,stringId);
		id = &m_stringCache.findById( stringId );
	}
	else
	{
		Imf::Int64 entrySize;
		readLittleEndian( f,entrySize );
		char *s = new char[entrySize+1];
		f.read( s, entrySize );
		s[entrySize] = '\0';
		id = new IndexedIO::EntryID(s);
		delete[] s;
	}

	IndexedIO::DataType dataType = IndexedIO::Invalid;
	Imf::Int64 arrayLength = 0;

	if ( m_version < 2 || (!isLink && entryType == IndexedIO::File) )
	{
		f.read( &t, sizeof(char) );
		dataType = (IndexedIO::DataType)t;

		if ( IndexedIO::Entry::isArray( dataType ) || m_version < 3 )
		{
			readLittleEndian( f,arrayLength );
		}
	}

	Imf::Int64 nodeId;
	readLittleEndian( f,nodeId );

	Imf::Int64 parentId;
	readLittleEndian( f, parentId );

	NodeBase *result = nullptr;

	if ( entryType == IndexedIO::File )
	{
		Imf::Int64 offset, size, decompressedSize = 0, numCompressedBlocks = 0;

		if ( isLink )		// Only version 4
		{
			Imf::Int64 targetNodeId;
			// load Target Node ID in m_offset
			readLittleEndian( f,targetNodeId );
			offset = targetNodeId;
			// we cannot assure that the target node is already loaded,
			/// so we set size to zero for now and after we set it after the whole index is loaded.
			size = 0;
		}
		else
		{
			readLittleEndian( f,offset );
			readLittleEndian( f,size );

			decompressedSize = size;

			if ( m_version == 4 )
			{
				/// ignore link count data
				typedef uint16_t LinkCount;
				LinkCount linkCount;
				readLittleEndian(f,linkCount);
			}
		}
		DataNode *n = new DataNode( *id, dataType, arrayLength, size, offset, decompressedSize, numCompressedBlocks );
		result = n;
	}
	else // Directory
	{
		DirectoryNode *n = new DirectoryNode( *id );

		if ( m_version < 2 )
		{
			Imf::Int64 offset, size;
			readLittleEndian( f, offset );
			readLittleEndian( f, size );
		}
		result = n;
	}

	if ( nodeId && parentId != std::numeric_limits<Imf::Int64>::max() )
	{
		DirectoryNode* parent = nullptr;
		if ( parentId < m_indexToNodeMap.size() )
		{
			parent = static_cast< DirectoryNode * >( m_indexToNodeMap[parentId] );
			if ( parent->nodeType() != NodeBase::Directory )
			{
				throw IOException("StreamIndexedIO: parent is not a directory!");
			}
		}

		if ( !parent )
		{
			throw IOException("StreamIndexedIO: parentId not found");
		}

		parent->registerChild(result);
	}

	if ( nodeId >= m_indexToNodeMap.size() )
	{
		m_indexToNodeMap.resize(nodeId+1, nullptr);
	}
	m_indexToNodeMap[nodeId] = result;

	if ( m_version < 1 )
	{
		delete id;
	}

	return result;
}

template<typename F>
NodeBase *StreamIndexedIO::Index::readNodeV5( F &f )
{
	char entryType;
	f.read( &entryType, sizeof( char ) );

	Imf::Int64 stringId;
	readLittleEndian( f, stringId );

	if( entryType == IndexedIO::File )
	{
		char t;
		IndexedIO::DataType dataType = IndexedIO::Invalid;
		Imf::Int64 arrayLength = 0;
		f.read( &t, sizeof( char ) );
		dataType = (IndexedIO::DataType) t;

		if( IndexedIO::Entry::isArray( dataType ) )
		{
			readLittleEndian( f, arrayLength );
		}

		Imf::Int64 offset, size;
		readLittleEndian( f, offset );
		readLittleEndian( f, size );

		if( arrayLength <= SmallDataNode::maxArrayLength && size <= SmallDataNode::maxSize )
		{
			SmallDataNode *n = new SmallDataNode( m_stringCache.findById( stringId ), dataType, arrayLength, size, offset );
			return n;
		}
		else
		{
			DataNode *n = new DataNode( m_stringCache.findById( stringId ), dataType, arrayLength, size, offset, size, 0 );
			return n;
		}
	}
	else if( entryType == IndexedIO::Directory )
	{
		uint32_t nodeCount = 0;
		readLittleEndian( f, nodeCount );

		DirectoryNode *n = new DirectoryNode( m_stringCache.findById( stringId ), nodeCount );

		for( uint32_t c = 0; c < nodeCount; c++ )
		{
			NodeBase *child = readNodeV5( f );
			n->registerChild( child );
		}
		// force sorting all children so that read-only is multi-threaded
		n->sortChildren();
		return n;
	}
	else if( entryType == SUBINDEX_DIR )
	{
		Imf::Int64 offset;
		readLittleEndian( f, offset );
		SubIndexNode *n = new SubIndexNode( m_stringCache.findById( stringId ), offset );
		return n;
	}
	else
	{
		throw IOException( boost::str( boost::format( "StreamIndexedIO::Index::readNodeV5 Invalid EntryType found '%1%'" ) % entryType ) );
	}
}

template < typename F >
NodeBase *StreamIndexedIO::Index::readNode( F &f )
{
	NodeBase::NodeType nodeType;
	f.read( (char *) &nodeType, sizeof( nodeType ) );

	Imf::Int64 stringId;
	readLittleEndian( f, stringId );

	if( nodeType == NodeBase::NodeType::SmallData || nodeType == NodeBase::NodeType::Data )
	{
		char t;
		IndexedIO::DataType dataType = IndexedIO::Invalid;
		Imf::Int64 arrayLength = 0;
		f.read( &t, sizeof(char) );
		dataType = (IndexedIO::DataType)t;

		if ( IndexedIO::Entry::isArray( dataType ) )
		{
			readLittleEndian( f, arrayLength );
		}

		Imf::Int64 offset, size, decompressedSize, numCompressedBlocks = 0;
		readLittleEndian( f, offset );
		readLittleEndian( f, size );

		if( nodeType == NodeBase::NodeType::SmallData )
		{
			SmallDataNode *n = new SmallDataNode( m_stringCache.findById( stringId ), dataType, arrayLength, size, offset );
			return n;
		}
		else
		{
			unsigned short numCompressedBlocksStorage;
			readLittleEndian( f, decompressedSize );
			readLittleEndian( f, numCompressedBlocksStorage );
			numCompressedBlocks = numCompressedBlocksStorage;

			DataNode *n = new DataNode( m_stringCache.findById( stringId ), dataType, arrayLength, size, offset, decompressedSize, numCompressedBlocks );
			return n;
		}
	}
	else if( nodeType == NodeBase::NodeType::Directory )
	{
		uint32_t nodeCount = 0;
		readLittleEndian( f, nodeCount );

		DirectoryNode *n = new DirectoryNode( m_stringCache.findById( stringId ), nodeCount );

		for ( uint32_t c = 0; c < nodeCount; c++ )
		{
			NodeBase *child = readNode( f );
			n->registerChild( child );
		}
		// force sorting all children so that read-only is multi-threaded
		n->sortChildren();
		return n;
	}
	else if( nodeType == NodeBase::NodeType::SubIndex )
	{
		Imf::Int64 offset;
		readLittleEndian( f, offset );
		SubIndexNode *n = new SubIndexNode( m_stringCache.findById( stringId ), offset );
		return n;
	}
	else
	{
		throw IOException( boost::str( boost::format( "StreamIndexedIO::Index::readNode - Invalid EntryType found '%1%'" ) % nodeType ) );
	}
}

template < typename F >
void StreamIndexedIO::Index::read( F &f )
{
	if (m_version >= 1)
	{
		m_stringCache = StringCache( f );
	}

	if( m_version >= 6 )
	{
		/// current file format reading
		m_root = static_cast< DirectoryNode *>( readNode( f ) );

		if( m_root->nodeType() != NodeBase::Directory )
		{
			throw Exception( "StreamIndexedIO::Index::read - Root node is not a directory!!" );
		}
	}
	else if( m_version == 5 )
	{
		/// current file format reading
		m_root = static_cast< DirectoryNode *>( readNodeV5( f ) );

		if ( m_root->nodeType() != NodeBase::Directory)
		{
			throw Exception( "StreamIndexedIO::Index::read - Root node is not a directory!!" );
		}
	}
	else
	{
		/// Backward compatible reading

		Imf::Int64 numNodes;
		readLittleEndian( f, numNodes );
		m_indexToNodeMap.reserve( numNodes );

		for (Imf::Int64 i = 0; i < numNodes; i++)
		{
			readNodeV4( f );
		}
		m_root = static_cast< DirectoryNode *>( m_indexToNodeMap[0] );

		if ( m_root->nodeType() != NodeBase::Directory)
		{
			throw Exception( "StreamIndexedIO::Index::read - Root node is not a directory!!" );
		}

		// force sorting all children so that read-only is multi-threaded
		for (IndexToNodeMap::const_iterator it = m_indexToNodeMap.begin(); it != m_indexToNodeMap.end(); it++ )
		{
			NodeBase *n = *it;
			if ( !n )
			{
				continue;
			}
			if ( n->nodeType() == NodeBase::Directory )
			{
				static_cast< DirectoryNode * >(n)->sortChildren();
			}
		}

		if ( m_version == 4 )
		{
			// In Version 4, symlinks have to get the Entry information from their target nodes.
			for (IndexToNodeMap::const_iterator it = m_indexToNodeMap.begin(); it != m_indexToNodeMap.end(); it++ )
			{
				DataNode *n = static_cast< DataNode *>(*it);
				if ( !n || n->nodeType() != NodeBase::Data )
				{
					continue;
				}

				if ( !n->size() )
				{
					Imf::Int64 targetNodeId = n->offset();
					if ( targetNodeId >= m_indexToNodeMap.size() || !m_indexToNodeMap[targetNodeId] )
					{
						throw IOException("StreamIndexedIO: targetNodeId not found");
					}
					DataNode* targetNode = static_cast< DataNode *>(m_indexToNodeMap[targetNodeId]);
					if ( targetNode->nodeType() != NodeBase::Data )
					{
						throw IOException("StreamIndexedIO: targetNode if not of type File!" );
					}
					n->copyFrom( targetNode );
				}
			}
		}
	}

	if ( !m_root )
	{
		throw Exception( "No root node in file!" );
	}

	Imf::Int64 numFreePages;
	readLittleEndian( f, numFreePages );

	m_next = m_offset;

	for (Imf::Int64 i = 0; i < numFreePages; i++)
	{
		Imf::Int64 offset, sz;
		readLittleEndian( f, offset );
		readLittleEndian( f, sz );

		addFreePage( offset, sz );
	}
}

template < typename F, typename D >
void StreamIndexedIO::Index::writeDataNode( D *node, F &f )
{
	NodeBase::NodeType nodeType = node->nodeType();
	f.write( (char *) &nodeType, sizeof( char ) );

	Imf::Int64 id = m_stringCache.find( node->name() );
	writeLittleEndian( f, id );

	char t = node->dataType();
	f.write( &t, sizeof(char) );

	if ( IndexedIO::Entry::isArray(node->dataType()) )
	{
		writeLittleEndian<F,Imf::Int64>( f, node->arrayLength() );
	}

	writeLittleEndian(f, node->offset());
	writeLittleEndian<F,Imf::Int64>(f, node->size());

	if ( node->nodeType() == NodeBase::Data )
	{
		writeLittleEndian<F, Imf::Int64>( f, node->decompressedSize() );
		writeLittleEndian<F, unsigned short>(f, node->compressedBlocks() );
	}

}

template < typename F >
void StreamIndexedIO::Index::writeNode( SubIndexNode *node, F &f )
{
	NodeBase::NodeType nodeType = node->nodeType();
	f.write( (char *) &nodeType, sizeof( char ) );

	Imf::Int64 id = m_stringCache.find( node->name() );
	writeLittleEndian( f, id );
	writeLittleEndian(f, node->offset() );
}

template < typename F >
void StreamIndexedIO::Index::writeNodeChildren( DirectoryNode *n, F &f )
{
	uint32_t nodeCount = n->children().size();
	writeLittleEndian( f, nodeCount );

	for (DirectoryNode::ChildMap::const_iterator it = n->children().begin(); it != n->children().end(); ++it)
	{
		NodeBase *p = *it;
		switch( p->nodeType() )
		{
			case NodeBase::Data :
			{
				DataNode *childNode = static_cast< DataNode *>(p);
				writeDataNode( childNode, f );
				break;
			}
			case NodeBase::SmallData :
			{
				SmallDataNode *childNode = static_cast< SmallDataNode * >(p);
				writeDataNode( childNode, f );
				break;
			}
			case NodeBase::Directory :
			{
				DirectoryNode *childNode = static_cast< DirectoryNode *>(p);
				writeNode( childNode, f );
				break;
			}
			case NodeBase::SubIndex :
			{
				SubIndexNode *childNode = static_cast< SubIndexNode *>(p);
				writeNode( childNode, f );
				break;
			}
			default:
				throw Exception( "Invalid node type!" );
		}
	}
}

template < typename F >
void StreamIndexedIO::Index::writeNode( DirectoryNode *node, F &f )
{
	BOOST_STATIC_ASSERT( sizeof( NodeBase::NodeType ) == 1 );

	NodeBase::NodeType nodeType = node->subindex() ? NodeBase::NodeType::SubIndex : node->nodeType();
	f.write( (char *) &nodeType, sizeof( nodeType ) );

	Imf::Int64 id = m_stringCache.find( node->name() );
	writeLittleEndian( f, id );

	if ( node->subindex() )
	{
		writeLittleEndian(f, node->offset() );
	}
	else
	{
		writeNodeChildren( node, f );
	}
}

Imf::Int64 StreamIndexedIO::Index::write()
{
	StreamIndexedIO::StreamFile &f = *m_stream;

	/// Write index at end
	std::streampos indexStart = m_next;

	f.seekp( m_next, std::ios::beg );

	m_offset = indexStart;

	// todo estimate the memory required to avoid re-allocations?
	MemoryStreamSink sink;
	io::filtering_ostream indexOutStream;
	indexOutStream.push( sink );

	assert( indexOutStream.is_complete() );

	m_stringCache.write( indexOutStream );

	writeNode( m_root, indexOutStream );

	assert( m_freePagesOffset.size() == m_freePagesSize.size() );
	Imf::Int64 numFreePages = m_freePagesSize.size();

	// Write out number of free "pages"
	writeLittleEndian( indexOutStream, numFreePages);

	/// Write out each free page
	for ( FreePagesSizeMap::const_iterator it = m_freePagesSize.begin(); it != m_freePagesSize.end(); ++it)
	{
		writeLittleEndian( indexOutStream, it->second->m_offset );
		writeLittleEndian( indexOutStream, it->second->m_size );
	}

	/// To synchronize/close, etc.
	indexOutStream.pop();

	// compress the output stream using blosc before writing
	char* indexData = nullptr;
	std::streamsize indexDataSize;

	sink.get(indexData, indexDataSize);

	std::vector<char> compressedIndex;
	compress( indexData, indexDataSize, compressedIndex, indexCompressionLevel, indexCompressor, 1, BLOSC_MAX_BUFFERSIZE, 0);

	f.write( &compressedIndex[0], compressedIndex.size() );

	writeLittleEndian( f, getCompressionCode( m_compressor ));
	writeLittleEndian( f, m_compressionLevel );

	writeLittleEndian( f, m_offset );
	writeLittleEndian( f, g_currentVersion );
	writeLittleEndian( f, g_versionedMagicNumber );

	m_hasChanged = false;

	return f.tellp();
}

Imf::Int64 StreamIndexedIO::Index::allocate( Imf::Int64 sz )
{
	Imf::Int64 loc = 0;

	FreePagesSizeMap::iterator it = m_freePagesSize.find( sz );

	if (it == m_freePagesSize.end())
	{
		it = m_freePagesSize.upper_bound( sz );
	}

	if (it != m_freePagesSize.end())
	{
		assert( it->first >= sz );
		assert( m_freePagesOffset.find(it->second->m_offset) != m_freePagesOffset.end() );

		Imf::Int64 pageSize = it->first;
		FreePage *page = it->second;

		assert( page );
		assert( page->m_size == it->first );

		pageSize -= sz;

		if (pageSize == 0)
		{
			/// Page now used entirely, so delete from the free pages list
			m_freePagesSize.erase(it);
			m_freePagesOffset.erase( page->m_offset );

			assert( m_freePagesOffset.size() == m_freePagesSize.size() );

			loc = page->m_offset;
			delete page;
		}
		else
		{
			/// Adjust the page to account for the consumed space, and reinsert into the sorted structures
			m_freePagesSize.erase( it );
			m_freePagesOffset.erase( page->m_offset );

			assert( m_freePagesOffset.size() == m_freePagesSize.size() );

			loc = page->m_offset;

			addFreePage( page->m_offset+sz, page->m_size-sz );

			delete page;


		}

		assert( m_freePagesOffset.size() == m_freePagesSize.size() );
	}
	else
	{
		loc = m_next;

		/// update next location
		m_next += sz;
	}

	return loc;
}

template< typename D >
void StreamIndexedIO::Index::deallocate( D* n )
{
	assert(n);
	assert(n->nodeType() == NodeBase::Data || n->nodeType() == NodeBase::SmallData );

	addFreePage( n->m_offset, n->m_size );
}

void StreamIndexedIO::Index::addFreePage(  Imf::Int64 offset, Imf::Int64 sz )
{
	assert( m_freePagesOffset.size() == m_freePagesSize.size() );

	if (sz == 0)
	{
		return;
	}

	assert( m_freePagesOffset.find( offset ) == m_freePagesOffset.end() );

	/// Is there a free page immediately after this?
	FreePagesOffsetMap::iterator it = m_freePagesOffset.find( offset + sz );

	bool merged = false;

	if (it != m_freePagesOffset.end())
	{
		/// The next page in the free page list is contiguos with the one we'd like to add, so remove it and
		/// add a new one which represents both.
		FreePage *nextPage = it->second;

		assert( nextPage );

		/// Remove ne
		m_freePagesSize.erase( nextPage->m_sizeIterator );
		m_freePagesOffset.erase( nextPage->m_offsetIterator );

		nextPage->m_offset = offset;
		nextPage->m_size += sz;

		nextPage->m_offsetIterator = m_freePagesOffset.insert( FreePagesOffsetMap::value_type( nextPage->m_offset, nextPage ) ).first;
		nextPage->m_sizeIterator = m_freePagesSize.insert( FreePagesSizeMap::value_type( nextPage->m_size, nextPage ) );

		merged = true;
	}
	else if (offset > 0)
	{
		/// Is there a free page immediately before this?

		it = m_freePagesOffset.lower_bound( offset  );
		if (it != m_freePagesOffset.end())
		{
			assert( m_freePagesOffset.find(it->first) != m_freePagesOffset.end() );
			assert( it->first != offset );
			assert( it->second->m_offset  != offset );

			/// For some reason lower_bound doesn't give quite what's expected, we actually need the previous element in the iteration,
			/// if it exists
			if (it != m_freePagesOffset.begin())
			{
				it--;
				assert (it->first < offset);

				FreePage* previousPage = it->second;

				/// Now we know exactly where the previous page is, see if it's contiguous with the one we're
				/// wanting to add
				if (previousPage->m_offset + previousPage->m_size == offset)
				{
					/// Pages are contiguos, so simply expand the previous page
					/// making sure its position in the sizeMap is updated accordingly.
					m_freePagesSize.erase( previousPage->m_sizeIterator );

					previousPage->m_size += sz;

					previousPage->m_sizeIterator = m_freePagesSize.insert( FreePagesSizeMap::value_type( previousPage->m_size, previousPage ) );
					merged = true;
				}
			}
		}
	}

	if (!merged)
	{
		/// Is this page the last one? If so, just bring back the next-empty-slot offset "file pointer".
		if ( offset + sz == m_next)
		{
			m_next -= sz;
			return;
		}
		else
		{
			/// Simply add new page
			FreePage *page = new FreePage( offset, sz );
			assert(page);

			page->m_offsetIterator = m_freePagesOffset.insert( FreePagesOffsetMap::value_type( offset, page ) ).first;
			page->m_sizeIterator = m_freePagesSize.insert( FreePagesSizeMap::value_type( sz, page ) );
		}
	}
	else
	{
		/// We did a merge - so there should be least one page here!
		assert( m_freePagesOffset.size() );

		/// We might have joined up a string of pages which mean the end of the file is completely blank. If this is case
		/// delete the last page, and bring back the next-empty-slot offset "file pointer".
		FreePagesOffsetMap::iterator last = m_freePagesOffset.end();

		last--;

		FreePage *lastPage = last->second;
		assert(lastPage);

		if (lastPage->m_offset + lastPage->m_size == m_next)
		{
			m_freePagesOffset.erase( lastPage->m_offsetIterator );
			m_freePagesSize.erase( lastPage->m_sizeIterator );

			m_next = lastPage->m_offset;

			delete lastPage;
		}

	}

	assert( m_freePagesOffset.size() == m_freePagesSize.size() );
}

Imf::Int64 StreamIndexedIO::Index::writeUniqueData( const char *data, size_t size, bool prefixSize )
{
	m_hasChanged = true;

	/// Find next writable location
	Imf::Int64 loc;

	// compute hash for the data
	MurmurHash hash;
	hash.append( data, size );

	if ( size >= UINT32_MAX )
	{
		throw IOException( "StreamIndexedIO: Data size too long!" );
	}
	uint32_t clampedSize = size;
	size_t totalSize = size;

	if ( prefixSize )
	{
		totalSize += sizeof( clampedSize );
	}

	// see if it's already stored by another node..
	std::pair< HashToDataMap::iterator,bool > ret = m_hashToDataMap.insert( HashToDataMap::value_type( std::pair< MurmurHash,Imf::Int64>(hash,totalSize), 0 ) );
	if ( !ret.second )
	{
		// we already saved this data, so we dont save any additional data
		return ret.first->second;
	}

	/// New data, find next writable location.
	loc = allocate( totalSize );
	ret.first->second = loc;

	/// Seek 'write' pointer to writable location
	m_stream->seekp( loc, std::ios::beg );

	if ( prefixSize )
	{
		writeLittleEndian( *m_stream, clampedSize );
	}

	/// Write data
	m_stream->write( data, size );

	return loc;
}

StreamIndexedIO::Index::WriteInfo StreamIndexedIO::Index::writeUniqueDataCompressed( const char *data, size_t size, bool prefixSize )
{
	WriteInfo writeInfo;

	std::vector<char> compressedBuffer;
	size_t numBlocks = 0;

	if ( m_compressionLevel )
	{
		numBlocks = compress( data, size, compressedBuffer, m_compressionLevel, m_compressor, m_compressionThreadCount, m_maxCompressedBlockSize );
	}

	//! if compression fails or produces a buffer larger than the original
	//! write the original source data uncompressed
	if( numBlocks && !compressedBuffer.empty() && ( compressedBuffer.size() < size ) )
	{
		writeInfo.offset = writeUniqueData( compressedBuffer.data(), compressedBuffer.size(), prefixSize );
		writeInfo.size = compressedBuffer.size();
		writeInfo.numCompressedBlocks = numBlocks;
	}
	else
	{
		writeInfo.offset = writeUniqueData( data, size, prefixSize );
		writeInfo.size = size;
		writeInfo.numCompressedBlocks = 0;
	}


	return writeInfo;
}

void StreamIndexedIO::Index::deallocateWalk( NodeBase* n )
{
	assert(n);

	// store the pointer for future deallocation
	m_removedNodes.push_back( n );

	if ( n->nodeType() == NodeBase::Directory )
	{
		DirectoryNode *nn = static_cast< DirectoryNode *>(n);

		for (DirectoryNode::ChildMap::const_iterator it = nn->children().begin(); it != nn->children().end(); ++it)
		{
			deallocateWalk( *it );
		}

		nn->children().clear();
	}
	else
	{
		// We don't deallocate data node blocks because they could be referred by other nodes.
		// As a result, editing files will usually increase file size.
	}

}

void StreamIndexedIO::Index::commitNodeToSubIndex( DirectoryNode *n )
{
	if (!n)
	{
		return;
	}

	if ( n->subindex() == DirectoryNode::NoSubIndex )
	{
		MemoryStreamSink sink;
		io::filtering_ostream outIndexStream;
		outIndexStream.push( sink );
		assert( outIndexStream.is_complete() );

		writeNodeChildren( n, outIndexStream );

		outIndexStream.pop();

		// compress the output stream using blosc before writing
		char* indexData = nullptr;
		std::streamsize indexDataSize;

		sink.get(indexData, indexDataSize);

		std::vector<char> compressedIndex;
		compress( indexData, indexDataSize, compressedIndex, indexCompressionLevel, indexCompressor, 1, BLOSC_MAX_BUFFERSIZE, 0);

		uint32_t subindexSize = compressedIndex.size();

		// tell the Directory node that it's contents have been written as a subindex
		Imf::Int64 offset = writeUniqueData( &compressedIndex[0], subindexSize, true );
		n->setSubIndexOffset( offset );
	}
}

void StreamIndexedIO::Index::readNodeFromSubIndex( DirectoryNode *n )
{
	/// guarantees thread safe access to the file and also to the m_subindex variable
	StreamFile::MutexLock lock( m_stream->mutex() );

	if ( n->subindex() == DirectoryNode::LoadedSubIndex )
	{
		return;
	}

	m_stream->seekg( n->offset(), std::ios::beg );

	uint32_t subindexSize = 0;
	readLittleEndian( *m_stream, subindexSize );

	char *data = m_stream->ioBuffer(subindexSize);
	m_stream->read( data, subindexSize );

	io::filtering_istream indexInStream;

	if (m_version >= 7)
	{
		std::vector<char> decompressedIndex;
		decompress( data, subindexSize, decompressedIndex, 1 );

		MemoryStreamSource source( &decompressedIndex[0], decompressedIndex.size(), false );
		indexInStream.push( source );
		assert( indexInStream.is_complete() );

		uint32_t nodeCount = 0;

		readLittleEndian( indexInStream, nodeCount );

		for( uint32_t i = 0; i < nodeCount; i++ )
		{
			NodeBase *child = m_version >= 6 ? readNode( indexInStream ) : readNodeV5( indexInStream );
			n->registerChild( child );
		}
	}
	else
	{
		MemoryStreamSource source( data, subindexSize, false );

		indexInStream.push( io::gzip_decompressor() );
		indexInStream.push( source );
		assert( indexInStream.is_complete() );

		uint32_t nodeCount = 0;

		readLittleEndian( indexInStream, nodeCount );

		for( uint32_t i = 0; i < nodeCount; i++ )
		{
			NodeBase *child = m_version >= 6 ? readNode( indexInStream ) : readNodeV5( indexInStream );
			n->registerChild( child );
		}
	}

	/// make sure the children is sorted to avoid non-thread safe sorting happening later...
	n->sortChildren();

	/// mark the node as loaded from subindex
	n->recoveredSubIndex();
}

void StreamIndexedIO::Index::lockDirectory( MutexLock &lock, const DirectoryNode *n, bool writeAccess ) const
{
	if ( n->subindexChildren() )
	{
		// choose one of the mutexes from the pool (in a deterministic way)
		size_t v = (size_t)n / sizeof(DirectoryNode*);
		unsigned int m = ( (v + 1) / 3 ) % MAX_MUTEXES;

		lock.acquire( m_mutexes[ m ], writeAccess );
	}
}

///////////////////////////////////////////////
//
// StreamIndexedIO::Index (end)
//
///////////////////////////////////////////////

///////////////////////////////////////////////
//
// StreamIndexedIO::StreamFile (begin)
//
///////////////////////////////////////////////

StreamIndexedIO::StreamFile::StreamFile( IndexedIO::OpenMode mode ) : m_openmode( mode ), m_stream( nullptr ), m_ioBufferLen( 0 ), m_ioBuffer( nullptr )
{
	IndexedIO::validateOpenMode(m_openmode);
}

StreamIndexedIO::StreamFile::~StreamFile()
{
	if ( m_stream )
	{
		delete m_stream;
	}
	if ( m_ioBuffer )
	{
		delete [] m_ioBuffer;
	}
}

IndexedIO::OpenMode StreamIndexedIO::StreamFile::openMode() const
{
	return m_openmode;
}

void StreamIndexedIO::StreamFile::setInput( std::iostream *stream, bool emptyFile, const std::string& fileName )
{
	m_stream = stream;
	if ( m_openmode & IndexedIO::Append && emptyFile )
	{
		m_openmode = (m_openmode ^ IndexedIO::Append) | IndexedIO::Write;
	}

	if ( fileName != "" && getenv("IECORE_OFFSETREAD_DISABLED") == nullptr )
	{
		m_platformReader = PlatformReader::create( fileName );
	}
}

char *StreamIndexedIO::StreamFile::ioBuffer( unsigned long size )
{
	if ( !m_ioBuffer )
	{
		m_ioBuffer = new char[size];
		m_ioBufferLen = size;
	}
	else if ( size > m_ioBufferLen )
	{
		delete [] m_ioBuffer;
		m_ioBuffer = new char[size];
		m_ioBufferLen = size;
	}
	return m_ioBuffer;
}

StreamIndexedIO::StreamFile::Mutex & StreamIndexedIO::StreamFile::mutex()
{
	return m_mutex;
}

void StreamIndexedIO::StreamFile::flush( size_t endPosition )
{
	assert( m_stream );
	m_stream->flush();
}

bool StreamIndexedIO::StreamFile::canRead( std::iostream &f )
{
	f.seekg( 0, std::ios::end );
	Imf::Int64 end = f.tellg();

	f.seekg( end-1*sizeof(Imf::Int64), std::ios::beg );

	Imf::Int64 magicNumber;
	readLittleEndian( f,magicNumber );

	if ( magicNumber == g_versionedMagicNumber || magicNumber == g_unversionedMagicNumber )
	{
		return true;
	}
	else
	{
		return false;
	}
}

void StreamIndexedIO::StreamFile::read( char *buffer, size_t size, size_t pos )
{
	if ( !m_platformReader || ( !m_platformReader->read( buffer, size, pos ) ) )
	{
		MutexLock lock( m_mutex );
		seekg( pos, std::ios::beg );
		read( buffer, size );
	}
}

void StreamIndexedIO::StreamFile::seekg( size_t pos, std::ios_base::seekdir dir )
{
	m_stream->seekg( pos, dir );
}

void StreamIndexedIO::StreamFile::seekp( size_t pos, std::ios_base::seekdir dir )
{
	/// Seek 'write' pointer to writable location
	m_stream->seekp( pos, dir );

	/// Clear error flags because problem on GCC 3.3.4:
	/// When the file is a std::stringstream then the first seekp(0) will fail and inhibit following operations on the file.
	m_stream->clear();
}

Imf::Int64 StreamIndexedIO::StreamFile::tellg()
{
	return m_stream->tellg();
}

Imf::Int64 StreamIndexedIO::StreamFile::tellp()
{
	return m_stream->tellp();
}

void StreamIndexedIO::StreamFile::read( char *buffer, size_t size )
{
	m_stream->read( buffer, size );
}

void StreamIndexedIO::StreamFile::write( const char *buffer, size_t size )
{
	m_stream->write( buffer, size );
}

///////////////////////////////////////////////
//
// StreamIndexedIO::StreamFile (end)
//
///////////////////////////////////////////////

///////////////////////////////////////////////
//
// StreamIndexedIO (begin)
//
///////////////////////////////////////////////

StreamIndexedIO::StreamIndexedIO() : m_node(nullptr)
{
}

StreamIndexedIO::StreamIndexedIO( StreamIndexedIO::Node &node )
{
	m_node = &node;
}

void StreamIndexedIO::open( StreamFilePtr file, const IndexedIO::EntryIDList &root, const CompoundData *options )
{
	IndexPtr newIndex = new Index( file, options );
	newIndex->openStream();
	m_node = new StreamIndexedIO::Node( newIndex.get(), newIndex->root() );
	setRoot( root );

	// \todo Currently in Append mode, the nodes lazily loaded will not be editable.
	// In order to fully support it, we should probably read all indexes in memory,
	// deallocate their data blocks, mark Index as changed and force saving all of
	// the nodes in the main index, or commit them backwardly.
}

StreamIndexedIO::~StreamIndexedIO()
{
	delete m_node;
	// \todo Consider a mechanism for deallocating sub-indexes
}

void StreamIndexedIO::setRoot( const IndexedIO::EntryIDList &root )
{
	IndexedIO::EntryIDList::const_iterator t = root.begin();
	for ( ; t != root.end(); t++ )
	{
		DirectoryNode* childNode = m_node->directoryChild( *t );
		if ( !childNode )
		{
			break;
		}
		m_node->m_node = childNode;
	}
	bool found = ( t == root.end() );

	if (openMode() & IndexedIO::Read)
	{
		if (!found)
		{
			throw IOException( "StreamIndexedIO: Cannot find entry '" + (*t).value() + "'" );
		}
	}
	else
	{
		if ( (openMode() & IndexedIO::Write) && found )
		{
			// we remove the current contents if we open in Write mode
			removeAll();
		}
		else
		{
			for ( ; t != root.end(); t++ )
			{
				DirectoryNode *childNode = m_node->addChild( *t );
				if ( !childNode )
				{
					throw IOException( "StreamIndexedIO: Cannot create entry '" + (*t).value() + "'" );
				}
				m_node->m_node = childNode;
			}
		}
	}
	assert( m_node );
}

void StreamIndexedIO::flush()
{
	m_node->m_idx->flush();
}

StreamIndexedIO::StreamFile& StreamIndexedIO::streamFile() const
{
	return m_node->m_idx->streamFile();
}

IndexedIO::OpenMode StreamIndexedIO::openMode() const
{
	return streamFile().openMode();
}

CompoundDataPtr StreamIndexedIO::metadata() const
{
	return m_node->m_idx->metadata();
}

const IndexedIO::EntryID &StreamIndexedIO::currentEntryId() const
{
	return m_node->name();
}

void StreamIndexedIO::path( IndexedIO::EntryIDList &result ) const
{
	result.clear();
	m_node->m_node->path(result);
}

void StreamIndexedIO::entryIds( IndexedIO::EntryIDList &names ) const
{
	m_node->childNames( names );
}

void StreamIndexedIO::entryIds( IndexedIO::EntryIDList &names, IndexedIO::EntryType type ) const
{
	m_node->childNames( names, type );
}

bool StreamIndexedIO::hasEntry( const IndexedIO::EntryID &name ) const
{
	assert( m_node );
	return m_node->hasChild( name );
}

IndexedIOPtr StreamIndexedIO::subdirectory( const IndexedIO::EntryID &name, IndexedIO::MissingBehaviour missingBehaviour )
{
	assert( m_node );
	DirectoryNode *childNode = m_node->directoryChild( name );
	if ( !childNode )
	{
		if ( missingBehaviour == IndexedIO::CreateIfMissing )
		{
			writable( name );
			childNode = m_node->addChild( name );
			if ( !childNode )
			{
				throw IOException( "StreamIndexedIO: Could not insert child '" + name.value() + "'" );
			}
		}
		else if ( missingBehaviour == IndexedIO::NullIfMissing )
		{
			return nullptr;
		}
		else
		{
			throw IOException( "StreamIndexedIO: Could not find child '" + name.value() + "'" );
		}
	}
	StreamIndexedIO::Node *newNode = new StreamIndexedIO::Node( m_node->m_idx.get(), childNode );
	return duplicate(*newNode);
}

ConstIndexedIOPtr StreamIndexedIO::subdirectory( const IndexedIO::EntryID &name, IndexedIO::MissingBehaviour missingBehaviour ) const
{
	readable(name);
	assert( m_node );
	DirectoryNode *childNode = m_node->directoryChild( name );
	if ( !childNode )
	{
		if ( missingBehaviour == IndexedIO::NullIfMissing )
		{
			return nullptr;
		}
		if ( missingBehaviour == IndexedIO::CreateIfMissing )
		{
			throw IOException( "StreamIndexedIO: No write access!" );
		}
		throw IOException( "StreamIndexedIO: Could not find child '" + name.value() + "'" );
	}
	StreamIndexedIO::Node *newNode = new StreamIndexedIO::Node( m_node->m_idx.get(), childNode );
	return duplicate(*newNode);
}

IndexedIOPtr StreamIndexedIO::createSubdirectory( const IndexedIO::EntryID &name )
{
	assert( m_node );
	if ( m_node->hasChild(name) )
	{
		throw IOException( "Child '" + name.value() + "' already exists!" );
	}
	writable( name );
	DirectoryNode *childNode = m_node->addChild( name );
	if ( !childNode )
	{
		throw IOException( "StreamIndexedIO: Could not insert child '" + name.value() + "'" );
	}
	StreamIndexedIO::Node *newNode = new StreamIndexedIO::Node( m_node->m_idx.get(), childNode );
	return duplicate(*newNode);
}

void StreamIndexedIO::remove( const IndexedIO::EntryID &name )
{
	assert( m_node );
	remove(name, true);
}

void StreamIndexedIO::removeAll( )
{
	assert( m_node );

	if ( m_node->m_node->subindex() )
	{
		throw Exception( "Cannot modify the file at current location! It was already committed to the file." );
	}

	IndexedIO::EntryIDList names;
	m_node->childNames( names );
	for ( IndexedIO::EntryIDList::const_iterator it = names.begin(); it != names.end(); it++ )
	{
		m_node->removeChild( *it );
	}
}

void StreamIndexedIO::remove( const IndexedIO::EntryID &name, bool throwIfNonExistent )
{
	assert( m_node );
	writable(name);

	if ( m_node->m_node->subindex() )
	{
		throw Exception( "Cannot modify the file at current location! It was already committed to the file." );
	}

	m_node->removeChild( name, throwIfNonExistent );
}

IndexedIO::Entry StreamIndexedIO::entry(const IndexedIO::EntryID &name) const
{
	assert( m_node );
	readable(name);

	Index::MutexLock lock;
	m_node->m_idx->lockDirectory( lock, m_node->m_node );

	DirectoryNode::ChildMap::iterator it = m_node->m_node->findChild( name );
	if ( it == m_node->m_node->children().end() )
	{
		throw IOException( "StreamIndexedIO::entry: Entry not found '" + name.value() + "'" );
	}

	NodeBase *node = *it;

	switch( node->nodeType() )
	{
		case NodeBase::Data:
			{
				DataNode *dn = static_cast< DataNode * >(node);
				return IndexedIO::Entry( dn->name(), IndexedIO::File, dn->dataType(), dn->arrayLength() );
			}

		case NodeBase::SmallData:
			{
				SmallDataNode *dn = static_cast< SmallDataNode * >(node);
				return IndexedIO::Entry( dn->name(), IndexedIO::File, dn->dataType(), dn->arrayLength() );
			}

		case NodeBase::Directory:
		case NodeBase::SubIndex:
			return IndexedIO::Entry( node->name(), IndexedIO::Directory, IndexedIO::Invalid, 0 );

		default:
			throw Exception("Invalid node type!");
	}
}

IndexedIOPtr StreamIndexedIO::parentDirectory()
{
	assert( m_node );
	DirectoryNode *parentNode = m_node->m_node->parent();
	if ( !parentNode )
	{
		return nullptr;
	}
	StreamIndexedIO::Node *newNode = new StreamIndexedIO::Node( m_node->m_idx.get(), parentNode );
	return duplicate(*newNode);
}

ConstIndexedIOPtr StreamIndexedIO::parentDirectory() const
{
	assert( m_node );
	DirectoryNode* parentNode = m_node->m_node->parent();
	if ( !parentNode )
	{
		return nullptr;
	}
	StreamIndexedIO::Node *newNode = new StreamIndexedIO::Node( m_node->m_idx.get(), parentNode );
	return duplicate(*newNode);
}

IndexedIOPtr StreamIndexedIO::directory( const IndexedIO::EntryIDList &path, IndexedIO::MissingBehaviour missingBehaviour )
{
	// from the root go to the path
	StreamIndexedIO::Node *newNode = new StreamIndexedIO::Node( m_node->m_idx.get(), m_node->m_idx->root() );

	for ( IndexedIO::EntryIDList::const_iterator pIt = path.begin(); pIt != path.end(); pIt++ )
	{
		const IndexedIO::EntryID &name = *pIt;

		DirectoryNode* childNode = newNode->directoryChild( name );
		if ( !childNode )
		{
			if ( missingBehaviour == IndexedIO::CreateIfMissing )
			{
				writable( name );
				childNode = newNode->addChild( name );
				if ( !childNode )
				{
					throw IOException( "StreamIndexedIO: Could not insert child '" + name.value() + "'" );
				}
			}
			else if ( missingBehaviour == IndexedIO::NullIfMissing )
			{
				return nullptr;
			}
			else
			{
				throw IOException( "StreamIndexedIO: Could not find child '" + name.value() + "'" );
			}
		}
		newNode->m_node = childNode;
	}
	return duplicate(*newNode);
}

ConstIndexedIOPtr StreamIndexedIO::directory( const IndexedIO::EntryIDList &path, IndexedIO::MissingBehaviour missingBehaviour ) const
{
	return const_cast< StreamIndexedIO * >(this)->directory( path, missingBehaviour == IndexedIO::CreateIfMissing ? IndexedIO::ThrowIfMissing : missingBehaviour );
}

void StreamIndexedIO::commit()
{
	m_node->m_idx->commitNodeToSubIndex( m_node->m_node );
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const InternedString *x, unsigned long arrayLength)
{
	writable(name);
	remove(name, false);

	Imf::Int64 *ids = new Imf::Int64[arrayLength];
	const Imf::Int64 *constIds = ids;
	unsigned long size = IndexedIO::DataSizeTraits<Imf::Int64 *>::size(constIds, arrayLength);
	IndexedIO::DataType dataType = IndexedIO::InternedStringArray;

	char *data = streamFile().ioBuffer(size);
	assert(data);

	Index *index = m_node->m_idx.get();

	StringCache &stringCache = index->stringCache();

	for ( unsigned long i = 0; i < arrayLength; i++ )
	{
		ids[i] = stringCache.find( x[i], false /* create entry if missing */ );
	}

	IndexedIO::DataFlattenTraits<Imf::Int64*>::flatten(constIds, arrayLength, data);

	Index::WriteInfo info = index->writeUniqueDataCompressed( data, size );
	m_node->addDataChild( name, dataType, arrayLength, info.offset, info.size, size, info.numCompressedBlocks );

	delete [] ids;
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, InternedString *&x, unsigned long arrayLength) const
{
	assert( m_node );
	readable( name );

	StreamIndexedIO::Node::Info nodeInfo;

	if( !m_node->dataChildInfo( name, nodeInfo ) )
	{
		throw IOException( "StreamIndexedIO::read : Data entry not found '" + name.value() + "'" );
	}

	Imf::Int64 *ids = new Imf::Int64[arrayLength];

	size_t arraySizeInBytes = sizeof(Imf::Int64) * arrayLength;
	if ( arraySizeInBytes != nodeInfo.decompressedSize )
	{
		throw IECore::IOException(
			boost::str(
				boost::format( "StreamIndexedIO::rawRead - array size (%1%) does not match block size (%2%) " ) %
					arraySizeInBytes %
					nodeInfo.decompressedSize
			)
		);
	}

	StreamIndexedIO::StreamFile &f = streamFile();
	Reader reader( f, nodeInfo, m_node->m_idx->decompressionThreadCount(), reinterpret_cast<char *>( ids ) );

	const StringCache &stringCache = m_node->m_idx->stringCache();
	if (!x)
	{
		x = new InternedString[arrayLength];
	}

	for ( unsigned long i = 0; i < arrayLength; i++ )
	{
		x[i] = stringCache.findById( ids[i] );
	}
	delete [] ids;
}

template<typename T>
void StreamIndexedIO::write(const IndexedIO::EntryID &name, const T *x, unsigned long arrayLength)
{
	writable(name);
	remove(name, false);

	unsigned long size = IndexedIO::DataSizeTraits<T*>::size(x, arrayLength);
	IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T*>::type();

	char *data = streamFile().ioBuffer(size);
	assert(data);
	IndexedIO::DataFlattenTraits<T*>::flatten(x, arrayLength, data);

	Index::WriteInfo info = m_node->m_idx->writeUniqueDataCompressed( data, size );
	m_node->addDataChild( name, dataType, arrayLength, info.offset, info.size, size, info.numCompressedBlocks );
}

template<typename T>
void StreamIndexedIO::rawWrite(const IndexedIO::EntryID &name, const T *x, unsigned long arrayLength)
{
	writable(name);
	remove(name, false);

	unsigned long size = IndexedIO::DataSizeTraits<T*>::size(x, arrayLength);
	IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T*>::type();

	Index::WriteInfo info = m_node->m_idx->writeUniqueDataCompressed( (char *) x, size );
	m_node->addDataChild( name, dataType, arrayLength, info.offset, info.size, size, info.numCompressedBlocks );
}

template<typename T>
void StreamIndexedIO::write(const IndexedIO::EntryID &name, const T &x)
{
	writable(name);
	remove(name, false);

	unsigned long size = IndexedIO::DataSizeTraits<T>::size(x);
	IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T>::type();

	char *data = streamFile().ioBuffer(size);
	assert(data);
	IndexedIO::DataFlattenTraits<T>::flatten(x, data);

	Index::WriteInfo info = m_node->m_idx->writeUniqueDataCompressed( data, size );
	m_node->addDataChild( name, dataType, 0, info.offset, info.size, size, info.numCompressedBlocks );
}

template<typename T>
void StreamIndexedIO::rawWrite(const IndexedIO::EntryID &name, const T &x)
{
	writable(name);
	remove(name, false);

	unsigned long size = IndexedIO::DataSizeTraits<T>::size(x);
	IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T>::type();

	Index::WriteInfo info = m_node->m_idx->writeUniqueDataCompressed( (char *) &x, size );
	m_node->addDataChild( name, dataType, 0, info.offset, info.size, size, info.numCompressedBlocks );
}

template<typename T>
void StreamIndexedIO::read(const IndexedIO::EntryID &name, T *&x, unsigned long arrayLength) const
{
	assert( m_node );
	readable(name);

	StreamIndexedIO::Node::Info nodeInfo;
	if( !m_node->dataChildInfo( name, nodeInfo ) )
	{
		throw IOException( "StreamIndexedIO::read: Data entry not found '" + name.value() + "'" );
	}

	Reader reader( streamFile(), nodeInfo, m_node->m_idx->decompressionThreadCount() );
	IndexedIO::DataFlattenTraits<T *>::unflatten( reader.data(), x, arrayLength );
}

template<typename T>
void StreamIndexedIO::rawRead(const IndexedIO::EntryID &name, T *&x, unsigned long arrayLength) const
{
	assert( m_node );
	readable(name);

	StreamIndexedIO::Node::Info nodeInfo;
	if( !m_node->dataChildInfo( name, nodeInfo ) )
	{
		throw IOException( "StreamIndexedIO::rawRead: Data entry not found '" + name.value() + "'" );
	}

	if (!x)
	{
		x = new T[arrayLength];
	}

	size_t arraySizeInBytes = sizeof(T) * arrayLength;
	if ( arraySizeInBytes != nodeInfo.decompressedSize )
	{
		throw IECore::IOException(
			boost::str(
				boost::format( "StreamIndexedIO::rawRead - array size (%1%) does not match block size (%2%) " ) %
					arraySizeInBytes %
					nodeInfo.decompressedSize
			)
		);
	}

	Reader reader( streamFile(), nodeInfo, m_node->m_idx->decompressionThreadCount(), reinterpret_cast<char *>( x ) );
}

template<typename T>
void StreamIndexedIO::read(const IndexedIO::EntryID &name, T &x) const
{
	assert( m_node );
	readable(name);

	StreamIndexedIO::Node::Info nodeInfo;

	if( !m_node->dataChildInfo( name, nodeInfo ) )
	{
		throw IOException( "StreamIndexedIO::read Data entry not found '" + name.value() + "'" );
	}

	Reader reader( streamFile(), nodeInfo, m_node->m_idx->decompressionThreadCount() );
	IndexedIO::DataFlattenTraits<T>::unflatten( reader.data(), x );
}

template<typename T>
void StreamIndexedIO::rawRead(const IndexedIO::EntryID &name, T &x) const
{
	assert( m_node );
	readable(name);

	StreamIndexedIO::Node::Info nodeInfo;
	if( !m_node->dataChildInfo( name, nodeInfo ) )
	{
		throw IOException( "StreamIndexedIO::rawRead: Data entry not found '" + name.value() + "'" );
	}

	if( nodeInfo.size != nodeInfo.decompressedSize )
	{
		throw Exception( "Simple type can't be compressed" );
	}

	streamFile().read( (char *) &x, nodeInfo.size, nodeInfo.offset );
}

#ifdef IE_CORE_LITTLE_ENDIAN
#define READ	rawRead
#define WRITE	rawWrite
#else
#define READ	read
#define WRITE	write
#endif

// Write

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const float *x, unsigned long arrayLength)
{
	WRITE<float>(name, x, arrayLength);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const double *x, unsigned long arrayLength)
{
	WRITE<double>(name, x, arrayLength);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const half *x, unsigned long arrayLength)
{
	WRITE<half>(name, x, arrayLength);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const int *x, unsigned long arrayLength)
{
	WRITE<int>(name, x, arrayLength);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const int64_t *x, unsigned long arrayLength)
{
	WRITE<int64_t>(name, x, arrayLength);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const uint64_t *x, unsigned long arrayLength)
{
	WRITE<uint64_t>(name, x, arrayLength);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const unsigned int *x, unsigned long arrayLength)
{
	WRITE<unsigned int>(name, x, arrayLength);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const char *x, unsigned long arrayLength)
{
	WRITE<char>(name, x, arrayLength);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const unsigned char *x, unsigned long arrayLength)
{
	WRITE<unsigned char>(name, x, arrayLength);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const std::string *x, unsigned long arrayLength)
{
	write<std::string>(name, x, arrayLength);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const short *x, unsigned long arrayLength)
{
	WRITE<short>(name, x, arrayLength);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const unsigned short *x, unsigned long arrayLength)
{
	WRITE<unsigned short>(name, x, arrayLength);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const float &x)
{
	WRITE<float>(name, x);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const double &x)
{
	WRITE<double>(name, x);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const half &x)
{
	WRITE<half>(name, x);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const int &x)
{
	WRITE<int>(name, x);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const int64_t &x)
{
	WRITE<int64_t>(name, x);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const uint64_t &x)
{
	WRITE<uint64_t>(name, x);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const std::string &x)
{
	write<std::string>(name, x);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const unsigned int &x)
{
	WRITE<unsigned int>(name, x);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const char &x)
{
	WRITE<char>(name, x);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const unsigned char &x)
{
	WRITE<unsigned char>(name, x);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const short &x)
{
	WRITE<short>(name, x);
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const unsigned short &x)
{
	WRITE<unsigned short>(name, x);
}
// Read

void StreamIndexedIO::read(const IndexedIO::EntryID &name, float *&x, unsigned long arrayLength) const
{
	READ<float>(name, x, arrayLength);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, double *&x, unsigned long arrayLength) const
{
	READ<double>(name, x, arrayLength);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, half *&x, unsigned long arrayLength) const
{
	READ<half>(name, x, arrayLength);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, int *&x, unsigned long arrayLength) const
{
	READ<int>(name, x, arrayLength);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, int64_t *&x, unsigned long arrayLength) const
{
	READ<int64_t>(name, x, arrayLength);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, uint64_t *&x, unsigned long arrayLength) const
{
	READ<uint64_t>(name, x, arrayLength);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, unsigned int *&x, unsigned long arrayLength) const
{
	READ<unsigned int>(name, x, arrayLength);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, char *&x, unsigned long arrayLength) const
{
	READ<char>(name, x, arrayLength);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, unsigned char *&x, unsigned long arrayLength) const
{
	READ<unsigned char>(name, x, arrayLength);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, std::string *&x, unsigned long arrayLength) const
{
	read<std::string>(name, x, arrayLength);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, short *&x, unsigned long arrayLength) const
{
	READ<short>(name, x, arrayLength);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, unsigned short *&x, unsigned long arrayLength) const
{
	READ<unsigned short>(name, x, arrayLength);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, float &x) const
{
	READ<float>(name, x);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, double &x) const
{
	READ<double>(name, x);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, half &x) const
{
	READ<half>(name, x);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, int &x) const
{
	READ<int>(name, x);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, int64_t &x) const
{
	READ<int64_t>(name, x);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, uint64_t &x) const
{
	READ<uint64_t>(name, x);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, std::string &x) const
{
	read<std::string>(name, x);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, unsigned int &x) const
{
	READ<unsigned int>(name, x);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, char &x) const
{
	READ<char>(name, x);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, unsigned char &x) const
{
	READ<unsigned char>(name, x);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, short &x) const
{
	READ<short>(name, x);
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, unsigned short &x) const
{
	READ<unsigned short>(name, x);
}
