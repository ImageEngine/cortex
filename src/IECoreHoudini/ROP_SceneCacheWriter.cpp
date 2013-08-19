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

#include "boost/filesystem/path.hpp"

#include "GEO/GEO_AttributeHandle.h"
#include "GU/GU_Detail.h"
#include "OBJ/OBJ_Node.h" 
#include "OP/OP_Director.h" 
#include "PRM/PRM_Include.h"
#include "PRM/PRM_SpareData.h"
#include "ROP/ROP_Error.h"

#include "IECore/LinkedScene.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/HoudiniScene.h"
#include "IECoreHoudini/ROP_SceneCacheWriter.h"

using namespace IECore;
using namespace IECoreHoudini;

const char *ROP_SceneCacheWriter::typeName = "ieSceneCacheWriter";

ROP_SceneCacheWriter::ROP_SceneCacheWriter( OP_Network *net, const char *name, OP_Operator *op )
	: ROP_Node( net, name, op ), m_liveScene( 0 ), m_outScene( 0 )
{
}

ROP_SceneCacheWriter::~ROP_SceneCacheWriter()
{
}

OP_Node *ROP_SceneCacheWriter::create( OP_Network *net, const char *name, OP_Operator *op )
{
	return new ROP_SceneCacheWriter( net, name, op );
}

PRM_Name ROP_SceneCacheWriter::pFile( "file", "File" );
PRM_Name ROP_SceneCacheWriter::pRootObject( "rootObject", "Root Object" );

PRM_Default ROP_SceneCacheWriter::fileDefault( 0, "$HIP/output.scc" );
PRM_Default ROP_SceneCacheWriter::rootObjectDefault( 0, "/obj" );

OP_TemplatePair *ROP_SceneCacheWriter::buildParameters()
{
	static PRM_Template *thisTemplate = 0;
	if ( !thisTemplate )
	{
		thisTemplate = new PRM_Template[3];
		
		thisTemplate[0] = PRM_Template(
			PRM_FILE, 1, &pFile, &fileDefault, 0, 0, 0, 0, 0,
			"An SCC file to write, based on the Houdini hierarchy defined by the Root Object provided."
		);
		
		thisTemplate[1] = PRM_Template(
			PRM_STRING, PRM_TYPE_DYNAMIC_PATH, 1, &pRootObject, &rootObjectDefault, 0, 0, 0,
			&PRM_SpareData::objPath, 0, "The node to use as the root of the SceneCache"
		);
	}
	
	static OP_TemplatePair *templatePair = 0;
	if ( !templatePair )
	{
		OP_TemplatePair *extraTemplatePair = new OP_TemplatePair( thisTemplate );
		templatePair = new OP_TemplatePair( ROP_Node::getROPbaseTemplate(), extraTemplatePair );
	}
	
	return templatePair;
}

int ROP_SceneCacheWriter::startRender( int nframes, fpreal s, fpreal e )
{
	UT_String nodePath;
	evalString( nodePath, pRootObject.getToken(), 0, 0 );
	
	UT_String value;
	evalString( value, pFile.getToken(), 0, 0 );
	std::string file = value.toStdString();
	
	try
	{
		SceneInterface::Path emptyPath;
		m_liveScene = new IECoreHoudini::HoudiniScene( nodePath, emptyPath, emptyPath );
		
		// wrapping with a LinkedScene to ensure full expansion when writing the non-linked file
		if ( boost::filesystem::path( file ).extension().string() != ".lscc" )
		{
			m_liveScene = new LinkedScene( m_liveScene );
		}
	}
	catch ( IECore::Exception &e )
	{		
		addError( ROP_MESSAGE, e.what() );
		return false;
	}
	
	try
	{
		m_outScene = SceneInterface::create( file, IndexedIO::Write );
	}
	catch ( IECore::Exception &e )
	{
		addError( ROP_MESSAGE, ( "Could not create a writable IECore::SceneInterface at \"" + file + "\"" ).c_str() );
		return false;
	}
	
	return true;
}

ROP_RENDER_CODE ROP_SceneCacheWriter::renderFrame( fpreal time, UT_Interrupt *boss )
{
	SceneInterfacePtr outScene = m_outScene;
	
	// we need to re-root the scene if its trying to cache a top level object
	UT_String nodePath;
	evalString( nodePath, pRootObject.getToken(), 0, 0 );
	OBJ_Node *node = OPgetDirector()->findNode( nodePath )->castToOBJNode();
	if ( node && node->getObjectType() == OBJ_GEOMETRY )
	{
		OP_Context context( CHgetEvalTime() );
		const GU_Detail *geo = node->getRenderGeometry( context );
		const GEO_AttributeHandle attrHandle = geo->getPrimAttribute( "name" );
		bool reRoot = !attrHandle.isAttributeValid();
		if ( attrHandle.isAttributeValid() )
		{
			const GA_ROAttributeRef attrRef( attrHandle.getAttribute() );
			int numShapes = geo->getUniqueValueCount( attrRef );
			reRoot = ( numShapes == 0 );
			if ( numShapes == 1 )
			{
				const char *name = geo->getUniqueStringValue( attrRef, 0 );
				if ( !strcmp( name, "" ) || !strcmp( name, "/" ) )
				{
					reRoot = true;
				}
			}
		}

		if ( reRoot )
		{
			outScene = m_outScene->createChild( node->getName().toStdString() );
		}
	}
	
	return doWrite( m_liveScene, outScene, time );
}

ROP_RENDER_CODE ROP_SceneCacheWriter::endRender()
{
	m_liveScene = 0;
	m_outScene = 0;
	
	return ROP_CONTINUE_RENDER;
}

ROP_RENDER_CODE ROP_SceneCacheWriter::doWrite( const SceneInterface *liveScene, SceneInterface *outScene, double time )
{
	if ( liveScene != m_liveScene )
	{
		outScene->writeTransform( liveScene->readTransform( time ), time );
	}
	
	bool link = false;
	SceneInterface::NameList attrs;
	liveScene->attributeNames( attrs );
	for ( SceneInterface::NameList::iterator it = attrs.begin(); it != attrs.end(); ++it )
	{
		outScene->writeAttribute( *it, liveScene->readAttribute( *it, time ), time );
		if ( *it == LinkedScene::linkAttribute )
		{
			link = true;
		}
	}
	
	// If this is a link, we exit now, since all other write calls will throw exceptions
	if ( link )
	{
		return ROP_CONTINUE_RENDER;
	}
	
	SceneInterface::NameList tags;
	liveScene->readTags( tags, false );
	outScene->writeTags( tags );
	
	if ( liveScene->hasObject() )
	{
		try
		{
			outScene->writeObject( liveScene->readObject( time ), time );
		}
		catch ( IECore::Exception &e )
		{
			addError( ROP_MESSAGE, e.what() );
			return ROP_ABORT_RENDER;
		}
	}
	
	SceneInterface::NameList children;
	liveScene->childNames( children );
	for ( SceneInterface::NameList::iterator it = children.begin(); it != children.end(); ++it )
	{
		ConstSceneInterfacePtr liveChild = liveScene->child( *it );
		SceneInterfacePtr outChild = outScene->child( *it, SceneInterface::CreateIfMissing );
		ROP_RENDER_CODE status = doWrite( liveChild, outChild, time );
		if ( status != ROP_CONTINUE_RENDER )
		{
			return status;
		}
	}
	
	return ROP_CONTINUE_RENDER;
}
