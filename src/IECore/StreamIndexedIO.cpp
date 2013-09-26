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
#define SUBINDEX_DIR			126

static const Imf::Int64 g_unversionedMagicNumber = 0x0B00B1E5;
static const Imf::Int64 g_versionedMagicNumber = 0xB00B1E50;

/// File format history:
/// Version 4: introduced hard links (automatic data deduplication), also ability to store InternedString data.
/// Version 5: introduced subindex as zipped data blocks (to reduce size of the main index). 
///            Hard links are represented as regular data nodes, that points to same data on file (no removal of data ever). 
///            Removed the linkCount field on the data nodes.
static const Imf::Int64 g_currentVersion = 5;

/// FileFormat ::= Data Index IndexOffset Version MagicNumber
/// Data ::= DataEntry*
/// Index ::= zip(StringCache NodeTree FreePages)

/// DataEntry ::= Stores data from nodes: 
///                [Data nodes] binary data indexed by DataOffset/DataSize and 
///                [Subindex]   SubIndexSize zip(NodeCount NodeTree*) indexed by SubIndexOffset.
/// SubIndexSize :: = unsigned int - number of bytes in the zipped subindex that follows

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
/// NodeCount ::= unsigned int ( number of child nodes in the directory - stored right after this node leading to recursive definition of a tree )
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

class BaseNode : public RefCounted
{
	public :

		// name of the node in the current directory
		IndexedIO::EntryID m_name;

		// directory or file
		virtual IndexedIO::EntryType entryType() const = 0;

};

IE_CORE_DECLAREPTR( BaseNode );

/// Class that represents a Data node
class DataNode : public BaseNode
{
	public :

		/// data fields from IndexedIO::Entry
		IndexedIO::DataType m_dataType;

		/// data fields from IndexedIO::Entry
		unsigned long m_arrayLength;

		/// The offset in the file to this node's data
		Imf::Int64 m_offset;

		/// The size of this node's data chunk within the file
		Imf::Int64 m_size;

		IndexedIO::EntryType entryType() const
		{
			return IndexedIO::File;
		}

};

/// A directory node within an index
/// It also represents subindex directory nodes by setting m_subindex at the root and all it's child nodes to true.
class StreamIndexedIO::Node : public BaseNode
{
	public:

		/// Directory nodes can save it's children to sub-indexes to free resources and reduce the size of the main index.
		/// Once saved to a subindex, they become read-only.
		enum SubIndexMode {
			NoSubIndex = 0,
			SavedSubIndex,
			LoadedSubIndex,
		};

		SubIndexMode m_subindex;

		/// The offset in the file to this node's subindex block if m_subindex is not NoSubIndex.
		Imf::Int64 m_offset;

		/// A shared pointer to the main file index
		StreamIndexedIO::Index* m_idx;

		/// A pointer to the parent node in the tree - will be NULL for the root node
		Node* m_parent;

		/// Pointers to this node's children (Node or DataNode)
		typedef std::map< IndexedIO::EntryID, BaseNodePtr> ChildMap;
		ChildMap m_children;

	public:

		/// Construct a new Node in the given index with the given numeric id
		Node(StreamIndexedIO::Index* index);

		virtual ~Node();

		void childNames( IndexedIO::EntryIDList &names ) const;
		void childNames( IndexedIO::EntryIDList &names, IndexedIO::EntryType ) const;

		const IndexedIO::EntryID &name() const;
		void path( IndexedIO::EntryIDList &result ) const;

		bool hasChild( const IndexedIO::EntryID &name ) const;

		/// Returns a Node or DataNode or NULL if not existent.
		BaseNode* child( const IndexedIO::EntryID &name ) const;
		// Returns the named child node or NULL if not existent.
		// If loadChildren is true, then it loads the subindex for the child nodes (if applicable).
		Node* child( const IndexedIO::EntryID &name, bool loadChildren ) const;
		DataNode* dataChild( const IndexedIO::EntryID &name ) const;

