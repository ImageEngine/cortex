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

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/tokenizer.hpp"

#include "OBJ/OBJ_Geometry.h"
#include "OBJ/OBJ_SubNet.h"
#include "PRM/PRM_ChoiceList.h"
#include "SOP/SOP_Node.h"

#include "IECoreHoudini/ModelCacheNode.h"

using namespace IECore;
using namespace IECoreHoudini;

//////////////////////////////////////////////////////////////////////////////////////////
// ModelCacheNode implementation
//////////////////////////////////////////////////////////////////////////////////////////

template<typename BaseType>
ModelCacheNode<BaseType>::ModelCacheNode( OP_Network *net, const char *name, OP_Operator *op ) : BaseType( net, name, op )
{
}

template<typename BaseType>
ModelCacheNode<BaseType>::~ModelCacheNode()
{
}

template<typename BaseType>
PRM_Name ModelCacheNode<BaseType>::pFile( "file", "File" );

template<typename BaseType>
PRM_Name ModelCacheNode<BaseType>::pReload( "reload", "Reload" );

template<typename BaseType>
PRM_Name ModelCacheNode<BaseType>::pRoot( "root", "Root" );

template<typename BaseType>
PRM_Name ModelCacheNode<BaseType>::pSpace( "space", "Space" );

template<typename BaseType>
PRM_Default ModelCacheNode<BaseType>::rootDefault( 0, "/" );

template<typename BaseType>
PRM_Default ModelCacheNode<BaseType>::spaceDefault( World );

static PRM_Name spaceNames[] = {
	PRM_Name( "0", "World" ),
	PRM_Name( "1", "Path" ),
	PRM_Name( "2", "Leaf" ),
	PRM_Name( "3", "Object" ),
	PRM_Name( 0 ) // sentinal
};

template<typename BaseType>
PRM_ChoiceList ModelCacheNode<BaseType>::rootMenu( PRM_CHOICELIST_REPLACE, &ModelCacheNode<BaseType>::buildRootMenu );

template<typename BaseType>
PRM_ChoiceList ModelCacheNode<BaseType>::spaceList( PRM_CHOICELIST_SINGLE, &spaceNames[0] );

template<typename BaseType>
PRM_Template ModelCacheNode<BaseType>::parameters[] = {
	PRM_Template( PRM_FILE | PRM_TYPE_JOIN_NEXT, 1, &pFile ),
	PRM_Template(
		PRM_CALLBACK, 1, &pReload, 0, 0, 0, &ModelCacheNode<BaseType>::reloadButtonCallback, 0, 0,
		"Removes the current MDC file from the cache. This will force a recook on this node, and "
		"cause all other nodes using this MDC file to require a recook as well."
	),
	PRM_Template(
		PRM_STRING, 1, &pRoot, &rootDefault, &rootMenu, 0, 0, 0, 0,
		"Root path inside the MDC of the hierarchy to load"
	),
	PRM_Template(
		PRM_INT, 1, &pSpace, &spaceDefault, &spaceList, 0, 0, 0, 0,
		"Re-orient the objects by choosing a space. World transforms from \"/\" on down the hierarchy, "
		"Path re-roots the transformation starting at the specified root path, Leaf uses the leaf level "
		"transformations only, and Object is an identity transform"
	),
	PRM_Template()
};

template<typename BaseType>
void ModelCacheNode<BaseType>::buildRootMenu( void *data, PRM_Name *menu, int maxSize, const PRM_SpareData *, const PRM_Parm * )
{
	ModelCacheNode<BaseType> *node = reinterpret_cast<ModelCacheNode<BaseType>*>( data );
	if ( !node )
	{
		return;
	}
	
	menu[0].setToken( "/" );
	menu[0].setLabel( "/" );
	
	std::string file;
	if ( !node->ensureFile( file ) )
	{
		// mark the end of our menu
		menu[1].setToken( 0 );
		return;
	}
	
	std::vector<std::string> descendants;
	ModelCacheUtil::Cache::EntryPtr entry = cache().entry( file, "/" );
	node->descendantNames( entry->modelCache(), descendants );
	node->createMenu( menu, descendants );
}

template<typename BaseType>
int ModelCacheNode<BaseType>::reloadButtonCallback( void *data, int index, float time, const PRM_Template *tplate )
{
	std::string file;
	ModelCacheNode<BaseType> *node = reinterpret_cast<ModelCacheNode<BaseType>*>( data );
	if ( !node || !node->ensureFile( file ) )
	{
		return 0;
	}
	
	cache().erase( file );
	node->forceRecook();
	
	return 1;
}

template<typename BaseType>
bool ModelCacheNode<BaseType>::ensureFile( std::string &file )
{
	UT_String value;
	this->evalString( value, pFile.getToken(), 0, 0 );
	file = value.toStdString();
	
	boost::filesystem::path filePath = boost::filesystem::path( file );
	if ( filePath.extension() == ".mdc" && boost::filesystem::exists( filePath ) )
	{
		return true;
	}
	
	return false;
}

