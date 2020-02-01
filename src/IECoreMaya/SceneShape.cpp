//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2014, Image Engine Design Inc. All rights reserved.
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

#include "IECoreScene/SharedSceneInterfaces.h"
#include "IECoreScene/LinkedScene.h"

#include "IECoreMaya/SceneShape.h"
#include "IECoreMaya/LiveScene.h"
#include "IECoreMaya/MayaTypeIds.h"

#include "maya/MFnTypedAttribute.h"
#include "maya/MFnStringData.h"
#include "maya/MPlugArray.h"
#include "maya/MFnDagNode.h"
#include "maya/MTime.h"
#include "maya/MEvaluationNode.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreMaya;

MTypeId SceneShape::id = SceneShapeId;
MObject SceneShape::aSceneFilePlug;
MObject SceneShape::aSceneRootPlug;

// registers this class in LiveScene
SceneShape::LiveSceneAddOn SceneShape::g_liveSceneAddon;

SceneShape::LiveSceneAddOn::LiveSceneAddOn()
{
	LiveScene::registerCustomObject( SceneShape::hasSceneShapeObject, SceneShape::readSceneShapeObject );
	LiveScene::registerCustomAttributes( SceneShape::sceneShapeAttributeNames, SceneShape::readSceneShapeAttribute );
	LiveScene::registerCustomTags( SceneShape::hasTag, SceneShape::readTags );
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
	aSceneFilePlug = tAttr.create( "file", "scf", MFnData::kString );
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

IECoreScene::ConstSceneInterfacePtr SceneShape::getSceneInterface()
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
		m_scene = IECoreScene::SharedSceneInterfaces::get( sceneFile.asChar() );
		IECoreScene::SceneInterface::Path rootPath;
		IECoreScene::SceneInterface::stringToPath( sceneRoot.asChar(), rootPath );
		m_scene = m_scene->scene( rootPath );

		m_sceneDirty = false;
	}
	catch( std::exception &e )
	{
		m_scene = nullptr;
	}

	return m_scene;
}

MStatus SceneShape::setDependentsDirty( const MPlug &plug, MPlugArray &plugArray )
{
	if( plug == aSceneFilePlug || plug == aSceneRootPlug || plug == aObjectDependency )
	{
		m_sceneDirty = true;
		setDirty();
		childChanged( kBoundingBoxChanged );
	}

	return SceneShapeInterface::setDependentsDirty( plug, plugArray );
}

MStatus SceneShape::preEvaluation( const  MDGContext& context, const MEvaluationNode& evaluationNode )
{
	// Dirty implementation for Evaluation Graph (Parallel / Serial Mode)
	MStatus status;

	// Do nothing if context is not normal
	if( !context.isNormal() )
	{
		return MStatus::kFailure;
	}

	if( ( evaluationNode.dirtyPlugExists( aSceneFilePlug, &status ) && status ) || ( evaluationNode.dirtyPlugExists( aSceneRootPlug, &status ) && status ) || ( evaluationNode.dirtyPlugExists( aObjectDependency, &status ) && status )  )
	{
		m_sceneDirty = true;
		setDirty();
		childChanged( kBoundingBoxChanged );
	}

	return SceneShapeInterface::preEvaluation( context, evaluationNode );
}

SceneShape *SceneShape::findScene( const MDagPath &p, bool noIntermediate, MDagPath *dagPath )
{
	// Parse all children because numberOfShapesDirectlyBelow does not include intermediate shapes
	unsigned int childCount = p.childCount();
	for ( unsigned int c = 0; c < childCount; c++ )
	{
		MStatus st;
		MObject childObject = p.child( c, &st );
		if( st )
		{
			MFnDagNode fnChildDag(childObject);
			MPxNode* userNode = fnChildDag.userNode();

			if( userNode && userNode->typeId() == SceneShapeId )
			{
				if ( noIntermediate && fnChildDag.isIntermediateObject() )
				{
					continue;
				}

				SceneShape *sceneShape = dynamic_cast< SceneShape * >( userNode );
				if ( !sceneShape )
				{
					throw Exception( "Could not get a pointer to SceneShape!");
				}
				if ( dagPath )
				{
					MDagPath childDag;
					fnChildDag.getPath( childDag );
					*dagPath = childDag;
				}
				return sceneShape;
			}
		}
	}
	return nullptr;
}

