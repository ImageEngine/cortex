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

#include "OpenEXR/ImathBoxAlgo.h"
#include "OpenEXR/ImathMatrixAlgo.h"

#include "OBJ/OBJ_Node.h" 
#include "OP/OP_Director.h" 
#include "OP/OP_Input.h" 
#include "MGR/MGR_Node.h" 
#include "MOT/MOT_Director.h" 
#include "UT/UT_WorkArgs.h" 

#include "IECore/Group.h"
#include "IECore/TransformationMatrixData.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/GU_CortexPrimitive.h"
#include "IECoreHoudini/HoudiniScene.h"
#include "IECoreHoudini/FromHoudiniGeometryConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

static InternedString contentName( "geo" );

HoudiniScene::HoudiniScene() : m_rootIndex( 0 ), m_contentIndex( 0 )
{
	MOT_Director *motDirector = dynamic_cast<MOT_Director *>( OPgetDirector() );
	motDirector->getObjectManager()->getFullPath( m_nodePath );
	
	Path contentPath, rootPath;
	calculatePath( contentPath, rootPath );
}

HoudiniScene::HoudiniScene( const UT_String &nodePath, const Path &contentPath, const Path &rootPath ) : m_rootIndex( 0 ), m_contentIndex( 0 )
{
	m_nodePath = nodePath;
	m_nodePath.hardenIfNeeded();
	
	OP_Node *contentNode = locateContent( retrieveNode() );
	if ( contentNode )
	{
		contentNode->getFullPath( m_contentPath );
		m_contentPath.hardenIfNeeded();
	}
	
	calculatePath( contentPath, rootPath );
	
	// make sure the node is valid
	OP_Node *node = retrieveNode();
	if ( !node->isManager() && !node->castToOBJNode() )
	{
		throw Exception( "IECoreHoudini::HoudiniScene: Node \"" + m_nodePath.toStdString() + "\" is not a valid OBJ." );
	}
}

HoudiniScene::~HoudiniScene()
{
}

const OP_Node *HoudiniScene::node() const
{
	return retrieveNode( false, NullIfMissing );
}

std::string HoudiniScene::fileName() const
{
	throw Exception( "HoudiniScene does not support fileName()." );
}

SceneInterface::Name HoudiniScene::name() const
{
	if ( m_path.empty() || m_rootIndex == m_path.size() )
	{
		return SceneInterface::rootName;
	}
	
	return *m_path.rbegin();
}

void HoudiniScene::path( Path &p ) const
{
	p.resize( m_path.size() - m_rootIndex );
	std::copy( m_path.begin() + m_rootIndex, m_path.end(), p.begin() );
}

void HoudiniScene::calculatePath( const Path &contentPath, const Path &rootPath )
{
	OP_Node *node = OPgetDirector()->findNode( m_nodePath );
	if ( !node )
	{
		throw Exception( "IECoreHoudini::HoudiniScene: Node \"" + m_nodePath.toStdString() + "\" no longer exists." );
	}
	
	if ( node->isManager() )
	{
		return;
	}
	
	UT_String tmp( m_nodePath );
	UT_WorkArgs workArgs;
	tmp.tokenize( workArgs, "/" );
	
	OP_Node *current = dynamic_cast<MOT_Director *>( OPgetDirector() )->getObjectManager();
	// skipping the token for the OBJ manager
	for ( int i = 1; i < workArgs.getArgc(); ++i )
	{
		current = current->getChild( workArgs[i] );
		
		/// recursively collect all input connections
		OP_Node *parent = current->getInput( 0 );
		UT_StringArray parentNames;
		while ( parent )
		{
			parentNames.append( parent->getName() );
			parent = parent->getInput( 0 );
		}
		
		// add them in reverse order
		for ( int j = parentNames.entries() - 1; j >= 0; --j )
		{
			m_path.push_back( Name( parentNames( j ) ) );
		}
		
		if ( ( i < workArgs.getArgc() - 1 ) || Name( workArgs[i] ) != contentName )
		{
			m_path.push_back( Name( workArgs[i] ) );
		}
	}
	
	if ( !contentPath.empty() )
	{
		m_contentIndex = m_path.size();
		m_path.resize( m_path.size() + contentPath.size() );
		std::copy( contentPath.begin(), contentPath.end(), m_path.begin() + m_contentIndex );
	}
	
	if ( m_path.size() < rootPath.size() )
	{
		std::string pStr, rStr;
		pathToString( m_path, pStr );
		pathToString( rootPath, rStr );
		throw Exception( "IECoreHoudini::HoudiniScene: Path \"" + pStr + "\" is not a valid child of root \"" + rStr + "\"." );
	}
	
	for ( size_t i = 0; i < rootPath.size(); ++i )
	{
		if ( rootPath[i] != m_path[i] )
		{
			std::string pStr, rStr;
			pathToString( m_path, pStr );
			pathToString( rootPath, rStr );
			throw Exception( "IECoreHoudini::HoudiniScene: Path \"" + pStr + "\" is not a valid child of root \"" + rStr + "\"." );
		}
	}
	
	m_rootIndex = rootPath.size();
}

