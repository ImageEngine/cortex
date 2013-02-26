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
	motDirector->getObjectManager()->getFullPath( m_path );
}

HoudiniScene::HoudiniScene( const std::string &fileName, IndexedIO::OpenMode )
{
	MOT_Director *motDirector = dynamic_cast<MOT_Director *>( OPgetDirector() );
	motDirector->getObjectManager()->getFullPath( m_path );
}

HoudiniScene::HoudiniScene( UT_String &path )
{
	m_path = path;
	m_path.hardenIfNeeded();
	
	// make sure the node exists
	retrieveNode();
}

HoudiniScene::~HoudiniScene()
{
}

SceneInterface::Name HoudiniScene::name() const
{
	if ( m_path.length() == 1 || m_path == "/obj" )
	{
		return SceneInterface::rootName;
	}
	
	UT_String path, name;
	m_path.splitPath( path, name );
	
	return name.toStdString();
}

void HoudiniScene::path( Path &p ) const
{
	p.clear();
	
	OP_Node *node = retrieveNode();
	if ( node->isManager() )
	{
		return;
	}
	
	UT_String tmp( m_path );
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
}

Imath::Box3d HoudiniScene::readBound( double time ) const
{
	OP_Node *node = retrieveNode();
	
	Imath::Box3d bounds;
	UT_BoundingBox box;
	OP_Context context( time );
	if ( node->getBoundingBox( box, context ) )
	{
		bounds = IECore::convert<Imath::Box3d>( box );
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

ObjectPtr HoudiniScene::readAttribute( const Name &name, double time )
{
	return 0;
}

void HoudiniScene::writeAttribute( const Name &name, const Object *attribute, double time )
{
	throw Exception( "IECoreHoudini::HoudiniScene is read-only" );
}

bool HoudiniScene::hasObject() const
{
	OP_Node *node = retrieveNode();
	if ( node->isManager() )
	{
		return false;
	}
	
	OBJ_Node *objNode = node->castToOBJNode();
	if ( objNode )
	{
		OBJ_OBJECT_TYPE type = objNode->getObjectType();
		/// \todo: need to account for cameras and lights
		return ( type == OBJ_GEOMETRY  ); //|| type == OBJ_CAMERA || type == OBJ_LIGHT );
	}
	
	return false;
}

ObjectPtr HoudiniScene::readObject( double time ) const
{
	OP_Node *node = retrieveNode();
	OBJ_Node *objNode = node->castToOBJNode();
	if ( !objNode )
	{
		return 0;
	}
	
	if ( objNode->getObjectType() == OBJ_GEOMETRY )
	{
		OP_Context context( time );
		GU_DetailHandle handle = objNode->getRenderGeometryHandle( context );
		FromHoudiniGeometryConverterPtr converter = FromHoudiniGeometryConverter::create( handle );
		if ( !converter )
		{
			return 0;
		}
		
		return converter->convert();
	}
	
	/// \todo: need to account for cameras and lights
	
	return 0;
}

void HoudiniScene::writeObject( const Object *object, double time )
{
	throw Exception( "IECoreHoudini::HoudiniScene is read-only" );
}

void HoudiniScene::childNames( NameList &childNames ) const
{
	OP_Node *node = retrieveNode();
	OBJ_Node *objNode = node->castToOBJNode();
	
	// add subnet children
	if ( node->isManager() || ( objNode && objNode->getObjectType() == OBJ_SUBNET ) )
	{
		for ( int i=0; i < node->getNchildren(); ++i )
		{
			OP_Node *child = node->getChild( i );
			// ignore children that have incoming connections, as those are actually grandchildren
			if ( !child->nInputs() )
			{
				childNames.push_back( Name( child->getName() ) );
			}
		}
	}
	
	if ( !objNode )
	{
		return;
	}
	
	// add connected outputs
	for ( unsigned i=0; i < objNode->nOutputs(); ++i )
	{
		childNames.push_back( Name( objNode->getOutput( i )->getName() ) );
	}
}

bool HoudiniScene::hasChild( const Name &name ) const
{
	return (bool)retrieveChild( name, SceneInterface::NullIfMissing );
}

SceneInterfacePtr HoudiniScene::child( const Name &name, MissingBehaviour missingBehaviour )
{
	OP_Node *child = retrieveChild( name, missingBehaviour );
	if ( !child )
	{
		return 0;
	}
	
	UT_String hPath;
	child->getFullPath( hPath );
	return new HoudiniScene( hPath );
}

ConstSceneInterfacePtr HoudiniScene::child( const Name &name, MissingBehaviour missingBehaviour ) const
{
	OP_Node *child = retrieveChild( name, missingBehaviour );
	if ( !child )
	{
		return 0;
	}
	
	UT_String hPath;
	child->getFullPath( hPath );
	return new HoudiniScene( hPath );
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

OP_Node *HoudiniScene::retrieveNode( MissingBehaviour missingBehaviour ) const
{
	OP_Node *node = OPgetDirector()->findNode( m_path );
	if ( !node && missingBehaviour == ThrowIfMissing )
	{
		throw Exception( "IECoreHoudini::HoudiniScene: Path \"" + m_path.toStdString() + "\" no longer exists." );
	}
	
	return node;
}

OP_Node *HoudiniScene::retrieveChild( const Name &name, MissingBehaviour missingBehaviour ) const
{
	OP_Node *node = retrieveNode( missingBehaviour );
	if ( !node )
	{
		return 0;
	}
	
	OBJ_Node *objNode = node->castToOBJNode();
	
	// check subnet children
	if ( node->isManager() || ( objNode && objNode->getObjectType() == OBJ_SUBNET ) )
	{
		for ( int i=0; i < node->getNchildren(); ++i )
		{
			OP_Node *child = node->getChild( i );
			if ( child->getName().toStdString() == name.string() )
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
	
	if ( objNode )
	{
		// check connected outputs
		for ( unsigned i=0; i < objNode->nOutputs(); ++i )
		{
			OP_Node *child = objNode->getOutput( i );
			if ( child->getName().toStdString() == name.string() )
			{
				return child;
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
