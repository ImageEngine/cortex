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

#include "IECore/ByteOrder.h"

#include "IECore/FileIndexedIO.h"

/// \todo This cache class should be extracted into a public header
/// so it can be usable all over. The reason it isn't right now is that
/// it isn't good enough. The principal problem is that we need to find
/// a good return type for get() that will work with any Data type in the
/// three cases we have 1) unable to get the object 2) getting the object
/// but not putting it in the cache 3) getting the object from the cache.
template<typename Key, typename Data, typename GetterFn>
class LRUCache
{
	public:
		typedef typename GetterFn::Cost Cost;
		
		typedef std::pair<Data, Cost> DataCost;
	
		typedef std::list< std::pair<Key, DataCost> > List;		
		
		typedef std::map< Key, typename List::iterator > Cache;
		
		Cost m_maxCost;
		mutable Cost m_currentCost;
		
		mutable List m_list;
		mutable Cache m_cache;
		
		LRUCache()
		{
			m_maxCost = 500;
			m_currentCost = Cost();
		}
	
		void clear()
		{
			m_currentCost = Cost();
			m_list.clear();
			m_cache.clear();
		}
		
		Data get( const Key& key, GetterFn fn ) const
		{
			typename Cache::iterator it = m_cache.find( key );
			
			if ( it == m_cache.end() )
			{
				/// Not found in the cache, so compute the data and its associated cost
				
				Data data;
				Cost cost;
				bool found = fn.get( key, data, cost );
				
				if (found)
				{
					if (cost > m_maxCost)
					{
						/// Don't store as we'll exceed the maximum cost immediately. So just return
						/// the data, without modifiying the cache.

						return data;
					}
					
					/// If necessary, clear out any data with a least-recently-used strategy until we
					/// have enough remaining "cost" to store.
					while (m_currentCost + cost > m_maxCost)
					{
						m_currentCost -= m_list.back().second.second;
						
						/// The back of the list contains the LRU entry.
						m_cache.erase( m_list.back().first );						
						m_list.pop_back();
					}
					
					/// Update the cost to reflect the storage of the new item
					m_currentCost += cost;
					
					/// Insert the item at the front of the list
					std::pair<Key, DataCost> listEntry( key, DataCost( data, cost ) );						
					m_list.push_front( listEntry );
					
					/// Add the item to the map
					std::pair<typename Cache::iterator, bool> it  = m_cache.insert( typename Cache::value_type( key, m_list.begin() ) );
					
					assert( m_list.size() == m_cache.size() );

//					/// Return the actual data
					return (it.first->second)->second.first;					
				}
				else
				{					
					/// \todo We need a way of returning "NOT FOUND"
					return Data();
				}
			}
			else
			{
				/// Move the entry to the front of the list
				std::pair<Key, DataCost> listEntry = *(it->second);
				m_list.erase( it->second );
				m_list.push_front( listEntry );
				
				assert( m_list.size() == m_cache.size() );
				
				/// Update the map to reflect the change in list position
				it->second = m_list.begin();

				/// Return the actual data
				return (it->second)->second.first;
				
			}
			
		}

};

using std::fstream;
using namespace IECore;
namespace fs = boost::filesystem;

/// A tree to represent nodes in a filesystem, along with their locations in a file.
class FileIndexedIO::Index : public RefCounted
{
	public:
		class Node;
		IE_CORE_DECLAREPTR( Node );
	
		typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
		typedef std::vector<std::string> PathParts;
		
		struct CacheFn
		{
			CacheFn( ConstIndexPtr idx ) : m_idx(idx)
			{
			}
			
			typedef int Cost;
			
			bool get( const std::string &fullPath, NodePtr &node, Cost &cost );
						
			ConstIndexPtr m_idx;
		};
		
		typedef LRUCache< std::string, NodePtr, CacheFn> Cache;
		
		Cache m_cache;
	
		/// Construct an empty index
		Index();
		
		/// Construct an index from reading a file stream.
		Index( fstream &f );
		virtual ~Index();
	
		
			
		NodePtr m_root;	
		
		/// Returns where or not the index has changed since the last time it was written
		bool hasChanged() const;
		
