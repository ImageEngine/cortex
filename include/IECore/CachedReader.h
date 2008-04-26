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

#ifndef IE_CORE_CACHEDREADER_H
#define IE_CORE_CACHEDREADER_H

#include "IECore/Reader.h"
#include "IECore/SearchPath.h"

#include <set>

namespace IECore
{

IE_CORE_FORWARDDECLARE( CachedReader );

/// The CachedReader class provides a means of loading files
/// using the Reader subclasses, but caching them in memory to
/// allow fast repeated loads.
/// \todo We probably need a way of setting parameters for the
/// Readers, and treating reads with different parameters as different
/// entities in the cache.
/// \todo Stats on cache misses etc.
/// \todo Can we do something to make sure that two paths to the same
/// file (symlinks) result in only a single cache entry?
/// \todo Can we use LRUCache in the implementation? There seems to be duplicated
/// functionality here
class CachedReader : public RefCounted
{

	public :

		/// Creates a reader that will search for files on the
		/// given paths and load them. Up to maxMemory bytes of
		/// memory will be used to cache loading.
		CachedReader( const SearchPath &paths, size_t maxMemory );
		
		/// Searches for the given file and loads it if found.
		/// Throws an exception in case it cannot be found or no suitable Reader
		/// exists. The Object is returned with only const access as
		/// it actually refers to an object within the cache - you must
		/// call the copy() function on it if you wish to have something
		/// you are free to modify.
		ConstObjectPtr read( const std::string &file );
		
		/// Returns the amount of memory currently occupied by
		/// the cache.
		size_t memoryUsage() const;
		/// Frees all memory used by the cache.
		void clear();
		
		/// Returns the SearchPath in use.
		const SearchPath &getSearchPath() const;
		/// Changes the SearchPath used to find files. Note
		/// that this calls clear(), as changing the paths
		/// potentially invalidates the contents of the cache.
		void setSearchPath( const SearchPath &paths );
		
		/// Returns the maximum amount of memory the cache will use.
		size_t getMaxMemory() const;
		/// Sets the maximum amount of memory the cache will use. If this
		/// is less than memoryUsage() then cache removals will result.
		void setMaxMemory( size_t maxMemory );
		
		/// Returns a static CachedReader instance to be used by anything
		/// wishing to share it's cache with others. It makes sense to use
		/// this wherever possible to conserve memory. This initially
		/// has a memory limit of 100Mb and searchPaths set from the
		/// IECORE_CACHEDREADER_PATHS environment variable. If either of these
		/// need changing it's recommended to do that from a config file loaded
		/// by the ConfigLoader, to avoid multiple clients fighting over the
		/// same set of settings.
		static CachedReaderPtr defaultCachedReader();
		
	private :
	
		SearchPath m_paths;
	
		size_t m_maxMemory;
		size_t m_currentMemory;
		
		// Remove things from the cache till m_currentMemory < m_maxMemory
		void reduce( size_t size );
	
		// Least recently accessed filenames at the front
		std::list<std::string> m_accessOrder;
		std::map<std::string, ConstObjectPtr> m_cache;
		std::set<std::string> m_unreadables;
		
};

} // namespace IECore

#endif // IE_CORE_CACHEDREADER_H