Imath::Box3d HoudiniScene::readBound( double time ) const
{
	OP_Node *node = retrieveNode( true );
	
	Imath::Box3d bounds;
	UT_BoundingBox box;
	OP_Context context( time );
	/// \todo: this doesn't account for SOPs containing multiple shapes
	/// if we fix it, we need to fix the condition below as well
	if ( node->getBoundingBox( box, context ) )
	{
		bounds = IECore::convert<Imath::Box3d>( box );
	}
	
	// paths embedded within a sop already have bounds accounted for
	if ( m_contentIndex )
	{
		return bounds;
	}
	
	NameList children;
	childNames( children );
	for ( NameList::iterator it=children.begin(); it != children.end(); ++it )
	{
		ConstSceneInterfacePtr childScene = child( *it );
		Imath::Box3d childBound = childScene->readBound( time );
		if ( !childBound.isEmpty() )
		{
			bounds.extendBy( Imath::transform( childBound, childScene->readTransformAsMatrix( time ) ) );
		}
	}
	
	return bounds;
}

void HoudiniScene::writeBound( const Imath::Box3d &bound, double time )
{
	throw Exception( "IECoreHoudini::HoudiniScene is read-only" );
}

ConstDataPtr HoudiniScene::readTransform( double time ) const
{
	Imath::V3d s, h, r, t;
	Imath::M44d matrix = readTransformAsMatrix( time );
	Imath::extractSHRT( matrix, s, h, r, t, true );
	
	return new TransformationMatrixdData( TransformationMatrixd( s, r, t ) );
}

Imath::M44d HoudiniScene::readTransformAsMatrix( double time ) const
{
	OP_Node *node = retrieveNode();	
	if ( node->isManager() )
	{
		return Imath::M44d();
	}
	
	OBJ_Node *objNode = node->castToOBJNode();
	if ( !objNode )
	{
		return Imath::M44d();
	}
	
	// paths embedded within a sop always have identity transforms
	if ( m_contentIndex )
	{
		return Imath::M44d();
	}
	
	UT_DMatrix4 matrix;
	OP_Context context( time );
	if ( !objNode->getLocalTransform( context, matrix ) )
	{
		return Imath::M44d();
	}
	
	return IECore::convert<Imath::M44d>( matrix );
}

void HoudiniScene::writeTransform( const Data *transform, double time )
{
	throw Exception( "IECoreHoudini::HoudiniScene is read-only" );
}

bool HoudiniScene::hasAttribute( const Name &name ) const
{
	std::map<Name, CustomReader>::const_iterator it = customAttributeReaders().find( name );
	if ( it != customAttributeReaders().end() )
	{
		return it->second.m_has( retrieveNode() );
	}
	
	return false;
}

void HoudiniScene::attributeNames( NameList &attrs ) const
{
	attrs.clear();
	for ( std::map<Name, CustomReader>::const_iterator it = customAttributeReaders().begin(); it != customAttributeReaders().end(); ++it )
	{
		if ( it->second.m_has( retrieveNode() ) )
		{
			attrs.push_back( it->first );
		}
	}
}

ConstObjectPtr HoudiniScene::readAttribute( const Name &name, double time ) const
{
	std::map<Name, CustomReader>::const_iterator it = customAttributeReaders().find( name );
	if ( it != customAttributeReaders().end() )
	{
		return it->second.m_read( retrieveNode() );
	}
	
	return 0;
}

void HoudiniScene::writeAttribute( const Name &name, const Object *attribute, double time )
{
	throw Exception( "IECoreHoudini::HoudiniScene is read-only" );
}