		/// Attempts to find a path in the index. Will seek out the closest node in the hierarchy, and return
		/// whether or not this node is an exact match for the query. Optionally, it can fill in a list of
		/// path components which were not successfully matched.					
		bool find( const IndexedIOPath &p, NodePtr &nearest, PathParts *remainder = 0) const;
				
		/// Remove a node, and all its subnodes from the index		
		void remove( NodePtr n );
	
		/// Insert a new entry into the index, returning the node which stores it
		NodePtr insert( const IndexedIOPath &parentPath, IndexedIO::Entry e );

		/// Write the index to a file stream
		void write( fstream & f );
	
		/// Allocate a new chunk of data of the requested size, returning its offset within the file
		Imf::Int64 allocate( Imf::Int64 sz );
	
		/// Deallocate a node's data from the file. 
		void deallocate( NodePtr n );
	
		/// Return the total number of nodes in the index.
		Imf::Int64 nodeCount();
		
		/// Output the index to stderr, for debugging purposes
		void dump();	
	
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
		
		void write( fstream &f, Node* n );
	
		void readNode( fstream &f );
	
		Imf::Int64 nodeCount( Node* n );
	
		void dump( Node* n, int depth );
		
		void write( fstream &f, Imf::Int64 n);
	
		void read( fstream &f, Imf::Int64 &n);			
	
};

/// A single node within an index
class FileIndexedIO::Index::Node : public RefCounted
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
		typedef std::map< std::string, NodePtr> ChildMap;
		ChildMap m_children;
	
		/// Construct a new Node in the given index with the given numeric id
		Node(Index* idx, Imf::Int64 id);
	
		virtual ~Node();
				
		/// Add	a child node
		void addChild( NodePtr c );
	
		/// Write this node to a stream
		void write( fstream &f );

		/// Replace the contents of this node with data read from a stream
		void read( fstream &f );
		
		/// Traverse through this node and down its children, removing the front 
		/// of the list of 'parts' every time we descend through a match.
		bool find( Tokenizer::iterator &parts, Tokenizer::iterator end, NodePtr &nearest ) const;
	
		/// As above, but just traverse through the children.
		bool findInChildren( Tokenizer::iterator &parts, Tokenizer::iterator end, NodePtr &nearest ) const;
		
	protected:
		
		Index *m_idx;
};

bool FileIndexedIO::Index::CacheFn::get( const std::string &fullPath, NodePtr &node, Cost &cost )
{
	Tokenizer tokens(fullPath, boost::char_separator<char>("/"));

	Tokenizer::iterator t = tokens.begin();

	bool found = m_idx->m_root->findInChildren( t, tokens.end(), node );
	
	cost = std::distance( tokens.begin(), tokens.end() );
	
	return found;			
}

bool FileIndexedIO::Index::hasChanged() const
{
	return m_hasChanged;
}

void FileIndexedIO::Index::write( fstream &f, Imf::Int64 n)
{
	Imf::Int64 nl = asLittleEndian<>(n);
	f.write( (char*) &nl, sizeof( Imf::Int64) );
}
	
void FileIndexedIO::Index::read( fstream &f, Imf::Int64 &n)
{
	Imf::Int64 nl;
	f.read(( char*) &nl, sizeof(Imf::Int64) );
	
	if (bigEndian())
	{
		n = asBigEndian<>(nl);
	}
	else
	{
		/// Already little endian
		n = nl;
	}
}
	
Imf::Int64 FileIndexedIO::Index::makeId()
{
	/// \todo maybe hash the name, check for uniqueness in the tree, then use that?
	return ++m_prevId;
}			
	
