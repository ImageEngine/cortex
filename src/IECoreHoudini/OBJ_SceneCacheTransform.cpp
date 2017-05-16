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

#include "OP/OP_Layout.h"

#if UT_MAJOR_VERSION_INT >= 16

#include "OP/OP_SubnetIndirectInput.h"

#endif

#include "PRM/PRM_ChoiceList.h"
#include "UT/UT_Interrupt.h"
#include "UT/UT_PtrArray.h"

#include "IECoreHoudini/OBJ_SceneCacheGeometry.h"
#include "IECoreHoudini/OBJ_SceneCacheTransform.h"

using namespace IECore;
using namespace IECoreHoudini;

const char *OBJ_SceneCacheTransform::typeName = "ieSceneCacheTransform";

int *OBJ_SceneCacheTransform::g_indirection = 0;

OBJ_SceneCacheTransform::OBJ_SceneCacheTransform( OP_Network *net, const char *name, OP_Operator *op ) : OBJ_SceneCacheNode<OBJ_SubNet>( net, name, op )
{
	if ( !g_indirection )
	{
		g_indirection = OP_Parameters::allocIndirect( this->getParmList()->getEntries() );
	}
}

OBJ_SceneCacheTransform::~OBJ_SceneCacheTransform()
{
}

OP_Node *OBJ_SceneCacheTransform::create( OP_Network *net, const char *name, OP_Operator *op )
{
	return new OBJ_SceneCacheTransform( net, name, op );
}

PRM_Name OBJ_SceneCacheTransform::pHierarchy( "hierarchy", "Hierarchy" );
PRM_Name OBJ_SceneCacheTransform::pDepth( "depth", "Depth" );

PRM_Default OBJ_SceneCacheTransform::hierarchyDefault( SubNetworks );
PRM_Default OBJ_SceneCacheTransform::depthDefault( AllDescendants );

static PRM_Name hierarchyNames[] = {
	PRM_Name( "0", "SubNetworks" ),
	PRM_Name( "1", "Parenting" ),
	PRM_Name( "2", "Flat Geometry" ),
	PRM_Name( 0 ) // sentinal
};

static PRM_Name depthNames[] = {
	PRM_Name( "0", "All Descendants" ),
	PRM_Name( "1", "Children" ),
	PRM_Name( 0 ) // sentinal
};

PRM_ChoiceList OBJ_SceneCacheTransform::hierarchyList( PRM_CHOICELIST_SINGLE, &hierarchyNames[0] );
PRM_ChoiceList OBJ_SceneCacheTransform::depthList( PRM_CHOICELIST_SINGLE, &depthNames[0] );

OP_TemplatePair *OBJ_SceneCacheTransform::buildParameters()
{
	static OP_TemplatePair *templatePair = 0;
	if ( !templatePair )
	{
		templatePair = new OP_TemplatePair( OBJ_SceneCacheNode<OBJ_SubNet>::buildParameters( buildExtraParameters() ) );
	}
	
	return templatePair;
}

OP_TemplatePair *OBJ_SceneCacheTransform::buildExtraParameters()
{
	static PRM_Template *thisTemplate = 0;
	if ( !thisTemplate )
	{
		thisTemplate = new PRM_Template[3];
		
		thisTemplate[0] = PRM_Template(
			PRM_INT, 1, &pHierarchy, &hierarchyDefault, &hierarchyList, 0, 0, 0, 0,
			"Choose the node network style used when expanding. Parenting will create a graph using "
			"node connections, SubNetworks will create a deep hierarchy, and Flat Geometry will "
			"create a single OBJ and SOP."
		);
		thisTemplate[1] = PRM_Template(
			PRM_INT, 1, &pDepth, &depthDefault, &depthList, 0, 0, 0, 0,
			"Choose how deep to expand. All Descendants will expand everything below the specified root "
			"path and Children will only expand the immediate children of the root path, which may "
			"or may not contain geometry."
		);
	}
	
	static OP_TemplatePair *templatePair = 0;
	if ( !templatePair )
	{
		templatePair = new OP_TemplatePair( thisTemplate );
	}
	
	return templatePair;
}

