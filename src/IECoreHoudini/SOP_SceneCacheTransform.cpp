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

#include "OP/OP_NodeInfoParms.h"
#include "PRM/PRM_ChoiceList.h"
#include "PRM/PRM_Default.h"
#include "UT/UT_Interrupt.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreHoudini/GEO_CortexPrimitive.h"
#include "IECoreHoudini/SOP_SceneCacheTransform.h"

using namespace IECore;
using namespace IECoreHoudini;

static InternedString pName( "P" );

const char *SOP_SceneCacheTransform::typeName = "ieSceneCacheTransform";

PRM_Name SOP_SceneCacheTransform::pMode( "mode", "Mode" );
PRM_Name SOP_SceneCacheTransform::pInvert( "invert", "Invert" );

PRM_Default SOP_SceneCacheTransform::modeDefault( Name );

static PRM_Name modes[] = {
	PRM_Name( "0", "Name" ),
	PRM_Name( "1", "Root" ),
	PRM_Name( 0 ) // sentinal
};

PRM_ChoiceList SOP_SceneCacheTransform::modeList( PRM_CHOICELIST_SINGLE, &modes[0] );

OP_TemplatePair *SOP_SceneCacheTransform::buildParameters()
{
	static OP_TemplatePair *templatePair = 0;
	if ( templatePair )
	{
		return templatePair;
	}
	
	PRM_Template *mainTemplate = SOP_SceneCacheTransform::buildMainParameters()->myTemplate;
	unsigned numMainParms = PRM_Template::countTemplates( mainTemplate );
	static PRM_Template *thisTemplate = new PRM_Template[ numMainParms + 3 ];
	
	// add the generic SceneCacheNode parms
	unsigned totalParms = 0;
	for ( unsigned i = 0; i < numMainParms; ++i, ++totalParms )
	{
		thisTemplate[totalParms] = mainTemplate[i];
	}
	
	thisTemplate[totalParms] = PRM_Template(
		PRM_INT, 1, &pMode, &modeDefault, &modeList, 0, 0, 0, 0,
		"The transformation mode. \"Name\" mode transforms each named primitive range by the "
		"matching path in the given SceneCache file, relative to the \"root\" parameter. \"Root\" "
		"mode transforms all primitives by a single transform from the SceneCache file, as "
		"specified by the \"root\" parameter."
	);
	totalParms++;
	
	thisTemplate[totalParms] = PRM_Template(
		PRM_TOGGLE, 1, &pInvert, 0, 0, 0, 0, 0, 0,
		"Invert the transformation matrix before applying it to the geometry."
	);
	totalParms++;
	
	templatePair = new OP_TemplatePair( thisTemplate );
	return templatePair;
}

SOP_SceneCacheTransform::SOP_SceneCacheTransform( OP_Network *net, const char *name, OP_Operator *op ) : SceneCacheNode<SOP_Node>( net, name, op )
{
}

SOP_SceneCacheTransform::~SOP_SceneCacheTransform()
{
}

OP_Node *SOP_SceneCacheTransform::create( OP_Network *net, const char *name, OP_Operator *op )
{
	return new SOP_SceneCacheTransform( net, name, op );
}

void SOP_SceneCacheTransform::sceneChanged()
{
	SceneCacheNode<SOP_Node>::sceneChanged();
	m_static = boost::indeterminate;
}

OP_ERROR SOP_SceneCacheTransform::cookMySop( OP_Context &context )
{
	std::string file;
	if ( !ensureFile( file ) )
	{
		addError( SOP_ATTRIBUTE_INVALID, ( file + " is not a valid .scc" ).c_str() );
		gdp->clearAndDestroy();
		sceneChanged();
		return error();
	}
	
	std::string path = getPath();
	Space space = getSpace();
	Mode mode = (Mode)this->evalInt( pMode.getToken(), 0, 0 );
	bool invert = this->evalInt( pInvert.getToken(), 0, 0 );
	
	ConstSceneInterfacePtr scene = this->scene( file, path );
	if ( !scene )
	{
		addError( SOP_ATTRIBUTE_INVALID, ( path + " is not a valid location in " + file ).c_str() );
		gdp->clearAndDestroy();
		sceneChanged();
		return error();
	}
	
	MurmurHash hash;
	hash.append( file );
	hash.append( path );
	hash.append( space );
	
	if ( !m_loaded || m_hash != hash )
	{
		sceneChanged();
	}
	
	if ( lockInputs( context ) >= UT_ERROR_ABORT )
	{
		return error();
	}
	
	gdp->stashAll();
	duplicatePointSource( 0, context );
	
	UT_Interrupt *progress = UTgetInterrupt();
	if ( !progress->opStart( "Transforming objects" ) )
	{
		addError( SOP_ATTRIBUTE_INVALID, "Cooking interrupted before it started" );
		gdp->clearAndDestroy();
		unlockInputs();
		return error();
	}
	
	m_static = boost::indeterminate;
	
	double readTime = time( context );
	
	if ( mode == Root )
	{
		UT_Matrix4 transform = getTransform( scene.get(), scene.get(), readTime, space, invert );
		gdp->transform( transform );
	}
	else if ( mode == Name )
	{
		transformByName( scene.get(), readTime, space, invert );
	}
	else
	{
		addError( SOP_ATTRIBUTE_INVALID, "Invalid Mode" );
	}
	
	flags().setTimeDep( bool( !m_static ) );
	
	if ( progress->opInterrupt( 100 ) )
	{
		addError( SOP_ATTRIBUTE_INVALID, "Cooking interrupted" );
		gdp->clearAndDestroy();		
		m_loaded = false;
		m_hash = MurmurHash();
	}
	else
	{
		m_loaded = true;
		m_hash = hash;
	}
	
	progress->opEnd();
	
	unlockInputs();
	return error();
}

