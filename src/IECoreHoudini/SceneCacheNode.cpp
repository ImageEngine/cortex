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

#include "OBJ/OBJ_Geometry.h"
#include "OBJ/OBJ_SubNet.h"
#include "PRM/PRM_ChoiceList.h"
#include "PRM/PRM_Parm.h"
#include "SOP/SOP_Node.h"

#include "IECore/SharedSceneInterfaces.h"

#include "IECoreHoudini/SceneCacheNode.h"

using namespace IECore;
using namespace IECoreHoudini;

//////////////////////////////////////////////////////////////////////////////////////////
// SceneCacheNode implementation
//////////////////////////////////////////////////////////////////////////////////////////

template<typename BaseType>
SceneCacheNode<BaseType>::SceneCacheNode( OP_Network *net, const char *name, OP_Operator *op ) :
	BaseType( net, name, op ), m_loaded( false ), m_static( boost::indeterminate )
{
	BaseType::flags().setTimeDep( true );
}

template<typename BaseType>
SceneCacheNode<BaseType>::~SceneCacheNode()
{
}

template<typename BaseType>
PRM_Name SceneCacheNode<BaseType>::pFile( "file", "File" );

template<typename BaseType>
PRM_Name SceneCacheNode<BaseType>::pReload( "reload", "Reload" );

template<typename BaseType>
PRM_Name SceneCacheNode<BaseType>::pRoot( "root", "Root" );

template<typename BaseType>
PRM_Name SceneCacheNode<BaseType>::pSpace( "space", "Space" );

template<typename BaseType>
PRM_Name SceneCacheNode<BaseType>::pAttributeFilter( "attributeFilter", "Attribute Filter" );

template<typename BaseType>
PRM_Name SceneCacheNode<BaseType>::pGeometryType( "geometryType", "Geometry Type" );

template<typename BaseType>
PRM_Default SceneCacheNode<BaseType>::rootDefault( 0, "/" );

template<typename BaseType>
PRM_Default SceneCacheNode<BaseType>::spaceDefault( World );

template<typename BaseType>
PRM_Default SceneCacheNode<BaseType>::filterDefault( 0, "*" );

template<typename BaseType>
PRM_Default SceneCacheNode<BaseType>::geometryTypeDefault( Cortex );

static PRM_Name spaceNames[] = {
	PRM_Name( "0", "World" ),
	PRM_Name( "1", "Path" ),
	PRM_Name( "2", "Local" ),
	PRM_Name( "3", "Object" ),
	PRM_Name( 0 ) // sentinal
};

static PRM_Name geometryTypes[] = {
	PRM_Name( "0", "Cortex Primitives" ),
	PRM_Name( "1", "Houdini Geometry" ),
	PRM_Name( 0 ) // sentinal
};

template<typename BaseType>
PRM_ChoiceList SceneCacheNode<BaseType>::rootMenu( PRM_CHOICELIST_REPLACE, &SceneCacheNode<BaseType>::buildRootMenu );

template<typename BaseType>
PRM_ChoiceList SceneCacheNode<BaseType>::spaceList( PRM_CHOICELIST_SINGLE, &spaceNames[0] );

template<typename BaseType>
PRM_ChoiceList SceneCacheNode<BaseType>::geometryTypeList( PRM_CHOICELIST_SINGLE, &geometryTypes[0] );

template<typename BaseType>
PRM_Template SceneCacheNode<BaseType>::parameters[] = {
	PRM_Template(
		PRM_FILE | PRM_TYPE_JOIN_NEXT, 1, &pFile, 0, 0, 0, &SceneCacheNode<BaseType>::sceneParmChangedCallback, 0, 0,
		"A static or animated SCC or LSCC file to load, starting at the Root path provided."
	),
	PRM_Template(
		PRM_CALLBACK, 1, &pReload, 0, 0, 0, &SceneCacheNode<BaseType>::reloadButtonCallback, 0, 0,
		"Removes the current SCC or LSCC file from the cache. This will force a recook on this node, and "
		"cause all other nodes using this file to require a recook as well."
	),
	PRM_Template(
		PRM_STRING, 1, &pRoot, &rootDefault, &rootMenu, 0, &SceneCacheNode<BaseType>::sceneParmChangedCallback, 0, 0,
		"Root path inside the SCC or LSCC of the hierarchy to load"
	),
	PRM_Template(
		PRM_INT, 1, &pSpace, &spaceDefault, &spaceList, 0, 0, 0, 0,
		"Re-orient the objects by choosing a space. World transforms from \"/\" on down the hierarchy, "
		"Path re-roots the transformation starting at the specified root path, Local uses the current level "
		"transformations only, and Object is an identity transform"
	),
	PRM_Template(
		PRM_INT, 1, &pGeometryType, &geometryTypeDefault, &geometryTypeList, 0, 0, 0, 0,
		"The type of geometry to load. Cortex Primitives are faster, but only allow manipulation through "
		"OpHolders or specificly designed nodes. Houdini Geometry will use the converters to create standard "
		"geo that can be manipulated anywhere."
	),
	PRM_Template(
		PRM_STRING, 1, &pAttributeFilter, &filterDefault, 0, 0, 0, 0, 0,
		"A list of attribute names to load, if they exist on each shape. Uses Houdini matching syntax. "
		"The filter expects Cortex names as exist in the cache, and performs automated conversion to standard Houdini Attributes (i.e. Pref->rest ; Cs->Cd ; s,t->uv). "
		"P will always be loaded."
	),
	PRM_Template()
};

