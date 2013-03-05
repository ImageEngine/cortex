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


#include "IECoreMaya/SceneShape.h"
#include "IECoreMaya/MayaTypeIds.h"

#include "maya/MFnTypedAttribute.h"


using namespace IECore;
using namespace IECoreMaya;

MTypeId SceneShape::id = SceneShapeId;
MObject SceneShape::aSceneFilePlug;
MObject SceneShape::aSceneRootPlug;


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
	aSceneFilePlug = tAttr.create( "sceneFile", "scf", MFnData::kString, &s );
	assert( s );
	s = addAttribute( aSceneFilePlug );
	assert( s );
	
	aSceneRootPlug = tAttr.create( "sceneRoot", "scr", MFnData::kString, &s );
	assert( s );
	s = addAttribute( aSceneRootPlug );
	assert( s );

	attributeAffects( aSceneFilePlug, aChildrenNames );
	attributeAffects( aSceneRootPlug, aChildrenNames );

	return s;
}

IECore::SceneInterfacePtr SceneShape::getSceneInterface()
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
		m_scene = IECore::SceneInterface::create( sceneFile.asChar(), IECore::IndexedIO::Read );
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

IECore::SceneInterface::Path SceneShape::getSceneRoot()
{
	MPlug pSceneRoot( thisMObject(), aSceneRootPlug );
	MString sceneRoot;
	pSceneRoot.getValue( sceneRoot );

	IECore::SceneInterface::Path rootPath;
	IECore::SceneInterface::stringToPath( sceneRoot.asChar(), rootPath );
	return rootPath;
}

MStatus SceneShape::setDependentsDirty( const MPlug &plug, MPlugArray &plugArray )
{
	if( plug == aSceneFilePlug || plug == aSceneRootPlug )
	{
		m_sceneDirty = true;
		setDirty();
	}

	return SceneShapeInterface::setDependentsDirty( plug, plugArray );
}


	
	
	
	
