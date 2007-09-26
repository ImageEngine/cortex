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

#include <algorithm>
#include <list>
#include <iostream>
#include <cassert>
#include <map>
#include <set>
#include <boost/tokenizer.hpp>
#include <boost/filesystem/operations.hpp> 
#include <boost/iostreams/device/file.hpp> 

#include "IECore/ByteOrder.h"

#include "IECore/FileIndexedIO.h"



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

template<typename T>
void writeLittleEndian( io::file &f, T n )
{
	const T nl = asLittleEndian<>(n);
	f.write( (const char*) &nl, sizeof(T) );
}

template<typename T>
void readLittleEndian( io::file &f, T &n )
{
	static bool g_bigEndian = bigEndian(); // Cache value to prevent unnecessary function calls
	f.read( (char*) &n, sizeof(T) );
	
	if (g_bigEndian)
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
	
		StringCache()
		{
			m_prevId = 0;			
		}
		
		StringCache( io::file &f ) : m_prevId(0)
		{
			Imf::Int64 sz;
			readLittleEndian(f, sz);
			
			for (Imf::Int64 i = 0; i < sz; ++i)
			{
				std::string s;
				read(f, s);
				
				Imf::Int64 id;
				readLittleEndian<Imf::Int64>( f, id );
				
				m_prevId = std::max( id, m_prevId );
				
				m_stringToIdMap[s] = id;
				m_idToStringMap[id] = s;
			}
		}
		
		void write( io::file &f ) const
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
		
		Imf::Int64 find( const std::string &s, bool errIfNotFound = true )
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
				assert( m_stringToIdMap.find(s) != m_stringToIdMap.end() );
				m_idToStringMap[id] = s;
				
				return id;
			}
			else
			{
				return it->second;
			}
		}
		
		const std::string &find( const Imf::Int64 &id ) const
		{
			IdToStringMap::const_iterator it = m_idToStringMap.find( id );
			assert( it != m_idToStringMap.end() );
			
			return it->second;			
		}
		
		void add( const std::string &s )
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
	
		void write( io::file &f, const std::string &s ) const
		{
			Imf::Int64 sz = s.size();
			writeLittleEndian<Imf::Int64>( f, sz );
			
			/// Does not include null terminator
			f.write( s.c_str(), sz * sizeof(char) );			
		}
		
		void read( io::file &f, std::string &s ) const
		{		
			Imf::Int64 sz;	
			readLittleEndian<Imf::Int64>( f, sz );
			
			char *buf = new char[sz + 1];
			f.read( buf, sz*sizeof(char));
			
			buf[sz] = '\0';
			s = buf;
			delete[] buf;			
		}
	
		Imf::Int64 m_prevId;
		
		typedef std::map< std::string, Imf::Int64 > StringToIdMap;
		typedef std::map< Imf::Int64, std::string > IdToStringMap;
		
		StringToIdMap m_stringToIdMap;
		IdToStringMap m_idToStringMap;
		
};

/// A tree to represent nodes in a filesystem, along with their locations in a file.
class FileIndexedIO::Index : public RefCounted
{
	public:
	
		friend class Node;
	
		typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
	
		/// Construct an empty index
		Index();
		
		static bool canRead( const std::string &path );
		
		/// Construct an index from reading a file stream.
		Index( io::file &f );
		virtual ~Index();
			
		NodePtr m_root;	
		
		/// Returns where or not the index has changed since the last time it was written
		bool hasChanged() const;
		
		/// Attempts to find a path in the index. Will seek out the closest node in the hierarchy, and return
		/// whether or not this node is an exact match for the query.				
		bool find( const IndexedIOPath &p, NodePtr &nearest, NodePtr topNode ) const;
				
		/// Remove a node, and all its subnodes from the index		
		void remove( NodePtr n );
	
		/// Insert a new entry into the index, returning the node which stores it
		NodePtr insert( NodePtr parentNode, IndexedIO::Entry e );

		/// Write the index to a file stream
		void write( io::file & f );
	
		/// Allocate a new chunk of data of the requested size, returning its offset within the file
		Imf::Int64 allocate( Imf::Int64 sz );
	