template<typename BaseType>
void SceneCacheNode<BaseType>::buildRootMenu( void *data, PRM_Name *menu, int maxSize, const PRM_SpareData *, const PRM_Parm * )
{
	SceneCacheNode<BaseType> *node = reinterpret_cast<SceneCacheNode<BaseType>*>( data );
	if ( !node )
	{
		return;
	}
	
	menu[0].setToken( SceneInterface::rootName.c_str() );
	menu[0].setLabel( SceneInterface::rootName.c_str() );
	
	std::string file;
	if ( !node->ensureFile( file ) )
	{
		// mark the end of our menu
		menu[1].setToken( 0 );
		return;
	}
	
	std::vector<std::string> descendants;
	node->descendantNames( node->scene( file, SceneInterface::rootName ), descendants );
	node->createMenu( menu, descendants );
}

template<typename BaseType>
int SceneCacheNode<BaseType>::sceneParmChangedCallback( void *data, int index, float time, const PRM_Template *tplate )
{
	SceneCacheNode<BaseType> *node = reinterpret_cast<SceneCacheNode<BaseType>*>( data );
	if ( !node )
	{
		return 0;
	}
	
	node->sceneChanged();
	
	return 1;
}

template<typename BaseType>
int SceneCacheNode<BaseType>::reloadButtonCallback( void *data, int index, float time, const PRM_Template *tplate )
{
	std::string file;
	SceneCacheNode<BaseType> *node = reinterpret_cast<SceneCacheNode<BaseType>*>( data );
	if ( !node || !node->ensureFile( file ) )
	{
		return 0;
	}
	
	SharedSceneInterfaces::erase( file );
	node->sceneChanged();
	node->forceRecook();
	
	return 1;
}

template<typename BaseType>
void SceneCacheNode<BaseType>::sceneChanged()
{
	m_loaded = false;
}

template<typename BaseType>
bool SceneCacheNode<BaseType>::ensureFile( std::string &file )
{
	file = getFile();
	
	boost::filesystem::path filePath = boost::filesystem::path( file );
	std::vector<std::string> extensions = SceneInterface::supportedExtensions( IndexedIO::Read );
	if ( filePath.has_extension() && std::find( extensions.begin(), extensions.end(), filePath.extension().string().substr( 1 ) ) != extensions.end() && boost::filesystem::exists( filePath ) )
	{
		return true;
	}
	
	return false;
}

template<typename BaseType>
std::string SceneCacheNode<BaseType>::getFile() const
{
	UT_String value;
	this->evalString( value, pFile.getToken(), 0, 0 );
	return value.toStdString();
}

template<typename BaseType>
void SceneCacheNode<BaseType>::setFile( std::string file )
{
	this->setString( UT_String( file ), CH_STRING_LITERAL, pFile.getToken(), 0, 0 );
	sceneChanged();
}

template<typename BaseType>
std::string SceneCacheNode<BaseType>::getPath() const
{
	UT_String value;
	this->evalString( value, pRoot.getToken(), 0, 0 );
	return ( value == "" ) ? "/" : value.toStdString();
}

template<typename BaseType>
void SceneCacheNode<BaseType>::setPath( const IECore::SceneInterface *scene )
{
	std::string str;
	SceneInterface::Path p;
	scene->path( p );
	SceneInterface::pathToString( p, str );
	
	this->setString( UT_String( str ), CH_STRING_LITERAL, pRoot.getToken(), 0, 0 );
	sceneChanged();
}

template<typename BaseType>
typename SceneCacheNode<BaseType>::Space SceneCacheNode<BaseType>::getSpace() const
{
	return (Space)this->evalInt( pSpace.getToken(), 0, 0 );
}