void OBJ_SceneCacheTransform::expandHierarchy( const SceneInterface *scene )
{
	if ( !scene )
	{
		return;
	}
	
	Parameters params;
	params.geometryType = getGeometryType();
	params.depth = (Depth)evalInt( pDepth.getToken(), 0, 0 );
	params.hierarchy = (Hierarchy)evalInt( pHierarchy.getToken(), 0, 0 );
	params.tagGroups = getTagGroups();
	getAttributeFilter( params.attributeFilter );
	getAttributeCopy( params.attributeCopy );
	getShapeFilter( params.shapeFilter );
	getTagFilter( params.tagFilterStr );
	getTagFilter( params.tagFilter );
	getFullPathName( params.fullPathName );
	
	if ( params.hierarchy == FlatGeometry )
	{
		// Collapse first, in case the immediate object was already created on during parent expansion
		collapseHierarchy();
		doExpandObject( scene, this, params );
		setInt( pExpanded.getToken(), 0, 0, 1 );
		return;
	}
	
	OBJ_Node *rootNode = this;
	if ( scene->hasObject() )
	{
		Parameters rootParams( params );
		rootParams.hierarchy = SubNetworks;
		rootParams.depth = Children;
		OBJ_Node *objNode = doExpandObject( scene, this, rootParams );
		if ( params.hierarchy == Parenting )
		{
			rootNode = objNode;
		}
	}
	else if ( params.hierarchy == Parenting )
	{
		/// \todo: this is terrible. can we use the subnet input instead?
		rootNode = reinterpret_cast<OBJ_Node*>( createNode( "geo", "TMP" ) );
	}
	
	if ( params.hierarchy == Parenting )
	{
		rootNode->setIndirectInput( 0, this->getParentInput( 0 ) );
	}
	
	UT_Interrupt *progress = UTgetInterrupt();
	if ( !progress->opStart( ( "Expand Hierarchy for " + getPath() ).c_str() ) )
	{
		return;
	}
	
	doExpandChildren( scene, rootNode, params );
	setInt( pExpanded.getToken(), 0, 0, 1 );
	
	if ( params.hierarchy == Parenting && !scene->hasObject() )
	{
		destroyNode( rootNode );
	}
	
	progress->opEnd();
}

OBJ_Node *OBJ_SceneCacheTransform::doExpandObject( const SceneInterface *scene, OP_Network *parent, const Parameters &params )
{
	const char *name = ( params.hierarchy == Parenting ) ? scene->name().c_str() : "geo";
	OP_Node *opNode = parent->createNode( OBJ_SceneCacheGeometry::typeName, name );
	OBJ_SceneCacheGeometry *geo = reinterpret_cast<OBJ_SceneCacheGeometry*>( opNode );
	
	geo->referenceParent( pFile.getToken() );
	if ( params.hierarchy == Parenting )
	{
		geo->setPath( scene );
	}
	else
	{
		geo->referenceParent( pRoot.getToken() );
		geo->setIndirectInput( 0, parent->getParentInput( 0 ) );
	}
	
	Space space = ( params.depth == AllDescendants ) ? Path : ( params.hierarchy == Parenting ) ? Local : Object;
	geo->setSpace( (OBJ_SceneCacheGeometry::Space)space );
	geo->setGeometryType( (OBJ_SceneCacheGeometry::GeometryType)params.geometryType );
	geo->setAttributeFilter( params.attributeFilter );
	geo->setAttributeCopy( params.attributeCopy );
	geo->setShapeFilter( params.shapeFilter );
	geo->setFullPathName( params.fullPathName );
	
	bool visible = tagged( scene, params.tagFilter );
	if ( visible )
	{
		geo->setTagFilter( params.tagFilterStr );
		geo->setTagGroups( params.tagGroups );
	}
	
	geo->setDisplay( visible );
	geo->expandHierarchy( scene );
	
	return geo;
}