bool HoudiniScene::hasTag( const Name &name, bool includeChildren ) const
{
	const OP_Node *node = retrieveNode();
	if ( !node )
	{
		return false;
	}
	
	std::vector<CustomTagReader> &tagReaders = customTagReaders();
	for ( std::vector<CustomTagReader>::const_iterator it = tagReaders.begin(); it != tagReaders.end(); ++it )
	{
		if ( it->m_has( node, name ) )
		{
			return true;
		}
	}
	
	return false;
}

void HoudiniScene::readTags( NameList &tags, bool includeChildren ) const
{
	tags.clear();
	
	const OP_Node *node = retrieveNode();
	if ( !node )
	{
		return;
	}
	
	std::vector<CustomTagReader> &tagReaders = customTagReaders();
	for ( std::vector<CustomTagReader>::const_iterator it = tagReaders.begin(); it != tagReaders.end(); ++it )
	{
		NameList values;
		it->m_read( node, values, includeChildren );
		tags.insert( tags.end(), values.begin(), values.end() );
	}
}

void HoudiniScene::writeTags( const NameList &tags )
{
	throw Exception( "HoudiniScene::writeTags not supported" );
}

bool HoudiniScene::hasObject() const
{
	OP_Node *node = retrieveNode( true );
	if ( node->isManager() )
	{
		return false;
	}
	
	OBJ_Node *objNode = node->castToOBJNode();
	if ( !objNode )
	{
		return false;
	}
	
	OBJ_OBJECT_TYPE type = objNode->getObjectType();
	if ( type == OBJ_GEOMETRY  )
	{
		OP_Context context( CHgetEvalTime() );
		const GU_Detail *geo = objNode->getRenderGeometry( context );
		// multiple named shapes define children that contain each object
		/// \todo: similar attribute logic is repeated in several places. unify in a single function if possible
		const GEO_AttributeHandle attrHandle = geo->getPrimAttribute( "name" );
		if ( !attrHandle.isAttributeValid() )
		{
			return true;
		}
		
		const GA_ROAttributeRef attrRef( attrHandle.getAttribute() );
		int numShapes = geo->getUniqueValueCount( attrRef );
		if ( !numShapes )
		{
			return true;
		}
		
		for ( int i=0; i < numShapes; ++i )
		{
			Path childPath;
			bool valid = relativePath( geo->getUniqueStringValue( attrRef, i ), childPath );
			if ( valid && childPath.empty() )
			{
				return true;
			}
		}
		
		return false;
	}
	
	/// \todo: need to account for OBJ_CAMERA and OBJ_LIGHT
	
	return false;
}

ConstObjectPtr HoudiniScene::readObject( double time ) const
{
	OBJ_Node *objNode = retrieveNode( true )->castToOBJNode();
	if ( !objNode )
	{
		return 0;
	}
	
	if ( objNode->getObjectType() == OBJ_GEOMETRY )
	{
		OP_Context context( time );
		GU_DetailHandle handle = objNode->getRenderGeometryHandle( context );
		GU_DetailHandleAutoReadLock readHandle( handle );
		const GU_Detail *geo = readHandle.getGdp();
		if ( !geo )
		{
			return 0;
		}
		
		UT_StringMMPattern nameFilter;
		nameFilter.compile( contentPathValue() );
		GU_DetailHandle newHandle = FromHoudiniGeometryConverter::extract( geo, nameFilter );
		FromHoudiniGeometryConverterPtr converter = FromHoudiniGeometryConverter::create( ( newHandle.isNull() ) ? handle : newHandle );
		if ( !converter )
		{
			return 0;
		}
		
		return converter->convert();
	}
	
	/// \todo: need to account for cameras and lights
	
	return 0;
}

PrimitiveVariableMap HoudiniScene::readObjectPrimitiveVariables( const std::vector<InternedString> &primVarNames, double time ) const
{
	// \todo Optimize this function, adding special cases such as for Meshes.
	ConstPrimitivePtr prim = runTimeCast< const Primitive >( readObject( time ) );
	if ( !prim )
	{
		throw Exception( "Object does not have primitive variables!" );
	}
	return prim->variables;
}

void HoudiniScene::writeObject( const Object *object, double time )
{
	throw Exception( "IECoreHoudini::HoudiniScene is read-only" );
}

