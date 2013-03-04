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
#include "MGR/MGR_Node.h" 
#include "MOT/MOT_Director.h" 
#include "UT/UT_WorkArgs.h" 

#include "IECore/Group.h"
#include "IECore/TransformationMatrixData.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/HoudiniScene.h"
#include "IECoreHoudini/FromHoudiniGeometryConverter.h"

using namespace IECore;
using namespace IECoreHoudini;

SceneInterface::FileFormatDescription< HoudiniScene > HoudiniScene::s_description( ".hip", IndexedIO::Read );

HoudiniScene::HoudiniScene()
{
	MOT_Director *motDirector = dynamic_cast<MOT_Director *>( OPgetDirector() );
	motDirector->getObjectManager()->getFullPath( m_nodePath );
}

HoudiniScene::HoudiniScene( const std::string &fileName, IndexedIO::OpenMode )
{
	MOT_Director *motDirector = dynamic_cast<MOT_Director *>( OPgetDirector() );
	motDirector->getObjectManager()->getFullPath( m_nodePath );
}

HoudiniScene::HoudiniScene( const UT_String &nodePath, const Path &relativePath )
{
	m_nodePath = nodePath;
	m_nodePath.hardenIfNeeded();
	
	OP_Node *contentNode = locateContent( retrieveNode() );
	if ( contentNode )
	{
		contentNode->getFullPath( m_contentPath );
		m_contentPath.hardenIfNeeded();
	}
	
	std::copy( relativePath.begin(), relativePath.end(), m_relativePath.begin() );
	
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

SceneInterface::Name HoudiniScene::name() const
{
	if ( m_relativePath.empty() )
	{
		if ( m_nodePath.length() == 1 || m_nodePath == "/obj" )
		{
			return SceneInterface::rootName;
		}
		
		UT_String path, name;
		m_nodePath.splitPath( path, name );
		return name.toStdString();
	}
	
	return *m_relativePath.rbegin();
}

void HoudiniScene::path( Path &p ) const
{
	p.clear();
	
	OP_Node *node = retrieveNode();
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
			p.push_back( Name( parentNames( j ) ) );
		}
		
		p.push_back( Name( workArgs[i] ) );
	}
	
	if ( !m_relativePath.empty() )
	{
		std::copy( m_relativePath.begin(), m_relativePath.end(), p.end() );
	}
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
	if ( !m_relativePath.empty() )
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

DataPtr HoudiniScene::readTransform( double time ) const
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
	if ( !m_relativePath.empty() )
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
	return false;
}

void HoudiniScene::readAttributeNames( NameList &attrs ) const
{
}

ObjectPtr HoudiniScene::readAttribute( const Name &name, double time ) const
{
	return 0;
}

void HoudiniScene::writeAttribute( const Name &name, const Object *attribute, double time )
{
	throw Exception( "IECoreHoudini::HoudiniScene is read-only" );
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
			relativePath( geo->getUniqueStringValue( attrRef, 0 ), childPath );
			if ( childPath.empty() )
			{
				return true;
			}
		}
		
		return false;
	}
	
	/// \todo: need to account for OBJ_CAMERA and OBJ_LIGHT
	
	return false;
}

ObjectPtr HoudiniScene::readObject( double time ) const
{
	OBJ_Node *objNode = retrieveNode( true )->castToOBJNode();
	if ( !objNode )
	{
		return 0;
	}
	
	ObjectPtr result = 0;
	if ( objNode->getObjectType() == OBJ_GEOMETRY )
	{
		OP_Context context( time );
		GU_DetailHandle handle = objNode->getRenderGeometryHandle( context );
		FromHoudiniGeometryConverterPtr converter = FromHoudiniGeometryConverter::create( handle );
		if ( !converter )
		{
			return 0;
		}
		
		result = converter->convert();
		/// \todo: converters should ditch the name, namedIndices primVars
		/// \todo: add parameter to GroupConverter (or all of them?) to only convert named shapes
		///	   identify the appropriate shape name using pathToString( m_relativePaths )
		///	   use that parameter to avoid converting the entire group
		Group *group = IECore::runTimeCast<Group>( result );
		if ( group )
		{
			const Group::ChildContainer &children = group->children();
			for ( Group::ChildContainer::const_iterator it = children.begin(); it != children.end(); ++it )
			{
				const StringData *name = (*it)->blindData()->member<StringData>( "name", false );
				if ( name )
				{
					Path childPath;
					relativePath( name->readable().c_str(), childPath );
					if ( childPath.empty() )
					{
						return *it;
					}
				}
			}
		}
	}
	
	/// \todo: need to account for cameras and lights
	
	return result;
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
			if ( !child->nInputs() && child != contentNode )
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
			relativePath( geo->getUniqueStringValue( attrRef, i ), childPath );
			if ( !childPath.empty() && std::find( childNames.begin(), childNames.end(), *childPath.begin() ) == childNames.end() )
			{
				childNames.push_back( *childPath.begin() );
			}
		}
	}
}