OBJ_Node *OBJ_SceneCacheTransform::doExpandChild( const SceneInterface *scene, OP_Network *parent, const Parameters &params )
{
	OP_Node *opNode = parent->createNode( OBJ_SceneCacheTransform::typeName, scene->name().c_str() );
	OBJ_SceneCacheTransform *xform = reinterpret_cast<OBJ_SceneCacheTransform*>( opNode );
	
	xform->referenceParent( pFile.getToken() );
	xform->setPath( scene );
	xform->setSpace( Local );
	xform->setGeometryType( (OBJ_SceneCacheTransform::GeometryType)params.geometryType );
	xform->setAttributeFilter( params.attributeFilter );
	xform->setAttributeCopy( params.attributeCopy );
	xform->setShapeFilter( params.shapeFilter );
	xform->setFullPathName( params.fullPathName );
	xform->setInt( pHierarchy.getToken(), 0, 0, params.hierarchy );
	xform->setInt( pDepth.getToken(), 0, 0, params.depth );
	
	SceneInterface::NameList children;
	scene->childNames( children );
	if ( children.empty() && !scene->hasObject() )
	{
		xform->setInt( pExpanded.getToken(), 0, 0, 1 );
	}
	
	if ( tagged( scene, params.tagFilter ) )
	{
		xform->setTagFilter( params.tagFilterStr );
	}
	else
	{
		xform->setDisplay( false );
	}
	
	if ( params.hierarchy == SubNetworks )
	{
		xform->setIndirectInput( 0, parent->getParentInput( 0 ) );
	}
	
	return xform;
}

void OBJ_SceneCacheTransform::doExpandChildren( const SceneInterface *scene, OP_Network *parent, const Parameters &params )
{
	UT_Interrupt *progress = UTgetInterrupt();
	progress->setLongOpText( ( "Expanding " + scene->name().string() ).c_str() );
	if ( progress->opInterrupt() )
	{
		return;
	}
	
	OP_Network *inputNode = parent;
	if ( params.hierarchy == Parenting )
	{
		parent = parent->getParent();
	}
	
	SceneInterface::NameList children;
	scene->childNames( children );
	for ( SceneInterface::NameList::const_iterator it=children.begin(); it != children.end(); ++it )
	{
		ConstSceneInterfacePtr child = scene->child( *it );
		
		OBJ_Node *childNode = 0;
		if ( params.hierarchy == SubNetworks )
		{
			childNode = doExpandChild( child.get(), parent, params );
			if ( params.depth == AllDescendants && child->hasObject() && tagged( child.get(), params.tagFilter ) )
			{
				Parameters childParams( params );
				childParams.depth = Children;
				doExpandObject( child.get(), childNode, childParams );
			}
		}
		else if ( params.hierarchy == Parenting )
		{
			if ( child->hasObject() )
			{
				Parameters childParams( params );
				childParams.depth = Children;
				childNode = doExpandObject( child.get(), parent, childParams );
			}
			else
			{
				childNode = doExpandChild( child.get(), parent, params );
			}
			
			childNode->setInput( 0, inputNode );
		}
		
		if ( params.depth == AllDescendants )
		{
			if ( params.hierarchy == SubNetworks && !tagged( child.get(), params.tagFilter ) )
			{
				// we don't expand non-tagged children for SubNetwork mode, but we
				// do for Parenting mode, because otherwise the hierarchy would be
				// stuck in an un-expandable state.
				continue;
			}
			
			doExpandChildren( child.get(), childNode, params );
			childNode->setInt( pExpanded.getToken(), 0, 0, 1 );
		}
	}
	
	OP_Layout layout( parent );
	
#if UT_MAJOR_VERSION_INT >= 16

	OP_SubnetIndirectInput *parentInput = parent->getParentInput( 0 );
	layout.addLayoutItem( parentInput->getInputItem() );
	for ( int i=0; i < parent->getNchildren(); ++i )
	{
		layout.addLayoutItem( parent->getChild( i ) );
	}
	
#else

	layout.addLayoutOp( parent->getParentInput( 0 ) );
	for ( int i=0; i < parent->getNchildren(); ++i )
	{
		layout.addLayoutOp( parent->getChild( i ) );
	}
	
#endif

	layout.layoutOps( OP_LAYOUT_TOP_TO_BOT, parent, parent->getParentInput( 0 ) );
}

