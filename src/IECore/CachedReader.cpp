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

#include "IECore/CachedReader.h"

#include "IECore/ComputationCache.h"
#include "IECore/ModifyOp.h"
#include "IECore/Object.h"
#include "IECore/Reader.h"

#include "boost/format.hpp"
#include "boost/lexical_cast.hpp"

#include "tbb/concurrent_hash_map.h"
#include "tbb/mutex.h"

// Windows defines SearchPath
#ifdef SearchPath
#undef SearchPath
#endif

using namespace IECore;
using namespace boost;
using namespace boost::filesystem;
using namespace std;

//////////////////////////////////////////////////////////////////////////
// MemberData
//////////////////////////////////////////////////////////////////////////

#define PARAM(file)	MemberData::ComputeParameters( file, m_data.get() )

struct CachedReader::MemberData
{
	public :

		MemberData(const SearchPath &paths, ConstModifyOpPtr postProcessor, ObjectPoolPtr objectPool )
			:	m_searchPaths( paths ), m_cache( computeFn, hashFn, 10000, objectPool ), m_postProcessor( postProcessor )
		{
		}

		typedef std::pair< std::string, MemberData * > ComputeParameters;
		typedef IECore::ComputationCache< ComputeParameters > Cache;

		SearchPath m_searchPaths;
		Cache m_cache;
		ConstModifyOpPtr m_postProcessor;
		tbb::mutex m_postProcessorMutex;
		typedef tbb::concurrent_hash_map< std::string, std::string > FileErrors;
		FileErrors m_fileErrors;

	private :

		void registerFileError( const std::string &filePath, const std::string &errorMsg )
		{
			FileErrors::accessor it;
			if ( m_fileErrors.insert( it, filePath ) )
			{
				it->second = errorMsg;
			}
		}

		static MurmurHash hashFn( const ComputeParameters &params )
		{
			const std::string &filePath = params.first;
			MurmurHash h;
			h.append( filePath );
			return h;
		}

		// static function used by the cache mechanism to actually load the object data from file.
		static ObjectPtr computeFn( const ComputeParameters &params )
		{
			const std::string &filePath = params.first;
			MemberData *data = params.second;

			{
				/// Check if the file failed before...
				FileErrors::const_accessor cit;
				if ( data->m_fileErrors.find( cit, filePath ) )
				{
					throw Exception( ( format( "Previous attempt to read %s failed: %s" ) % filePath % cit->second ).str() );
				}
			}

			ObjectPtr result(nullptr);
			try
			{
				path resolvedPath = data->m_searchPaths.find( filePath );
				if( resolvedPath.empty() )
				{
					string pathList;
					for( list<path>::const_iterator it =  data->m_searchPaths.paths.begin(); it!= data->m_searchPaths.paths.end(); it++ )
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
					throw Exception( "Could not find file '" + filePath + "' at the following paths: " + pathList );
				}

				ReaderPtr r = Reader::create( resolvedPath.string() );
				if( !r )
				{
					throw Exception( "Could not create reader for '" + resolvedPath.string() + "'" );
				}

				result = r->read();
				/// \todo Why would this ever be NULL? Wouldn't we have thrown an exception already if
				/// we were unable to read the file?
				if( !result )
				{
					throw Exception( "Reader for '" + resolvedPath.string() + "' returned no data" );
				}

				if( data->m_postProcessor )
				{
					/// \todo We need to allow arguments to be passed to Op::operate() directly
					/// so that the same Op can be used from multiple threads with different arguments.
					/// This means adding an overloaded operate() method but more importantly making sure
					/// that all Ops only use their operands to access arguments and not go getting them
					/// from the Parameters directly.
					tbb::mutex::scoped_lock l( data->m_postProcessorMutex );
					ModifyOpPtr postProcessor = boost::const_pointer_cast<ModifyOp>( data->m_postProcessor );
					postProcessor->inputParameter()->setValue( result );
					postProcessor->copyParameter()->setTypedValue( false );
					postProcessor->operate();
				}
			}
			catch ( std::exception &e )
			{
				data->registerFileError( filePath, e.what() );
				throw;
			}
			catch ( ... )
			{
				data->registerFileError( filePath, "Unexpected error." );
				throw;
			}
			return result;
		}
};

//////////////////////////////////////////////////////////////////////////
// CachedReader
//////////////////////////////////////////////////////////////////////////

CachedReader::CachedReader( const SearchPath &paths, ObjectPoolPtr objectPool  )
	:	m_data( new MemberData( paths, nullptr, objectPool ) )
{
}

CachedReader::CachedReader( const SearchPath &paths, ConstModifyOpPtr postProcessor, ObjectPoolPtr objectPool )
	:	m_data( new MemberData( paths, postProcessor, objectPool ) )
{
}

ConstObjectPtr CachedReader::read( const std::string &file )
{
	return m_data->m_cache.get( PARAM(file) );
}

void CachedReader::insert( const std::string &file, ConstObjectPtr obj )
{
	m_data->m_fileErrors.erase( file );
	m_data->m_cache.set( PARAM(file), obj.get(), ObjectPool::StoreReference );
}

bool CachedReader::cached( const std::string &file ) const
{
	{
		/// Check if the file failed before...
		MemberData::FileErrors::const_accessor cit;
		if ( m_data->m_fileErrors.find( cit, file ) )
		{
			return false;
		}
	}

	return static_cast<bool>( m_data->m_cache.get( PARAM(file), MemberData::Cache::NullIfMissing ) );
}

void CachedReader::clear()
{
	m_data->m_fileErrors.clear();
	m_data->m_cache.clear();
}

void CachedReader::clear( const std::string &file )
{
	m_data->m_fileErrors.erase(file);
	m_data->m_cache.erase( PARAM(file) );
}

const SearchPath &CachedReader::getSearchPath() const
{
	return m_data->m_searchPaths;
}

void CachedReader::setSearchPath( const SearchPath &paths )
{
	if( m_data->m_searchPaths!=paths )
	{
		m_data->m_searchPaths = paths;
		clear();
	}
}

ObjectPool *CachedReader::objectPool() const
{
	return m_data->m_cache.objectPool();
}

CachedReader *CachedReader::defaultCachedReader()
{
	static CachedReaderPtr c = nullptr;
	if( !c )
	{
		const char *sp = getenv( "IECORE_CACHEDREADER_PATHS" );
		c = new CachedReader( SearchPath( sp ? sp : "" ) );
	}
	return c.get();
}

