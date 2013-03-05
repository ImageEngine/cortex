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
#include "boost/optional.hpp"
#include "boost/format.hpp"
#include "boost/iostreams/device/file.hpp"
#include "boost/iostreams/filtering_streambuf.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/iostreams/stream.hpp"
#include "boost/iostreams/filter/gzip.hpp"

#include "IECore/ByteOrder.h"
#include "IECore/MemoryStream.h"
#include "IECore/MessageHandler.h"
#include "IECore/StreamIndexedIO.h"
#include "IECore/VectorTypedData.h"
#include "IECore/MurmurHash.h"

#define HARDLINK				127
#define NODES_EOF				125
static const Imf::Int64 g_unversionedMagicNumber = 0x0B00B1E5;
static const Imf::Int64 g_versionedMagicNumber = 0xB00B1E50;

/// File format history:
/// Version 4: introduced hard links (automatic data deduplication), also ability to store InternedString data.
/// Version 5: Hard links are represented as regular data nodes, that points to same data on file (no removal of data ever). 
///            Removed the linkCount field on the data nodes.
static const Imf::Int64 g_currentVersion = 5;

/// FileFormat ::= Data Index IndexOffset Version MagicNumber
/// Data ::= DataEntry*
/// Index ::= zip(StringCache Nodes FreePages)

/// DataEntry ::= Stores data from nodes: 
///                Data nodes: binary data indexed by DataOffset/DataSize and 

/// StringCache ::= NumStrings String*
/// NumStrings ::= int64
/// String ::= StringLength char*
/// StringLength ::= int64

/// Nodes ::= NumNodes Node* EndNode
/// NumNodes ::= int64 (total nodes stored in the file or last ID used )
/// Node ::= EntryType EntryStringCacheID NodeID ParentNodeID ( if EntryType == Directory )
///          EntryType EntryStringCacheID DataType ArrayLength NodeID ParentNodeID DataOffset DataSize ( if EntryType == File )
/// EndNode ::= char ( NODES_EOF )
/// EntryType ::= char ( value from IndexedIO::EntryType )
/// EntryStringCacheID ::= int64 ( index in StringCache )
/// DataType ::= char ( value from IndexedIO::DataType )
/// ArrayLength ::= int64 ( if DataType is array, then this tells how long they are )
/// NodeID ::= int64 ( unique Id of this node in the file )
/// ParentNodeID ::= int64 ( Id for the parent node )
/// DataOffset ::= int64 ( this is offset where the data is located )
/// DataSize ::= int64 ( number of bytes stored in the data section )

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
void writeLittleEndian( F &f, T n )
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

class StreamIndexedIO::StringCache
{
	public:

		StringCache() : m_prevId(0), m_ioBuffer(0), m_ioBufferLen(0)
		{
			m_idToStringMap.reserve(100);
		}

