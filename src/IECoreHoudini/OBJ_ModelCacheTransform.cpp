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

#include "IECoreHoudini/OBJ_ModelCacheGeometry.h"
#include "IECoreHoudini/OBJ_ModelCacheTransform.h"

using namespace IECore;
using namespace IECoreHoudini;

const char *OBJ_ModelCacheTransform::typeName = "ieModelCacheTransform";

OBJ_ModelCacheTransform::OBJ_ModelCacheTransform( OP_Network *net, const char *name, OP_Operator *op ) : OBJ_ModelCacheNode<OBJ_SubNet>( net, name, op )
{
}

OBJ_ModelCacheTransform::~OBJ_ModelCacheTransform()
{
}

OP_Node *OBJ_ModelCacheTransform::create( OP_Network *net, const char *name, OP_Operator *op )
{
	return new OBJ_ModelCacheTransform( net, name, op );
}

PRM_Name OBJ_ModelCacheTransform::pHierarchy( "hierarchy", "Hierarchy" );
PRM_Name OBJ_ModelCacheTransform::pDepth( "depth", "Depth" );

PRM_Default OBJ_ModelCacheTransform::hierarchyDefault( SubNetworks );
PRM_Default OBJ_ModelCacheTransform::depthDefault( AllDescendants );

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

PRM_ChoiceList OBJ_ModelCacheTransform::hierarchyList( PRM_CHOICELIST_SINGLE, &hierarchyNames[0] );
PRM_ChoiceList OBJ_ModelCacheTransform::depthList( PRM_CHOICELIST_SINGLE, &depthNames[0] );

OP_TemplatePair *OBJ_ModelCacheTransform::buildParameters()
{
	static PRM_Template *thisTemplate = 0;
	if ( !thisTemplate )
	{
		PRM_Template *parentTemplate = OBJ_ModelCacheNode<OBJ_SubNet>::buildParameters()->myTemplate;
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

void OBJ_ModelCacheTransform::buildHierarchy( const ModelCache *cache )
{
	Depth depth = (Depth)evalInt( pDepth.getToken(), 0, 0 );
	Hierarchy hierarchy = (Hierarchy)evalInt( pHierarchy.getToken(), 0, 0 );
	
	if ( hierarchy == FlatGeometry )
	{
		doBuildObject( cache, this, hierarchy, depth );
	}
	/// \todo: can we combine the two cases below by fixing up doBuildChildren?
	else if ( hierarchy == SubNetworks )
	{
		/// \todo: this doesn't work. should build the child's geo, not just this one...
		if ( cache->hasObject() )
		{
			doBuildObject( cache, this, hierarchy, Children );
		}
		
		IndexedIO::EntryIDList children;
		cache->childNames( children );
		for ( IndexedIO::EntryIDList::const_iterator it=children.begin(); it != children.end(); ++it )
		{
			doBuildChild( cache->readableChild( *it ), this, hierarchy, depth );
		}
	}
	else
	{
		OBJ_Node *objNode = 0;
		if ( cache->hasObject() )
		{
			objNode = doBuildObject( cache, this, SubNetworks, Children );
		}
		else
		{
			/// \todo: this is terrible. can we use the subnet input instead?
			objNode = reinterpret_cast<OBJ_Node*>( createNode( "geo", "TMP" ) );
		}
		
		doBuildChildren( cache, objNode, hierarchy, depth );
		
		if ( !cache->hasObject() )
		{
			destroyNode( objNode );
		}
	}
}

OBJ_Node *OBJ_ModelCacheTransform::doBuildObject( const ModelCache *cache, OP_Network *parent, Hierarchy hierarchy, Depth depth )
{
	const char *name = ( hierarchy == Parenting ) ? cache->name().c_str() : "geo";
	OP_Node *opNode = parent->createNode( OBJ_ModelCacheGeometry::typeName, name );
	OBJ_ModelCacheGeometry *geo = reinterpret_cast<OBJ_ModelCacheGeometry*>( opNode );
	
	geo->setFile( getFile() );
	geo->setPath( cache->path() );
	
	Space space = ( depth == AllDescendants ) ? Path : ( hierarchy == Parenting ) ? Local : Object;
	geo->setSpace( (OBJ_ModelCacheGeometry::Space)space );
	
	geo->buildHierarchy( cache );
	
	return geo;
}

OBJ_Node *OBJ_ModelCacheTransform::doBuildChild( const ModelCache *cache, OP_Network *parent, Hierarchy hierarchy, Depth depth )
{
	OP_Node *opNode = parent->createNode( OBJ_ModelCacheTransform::typeName, cache->name().c_str() );
	OBJ_ModelCacheTransform *xform = reinterpret_cast<OBJ_ModelCacheTransform*>( opNode );
	
	xform->setFile( getFile() );
	xform->setPath( cache->path() );
	xform->setSpace( Local );
	xform->setInt( pHierarchy.getToken(), 0, 0, hierarchy );
	xform->setInt( pDepth.getToken(), 0, 0, depth );
	
	if ( hierarchy == SubNetworks && depth == AllDescendants )
	{
		xform->buildHierarchy( cache );
	}
	
	return xform;
}

void OBJ_ModelCacheTransform::doBuildChildren( const ModelCache *cache, OP_Network *parent, Hierarchy hierarchy, Depth depth )
{
	IndexedIO::EntryIDList children;
	cache->childNames( children );
	for ( IndexedIO::EntryIDList::const_iterator it=children.begin(); it != children.end(); ++it )
	{
		ConstModelCachePtr child = cache->readableChild( *it );
		
		OBJ_Node *objNode = 0;
		if ( child->hasObject() )
		{
			objNode = doBuildObject( child, parent->getParent(), hierarchy, Children );
		}
		else
		{
			objNode = doBuildChild( child, parent->getParent(), hierarchy, depth );
		}
		
		objNode->setInput( 0, parent );
		
		if ( depth == AllDescendants )
		{
			doBuildChildren( child, objNode, hierarchy, depth );
		}
	}
}
