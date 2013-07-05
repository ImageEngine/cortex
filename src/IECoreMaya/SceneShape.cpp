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

#include "IECore/SharedSceneInterfaces.h"
#include "IECore/LinkedScene.h"

#include "IECoreMaya/SceneShape.h"
#include "IECoreMaya/MayaScene.h"
#include "IECoreMaya/MayaTypeIds.h"

#include "maya/MFnTypedAttribute.h"
#include "maya/MFnStringData.h"
#include "maya/MPlugArray.h"
#include "maya/MFnDagNode.h"
#include "maya/MTime.h"

using namespace IECore;
using namespace IECoreMaya;

MTypeId SceneShape::id = SceneShapeId;
MObject SceneShape::aSceneFilePlug;
MObject SceneShape::aSceneRootPlug;

// registers this class in MayaScene
SceneShape::MayaSceneAddOn SceneShape::g_mayaSceneAddon;

SceneShape::MayaSceneAddOn::MayaSceneAddOn()
{
	MayaScene::registerCustomObject( SceneShape::hasSceneShapeObject, SceneShape::readSceneShapeObject );
	MayaScene::registerCustomAttribute( LinkedScene::linkAttribute, SceneShape::hasSceneShapeLink, SceneShape::readSceneShapeLink );
	MayaScene::registerCustomTags( SceneShape::hasTag, SceneShape::readTags );
}

SceneShape::SceneShape()
	: m_sceneDirty( true )
{
}

SceneShape::~SceneShape()
{
}

void *SceneShape::creator()
{
	return new SceneShape;
}

void SceneShape::postConstructor()
{
	SceneShapeInterface::postConstructor();
	setRenderable( true );
}

MStatus SceneShape::initialize()
{
	MStatus s = inheritAttributesFrom( "ieSceneShapeInterface" );
	MFnTypedAttribute tAttr;
	
	// will need to check for sceneFile extensions
	aSceneFilePlug = tAttr.create( "file", "scf", MFnData::kString, &s );
	assert( s );
	s = addAttribute( aSceneFilePlug );
	assert( s );
	
	aSceneRootPlug = tAttr.create( "root", "scr", MFnData::kString, MFnStringData().create( "/" ), &s );
	assert( s );
	s = addAttribute( aSceneRootPlug );
	assert( s );
	
	attributeAffects( aSceneFilePlug, aTransform );
	attributeAffects( aSceneFilePlug, aBound );
	attributeAffects( aSceneFilePlug, aOutputObjects );
	attributeAffects( aSceneFilePlug, aAttributes );
	
	attributeAffects( aSceneRootPlug, aTransform );
	attributeAffects( aSceneRootPlug, aBound );
	attributeAffects( aSceneRootPlug, aOutputObjects );
	attributeAffects( aSceneRootPlug, aAttributes );

	return s;
}

IECore::ConstSceneInterfacePtr SceneShape::getSceneInterface()
{
	if( !m_sceneDirty )
	{
		return m_scene;
	}
	
	MPlug pSceneFile( thisMObject(), aSceneFilePlug );
	MString sceneFile;
	pSceneFile.getValue( sceneFile );
	
	MPlug pSceneRoot( thisMObject(), aSceneRootPlug );
	MString sceneRoot;
	pSceneRoot.getValue( sceneRoot );
	
	try
	{
		m_scene = IECore::SharedSceneInterfaces::get( sceneFile.asChar() );
		IECore::SceneInterface::Path rootPath;
		IECore::SceneInterface::stringToPath( sceneRoot.asChar(), rootPath );
		m_scene = m_scene->scene( rootPath );
	
		m_sceneDirty = false;
	}
	catch( std::exception &e )
	{
		m_scene = 0;
	}
	
	return m_scene;
}

MStatus SceneShape::setDependentsDirty( const MPlug &plug, MPlugArray &plugArray )
{
	if( plug == aSceneFilePlug || plug == aSceneRootPlug )
	{
		m_sceneDirty = true;
		setDirty();
		childChanged( kBoundingBoxChanged );
	}

	return SceneShapeInterface::setDependentsDirty( plug, plugArray );
}

SceneShape *SceneShape::findScene( const MDagPath &p, MDagPath *dagPath )
{
	unsigned int childCount = 0;
	p.numberOfShapesDirectlyBelow(childCount);

	for ( unsigned int c = 0; c < childCount; c++ )
	{
		MDagPath childDag = p;
		if( childDag.extendToShapeDirectlyBelow( c ) )
		{
			MFnDagNode fnChildDag(childDag);
			if ( fnChildDag.isIntermediateObject() )
			{
				continue;
			}

			MPxNode* userNode = fnChildDag.userNode();

			if( userNode && userNode->typeId() == SceneShapeId )
			{
				SceneShape *sceneShape = dynamic_cast< SceneShape * >( userNode );
				if ( !sceneShape )
				{
					throw Exception( "Could not get a pointer to SceneShape!");
				}
				if ( dagPath )
				{
					*dagPath = childDag;
				}
				return sceneShape;
			}
		}
	}
	return 0;			
}

