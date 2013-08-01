//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_OBJECTPOOL_H
#define IECORE_OBJECTPOOL_H

#include "boost/shared_ptr.hpp"

#include "IECore/Object.h"
#include "IECore/MurmurHash.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ObjectPool );

/// \addtogroup environmentGroup
///
/// <b>IECORE_OBJECTPOOL_MEMORY</b><br>
/// Used to specify the memory limits for the default ObjectPool. See
/// ObjectPool::defaultObjectPool() for more information.

/// The ObjectPool class implements a cache of Object instances indexed by their own hash and limited by the memory consumption.
/// The function defaultObjectPool() returns a singleton object that should be used by most of the operations, 
/// so there will be one single place where the total memory used by IECore objects is defined. 
/// 
/// \ingroup utilityGroup
class ObjectPool : public RefCounted
{
	public:

		IE_CORE_DECLAREMEMBERPTR( ObjectPool );

		ObjectPool( size_t maxMemory );
		virtual ~ObjectPool();

		// Clears all the objects in the pool
		void clear();

		// Erases the Object with the given hash if it is held in the pool. Returns whether any item was removed.
		bool erase( const MurmurHash &hash );

		/// Set the maximum memory cost of the items held in the pool, discarding any items if necessary.
		void setMaxMemoryUsage( size_t maxMemory );

		/// Get the maximum possible memory cost of all items held in the pool
		size_t getMaxMemoryUsage() const;

		/// Returns the current memory cost of items held in the pool
		size_t memoryUsage() const;

		/// Returns true if the object with the given hash is in the pool.
		/// Note: this function doesn't garantee that retrieve() will return an object in a multi-threaded application.
		bool contains( const MurmurHash &hash ) const;

		/// Retrieves the Object with the given hash, or NULL if not held in the pool.
		ConstObjectPtr retrieve( const MurmurHash &hash ) const;

		/// Enum used to specify how to store the pointer passed to the store() function.
		enum StoreMode
		{
			StoreCopy = 0,
			StoreReference,
		};

		/// Stores a reference to the object or a copy to the object in the pool, depending on the storeMode parameter. 
		/// If the object passed is already in the pool, than this function will have no effect and will return the stored object instead.
		/// If the storeMode is Reference, then the object should not be modified after the call to this function to 
		/// prevent affecting the contents of the pool and it's memoryUsage count.
		ConstObjectPtr store( const Object *obj, StoreMode mode );

		/// Returns a static ObjectPool instance to be used by anything
		/// wishing to share IECore::Object instances. 
		/// It makes sense to use this wherever possible to conserve memory. This initially
		/// has a memory limit specified in megabytes by the IECORE_OBJECTPOOL_MEMORY
		/// environment variable. If it needs changing it's recommended to do 
		/// that from a config file loaded by the ConfigLoader, to avoid multiple 
		/// clients fighting over the same set of settings.
		static ObjectPoolPtr defaultObjectPool();

	private:

		struct MemberData;
		boost::shared_ptr<MemberData> m_data;
};

} // namespace IECore

#endif // IECORE_OBJECTPOOL_H

