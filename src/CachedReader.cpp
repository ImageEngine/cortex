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

#include "IECore/CachedReader.h"
#include "IECore/Object.h"

using namespace IECore;
using namespace boost::filesystem;
using namespace std;

CachedReader::CachedReader( const SearchPath &paths, size_t maxMemory )
	:	m_paths( paths ), m_maxMemory( maxMemory ), m_currentMemory( 0 )
{
}
		
ConstObjectPtr CachedReader::read( const std::string &file )
{
	// if we've failed to read it before then don't try again
	if( m_unreadables.count( file ) )
	{
		return 0;
	}

	// try to find it in the cache
	map<string, ConstObjectPtr>::iterator it = m_cache.find( file );
	if( it!=m_cache.end() )
	{
		/// \todo The remove is a linear time operation on the size
		/// of the cache. Do we need a faster way of doing this?
		m_accessOrder.remove( file );
		m_accessOrder.push_back( file );
		return it->second;
	}
	
	// then try to load it normally
	path resolvedPath = m_paths.find( file );
	if( resolvedPath.empty() )
	{
		m_unreadables.insert( file );
		return 0;
	}
	
	ReaderPtr r = Reader::create( resolvedPath.string() );
	if( !r )
	{
		m_unreadables.insert( file );
		return 0;	
	}
	
	ObjectPtr object = r->read();
	if( !object )
	{
		m_unreadables.insert( file );
		return 0;
	}
	
	// cache it if we can
	size_t objectMemory = object->memoryUsage();
	if( objectMemory<=m_maxMemory )
	{
		// it's small enough to cache
		m_accessOrder.push_back( file );
		m_cache[file] = object;
		m_currentMemory += objectMemory;
		reduce( m_maxMemory );
	}
	
	return object;
}

size_t CachedReader::memoryUsage() const
{
	return m_currentMemory;
}

void CachedReader::clear()
{
	reduce( 0 );
}

void CachedReader::reduce( size_t size )
{
	while( m_currentMemory > size )
	{
		string f = *m_accessOrder.begin();
		map<string, ConstObjectPtr>::iterator it = m_cache.find( f );
		assert( it!=m_cache.end() );
		size_t objectMemory = it->second->memoryUsage();
		assert( objectMemory>=m_currentMemory );
		m_currentMemory -= objectMemory;
		m_cache.erase( it );
		m_accessOrder.erase( m_accessOrder.begin() );
	}
	assert( m_accessOrder.size()==m_cache.size() );
}

const SearchPath &CachedReader::getSearchPath() const
{
	return m_paths;
}

void CachedReader::setSearchPath( const SearchPath &paths )
{
	m_paths = paths;
	clear();
}

size_t CachedReader::getMaxMemory() const
{
	return m_maxMemory;
}

void CachedReader::setMaxMemory( size_t maxMemory )
{
	m_maxMemory = maxMemory;
	reduce( m_maxMemory );
}

CachedReaderPtr CachedReader::defaultCachedReader()
{
	static CachedReaderPtr c = 0;
	if( !c )
	{
		const char *sp = getenv( "IECORE_CACHEDREADER_PATHS" );
		c = new CachedReader( SearchPath( sp ? sp : "", ":" ), 1024 * 1024 * 100 );
	}
	return c;
}
