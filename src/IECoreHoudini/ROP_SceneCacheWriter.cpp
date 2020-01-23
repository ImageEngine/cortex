//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2015, Image Engine Design Inc. All rights reserved.
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

#include "IECoreHoudini/ROP_SceneCacheWriter.h"

#include "IECoreHoudini/Convert.h"

#include "IECoreScene/LinkedScene.h"

#include "GA/GA_Names.h"
#include "GU/GU_Detail.h"
#include "OBJ/OBJ_Node.h"
#include "OP/OP_Bundle.h"
#include "OP/OP_Director.h"
#include "PRM/PRM_Include.h"
#include "PRM/PRM_Parm.h"
#include "PRM/PRM_SpareData.h"
#include "ROP/ROP_Error.h"
#include "SOP/SOP_Node.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_StringMMPattern.h"
#include "UT/UT_ValArray.h"

#include "boost/filesystem/path.hpp"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreHoudini;

const char *ROP_SceneCacheWriter::typeName = "ieSceneCacheWriter";

ROP_SceneCacheWriter::ROP_SceneCacheWriter( OP_Network *net, const char *name, OP_Operator *op )
	: ROP_Node( net, name, op ), m_liveHoudiniScene( 0 ), m_liveScene( 0 ), m_outScene( 0 ), m_forceFilter( 0 ), m_startTime( 0 ), m_endTime( 0 )
{
}

ROP_SceneCacheWriter::~ROP_SceneCacheWriter()
{
	delete m_forceFilter;
}

OP_Node *ROP_SceneCacheWriter::create( OP_Network *net, const char *name, OP_Operator *op )
{
	return new ROP_SceneCacheWriter( net, name, op );
}

PRM_Name ROP_SceneCacheWriter::pFile( "file", "File" );
PRM_Name ROP_SceneCacheWriter::pRootObject( "rootObject", "Root Object" );
PRM_Name ROP_SceneCacheWriter::pForceObjects( "forceObjects", "Force Objects" );

PRM_Default ROP_SceneCacheWriter::fileDefault( 0, "$HIP/output.scc" );
PRM_Default ROP_SceneCacheWriter::rootObjectDefault( 0, "/obj" );
PRM_SpareData ROP_SceneCacheWriter::forceObjectsSpareData;

const SceneInterface::Name &ROP_SceneCacheWriter::changingHierarchyAttribute( "sceneInterface:changingHierarchy" );