		template < typename F >
		StringCache( F &f ) : m_prevId(0), m_ioBuffer(0), m_ioBufferLen(0)
		{
			Imf::Int64 sz;
			readLittleEndian(f,sz);

			m_idToStringMap.reserve(sz + 100);

			for (Imf::Int64 i = 0; i < sz; ++i)
			{
				const char *s = read(f);

				Imf::Int64 id;
				readLittleEndian( f,id );
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

			assert( m_stringToIdMap.size() == m_idToStringMap.size() );
		}

		Imf::Int64 size() const
		{
			assert( m_stringToIdMap.size() == m_idToStringMap.size() );
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

/// A single node within an index
class StreamIndexedIO::Node : public RefCounted
{
	public:

		/// A unique numeric ID for this node
		Imf::Int64 m_id;

		/// The offset in the file to this node's data
		Imf::Int64 m_offset;

		/// The size of this node's data chunk within the file
		Imf::Int64 m_size;

		/// A brief description of the node
		IndexedIO::Entry m_entry;

		/// A shared pointer to the main file index
		StreamIndexedIO::Index* m_idx;

		/// A pointer to the parent node in the tree - will be NULL for the root node
		Node* m_parent;

		/// Pointers to this node's children
		typedef std::map< IndexedIO::EntryID, NodePtr> ChildMap;
		ChildMap m_children;

		/// only used for file format version 4
		bool m_isLink;

	public:

		/// Construct a new Node in the given index with the given numeric id
		Node(StreamIndexedIO::Index* index);

		virtual ~Node();

		/// Write this node to a stream
		template < typename F >
		void write( F &f );

		/// Replace the contents of this node with data read from a stream.
		/// Returns True if the node was loaded and false if EOF is reached.
		template < typename F >
		bool read( F &f );

		void childNames( IndexedIO::EntryIDList &names ) const;
		void childNames( IndexedIO::EntryIDList &names, IndexedIO::EntryType ) const;

		const IndexedIO::EntryID &name() const;
		void path( IndexedIO::EntryIDList &result ) const;

		bool hasChild( const IndexedIO::EntryID &name ) const;

		// Returns the named child node or NULL if not existent.
		Node* child( const IndexedIO::EntryID &name ) const;

		Node* addChild( const IndexedIO::EntryID & childName );

		// Returns the location for the data
		Imf::Int64 offset() const;

	protected:

		friend class StreamIndexedIO::Index;

		

		/// registers a child node in this node
		void registerChild( Node* c );
};

/// A tree to represent nodes in a filesystem, along with their locations in a file.
class StreamIndexedIO::Index : public RefCounted
{
	public:

		friend class Node;

		/// Construct an index from reading a file stream.
		Index( StreamIndexedIO::StreamFilePtr stream );
		virtual ~Index();

		/// function called right after construction
		void openStream();

		Node *root() const;

		/// Remove a node, and all its subnodes from the index
		void remove( Node* n );

		/// Insert a new entry into the index, returning the node which stores it. If it already exists, returns 0
		Node* insert( Node* parentNode, IndexedIO::Entry e );

		/// Allocate a new chunk of data of the requested size, returning its offset within the file
		Imf::Int64 allocate( Imf::Int64 sz );

		/// Deallocate a node's data from the file.
		void deallocate( Node* n );

		/// Return the total number of nodes in the index.
		Imf::Int64 nodeCount() const;

		/// Queries the string cache
		StringCache &stringCache();

		StreamIndexedIO::StreamFile &streamFile() const;

		/// flushes index to the file
		void flush();

		/// Write node data to the file. Its position is automatically allocated within the file, and the node
		/// is updated to record this offset along with its size (or it's mapped to a previously saved data block if the hash matches - hardlink).
		/// The hardlinks will only be created for nodes that have been saved on the same session. So edit mode will not be that great.
		void writeNodeData( Node *node, const char *data, Imf::Int64 size );

	protected:

		NodePtr m_root;

		Imf::Int64 m_version;

		bool m_hasChanged;

		Imf::Int64 m_offset;
		Imf::Int64 m_next;

		Imf::Int64 m_prevId;

		Imf::Int64 makeId();

		typedef std::vector< Node* > IndexToNodeMap;

		IndexToNodeMap m_indexToNodeMap;

#ifndef NDEBUG
		typedef std::map< Node*, unsigned long > NodeToIndexMap;
		NodeToIndexMap m_nodeToIndexMap;
#endif
		typedef std::map< std::pair<MurmurHash,Imf::Int64>, Imf::Int64 > HashToDataMap;
		HashToDataMap m_hashToDataMap;

		StringCache m_stringCache;

		StreamIndexedIO::StreamFilePtr m_stream;

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

		/// Write the index to the file stream
		Imf::Int64 write();

		template < typename F >
		void write( F &f, Node* n );

		template < typename F >
		void read( F &f );

		template < typename F >
		bool readNode( F &f );

		static Imf::Int64 nodeCount( Node* n );

};

///////////////////////////////////////////////
//
// StreamIndexedIO::Node (begin)
//
///////////////////////////////////////////////

StreamIndexedIO::Node::Node(Index* index) : RefCounted(), m_id(0), m_offset(0), m_size(0), m_idx(index), m_parent(0)
{
	m_isLink = false;
}

StreamIndexedIO::Node::~Node()
{
}

Imf::Int64 StreamIndexedIO::Node::offset() const
{
	return m_offset;
}

void StreamIndexedIO::Node::registerChild( Node* c )
{
	if (c->m_parent)
	{
		throw IOException("StreamIndexedIO: Node already has parent!");
	}

#ifndef NDEBUG
	/// Make sure we never try to add the same child twice
	ChildMap::const_iterator cit = m_children.find( c->m_entry.id() );
	assert (cit == m_children.end());
#endif

	c->m_parent = this;
	m_children.insert( std::map< IndexedIO::EntryID, NodePtr >::value_type( c->m_entry.id(), c) );
}

template < typename F >
void StreamIndexedIO::Node::write( F &f )
{
	char t = m_entry.entryType();
	f.write( &t, sizeof(char) );

	Imf::Int64 id = m_idx->m_stringCache.find( m_entry.id() );
	writeLittleEndian( f, id );
	if ( m_entry.entryType() == IndexedIO::File )
	{
		t = m_entry.dataType();
		f.write( &t, sizeof(char) );

		if ( m_entry.isArray() )
		{
			writeLittleEndian<F,Imf::Int64>( f, m_entry.arrayLength() );
		}
	}

	writeLittleEndian(f, m_id);

	if (m_parent)
	{
		assert( m_idx->m_nodeToIndexMap.find( m_parent ) != m_idx->m_nodeToIndexMap.end() );
		assert( m_idx->m_nodeToIndexMap.find( m_parent )->second == m_parent->m_id );
		writeLittleEndian(f, m_parent->m_id);
	}
	else
	{
		writeLittleEndian<F,Imf::Int64>(f, Imath::limits<Imf::Int64>::max() );
	}

	// Data nodes have to store the data block offset/size
	if ( m_entry.entryType() == IndexedIO::File )
	{
		writeLittleEndian(f, m_offset);
		writeLittleEndian(f, m_size);
	}
}

template < typename F >
bool StreamIndexedIO::Node::read( F &f )
{
	assert( m_idx );

	char t;
	f.read( &t, sizeof(char) );

	IndexedIO::EntryType entryType = (IndexedIO::EntryType)t;

	if ( t == NODES_EOF )
	{
		return false;
	}

	if ( t == HARDLINK ) /// only at version = 4
	{
		entryType = IndexedIO::File;
		m_isLink = true;
	}

	IndexedIO::DataType dataType = IndexedIO::Invalid;
	Imf::Int64 arrayLength = 0;

	const IndexedIO::EntryID *id;
	if (m_idx->m_version >= 1)
	{
		Imf::Int64 stringId;
		readLittleEndian(f,stringId);
		id = &m_idx->m_stringCache.findById( stringId );
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

	if ( !m_isLink )
	{
		if ( entryType == IndexedIO::File || m_idx->m_version < 2 )
		{
			f.read( &t, sizeof(char) );
			dataType = (IndexedIO::DataType)t;
	
			if ( IndexedIO::Entry::isArray( dataType ) || m_idx->m_version < 3 )
			{
				readLittleEndian( f,arrayLength );
			}
		}
	}

	m_entry = IndexedIO::Entry( *id, entryType, dataType, static_cast<unsigned long>( arrayLength ) );

	if ( m_idx->m_version < 1 )
	{
		delete id;
	}

	readLittleEndian( f,m_id );

	if ( m_id >= m_idx->m_indexToNodeMap.size() )
	{
		m_idx->m_indexToNodeMap.resize(m_id+1, NULL);
	}
	m_idx->m_indexToNodeMap[m_id] = this;

	// we only need to keep the map node=>index if we intend to save the file...
#ifndef NDEBUG
	m_idx->m_nodeToIndexMap[this] = m_id;
#endif

	m_idx->m_prevId = std::max( m_idx->m_prevId, m_id );

	Imf::Int64 parentId;
	readLittleEndian( f,parentId );

	if ( m_id && parentId != Imath::limits<Imf::Int64>::max() )
	{
		m_idx->m_prevId = std::max( m_idx->m_prevId, parentId );

		Node* parent = 0;
		if ( parentId < m_idx->m_indexToNodeMap.size() )
		{
			parent = m_idx->m_indexToNodeMap[parentId] ;
		}

		if ( !parent )
		{
			throw IOException("StreamIndexedIO: parentId not found");
		}

		parent->registerChild(this);
	}
	
	if ( m_isLink )
	{
		Imf::Int64 targetNodeId;
		// load Target Node ID in m_offset
		readLittleEndian( f,targetNodeId );
		m_offset = targetNodeId;
		m_size = 0;	// we cannot assure that the target node is already loaded, so we set size to zero for now
	}
	else if ( entryType == IndexedIO::File || m_idx->m_version < 2 )
	{
		readLittleEndian( f,m_offset );
		readLittleEndian( f,m_size );

		if ( m_idx->m_version == 4 )
		{
			/// ignore link count data
			typedef unsigned short LinkCount;
			LinkCount linkCount;
			readLittleEndian(f,linkCount);
		}
	}
	else
	{
		m_offset = 0;
		m_size = 0;
	}

	return true;
}

bool StreamIndexedIO::Node::hasChild( const IndexedIO::EntryID &name ) const
{
	return child( name );
}

StreamIndexedIO::Node* StreamIndexedIO::Node::child( const IndexedIO::EntryID &name ) const
{
	ChildMap::const_iterator cit = m_children.find( name );
	if (cit == m_children.end())
	{
		return 0;
	}
	Node *n = cit->second.get();
	return n;
}

StreamIndexedIO::Node* StreamIndexedIO::Node::addChild( const IndexedIO::EntryID &childName )
{
	Node* child = m_idx->insert( this, IndexedIO::Entry( childName, IndexedIO::Directory, IndexedIO::Invalid, 0 ) );
	return child;
}

void StreamIndexedIO::Node::path( IndexedIO::EntryIDList &result ) const
{
	if ( m_parent )
	{
		m_parent->path( result );
		result.push_back( m_entry.id() );
	}
}

const IndexedIO::EntryID &StreamIndexedIO::Node::name() const
{
	return m_entry.id();
}

void StreamIndexedIO::Node::childNames( IndexedIO::EntryIDList &names ) const
{
	names.clear();
	names.reserve( m_children.size() );
	for ( ChildMap::const_iterator cit = m_children.begin(); cit != m_children.end(); cit++ )
	{
		names.push_back( cit->first );
	}
}

void StreamIndexedIO::Node::childNames( IndexedIO::EntryIDList &names, IndexedIO::EntryType type ) const
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
// StreamIndexedIO::Node (end)
//
///////////////////////////////////////////////

///////////////////////////////////////////////
//
// StreamIndexedIO::Index (begin)
//
///////////////////////////////////////////////

StreamIndexedIO::Index::Index( StreamIndexedIO::StreamFilePtr stream ) : m_root(0), m_version(g_currentVersion), m_hasChanged(false), m_offset(0), m_next(0), m_prevId(0), m_stream(stream)
{
	m_stringCache.add(IndexedIO::rootName);
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
		Imf::Int64 end = f.tellg();
		f.seekg( end-1*sizeof(Imf::Int64), std::ios::beg );

		Imf::Int64 magicNumber = 0;
		readLittleEndian( f,magicNumber );

		if ( magicNumber == g_versionedMagicNumber )
		{
			end -= 3*sizeof(Imf::Int64);
			f.seekg( end, std::ios::beg );
			readLittleEndian( f,m_offset );
			readLittleEndian( f,m_version );
		}
		else if (magicNumber == g_unversionedMagicNumber )
		{
			m_version = 0;
			end -= 2*sizeof(Imf::Int64);
			f.seekg( end, std::ios::beg );
			readLittleEndian( f,m_offset );
		}
		else
		{
			throw IOException("Not a StreamIndexedIO file");
		}

		f.seekg( m_offset, std::ios::beg );

		if (m_version >= 2 )
		{
			io::filtering_istream decompressingStream;
			char *compressedIndex = new char[ end - m_offset ];
			f.read( compressedIndex, end - m_offset );
			MemoryStreamSource source( compressedIndex, end - m_offset, true );
			decompressingStream.push( io::gzip_decompressor() );
			decompressingStream.push( source );
			assert( decompressingStream.is_complete() );

			read( decompressingStream );
		}
		else
		{
			read( f );
		}
	}
	else
	{
		// creating a new empty Index
		m_root = new Node( this );
		// assign index 0 to the root node
		m_indexToNodeMap.push_back(m_root.get());

#ifndef NDEBUG
		m_nodeToIndexMap[m_root.get()] = 0;
#endif
		m_root->m_entry = IndexedIO::Entry(IndexedIO::rootName, IndexedIO::Directory, IndexedIO::Invalid, 0);
		m_hasChanged = true;
	}
}

StreamIndexedIO::Node *StreamIndexedIO::Index::root() const
{
	return m_root.get();
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
void StreamIndexedIO::Index::read( F &f )
{
	if (m_version >= 1)
	{
		m_stringCache = StringCache( f );
	}

	Imf::Int64 numNodes;
	readLittleEndian( f, numNodes );

	m_prevId = numNodes - 1;

	m_indexToNodeMap.reserve( numNodes );

	if ( m_version <= 4 )
	{
		for (Imf::Int64 i = 0; i < numNodes; i++)
		{
			readNode( f );
		}
	}
	else
	{
		// read all nodes until we reach the EOF character.
		while( readNode( f ) );
	}

	if ( m_version == 4 )
	{
		// symlinks have to get the Entry information from their target nodes.
		for (IndexToNodeMap::const_iterator it = m_indexToNodeMap.begin(); it != m_indexToNodeMap.end(); it++ )
		{
			Node *n = *it;
			if ( !n )
			{
				continue;
			}

			if ( n->m_isLink && !n->m_size )
			{
				Imf::Int64 targetNodeId = n->m_offset;
				if ( targetNodeId >= m_indexToNodeMap.size() || !m_indexToNodeMap[targetNodeId] )
				{
					throw IOException("StreamIndexedIO: targetNodeId not found");
				}
				Node* targetNode = m_indexToNodeMap[targetNodeId];
				unsigned long arrayLength = 0;
				IndexedIO::DataType dataType = targetNode->m_entry.dataType();
				if ( targetNode->m_entry.isArray() )
				{
					arrayLength = targetNode->m_entry.arrayLength();
				}
				/// turn this node into a regular data node that happens to point to the same data.
				n->m_isLink = false;
				n->m_entry = IndexedIO::Entry( n->m_entry.id(), IndexedIO::File, dataType, arrayLength );
				n->m_offset = targetNode->m_offset;
				n->m_size = targetNode->m_size;
			}
		}
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

Imf::Int64 StreamIndexedIO::Index::write()
{
	StreamIndexedIO::StreamFile &f = *m_stream;

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

	writeLittleEndian( compressingStream, m_prevId+1 );

	write( compressingStream, m_root.get() );

	unsigned char eof = NODES_EOF;
	writeLittleEndian( compressingStream, eof );

	assert( m_freePagesOffset.size() == m_freePagesSize.size() );
	Imf::Int64 numFreePages = m_freePagesSize.size();

	// Write out number of free "pages"
	writeLittleEndian( compressingStream, numFreePages);

	/// Write out each free page
	for ( FreePagesSizeMap::const_iterator it = m_freePagesSize.begin(); it != m_freePagesSize.end(); ++it)
	{
		writeLittleEndian( compressingStream, it->second->m_offset );
		writeLittleEndian( compressingStream, it->second->m_size );
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

void StreamIndexedIO::Index::deallocate( Node* n )
{
	assert(n);
	assert(n->m_entry.entryType() == IndexedIO::File);

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

Imf::Int64 StreamIndexedIO::Index::nodeCount() const
{
	Imf::Int64 c = nodeCount(m_root.get());
	return c;
}

template < typename F >
void StreamIndexedIO::Index::write( F &f, Node* n )
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

void StreamIndexedIO::Index::writeNodeData( Node *node, const char *data, Imf::Int64 size )
{
	/// Find next writable location
	Imf::Int64 loc;

	node->m_size = size;

	// compute hash for the data
	MurmurHash hash;
	hash.append( data, size );

	// see if it's already stored by another node..
	std::pair< HashToDataMap::iterator,bool > ret = m_hashToDataMap.insert( HashToDataMap::value_type( std::pair< MurmurHash,Imf::Int64>(hash,size), 0 ) );
	if ( !ret.second )
	{
		// we already saved this data, so we dont save any additional data for this node.
		loc = ret.first->second;
		m_hasChanged = true;
		node->m_offset = loc;
		return;
	}

	/// New data, find next writable location.
	loc = allocate( size );
	ret.first->second = loc;

	/// stores in the node the new location
	node->m_offset = loc;

	/// Seek 'write' pointer to writable location
	m_stream->seekp( loc, std::ios::beg );

	/// Write data
	m_stream->write( data, size );
}

template < typename F >
bool StreamIndexedIO::Index::readNode( F &f )
{
	Node *n = new Node( this );

	assert(n);

	/// read the node information from file
	if ( !n->read( f ) )
	{
		// reached the end of the block
		delete n;
		return false;
	}

	if (n->m_id == 0)
	{
		m_root = n;
	}
	else if ( !n->m_parent )
	{
		throw IOException("StreamIndexedIO: Non-root node has no parent.");
	}

	return true;
}

Imf::Int64 StreamIndexedIO::Index::nodeCount( Node* n )
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

Imf::Int64 StreamIndexedIO::Index::makeId()
{
	return ++m_prevId;
}

void StreamIndexedIO::Index::deallocateWalk( Node* n )
{
	assert(n);

	if (n->m_entry.entryType() == IndexedIO::File)
	{
		// We don't deallocate data blocks because they could be referred by other nodes.
		// As a result, editing files will usually increase file size.
	}

	for (Node::ChildMap::const_iterator it = n->m_children.begin(); it != n->m_children.end(); ++it)
	{
		deallocateWalk( it->second.get() );
	}

	n->m_children.clear();
}

void StreamIndexedIO::Index::remove( Node* n )
{
	assert(n);

	Node *parent = n->m_parent;

	deallocateWalk(n);

	if (parent)
	{
		parent->m_children.erase( n->m_entry.id() );
	}
}

StreamIndexedIO::Node* StreamIndexedIO::Index::insert( Node* parent, IndexedIO::Entry e )
{
	if ( parent->hasChild(e.id()) )
	{
		return 0;
	}

	Imf::Int64 newId = makeId();
	Node* child = new Node(this);
	child->m_id = newId;

	if ( newId >= m_indexToNodeMap.size() )
	{
		m_indexToNodeMap.resize(newId+1, NULL );
	}
	m_indexToNodeMap[newId] = child;
#ifndef NDEBUG
	m_nodeToIndexMap[child] = newId;
#endif

	child->m_entry = e;

	m_stringCache.add( e.id() );

	parent->registerChild( child );

	m_hasChanged = true;

	return child;
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

StreamIndexedIO::StreamFile::StreamFile( IndexedIO::OpenMode mode ) : m_openmode(mode), m_stream(0), m_ioBufferLen(0), m_ioBuffer(0)
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

void StreamIndexedIO::StreamFile::setStream( std::iostream *stream, bool emptyFile )
{
	m_stream = stream;
	assert( m_stream->is_complete() );
	if ( m_openmode & IndexedIO::Append && emptyFile )
	{
		m_openmode = (m_openmode ^ IndexedIO::Append) | IndexedIO::Write;
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

tbb::mutex & StreamIndexedIO::StreamFile::mutex()
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
	assert( f.is_complete() );

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

StreamIndexedIO::StreamIndexedIO() : m_node(0)
{
}

StreamIndexedIO::StreamIndexedIO( StreamIndexedIO::Node &node )
{
	m_node = &node;
	// we add a reference to the index
	m_node->m_idx->addRef();
}

void StreamIndexedIO::open( StreamFilePtr file, const IndexedIO::EntryIDList &root )
{
	IndexPtr newIndex = new Index( file );
	newIndex->openStream();
	m_node = newIndex->root();
	setRoot( root );
	assert( m_node );
	// we add a reference to the index
	newIndex->addRef();
}

StreamIndexedIO::~StreamIndexedIO()
{
	if ( m_node.get() )
	{
		// remove our reference to the index, it's destructor triggers the 
		// flush to disk.
		m_node->m_idx->removeRef();
	}
}

void StreamIndexedIO::setRoot( const IndexedIO::EntryIDList &root )
{
	IndexedIO::EntryIDList::const_iterator t = root.begin();
	for ( ; t != root.end(); t++ )
	{
		NodePtr childNode = m_node->child( *t );
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
				NodePtr childNode = m_node->addChild( *t );
				if ( !childNode )
				{
					throw IOException( "StreamIndexedIO: Cannot create entry '" + (*t).value() + "'" );
				}
				m_node = childNode;
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

const IndexedIO::EntryID &StreamIndexedIO::currentEntryId() const
{
	return m_node->name();
}

void StreamIndexedIO::path( IndexedIO::EntryIDList &result ) const
{
	result.clear();
	m_node->path(result);
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
	Node *childNode = m_node->child( name );
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
			return NULL;
		}
		else
		{
			throw IOException( "StreamIndexedIO: Could not find child '" + name.value() + "'" );
		}
	}
	return duplicate(*childNode);
}

ConstIndexedIOPtr StreamIndexedIO::subdirectory( const IndexedIO::EntryID &name, IndexedIO::MissingBehaviour missingBehaviour ) const
{
	readable(name);
	assert( m_node );
	Node *childNode = m_node->child( name );
	if ( !childNode )
	{
		if ( missingBehaviour == IndexedIO::NullIfMissing )
		{
			return NULL;
		}
		if ( missingBehaviour == IndexedIO::CreateIfMissing )
		{
			throw IOException( "StreamIndexedIO: No write access!" );
		}
		throw IOException( "StreamIndexedIO: Could not find child '" + name.value() + "'" );
	}
	return duplicate(*childNode);
}

IndexedIOPtr StreamIndexedIO::createSubdirectory( const IndexedIO::EntryID &name )
{
	assert( m_node );
	if ( m_node->hasChild(name) )
	{
		throw IOException( "Child '" + name.value() + "' already exists!" );
	}
	writable( name );
	Node *childNode = m_node->addChild( name );
	if ( !childNode )
	{
		throw IOException( "StreamIndexedIO: Could not insert child '" + name.value() + "'" );
	}
	return duplicate(*childNode);
}

void StreamIndexedIO::remove( const IndexedIO::EntryID &name )
{
	assert( m_node );
	remove(name, true);
}

void StreamIndexedIO::removeAll( )
{
	assert( m_node );
	IndexedIO::EntryIDList names;
	m_node->childNames( names );
	Index *index = m_node->m_idx;
	for ( IndexedIO::EntryIDList::const_iterator it = names.begin(); it != names.end(); it++ )
	{
		index->remove( m_node->child( *it ) );
	}
}

void StreamIndexedIO::remove( const IndexedIO::EntryID &name, bool throwIfNonExistent )
{
	assert( m_node );
	writable(name);

	Node* node = m_node->child( name );

	if (!node)
	{
		if (throwIfNonExistent)
		{
			throw IOException( "StreamIndexedIO: Entry not found '" + name.value() + "'" );
		}
		else
		{
			return;
		}
	}
	node->m_idx->remove( node );
}

IndexedIO::Entry StreamIndexedIO::entry(const IndexedIO::EntryID &name) const
{
	assert( m_node );
	readable(name);

	Node* node = m_node->child( name );

	if (!node)
	{
		throw IOException( "StreamIndexedIO: Entry not found '" + name.value() + "'" );
	}

	return node->m_entry;
}

IndexedIOPtr StreamIndexedIO::parentDirectory()
{
	assert( m_node );
	Node *parentNode = m_node->m_parent;
	if ( !parentNode )
	{
		return NULL;
	}
	return duplicate(*parentNode);
}

ConstIndexedIOPtr StreamIndexedIO::parentDirectory() const
{
	assert( m_node );
	Node* parentNode = m_node->m_parent;
	if ( !parentNode )
	{
		return NULL;
	}
	return duplicate(*parentNode);
}

IndexedIOPtr StreamIndexedIO::directory( const IndexedIO::EntryIDList &path, IndexedIO::MissingBehaviour missingBehaviour )
{
	// from the root go to the path
	Node* root = m_node;
	Node* parentNode = root->m_parent;
	while (parentNode)
	{
		root = parentNode;
		parentNode = root->m_parent;
	}
	Node* node = root;
	for ( IndexedIO::EntryIDList::const_iterator pIt = path.begin(); pIt != path.end(); pIt++ )
	{
		const IndexedIO::EntryID &name = *pIt;

		Node* childNode = node->child( name );
		if ( !childNode )
		{
			if ( missingBehaviour == IndexedIO::CreateIfMissing )
			{
				writable( name );
				childNode = node->addChild( name );
				if ( !childNode )	
				{
					throw IOException( "StreamIndexedIO: Could not insert child '" + name.value() + "'" );
				}
			}
			else if ( missingBehaviour == IndexedIO::NullIfMissing )
			{
				return NULL;
			}
			else
			{
				throw IOException( "StreamIndexedIO: Could not find child '" + name.value() + "'" );
			}
		}
		node = childNode;
	}
	return duplicate(*node);
}

ConstIndexedIOPtr StreamIndexedIO::directory( const IndexedIO::EntryIDList &path, IndexedIO::MissingBehaviour missingBehaviour ) const
{
	return const_cast< StreamIndexedIO * >(this)->directory( path, missingBehaviour == IndexedIO::CreateIfMissing ? IndexedIO::ThrowIfMissing : missingBehaviour );
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const InternedString *x, unsigned long arrayLength)
{
	writable(name);
	remove(name, false);

	Node* node = m_node->addChild( name );
	if (!node)
	{
		throw IOException( "StreamIndexedIO: Could not insert node '" + name.value() + "' into index" );
	}

	Imf::Int64 *ids = new Imf::Int64[arrayLength];
	const Imf::Int64 *constIds = ids;
	unsigned long size = IndexedIO::DataSizeTraits<Imf::Int64 *>::size(constIds, arrayLength);
	IndexedIO::DataType dataType = IndexedIO::InternedStringArray;

	char *data = streamFile().ioBuffer(size);
	assert(data);

	Index *index = node->m_idx;

	StringCache &stringCache = index->stringCache();

	for ( unsigned long i = 0; i < arrayLength; i++ )
	{
		ids[i] = stringCache.find( x[i], false /* create entry if missing */ );
	}

	IndexedIO::DataFlattenTraits<Imf::Int64*>::flatten(constIds, arrayLength, data);

	node->m_entry = IndexedIO::Entry( node->m_entry.id(), IndexedIO::File, dataType, arrayLength) ;

	index->writeNodeData( node, data, size );

	delete [] ids;
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, InternedString *&x, unsigned long arrayLength) const
{
	assert( m_node );
	readable(name);

	Node* node = m_node->child( name );

	if (!node || node->m_entry.entryType() != IndexedIO::File)
	{
		throw IOException( "StreamIndexedIO: Entry not found '" + name.value() + "'" );
	}

	Imf::Int64 *ids = new Imf::Int64[arrayLength];
	Imf::Int64 size = node->m_size;
	StreamIndexedIO::StreamFile &f = streamFile();
	{
		StreamFile::MutexLock lock( f.mutex() );
		f.seekg( node->offset(), std::ios::beg );

#ifdef IE_CORE_LITTLE_ENDIAN
		// raw read
		f.read( (char*)ids, size );
	}
#else
		char *data = f.ioBuffer(size);
		f.read( data, size );
	}
	IndexedIO::DataFlattenTraits<Imf::Int64*>::unflatten( data, ids, arrayLength );
#endif

	const StringCache &stringCache = node->m_idx->stringCache();
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

	Node* node = m_node->addChild( name );
	if (node)
	{
		unsigned long size = IndexedIO::DataSizeTraits<T*>::size(x, arrayLength);
		IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T*>::type();

		char *data = streamFile().ioBuffer(size);
		assert(data);
		IndexedIO::DataFlattenTraits<T*>::flatten(x, arrayLength, data);

		node->m_entry = IndexedIO::Entry( node->m_entry.id(), IndexedIO::File, dataType, arrayLength) ;

		node->m_idx->writeNodeData( node, data, size );
	}
	else
	{
		throw IOException( "StreamIndexedIO: Could not insert node '" + name.value() + "' into index" );
	}
}

template<typename T>
void StreamIndexedIO::rawWrite(const IndexedIO::EntryID &name, const T *x, unsigned long arrayLength)
{
	writable(name);
	remove(name, false);

	Node* node = m_node->addChild( name );
	if (node)
	{
		unsigned long size = IndexedIO::DataSizeTraits<T*>::size(x, arrayLength);
		IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T*>::type();

		node->m_entry = IndexedIO::Entry( node->m_entry.id(), IndexedIO::File, dataType, arrayLength) ;

		node->m_idx->writeNodeData( node, (char*)x, size );
	}
	else
	{
		throw IOException( "StreamIndexedIO: Could not insert node '" + name.value() + "' into index" );
	}
}

template<typename T>
void StreamIndexedIO::write(const IndexedIO::EntryID &name, const T &x)
{
	writable(name);
	remove(name, false);

	Node* node = m_node->addChild( name );
	if (node)
	{
		unsigned long size = IndexedIO::DataSizeTraits<T>::size(x);
		IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T>::type();

		char *data = streamFile().ioBuffer(size);
		assert(data);
		IndexedIO::DataFlattenTraits<T>::flatten(x, data);

		node->m_entry = IndexedIO::Entry( node->m_entry.id(), IndexedIO::File, dataType, 0) ;

		node->m_idx->writeNodeData( node, data, size );
	}
	else
	{
		throw IOException( "StreamIndexedIO: Could not insert node '" + name.value() + "' into index" );
	}
}

template<typename T>
void StreamIndexedIO::rawWrite(const IndexedIO::EntryID &name, const T &x)
{
	writable(name);
	remove(name, false);

	Node* node = m_node->addChild( name );
	if (node)
	{
		unsigned long size = IndexedIO::DataSizeTraits<T>::size(x);
		IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T>::type();

		node->m_entry = IndexedIO::Entry( node->m_entry.id(), IndexedIO::File, dataType, 0) ;

		node->m_idx->writeNodeData( node, (char*)&x, size );
	}
	else
	{
		throw IOException( "StreamIndexedIO: Could not insert node '" + name.value() + "' into index" );
	}
}

template<typename T>
void StreamIndexedIO::read(const IndexedIO::EntryID &name, T *&x, unsigned long arrayLength) const
{
	assert( m_node );
	readable(name);

	Node* node = m_node->child( name );

	if (!node || node->m_entry.entryType() != IndexedIO::File)
	{
		throw IOException( "StreamIndexedIO: Entry not found '" + name.value() + "'" );
	}

	StreamIndexedIO::StreamFile &f = streamFile();
	Imf::Int64 size = node->m_size;
	{
		StreamFile::MutexLock lock( f.mutex() );
		char *data = f.ioBuffer(size);
		f.seekg( node->offset(), std::ios::beg );
		f.read( data, size );
		IndexedIO::DataFlattenTraits<T*>::unflatten( data, x, arrayLength );
	}
}

template<typename T>
void StreamIndexedIO::rawRead(const IndexedIO::EntryID &name, T *&x, unsigned long arrayLength) const
{
	assert( m_node );
	readable(name);

	Node* node = m_node->child( name );

	if (!node || node->m_entry.entryType() != IndexedIO::File)
	{
		throw IOException( "StreamIndexedIO: Entry not found '" + name.value() + "'" );
	}

	Imf::Int64 size = node->m_size;
	if (!x)
	{
		x = new T[arrayLength];
	}

	StreamIndexedIO::StreamFile &f = streamFile();
	{
		StreamFile::MutexLock lock( f.mutex() );
		f.seekg( node->offset(), std::ios::beg );
		f.read( (char*)x, size );
	}
}

template<typename T>
void StreamIndexedIO::read(const IndexedIO::EntryID &name, T &x) const
{
	assert( m_node );
	readable(name);

	Node* node = m_node->child( name );

	if (!node || node->m_entry.entryType() != IndexedIO::File)
	{
		throw IOException( "StreamIndexedIO: Entry not found '" + name.value() + "'" );
	}

	Imf::Int64 size = node->m_size;
	StreamIndexedIO::StreamFile &f = streamFile();
	{
		StreamFile::MutexLock lock( f.mutex() );
		char *data = f.ioBuffer(size);
		f.seekg( node->offset(), std::ios::beg );
		f.read( data, size );
		IndexedIO::DataFlattenTraits<T>::unflatten( data, x );
	}
}

template<typename T>
void StreamIndexedIO::rawRead(const IndexedIO::EntryID &name, T &x) const
{
	assert( m_node );
	readable(name);

	Node* node = m_node->child( name );

	if (!node || node->m_entry.entryType() != IndexedIO::File)
	{
		throw IOException( "StreamIndexedIO: Entry not found '" + name.value() + "'" );
	}

	Imf::Int64 size = node->m_size;
	StreamIndexedIO::StreamFile &f = streamFile();
	{
		StreamFile::MutexLock lock( f.mutex() );
		f.seekg( node->offset(), std::ios::beg );
		f.read( (char*)&x, size );
	}
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
