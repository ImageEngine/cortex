//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore/SearchPath.h"
#include "IECore/LRUCache.h"

#include <set>

namespace IECore
{

IE_CORE_FORWARDDECLARE( CachedReader );
IE_CORE_FORWARDDECLARE( ModifyOp );
IE_CORE_FORWARDDECLARE( Object );

/// \addtogroup environmentGroup
///
/// <b>IECORE_CACHEDREADER_MEMORY</b><br>
/// <b>IECORE_CACHEDREADER_PATHS</b><br>
/// Used to specify the settings for the default CachedReader. See
/// CachedReader::defaultCachedReader() for more information.

/// The CachedReader class provides a means of loading files
/// using the Reader subclasses, but caching them in memory to
/// allow fast repeated loads.
/// \todo We probably need a way of setting parameters for the
/// Readers, and treating reads with different parameters as different
/// entities in the cache.
/// \todo Stats on cache misses etc.
/// \todo Can we do something to make sure that two paths to the same
/// file (symlinks) result in only a single cache entry?
/// \ingroup ioGroup
class CachedReader : public RefCounted
{

	public :

		IE_CORE_DECLAREMEMBERPTR( CachedReader );

		/// Creates a reader that will search for files on the
		/// given paths and load them. Up to maxMemory bytes of
		/// memory will be used to cache loading.
		CachedReader( const SearchPath &paths, size_t maxMemory );
		/// As above, but also takes an Op which will be applied to
		/// objects following loading.
		CachedReader( const SearchPath &paths, size_t maxMemory, ConstModifyOpPtr postProcessor );

		/// Searches for the given file and loads it if found.
		/// Throws an exception in case it cannot be found or no suitable Reader
		/// exists. The Object is returned with only const access as
		/// it actually refers to an object within the cache - you must
		/// call the copy() function on it if you wish to have something
		/// you are free to modify.
		/// \threading It is safe to call this method from multiple
		/// concurrent threads.
		ConstObjectPtr read( const std::string &file );

		/// Returns the amount of memory currently occupied by
		/// the cache.
		size_t memoryUsage() const;
		/// Frees all memory used by the cache.
		void clear();
		/// Clears the cache for the given file.
		void clear( const std::string &file );
		/// Forces insertion on the cache for a file that is already loaded on memory.
		void insert( const std::string &file, ConstObjectPtr obj );
		/// Returns true if the object is cached on memory.
		bool cached( const std::string &file ) const;

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
		/// has a memory limit specified in megabytes by the IECORE_CACHEDREADER_MEMORY
		/// environment variable and searchPaths set from the
		/// IECORE_CACHEDREADER_PATHS environment variable. If either of these
		/// need changing it's recommended to do that from a config file loaded
		/// by the ConfigLoader, to avoid multiple clients fighting over the
		/// same set of settings.
		static CachedReaderPtr defaultCachedReader();

	private :

		struct Getter;

		typedef LRUCache<std::string, ConstObjectPtr> Cache;
		SearchPath m_paths;
		Cache m_cache;

};

IE_CORE_DECLAREPTR( CachedReader )

} // namespace IECore

#endif // IE_CORE_CACHEDREADER_H
