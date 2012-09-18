//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "tbb/mutex.h"

#include "boost/lexical_cast.hpp"

#include "IECore/CachedReader.h"
#include "IECore/Reader.h"
#include "IECore/Object.h"
#include "IECore/ModifyOp.h"

using namespace IECore;
using namespace boost::filesystem;
using namespace std;

//////////////////////////////////////////////////////////////////////////
// Getter
//////////////////////////////////////////////////////////////////////////

struct CachedReader::Getter
{
	Getter( const SearchPath &paths, ConstModifyOpPtr postProcessor=0 )
	:	m_paths( paths ), m_postProcessor( postProcessor )
	{
	}

	Getter( const Getter &g )
	: m_paths( g.m_paths ), m_postProcessor( g.m_postProcessor )
	{
	}

	const SearchPath &m_paths; // references CachedReader::m_paths
	ConstModifyOpPtr m_postProcessor;
	tbb::mutex m_postProcessorMutex;

	ConstObjectPtr operator()( const std::string &file, size_t &cost )
	{
		cost = 0;

		path resolvedPath = m_paths.find( file );
		if( resolvedPath.empty() )
		{
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
			throw Exception( "Could not create reader for '" + resolvedPath.string() + "'" );
		}

		ObjectPtr result = r->read();
		/// \todo Why would this ever be NULL? Wouldn't we have thrown an exception already if
		/// we were unable to read the file?
		if( !result )
		{
			throw Exception( "Reader for '" + resolvedPath.string() + "' returned no data" );
		}

		if( m_postProcessor )
		{
			/// \todo We need to allow arguments to be passed to Op::operate() directly
			/// so that the same Op can be used from multiple threads with different arguments.
			/// This means adding an overloaded operate() method but more importantly making sure
			/// that all Ops only use their operands to access arguments and not go getting them
			/// from the Parameters directly.
			tbb::mutex::scoped_lock l( m_postProcessorMutex );
			ModifyOpPtr postProcessor = constPointerCast<ModifyOp>( m_postProcessor );
			postProcessor->inputParameter()->setValue( result );
			postProcessor->copyParameter()->setTypedValue( false );
			postProcessor->operate();
		}

		cost = result->memoryUsage();

		return result;
	}

};

//////////////////////////////////////////////////////////////////////////
// CachedReader
//////////////////////////////////////////////////////////////////////////

CachedReader::CachedReader( const SearchPath &paths, size_t maxMemory )
	:	m_paths( paths ), m_cache( Getter( m_paths ), maxMemory )
{
}

CachedReader::CachedReader( const SearchPath &paths, size_t maxMemory, ConstModifyOpPtr postProcessor )
	:	m_paths( paths ), m_cache( Getter( m_paths, postProcessor ), maxMemory )
{
}

ConstObjectPtr CachedReader::read( const std::string &file )
{
	return m_cache.get( file );
}

void CachedReader::insert( const std::string &file, ConstObjectPtr obj )
{
	m_cache.set( file, obj, obj->memoryUsage() );
}

bool CachedReader::cached( const std::string &file ) const
{
	return m_cache.cached( file );
}

size_t CachedReader::memoryUsage() const
{
	return m_cache.currentCost();
}

void CachedReader::clear()
{
	m_cache.clear();
}

void CachedReader::clear( const std::string &file )
{
	m_cache.erase( file );
}

const SearchPath &CachedReader::getSearchPath() const
{
	return m_paths;
}

void CachedReader::setSearchPath( const SearchPath &paths )
{
	if( m_paths!=paths )
	{
		m_paths = paths;
		clear();
	}
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
		const char *m = getenv( "IECORE_CACHEDREADER_MEMORY" );
		int mi = m ? boost::lexical_cast<int>( m ) : 100;
	
		const char *sp = getenv( "IECORE_CACHEDREADER_PATHS" );
		c = new CachedReader( SearchPath( sp ? sp : "", ":" ), 1024 * 1024 * mi );
	}
	return c;
}