template<typename BaseType>
void SceneCacheNode<BaseType>::setSpace( SceneCacheNode<BaseType>::Space space )
{
	this->setInt( pSpace.getToken(), 0, 0, space );
}

template<typename BaseType>
typename SceneCacheNode<BaseType>::GeometryType SceneCacheNode<BaseType>::getGeometryType() const
{
	return (GeometryType)this->evalInt( pGeometryType.getToken(), 0, 0 );
}

template<typename BaseType>
void SceneCacheNode<BaseType>::setGeometryType( SceneCacheNode<BaseType>::GeometryType type )
{
	this->setInt( pGeometryType.getToken(), 0, 0, type );
}

template<typename BaseType>
void SceneCacheNode<BaseType>::getAttributeFilter( UT_String &filter ) const
{
	this->evalString( filter, pAttributeFilter.getToken(), 0, 0 );
}

template<typename BaseType>
void SceneCacheNode<BaseType>::setAttributeFilter( const UT_String &filter )
{
	this->setString( filter, CH_STRING_LITERAL, pAttributeFilter.getToken(), 0, 0 );
}

template<typename BaseType>
void SceneCacheNode<BaseType>::referenceParent( const char *parmName )
{
	this->getParm( parmName ).setChannelReference( 0, 0, ( std::string( "../" ) + parmName ).c_str() );
	sceneChanged();
}

template<typename BaseType>
void SceneCacheNode<BaseType>::descendantNames( const IECore::SceneInterface *scene, std::vector<std::string> &descendants )
{
	SceneInterface::NameList children;
	scene->childNames( children );
	
	std::string current = "";
	if ( scene->name() != SceneInterface::rootName )
	{
		SceneInterface::Path p;
		scene->path( p );
		SceneInterface::pathToString( p, current );
	}
	
	for ( SceneInterface::NameList::const_iterator it=children.begin(); it != children.end(); ++it )
	{
		descendants.push_back( current + "/" + it->value() );
	}
	
	for ( SceneInterface::NameList::const_iterator it=children.begin(); it != children.end(); ++it )
	{
		descendantNames( scene->child( *it ), descendants );
	}
};

template<typename BaseType>
void SceneCacheNode<BaseType>::objectNames( const IECore::SceneInterface *scene, std::vector<std::string> &objects )
{
	if ( scene->hasObject() )
	{
		objects.push_back( scene->name() );
	}
	
	SceneInterface::NameList children;
	scene->childNames( children );
	for ( SceneInterface::NameList::const_iterator it=children.begin(); it != children.end(); ++it )
	{
		objectNames( scene->child( *it ), objects );
	}
};

template<typename BaseType>
void SceneCacheNode<BaseType>::createMenu( PRM_Name *menu, const std::vector<std::string> &values )
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
ConstSceneInterfacePtr SceneCacheNode<BaseType>::scene()
{
	try
	{
		return this->scene( getFile(), getPath() );
	}
	catch( ... )
	{
		return 0;
	}
	
	return 0;
}

template<typename BaseType>
ConstSceneInterfacePtr SceneCacheNode<BaseType>::scene( const std::string &fileName, const std::string &path )
{
	ConstSceneInterfacePtr result = SharedSceneInterfaces::get( fileName );
	if ( path != SceneInterface::rootName.string() )
	{
		SceneInterface::Path p;
		SceneInterface::stringToPath( path, p );
		result = result->scene( p, SceneInterface::NullIfMissing );
	}
	
	return result;
}

template<typename BaseType>
Imath::M44d SceneCacheNode<BaseType>::worldTransform( const std::string &fileName, const std::string &path, double time )
{
	ConstSceneInterfacePtr scene = this->scene( fileName, SceneInterface::rootName );
	
	SceneInterface::Path p;
	SceneInterface::stringToPath( path, p );
	Imath::M44d result = scene->readTransformAsMatrix( time );
	for ( SceneInterface::Path::const_iterator it = p.begin(); scene && it != p.end(); ++it )
	{
		scene = scene->child( *it, SceneInterface::NullIfMissing );
		if ( !scene )
		{
			break;
		}
		
		result = scene->readTransformAsMatrix( time ) * result;
	}
	
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Known Specializations
//////////////////////////////////////////////////////////////////////////////////////////

template class SceneCacheNode<OP_Node>;
template class SceneCacheNode<OBJ_Node>;
template class SceneCacheNode<OBJ_Geometry>;
template class SceneCacheNode<OBJ_SubNet>;
template class SceneCacheNode<SOP_Node>;
