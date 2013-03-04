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

#include "PRM/PRM_Include.h"
#include "PRM/PRM_SpareData.h"
#include "ROP/ROP_Error.h"

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
		PRM_Template *parentTemplate = ROP_Node::getROPbaseTemplate();
		unsigned numParentParms = PRM_Template::countTemplates( parentTemplate );
		thisTemplate = new PRM_Template[ numParentParms + 3 ];
		
		// add the common ROP parms
		for ( unsigned i = 0; i < numParentParms; ++i )
		{
			thisTemplate[i] = parentTemplate[i];
		}
		
		thisTemplate[numParentParms] = PRM_Template(
			PRM_FILE, 1, &pFile, &fileDefault, 0, 0, 0, 0, 0,
			"An SCC file to write, based on the Houdini hierarchy defined by the Root Object provided."
		);
		thisTemplate[numParentParms+1] = PRM_Template(
			PRM_STRING, PRM_TYPE_DYNAMIC_PATH, 1, &pRootObject, &rootObjectDefault, 0, 0, 0,
			&PRM_SpareData::objPath, 0, "The node to use as the root of the SceneCache"
		);
	}
	
	static OP_TemplatePair *templatePair = 0;
	if ( !templatePair )
	{
		templatePair = new OP_TemplatePair( thisTemplate );
	}
	
	return templatePair;
}

int ROP_SceneCacheWriter::startRender( int nframes, fpreal s, fpreal e )
{
	UT_String value;
	evalString( value, pRootObject.getToken(), 0, 0 );
	
	try
	{
		SceneInterface::Path path;
		m_liveScene = new IECoreHoudini::HoudiniScene( value, path );
	}
	catch ( IECore::Exception &e )
	{		
		addError( ROP_MESSAGE, e.what() );
		return false;
	}
	
	evalString( value, pFile.getToken(), 0, 0 );
	std::string file = value.toStdString();
	
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
	return doWrite( m_liveScene, m_outScene, time );
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
	
	if ( liveScene->hasObject() )
	{
		try
		{
			/// \todo: does an invisible node mean there is no object?
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
		/// \todo: does an invisible node mean its not a child?
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