		Node* addChild( const IndexedIO::EntryID & childName );
		DataNode* addDataChild( const IndexedIO::EntryID & childName );

		void removeChild( const IndexedIO::EntryID &childName, bool throwException = true );

		IndexedIO::EntryType entryType() const
		{
			return IndexedIO::Directory;
		}

	protected:

		friend class StreamIndexedIO::Index;

		/// registers a child node in this node
		void registerChild( BaseNode* c );
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

		/// Allocate a new chunk of data of the requested size, returning its offset within the file
		Imf::Int64 allocate( Imf::Int64 sz );

		/// Deallocate a node's data from the file.
		void deallocate( DataNode* n );

		/// Queries the string cache
		StringCache &stringCache();

		StreamIndexedIO::StreamFile &streamFile() const;

		/// flushes index to the file
		void flush();

		/// Returns the offset after saving the data to file or the offset for a previouly saved data (with matching hash)
		/// \param prefixSize If true than it will prepend to the block, the size of it
		Imf::Int64 writeUniqueData( const char *data, unsigned int size, bool prefixSize = false );

		/// flushes the children of the given directory node to a subindex in the file
		void commitNodeToSubIndex( Node *n );

		/// read the subindex that contains the children of the given node
		void readNodeFromSubIndex( Node *n );

	protected:

		NodePtr m_root;

		Imf::Int64 m_version;

		bool m_hasChanged;

		Imf::Int64 m_offset;
		Imf::Int64 m_next;

		// only used on Version <= 4
		typedef std::vector< BaseNode* > IndexToNodeMap;
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

		struct FreePage
		{
			FreePage( Imf::Int64 offset, Imf::Int64 sz ) : m_offset(offset), m_size(sz) {}

			Imf::Int64 m_offset;
			Imf::Int64 m_size;

			FreePagesOffsetMap::iterator m_offsetIterator;
			FreePagesSizeMap::iterator m_sizeIterator;
		};

		void addFreePage( Imf::Int64 offset, Imf::Int64 sz );

		void deallocateWalk( BaseNode* n );

		/// Write the index to the file stream
		Imf::Int64 write();

		/// Write the node (and all child nodes) to a stream
		template < typename F >
		void writeNode( Node *n, F &f );

		/// Write the data node to a stream
		template < typename F >
		void writeDataNode( DataNode *n, F &f );

		template < typename F >
		void read( F &f );

		/// Read method used on previous file format versions up to version 4
		/// Returns a newly created Node or DataNode.
		template < typename F >
		BaseNode *readNodeV4( F &f );

		/// Replace the contents of this node with data read from a stream.
		/// Returns a newly created Node or DataNode.
		template < typename F >
		BaseNode *readNode( F &f );

		void recursiveSetSubIndex( Node *n );

};

///////////////////////////////////////////////
//
// StreamIndexedIO::Node (begin)
//
///////////////////////////////////////////////

StreamIndexedIO::Node::Node(Index* index) : BaseNode(), m_subindex(NoSubIndex), m_offset(0), m_idx(index), m_parent(0)
{
}

StreamIndexedIO::Node::~Node()
{
}

void StreamIndexedIO::Node::registerChild( BaseNode* c )
{
	if ( !c )
	{
		throw Exception("Invalid pointer for child node!!");
	}

	if ( c->entryType() == IndexedIO::Directory )
	{
		Node *childNode = static_cast< Node *>(c);
		if (childNode->m_parent)
		{
			throw IOException("StreamIndexedIO: Node already has parent!");
		}

		childNode->m_parent = this;
	}

	m_children.insert( std::map< IndexedIO::EntryID, BaseNodePtr >::value_type( c->m_name, c) );
}

bool StreamIndexedIO::Node::hasChild( const IndexedIO::EntryID &name ) const
{
	return m_children.find( name ) != m_children.end();
}

