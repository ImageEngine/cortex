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

#include "OP/OP_Layout.h"
#include "PRM/PRM_ChoiceList.h"
#include "UT/UT_StringMMPattern.h"

#include "IECoreHoudini/OBJ_SceneCacheGeometry.h"
#include "IECoreHoudini/OBJ_SceneCacheTransform.h"

using namespace IECore;
using namespace IECoreHoudini;

const char *OBJ_SceneCacheTransform::typeName = "ieSceneCacheTransform";

OBJ_SceneCacheTransform::OBJ_SceneCacheTransform( OP_Network *net, const char *name, OP_Operator *op ) : OBJ_SceneCacheNode<OBJ_SubNet>( net, name, op )
{
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
PRM_Name OBJ_SceneCacheTransform::pTagFilter( "tagFilter", "Tag Filter" );

PRM_Default OBJ_SceneCacheTransform::hierarchyDefault( SubNetworks );
PRM_Default OBJ_SceneCacheTransform::depthDefault( AllDescendants );
PRM_Default OBJ_SceneCacheTransform::filterDefault( 0, "*" );

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
PRM_ChoiceList OBJ_SceneCacheTransform::tagFilterMenu( PRM_CHOICELIST_TOGGLE, &OBJ_SceneCacheTransform::buildTagFilterMenu );

OP_TemplatePair *OBJ_SceneCacheTransform::buildParameters()
{
	static OP_TemplatePair *templatePair = 0;
	if ( !templatePair )
	{
		OP_TemplatePair *firstTemplatePair = new OP_TemplatePair( buildExtraParameters()->myTemplate, buildExpansionParameters() );
		templatePair = new OP_TemplatePair( buildBaseParameters()->myTemplate, firstTemplatePair );
	}
	
	return templatePair;
}

OP_TemplatePair *OBJ_SceneCacheTransform::buildExtraParameters()
{
	static PRM_Template *thisTemplate = 0;
	if ( !thisTemplate )
	{
		thisTemplate = new PRM_Template[4];
		
		thisTemplate[0] = PRM_Template(
			PRM_STRING, 1, &pTagFilter, &filterDefault, &tagFilterMenu, 0, 0, 0, 0,
			"A list of filters to decide which tags to display when expanding. All children will be created, "
			"the tag filters just control initial visibility. Uses Houdini matching syntax, but nodes will be "
			"visible if the filter matches *any* of their tags."
		);
		thisTemplate[1] = PRM_Template(
			PRM_INT, 1, &pHierarchy, &hierarchyDefault, &hierarchyList, 0, 0, 0, 0,
			"Choose the node network style used when expanding. Parenting will create a graph using "
			"node connections, SubNetworks will create a deep hierarchy, and Flat Geometry will "
			"create a single OBJ and SOP."
		);
		thisTemplate[2] = PRM_Template(
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

void OBJ_SceneCacheTransform::buildTagFilterMenu( void *data, PRM_Name *menu, int maxSize, const PRM_SpareData *, const PRM_Parm * )
{
	OBJ_SceneCacheTransform *node = reinterpret_cast<OBJ_SceneCacheTransform*>( data );
	if ( !node )
	{
		return;
	}
	
	menu[0].setToken( "*" );
	menu[0].setLabel( "*" );
	
	std::string file;
	if ( !node->ensureFile( file ) )
	{
		// mark the end of our menu
		menu[1].setToken( 0 );
		return;
	}
	
	ConstSceneInterfacePtr scene = node->scene( file, node->getPath() );
	if ( !scene )
	{
		// mark the end of our menu
		menu[1].setToken( 0 );
		return;
	}
	
	SceneInterface::NameList tags;
	scene->readTags( tags );
	std::vector<std::string> tagStrings;
	for ( SceneInterface::NameList::const_iterator it=tags.begin(); it != tags.end(); ++it )
	{
		tagStrings.push_back( *it );
	}
	
	node->createMenu( menu, tagStrings );
}

void OBJ_SceneCacheTransform::expandHierarchy( const SceneInterface *scene )
{
	if ( !scene )
	{
		return;
	}
	
	GeometryType geomType = getGeometryType();
	Depth depth = (Depth)evalInt( pDepth.getToken(), 0, 0 );
	Hierarchy hierarchy = (Hierarchy)evalInt( pHierarchy.getToken(), 0, 0 );
	
	UT_String attributeFilter;
	getAttributeFilter( attributeFilter );
	
	UT_String tagFilterStr;
	evalString( tagFilterStr, pTagFilter.getToken(), 0, 0 );
	UT_StringMMPattern tagFilter;
	tagFilter.compile( tagFilterStr );
	
	if ( hierarchy == FlatGeometry )
	{
		// Collapse first, in case the immediate object was already created on during parent expansion
		collapseHierarchy();
		doExpandObject( scene, this, geomType, hierarchy, depth, attributeFilter, tagFilter );
		setInt( pExpanded.getToken(), 0, 0, 1 );
		return;
	}
	
	OBJ_Node *rootNode = this;
	if ( scene->hasObject() )
	{
		OBJ_Node *objNode = doExpandObject( scene, this, geomType, SubNetworks, Children, attributeFilter, tagFilter );
		if ( hierarchy == Parenting )
		{
			rootNode = objNode;
		}
	}
	else if ( hierarchy == Parenting )
	{
		/// \todo: this is terrible. can we use the subnet input instead?
		rootNode = reinterpret_cast<OBJ_Node*>( createNode( "geo", "TMP" ) );
	}
	
	if ( hierarchy == Parenting )
	{
		rootNode->setIndirectInput( 0, this->getParentInput( 0 ) );
	}
	
	doExpandChildren( scene, rootNode, geomType, hierarchy, depth, attributeFilter, tagFilter );
	setInt( pExpanded.getToken(), 0, 0, 1 );
	
	if ( hierarchy == Parenting && !scene->hasObject() )
	{
		destroyNode( rootNode );
	}
}

OBJ_Node *OBJ_SceneCacheTransform::doExpandObject( const SceneInterface *scene, OP_Network *parent, GeometryType geomType, Hierarchy hierarchy, Depth depth, const UT_String &attributeFilter, const UT_StringMMPattern &tagFilter )
{
	const char *name = ( hierarchy == Parenting ) ? scene->name().c_str() : "geo";
	OP_Node *opNode = parent->createNode( OBJ_SceneCacheGeometry::typeName, name );
	OBJ_SceneCacheGeometry *geo = reinterpret_cast<OBJ_SceneCacheGeometry*>( opNode );
	
	geo->referenceParent( pFile.getToken() );
	geo->setPath( scene );
	
	Space space = ( depth == AllDescendants ) ? Path : ( hierarchy == Parenting ) ? Local : Object;
	geo->setSpace( (OBJ_SceneCacheGeometry::Space)space );
	geo->setGeometryType( (OBJ_SceneCacheGeometry::GeometryType)geomType );
	geo->setAttributeFilter( attributeFilter );
	
	geo->expandHierarchy( scene );
	
	geo->setVisible( tagged( scene, tagFilter ) );
	
	if ( hierarchy != Parenting )
	{
		geo->setIndirectInput( 0, parent->getParentInput( 0 ) );
	}
	
	return geo;
}

OBJ_Node *OBJ_SceneCacheTransform::doExpandChild( const SceneInterface *scene, OP_Network *parent, GeometryType geomType, Hierarchy hierarchy, Depth depth, const UT_String &attributeFilter, const UT_StringMMPattern &tagFilter )
{
	OP_Node *opNode = parent->createNode( OBJ_SceneCacheTransform::typeName, scene->name().c_str() );
	OBJ_SceneCacheTransform *xform = reinterpret_cast<OBJ_SceneCacheTransform*>( opNode );
	
	xform->referenceParent( pFile.getToken() );
	xform->setPath( scene );
	xform->setSpace( Local );
	xform->setGeometryType( (OBJ_SceneCacheTransform::GeometryType)geomType );
	xform->setAttributeFilter( attributeFilter );
	xform->setInt( pHierarchy.getToken(), 0, 0, hierarchy );
	xform->setInt( pDepth.getToken(), 0, 0, depth );
	
	SceneInterface::NameList children;
	scene->childNames( children );
	if ( children.empty() && !scene->hasObject() )
	{
		xform->setInt( pExpanded.getToken(), 0, 0, 1 );
	}
	
	if ( tagged( scene, tagFilter ) )
	{
		// we can't get the string directly from the UT_StringMMPattern, and we don't want to re-compile the UT_StringMMPattern
		// from a string during a recursive expansion, so we get the filter string from the parameter again.
		UT_String tagFilterStr;
		evalString( tagFilterStr, pTagFilter.getToken(), 0, 0 );
		xform->setString( tagFilterStr, CH_STRING_LITERAL, pTagFilter.getToken(), 0, 0 );
	}
	else
	{
		xform->setVisible( false );
	}
	
	if ( hierarchy == SubNetworks )
	{
		xform->setIndirectInput( 0, parent->getParentInput( 0 ) );
	}
	
	return xform;
}

void OBJ_SceneCacheTransform::doExpandChildren( const SceneInterface *scene, OP_Network *parent, GeometryType geomType, Hierarchy hierarchy, Depth depth, const UT_String &attributeFilter, const UT_StringMMPattern &tagFilter )
{
	OP_Network *inputNode = parent;
	if ( hierarchy == Parenting )
	{
		parent = parent->getParent();
	}
	
	SceneInterface::NameList children;
	scene->childNames( children );
	for ( SceneInterface::NameList::const_iterator it=children.begin(); it != children.end(); ++it )
	{
		ConstSceneInterfacePtr child = scene->child( *it );
		
		OBJ_Node *childNode = 0;
		if ( hierarchy == SubNetworks )
		{
			childNode = doExpandChild( child, parent, geomType, hierarchy, depth, attributeFilter, tagFilter );
			if ( depth == AllDescendants && child->hasObject() )
			{
				doExpandObject( child, childNode, geomType, hierarchy, Children, attributeFilter, tagFilter );
			}
		}
		else if ( hierarchy == Parenting )
		{
			if ( child->hasObject() )
			{
				childNode = doExpandObject( child, parent, geomType, hierarchy, Children, attributeFilter, tagFilter );
			}
			else
			{
				childNode = doExpandChild( child, parent, geomType, hierarchy, depth, attributeFilter, tagFilter );
			}
			
			childNode->setInput( 0, inputNode );
		}
		
		if ( depth == AllDescendants )
		{
			doExpandChildren( child, childNode, geomType, hierarchy, depth, attributeFilter, tagFilter );
			childNode->setInt( pExpanded.getToken(), 0, 0, 1 );
		}
	}
	
	OP_Layout layout( parent );
	layout.addLayoutOp( parent->getParentInput( 0 ) );
	for ( int i=0; i < parent->getNchildren(); ++i )
	{
		layout.addLayoutOp( parent->getChild( i ) );
	}
	layout.layoutOps( OP_LAYOUT_TOP_TO_BOT, parent, parent->getParentInput( 0 ) );
}

bool OBJ_SceneCacheTransform::tagged( const IECore::SceneInterface *scene, const UT_StringMMPattern &filter )
{
	SceneInterface::NameList tags;
	scene->readTags( tags );
	for ( SceneInterface::NameList::const_iterator it=tags.begin(); it != tags.end(); ++it )
	{
		if ( UT_String( *it ).multiMatch( filter ) )
		{
			return true;
		}
	}
	
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Registration for HoudiniScene extra attributes
//////////////////////////////////////////////////////////////////////////////////////////

OBJ_SceneCacheTransform::HoudiniSceneAddOn OBJ_SceneCacheTransform::g_houdiniSceneAddOn;

OBJ_SceneCacheTransform::HoudiniSceneAddOn::HoudiniSceneAddOn()
{
	HoudiniScene::registerCustomAttribute( LinkedScene::linkAttribute, OBJ_SceneCacheTransform::hasLink, OBJ_SceneCacheTransform::readLink );
	HoudiniScene::registerCustomTags( OBJ_SceneCacheTransform::hasTag, OBJ_SceneCacheTransform::readTags );
}

bool OBJ_SceneCacheTransform::hasLink( const OP_Node *node )
{
	// make sure its a SceneCacheNode
	if ( !node->hasParm( pFile.getToken() ) || !node->hasParm( pRoot.getToken() ) )
	{
		return false;
	}
	
	const char *expanded = pExpanded.getToken();
	if ( node->hasParm( expanded ) && !node->evalInt( expanded, 0, 0 ) )
	{
		return true;
	}
	
	return false;
}

IECore::ConstObjectPtr OBJ_SceneCacheTransform::readLink( const OP_Node *node )
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
	
	return LinkedScene::linkAttributeData( scene );
}

bool OBJ_SceneCacheTransform::hasTag( const OP_Node *node, const SceneInterface::Name &tag )
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
	
	return scene->hasTag( tag );
}

void OBJ_SceneCacheTransform::readTags( const OP_Node *node, SceneInterface::NameList &tags, bool includeChildren )
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
	
	scene->readTags( tags, includeChildren );
}