bool HoudiniScene::hasChild( const Name &name ) const
{
	Path relativePath;
	return (bool)retrieveChild( name, relativePath, NullIfMissing );
}

SceneInterfacePtr HoudiniScene::child( const Name &name, MissingBehaviour missingBehaviour )
{
	Path relativePath;
	OP_Node *child = retrieveChild( name, relativePath, missingBehaviour );
	if ( !child )
	{
		return 0;
	}
	
	UT_String nodePath;
	child->getFullPath( nodePath );
	return new HoudiniScene( nodePath, relativePath );
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
	
	UT_String contentPath = ( m_contentPath.length() ) ? m_contentPath : m_nodePath;
	OP_Node *contentNode = OPgetDirector()->findNode( contentPath );
	if ( content )
	{
		if ( !contentNode && missingBehaviour == ThrowIfMissing )
		{
			throw Exception( "IECoreHoudini::HoudiniScene: Node \"" + contentPath.toStdString() + "\" no longer exists." );
		}
		
		node = contentNode;
	}
	
	if ( !m_relativePath.empty() )
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
				for ( int i=0; i < numShapes; ++i )
				{
					Path childPath;
					relativePath( geo->getUniqueStringValue( attrRef, i ), childPath );
					if ( childPath.empty() )
					{
						return node;
					}
				}
				
				if ( missingBehaviour == ThrowIfMissing )
				{
					throw Exception( "IECoreHoudini::HoudiniScene: Node \"" + contentPath.toStdString() + "\" does not contain the expected geometry." );
				}
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
			if ( child->getName().equal( "geo" ) )
			{
				return child;
			}
		}
	}
	
	return node;
}

OP_Node *HoudiniScene::retrieveChild( const Name &name, Path &relativePath, MissingBehaviour missingBehaviour ) const
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
			
			if ( child->getName().equal( name.c_str() ) )
			{
				// if it has inputs, then it's not a direct child
				if ( child->nInputs() )
				{
					continue;
				}
				
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
					this->relativePath( geo->getUniqueStringValue( attrRef, i ), relativePath );
					if ( !relativePath.empty() && name == *relativePath.begin() )
					{
						return contentNode;
					}
					else
					{
						relativePath.clear();
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
	SceneInterfacePtr scene = new HoudiniScene();
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

void HoudiniScene::relativePath( const char *value, Path &result ) const
{
	Path path;
	std::string pStr = value;
	stringToPath( pStr, path );
	
	Path myPath;
	this->path( myPath );
	
	size_t pathSize = path.size();
	size_t myPathSize = myPath.size();
	if ( pathSize < myPathSize )
	{
		std::string myPStr;
		pathToString( myPath, myPStr );
		throw Exception( "IECoreHoudini::HoudiniScene::relativePath: Path \"" + pStr + "\" is not a valid child of \"" + myPStr + "\"." );
	}
	
	for ( size_t i = 0; i < myPathSize; ++i )
	{
		if ( path[i] != myPath[i] )
		{
			std::string myPStr;
			pathToString( myPath, myPStr );
			throw Exception( "IECoreHoudini::HoudiniScene::relativePath: Path \"" + pStr + "\" is not a valid child of \"" + myPStr + "\"." );
		}
	}
	
	std::copy( path.begin() + myPathSize, path.end(), result.begin() );
}
