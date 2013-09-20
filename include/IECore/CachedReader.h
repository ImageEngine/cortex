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

#ifndef IE_CORE_CACHEDREADER_H
#define IE_CORE_CACHEDREADER_H

#include "boost/shared_ptr.hpp"

#include "IECore/ObjectPool.h"
#include "IECore/SearchPath.h"
#include "IECore/ModifyOp.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( CachedReader );

/// \addtogroup environmentGroup
///
/// <b>IECORE_CACHEDREADER_PATHS</b><br>
/// Used to specify the settings for the default CachedReader. See
/// CachedReader::defaultCachedReader() for more information.

/// The CachedReader class provides a means of loading files
/// using the Reader subclasses, but caching them in memory to
/// allow fast repeated loads. It uses a ObjectPool to store the images.
/// It's recomended using the defaultObjectPool for sharing objects, which 
/// limits the memory used by the IECORE_OBJECTPOOL_MEMORY
/// environment variable.
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
		/// given paths and load them. Will use the given ObjectPool to store the loaded objects.
		CachedReader( const SearchPath &paths, ObjectPoolPtr objectPool = ObjectPool::defaultObjectPool() );

		/// As above, but also takes an Op which will be applied to
		/// objects following loading. Will use the given ObjectPool to store the loaded objects.
		CachedReader( const SearchPath &paths, ConstModifyOpPtr postProcessor, ObjectPoolPtr objectPool = ObjectPool::defaultObjectPool() );

		/// Searches for the given file and loads it if found.
		/// Throws an exception in case it cannot be found or no suitable Reader
		/// exists. The Object is returned with only const access as
		/// it actually refers to an object within the cache - you must
		/// call the copy() function on it if you wish to have something
		/// you are free to modify.
		/// \threading It is safe to call this method from multiple
		/// concurrent threads.
		ConstObjectPtr read( const std::string &file );

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

		/// Returns the ObjectPool object used by this CachedReader.
		ObjectPool *objectPool() const;

		/// Returns a static CachedReader instance to be used by anything
		/// wishing to share it's cache with others. It makes sense to use
		/// this wherever possible to conserve memory. This initially
		/// has searchPaths set from the IECORE_CACHEDREADER_PATHS 
		/// environment variable. If it needs changing it's recommended to do 
		/// that from a config file loaded by the ConfigLoader, to avoid multiple 
		/// clients fighting over the same set of settings.
		static CachedReaderPtr defaultCachedReader();

	private :

		struct MemberData;
		boost::shared_ptr<MemberData> m_data;

};

IE_CORE_DECLAREPTR( CachedReader )

} // namespace IECore

#endif // IE_CORE_CACHEDREADER_H
