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

#ifndef IECOREHOUDINI_MODELCACHENODE_H
#define IECOREHOUDINI_MODELCACHENODE_H

#include "tbb/mutex.h"

#include "PRM/PRM_Name.h"

#include "IECore/LRUCache.h"
#include "IECore/ModelCache.h"

namespace IECoreHoudini
{

namespace ModelCacheUtil
{
class Cache;
}

/// Abstract class for using an IECore::ModelCache in Houdini.
/// Derived nodes will do something useful with the data.
template<typename BaseType>
class ModelCacheNode : public BaseType
{
	public :
		
		ModelCacheNode( OP_Network *net, const char *name, OP_Operator *op );
		virtual ~ModelCacheNode();
		
		static PRM_Template parameters[];
		
		static PRM_Name pFile;
		static PRM_Name pRoot;
		static PRM_Name pSpace;
		static PRM_Name pReload;
		
		static PRM_Default rootDefault;
		static PRM_Default spaceDefault;
		
		static PRM_ChoiceList rootMenu;
		static PRM_ChoiceList spaceList;
		
		static int reloadButtonCallback( void *data, int index, float time, const PRM_Template *tplate );
		static void buildRootMenu( void *data, PRM_Name *menu, int maxSize, const PRM_SpareData *, const PRM_Parm * );
		
		enum Space
		{
			World,
			Path,
			/// \todo: rename to Local
			Leaf,
			/// \todo: rename to Identity?
			Object
		};
		
		/// convienence methods for the common parameters;
		std::string getFile();
		void setFile( std::string file );
		std::string getPath();
		void setPath( std::string path );
		Space getSpace();
		void setSpace( Space space );
	
	protected :
		
		/// Access point to the actual ModelCache. All derived classes should only access the cache
		/// using this method, and must hold onto an EntryPtr retrieved from this utility while
		/// reading the ModelCache.
		static ModelCacheUtil::Cache &cache();
		
		/// get the file and ensure it is a valid MDC
		bool ensureFile( std::string &file );
		/// get a breadth first list of all descendant paths
		void descendantNames( const IECore::ModelCache *cache, std::vector<std::string> &descendants );
		/// get a depth first list of all object names
		void objectNames( const IECore::ModelCache *cache, std::vector<std::string> &objects );
		/// utility method to build a UI menu from one of the previous lists
		void createMenu( PRM_Name *menu, const std::vector<std::string> &values );

};

namespace ModelCacheUtil
{
/// \todo: much of this is copied from GafferScene::ModelCacheSource. Can we put it somewhere common instead?
class Cache
{
	
	private :
	
		IE_CORE_FORWARDDECLARE( FileAndMutex )
	
	public :
	
		Cache();
		
		/// This class provides access to a particular location within the ModelCache,
		/// and ensures that access is threadsafe by holding a mutex on the file.
		class Entry : public IECore::RefCounted
		{
		
			public :
			
				const IECore::ModelCache *modelCache();				
			
			private :
			
				Entry( FileAndMutexPtr fileAndMutex );
				
				FileAndMutexPtr m_fileAndMutex;
				tbb::mutex::scoped_lock m_lock;
				IECore::ConstModelCachePtr m_entry;
				
				friend class Cache;
		
		};
		
		IE_CORE_DECLAREPTR( Entry )
		
		EntryPtr entry( const std::string &fileName, const std::string &path );
		Imath::M44d worldTransform( const std::string &fileName, const std::string &path );
		void erase( const std::string &fileName );
	
	private :
	
		class FileAndMutex : public IECore::RefCounted
		{
			public :
				
				tbb::mutex mutex;
				IECore::ModelCachePtr file;
				
		};
				
		static FileAndMutexPtr fileCacheGetter( const std::string &fileName, size_t &cost );
		
		typedef IECore::LRUCache<std::string, FileAndMutexPtr> FileCache;
		FileCache m_fileCache;

};
}

} // namespace IECoreHoudini

#endif // IECOREHOUDINI_MODELCACHENODE_H