bool SceneShape::hasSceneShapeLink( const MDagPath &p )
{
	// We exclude intermediate objects because this indicates that a native maya dag node is
	// replacing the object (and hierarchy) at this location, and we can no longer link back
	// to the original cache.
	MDagPath dagPath;
	SceneShape *sceneShape = findScene( p, true, &dagPath );
	if ( !sceneShape )
	{
		return false;
	}

	MFnDagNode fnChildDag( dagPath );
	MStatus st;
	MPlug objectOnlyPlug = fnChildDag.findPlug( aObjectOnly, false, &st );
	if( !st )
	{
		throw Exception( "Could not find 'objectOnly' plug in SceneShape!");
	}

	// if we're doing objects only, we just output the object directly, so we don't need link attributes...
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

ConstObjectPtr SceneShape::readSceneShapeLink( const MDagPath &p )
{
	MDagPath dagPath;
	SceneShape *sceneShape = findScene( p, true, &dagPath );
	if ( !sceneShape )
	{
		throw Exception("readSceneShapeLink: Could not find SceneShape!");
	}

	const SceneInterface *scene = sceneShape->getSceneInterface().get();
	if ( !scene )
	{
		throw Exception( "Empty scene!");
	}

	MFnDagNode fnChildDag( dagPath );
	MStatus st;
	MPlug timePlug = fnChildDag.findPlug( aTime, false, &st );
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

void SceneShape::sceneShapeAttributeNames( const MDagPath &p, SceneInterface::NameList &attributeNames )
{
	// Note that we include intermediate SceneShapes objects since we may have substituted native
	// maya dag nodes for our SceneShape nodes, but we don't want to lose visibility of our attributes
	MDagPath dagPath;
	SceneShape *sceneShape = findScene( p, false, &dagPath );
	if ( !sceneShape )
	{
		return;
	}

	const SceneInterface *scene = sceneShape->getSceneInterface().get();
	if ( !scene )
	{
		return;
	}

	SceneInterface::NameList sceneAttrNames;
	scene->attributeNames( sceneAttrNames );
	attributeNames.insert( attributeNames.end(), sceneAttrNames.begin(), sceneAttrNames.end() );

	MFnDagNode fnChildDag( dagPath );
	if( !fnChildDag.isIntermediateObject() && hasSceneShapeLink( p ) )
	{
		attributeNames.push_back( LinkedScene::linkAttribute );
	}

}

ConstObjectPtr SceneShape::readSceneShapeAttribute( const MDagPath &p, SceneInterface::Name attributeName )
{
	MDagPath dagPath;
	SceneShape *sceneShape = findScene( p, false, &dagPath );
	if ( !sceneShape )
	{
		return nullptr;
	}

	MFnDagNode fnChildDag( dagPath );
	if( attributeName == LinkedScene::linkAttribute )
	{
		if( !fnChildDag.isIntermediateObject() )
		{
			return readSceneShapeLink(p);
		}
	}

	const SceneInterface *scene = sceneShape->getSceneInterface().get();
	if ( !scene )
	{
		return nullptr;
	}

	MPlug timePlug = fnChildDag.findPlug( aTime, false );
	MTime time;
	timePlug.getValue( time );
	try
	{
		return scene->readAttribute( attributeName, time.as( MTime::kSeconds ) );
	}
	catch( ... )
	{
		return nullptr;
	}
}

bool SceneShape::hasSceneShapeObject( const MDagPath &p )
{
	MDagPath dagPath;
	SceneShape *sceneShape = findScene( p, true, &dagPath );
	if ( !sceneShape )
	{
		return false;
	}

	const SceneInterface *scene = sceneShape->getSceneInterface().get();
	if( !scene )
	{
		return false;
	}

	MStatus st;
	MFnDagNode fnChildDag( dagPath );
	MPlug objectOnlyPlug = fnChildDag.findPlug( aObjectOnly, false, &st );
	if( !st )
	{
		throw Exception( "Could not find 'objectOnly' plug in SceneShape!");
	}

	// When objectOnly == true, we assume that this SceneShape will contain the object (if one exists)
	// When objectOnly == false, we expose this SceneShapes path to LiveScene as a link.
	// Paths with links do not hold their own objects, they are contained in the associated LinkedScene.
	// Therefore, if objectOnly == false, we assume we are a link and that we do not have an object.
	// If we want to be able to read the links transparently with LiveScene, then we need to decorate LiveScene with LinkedScene.
	// ie) LinkedScenePtr linkedScene = new LinkedScene{ new LiveScene };
	if( !objectOnlyPlug.asBool() )
	{
		return false;
	}

	return scene->hasObject();
}

ConstObjectPtr SceneShape::readSceneShapeObject( const MDagPath &p )
{
	SceneShape *sceneShape = findScene( p, true );
	if ( !sceneShape )
	{
		return nullptr;
	}

	const SceneInterface *scene = sceneShape->getSceneInterface().get();
	if( !scene )
	{
		return nullptr;
	}

	// Live scene will call `hasSceneShapeObject` before `readSceneShapeObject`
	// Therefore it's unnecessary to recheck for the objectOnly plug (objectOnly = True)

	MPlug pTime( sceneShape->thisMObject(), aTime );
	MTime time;
	pTime.getValue( time );
	double t = time.as( MTime::kSeconds );

	return scene->readObject( t );
}

bool SceneShape::hasTag( const MDagPath &p, const SceneInterface::Name &tag, int filter )
{
	SceneShape *sceneShape = findScene( p, false );
	if ( !sceneShape )
	{
		return false;
	}

	const SceneInterface *scene = sceneShape->getSceneInterface().get();
	if ( !scene )
	{
		return false;
	}

	return scene->hasTag( tag, filter );
}

void SceneShape::readTags( const MDagPath &p, SceneInterface::NameList &tags, int filter )
{
	SceneShape *sceneShape = findScene( p, false );
	if ( !sceneShape )
	{
		return;
	}

	const SceneInterface *scene = sceneShape->getSceneInterface().get();
	if ( !scene )
	{
		return;
	}

	scene->readTags( tags, filter );
}
