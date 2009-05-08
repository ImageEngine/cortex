//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

bool CachedReader::CacheFn::get( const std::string &file, ConstObjectPtr &object, Cost &cost )
{
	object = 0;
	cost = 0;

	// if we've failed to read it before then don't try again
	if( m_unreadables.count( file ) )
	{
		throw Exception( "Unreadable file '" + file + "'" );
	}

	// then try to load it normally
	path resolvedPath = m_paths.find( file );
	if( resolvedPath.empty() )
	{
		m_unreadables.insert( file );
		string pathList;
		for( list<path>::const_iterator it = m_paths.paths.begin(); it!=m_paths.paths.end(); it++ )
		{
			if ( pathList.size() > 0 )
			{
				pathList += ":" + it->string();
			}
			else
			{
				pathList = it->string();
			}
		}
		throw Exception( "Could not find file '" + file + "' at the following paths: " + pathList );
	}

	ReaderPtr r = Reader::create( resolvedPath.string() );
	if( !r )
	{
		m_unreadables.insert( file );
		throw Exception( "Could not create reader for '" + resolvedPath.string() + "'" );
	}

	object = r->read();
	/// \todo Why would this ever be NULL? Wouldn't we have thrown an exception already if
	/// we were unable to read the file?
	if( !object )
	{
		m_unreadables.insert( file );
		throw Exception( "Reader for '" + resolvedPath.string() + "' returned no data" );
	}

	cost = object->memoryUsage();

	return true;
}

CachedReader::CachedReader( const SearchPath &paths, size_t maxMemory )
{
	m_fn.m_paths = paths;

	m_cache.setMaxCost( maxMemory );
}

ConstObjectPtr CachedReader::read( const std::string &file )
{
	ConstObjectPtr data;

	bool result = m_cache.get( file, m_fn, data );
	assert( result );
	(void) result;

	return data;
}

size_t CachedReader::memoryUsage() const
{
	return m_cache.currentCost();
}

void CachedReader::clear()
{
	m_cache.clear();
}

const SearchPath &CachedReader::getSearchPath() const
{
	return m_fn.m_paths;
}

void CachedReader::setSearchPath( const SearchPath &paths )
{
	m_fn.m_paths = paths;
	clear();
}

size_t CachedReader::getMaxMemory() const
{
	return m_cache.getMaxCost();
}

void CachedReader::setMaxMemory( size_t maxMemory )
{
	m_cache.setMaxCost( maxMemory );
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