template<typename BaseType>
std::string ModelCacheNode<BaseType>::getPath()
{
	UT_String value;
	this->evalString( value, pRoot.getToken(), 0, 0 );
	return ( value == "" ) ? "/" : value.toStdString();
}

template<typename BaseType>
void ModelCacheNode<BaseType>::descendantNames( const IECore::ModelCache *cache, std::vector<std::string> &descendants )
{
	IndexedIO::EntryIDList children;
	cache->childNames( children );
	
	std::string current = ( cache->path() == "/" ) ? "" : cache->path();
	for ( IndexedIO::EntryIDList::const_iterator it=children.begin(); it != children.end(); ++it )
	{
		descendants.push_back( current + "/" + it->value() );
	}
	
	for ( IndexedIO::EntryIDList::const_iterator it=children.begin(); it != children.end(); ++it )
	{
		descendantNames( cache->readableChild( *it ), descendants );
	}
};

template<typename BaseType>
void ModelCacheNode<BaseType>::objectNames( const IECore::ModelCache *cache, std::vector<std::string> &objects )
{
	if ( cache->hasObject() )
	{
		objects.push_back( cache->name() );
	}
	
	IndexedIO::EntryIDList children;
	cache->childNames( children );
	for ( IndexedIO::EntryIDList::const_iterator it=children.begin(); it != children.end(); ++it )
	{
		objectNames( cache->readableChild( *it ), objects );
	}
};

template<typename BaseType>
void ModelCacheNode<BaseType>::createMenu( PRM_Name *menu, const std::vector<std::string> &values )
{
	unsigned pos = 1;
	// currently menus display funny if we exceed 1500 despite the limit being 8191...
	for ( std::vector<std::string>::const_iterator it=values.begin(); ( it != values.end() ) && ( pos < 1500 ); ++it, ++pos )
	{
		menu[pos].setToken( (*it).c_str() );
		menu[pos].setLabel( (*it).c_str() );
	}
	
	// mark the end of our menu
	menu[pos].setToken( 0 );
}

template<typename BaseType>
ModelCacheUtil::Cache &ModelCacheNode<BaseType>::cache()
{
	static ModelCacheUtil::Cache c;
	return c;
}

//////////////////////////////////////////////////////////////////////////////////////////
// ModelCacheUtil Cache implementation
//////////////////////////////////////////////////////////////////////////////////////////

ModelCacheUtil::Cache::Cache() : m_fileCache( fileCacheGetter, 200 )
{
};

ModelCacheUtil::Cache::EntryPtr ModelCacheUtil::Cache::entry( const std::string &fileName, const std::string &path )
{
	FileAndMutexPtr f = m_fileCache.get( fileName );
	EntryPtr result = new Entry( f ); // this locks the mutex for us
	result->m_entry = result->m_fileAndMutex->file;

	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
	Tokenizer tokens( path, boost::char_separator<char>( "/" ) );
	for ( Tokenizer::iterator tIt=tokens.begin(); tIt!=tokens.end(); ++tIt )
	{
		result->m_entry = result->m_entry->readableChild( *tIt );	
	}

	return result;
}

Imath::M44d ModelCacheUtil::Cache::worldTransform( const std::string &fileName, const std::string &path )
{
	EntryPtr thisEntry = entry( fileName, "/" );
	ConstModelCachePtr cache = thisEntry->modelCache();
	Imath::M44d result = cache->readTransform();
	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
	Tokenizer tokens( path, boost::char_separator<char>( "/" ) );
	for ( Tokenizer::iterator tIt=tokens.begin(); tIt!=tokens.end(); ++tIt )
	{
		cache = cache->readableChild( *tIt );
		result = cache->readTransform() * result;
	}

	return result;
}

void ModelCacheUtil::Cache::erase( const std::string &fileName )
{
	m_fileCache.erase( fileName );
}

ModelCacheUtil::Cache::FileAndMutexPtr ModelCacheUtil::Cache::fileCacheGetter( const std::string &fileName, size_t &cost )
{
	FileAndMutexPtr result = new FileAndMutex;
	result->file = new ModelCache( fileName, IndexedIO::Read );
	cost = 1;
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////
// ModelCacheUtil Entry implementation
//////////////////////////////////////////////////////////////////////////////////////////

ModelCacheUtil::Cache::Entry::Entry( FileAndMutexPtr fileAndMutex )
	: m_fileAndMutex( fileAndMutex ), m_lock( m_fileAndMutex->mutex )
{
}

const ModelCache *ModelCacheUtil::Cache::Entry::modelCache()
{
	return m_entry;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Known Specializations
//////////////////////////////////////////////////////////////////////////////////////////

template class ModelCacheNode<OP_Node>;
template class ModelCacheNode<OBJ_Node>;
template class ModelCacheNode<OBJ_Geometry>;
template class ModelCacheNode<OBJ_SubNet>;
template class ModelCacheNode<SOP_Node>;
