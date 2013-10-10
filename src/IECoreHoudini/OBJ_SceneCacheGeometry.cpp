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

#include "IECoreHoudini/OBJ_SceneCacheGeometry.h"
#include "IECoreHoudini/SOP_SceneCacheSource.h"

using namespace IECore;
using namespace IECoreHoudini;

const char *OBJ_SceneCacheGeometry::typeName = "ieSceneCacheGeometry";

OBJ_SceneCacheGeometry::OBJ_SceneCacheGeometry( OP_Network *net, const char *name, OP_Operator *op ) : OBJ_SceneCacheNode<OBJ_Geometry>( net, name, op )
{
}

OBJ_SceneCacheGeometry::~OBJ_SceneCacheGeometry()
{
}

OP_Node *OBJ_SceneCacheGeometry::create( OP_Network *net, const char *name, OP_Operator *op )
{
	return new OBJ_SceneCacheGeometry( net, name, op );
}

OP_TemplatePair *OBJ_SceneCacheGeometry::buildParameters()
{
	static OP_TemplatePair *templatePair = 0;
	if ( !templatePair )
	{
		templatePair = new OP_TemplatePair( OBJ_SceneCacheNode<OBJ_Geometry>::buildParameters() );
	}
	
	return templatePair;
}

void OBJ_SceneCacheGeometry::expandHierarchy( const SceneInterface *scene )
{
	if ( !scene )
	{
		return;
	}
	
	doExpandGeometry( scene );
	setInt( pExpanded.getToken(), 0, 0, 1 );
}

void OBJ_SceneCacheGeometry::pushToHierarchy()
{
	UT_String attribFilter, tagFilter, shapeFilter;
	getAttributeFilter( attribFilter );
	getTagFilter( tagFilter );
	getShapeFilter( shapeFilter );
	GeometryType geomType = getGeometryType();
	
	UT_PtrArray<OP_Node*> children;
	int numSceneSops = getOpsByName( SOP_SceneCacheSource::typeName, children );
	for ( int i=0; i < numSceneSops; ++i )
	{
		SOP_SceneCacheSource *sop = reinterpret_cast<SOP_SceneCacheSource*>( children[i] );
		sop->setAttributeFilter( attribFilter );
		sop->setTagFilter( tagFilter );
		sop->setShapeFilter( shapeFilter );
		sop->setGeometryType( (SOP_SceneCacheSource::GeometryType)geomType );
	}
}

void OBJ_SceneCacheGeometry::doExpandGeometry( const SceneInterface *scene )
{
	const char *name = ( scene->name() == SceneInterface::rootName ) ? "root" : scene->name().c_str();
	OP_Node *opNode = createNode( SOP_SceneCacheSource::typeName, name );
	SOP_SceneCacheSource *sop = reinterpret_cast<SOP_SceneCacheSource*>( opNode );
	
	sop->referenceParent( pFile.getToken() );
	sop->referenceParent( pRoot.getToken() );
	
	bool objectOnly = true;
	Space space = getSpace();
	SOP_SceneCacheSource::Space sopSpace = SOP_SceneCacheSource::Object;
	if ( space == World || space == Path )
	{
		objectOnly = false;
		sopSpace = SOP_SceneCacheSource::Path;
	}
	
	UT_String attribFilter, tagFilter, shapeFilter;
	getAttributeFilter( attribFilter );
	sop->setAttributeFilter( attribFilter );
	getTagFilter( tagFilter );
	sop->setTagFilter( tagFilter );
	getShapeFilter( shapeFilter );
	sop->setShapeFilter( shapeFilter );
	
	sop->setSpace( sopSpace );
	sop->setObjectOnly( objectOnly );
	sop->setGeometryType( (SOP_SceneCacheSource::GeometryType)getGeometryType() );
}