void HoudiniScene::childNames( NameList &childNames ) const
{
	OP_Node *node = retrieveNode();
	OBJ_Node *objNode = node->castToOBJNode();
	OBJ_Node *contentNode = retrieveNode( true )->castToOBJNode();
	
	// add subnet children
	if ( node->isManager() || ( objNode && objNode->getObjectType() == OBJ_SUBNET ) )
	{
		for ( int i=0; i < node->getNchildren(); ++i )
		{
			OP_Node *child = node->getChild( i );
			
			// ignore children that have incoming connections, as those are actually grandchildren
			// also ignore the contentNode, which is actually an extension of ourself
			if ( child != contentNode && !hasInput( child ) )
			{
				childNames.push_back( Name( child->getName() ) );
			}
		}
	}
	
	if ( !contentNode )
	{
		return;
	}
	
	// add connected outputs
	for ( unsigned i=0; i < contentNode->nOutputs(); ++i )
	{
		childNames.push_back( Name( contentNode->getOutput( i )->getName() ) );
	}
	
	// add child shapes within the geometry
	if ( contentNode->getObjectType() == OBJ_GEOMETRY )
	{
		OP_Context context( CHgetEvalTime() );
		const GU_Detail *geo = contentNode->getRenderGeometry( context );
		const GEO_AttributeHandle attrHandle = geo->getPrimAttribute( "name" );
		if ( !attrHandle.isAttributeValid() )
		{
			return;
		}
		
		const GA_ROAttributeRef attrRef( attrHandle.getAttribute() );
		int numShapes = geo->getUniqueValueCount( attrRef );
		for ( int i=0; i < numShapes; ++i )
		{
			Path childPath;
			bool valid = relativePath( geo->getUniqueStringValue( attrRef, i ), childPath );
			if ( valid && !childPath.empty() && std::find( childNames.begin(), childNames.end(), *childPath.begin() ) == childNames.end() )
			{
				childNames.push_back( *childPath.begin() );
			}
		}
	}
}

bool HoudiniScene::hasChild( const Name &name ) const
{
	Path contentPath;
	return (bool)retrieveChild( name, contentPath, NullIfMissing );
}

SceneInterfacePtr HoudiniScene::child( const Name &name, MissingBehaviour missingBehaviour )
{
	Path contentPath;
	OP_Node *child = retrieveChild( name, contentPath, missingBehaviour );
	if ( !child )
	{
		return 0;
	}
	
	UT_String nodePath;
	child->getFullPath( nodePath );
	
	Path rootPath;
	rootPath.resize( m_rootIndex );
	std::copy( m_path.begin(), m_path.begin() + m_rootIndex, rootPath.begin() );
	
	/// \todo: is this really what we want? can we just pass rootIndex and contentIndex instead?
	return new HoudiniScene( nodePath, contentPath, rootPath );
}

ConstSceneInterfacePtr HoudiniScene::child( const Name &name, MissingBehaviour missingBehaviour ) const
{
	return const_cast<HoudiniScene *>( this )->child( name, missingBehaviour );
}

SceneInterfacePtr HoudiniScene::createChild( const Name &name )
{
	throw Exception( "IECoreHoudini::HoudiniScene is read-only" );
}

ConstSceneInterfacePtr HoudiniScene::scene( const Path &path, MissingBehaviour missingBehaviour ) const
{
	return retrieveScene( path, missingBehaviour );
}

SceneInterfacePtr HoudiniScene::scene( const Path &path, MissingBehaviour missingBehaviour )
{
	return retrieveScene( path, missingBehaviour );
}