bool FileIndexedIO::Index::find( const IndexedIOPath &p, NodePtr &nearest, PathParts *remainder) const
{	
	bool found = false;
	Tokenizer *tokens;
	Tokenizer::iterator t;

	CacheFn fn(this);
	const std::string &head = p.head();
	NodePtr data = m_cache.get( head, fn );

	if (data)
	{
		tokens = new Tokenizer(p.tail(), boost::char_separator<char>("/"));
		nearest = data;
		t = tokens->begin();
		
		found = (data)->findInChildren( t, tokens->end(), nearest );
	}
	else
	{
		tokens = new Tokenizer(p.fullPath(), boost::char_separator<char>("/"));
	
		t = tokens->begin();

		found = m_root->findInChildren( t, tokens->end(), nearest );
	}
	
	if (!found && remainder)
	{
		while(  t != tokens->end() )
		{
			remainder->push_back( *t++ );
		}
	}
	
	delete tokens;
	
	return found; 
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
	
	m_cache.clear();
}

FileIndexedIO::Index::NodePtr FileIndexedIO::Index::insert( const IndexedIOPath &parentPath, IndexedIO::Entry e )
{	
	PathParts remainder;
	NodePtr parent = m_root;
	bool found = find( parentPath, parent, &remainder );
	NodePtr child;
	
	if (!found)
	{			
		for (PathParts::iterator it = remainder.begin(); it != remainder.end(); ++it)
		{
			child = new Index::Node( this, makeId() );
			
			child->m_entry = IndexedIO::Entry(*it, IndexedIO::Directory, IndexedIO::Invalid, 0);
			
			parent->addChild( child );
			parent = child;
		}
	}
	else
	{
		assert( remainder.size() == 0 );	
	}
			
	Imf::Int64 newId = makeId();	
	child = new Node(this, newId);
	
	m_indexToNodeMap[newId] = child.get();
	m_nodeToIndexMap[child.get()] = newId;
		
	child->m_entry = e;
	
	parent->addChild( child );
	
	m_hasChanged = true;
	
	return child;
}