		/// Deallocate a node's data from the file. 
		void deallocate( NodePtr n );
	
		/// Return the total number of nodes in the index.
		Imf::Int64 nodeCount();
					
	protected:
	
		static const Imf::Int64 g_unversionedMagicNumber = 0x0B00B1E5;
		static const Imf::Int64 g_versionedMagicNumber = 0xB00B1E50;
		
		static const Imf::Int64 g_currentVersion = 1;
		
		Imf::Int64 m_version;		
	
		bool m_hasChanged;
		
		Imf::Int64 m_offset;
		Imf::Int64 m_next;
	
		Imf::Int64 m_prevId;
		
		Imf::Int64 makeId();
				
		typedef std::map< unsigned long, Node* > IndexToNodeMap;
		typedef std::map< Node*, unsigned long > NodeToIndexMap;
	
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
		
		void write( io::file &f, Node* n );
	
		void readNode( io::file &f );
	
		Imf::Int64 nodeCount( Node* n );
							
};

/// A single node within an index
class FileIndexedIO::Node : public RefCounted
{	
	public:
	
		typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
		typedef std::vector<std::string> PathParts;	
									
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
		typedef std::map< std::string, NodePtr> ChildMap;
		ChildMap m_children;
	
		/// Construct a new Node in the given index with the given numeric id
		Node(Index* idx, Imf::Int64 id);
	
		virtual ~Node();
				
		/// Add	a child node
		void addChild( NodePtr c );
	
		/// Write this node to a stream
		void write( io::file &f );

		/// Replace the contents of this node with data read from a stream
		void read( io::file &f );
		
		/// Traverse through this node and down its children, removing the front 
		/// of the list of 'parts' every time we descend through a match.
		bool find( Tokenizer::iterator &parts, Tokenizer::iterator end, NodePtr &nearest, NodePtr topNode ) const;
	
		/// As above, but just traverse through the children.
		bool findInChildren( Tokenizer::iterator &parts, Tokenizer::iterator end, NodePtr &nearest, NodePtr topNode ) const;
		
		NodePtr insert( Tokenizer::iterator &parts, Tokenizer::iterator end );
		
	protected:
		
		Index *m_idx;
};