BaseNode* StreamIndexedIO::Node::child( const IndexedIO::EntryID &name ) const
{
	ChildMap::const_iterator cit = m_children.find( name );
	if (cit == m_children.end())
	{
		return 0;
	}
	return cit->second.get();
}

StreamIndexedIO::Node* StreamIndexedIO::Node::child( const IndexedIO::EntryID &name, bool loadChildren ) const
{
	Node *n = 0;
	BaseNode *p = child(name);
	if ( p )
	{
		if ( p->entryType() == IndexedIO::Directory )
		{
			n = static_cast< Node *>( p );
			
			if ( loadChildren && n->m_subindex )
			{
				m_idx->readNodeFromSubIndex( n );
			}
		}
	}
	return n;
}

DataNode* StreamIndexedIO::Node::dataChild( const IndexedIO::EntryID &name ) const
{
	DataNode *n = 0;
	BaseNode *p = child(name);
	if ( p )
	{
		if ( p->entryType() == IndexedIO::File )
		{
			n = static_cast< DataNode *>( p );
		}
	}
	return n;
}

StreamIndexedIO::Node* StreamIndexedIO::Node::addChild( const IndexedIO::EntryID &childName )
{
	if ( m_subindex )
	{
		throw Exception( "Cannot modify the file at current location! It was already committed to the file." );
	}

	if ( hasChild(childName) )
	{
		return 0;
	}

	Node* child = new Node(m_idx);
	if ( !child )
	{
		throw Exception( "Failed to allocate node!" );
	}
	child->m_name = childName;
	m_idx->m_stringCache.add( childName );

	registerChild( child );

	m_idx->m_hasChanged = true;

	return child;
}

DataNode* StreamIndexedIO::Node::addDataChild( const IndexedIO::EntryID &childName )
{
	if ( m_subindex )
	{
		throw Exception( "Cannot modify the file at current location! It was already committed to the file." );
	}

	if ( hasChild(childName) )
	{
		return 0;
	}

	DataNode* child = new DataNode;
	if ( !child )
	{
		throw Exception( "Failed to allocate node!" );
	}
	child->m_name = childName;
	child->m_dataType = IndexedIO::Invalid;
	child->m_arrayLength = 0;
	m_idx->m_stringCache.add( childName );

	registerChild( child );

	m_idx->m_hasChanged = true;

	return child;
}

void StreamIndexedIO::Node::path( IndexedIO::EntryIDList &result ) const
{
	if ( m_parent )
	{
		m_parent->path( result );
		result.push_back( m_name );
	}
}

const IndexedIO::EntryID &StreamIndexedIO::Node::name() const
{
	return m_name;
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
	
	bool typeIsDirectory = ( type == IndexedIO::Directory );

	for ( ChildMap::const_iterator cit = m_children.begin(); cit != m_children.end(); cit++ )
	{
		bool childIsDirectory = ( cit->second->entryType() == IndexedIO::Directory );
		if ( typeIsDirectory == childIsDirectory )
		{
			names.push_back( cit->first );
		}
	}
}