bool SceneShape::hasSceneShapeLink( const MDagPath &p )
{
	MDagPath dagPath;
	SceneShape *sceneShape = findScene( p, &dagPath );
	if ( !sceneShape )
	{
		return false;
	}
	
	MFnDagNode fnChildDag( dagPath );
	MStatus st;
	MPlug objectOnlyPlug = fnChildDag.findPlug( aObjectOnly, &st );
	if( !st )
	{
		throw Exception( "Could not find 'objectOnly' plug in SceneShape!");
	}
			
	// if we're doing objects only, we just output the object directy, so we don't need link attributes... 
	if( objectOnlyPlug.asBool() )
	{
		return false;
	}

	if ( !sceneShape->getSceneInterface() )
	{
		return false;
	}
	
	// so if it's not object only, then we know the scene loads everything and we can create a link to it.
	return true;
}

ObjectPtr SceneShape::readSceneShapeLink( const MDagPath &p )
{
	MDagPath dagPath;
	SceneShape *sceneShape = findScene( p, &dagPath );
	if ( !sceneShape )
	{
		throw Exception("readSceneShapeLink: Could not find SceneShape!");
	}

	ConstSceneInterfacePtr scene = sceneShape->getSceneInterface();
	if ( !scene )
	{
		throw Exception( "Empty scene!");
	}

	MFnDagNode fnChildDag( dagPath );
	MStatus st;
	MPlug timePlug = fnChildDag.findPlug( aTime, &st );
	if( !st )
	{
		throw Exception( "Could not find 'time' plug in SceneShape!");
	}

	// if time plug is connected to maya global time, then we assume there's no time remapping between the Maya scene and the loaded scene.
	MPlugArray array;
	timePlug.connectedTo( array, true, false, &st );
	if( !st )
	{
		throw Exception( "Could not find 'time' plug connections in SceneShape!");
	}

	for ( unsigned int i = 0; i < array.length(); i++ )
	{
		if ( array[i].name() == "time1.outTime" )
		{
			/// connected to time, so no time remapping between maya scene and loaded scene.
			return LinkedScene::linkAttributeData( scene );
		}
	}
	/// couldn't find connection to maya time, so this node is mapping the time some other way.
	MTime time;
	timePlug.getValue( time );
	return LinkedScene::linkAttributeData( scene, time.as( MTime::kSeconds ) );
}

bool SceneShape::hasSceneShapeObject( const MDagPath &p )
{
	MDagPath dagPath;
	SceneShape *sceneShape = findScene( p, &dagPath );
	if ( !sceneShape )
	{
		return false;
	}

	MFnDagNode fnChildDag( dagPath );
	MStatus st;
	MPlug objectOnlyPlug = fnChildDag.findPlug( aObjectOnly, &st );
	if( !st )
	{
		throw Exception( "Could not find 'objectOnly' plug in SceneShape!");
	}
			
	// if we're rendering object and children than it should only be represented as a link to the scene and no objects. 
	if( !objectOnlyPlug.asBool() )
	{
		return false;
	}

	return sceneShape->getSceneInterface()->hasObject();
}

ObjectPtr SceneShape::readSceneShapeObject( const MDagPath &p )
{
	SceneShape *sceneShape = findScene( p );
	if ( !sceneShape )
	{
		return 0;
	}

	MPlug pTime( sceneShape->thisMObject(), aTime );
	MTime time;
	pTime.getValue( time );
	double t = time.as( MTime::kSeconds );
	return sceneShape->getSceneInterface()->readObject( t );
}

bool SceneShape::hasTag( const MDagPath &p, const SceneInterface::Name &tag )
{
	SceneShape *sceneShape = findScene( p );
	if ( !sceneShape )
	{
		return false;
	}
	
	const SceneInterface *scene = sceneShape->getSceneInterface();
	if ( !scene )
	{
		return false;
	}
	
	return scene->hasTag( tag );
}

void SceneShape::readTags( const MDagPath &p, SceneInterface::NameList &tags, bool includeChildren )
{
	SceneShape *sceneShape = findScene( p );
	if ( !sceneShape )
	{
		return;
	}
	
	const SceneInterface *scene = sceneShape->getSceneInterface();
	if ( !scene )
	{
		return;
	}
	
	scene->readTags( tags, includeChildren );
}
