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

#include "PRM/PRM_ChoiceList.h"

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
	static PRM_Template *thisTemplate = 0;
	if ( !thisTemplate )
	{
		PRM_Template *parentTemplate = OBJ_SceneCacheNode<OBJ_SubNet>::buildParameters()->myTemplate;
		unsigned numParentParms = PRM_Template::countTemplates( parentTemplate );
		thisTemplate = new PRM_Template[ numParentParms + 3 ];
		
		// add the common OBJ parms
		for ( unsigned i = 0; i < numParentParms - 1; ++i )
		{
			thisTemplate[i] = parentTemplate[i];
		}
		
		// then the build options
		thisTemplate[numParentParms-1] = PRM_Template(
			PRM_INT, 1, &pHierarchy, &hierarchyDefault, &hierarchyList, 0, 0, 0, 0,
			"Choose the node network style used when building. Parenting will create a graph using "
			"node connections, SubNetworks will create a deep hierarchy, and Flat Geometry will "
			"create a single OBJ and SOP."
		);
		thisTemplate[numParentParms] = PRM_Template(
			PRM_INT, 1, &pDepth, &depthDefault, &depthList, 0, 0, 0, 0,
			"Choose how deep to build. All Descendants will build everything below the specified root "
			"path and Children will only build the immediate children of the root path, which may "
			"or may not contain geometry."
		);
		
		// then the build button
		thisTemplate[numParentParms+1] = parentTemplate[numParentParms-1];
	}
	
	static OP_TemplatePair *templatePair = 0;
	if ( !templatePair )
	{
		templatePair = new OP_TemplatePair( thisTemplate );
	}
	
	return templatePair;
}

void OBJ_SceneCacheTransform::buildHierarchy( const SceneInterface *scene )
{
	if ( !scene )
	{
		return;
	}
	
	Depth depth = (Depth)evalInt( pDepth.getToken(), 0, 0 );
	Hierarchy hierarchy = (Hierarchy)evalInt( pHierarchy.getToken(), 0, 0 );
	
	if ( hierarchy == FlatGeometry )
	{
		doBuildObject( scene, this, hierarchy, depth );
		return;
	}
	
	OBJ_Node *rootNode = this;
	if ( scene->hasObject() )
	{
		OBJ_Node *objNode = doBuildObject( scene, this, SubNetworks, Children );
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
	
	doBuildChildren( scene, rootNode, hierarchy, depth );
	
	if ( hierarchy == Parenting && !scene->hasObject() )
	{
		destroyNode( rootNode );
	}
}

OBJ_Node *OBJ_SceneCacheTransform::doBuildObject( const SceneInterface *scene, OP_Network *parent, Hierarchy hierarchy, Depth depth )
{
	const char *name = ( hierarchy == Parenting ) ? scene->name().c_str() : "geo";
	OP_Node *opNode = parent->createNode( OBJ_SceneCacheGeometry::typeName, name );
	OBJ_SceneCacheGeometry *geo = reinterpret_cast<OBJ_SceneCacheGeometry*>( opNode );
	
	geo->setFile( getFile() );
	geo->setPath( scene );
	
	Space space = ( depth == AllDescendants ) ? Path : ( hierarchy == Parenting ) ? Local : Object;
	geo->setSpace( (OBJ_SceneCacheGeometry::Space)space );
	
	geo->buildHierarchy( scene );
	
	return geo;
}

OBJ_Node *OBJ_SceneCacheTransform::doBuildChild( const SceneInterface *scene, OP_Network *parent, Hierarchy hierarchy, Depth depth )
{
	OP_Node *opNode = parent->createNode( OBJ_SceneCacheTransform::typeName, scene->name().c_str() );
	OBJ_SceneCacheTransform *xform = reinterpret_cast<OBJ_SceneCacheTransform*>( opNode );
	
	xform->setFile( getFile() );
	xform->setPath( scene );
	xform->setSpace( Local );
	xform->setInt( pHierarchy.getToken(), 0, 0, hierarchy );
	xform->setInt( pDepth.getToken(), 0, 0, depth );
	
	return xform;
}

void OBJ_SceneCacheTransform::doBuildChildren( const SceneInterface *scene, OP_Network *parent, Hierarchy hierarchy, Depth depth )
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
			childNode = doBuildChild( child, parent, hierarchy, depth );
			if ( child->hasObject() )
			{
				doBuildObject( child, childNode, hierarchy, Children );
			}
		}
		else if ( hierarchy == Parenting )
		{
			if ( child->hasObject() )
			{
				childNode = doBuildObject( child, parent, hierarchy, Children );
			}
			else
			{
				childNode = doBuildChild( child, parent, hierarchy, depth );
			}
			
			childNode->setInput( 0, inputNode );
		}
		
		if ( depth == AllDescendants )
		{
			doBuildChildren( child, childNode, hierarchy, depth );
		}
	}
}