FileIndexedIO::Index::Index() : m_root(0), m_prevId(0)
{
	m_version = g_currentVersion;
	
	m_root = new Node( this, 0 );
	
	m_indexToNodeMap[0] = m_root.get();
	m_nodeToIndexMap[m_root.get()] = 0;
	
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
			
FileIndexedIO::Index::Index( fstream &f ) : m_prevId(0)
{
	m_hasChanged = false;
	f.seekg( 0, std::ios::end);
	Imf::Int64 end = f.tellg();		

	f.seekg( end - 1*sizeof(Imf::Int64) );
	
	Imf::Int64 magicNumber;
	read( f, magicNumber );
	
	if ( magicNumber == g_versionedMagicNumber )
	{
		f.seekg( end - 3*sizeof(Imf::Int64) );
		read( f, m_offset );
		read( f, m_version );						
	}
	else if (magicNumber == g_unversionedMagicNumber )
	{
		m_version = 0;
		
		f.seekg( end - 2*sizeof(Imf::Int64) );	
		read( f, m_offset );
	}
	else
	{	
		throw IOException("Not a FileIndexedIO file");
	}
			
	f.seekg( m_offset );
	
	Imf::Int64 numNodes;		
	read( f, numNodes );

	for (Imf::Int64 i = 0; i < numNodes; i++)
	{
		readNode( f );
	}
	Imf::Int64 numFreePages;		
	read( f, numFreePages );
	

	for (Imf::Int64 i = 0; i < numFreePages; i++)
	{
		Imf::Int64 offset, sz;
		read( f, offset );
		read( f, sz );
		
		addFreePage( offset, sz );
	}
	
	m_next = m_offset;
}
	

void FileIndexedIO::Index::write( fstream & f )
{
	/// Write index at end
	fstream::pos_type indexStart = m_next;
	
	f.seekp( m_next );
	
	m_offset = indexStart;
	
	Imf::Int64 numNodes = nodeCount();
	
	write(f, numNodes);
	
	write( f, m_root.get() );

	assert( m_freePagesOffset.size() == m_freePagesSize.size() );			
	Imf::Int64 numFreePages = m_freePagesSize.size();

	// Write out number of free "pages"		
	write(f, numFreePages);
			
	/// Write out each free page
	for ( FreePagesSizeMap::const_iterator it = m_freePagesSize.begin(); it != m_freePagesSize.end(); ++it)
	{
		write(f, it->second->m_offset);
		write(f, it->second->m_size);
	}
	
	write( f, m_offset );
	write( f, m_version );		
	write( f, g_versionedMagicNumber );
	
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

void FileIndexedIO::Index::dump()
{
	dump(m_root.get(), 0);
}

void FileIndexedIO::Index::write( fstream & f, Node* n )
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

void FileIndexedIO::Index::readNode( fstream &f )
{
	NodePtr n = new Node( this, 0 );
			
	assert(n);
	
	n->read( f );

	if (n->m_id == 0)
	{
		m_root = n;
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

void FileIndexedIO::Index::dump( Node* n, int depth )
{
	for (int i = 0; i < depth; i++)
		std::cerr << "..";
	
	std::cerr << n->m_entry.id();
	
	switch (n->m_entry.entryType())
	{
		case IndexedIO::Directory:
			std::cerr << " (d)";
			break;
		case IndexedIO::File:
			std::cerr << " (f)";
			break;		
		default:
			assert(0);		
	}
	
	std::cerr << std::endl;
	
	for (Node::ChildMap::const_iterator it = n->m_children.begin(); it != n->m_children.end(); ++it)
	{
		dump( it->second.get(), depth+1 );		
	}

}


FileIndexedIO::Index::Node::Node(Index* idx, Imf::Int64 id) : RefCounted()
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

FileIndexedIO::Index::Node::~Node()
{
}
			
void FileIndexedIO::Index::Node::addChild( NodePtr c ) 
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

void FileIndexedIO::Index::Node::write( fstream &f )
{
	char t = m_entry.entryType();			
	f.write( &t, sizeof(char) );
			
	m_idx->write(f, m_entry.id().size());
	f.write( m_entry.id().c_str(), m_entry.id().size() );
	
	
	t = m_entry.m_dataType;			
	f.write( &t, sizeof(char) );
	
	m_idx->write(f, m_entry.m_arrayLength );
								
	m_idx->write(f, m_id);
	
	if (m_parent)
	{					
		assert( m_idx->m_nodeToIndexMap.find( m_parent ) != m_idx->m_nodeToIndexMap.end() );	
		m_idx->write(f, m_idx->m_nodeToIndexMap[m_parent]);		
	}
	else
	{
		m_idx->write(f, (Imf::Int64)0);
	}
	
	m_idx->write(f, m_offset);
	m_idx->write(f, m_size);
}

void FileIndexedIO::Index::Node::read( fstream &f )
{
	char t;
	
	f.read( &t, sizeof(char) );		
	m_entry.m_entryType = (IndexedIO::EntryType)t;

	Imf::Int64 entrySize;
	m_idx->read( f, entrySize );
	char *s = new char[entrySize+1];
	f.read( s, entrySize );
	s[entrySize] = '\0';

	m_entry.m_ID = s;
	f.read( &t, sizeof(char) );		
	m_entry.m_dataType = (IndexedIO::DataType)t;

	Imf::Int64 arrayLength;
	m_idx->read( f, arrayLength );
	m_entry.m_arrayLength = arrayLength;			
	
	delete[] s;
	m_idx->read(f, m_id );

	m_idx->m_indexToNodeMap[m_id] = this;
	m_idx->m_nodeToIndexMap[this] = m_id;
	
	Imf::Int64 parentId;
	m_idx->read(f, parentId );
	
	m_idx->m_prevId = std::max( m_idx->m_prevId, parentId );
	m_idx->m_prevId = std::max( m_idx->m_prevId, m_id );			

	IndexToNodeMap::iterator it = m_idx->m_indexToNodeMap.find( parentId );
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
	
	
		
	m_idx->read(f, m_offset );
	m_idx->read(f, m_size );
	
}

bool FileIndexedIO::Index::Node::find( Tokenizer::iterator &parts, Tokenizer::iterator end, NodePtr &nearest ) const
{
	if (parts == end)
	{
		return true;
	}
	else 
	{
		if (*parts == m_entry.id())
		{			
			nearest = const_cast<Node*>(this);	

			return findInChildren( ++parts, end, nearest );
		
		}
		else
		{
			return false;
		}
	}
}

bool FileIndexedIO::Index::Node::findInChildren( Tokenizer::iterator &parts, Tokenizer::iterator end, NodePtr &nearest ) const
{	
	if (parts == end)
	{
		return true;
	}
	
	ChildMap::const_iterator cit = m_children.find( *parts );
	
	if (cit != m_children.end())
	{
		return cit->second->find( parts, end, nearest );
	}
	
	return false;			
}	
	


class FileIndexedIO::IndexedFile : public RefCounted
{
	public:
		fstream *m_file;
			
		IndexedFile( const std::string &filename, IndexedIO::OpenMode mode );
		virtual ~IndexedFile();
			
		/// Obtain the index for this file		
		IndexPtr index() const;
		
		/// Seek to a particular node within the file for reading
		void seekg( Index::NodePtr node );		
				
		/// Write some data to the file. Its position is automatically allocated within the file, and the node
		/// is updated to record this offset along with its size.
		void write(Index::NodePtr node, const char *data, Imf::Int64 size);
	
	protected:
	
		IndexPtr m_index;
};

			
FileIndexedIO::IndexedFile::IndexedFile( const std::string &filename, IndexedIO::OpenMode mode )
{
	if (mode & IndexedIO::Write)
	{
		m_file = new fstream(filename.c_str(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
		
		if (m_file->bad() || !m_file->good())
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
			m_file = new fstream(filename.c_str(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
		
			if (m_file->bad() || !m_file->good())
			{
				throw IOException(filename);
			}
		
			m_index = new Index();
		}
		else
		{		
			/// Read existing file
			m_file = new fstream(filename.c_str(), std::ios::in | std::ios::out | std::ios::binary);
		
			if (m_file->bad() || !m_file->good())
			{
				throw IOException(filename);
			}

			/// Read index		
			try 
			{
				m_index = new Index(*m_file);
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
		m_file = new fstream(filename.c_str(), std::ios::in | std::ios::binary);
		
		if (m_file->bad() || !m_file->good())
		{
			throw IOException(filename);
		}

		/// Read index		
		try 
		{
			m_index = new Index(*m_file);
		} 
		catch (...)
		{
			throw IOException(filename);
		}
	}
	
	assert(m_index);
}

void FileIndexedIO::IndexedFile::seekg( Index::NodePtr node )	
{
	assert( node->m_entry.entryType() == IndexedIO::File );
	assert( m_file );
	
	m_file->seekg( node->m_offset );							
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

void FileIndexedIO::IndexedFile::write(Index::NodePtr node, const char *data, Imf::Int64 size)
{
	/// Find next writable location			
	Imf::Int64 loc = m_index->allocate( size );
	
	/// Seek 'write' pointer to writable location
	m_file->seekp( loc );
	
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

FileIndexedIO::FileIndexedIO(const FileIndexedIO &other, const IndexedIO::EntryID &root, IndexedIO::OpenMode mode  )
{
	m_mode = mode;
	m_file = other.m_file;
	m_currentDirectory = IndexedIOPath(root);
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
}

FileIndexedIO::~FileIndexedIO()
{

	
}

IndexedIO::EntryID FileIndexedIO::pwd()
{
	readable(".");
	
	return m_currentDirectory.relativePath();
}

void FileIndexedIO::mkdir(const IndexedIO::EntryID &name)
{		
	writable(name);
	
	const IndexedIOPath d = m_currentDirectory.appended(name);
		
	/// Add to index
	if (!exists( name ) )
	{
		m_file->index()->insert( d.head(), IndexedIO::Entry( d.tail() , IndexedIO::Directory, IndexedIO::Invalid, 0) );
	}
}

void FileIndexedIO::chdir(const IndexedIO::EntryID &name)
{	
	readable(name);
	IndexedIOPath d = m_currentDirectory.appended(name);
	
	if (!exists(d.fullPath(), IndexedIO::Directory))
	{ 	
		throw IOException(name);
	}
	
	m_currentDirectory = d;
}

IndexedIO::EntryList FileIndexedIO::ls(IndexedIOFilterPtr f)
{		
	readable(".");
	
	IndexedIO::EntryList result;
	
	IndexedIOPath p = m_currentDirectory;
	
	Index::NodePtr nearest = m_file->index()->m_root;
	bool found = m_file->index()->find( p, nearest );	
	
	if (!found)
	{
		throw IOException( p.fullPath() );
	}
	
	for ( Index::Node::ChildMap::iterator it = nearest->m_children.begin(); it != nearest->m_children.end(); ++it)
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
	writable(name);
	
	IndexedIOPath p = m_currentDirectory.appended(name);
	
	Index::NodePtr nearest = m_file->index()->m_root;	
	bool found = m_file->index()->find( p, nearest );		
	
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
	
	m_file->index()->remove( nearest );
	
	return 0;
}

bool FileIndexedIO::exists(const IndexedIO::EntryID &name) const
{
	readable(name);
		
	IndexedIOPath p = m_currentDirectory.appended(name);
	
	if (p.fullPath() == "/")
	{
		return true;
	}
	
	Index::NodePtr nearest = m_file->index()->m_root;
	return m_file->index()->find( p, nearest );
}

bool FileIndexedIO::exists(const IndexedIOPath &path, IndexedIO::EntryType e) const
{
	if (path.fullPath() == "/" && e == IndexedIO::Directory)
	{
		return true;
	}
	
	Index::NodePtr nearest = m_file->index()->m_root;
	bool found= m_file->index()->find( path, nearest );
	
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
	readable(name);

	IndexedIOPath p = m_currentDirectory.appended(name);
	
	Index::NodePtr nearest = m_file->index()->m_root;
	bool found = m_file->index()->find( p, nearest );
	if (!found)
	{
		throw IOException(name);
	}
	
	return nearest->m_entry;
}

template<typename T>
void FileIndexedIO::write(const IndexedIO::EntryID &name, const T *x, unsigned long arrayLength)
{
	writable(name);
	
	if (m_mode & IndexedIO::Write || m_mode & IndexedIO::Append)
	{
		rm(name, false);
	}

	IndexedIOPath p = m_currentDirectory.appended(name);

	unsigned long size = IndexedIO::DataSizeTraits<T*>::size(x, arrayLength);	
	IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T*>::type();
	
	const char *data = IndexedIO::DataFlattenTraits<T*>::flatten(x, arrayLength);
	assert(data);
	
	Index::NodePtr node = m_file->index()->insert( p.head(), IndexedIO::Entry( p.tail(), IndexedIO::File, dataType, arrayLength) );
	m_file->write( node, data, size );
	
	IndexedIO::DataFlattenTraits<T*>::free(data);
}

template<typename T>
void FileIndexedIO::write(const IndexedIO::EntryID &name, const T &x)
{
	writable(name);
		
	if (m_mode & IndexedIO::Write || m_mode & IndexedIO::Append)
	{
		rm(name, false);
	}
	
	IndexedIOPath p = m_currentDirectory.appended(name);
	
	unsigned long size = IndexedIO::DataSizeTraits<T>::size(x);
	IndexedIO::DataType dataType = IndexedIO::DataTypeTraits<T>::type();
	
	const char *data = IndexedIO::DataFlattenTraits<T>::flatten(x);
	assert(data);
	
	Index::NodePtr node = m_file->index()->insert( p.head(), IndexedIO::Entry( p.tail(), IndexedIO::File, dataType, 0) );
	m_file->write( node, data, size );
	
	IndexedIO::DataFlattenTraits<T>::free(data);
}

template<typename T>
void FileIndexedIO::read(const IndexedIO::EntryID &name, T *&x, unsigned long arrayLength) const
{	
	readable(name);
	
	IndexedIOPath p = m_currentDirectory.appended(name);
	
	Index::NodePtr node = m_file->index()->m_root;
	bool found = m_file->index()->find( p, node );
	
	if (!found || node->m_entry.entryType() != IndexedIO::File )
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
	readable(name);

	IndexedIOPath p = m_currentDirectory.appended(name);
	Index::NodePtr node = m_file->index()->m_root;
	bool found = m_file->index()->find( p, node );
	
	if (!found || node->m_entry.entryType() != IndexedIO::File )
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