void SOP_SceneCacheTransform::transformByName( const SceneInterface *scene, double time, Space space, bool invert )
{
	// Building a map from shape name to primitive range
	/// \todo: this is duplicated from SOP_SceneCacheSource.cpp. Can it be unified somewhere?
	/// Is this an argument for a different kind of DetailSplitter? DetailRanger?
	std::map<std::string, GA_Range> namedRanges;
	GA_ROAttributeRef nameAttrRef = gdp->findStringTuple( GA_ATTRIB_PRIMITIVE, "name" );
	if ( nameAttrRef.isValid() )
	{
		const GA_Attribute *attr = nameAttrRef.getAttribute();
		const GA_AIFSharedStringTuple *tuple = attr->getAIFSharedStringTuple();
		
		std::map<std::string, GA_OffsetList> offsets;
		GA_Range primRange = gdp->getPrimitiveRange();
		for ( GA_Iterator it = primRange.begin(); !it.atEnd(); ++it )
		{
			std::string current = "";
			if ( const char *value = tuple->getString( attr, it.getOffset() ) )
			{
				current = value;
			}
			
			std::map<std::string, GA_OffsetList>::iterator oIt = offsets.find( current );
			if ( oIt == offsets.end() )
			{
				oIt = offsets.insert( std::pair<std::string, GA_OffsetList>( current, GA_OffsetList() ) ).first;
			}
			
			oIt->second.append( it.getOffset() );
		}
		
		for ( std::map<std::string, GA_OffsetList>::iterator oIt = offsets.begin(); oIt != offsets.end(); ++oIt )
		{
			namedRanges[oIt->first] = GA_Range( gdp->getPrimitiveMap(), oIt->second );
		}
	}
	
	std::string rootPathStr;
	SceneInterface::Path rootPath;
	scene->path( rootPath );
	SceneInterface::pathToString( rootPath, rootPathStr );
	
	for ( std::map<std::string, GA_Range>::iterator it = namedRanges.begin(); it != namedRanges.end(); ++it )
	{
		std::string fullPathStr = rootPathStr + it->first;
		SceneInterface::Path fullPath;
		SceneInterface::stringToPath( fullPathStr, fullPath );
		
		if ( ConstSceneInterfacePtr leaf = scene->scene( fullPath, SceneInterface::NullIfMissing ) )
		{
			UT_Matrix4 transform = getTransform( scene, leaf.get(), time, space, invert );
			gdp->transform( transform, it->second, false );
		}
	}
}

UT_Matrix4 SOP_SceneCacheTransform::getTransform( const SceneInterface *rootScene, const SceneInterface *scene, double time, Space space, bool invert )
{
	Imath::M44d transform;
	if ( space == World )
	{
		transform = relativeTransform( scene->scene( SceneInterface::rootPath ).get(), scene, time );
	}
	else if ( space == Local )
	{
		transform = scene->readTransformAsMatrix( time );
	}
	else if ( space == Path )
	{
		transform = relativeTransform( rootScene, scene, time );
	}
	
	// we don't need to update if we've already confirmed it is animated
	if ( m_static || boost::indeterminate( m_static ) )
	{
		if ( space == Local )
		{
			if ( const SampledSceneInterface *sampledScene = IECore::runTimeCast<const SampledSceneInterface>( scene ) )
			{
				m_static = ( sampledScene->numTransformSamples() < 2 );
			}
			else
			{
				m_static = false;
			}
		}
		else
		{
			// if we never set m_static false by this point, then the whole thing is static
			m_static = true;
		}
	}
	
	UT_Matrix4 matrix = UT_Matrix4( IECore::convert<UT_Matrix4D>( transform ) );
	if ( invert )
	{
		matrix.invert();
	}
	
	return matrix;
}

Imath::M44d SOP_SceneCacheTransform::relativeTransform( const IECore::SceneInterface *rootScene, const IECore::SceneInterface *scene, double time )
{
	Imath::M44d result;
	
	SceneInterface::Path p, rootPath;
	scene->path( p );
	rootScene->path( rootPath );
	
	ConstSceneInterfacePtr current = rootScene;
	for ( SceneInterface::Path::const_iterator it = p.begin() + rootPath.size(); current && it != p.end(); ++it )
	{
		current = current->child( *it, SceneInterface::NullIfMissing );
		if ( !current )
		{
			m_static = false;
			break;
		}
		
		if ( const SampledSceneInterface *sampledCurrent = IECore::runTimeCast<const SampledSceneInterface>( current.get() ) )
		{
			if ( sampledCurrent->numTransformSamples() > 1 )
			{
				m_static = false;
			}
		}
		else
		{
			m_static = false;
		}
		
		result = current->readTransformAsMatrix( time ) * result;
	}
	
	return result;
}

void SOP_SceneCacheTransform::getNodeSpecificInfoText( OP_Context &context, OP_NodeInfoParms &parms )
{
	SceneCacheNode<SOP_Node>::getNodeSpecificInfoText( context, parms );
	
	// add type descriptions for the Cortex Objects
	GEO_CortexPrimitive::infoText( getCookedGeo( context ), context, parms );
}