OP_Node *HoudiniScene::retrieveNode( bool content, MissingBehaviour missingBehaviour ) const
{
	OP_Node *node = OPgetDirector()->findNode( m_nodePath );
	if ( !node && missingBehaviour == ThrowIfMissing )
	{
		throw Exception( "IECoreHoudini::HoudiniScene: Node \"" + m_nodePath.toStdString() + "\" no longer exists." );
	}
	
	OP_Node *contentNode = 0;
	UT_String contentPath = m_contentPath;
	if ( m_contentPath.length() )
	{
		contentNode = OPgetDirector()->findNode( m_contentPath );
	}
	else
	{
		contentNode = node;
		contentPath = m_nodePath;
	}
	
	if ( content )
	{
		if ( !contentNode && missingBehaviour == ThrowIfMissing )
		{
			throw Exception( "IECoreHoudini::HoudiniScene: Node \"" + contentPath.toStdString() + "\" no longer exists." );
		}
		
		node = contentNode;
	}
	
	if ( m_contentIndex )
	{
		OBJ_Node *objNode = contentNode->castToOBJNode();
		if ( objNode && objNode->getObjectType() == OBJ_GEOMETRY )
		{
			OP_Context context( CHgetEvalTime() );
			const GU_Detail *geo = objNode->getRenderGeometry( context );
			const GEO_AttributeHandle attrHandle = geo->getPrimAttribute( "name" );
			if ( attrHandle.isAttributeValid() )
			{
				const GA_ROAttributeRef attrRef( attrHandle.getAttribute() );
				int numShapes = geo->getUniqueValueCount( attrRef );
				size_t contentSize = m_path.size() - m_contentIndex;
				for ( int i=0; i < numShapes; ++i )
				{
					Path childPath;
					stringToPath( geo->getUniqueStringValue( attrRef, i ), childPath );
					if ( childPath.empty() || name() == *( childPath.begin() + contentSize - 1 ) )
					{
						return node;
					}
				}
				
				if ( missingBehaviour == ThrowIfMissing )
				{
					throw Exception( "IECoreHoudini::HoudiniScene: Node \"" + contentPath.toStdString() + "\" does not contain the expected geometry for \"" + name().string() + "\"." );
				}
				
				return 0;
			}
		}
	}
	
	return node;
}

OP_Node *HoudiniScene::locateContent( OP_Node *node ) const
{
	OBJ_Node *objNode = node->castToOBJNode();
	if ( node->isManager() || ( objNode && objNode->getObjectType() == OBJ_SUBNET ) )
	{
		for ( int i=0; i < node->getNchildren(); ++i )
		{
			OP_Node *child = node->getChild( i );
			if ( child->getName().equal( contentName.c_str() ) )
			{
				return child;
			}
		}
	}
	
	return 0;
}

OP_Node *HoudiniScene::retrieveChild( const Name &name, Path &contentPath, MissingBehaviour missingBehaviour ) const
{
	OP_Node *node = retrieveNode( false, missingBehaviour );
	OP_Node *contentBaseNode = retrieveNode( true, missingBehaviour );
	if ( !node || !contentBaseNode )
	{
		return 0;
	}
	
	OBJ_Node *objNode = node->castToOBJNode();
	OBJ_Node *contentNode = contentBaseNode->castToOBJNode();
	
	// check subnet children
	if ( node->isManager() || ( objNode && objNode->getObjectType() == OBJ_SUBNET ) )
	{
		for ( int i=0; i < node->getNchildren(); ++i )
		{
			OP_Node *child = node->getChild( i );
			// the contentNode is actually an extension of ourself
			if ( child == contentNode )
			{
				continue;
			}
			
			if ( child->getName().equal( name.c_str() ) && !hasInput( child ) )
			{
				return child;
			}
		}
	}
	
	if ( contentNode )
	{
		// check connected outputs
		for ( unsigned i=0; i < contentNode->nOutputs(); ++i )
		{
			OP_Node *child = contentNode->getOutput( i );
			if ( child->getName().equal( name.c_str() ) )
			{
				return child;
			}
		}
		
		// check child shapes within the geo
		if ( contentNode->getObjectType() == OBJ_GEOMETRY )
		{
			OP_Context context( CHgetEvalTime() );
			const GU_Detail *geo = contentNode->getRenderGeometry( context );
			const GEO_AttributeHandle attrHandle = geo->getPrimAttribute( "name" );
			if ( attrHandle.isAttributeValid() )
			{
				const GA_ROAttributeRef attrRef( attrHandle.getAttribute() );
				int numShapes = geo->getUniqueValueCount( attrRef );
				for ( int i=0; i < numShapes; ++i )
				{
					SceneInterface::Path childPath;
					bool valid = relativePath( geo->getUniqueStringValue( attrRef, i ), childPath );
					if ( valid && !childPath.empty() && name == *childPath.begin() )
					{
						size_t contentSize = ( m_contentIndex ) ? m_path.size() - m_contentIndex : 0;
						if ( contentSize )
						{
							contentPath.resize( contentSize );
							std::copy( m_path.begin() + m_contentIndex, m_path.end(), contentPath.begin() );
						}
						
						contentPath.push_back( name );
						
						return contentNode;
					}
				}
			}
		}
	}
	
	if ( missingBehaviour == SceneInterface::ThrowIfMissing )
	{
		Path p;
		path( p );
		std::string pStr;
		pathToString( p, pStr );
		throw Exception( "IECoreHoudini::HoudiniScene::retrieveChild: Path \"" + pStr + "\" has no child named " + name.string() + "." );
	}
	
	return 0;
}