void OBJ_SceneCacheTransform::pushToHierarchy()
{
	UT_String attribFilter, attribCopy, shapeFilter, fullPathName;
	bool tagGroups = getTagGroups();
	getAttributeFilter( attribFilter );
	getAttributeCopy( attribCopy );
	getShapeFilter( shapeFilter );
	getFullPathName( fullPathName );
	GeometryType geometryType = getGeometryType();
	
	UT_String tagFilterStr;
	UT_StringMMPattern tagFilter;
	getTagFilter( tagFilterStr );
	tagFilter.compile( tagFilterStr );
	
	UT_PtrArray<OP_Node*> children;
	int numSceneNodes = getOpsByName( OBJ_SceneCacheTransform::typeName, children );
	for ( int i=0; i < numSceneNodes; ++i )
	{
		OBJ_SceneCacheTransform *xform = reinterpret_cast<OBJ_SceneCacheTransform*>( children( i ) );
		xform->setAttributeFilter( attribFilter );
		xform->setAttributeCopy( attribCopy );
		xform->setShapeFilter( shapeFilter );
		xform->setFullPathName( fullPathName );
		xform->setGeometryType( geometryType );
		
		std::string file;
		bool visible = false;
		if ( IECore::ConstSceneInterfacePtr scene = xform->scene() )
		{
			if ( tagged( scene.get(), tagFilter ) )
			{
				visible = true;
				xform->setTagFilter( tagFilterStr );
				xform->setTagGroups( tagGroups );
			}
		}
		
		xform->setRender( visible );
		xform->setDisplay( visible );
		xform->pushToHierarchy();
	}
	
	children.clear();
	numSceneNodes = getOpsByName( OBJ_SceneCacheGeometry::typeName, children );
	for ( int i=0; i < numSceneNodes; ++i )
	{
		OBJ_SceneCacheGeometry *geo = reinterpret_cast<OBJ_SceneCacheGeometry*>( children( i ) );
		geo->setAttributeFilter( attribFilter );
		geo->setAttributeCopy( attribCopy );
		geo->setShapeFilter( shapeFilter );
		geo->setFullPathName( fullPathName );
		geo->setGeometryType( (OBJ_SceneCacheGeometry::GeometryType)geometryType );
		
		std::string file;
		bool visible = false;
		if ( IECore::ConstSceneInterfacePtr scene = geo->scene() )
		{
			visible = tagged( scene.get(), tagFilter );
			if ( visible )
			{
				geo->setTagFilter( tagFilterStr );
				geo->setTagGroups( tagGroups );
			}
		}
		
		geo->setRender( visible );
		geo->setDisplay( visible );
		geo->pushToHierarchy();
	}
}

OBJ_SceneCacheTransform::Parameters::Parameters()
{
}

