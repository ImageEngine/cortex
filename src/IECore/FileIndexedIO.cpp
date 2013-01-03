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

#include <algorithm>
#include <list>
#include <iostream>
#include <cassert>
#include <map>
#include <set>

#include "boost/tokenizer.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/optional.hpp"
#include "boost/format.hpp"
#include "boost/iostreams/device/file.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/iostreams/stream.hpp"
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/detail/endian.hpp"

#include "IECore/ByteOrder.h"
#include "IECore/MemoryStream.h"
#include "IECore/MessageHandler.h"
#include "IECore/FileIndexedIO.h"
#include "IECore/VectorTypedData.h"

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
namespace io = boost::iostreams;

typedef io::filtering_stream< io::bidirectional_seekable > FilteredStream;

template<typename T>
void writeLittleEndian( std::ostream &f, T n )
{
	const T nl = asLittleEndian<>(n);
	f.write( (const char*) &nl, sizeof(T) );
}

template<typename T>
void readLittleEndian( std::istream &f, T &n )
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

/// \todo We could add a minimum length for strings, below which we don't bother caching them?
class StringCache
{
	public:

		StringCache() : m_prevId(0), m_ioBuffer(0), m_ioBufferLen(0)
		{
			m_idToStringMap.reserve(100);
		}

		StringCache( std::istream &f ) : m_prevId(0), m_ioBuffer(0), m_ioBufferLen(0)
		{
			Imf::Int64 sz;
			readLittleEndian(f, sz);

			m_idToStringMap.reserve(sz + 100);

			for (Imf::Int64 i = 0; i < sz; ++i)
			{
				const char *s = read(f);

				Imf::Int64 id;
				readLittleEndian<Imf::Int64>( f, id );
				assert( id < sz );
				m_prevId = std::max( id, m_prevId );

				m_stringToIdMap[s] = id;
				if ( id >= m_idToStringMap.size() )
				{
					m_idToStringMap.resize(id+1, (const char *)"");
				}
				m_idToStringMap[id] = s;
			}
		}

		void write( std::ostream &f ) const
		{
			Imf::Int64 sz = m_stringToIdMap.size();
			writeLittleEndian<Imf::Int64>(f, sz );

			for (StringToIdMap::const_iterator it = m_stringToIdMap.begin();
				it != m_stringToIdMap.end(); ++it)
			{
				write(f, it->first);

				writeLittleEndian<Imf::Int64>(f, it->second);
			}
		}