SceneInterfacePtr HoudiniScene::retrieveScene( const Path &path, MissingBehaviour missingBehaviour ) const
{
	Path rootPath, emptyPath;
	rootPath.resize( m_rootIndex );
	std::copy( m_path.begin(), m_path.begin() + m_rootIndex, rootPath.begin() );
	
	HoudiniScenePtr rootScene = new HoudiniScene();
	for ( Path::const_iterator it = rootPath.begin(); it != rootPath.end(); ++it )
	{
		rootScene = IECore::runTimeCast<HoudiniScene>( rootScene->child( *it ) );
		if ( !rootScene )
		{
			return 0;
		}
	}
	
	UT_String rootNodePath;
	OP_Node *node = rootScene->retrieveNode();
	if ( !node )
	{
		return 0;
	}
	node->getFullPath( rootNodePath );
	
	/// \todo: is this really what we want? can we just pass rootIndex and contentIndex instead?
	SceneInterfacePtr scene = new HoudiniScene( rootNodePath, emptyPath, rootPath );
	for ( Path::const_iterator it = path.begin(); it != path.end(); ++it )
	{
		scene = scene->child( *it, missingBehaviour );
		if ( !scene )
		{
			return 0;
		}
	}
	
	return scene;
}

bool HoudiniScene::hasInput( const OP_Node *node ) const
{
	int numInputs = node->nInputs();
	for ( int j=0; j < numInputs; ++j )
	{
		OP_Input *input = node->getInputReferenceConst( j );
		if ( input && !input->isIndirect() )
		{
			return true;
		}
	}
	
	return false;
}

bool HoudiniScene::relativePath( const char *value, Path &result ) const
{
	Path path;
	std::string pStr = value;
	stringToPath( pStr, path );
	
	size_t contentSize = ( m_contentIndex ) ? m_path.size() - m_contentIndex : 0;
	if ( contentSize > path.size() )
	{
		return false;
	}
	
	Path::iterator start = path.begin() + contentSize;
	if ( path.end() - start > 0 )
	{
		result.resize( path.end() - start );
		std::copy( start, path.end(), result.begin() );
	}
	
	// verify the pre-path matches
	Path::const_iterator mIt = m_path.begin() + m_contentIndex;
	for ( Path::iterator it = path.begin(); it != start && mIt != m_path.end(); ++it, ++mIt )
	{
		if ( *it != *mIt )
		{
			return false;
		}
	}
	
	return true;
}

const char *HoudiniScene::contentPathValue() const
{
	if ( !m_contentIndex )
	{
		return rootName.c_str();
	}
	
	Path relative;
	std::string name;
	relative.resize( m_path.size() - m_contentIndex );
	std::copy( m_path.begin() + m_contentIndex, m_path.end(), relative.begin() );
	pathToString( relative, name );
	return name.c_str();
}

void HoudiniScene::registerCustomAttribute( const Name &attrName, HasFn hasFn, ReadFn readFn )
{
	CustomReader r;
	r.m_has = hasFn;
	r.m_read = readFn;
	customAttributeReaders()[attrName] = r;
}

std::map<SceneInterface::Name, HoudiniScene::CustomReader> &HoudiniScene::customAttributeReaders()
{
	static std::map<SceneInterface::Name, HoudiniScene::CustomReader> readers;
	return readers;
}

void HoudiniScene::registerCustomTags( HasTagFn hasFn, ReadTagsFn readFn )
{
	CustomTagReader r;
	r.m_has = hasFn;
	r.m_read = readFn;
	customTagReaders().push_back( r );
}

std::vector<HoudiniScene::CustomTagReader> &HoudiniScene::customTagReaders()
{
	static std::vector<HoudiniScene::CustomTagReader> readers;
	return readers;
}