bool FileIndexedIO::Index::canRead( const std::string &path )
{
	io::file f( path, std::ios::in | std::ios::binary, std::ios::in);
	
	if (! f.is_open() )
	{
		return false;
	}
		
	f.seek( 0, std::ios::end, std::ios::in );
	Imf::Int64 end = f.seek( 0, std::ios::cur, std::ios::in );
	
	f.seek( end-1*sizeof(Imf::Int64), std::ios::beg, std::ios::in );
	
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
	
bool FileIndexedIO::Index::find( const IndexedIOPath &p, NodePtr &nearest, NodePtr topNode) const
{	
	Tokenizer tokens(p.fullPath(), boost::char_separator<char>("/"));	
	Tokenizer::iterator t = tokens.begin();
	nearest = m_root;
	
	return m_root->findInChildren( t, tokens.end(), nearest, topNode );
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

void FileIndexedIO::Index::remove( NodePtr n )
{
	assert(n);
	
	deallocateWalk(n.get());
	
	if (n->m_parent)
	{
		n->m_parent->m_children.erase( n->m_entry.id() );
	}
}

FileIndexedIO::NodePtr FileIndexedIO::Index::insert( NodePtr parent, IndexedIO::Entry e )
{		
	Imf::Int64 newId = makeId();	
	NodePtr child = new Node(this, newId);
	
	m_indexToNodeMap[newId] = child.get();
	m_nodeToIndexMap[child.get()] = newId;
		
	child->m_entry = e;
	
	m_stringCache.add( e.id() );
	
	parent->addChild( child );
	
	m_hasChanged = true;
	
	return child;
}


FileIndexedIO::NodePtr FileIndexedIO::Node::insert( Tokenizer::iterator &parts, Tokenizer::iterator end )
{
	if (parts == end)
	{
		return this;
	}
	
	NodePtr child = m_idx->insert( this, IndexedIO::Entry(*parts, IndexedIO::Directory, IndexedIO::Invalid, 0) );
	
	return child->insert( ++parts, end );
}

FileIndexedIO::Index::Index() : m_root(0), m_prevId(0)
{
	m_version = g_currentVersion;
	
	m_root = new Node( this, 0 );
	
	m_indexToNodeMap[0] = m_root.get();
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
			
FileIndexedIO::Index::Index( io::file &f ) : m_prevId(0)
{
	m_hasChanged = false;

	f.seek( 0, std::ios::end, std::ios::in );
	Imf::Int64 end = f.seek( 0, std::ios::cur, std::ios::in );
	
	f.seek( end-1*sizeof(Imf::Int64), std::ios::beg, std::ios::in );
	
	Imf::Int64 magicNumber;
	readLittleEndian<Imf::Int64>( f, magicNumber );
	
	if ( magicNumber == g_versionedMagicNumber )
	{
		f.seek( end-3*sizeof(Imf::Int64), std::ios::beg, std::ios::in );
		readLittleEndian<Imf::Int64>( f, m_offset );
		readLittleEndian<Imf::Int64>( f, m_version );						
	}
	else if (magicNumber == g_unversionedMagicNumber )
	{
		m_version = 0;
		
		f.seek( end-2*sizeof(Imf::Int64), std::ios::beg, std::ios::in );
		readLittleEndian<Imf::Int64>( f, m_offset );
	}
	else
	{	
		throw IOException("Not a FileIndexedIO file");
	}
				
	f.seek( m_offset, std::ios::beg, std::ios::in );
	
	if (m_version >= 1)
	{
		m_stringCache = StringCache( f );
	}
	
	Imf::Int64 numNodes;		
	readLittleEndian<Imf::Int64>( f, numNodes );

	for (Imf::Int64 i = 0; i < numNodes; i++)
	{
		readNode( f );
	}
	Imf::Int64 numFreePages;		
	readLittleEndian<Imf::Int64>( f, numFreePages );
	

	for (Imf::Int64 i = 0; i < numFreePages; i++)
	{
		Imf::Int64 offset, sz;
		readLittleEndian<Imf::Int64>( f, offset );
		readLittleEndian<Imf::Int64>( f, sz );
		
		addFreePage( offset, sz );
	}
	
	m_next = m_offset;
}

void FileIndexedIO::Index::write( io::file & f )
{
	/// Write index at end
	std::streampos indexStart = m_next;
	
	f.seek( m_next, std::ios_base::beg, std::ios_base::out );
	
	m_offset = indexStart;
	
	m_stringCache.write( f );
	
	
	Imf::Int64 numNodes = nodeCount();
	
	writeLittleEndian<Imf::Int64>(f, numNodes);
	
	write( f, m_root.get() );

	assert( m_freePagesOffset.size() == m_freePagesSize.size() );			
	Imf::Int64 numFreePages = m_freePagesSize.size();

	// Write out number of free "pages"		
	writeLittleEndian<Imf::Int64>(f, numFreePages);
			
	/// Write out each free page
	for ( FreePagesSizeMap::const_iterator it = m_freePagesSize.begin(); it != m_freePagesSize.end(); ++it)
	{
		writeLittleEndian<Imf::Int64>(f, it->second->m_offset);
		writeLittleEndian<Imf::Int64>(f, it->second->m_size);
	}
	
	writeLittleEndian<Imf::Int64>( f, m_offset );
	writeLittleEndian<Imf::Int64>( f, m_version );		
	writeLittleEndian<Imf::Int64>( f, g_versionedMagicNumber );
	
	m_hasChanged = false;
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

void FileIndexedIO::Index::deallocate( NodePtr n )
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

void FileIndexedIO::Index::write( io::file & f, Node* n )
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

void FileIndexedIO::Index::readNode( io::file &f )
{
	NodePtr n = new Node( this, 0 );
			
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

FileIndexedIO::Node::Node(Index* idx, Imf::Int64 id) : RefCounted()
{
	m_parent = 0;
	
	if (id != 0)
	{
		assert(idx->m_indexToNodeMap.find(id) == idx->m_indexToNodeMap.end());
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
			
void FileIndexedIO::Node::addChild( NodePtr c ) 
{
	if (c->m_parent)
	{
		throw IOException("Node already has parent!");
	}
		
#ifndef NDEBUG			
	/// Make sure we never try to add the same child twice
	ChildMap::const_iterator cit = m_children.find( c->m_entry.id() );
	assert (cit == m_children.end());
#endif			
					
	c->m_parent = this;
	m_children.insert( std::map< std::string, NodePtr >::value_type( c->m_entry.id(), c) );
}

void FileIndexedIO::Node::write( io::file &f )
{
	char t = m_entry.entryType();			
	f.write( &t, sizeof(char) );
	
	Imf::Int64 id = m_idx->m_stringCache.find( m_entry.id() );

	writeLittleEndian<Imf::Int64>( f, id );
	
	t = m_entry.m_dataType;			
	f.write( &t, sizeof(char) );
	
	writeLittleEndian<Imf::Int64>(f, m_entry.m_arrayLength );
								
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
	
	writeLittleEndian<Imf::Int64>(f, m_offset);
	writeLittleEndian<Imf::Int64>(f, m_size);
}

void FileIndexedIO::Node::read( io::file &f )
{
	char t;
	
	f.read( &t, sizeof(char) );		
	m_entry.m_entryType = (IndexedIO::EntryType)t;

	if (m_idx->m_version >= 1)
	{
		Imf::Int64 stringId;
		
		readLittleEndian<Imf::Int64>(f, stringId);		
	
		m_entry.m_ID = m_idx->m_stringCache.find( stringId );		
	}
	else
	{
		Imf::Int64 entrySize;
		readLittleEndian<Imf::Int64>( f, entrySize );
		char *s = new char[entrySize+1];
		f.read( s, entrySize );
		s[entrySize] = '\0';

		m_entry.m_ID = s;
		delete[] s;
	}
		
	f.read( &t, sizeof(char) );		
	m_entry.m_dataType = (IndexedIO::DataType)t;

	Imf::Int64 arrayLength;
	readLittleEndian<Imf::Int64>( f, arrayLength );
	m_entry.m_arrayLength = arrayLength;			
		
	readLittleEndian<Imf::Int64>(f, m_id );

	m_idx->m_indexToNodeMap[m_id] = this;
	m_idx->m_nodeToIndexMap[this] = m_id;
	
	Imf::Int64 parentId;
	readLittleEndian<Imf::Int64>(f, parentId );
	
	m_idx->m_prevId = std::max( m_idx->m_prevId, parentId );
	m_idx->m_prevId = std::max( m_idx->m_prevId, m_id );			

	Index::IndexToNodeMap::iterator it = m_idx->m_indexToNodeMap.find( parentId );
	if (it == m_idx->m_indexToNodeMap.end())
	{		
		throw IOException("parentId not found");
	}
	
	NodePtr parent = it->second ;
	if (m_id && parent)
	{
		parent->addChild(this);
	} 
	else if (m_id != 0)
	{
		throw IOException("Non-root node has no parent");
	}
	
	readLittleEndian<Imf::Int64>(f, m_offset );
	readLittleEndian<Imf::Int64>(f, m_size );	
}

bool FileIndexedIO::Node::find( Tokenizer::iterator &parts, Tokenizer::iterator end, NodePtr &nearest, NodePtr topNode ) const
{
	assert( nearest );
	
	if (parts == end)
	{
		return true;
	}
	else 
	{
		if (*parts == m_entry.id())
		{			
			nearest = const_cast<Node*>(this);	
			assert( nearest );

			return findInChildren( ++parts, end, nearest, topNode );		
		}
		else if (*parts == ".")
		{		
			return find( ++parts, end, nearest, topNode );			
		}
		else if (*parts == "..")
		{
			if (m_parent && this != topNode.get() )
			{
				nearest = m_parent;	
				return m_parent->find( ++parts, end, nearest, topNode );	
			}	
			return find( ++parts, end, nearest, topNode );
		}
		else
		{
			return false;
		}
	}
}

bool FileIndexedIO::Node::findInChildren( Tokenizer::iterator &parts, Tokenizer::iterator end, NodePtr &nearest, NodePtr topNode ) const
{	
	assert( nearest );
	
	if (parts == end)
	{
		return true;
	}
	
	if (*parts == ".")
	{		
		return find( ++parts, end, nearest, topNode );			
	}
	else if (*parts == "..")
	{
		if (m_parent && this != topNode.get() )
		{
			nearest = m_parent;	
			return m_parent->find( ++parts, end, nearest, topNode );	
		}
		return find( ++parts, end, nearest, topNode );
	}
	
	ChildMap::const_iterator cit = m_children.find( *parts );
	
	if (cit != m_children.end())
	{
		return cit->second->find( parts, end, nearest, topNode );
	}
	
	return false;			
}	

class FileIndexedIO::IndexedFile : public RefCounted
{
	public:
		io::file *m_file;
			
		IndexedFile( const std::string &filename, IndexedIO::OpenMode mode );
		virtual ~IndexedFile();
			
		/// Obtain the index for this file		
		IndexPtr index() const;
		
		/// Seek to a particular node within the file for reading
		void seekg( NodePtr node );		
				
		/// Write some data to the file. Its position is automatically allocated within the file, and the node
		/// is updated to record this offset along with its size.
		void write(NodePtr node, const char *data, Imf::Int64 size);
		
	protected:
	
		IndexPtr m_index;
};
			
FileIndexedIO::IndexedFile::IndexedFile( const std::string &filename, IndexedIO::OpenMode mode )
{
	if (mode & IndexedIO::Write)
	{
		m_file = new io::file(filename, std::ios::trunc | std::ios::binary);
		
		if (! m_file->is_open() )
		{
			throw IOException(filename);
		}
		
		m_index = new Index();
	}		
	else if (mode & IndexedIO::Append)
	{
		if (!fs::exists( filename.c_str() ) )
		{
			/// Create new file
			m_file = new io::file(filename.c_str(), std::ios::trunc | std::ios::binary);
		
			if (! m_file->is_open() )
			{
				throw IOException(filename);
			}
		
			m_index = new Index();
		}
		else
		{		
			/// Read existing file
			m_file = new io::file(filename.c_str(), std::ios::binary);
		
			if (! m_file->is_open() )
			{
				throw IOException(filename);
			}

			/// Read index		
			try 
			{
				m_index = new Index(*m_file);
			} 
			catch ( Exception &e )
			{
				throw;
			}
			catch (...)
			{
				throw IOException(filename);
			}
		}
	}
	else
	{
		assert( mode & IndexedIO::Read );
		m_file = new io::file(filename.c_str(), std::ios::binary, std::ios::in);
		
		if (! m_file->is_open() )
		{
			throw IOException(filename);
		}

		/// Read index		
		try 
		{
			m_index = new Index(*m_file);
		}
		catch ( Exception &e )
		{
			throw;
		} 
		catch (...)
		{
			throw IOException(filename);
		}
	}
	
	assert(m_index);
}

void FileIndexedIO::IndexedFile::seekg( NodePtr node )	
{
	assert( node->m_entry.entryType() == IndexedIO::File );
	assert( m_file );
	
	m_file->seek( node->m_offset, std::ios::beg, std::ios::in );							
}

FileIndexedIO::IndexPtr FileIndexedIO::IndexedFile::index() const
{
	assert(m_index);
	
	return m_index;
}

FileIndexedIO::IndexedFile::~IndexedFile()
{
	assert(m_file);
	
	if (index()->hasChanged())
	{			
		index()->write(*m_file);
	}
	
	m_file->close();
}

void FileIndexedIO::IndexedFile::write(NodePtr node, const char *data, Imf::Int64 size)
{
	/// Find next writable location			
	Imf::Int64 loc = m_index->allocate( size );
	
	/// Seek 'write' pointer to writable location
	m_file->seek( loc, std::ios_base::beg, std::ios_base::out );
	
	/// Update node with positional information within file
	node->m_offset = loc;
	node->m_size = size;
	
	/// Write data		
	m_file->write( data, size );
}

static IndexedIOInterface::Description<FileIndexedIO> registrar(".fio");

IndexedIOInterfacePtr FileIndexedIO::create(const std::string &path, const IndexedIO::EntryID &root, IndexedIO::OpenMode mode)
{
	return new FileIndexedIO(path, root, mode);
}

bool FileIndexedIO::canRead( const std::string &path )
{
	return Index::canRead( path );
}

FileIndexedIO::FileIndexedIO(const FileIndexedIO &other, const IndexedIO::EntryID &root, IndexedIO::OpenMode mode  )
{
	m_mode = mode;
	m_file = other.m_file;

	/// \todo No need to pass "root" as a parameter because we can derive it, according to this assert	
	assert( root == other.m_currentDirectory.fullPath() );
	
	m_currentDirectory = IndexedIOPath(root);
	
	m_rootDirectoryNode = other.m_currentDirectoryNode;
	m_currentDirectoryNode = other.m_currentDirectoryNode;
	
	assert( m_currentDirectoryNode );	
	assert( m_rootDirectoryNode );	
	
	chdir("/");	
}

IndexedIOInterfacePtr FileIndexedIO::resetRoot() const
{		
	IndexedIO::OpenMode mode = m_mode;
	
	if (mode & IndexedIO::Write)
	{
		mode &= ~IndexedIO::Write;
		assert( (mode & IndexedIO::Shared) == (m_mode & IndexedIO::Shared) );
		assert( (mode & IndexedIO::Exclusive) == (m_mode & IndexedIO::Exclusive) );
		assert( !(mode & IndexedIO::Write) );		
		mode |= IndexedIO::Append;
	}
		
	return new FileIndexedIO(*this, m_currentDirectory.fullPath(), mode);
}

FileIndexedIO::FileIndexedIO(const std::string &path, const IndexedIO::EntryID &root, IndexedIO::OpenMode mode)
{
	validateOpenMode(mode);
	const fs::path p = fs::path(path);
	const std::string filename = p.native_file_string();
	m_currentDirectory = IndexedIOPath(root);

	if (! fs::exists(filename) && (mode & IndexedIO::Read))
	{
		throw FileNotFoundIOException(filename);
	}

	m_file = new IndexedFile( filename, mode );
	
	m_rootDirectoryNode = m_file->index()->m_root;
	m_file->index()->find( m_currentDirectory.fullPath(), m_rootDirectoryNode, m_rootDirectoryNode );
	m_currentDirectoryNode = m_rootDirectoryNode;	
	
	if (mode & IndexedIO::Read)
	{
		if(!exists(m_currentDirectory.fullPath(), IndexedIO::Directory))
		{
			throw IOException(filename);
		}
	}
	else
	{	
		if (mode & IndexedIO::Write)
		{
			if (exists(m_currentDirectory.fullPath(), IndexedIO::Directory))
			{		
				rm("/");
			} 
		}
		
	} 	
	
	chdir("/");		
	
	assert( m_currentDirectoryNode );	
	assert( m_rootDirectoryNode );
}

FileIndexedIO::~FileIndexedIO()
{	
}

IndexedIO::EntryID FileIndexedIO::pwd()
{
	readable(".");
	
	return m_currentDirectory.relativePath();
}

FileIndexedIO::NodePtr FileIndexedIO::insert( const IndexedIO::EntryID &name  )
{
	assert( m_currentDirectoryNode );
	assert( m_rootDirectoryNode );
	
	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;	
	Tokenizer tokens(name, boost::char_separator<char>("/"));

	Tokenizer::iterator t = tokens.begin();
	
	NodePtr start = m_currentDirectoryNode;
	
	if (name[0] == '/')
	{
		start = m_rootDirectoryNode;
	}
		
	NodePtr node = start;	
	assert(node);
	
	bool found = start->findInChildren( t, tokens.end(), node, m_rootDirectoryNode );
	
	if (!found)
	{
		return node->insert( t, tokens.end() );
	}
	
	return 0;	
}


bool FileIndexedIO::find( const IndexedIO::EntryID &name, NodePtr &node ) const
{
	assert( m_currentDirectoryNode );
	assert( m_rootDirectoryNode );
	
	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;	
	Tokenizer tokens(name, boost::char_separator<char>("/"));

	Tokenizer::iterator t = tokens.begin();
	
	NodePtr start = m_currentDirectoryNode;
	
	if (name[0] == '/')
	{
		start = m_rootDirectoryNode;
	}
		
	node = start;	
	assert(node);
	
	return start->findInChildren( t, tokens.end(), node, m_rootDirectoryNode );
}

void FileIndexedIO::mkdir(const IndexedIO::EntryID &name)
{		
	writable(name);
	
	NodePtr node = insert( name );
	if (node)
	{		
		node->m_entry = IndexedIO::Entry( node->m_entry.id(), IndexedIO::Directory, IndexedIO::Invalid, 0);
	}
}

void FileIndexedIO::chdir(const IndexedIO::EntryID &name)
{	
	assert( m_currentDirectoryNode );
	readable(name);

	NodePtr node;	
	bool found = find( name, node );
	
	if (!found || node->m_entry.entryType() != IndexedIO::Directory)
	{
		throw IOException(name);
	}

	m_currentDirectory.append(name);
	m_currentDirectoryNode = node;
	
	assert( m_currentDirectoryNode );
}

IndexedIO::EntryList FileIndexedIO::ls(IndexedIOFilterPtr f)
{		
	readable(".");
	
	assert( m_currentDirectoryNode );
	
	IndexedIO::EntryList result;

	for ( Node::ChildMap::iterator it = m_currentDirectoryNode->m_children.begin(); it != m_currentDirectoryNode->m_children.end(); ++it)
	{
		result.push_back( it->second->m_entry );
	}
	
	if (f)
	{
		f->apply(result);
	}

	return result;
}

unsigned long FileIndexedIO::rm(const IndexedIO::EntryID &name)
{
	return rm(name, true);
}

unsigned long FileIndexedIO::rm(const IndexedIO::EntryID &name, bool throwIfNonExistent)
{
	assert( m_currentDirectoryNode );
	writable(name);
	
	NodePtr node;	
	bool found = find( name, node );
	
	if (!found)
	{
		if (throwIfNonExistent)
		{
			throw IOException(name);
		}
		else
		{
			return 0;
		}
	}
	
	m_file->index()->remove( node );
	
	chdir( m_currentDirectory.relativePath() );
	
	return 0;
}

bool FileIndexedIO::exists(const IndexedIO::EntryID &name) const
{
	assert( m_currentDirectoryNode );
	readable(name);
	
	NodePtr node;	
	return find( name, node );
}

bool FileIndexedIO::exists(const IndexedIOPath &path, IndexedIO::EntryType e) const
{
	if (path.fullPath() == "/" && e == IndexedIO::Directory)
	{
		return true;
	}
	
	NodePtr nearest = m_file->index()->m_root;
	bool found = m_file->index()->find( path, nearest, nearest );
	
	if (!found)
	{
		return false;
	}
	else 
	{
		return nearest->m_entry.entryType() == e;
	}
}


IndexedIO::Entry FileIndexedIO::ls(const IndexedIO::EntryID &name)
{
	assert( m_currentDirectoryNode );
	readable(name);

	NodePtr node;	
	bool found = find( name, node );
	
	if (!found)
	{
		throw IOException(name);
	}
	
	return node->m_entry;
}

template<typename T>
void FileIndexedIO::write(const IndexedIO::EntryID &name, const T *x, unsigned long arrayLength)
{
	writable(name);
	
	if (m_mode & IndexedIO::Write || m_mode & IndexedIO::Append)
	{
		rm(name, false);
	}
	
	NodePtr node = insert( name );
	if (node)
	{		

		unsigned long size = IndexedIO::DataSizeTraits<T*>::size(x, arrayLength);	
		IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T*>::type();
	
		const char *data = IndexedIO::DataFlattenTraits<T*>::flatten(x, arrayLength);
		assert(data);
		
		node->m_entry = IndexedIO::Entry( node->m_entry.id(), IndexedIO::File, dataType, arrayLength) ;
	
		m_file->write( node, data, size );
	
		IndexedIO::DataFlattenTraits<T*>::free(data);
	}
	else
	{
		throw IOException( name );
	}
}

template<typename T>
void FileIndexedIO::write(const IndexedIO::EntryID &name, const T &x)
{
	writable(name);
		
	if (m_mode & IndexedIO::Write || m_mode & IndexedIO::Append)
	{
		rm(name, false);
	}

	NodePtr node = insert( name );
	if (node)
	{		
		unsigned long size = IndexedIO::DataSizeTraits<T>::size(x);	
		IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T>::type();
	
		const char *data = IndexedIO::DataFlattenTraits<T>::flatten(x);
		assert(data);
		
		node->m_entry = IndexedIO::Entry( node->m_entry.id(), IndexedIO::File, dataType, 0) ;
	
		m_file->write( node, data, size );
	
		IndexedIO::DataFlattenTraits<T>::free(data);
	}
	else
	{
		throw IOException( name );
	}
}

template<typename T>
void FileIndexedIO::read(const IndexedIO::EntryID &name, T *&x, unsigned long arrayLength) const
{	
	assert( m_currentDirectoryNode );
	readable(name);
	
	NodePtr node;	
	bool found = find( name, node );
	
	if (!found || node->m_entry.entryType() != IndexedIO::File)
	{
		throw IOException(name);
	}
	
	m_file->seekg( node );
	
	Imf::Int64 size = node->m_size;
	char *data = new char[size];
	m_file->m_file->read( data, size );
	
	IndexedIO::DataFlattenTraits<T*>::unflatten( data, x, arrayLength );
	delete[]data;
}

template<typename T>
void FileIndexedIO::read(const IndexedIO::EntryID &name, T &x) const
{	
	assert( m_currentDirectoryNode );
	readable(name);

	NodePtr node;	
	bool found = find( name, node );
	
	if (!found || node->m_entry.entryType() != IndexedIO::File)
	{
		throw IOException(name);
	}

	m_file->seekg( node );

	Imf::Int64 size = node->m_size;
	char *data = new char[size];
	m_file->m_file->read( data, size );

	IndexedIO::DataFlattenTraits<T>::unflatten( data, x );
	delete[]data;
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

void FileIndexedIO::write(const IndexedIO::EntryID &name, const long *x, unsigned long arrayLength)
{
	write<long>(name, x, arrayLength);
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

void FileIndexedIO::write(const IndexedIO::EntryID &name, const long &x)
{
	write<long>(name, x);
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

// Read

void FileIndexedIO::read(const IndexedIO::EntryID &name, float *&x, unsigned long arrayLength)
{
	read<float>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, double *&x, unsigned long arrayLength)
{
	read<double>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, half *&x, unsigned long arrayLength)
{
	read<half>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, int *&x, unsigned long arrayLength)
{
	read<int>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, long *&x, unsigned long arrayLength)
{
	read<long>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, unsigned int *&x, unsigned long arrayLength)
{
	read<unsigned int>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, char *&x, unsigned long arrayLength)
{
	read<char>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, unsigned char *&x, unsigned long arrayLength)
{
	read<unsigned char>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, std::string *&x, unsigned long arrayLength)
{
	read<std::string>(name, x, arrayLength);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, float &x)
{
	read<float>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, double &x)
{
	read<double>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, half &x)
{
	read<half>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, int &x)
{
	read<int>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, long &x)
{
	read<long>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, std::string &x)
{
	read<std::string>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, unsigned int &x)
{
	read<unsigned int>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, char &x)
{
	read<char>(name, x);
}

void FileIndexedIO::read(const IndexedIO::EntryID &name, unsigned char &x)
{
	read<unsigned char>(name, x);
}