		Imf::Int64 find( const IndexedIO::EntryID &s, bool errIfNotFound = true )
		{
			StringToIdMap::const_iterator it = m_stringToIdMap.find( s );

			if ( it == m_stringToIdMap.end() )
			{
				if (errIfNotFound)
				{
					assert(false);
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

			assert( m_stringToIdMap.size() == m_idToStringMap.size() );
		}

		Imf::Int64 size() const
		{
			assert( m_stringToIdMap.size() == m_idToStringMap.size() );
			return m_stringToIdMap.size();
		}

	protected:

		void write( std::ostream &f, const std::string &s ) const
		{
			Imf::Int64 sz = s.size();
			writeLittleEndian<Imf::Int64>( f, sz );

			/// Does not include null terminator
			f.write( s.c_str(), sz * sizeof(char) );
		}

		const char *read( std::istream &f ) const
		{
			Imf::Int64 sz;
			readLittleEndian<Imf::Int64>( f, sz );

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

/// A single node within an index
class FileIndexedIO::Node : public RefCounted
{
	public:

		/// A unique numeric ID for this node
		Imf::Int64 m_id;

		/// The offset in the file to this node
		Imf::Int64 m_offset;

		/// The size of this node's data chunk within the file
		Imf::Int64 m_size;

		/// A brief description of the node
		IndexedIO::Entry m_entry;

		/// A pointer to the parent node in the tree - will be NULL for the root node
		Node* m_parent;

		/// Pointers to this node's children
		typedef std::map< IndexedIO::EntryID, NodePtr> ChildMap;
		ChildMap m_children;

		/// Construct a new Node in the given index with the given numeric id
		Node(Index* idx, Imf::Int64 id);

		virtual ~Node();

		/// Write this node to a stream
		void write( std::ostream &f );

		/// Replace the contents of this node with data read from a stream
		void read( std::istream &f );

		void childNames( IndexedIO::EntryIDList &names ) const;
		void childNames( IndexedIO::EntryIDList &names, IndexedIO::EntryType ) const;

		const IndexedIO::EntryID &name() const;
		void path( IndexedIO::EntryIDList &result ) const;

		// Returns the named child node or NULL if not existent.
		Node* child( const IndexedIO::EntryID &name ) const;

		Node* addChild( const IndexedIO::EntryID & childName );

	protected:

		friend class FileIndexedIO::Index;

		/// registers a child node in this node
		void registerChild( Node* c );

		Index *m_idx;
};

/// A tree to represent nodes in a filesystem, along with their locations in a file.
class FileIndexedIO::Index : public RefCounted
{
	public:

		friend class Node;

		/// Construct an empty index
		Index( bool readOnly );

		static bool canRead( const std::string &path );

		/// Construct an index from reading a file stream.
		Index( FilteredStream &f, bool readOnly );
		virtual ~Index();

		NodePtr m_root;

		/// Returns where or not the index has changed since the last time it was written
		bool hasChanged() const;

		/// Remove a node, and all its subnodes from the index
		void remove( Node* n );

		/// Insert a new entry into the index, returning the node which stores it. If it already exists, returns 0
		Node* insert( Node* parentNode, IndexedIO::Entry e );

		/// Write the index to a file stream
		Imf::Int64 write( std::ostream &f );

		/// Allocate a new chunk of data of the requested size, returning its offset within the file
		Imf::Int64 allocate( Imf::Int64 sz );

		/// Deallocate a node's data from the file.
		void deallocate( Node* n );

		/// Return the total number of nodes in the index.
		Imf::Int64 nodeCount();

	protected:

		static const Imf::Int64 g_unversionedMagicNumber = 0x0B00B1E5;
		static const Imf::Int64 g_versionedMagicNumber = 0xB00B1E50;

		static const Imf::Int64 g_currentVersion = 3;

		Imf::Int64 m_version;

		bool m_hasChanged;

		Imf::Int64 m_offset;
		Imf::Int64 m_next;

		Imf::Int64 m_prevId;

		Imf::Int64 makeId();

		typedef std::vector< Node* > IndexToNodeMap;
		typedef std::map< Node*, unsigned long > NodeToIndexMap;

		bool m_readOnly;
		IndexToNodeMap m_indexToNodeMap;
		NodeToIndexMap m_nodeToIndexMap;

		StringCache m_stringCache;

		struct FreePage;

		typedef std::map< Imf::Int64, FreePage* > FreePagesOffsetMap;
		typedef std::multimap< Imf::Int64, FreePage* > FreePagesSizeMap;

		FreePagesOffsetMap m_freePagesOffset;
		FreePagesSizeMap m_freePagesSize;

		struct FreePage
		{
			FreePage( Imf::Int64 offset, Imf::Int64 sz ) : m_offset(offset), m_size(sz) {}

			Imf::Int64 m_offset;
			Imf::Int64 m_size;

			FreePagesOffsetMap::iterator m_offsetIterator;
			FreePagesSizeMap::iterator m_sizeIterator;
		};

		void addFreePage( Imf::Int64 offset, Imf::Int64 sz );

		void deallocateWalk( Node* n );

		void write( std::ostream &f, Node* n );

		void readNode( std::istream &f );

		Imf::Int64 nodeCount( Node* n );
};

///////////////////////////////////////////////
//
// FileIndexedIO::Node (begin)
//
///////////////////////////////////////////////



FileIndexedIO::Node::Node(Index* idx, Imf::Int64 id) : RefCounted()
{
	m_parent = 0;

	if (id != 0)
	{
		assert(id >= idx->m_indexToNodeMap.size() || !idx->m_indexToNodeMap[id] );
	}

	m_id = id;
	m_idx = idx;

	m_offset = 0;
	m_size = 0;

	m_idx->m_prevId = std::max( m_idx->m_prevId, m_id );
}

FileIndexedIO::Node::~Node()
{
}

void FileIndexedIO::Node::registerChild( Node* c )
{
	if (c->m_parent)
	{
		throw IOException("FileIndexedIO: Node already has parent!");
	}

#ifndef NDEBUG
	/// Make sure we never try to add the same child twice
	ChildMap::const_iterator cit = m_children.find( c->m_entry.id() );
	assert (cit == m_children.end());
#endif

	c->m_parent = this;
	m_children.insert( std::map< IndexedIO::EntryID, NodePtr >::value_type( c->m_entry.id(), c) );
}

void FileIndexedIO::Node::write( std::ostream &f )
{
	char t = m_entry.entryType();
	f.write( &t, sizeof(char) );

	Imf::Int64 id = m_idx->m_stringCache.find( m_entry.id() );
	writeLittleEndian<Imf::Int64>( f, id );
	if ( m_entry.entryType() == IndexedIO::File )
	{
		t = m_entry.dataType();
		f.write( &t, sizeof(char) );

		if ( m_entry.isArray() )
		{
			writeLittleEndian<Imf::Int64>( f, m_entry.arrayLength() );
		}
	}

	writeLittleEndian<Imf::Int64>(f, m_id);

	if (m_parent)
	{
		assert( m_idx->m_nodeToIndexMap.find( m_parent ) != m_idx->m_nodeToIndexMap.end() );
		writeLittleEndian<Imf::Int64>(f, m_idx->m_nodeToIndexMap[m_parent]);
	}
	else
	{
		writeLittleEndian<Imf::Int64>(f, (Imf::Int64)0);
	}

	if ( m_entry.entryType() == IndexedIO::File )
	{
		writeLittleEndian<Imf::Int64>(f, m_offset);
		writeLittleEndian<Imf::Int64>(f, m_size);
	}
}

void FileIndexedIO::Node::read( std::istream &f )
{
	assert( m_idx );

	char t;
	f.read( &t, sizeof(char) );

	std::string id;
	IndexedIO::EntryType entryType = (IndexedIO::EntryType)t;
	IndexedIO::DataType dataType = IndexedIO::Invalid;
	Imf::Int64 arrayLength = 0;

	if (m_idx->m_version >= 1)
	{
		Imf::Int64 stringId;
		readLittleEndian<Imf::Int64>(f, stringId);
		id = m_idx->m_stringCache.findById( stringId );
	}
	else
	{
		Imf::Int64 entrySize;
		readLittleEndian<Imf::Int64>( f, entrySize );
		char *s = new char[entrySize+1];
		f.read( s, entrySize );
		s[entrySize] = '\0';

		id = s;
		delete[] s;
	}

	if ( entryType == IndexedIO::File || m_idx->m_version < 2 )
	{
		f.read( &t, sizeof(char) );
		dataType = (IndexedIO::DataType)t;

		if ( IndexedIO::Entry::isArray( dataType ) || m_idx->m_version < 3 )
		{
			readLittleEndian<Imf::Int64>( f, arrayLength );
		}
	}

	m_entry = IndexedIO::Entry( id, entryType, dataType, static_cast<unsigned long>( arrayLength ) );

	readLittleEndian<Imf::Int64>(f, m_id );

	if ( m_id >= m_idx->m_indexToNodeMap.size() )
	{
		m_idx->m_indexToNodeMap.resize(m_id+1, NULL);
	}
	m_idx->m_indexToNodeMap[m_id] = this;

	// we only need to keep the map node=>index if we intend to save the file...
	if ( !m_idx->m_readOnly )
	{
		m_idx->m_nodeToIndexMap[this] = m_id;
	}

	Imf::Int64 parentId;
	readLittleEndian<Imf::Int64>(f, parentId );

	m_idx->m_prevId = std::max( m_idx->m_prevId, parentId );
	m_idx->m_prevId = std::max( m_idx->m_prevId, m_id );

	if ( parentId >= m_idx->m_indexToNodeMap.size() || !m_idx->m_indexToNodeMap[parentId] )
	{
		throw IOException("FileIndexedIO: parentId not found");
	}

	Node* parent = m_idx->m_indexToNodeMap[parentId] ;
	if (m_id && parent)
	{
		parent->registerChild(this);
	}
	else if (m_id != 0)
	{
		throw IOException("FileIndexedIO: Non-root node has no parent");
	}

	if ( m_entry.entryType() == IndexedIO::File || m_idx->m_version < 2 )
	{
		readLittleEndian<Imf::Int64>(f, m_offset );
		readLittleEndian<Imf::Int64>(f, m_size );
	}
	else
	{
		m_offset = 0;
		m_size = 0;
	}
}

FileIndexedIO::Node* FileIndexedIO::Node::child( const IndexedIO::EntryID &name ) const
{
	ChildMap::const_iterator cit = m_children.find( name );
	if (cit != m_children.end())
	{
		return cit->second.get();
	}
	return 0;
}

FileIndexedIO::Node* FileIndexedIO::Node::addChild( const IndexedIO::EntryID &childName )
{
	Node* child = m_idx->insert( this, IndexedIO::Entry( childName, IndexedIO::Directory, IndexedIO::Invalid, 0 ) );
	return child;
}

void FileIndexedIO::Node::path( IndexedIO::EntryIDList &result ) const
{
	if ( m_parent )
	{
		m_parent->path( result );
		result.push_back( m_entry.id() );
	}
}

const IndexedIO::EntryID &FileIndexedIO::Node::name() const
{
	return m_entry.id();
}

void FileIndexedIO::Node::childNames( IndexedIO::EntryIDList &names ) const
{
	names.clear();
	names.reserve( m_children.size() );
	for ( ChildMap::const_iterator cit = m_children.begin(); cit != m_children.end(); cit++ )
	{
		names.push_back( cit->first );
	}
}

void FileIndexedIO::Node::childNames( IndexedIO::EntryIDList &names, IndexedIO::EntryType type ) const
{
	names.clear();
	names.reserve( m_children.size() );
	for ( ChildMap::const_iterator cit = m_children.begin(); cit != m_children.end(); cit++ )
	{
		if ( cit->second->m_entry.entryType() == type )
		{
			names.push_back( cit->first );
		}
	}
}

///////////////////////////////////////////////
//
// FileIndexedIO::Node (end)
//
///////////////////////////////////////////////


///////////////////////////////////////////////
//
// FileIndexedIO::Index (begin)
//
///////////////////////////////////////////////

FileIndexedIO::Index::Index( bool readOnly ) : m_root(0), m_prevId(0), m_readOnly(readOnly)
{
	m_version = g_currentVersion;

	m_root = new Node( this, 0 );

	m_indexToNodeMap.push_back(m_root.get());
	m_nodeToIndexMap[m_root.get()] = 0;

	m_stringCache.add("/");
	m_root->m_entry = IndexedIO::Entry("/", IndexedIO::Directory, IndexedIO::Invalid, 0);
	m_hasChanged = true;

	m_offset = 0;

	m_next = 0;
}

FileIndexedIO::Index::~Index()
{
	assert( m_freePagesOffset.size() == m_freePagesSize.size() );

	for (FreePagesOffsetMap::iterator it = m_freePagesOffset.begin(); it != m_freePagesOffset.end(); ++it)
	{
		assert(it->second);
		delete it->second;
	}
}

FileIndexedIO::Index::Index( FilteredStream &f, bool readOnly ) : m_prevId(0), m_readOnly(readOnly)
{
	m_hasChanged = false;

	f.seekg( 0, std::ios::end );
	Imf::Int64 end = f.tellg();

	f.seekg( end-1*sizeof(Imf::Int64), std::ios::beg );

	Imf::Int64 magicNumber = 0;
	readLittleEndian<Imf::Int64>( f, magicNumber );

	if ( magicNumber == g_versionedMagicNumber )
	{
		end -= 3*sizeof(Imf::Int64);
		f.seekg( end, std::ios::beg );
		readLittleEndian<Imf::Int64>( f, m_offset );
		readLittleEndian<Imf::Int64>( f, m_version );
	}
	else if (magicNumber == g_unversionedMagicNumber )
	{
		m_version = 0;
		end -= 2*sizeof(Imf::Int64);
		f.seekg( end, std::ios::beg );
		readLittleEndian<Imf::Int64>( f, m_offset );
	}
	else
	{
		throw IOException("Not a FileIndexedIO file");
	}

	f.seekg( m_offset, std::ios::beg );

	io::filtering_istream decompressingStream;
	std::istream *inputStream = &f;

	if (m_version >= 2 )
	{
		char *compressedIndex = new char[ end - m_offset ];
		f.read( compressedIndex, end - m_offset );
		MemoryStreamSource source( compressedIndex, end - m_offset, true );
		decompressingStream.push( io::gzip_decompressor() );
		decompressingStream.push( source );
		assert( decompressingStream.is_complete() );

		inputStream = &decompressingStream;
	}

	if (m_version >= 1)
	{
		m_stringCache = StringCache( *inputStream );
	}

	Imf::Int64 numNodes;
	readLittleEndian<Imf::Int64>( *inputStream, numNodes );

	m_indexToNodeMap.reserve( numNodes );

	for (Imf::Int64 i = 0; i < numNodes; i++)
	{
		readNode( *inputStream );
	}

	Imf::Int64 numFreePages;
	readLittleEndian<Imf::Int64>( *inputStream, numFreePages );

	m_next = m_offset;

	for (Imf::Int64 i = 0; i < numFreePages; i++)
	{
		Imf::Int64 offset, sz;
		readLittleEndian<Imf::Int64>( *inputStream, offset );
		readLittleEndian<Imf::Int64>( *inputStream, sz );

		addFreePage( offset, sz );
	}
}

Imf::Int64 FileIndexedIO::Index::write( std::ostream & f )
{
	/// Write index at end
	std::streampos indexStart = m_next;
	
	f.seekp( m_next, std::ios::beg );

	m_offset = indexStart;

	MemoryStreamSink sink;
	io::filtering_ostream compressingStream;
	compressingStream.push( io::gzip_compressor() );
	compressingStream.push( sink );
	assert( compressingStream.is_complete() );

	m_stringCache.write( compressingStream );

	Imf::Int64 numNodes = nodeCount();

	writeLittleEndian<Imf::Int64>( compressingStream, numNodes);

	write( compressingStream, m_root.get() );

	assert( m_freePagesOffset.size() == m_freePagesSize.size() );
	Imf::Int64 numFreePages = m_freePagesSize.size();

	// Write out number of free "pages"
	writeLittleEndian<Imf::Int64>( compressingStream, numFreePages);

	/// Write out each free page
	for ( FreePagesSizeMap::const_iterator it = m_freePagesSize.begin(); it != m_freePagesSize.end(); ++it)
	{
		writeLittleEndian<Imf::Int64>( compressingStream, it->second->m_offset );
		writeLittleEndian<Imf::Int64>( compressingStream, it->second->m_size );
	}

	/// To synchronize/close, etc.
	compressingStream.pop();
	compressingStream.pop();

	char *data=0;
	std::streamsize sz;
	sink.get( data, sz );
	assert( data );
	assert( sz > 0 );

	f.write( data, sz );

	writeLittleEndian<Imf::Int64>( f, m_offset );
	writeLittleEndian<Imf::Int64>( f, g_currentVersion );
	writeLittleEndian<Imf::Int64>( f, g_versionedMagicNumber );


	m_hasChanged = false;

	return f.tellp();
}

Imf::Int64 FileIndexedIO::Index::allocate( Imf::Int64 sz )
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

void FileIndexedIO::Index::deallocate( Node* n )
{
	assert(n);
	assert(n->m_entry.entryType() == IndexedIO::File);

	addFreePage( n->m_offset, n->m_size );
}

void FileIndexedIO::Index::addFreePage(  Imf::Int64 offset, Imf::Int64 sz )
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

Imf::Int64 FileIndexedIO::Index::nodeCount()
{
	return nodeCount(m_root.get());
}

void FileIndexedIO::Index::write( std::ostream &f, Node* n )
{
	n->write( f );

	for (Node::ChildMap::const_iterator it = n->m_children.begin(); it != n->m_children.end(); ++it)
	{
		/// Check tree consistency before writing
		assert( it->second->m_parent == n );
		assert( m_nodeToIndexMap.find( it->second->m_parent ) != m_nodeToIndexMap.end() );
		assert( m_nodeToIndexMap.find( it->second->m_parent )->second == n->m_id );

		write( f, it->second.get() );
	}
}

void FileIndexedIO::Index::readNode( std::istream &f )
{
	Node *n = new Node( this, 0 );

	assert(n);

	n->read( f );

	if (n->m_id == 0)
	{
		m_root = n;
		m_stringCache.add("/");
	}
}

Imf::Int64 FileIndexedIO::Index::nodeCount( Node* n )
{
	/// Size of this node...
	Imf::Int64 sz = 1;

	/// ... plus size of all children
	for (Node::ChildMap::const_iterator it = n->m_children.begin(); it != n->m_children.end(); ++it)
	{
		sz += nodeCount(it->second.get());
	}

	return sz;
}

bool FileIndexedIO::Index::canRead( const std::string &path )
{
	std::fstream d( path.c_str(), std::ios::binary | std::ios::in);

	if (! d.is_open() )
	{
		return false;
	}

	FilteredStream f;
	f.push<>( d );

	assert( f.is_complete() );

	f.seekg( 0, std::ios::end );
	Imf::Int64 end = f.tellg();

	f.seekg( end-1*sizeof(Imf::Int64), std::ios::beg );

	Imf::Int64 magicNumber;
	readLittleEndian<Imf::Int64>( f, magicNumber );

	if ( magicNumber == g_versionedMagicNumber || magicNumber == g_unversionedMagicNumber )
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool FileIndexedIO::Index::hasChanged() const
{
	return m_hasChanged;
}

Imf::Int64 FileIndexedIO::Index::makeId()
{
	/// \todo maybe hash the name, check for uniqueness in the tree, then use that?
	return ++m_prevId;
}

void FileIndexedIO::Index::deallocateWalk( Node* n )
{
	assert(n);

	if (n->m_entry.entryType() == IndexedIO::File)
	{
		deallocate(n);
	}

	for (Node::ChildMap::const_iterator it = n->m_children.begin(); it != n->m_children.end(); ++it)
	{
		deallocateWalk( it->second.get() );
	}
}

void FileIndexedIO::Index::remove( Node* n )
{
	assert(n);

	deallocateWalk(n);

	if (n->m_parent)
	{
		n->m_parent->m_children.erase( n->m_entry.id() );
	}
}

FileIndexedIO::Node* FileIndexedIO::Index::insert( Node* parent, IndexedIO::Entry e )
{
	if ( parent->child(e.id()) )
	{
		return 0;
	}

	Imf::Int64 newId = makeId();
	Node* child = new Node(this, newId);

	if ( newId >= m_indexToNodeMap.size() )
	{
		m_indexToNodeMap.resize(newId+1, NULL );
	}
	m_indexToNodeMap[newId] = child;
	m_nodeToIndexMap[child] = newId;

	child->m_entry = e;

	m_stringCache.add( e.id() );

	parent->registerChild( child );

	m_hasChanged = true;

	return child;
}

///////////////////////////////////////////////
//
// FileIndexedIO::Index (end)
//
///////////////////////////////////////////////

///////////////////////////////////////////////
//
// FileIndexedIO::IndexedFile (begin)
//
///////////////////////////////////////////////

class FileIndexedIO::IndexedFile : public RefCounted
{
	public:
		FilteredStream *m_stream;
		std::iostream *m_device;
		std::string m_filename;
		std::ios_base::openmode m_openmode;

		IndexedFile( const std::string &filename, IndexedIO::OpenMode mode );
		IndexedFile( std::iostream *device, bool newStream = false );

		virtual ~IndexedFile();

		/// Obtain the index for this file
		Index* index() const;

		/// Seek to a particular node within the file for reading
		void seekg( Node* node );

		/// Write some data to the file. Its position is automatically allocated within the file, and the node
		/// is updated to record this offset along with its size.
		void write(Node* node, const char *data, Imf::Int64 size);

		boost::optional<Imf::Int64> flush();

		std::iostream *device();

	protected:

		IndexPtr m_index;
};

FileIndexedIO::IndexedFile::IndexedFile( const std::string &filename, IndexedIO::OpenMode mode ) : m_filename( filename )
{
	if (mode & IndexedIO::Write)
	{
		m_openmode =  std::ios::trunc | std::ios::binary | std::ios::in | std::ios::out;
		std::fstream *f = new std::fstream(filename.c_str(), m_openmode);
		m_device = f;

		if (! f->is_open() )
		{
			throw IOException( "FileIndexedIO: Cannot open '" + filename + "' for writing" );
		}

		m_stream = new FilteredStream();
		m_stream->push<>( *m_device );
		assert( m_stream->is_complete() );

		m_index = new Index( false );
	}
	else if (mode & IndexedIO::Append)
	{
		if (!fs::exists( filename.c_str() ) )
		{
			/// Create new file
			m_openmode = std::ios::trunc | std::ios::binary | std::ios::in | std::ios::out;
			std::fstream *f = new std::fstream(filename.c_str(), m_openmode );
			m_device = f;

			if (! f->is_open() )
			{
				throw IOException( "FileIndexedIO: Cannot open '" + filename + "' for append" );
			}

			m_stream = new FilteredStream();
			m_stream->push<>( *m_device );
			assert( m_stream->is_complete() );

			m_index = new Index( false );
		}
		else
		{
			/// Read existing file
			m_openmode = std::ios::binary | std::ios::in | std::ios::out;
			std::fstream *f = new std::fstream(filename.c_str(), m_openmode );
			m_device = f;

			if (! f->is_open() )
			{
				throw IOException( "FileIndexedIO: Cannot open '" + filename + "' for read " );
			}

			m_stream = new FilteredStream();
			m_stream->push<>( *m_device );
			assert( m_stream->is_complete() );

			/// Read index
			try
			{
				m_index = new Index( *m_stream, false );
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
	else
	{
		assert( mode & IndexedIO::Read );
		m_openmode = std::ios::binary | std::ios::in;
		std::fstream *f = new std::fstream(filename.c_str(), m_openmode);
		m_device = f;

		if (! f->is_open() )
		{
			throw IOException( "FileIndexedIO: Cannot open file '" + filename + "' for read" );
		}

		m_stream = new FilteredStream();
		m_stream->push( *m_device );
		assert( m_stream->is_complete() );

		/// Read index
		try
		{
			m_index = new Index( *m_stream, true );
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

	assert( m_device );
	assert( m_stream );
	assert( m_stream->is_complete() );
	assert( m_index );
}


FileIndexedIO::IndexedFile::IndexedFile( std::iostream *device, bool newStream )
{
	assert( device );
	m_device = device;
	m_stream = new FilteredStream();
	m_stream->push<>( *m_device );
	assert( m_stream->is_complete() );

	if ( newStream )
	{
		m_index = new Index( false );
	}
	else
	{
		/// Read index
		try
		{
			m_index = new Index( *m_stream, false );
		}
		catch ( Exception &e )
		{
			throw;
		}
		catch (...)
		{
			throw IOException( "FileIndexedIO: Caught error reading index in device" );
		}
	}

	assert( m_device );
	assert( m_stream );
	assert( m_stream->is_complete() );
	assert( m_index );
}


void FileIndexedIO::IndexedFile::seekg( Node* node )
{
	assert( node->m_entry.entryType() == IndexedIO::File );
	assert( m_stream );

	m_stream->seekg( node->m_offset, std::ios::beg );
}

FileIndexedIO::Index* FileIndexedIO::IndexedFile::index() const
{
	assert(m_index);

	return m_index.get();
}

FileIndexedIO::IndexedFile::~IndexedFile()
{
	boost::optional<Imf::Int64> indexEnd = flush();

	assert( m_device );
	assert( m_stream );
	assert( m_stream->auto_close() );

	/// Truncate files which we're writing, at the end of the index
	/// \todo Derived classes also need to take care of this, e.g. MemoryIndexedIO. Add a general "truncate" virtual method in the
	/// next major version change.
	if ( indexEnd && ( m_openmode & std::ios::out ) )
	{
		std::fstream *f = dynamic_cast< std::fstream * >( m_device );

		/// If we're dealing with a physical file...
		if ( f )
		{
			f->seekg( 0, std::ios::end );
			Imf::Int64 fileEnd = f->tellg();

			// .. and the length of that file extends beyond the end of the index
			if ( fileEnd > *indexEnd )
			{
				/// Close the file before truncation
				delete m_stream;
				delete m_device;

				/// Truncate the file at the end of the index
				int err = truncate( m_filename.c_str(), *indexEnd );
				if ( err != 0 )
				{
					msg( Msg::Error, "FileIndexedIO", boost::format ( "Error truncating file '%s' to %d bytes: %s" ) % m_filename % (*indexEnd) % strerror( err ) );
				}
				return;
			}
		}
	}

	delete m_stream;
	delete m_device;
}

void FileIndexedIO::IndexedFile::write(Node* node, const char *data, Imf::Int64 size)
{
	/// Find next writable location
	Imf::Int64 loc = m_index->allocate( size );

	/// Seek 'write' pointer to writable location
	m_stream->seekp( loc, std::ios::beg );

	/// Clear error flags because problem on GCC 3.3.4:
	/// When the file is a std::stringstream then the first seekp(0) will fail and inhibit following operations on the file.
	m_stream->clear();

	/// Update node with positional information within file
	node->m_offset = loc;
	node->m_size = size;

	/// Write data
	m_stream->write( data, size );
}

boost::optional<Imf::Int64> FileIndexedIO::IndexedFile::flush()
{
	if (m_index->hasChanged())
	{
		Imf::Int64 end = m_index->write( *m_stream );
		assert( m_index->hasChanged() == false );
		return boost::optional<Imf::Int64>( end );
	}

	assert( m_device );
	m_device->flush();
	return boost::optional<Imf::Int64>();
}

std::iostream *FileIndexedIO::IndexedFile::device()
{
	return m_device;
}

///////////////////////////////////////////////
//
// FileIndexedIO::IndexedFile (end)
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
	return Index::canRead( path );
}

FileIndexedIO::FileIndexedIO(const std::string &path, const IndexedIO::EntryIDList &root, IndexedIO::OpenMode mode) : m_ioBufferLen(0), m_ioBuffer(0)
{
	validateOpenMode(mode);
	m_mode = mode;
	const fs::path p = fs::path(path);
	const std::string filename = p.native_file_string();

	if (! fs::exists(filename) && (mode & IndexedIO::Read))
	{
		throw FileNotFoundIOException(filename);
	}

	m_indexedFile = new IndexedFile( filename, mode );
	m_node = m_indexedFile->index()->m_root;
	setRoot( root );
	assert( m_node );
}

FileIndexedIO::FileIndexedIO() : m_ioBufferLen(0), m_ioBuffer(0)
{
}

void FileIndexedIO::open( std::iostream *device, const IndexedIO::EntryIDList &root, IndexedIO::OpenMode mode, bool newStream)
{
	validateOpenMode(mode);
	m_mode = mode;

	m_indexedFile = new IndexedFile( device, newStream );
	m_node = m_indexedFile->index()->m_root;
	setRoot( root );
	assert( m_node );
}

FileIndexedIO::~FileIndexedIO()
{
	if ( m_ioBuffer )
	{
		delete [] m_ioBuffer;
	}
}

void FileIndexedIO::setRoot( const IndexedIO::EntryIDList &root )
{
	IndexedIO::EntryIDList::const_iterator t = root.begin();
	for ( ; t != root.end(); t++ )
	{
		Node* childNode = m_node->child( *t );
		if ( !childNode )
		{
			break;
		}
		m_node = childNode;
	}
	bool found = ( t == root.end() );

	if (openMode() & IndexedIO::Read)
	{
		if (!found)
		{
			throw IOException( "FileIndexedIO: Cannot find entry '" + (*t).value() + "'" );
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
				Node* childNode = m_node->addChild( *t );
				if ( !childNode )
				{
					throw IOException( "FileIndexedIO: Cannot create entry '" + (*t).value() + "'" );
				}
				m_node = childNode;
			}
		}
	}
	assert( m_node );
}

boost::optional<Imf::Int64> FileIndexedIO::flush()
{
	return m_indexedFile->flush();
}

std::iostream *FileIndexedIO::device()
{
	return m_indexedFile->device();
}

IndexedIO::OpenMode FileIndexedIO::openMode() const
{
	return m_mode;
}

const IndexedIO::EntryID &FileIndexedIO::currentEntryId() const
{
	return m_node->name();
}

void FileIndexedIO::path( IndexedIO::EntryIDList &result ) const
{
	m_node->path(result);
}

void FileIndexedIO::entryIds( IndexedIO::EntryIDList &names ) const
{
	m_node->childNames( names );
}

void FileIndexedIO::entryIds( IndexedIO::EntryIDList &names, IndexedIO::EntryType type ) const
{
	m_node->childNames( names, type );
}

bool FileIndexedIO::hasEntry( const IndexedIO::EntryID &name ) const
{
	assert( m_node );
	return m_node->child( name );
}

FileIndexedIO::FileIndexedIO( const FileIndexedIO *other, Node *newRoot ) : m_ioBufferLen(0), m_ioBuffer(0)
{
	m_mode = other->m_mode;
	m_indexedFile = other->m_indexedFile;
	m_node = newRoot;
	assert( m_node );
}

IndexedIO * FileIndexedIO::duplicate(Node *rootNode) const
{
	return new FileIndexedIO( this, rootNode );
}

IndexedIOPtr FileIndexedIO::subdirectory( const IndexedIO::EntryID &name, IndexedIO::MissingBehavior missingBehavior )
{
	assert( m_node );
	Node* childNode = m_node->child( name );
	if ( !childNode )
	{
		if ( missingBehavior == IndexedIO::CreateIfMissing )
		{
			writable( name );
			childNode = m_node->addChild( name );
			if ( !childNode )
			{
				throw IOException( "FileIndexedIO: Could not insert child '" + name.value() + "'" );
			}
		}
		else if ( missingBehavior == IndexedIO::NullIfMissing )
		{
			return NULL;
		}
		else
		{
			throw IOException( "FileIndexedIO: Could not find child '" + name.value() + "'" );
		}
	}
	return duplicate(childNode);
}

ConstIndexedIOPtr FileIndexedIO::subdirectory( const IndexedIO::EntryID &name, IndexedIO::MissingBehavior missingBehavior ) const
{
	readable(name);
	assert( m_node );
	Node* childNode = m_node->child( name );
	if ( !childNode )
	{
		if ( missingBehavior == IndexedIO::NullIfMissing )
		{
			return NULL;
		}
		if ( missingBehavior == IndexedIO::CreateIfMissing )
		{
			throw IOException( "FileIndexedIO: No write access!" );
		}
		throw IOException( "FileIndexedIO: Could not find child '" + name.value() + "'" );
	}
	return duplicate(childNode);
}

void FileIndexedIO::remove( const IndexedIO::EntryID &name )
{
	assert( m_node );
	remove(name, true);
}

void FileIndexedIO::removeAll( )
{
	assert( m_node );
	IndexedIO::EntryIDList names;
	m_node->childNames( names );
	for ( IndexedIO::EntryIDList::const_iterator it = names.begin(); it != names.end(); it++ )
	{
		m_indexedFile->index()->remove( m_node->child( *it ) );
	}
}

void FileIndexedIO::remove( const IndexedIO::EntryID &name, bool throwIfNonExistent )
{
	assert( m_node );
	writable(name);

	Node* node = m_node->child( name );

	if (!node)
	{
		if (throwIfNonExistent)
		{
			throw IOException( "FileIndexedIO: Entry not found '" + name.value() + "'" );
		}
		else
		{
			return;
		}
	}
	m_indexedFile->index()->remove( node );
}

IndexedIO::Entry FileIndexedIO::entry(const IndexedIO::EntryID &name) const
{
	assert( m_node );
	readable(name);

	Node* node = m_node->child( name );

	if (!node)
	{
		throw IOException( "FileIndexedIO: Entry not found '" + name.value() + "'" );
	}

	return node->m_entry;
}

IndexedIOPtr FileIndexedIO::parentDirectory()
{
	assert( m_node );
	Node* parentNode = m_node->m_parent;
	if ( !parentNode )
	{
		return NULL;
	}
	return duplicate(parentNode);
}

ConstIndexedIOPtr FileIndexedIO::parentDirectory() const
{
	assert( m_node );
	Node* parentNode = m_node->m_parent;
	if ( !parentNode )
	{
		return NULL;
	}
	return duplicate(parentNode);
}

char *FileIndexedIO::ioBuffer( unsigned long size ) const
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

template<typename T>
void FileIndexedIO::write(const IndexedIO::EntryID &name, const T *x, unsigned long arrayLength)
{
	writable(name);
	remove(name, false);

	Node* node = m_node->addChild( name );
	if (node)
	{
		unsigned long size = IndexedIO::DataSizeTraits<T*>::size(x, arrayLength);
		IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T*>::type();

		char *data = ioBuffer(size);
		assert(data);
		IndexedIO::DataFlattenTraits<T*>::flatten(x, arrayLength, data);

		node->m_entry = IndexedIO::Entry( node->m_entry.id(), IndexedIO::File, dataType, arrayLength) ;

		m_indexedFile->write( node, data, size );
	}
	else
	{
		throw IOException( "FileIndexedIO: Could not insert node '" + name.value() + "' into index" );
	}
}

template<typename T>
void FileIndexedIO::write(const IndexedIO::EntryID &name, const T &x)
{
	writable(name);
	remove(name, false);

	Node* node = m_node->addChild( name );
	if (node)
	{
		unsigned long size = IndexedIO::DataSizeTraits<T>::size(x);
		IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T>::type();

		char *data = ioBuffer(size);
		assert(data);
		IndexedIO::DataFlattenTraits<T>::flatten(x, data);

		node->m_entry = IndexedIO::Entry( node->m_entry.id(), IndexedIO::File, dataType, 0) ;

		m_indexedFile->write( node, data, size );
	}
	else
	{
		throw IOException( "FileIndexedIO: Could not insert node '" + name.value() + "' into index" );
	}
}

template<typename T>
void FileIndexedIO::read(const IndexedIO::EntryID &name, T *&x, unsigned long arrayLength) const
{
	assert( m_node );
	readable(name);

	Node* node = m_node->child( name );

	if (!node || node->m_entry.entryType() != IndexedIO::File)
	{
		throw IOException( "FileIndexedIO: Entry not found '" + name.value() + "'" );
	}

	m_indexedFile->seekg( node );

	Imf::Int64 size = node->m_size;
	char *data = ioBuffer(size);
	m_indexedFile->m_stream->read( data, size );

	IndexedIO::DataFlattenTraits<T*>::unflatten( data, x, arrayLength );
}

template<typename T>
void FileIndexedIO::rawRead(const IndexedIO::EntryID &name, T *&x, unsigned long arrayLength) const
{
	assert( m_node );
	readable(name);

	Node* node = m_node->child( name );

	if (!node || node->m_entry.entryType() != IndexedIO::File)
	{
		throw IOException( "FileIndexedIO: Entry not found '" + name.value() + "'" );
	}

	m_indexedFile->seekg( node );

	Imf::Int64 size = node->m_size;
	m_indexedFile->m_stream->read( (char*)x, size );
}

template<typename T>
void FileIndexedIO::read(const IndexedIO::EntryID &name, T &x) const
{
	assert( m_node );
	readable(name);

	Node* node = m_node->child( name );

	if (!node || node->m_entry.entryType() != IndexedIO::File)
	{
		throw IOException( "FileIndexedIO: Entry not found '" + name.value() + "'" );
	}

	m_indexedFile->seekg( node );

	Imf::Int64 size = node->m_size;
	char *data = ioBuffer(size);
	m_indexedFile->m_stream->read( data, size );

	IndexedIO::DataFlattenTraits<T>::unflatten( data, x );
}

template<typename T>
void FileIndexedIO::rawRead(const IndexedIO::EntryID &name, T &x) const
{
	assert( m_node );
	readable(name);

	Node* node = m_node->child( name );

	if (!node || node->m_entry.entryType() != IndexedIO::File)
	{
		throw IOException( "FileIndexedIO: Entry not found '" + name.value() + "'" );
	}

	m_indexedFile->seekg( node );

	Imf::Int64 size = node->m_size;
	m_indexedFile->m_stream->read( (char*)&x, size );
}

// Write

void FileIndexedIO::write(const IndexedIO::EntryID &name, const float *x, unsigned long arrayLength)
{
	write<float>(name, x, arrayLength);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const double *x, unsigned long arrayLength)
{
	write<double>(name, x, arrayLength);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const half *x, unsigned long arrayLength)
{
	write<half>(name, x, arrayLength);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const int *x, unsigned long arrayLength)
{
	write<int>(name, x, arrayLength);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const int64_t *x, unsigned long arrayLength)
{
	write<int64_t>(name, x, arrayLength);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const uint64_t *x, unsigned long arrayLength)
{
	write<uint64_t>(name, x, arrayLength);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const unsigned int *x, unsigned long arrayLength)
{
	write<unsigned int>(name, x, arrayLength);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const char *x, unsigned long arrayLength)
{
	write<char>(name, x, arrayLength);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const unsigned char *x, unsigned long arrayLength)
{
	write<unsigned char>(name, x, arrayLength);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const std::string *x, unsigned long arrayLength)
{
	write<std::string>(name, x, arrayLength);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const short *x, unsigned long arrayLength)
{
	write<short>(name, x, arrayLength);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const unsigned short *x, unsigned long arrayLength)
{
	write<unsigned short>(name, x, arrayLength);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const float &x)
{
	write<float>(name, x);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const double &x)
{
	write<double>(name, x);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const half &x)
{
	write<half>(name, x);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const int &x)
{
	write<int>(name, x);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const int64_t &x)
{
	write<int64_t>(name, x);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const uint64_t &x)
{
	write<uint64_t>(name, x);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const std::string &x)
{
	write<std::string>(name, x);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const unsigned int &x)
{
	write<unsigned int>(name, x);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const char &x)
{
	write<char>(name, x);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const unsigned char &x)
{
	write<unsigned char>(name, x);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const short &x)
{
	write<short>(name, x);
}

void FileIndexedIO::write(const IndexedIO::EntryID &name, const unsigned short &x)
{
	write<unsigned short>(name, x);
}
// Read

#ifdef BOOST_LITTLE_ENDIAN
#define READ	rawRead
#else
#define READ	read
#endif

void FileIndexedIO::read(const IndexedIO::EntryID &name, float *&x, unsigned long arrayLength) const
{
	READ<float>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, double *&x, unsigned long arrayLength) const
{
	READ<double>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, half *&x, unsigned long arrayLength) const
{
	READ<half>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, int *&x, unsigned long arrayLength) const
{
	READ<int>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, int64_t *&x, unsigned long arrayLength) const
{
	READ<int64_t>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, uint64_t *&x, unsigned long arrayLength) const
{
	READ<uint64_t>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, unsigned int *&x, unsigned long arrayLength) const
{
	READ<unsigned int>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, char *&x, unsigned long arrayLength) const
{
	READ<char>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, unsigned char *&x, unsigned long arrayLength) const
{
	READ<unsigned char>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, std::string *&x, unsigned long arrayLength) const
{
	read<std::string>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, short *&x, unsigned long arrayLength) const
{
	READ<short>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, unsigned short *&x, unsigned long arrayLength) const
{
	READ<unsigned short>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, float &x) const
{
	READ<float>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, double &x) const
{
	READ<double>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, half &x) const
{
	READ<half>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, int &x) const
{
	READ<int>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, int64_t &x) const
{
	READ<int64_t>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, uint64_t &x) const
{
	READ<uint64_t>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, std::string &x) const
{
	read<std::string>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, unsigned int &x) const
{
	READ<unsigned int>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, char &x) const
{
	READ<char>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, unsigned char &x) const
{
	READ<unsigned char>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, short &x) const
{
	READ<short>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, unsigned short &x) const
{
	READ<unsigned short>(name, x);
}