OBJ_SceneCacheTransform::Parameters::Parameters( const Parameters &other )
{
	geometryType = other.geometryType;
	hierarchy = other.hierarchy;
	depth = other.depth;
	attributeFilter = other.attributeFilter;
	attributeCopy = other.attributeCopy;
	shapeFilter = other.shapeFilter;
	tagFilterStr = other.tagFilterStr;
	tagFilter.compile( tagFilterStr );
	tagGroups = other.tagGroups;
	fullPathName = other.fullPathName;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Registration for LiveScene extra attributes
//////////////////////////////////////////////////////////////////////////////////////////

OBJ_SceneCacheTransform::LiveSceneAddOn OBJ_SceneCacheTransform::g_liveSceneAddOn;

OBJ_SceneCacheTransform::LiveSceneAddOn::LiveSceneAddOn()
{
	LiveScene::registerCustomAttributes( OBJ_SceneCacheTransform::attributeNames, OBJ_SceneCacheTransform::readAttribute );
	LiveScene::registerCustomTags( OBJ_SceneCacheTransform::hasTag, OBJ_SceneCacheTransform::readTags );
}

void OBJ_SceneCacheTransform::attributeNames( const OP_Node *node, SceneInterface::NameList &attrs )
{
	// make sure its a SceneCacheNode
	if ( !node->hasParm( pFile.getToken() ) || !node->hasParm( pRoot.getToken() ) )
	{
		return;
	}
	
	const SceneCacheNode<OP_Node> *sceneNode = reinterpret_cast< const SceneCacheNode<OP_Node>* >( node );
	/// \todo: do we need to ensure the file exists first?
	ConstSceneInterfacePtr scene = OBJ_SceneCacheTransform::scene( sceneNode->getFile(), sceneNode->getPath() );
	if ( !scene )
	{
		return;
	}
	
	scene->attributeNames( attrs );
	
	const char *expanded = pExpanded.getToken();
	if ( node->hasParm( expanded ) && !node->evalInt( expanded, 0, 0 ) )
	{
		attrs.push_back( LinkedScene::linkAttribute );
	}
}

IECore::ConstObjectPtr OBJ_SceneCacheTransform::readAttribute( const OP_Node *node, const SceneInterface::Name &name, double time )
{
	// make sure its a SceneCacheNode
	if ( !node->hasParm( pFile.getToken() ) || !node->hasParm( pRoot.getToken() ) )
	{
		return 0;
	}
	
	const SceneCacheNode<OP_Node> *sceneNode = reinterpret_cast< const SceneCacheNode<OP_Node>* >( node );
	/// \todo: do we need to ensure the file exists first?
	ConstSceneInterfacePtr scene = OBJ_SceneCacheTransform::scene( sceneNode->getFile(), sceneNode->getPath() );
	if ( !scene )
	{
		return 0;
	}
	
	if ( name == LinkedScene::linkAttribute )
	{
		const char *expanded = pExpanded.getToken();
		if ( node->hasParm( expanded ) && !node->evalInt( expanded, 0, 0 ) )
		{
			return LinkedScene::linkAttributeData( scene.get(), time );
		}
		
		return 0;
	}
	
	try
	{
		return scene->readAttribute( name, time );
	}
	catch( ... )
	{
		return 0;
	}
}

bool OBJ_SceneCacheTransform::hasTag( const OP_Node *node, const SceneInterface::Name &tag, int filter )
{
	// make sure its a SceneCacheNode
	if ( !node->hasParm( pFile.getToken() ) || !node->hasParm( pRoot.getToken() ) )
	{
		return false;
	}
	
	const SceneCacheNode<OP_Node> *sceneNode = reinterpret_cast< const SceneCacheNode<OP_Node>* >( node );
	/// \todo: do we need to ensure the file exists first?
	ConstSceneInterfacePtr scene = OBJ_SceneCacheTransform::scene( sceneNode->getFile(), sceneNode->getPath() );
	if ( !scene )
	{
		return false;
	}
	
	return scene->hasTag( tag, filter );
}

void OBJ_SceneCacheTransform::readTags( const OP_Node *node, SceneInterface::NameList &tags, int filter )
{
	// make sure its a SceneCacheNode
	if ( !node->hasParm( pFile.getToken() ) || !node->hasParm( pRoot.getToken() ) )
	{
		return;
	}
	
	const SceneCacheNode<OP_Node> *sceneNode = reinterpret_cast< const SceneCacheNode<OP_Node>* >( node );
	/// \todo: do we need to ensure the file exists first?
	ConstSceneInterfacePtr scene = OBJ_SceneCacheTransform::scene( sceneNode->getFile(), sceneNode->getPath() );
	if ( !scene )
	{
		return;
	}
	
	scene->readTags( tags, filter );
}

int *OBJ_SceneCacheTransform::getIndirect() const
{
	return g_indirection;
}