OP_TemplatePair *ROP_SceneCacheWriter::buildParameters()
{
	static PRM_Template *thisTemplate = 0;
	if ( !thisTemplate )
	{
		PRM_Template *baseTemplate = ROP_Node::getROPbaseTemplate();
		PRM_Template *scriptTemplate = ROP_Node::getROPscriptTemplate();

		unsigned numBaseParms = PRM_Template::countTemplates( baseTemplate );
		unsigned numScriptParms = PRM_Template::countTemplates( scriptTemplate );

		thisTemplate = new PRM_Template[ numBaseParms + numScriptParms + 4 ];

		// add the ROP base parms
		unsigned totalParms = 0;
		for ( unsigned i = 0; i < numBaseParms; ++i, ++totalParms )
		{
			thisTemplate[totalParms] = baseTemplate[i];
		}

		// add the SceneCache parms
		thisTemplate[totalParms] = PRM_Template(
			PRM_FILE, 1, &pFile, &fileDefault, 0, 0, 0, 0, 0,
			"An SCC file to write, based on the Houdini hierarchy defined by the Root Object provided."
		);
		totalParms++;

		thisTemplate[totalParms] = PRM_Template(
			PRM_STRING, PRM_TYPE_DYNAMIC_PATH, 1, &pRootObject, &rootObjectDefault, 0, 0, 0,
			&PRM_SpareData::objPath, 0, "The node to use as the root of the SceneCache"
		);
		totalParms++;

		forceObjectsSpareData.copyFrom( PRM_SpareData::objPath );
		forceObjectsSpareData.setOpRelative( "/obj" );
		thisTemplate[totalParms] = PRM_Template(
			PRM_STRING, PRM_TYPE_DYNAMIC_PATH_LIST, 1, &pForceObjects, 0, 0, 0, 0,
			&forceObjectsSpareData, 0, "Optional list of nodes to force as expanded objects. "
			"If this list is used, then links will be stored for any node not listed."
		);
		totalParms++;

		// add the ROP script parms
		for ( unsigned i = 0; i < numScriptParms; ++i, ++totalParms )
		{
			thisTemplate[totalParms] = scriptTemplate[i];
		}
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
	UT_String nodePath;
	evalString( nodePath, pRootObject.getToken(), 0, 0 );
	OP_Node* node = this->findNode( nodePath );
	UT_String actualNodePath( node->getFullPath() );

	UT_String value;
	evalString( value, pFile.getToken(), 0, 0 );
	std::string file = value.toStdString();

	try
	{
		SceneInterface::Path emptyPath;
		m_liveHoudiniScene = new IECoreHoudini::LiveScene( actualNodePath, emptyPath, emptyPath, s + CHgetManager()->getSecsPerSample() );
		// wrapping with a LinkedScene to ensure full expansion when writing the non-linked file
		if ( linked( file ) )
		{
			m_liveScene = m_liveHoudiniScene;
		}
		else
		{
			m_liveScene = new LinkedScene( m_liveHoudiniScene );
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
		addError( ROP_MESSAGE, ( "Could not create a writable IECoreScene::SceneInterface at \"" + file + "\"" ).c_str() );
		return false;
	}

	UT_String forceObjects;
	evalString( forceObjects, pForceObjects.getToken(), 0, 0 );

	m_forceFilter =  0;
	if ( linked( file ) && !forceObjects.equal( "" ) )
	{
		// get the list of nodes matching the filter
		const PRM_SpareData *data = getParm( pForceObjects.getToken() ).getSparePtr();
		OBJ_Node *baseNode = OPgetDirector()->findNode( data->getOpRelative() )->castToOBJNode();
		OP_Bundle *bundle = getParmBundle( pForceObjects.getToken(), 0, forceObjects, baseNode, data->getOpFilter() );

		// add all of the parent nodes
		UT_ValArray<OP_Node *> nodes;
		bundle->getMembers( nodes );
		size_t numNodes = nodes.entries();
		for ( size_t i = 0; i < numNodes; ++i )
		{
			OP_Node *current = nodes( i )->getParent();
			while ( current )
			{
				bundle->addOp( current );
				current = current->getParent();
			}
		}

		// build a matchable filter from all these nodes
		UT_WorkBuffer buffer;
		bundle->buildString( buffer );
		buffer.copyIntoString( forceObjects );
		m_forceFilter = new UT_StringMMPattern();
		m_forceFilter->compile( forceObjects );
	}

	// We need to adjust the time for writing, because Houdini treats time starting
	// at Frame 1, while SceneInterfaces treat time starting at Frame 0.
	m_startTime = s + CHgetManager()->getSecsPerSample();
	m_endTime = e + CHgetManager()->getSecsPerSample();

	return true;
}

ROP_RENDER_CODE ROP_SceneCacheWriter::renderFrame( fpreal time, UT_Interrupt *boss )
{
	// We need to adjust the time for writing, because Houdini treats time starting
	// at Frame 1, while SceneInterfaces treat time starting at Frame 0.
	double writeTime = time + CHgetManager()->getSecsPerSample();

	// the interruptor passed in is null for some reason, so just get the global one
	UT_Interrupt *progress = UTgetInterrupt();
	if ( !progress->opStart( ( boost::format( "Writing Houdini time %f as SceneCache time %f" ) % time % writeTime ).str().c_str() ) )
	{
		addError( 0, "Cache aborted" );
		return ROP_ABORT_RENDER;
	}

	executePreFrameScript( time );

	// update the default evaluation time to avoid double cooking
	m_liveHoudiniScene->setDefaultTime( writeTime );

	SceneInterfacePtr outScene = m_outScene;

	// we need to re-root the scene if its trying to cache a top level object
	UT_String nodePath;
	evalString( nodePath, pRootObject.getToken(), 0, 0 );
	OBJ_Node *node = this->findNode( nodePath )->castToOBJNode();
	if ( node && node->getObjectType() == OBJ_GEOMETRY )
	{
		bool reRoot = true;
		OP_Context context( time );
		if ( const GU_Detail *geo = node->getRenderGeometry( context, false ) )
		{
			GA_ROHandleS nameAttrib( geo, GA_ATTRIB_PRIMITIVE, GA_Names::name );
			if ( nameAttrib.isValid() )
			{
				reRoot = false;
				GA_StringTableStatistics stats;
				const GA_Attribute *nameAttr = nameAttrib.getAttribute();
				const GA_AIFSharedStringTuple *tuple = nameAttr->getAIFSharedStringTuple();
				tuple->getStatistics( nameAttr, stats );
				GA_Size numShapes = stats.getEntries();
				if ( numShapes == 0 )
				{
					reRoot = true;
				}
				else if ( numShapes == 1 )
				{
					GA_Size numStrings = stats.getCapacity();
					for ( GA_Size i=0; i < numStrings; ++i )
					{
						GA_StringIndexType validatedIndex = tuple->validateTableHandle( nameAttr, i );
						if ( validatedIndex < 0 )
						{
							continue;
						}

						const char *name = tuple->getTableString( nameAttr, validatedIndex );
						if( ( name == 0 ) || ( !strcmp( name, "" ) || !strcmp( name, "/" ) ) )
						{
							reRoot = true;
						}
					}
				}
			}
		}
		else
		{
			UT_String msg;
			std::string messages = "Re-rooting flat geo failed.";
			node->getErrorMessages( msg );
			if ( msg != UT_String::getEmptyString() )
			{
				messages += "\n\nErrors from " + nodePath.toStdString() + ":\n" + msg.toStdString();
			}

			if ( SOP_Node *sop = node->getRenderSopPtr() )
			{
				sop->getErrorMessages( msg );
				if ( msg != UT_String::getEmptyString() )
				{
					sop->getFullPath( nodePath );
					messages += "\n\nErrors from " + nodePath.toStdString() + ":\n" + msg.toStdString();
				}
			}

			addError( 0, messages.c_str() );
			progress->opEnd();
			return ROP_ABORT_RENDER;
		}

		if ( reRoot )
		{
			outScene = m_outScene->child( node->getName().toStdString(), SceneInterface::CreateIfMissing );
		}
	}

	ROP_RENDER_CODE status = doWrite( m_liveScene.get(), outScene.get(), writeTime, progress );
	if ( status != ROP_ABORT_RENDER )
	{
		executePostFrameScript( time );
	}

	progress->opEnd();
	return status;
}

ROP_RENDER_CODE ROP_SceneCacheWriter::endRender()
{
	m_liveHoudiniScene = 0;
	m_liveScene = 0;
	m_outScene = 0;

	return ROP_CONTINUE_RENDER;
}

ROP_RENDER_CODE ROP_SceneCacheWriter::doWrite( const SceneInterface *liveScene, SceneInterface *outScene, double time, UT_Interrupt *progress )
{
	SceneInterface::Path p;
	std::string strPath;
	liveScene->path( p );
	SceneInterface::pathToString( p, strPath );

	progress->setLongOpText( ( "Writing " + liveScene->name().string() ).c_str() );
	if ( progress->opInterrupt() )
	{
		addError( 0, ( "Cache aborted during " + liveScene->name().string() ).c_str() );
		return ROP_ABORT_RENDER;
	}

	if ( liveScene != m_liveScene )
	{
		outScene->writeTransform( liveScene->readTransform( time ).get(), time );
	}

	Mode mode = NaturalExpand;
	const LiveScene *hScene = IECore::runTimeCast<const LiveScene>( liveScene );
	if ( hScene && m_forceFilter )
	{
		UT_String nodePath;
		hScene->node()->getFullPath( nodePath );
		mode = ( nodePath.multiMatch( *m_forceFilter ) ) ? ForcedExpand : ForcedLink;
	}

	SceneInterface::NameList attrs;
	liveScene->attributeNames( attrs );
	for ( SceneInterface::NameList::iterator it = attrs.begin(); it != attrs.end(); ++it )
	{
		if ( *it == LinkedScene::linkAttribute )
		{
			if ( mode == ForcedExpand )
			{
				continue;
			}

			mode = NaturalLink;
		}

		if ( ConstObjectPtr data = liveScene->readAttribute( *it, time ) )
		{
			outScene->writeAttribute( *it, data.get(), time );
		}
	}

	if ( mode == ForcedLink )
	{
		if ( const SceneCacheNode<OP_Node> *sceneNode = static_cast< const SceneCacheNode<OP_Node>* >( hScene->node() ) )
		{
			if ( ConstSceneInterfacePtr scene = sceneNode->scene() )
			{
				outScene->writeAttribute( LinkedScene::linkAttribute, LinkedScene::linkAttributeData( scene.get(), time ).get(), time );
				return ROP_CONTINUE_RENDER;
			}
		}
	}

	if ( mode == NaturalLink )
	{
		return ROP_CONTINUE_RENDER;
	}

	SceneInterface::NameList tags;
	try
	{
		liveScene->readTags( tags );
	}
	catch ( std::runtime_error &e )
	{
		addError( ROP_MESSAGE,
			boost::str(
				boost::format(
					"ROP Scene Cache Writer: Error reading tags for location %1% See below for more details.\n%2%") % strPath % e.what()
				).c_str()
		);
		return ROP_ABORT_RENDER;
	}
	outScene->writeTags( tags );

	bool hasObject = false;

	try
	{
		hasObject = liveScene->hasObject();
	}
	catch( const IECore::Exception &e )
	{
		addError( ROP_MESSAGE, e.what() );
	}

	if( hasObject )
	{
		try
		{
			auto srcObject = liveScene->readObject( time );
			if ( !srcObject )
			{
				addError( ROP_MESSAGE, boost::str( boost::format("unable to convert '%1%'") % strPath).c_str() );
			}
			outScene->writeObject( srcObject.get(), time );
		}
		catch ( IECore::Exception &e )
		{
			addError( ROP_MESSAGE, e.what() );
			return ROP_ABORT_RENDER;
		}
		catch ( std::runtime_error &e )
		{
			addError( ROP_MESSAGE,
				boost::str(
					boost::format(
						"ROP Scene Cache Writer: Error reading object for location %1% See below for more details.\n%2%") % strPath % e.what()
				).c_str()
				
			);
			return ROP_ABORT_RENDER;
		}
	}

	SceneInterface::NameList children;
	liveScene->childNames( children );
	for ( SceneInterface::NameList::iterator it = children.begin(); it != children.end(); ++it )
	{
		ConstSceneInterfacePtr liveChild = liveScene->child( *it );

		SceneInterfacePtr outChild = 0;
		if ( outScene->hasChild( *it ) )
		{
			outChild = outScene->child( *it );
		}
		else
		{
			outChild = outScene->createChild( *it );

			if ( time != m_startTime )
			{
				try
				{
					outChild->writeAttribute( changingHierarchyAttribute, new BoolData( true ), time );
					outChild->writeAttribute( IECoreScene::SceneInterface::visibilityName, new BoolData( false ), time - 1e-6 );
				}
				catch( const std::exception &e)
				{
					addError( ROP_MESSAGE,
						boost::str(
							boost::format(
							"ROP Scene Cache Writer: Name prim attribute (locations) are changing over time. Are the names consistent between time samples? See below for more details.\n%1%") % e.what()
						).c_str()
					);
					return ROP_ABORT_RENDER;
				}
			}
		}

		if ( outChild->hasAttribute( changingHierarchyAttribute ) )
		{
			try
			{
				outChild->writeAttribute( IECoreScene::SceneInterface::visibilityName, new BoolData( true ), time );
			}
			catch( const std::exception &e)
			{
				addError( ROP_MESSAGE,
					boost::str(
						boost::format(
							"ROP Scene Cache Writer: Name prim attribute (locations) are changing over time. Are the names consistent between time samples? See below for more details\n%1%") % e.what()
					).c_str()
				);
				return ROP_ABORT_RENDER;
			}
		}

		ROP_RENDER_CODE status = doWrite( liveChild.get(), outChild.get(), time, progress );
		if ( status != ROP_CONTINUE_RENDER )
		{
			return status;
		}
	}

	// turn visibility off if the child disappears
	SceneInterface::NameList outChildren;
	outScene->childNames( outChildren );
	for ( SceneInterface::NameList::iterator it = outChildren.begin(); it != outChildren.end(); ++it )
	{
		if ( !liveScene->hasChild( *it ) )
		{
			SceneInterfacePtr outChild = outScene->child( *it );
			if ( !outChild->hasAttribute( IECoreScene::SceneInterface::visibilityName ) )
			{
				try
				{
					outChild->writeAttribute( IECoreScene::SceneInterface::visibilityName, new BoolData( true ), time - 1e-6 );
				}
				catch( const std::exception &e)
				{
					addError( ROP_MESSAGE,
						boost::str(
							boost::format(
								"ROP Scene Cache Writer: Name prim attribute (locations) are changing over time. Are the names consistent between time samples? See below for more details\n%1%") % e.what()
						).c_str()
					);
					return ROP_ABORT_RENDER;
				}
			}

			try
			{
				outChild->writeAttribute( changingHierarchyAttribute, new BoolData( true ), time );
				outChild->writeAttribute( IECoreScene::SceneInterface::visibilityName, new BoolData( false ), time );
			}
			catch( const std::exception &e)
			{
				addError( ROP_MESSAGE,
					boost::str(
						boost::format(
							"ROP Scene Cache Writer: Name prim attribute (locations) are changing over time. Are the names consistent between time samples? See below for more details\n%1%" ) % e.what()
					).c_str()
				);
				return ROP_ABORT_RENDER;
			}
		}
	}

	return ROP_CONTINUE_RENDER;
}

bool ROP_SceneCacheWriter::updateParmsFlags()
{
	UT_String value;
	evalString( value, pFile.getToken(), 0, 0 );
	std::string file = value.toStdString();
	enableParm( pForceObjects.getToken(), linked( file ) );
	return true;
}

bool ROP_SceneCacheWriter::linked( const std::string &file ) const
{
	return ( boost::filesystem::path( file ).extension().string() == ".lscc" );
}