void StreamIndexedIO::Node::removeChild( const IndexedIO::EntryID &childName, bool throwException )
{
	ChildMap::iterator it = m_children.find( childName );
	if ( it == m_children.end() )
	{
		if (throwException)
		{
			throw IOException( "StreamIndexedIO:Node:removeChild: Entry not found '" + childName.value() + "'" );
		}
		return;
	}

	BaseNode *child = it->second.get();

	m_idx->deallocateWalk(child);

	m_children.erase( it );
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

StreamIndexedIO::Index::Index( StreamIndexedIO::StreamFilePtr stream ) : m_root(0), m_version(g_currentVersion), m_hasChanged(false), m_offset(0), m_next(0), m_stream(stream)
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
		m_root->m_name = IndexedIO::rootName;
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
BaseNode *StreamIndexedIO::Index::readNodeV4( F &f )
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

	BaseNode *result = 0;

	if ( entryType == IndexedIO::File )
	{
		DataNode *n = new DataNode;

		n->m_name = *id;
		n->m_dataType = dataType;
		n->m_arrayLength = static_cast<unsigned long>( arrayLength );

		if ( isLink )		// Only version 4
		{
			Imf::Int64 targetNodeId;
			// load Target Node ID in m_offset
			readLittleEndian( f,targetNodeId );
			n->m_offset = targetNodeId;
			// we cannot assure that the target node is already loaded, 
			/// so we set size to zero for now and after we set it after the whole index is loaded.
			n->m_size = 0;
		}
		else
		{
			readLittleEndian( f,n->m_offset );
			readLittleEndian( f,n->m_size );

			if ( m_version == 4 )
			{
				/// ignore link count data
				typedef unsigned short LinkCount;
				LinkCount linkCount;
				readLittleEndian(f,linkCount);
			}
		}
		result = n;
	}
	else // Directory
	{
		Node *n = new Node(this);
		n->m_name = *id;
		if ( m_version < 2 )
		{
			Imf::Int64 size;
			readLittleEndian( f, n->m_offset );
			readLittleEndian( f, size );
		}
		n->m_offset = 0;

		result = n;
	}

	if ( nodeId && parentId != Imath::limits<Imf::Int64>::max() )
	{
		Node* parent = 0;
		if ( parentId < m_indexToNodeMap.size() )
		{
			parent = static_cast< Node * >( m_indexToNodeMap[parentId] );
			if ( parent->entryType() != IndexedIO::Directory )
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
		m_indexToNodeMap.resize(nodeId+1, NULL);
	}
	m_indexToNodeMap[nodeId] = result;

	if ( m_version < 1 )
	{
		delete id;
	}

	return result;
}

template < typename F >
BaseNode *StreamIndexedIO::Index::readNode( F &f )
{
	char t;
	f.read( &t, sizeof(char) );

	IndexedIO::EntryType entryType = (IndexedIO::EntryType)t;
	StreamIndexedIO::Node::SubIndexMode subindex = StreamIndexedIO::Node::NoSubIndex;

	if ( t == SUBINDEX_DIR )
	{
		entryType = IndexedIO::Directory;
		subindex = StreamIndexedIO::Node::SavedSubIndex;
	}

	Imf::Int64 stringId;
	readLittleEndian(f,stringId);

	if ( entryType == IndexedIO::File )
	{
		IndexedIO::DataType dataType = IndexedIO::Invalid;
		Imf::Int64 arrayLength = 0;
		f.read( &t, sizeof(char) );
		dataType = (IndexedIO::DataType)t;
	
		if ( IndexedIO::Entry::isArray( dataType ) )
		{
			readLittleEndian( f,arrayLength );
		}

		DataNode *n = new DataNode;
		n->m_name = m_stringCache.findById( stringId );
		n->m_dataType = dataType;
		n->m_arrayLength = static_cast<unsigned long>( arrayLength );
		readLittleEndian( f,n->m_offset );
		readLittleEndian( f,n->m_size );
		return n;
	}
	else if ( entryType == IndexedIO::Directory )
	{
		Node *n = new Node( this );
		n->m_name = m_stringCache.findById( stringId );
		n->m_subindex = subindex;

		if ( subindex )
		{
			readLittleEndian( f,n->m_offset );
		}
		else
		{
			n->m_offset = 0;

			unsigned int nodeCount = 0;
			readLittleEndian( f, nodeCount );

			for ( unsigned int c = 0; c < nodeCount; c++ )
			{
				BaseNode *child = readNode( f );
				n->registerChild( child );
			}
		}
		return n;
	}
	else
	{
		throw IOException( "Invalid EntryType!" );
	}
}

template < typename F >
void StreamIndexedIO::Index::read( F &f )
{
	if (m_version >= 1)
	{
		m_stringCache = StringCache( f );
	}

	if ( m_version >= 5 )
	{
		/// current file format reading
		m_root = static_cast< Node *>( readNode( f ) );

		if ( m_root->entryType() != IndexedIO::Directory)
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
		m_root = static_cast< Node *>( m_indexToNodeMap[0] );

		if ( m_root->entryType() != IndexedIO::Directory)
		{
			throw Exception( "StreamIndexedIO::Index::read - Root node is not a directory!!" );
		}

		if ( m_version == 4 )
		{
			// In Version 4, symlinks have to get the Entry information from their target nodes.
			for (IndexToNodeMap::const_iterator it = m_indexToNodeMap.begin(); it != m_indexToNodeMap.end(); it++ )
			{
				DataNode *n = static_cast< DataNode *>(*it);
				if ( !n || n->entryType() != IndexedIO::File )
				{
					continue;
				}
	
				if ( !n->m_size )
				{
					Imf::Int64 targetNodeId = n->m_offset;
					if ( targetNodeId >= m_indexToNodeMap.size() || !m_indexToNodeMap[targetNodeId] )
					{
						throw IOException("StreamIndexedIO: targetNodeId not found");
					}
					DataNode* targetNode = static_cast< DataNode *>(m_indexToNodeMap[targetNodeId]);
					if ( targetNode->entryType() != IndexedIO::File )
					{
						throw IOException("StreamIndexedIO: targetNode if not of type File!" );
					}
					n->m_dataType = targetNode->m_dataType;
					n->m_arrayLength = targetNode->m_arrayLength;;
					n->m_offset = targetNode->m_offset;
					n->m_size = targetNode->m_size;
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

template < typename F >
void StreamIndexedIO::Index::writeDataNode( DataNode *node, F &f )
{
	char t = node->entryType();
	f.write( &t, sizeof(char) );

	Imf::Int64 id = m_stringCache.find( node->m_name );
	writeLittleEndian( f, id );

	t = node->m_dataType;
	f.write( &t, sizeof(char) );

	if ( IndexedIO::Entry::isArray( node->m_dataType ) )
	{
		writeLittleEndian<F,Imf::Int64>( f, node->m_arrayLength );
	}

	writeLittleEndian(f, node->m_offset);
	writeLittleEndian(f, node->m_size);
}

template < typename F >
void StreamIndexedIO::Index::writeNode( Node *node, F &f )
{
	char t = ( node->m_subindex ? SUBINDEX_DIR : IndexedIO::Directory );
	f.write( &t, sizeof(char) );

	Imf::Int64 id = m_stringCache.find( node->m_name );
	writeLittleEndian( f, id );

	if ( node->m_subindex )
	{
		writeLittleEndian(f, node->m_offset);
	}
	else
	{
		unsigned int nodeCount = node->m_children.size();
		writeLittleEndian(f, nodeCount);
		for (Node::ChildMap::const_iterator it = node->m_children.begin(); it != node->m_children.end(); ++it)
		{
			BaseNode *p = it->second.get();
			if ( p->entryType() == IndexedIO::File )
			{
				DataNode *child = static_cast< DataNode * >(p);
				writeDataNode( child, f );
			}
			else
			{
				Node *child = static_cast< Node * >(p);
				writeNode( child, f );
			}
		}
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

	writeNode( m_root, compressingStream );

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

void StreamIndexedIO::Index::deallocate( DataNode* n )
{
	assert(n);
	assert(n->entryType() == IndexedIO::File);

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

Imf::Int64 StreamIndexedIO::Index::writeUniqueData( const char *data, unsigned int size, bool prefixSize )
{
	m_hasChanged = true;

	/// Find next writable location
	Imf::Int64 loc;

	// compute hash for the data
	MurmurHash hash;
	hash.append( data, size );

	unsigned int totalSize = size;

	if ( prefixSize )
	{
		totalSize += sizeof( unsigned int );
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
		writeLittleEndian( *m_stream, size );
	}

	/// Write data
	m_stream->write( data, size );

	return loc;
}

void StreamIndexedIO::Index::deallocateWalk( BaseNode* n )
{
	assert(n);

	if ( n->entryType() == IndexedIO::Directory )
	{
		Node *nn = static_cast< Node *>(n);

		for (Node::ChildMap::const_iterator it = nn->m_children.begin(); it != nn->m_children.end(); ++it)
		{
			deallocateWalk( it->second.get() );
		}

		nn->m_children.clear();
	}
	else
	{
		// We don't deallocate data node blocks because they could be referred by other nodes.
		// As a result, editing files will usually increase file size.
	}

}

void StreamIndexedIO::Index::recursiveSetSubIndex( Node *n )
{
	n->m_subindex = Node::SavedSubIndex;

	for (Node::ChildMap::const_iterator it = n->m_children.begin(); it != n->m_children.end(); ++it)
	{
		Node *childNode = static_cast<Node*>(it->second.get());

		if ( childNode->entryType() != IndexedIO::Directory )
			continue;

		if (childNode && childNode->m_subindex == StreamIndexedIO::Node::NoSubIndex )
		{
			recursiveSetSubIndex( childNode );
		}
	}
}

void StreamIndexedIO::Index::commitNodeToSubIndex( Node *n )
{
	if (!n)
	{
		return;
	}

	if ( n->m_subindex == Node::NoSubIndex )
	{
		MemoryStreamSink sink;
		io::filtering_ostream compressingStream;
		compressingStream.push( io::gzip_compressor() );
		compressingStream.push( sink );
		assert( compressingStream.is_complete() );

		unsigned int nodeCount = n->m_children.size();

		writeLittleEndian( compressingStream, nodeCount );

		for (Node::ChildMap::const_iterator it = n->m_children.begin(); it != n->m_children.end(); ++it)
		{
			BaseNode *p = it->second.get();
			if ( p->entryType() == IndexedIO::File )
			{
				DataNode *childNode = static_cast< DataNode *>(p);
				writeDataNode( childNode, compressingStream );
			}
			else
			{
				Node *childNode = static_cast< Node *>(p);

				writeNode( childNode, compressingStream );
			}
		}

		compressingStream.pop();
		compressingStream.pop();

		char *data=0;
		std::streamsize sz;
		sink.get( data, sz );
		unsigned int subindexSize = sz;

		n->m_offset = writeUniqueData( data, subindexSize, true );

		// set all child nodes as committed to a subindex (this also makes them read-only)
		recursiveSetSubIndex( n );

		// remove all children from this node ( freeing some memory if there's no external references)
		n->m_children.clear();
	}
}

void StreamIndexedIO::Index::readNodeFromSubIndex( Node *n )
{
	if ( n->m_subindex == Node::NoSubIndex )
	{
		return;
	}

	/// guarantees thread safe access to the file and also to the m_subindex variable
	StreamFile::MutexLock lock( m_stream->mutex() );

	if ( n->m_subindex == Node::LoadedSubIndex )
	{
		return;
	}
	
	m_stream->seekg( n->m_offset, std::ios::beg );

	unsigned int subindexSize = 0;
	readLittleEndian( *m_stream, subindexSize );

	char *data = m_stream->ioBuffer(subindexSize);
	m_stream->read( data, subindexSize );

	io::filtering_istream decompressingStream;
	MemoryStreamSource source( data, subindexSize, false );
	decompressingStream.push( io::gzip_decompressor() );
	decompressingStream.push( source );
	assert( decompressingStream.is_complete() );

	unsigned int nodeCount = 0;

	readLittleEndian( decompressingStream, nodeCount );
	
	for ( unsigned int i = 0; i < nodeCount; i++ )
	{
		BaseNode *child = readNode( decompressingStream );
		n->registerChild( child );
	}

	/// mark the node as loaded from subindex
	n->m_subindex = Node::LoadedSubIndex;
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

	// \todo Currently in Append mode, the nodes lazily loaded will not be editable. 
	// In order to fully support it, we should probably read all indexes in memory, 
	// deallocate their data blocks, mark Index as changed and force saving all of 
	// the nodes in the main index, or commit them backwardly.
}

StreamIndexedIO::~StreamIndexedIO()
{
	if ( m_node.get() )
	{
		// remove our reference to the index, it's destructor triggers the 
		// flush to disk.
		m_node->m_idx->removeRef();

		// \todo Free some memory for committed directories in read-only mode:
		/// Should we deallocate the tree when the StreamIndexedIO for the root dies?
		/// m_node = 0;
		/// Lock read mutex
		/// if ( m_node->refCount() == 1 )
		///    only parent pointing to it...
		///    if ( m_node->n_subindex && m_node->n_offset )
		//         // check if all the children has refCount == 1
		//         if ( childrenRefCount is one )
		//             m_node->children.clear()
		//             m_node->m_subindex = Node::SavedSubIndex
	}
}

void StreamIndexedIO::setRoot( const IndexedIO::EntryIDList &root )
{
	IndexedIO::EntryIDList::const_iterator t = root.begin();
	for ( ; t != root.end(); t++ )
	{
		NodePtr childNode = m_node->child( *t, true );
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
	Node *childNode = m_node->child( name, true );
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
	Node *childNode = m_node->child( name, true );
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

	if ( m_node->m_subindex )
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

	if ( m_node->m_subindex )
	{
		throw Exception( "Cannot modify the file at current location! It was already committed to the file." );
	}

	m_node->removeChild( name, throwIfNonExistent );
}

IndexedIO::Entry StreamIndexedIO::entry(const IndexedIO::EntryID &name) const
{
	assert( m_node );
	readable(name);

	BaseNode* node = m_node->child( name );

	if (!node)
	{
		throw IOException( "StreamIndexedIO: Entry not found '" + name.value() + "'" );
	}

	if ( node->entryType() == IndexedIO::File )
	{
		DataNode *dn = static_cast< DataNode * >(node);
		return IndexedIO::Entry( dn->m_name, IndexedIO::File, dn->m_dataType, dn->m_arrayLength );
	}

	return IndexedIO::Entry( node->m_name, IndexedIO::Directory, IndexedIO::Invalid, 0 );
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

		Node* childNode = node->child( name, true );
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

void StreamIndexedIO::commit()
{
	m_node->m_idx->commitNodeToSubIndex( m_node );
}

void StreamIndexedIO::write(const IndexedIO::EntryID &name, const InternedString *x, unsigned long arrayLength)
{
	writable(name);
	remove(name, false);

	DataNode* node = m_node->addDataChild( name );
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

	Index *index = m_node->m_idx;

	StringCache &stringCache = index->stringCache();

	for ( unsigned long i = 0; i < arrayLength; i++ )
	{
		ids[i] = stringCache.find( x[i], false /* create entry if missing */ );
	}

	IndexedIO::DataFlattenTraits<Imf::Int64*>::flatten(constIds, arrayLength, data);

	node->m_dataType = dataType;
	node->m_arrayLength = arrayLength;
	node->m_offset = index->writeUniqueData( data, size );
	node->m_size = size;

	delete [] ids;
}

void StreamIndexedIO::read(const IndexedIO::EntryID &name, InternedString *&x, unsigned long arrayLength) const
{
	assert( m_node );
	readable(name);

	DataNode* node = m_node->dataChild( name );

	if (!node)
	{
		throw IOException( "StreamIndexedIO: Entry not found '" + name.value() + "'" );
	}

	Imf::Int64 *ids = new Imf::Int64[arrayLength];
	Imf::Int64 size = node->m_size;
	StreamIndexedIO::StreamFile &f = streamFile();
	{
		StreamFile::MutexLock lock( f.mutex() );
		f.seekg( node->m_offset, std::ios::beg );

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

	DataNode* node = m_node->addDataChild( name );
	if (node)
	{
		unsigned long size = IndexedIO::DataSizeTraits<T*>::size(x, arrayLength);
		IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T*>::type();

		char *data = streamFile().ioBuffer(size);
		assert(data);
		IndexedIO::DataFlattenTraits<T*>::flatten(x, arrayLength, data);

		node->m_dataType = dataType;
		node->m_arrayLength = arrayLength;
		node->m_offset = m_node->m_idx->writeUniqueData( data, size );
		node->m_size = size;
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

	DataNode* node = m_node->addDataChild( name );
	if (node)
	{
		unsigned long size = IndexedIO::DataSizeTraits<T*>::size(x, arrayLength);
		IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T*>::type();

		node->m_dataType = dataType;
		node->m_arrayLength = arrayLength;
		node->m_offset = m_node->m_idx->writeUniqueData( (char*)x, size );
		node->m_size = size;
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

	DataNode* node = m_node->addDataChild( name );
	if (node)
	{
		unsigned long size = IndexedIO::DataSizeTraits<T>::size(x);
		IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T>::type();

		char *data = streamFile().ioBuffer(size);
		assert(data);
		IndexedIO::DataFlattenTraits<T>::flatten(x, data);

		node->m_dataType = dataType;
		node->m_arrayLength = 0;
		node->m_offset = m_node->m_idx->writeUniqueData( data, size );
		node->m_size = size;
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

	DataNode* node = m_node->addDataChild( name );
	if (node)
	{
		unsigned long size = IndexedIO::DataSizeTraits<T>::size(x);
		IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T>::type();

		node->m_dataType = dataType;
		node->m_arrayLength = 0;
		node->m_offset = m_node->m_idx->writeUniqueData( (char*)&x, size );
		node->m_size = size;
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

	DataNode* node = m_node->dataChild( name );

	if (!node)
	{
		throw IOException( "StreamIndexedIO: Entry not found '" + name.value() + "'" );
	}

	StreamIndexedIO::StreamFile &f = streamFile();
	Imf::Int64 size = node->m_size;
	{
		StreamFile::MutexLock lock( f.mutex() );
		char *data = f.ioBuffer(size);
		f.seekg( node->m_offset, std::ios::beg );
		f.read( data, size );
		IndexedIO::DataFlattenTraits<T*>::unflatten( data, x, arrayLength );
	}
}

template<typename T>
void StreamIndexedIO::rawRead(const IndexedIO::EntryID &name, T *&x, unsigned long arrayLength) const
{
	assert( m_node );
	readable(name);

	DataNode* node = m_node->dataChild( name );

	if (!node)
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
		f.seekg( node->m_offset, std::ios::beg );
		f.read( (char*)x, size );
	}
}

template<typename T>
void StreamIndexedIO::read(const IndexedIO::EntryID &name, T &x) const
{
	assert( m_node );
	readable(name);

	DataNode* node = m_node->dataChild( name );

	if (!node)
	{
		throw IOException( "StreamIndexedIO: Entry not found '" + name.value() + "'" );
	}

	Imf::Int64 size = node->m_size;
	StreamIndexedIO::StreamFile &f = streamFile();
	{
		StreamFile::MutexLock lock( f.mutex() );
		char *data = f.ioBuffer(size);
		f.seekg( node->m_offset, std::ios::beg );
		f.read( data, size );
		IndexedIO::DataFlattenTraits<T>::unflatten( data, x );
	}
}

template<typename T>
void StreamIndexedIO::rawRead(const IndexedIO::EntryID &name, T &x) const
{
	assert( m_node );
	readable(name);

	DataNode* node = m_node->dataChild( name );

	if (!node)
	{
		throw IOException( "StreamIndexedIO: Entry not found '" + name.value() + "'" );
	}

	Imf::Int64 size = node->m_size;
	StreamIndexedIO::StreamFile &f = streamFile();
	{
		StreamFile::MutexLock lock( f.mutex() );
		f.seekg( node->m_offset, std::ios::beg );
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
