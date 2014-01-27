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

#include <cassert>

#include "IECoreMaya/SceneShapeInterface.h"
#include "IECoreMaya/SceneShapeInterfaceComponentBoundIterator.h"

#include "boost/python.hpp"
#include "boost/tokenizer.hpp"

#include "OpenEXR/ImathMatrixAlgo.h"
#include "OpenEXR/ImathBoxAlgo.h"

#include "IECoreGL/Renderer.h"
#include "IECoreGL/Scene.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/NameStateComponent.h"
#include "IECoreGL/State.h"
#include "IECoreGL/Camera.h"
#include "IECoreGL/Renderable.h"
#include "IECoreGL/Group.h"

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/ToMayaMeshConverter.h"
#include "IECoreMaya/MayaTypeIds.h"
#include "IECoreMaya/PostLoadCallback.h"

#include "IECorePython/ScopedGILLock.h"
#include "IECorePython/ScopedGILRelease.h"

#include "IECore/VectorOps.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/CompoundParameter.h"
#include "IECore/AngleConversion.h"
#include "IECore/VisibleRenderable.h"
#include "IECore/TransformBlock.h"
#include "IECore/AttributeBlock.h"
#include "IECore/SampledSceneInterface.h"
#include "IECore/CurvesPrimitive.h"
#include "IECore/TransformOp.h"
#include "IECore/CoordinateSystem.h"
#include "IECore/Transform.h"
#include "IECore/MatrixAlgo.h"
#include "IECore/LinkedScene.h"

#include "maya/MFnNumericAttribute.h"
#include "maya/MFnEnumAttribute.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MFnGenericAttribute.h"
#include "maya/MFnUnitAttribute.h"
#include "maya/MFnCompoundAttribute.h"
#include "maya/MFnSingleIndexedComponent.h"
#include "maya/MSelectionList.h"
#include "maya/MAttributeSpec.h"
#include "maya/MAttributeIndex.h"
#include "maya/MAttributeSpecArray.h"
#include "maya/MDagPath.h"
#include "maya/MFnStringData.h"
#include "maya/MFnMeshData.h"
#include "maya/MFnNurbsCurveData.h"
#include "maya/MFnGeometryData.h"
#include "maya/MPlugArray.h"
#include "maya/MFileIO.h"

using namespace Imath;
using namespace IECore;
using namespace IECoreMaya;

MTypeId SceneShapeInterface::id = SceneShapeInterfaceId;
MObject SceneShapeInterface::aObjectOnly;
MObject SceneShapeInterface::aDrawGeometry;
MObject SceneShapeInterface::aDrawRootBound;
MObject SceneShapeInterface::aDrawChildBounds;
MObject SceneShapeInterface::aDrawTagsFilter;
MObject SceneShapeInterface::aQuerySpace;
MObject SceneShapeInterface::aTime;
MObject SceneShapeInterface::aOutTime;
MObject SceneShapeInterface::aSceneQueries;
MObject SceneShapeInterface::aAttributeQueries;
MObject SceneShapeInterface::aOutputObjects;
MObject SceneShapeInterface::aObjectDependency;
MObject SceneShapeInterface::aAttributes;
MObject SceneShapeInterface::aAttributeValues;
MObject SceneShapeInterface::aTransform;
MObject SceneShapeInterface::aTranslate;
MObject SceneShapeInterface::aTranslateX;
MObject SceneShapeInterface::aTranslateY;
MObject SceneShapeInterface::aTranslateZ;
MObject SceneShapeInterface::aRotate;
MObject SceneShapeInterface::aRotateX;
MObject SceneShapeInterface::aRotateY;
MObject SceneShapeInterface::aRotateZ;
MObject SceneShapeInterface::aScale;
MObject SceneShapeInterface::aScaleX;
MObject SceneShapeInterface::aScaleY;
MObject SceneShapeInterface::aScaleZ;
MObject SceneShapeInterface::aBound;
MObject SceneShapeInterface::aBoundMin;
MObject SceneShapeInterface::aBoundMinX;
MObject SceneShapeInterface::aBoundMinY;
MObject SceneShapeInterface::aBoundMinZ;
MObject SceneShapeInterface::aBoundMax;
MObject SceneShapeInterface::aBoundMaxX;
MObject SceneShapeInterface::aBoundMaxY;
MObject SceneShapeInterface::aBoundMaxZ;
MObject SceneShapeInterface::aBoundCenter;
MObject SceneShapeInterface::aBoundCenterX;
MObject SceneShapeInterface::aBoundCenterY;
MObject SceneShapeInterface::aBoundCenterZ;

// This post load callback is used to dirty the aOutputObjects elements
// following loading - see further  comments in initialize.
class SceneShapeInterface::PostLoadCallback : public IECoreMaya::PostLoadCallback
{

	public :
	
		PostLoadCallback( SceneShapeInterface *node ) : m_node( node )
		{
		}
	
	protected :
		
		SceneShapeInterface *m_node;
		
		virtual void postLoad()
		{
			MFnDependencyNode fnDN( m_node->thisMObject() );
			MPlug plug = fnDN.findPlug( aObjectDependency );
			plug.setValue( 1 );
			
			m_node->m_postLoadCallback = 0; // remove this callback
		}
		
};

SceneShapeInterface::SceneShapeInterface()
	: m_previewSceneDirty( true )
{
	// We only create the post load callback when Maya is reading a scene,
	// so that it does not effect nodes as they are created by users.
	m_postLoadCallback = ( MFileIO::isReadingFile() ) ? new PostLoadCallback( this ) : NULL;
}

SceneShapeInterface::~SceneShapeInterface()
{
	
}

void *SceneShapeInterface::creator()
{
	return new SceneShapeInterface;
}

void SceneShapeInterface::postConstructor()
{
	setExistWithoutInConnections(true);
	setExistWithoutOutConnections(true);
}

MStatus SceneShapeInterface::initialize()
{
	MFnNumericAttribute nAttr;
	MFnTypedAttribute tAttr;
	MFnCompoundAttribute cAttr;
	MFnGenericAttribute gAttr;
	MFnUnitAttribute uAttr;
	MFnEnumAttribute eAttr;
	
	MStatus s;
	
	aObjectOnly = nAttr.create( "objectOnly", "obj", MFnNumericData::kBoolean, 0 );
	nAttr.setReadable( true );
	nAttr.setWritable( true );
	nAttr.setStorable( true );
	nAttr.setConnectable( true );
	nAttr.setHidden( false );

	addAttribute( aObjectOnly );
	
	aDrawGeometry = nAttr.create( "drawGeometry", "drg", MFnNumericData::kBoolean, 0, &s );
	nAttr.setReadable( true );
	nAttr.setWritable( true );
	nAttr.setStorable( true );
	nAttr.setConnectable( true );
	nAttr.setHidden( false );
	nAttr.setChannelBox( true );

	s = addAttribute( aDrawGeometry );

	aDrawRootBound = nAttr.create( "drawRootBound", "drbd", MFnNumericData::kBoolean, 1, &s );
	nAttr.setReadable( true );
	nAttr.setWritable( true );
	nAttr.setStorable( true );
	nAttr.setConnectable( true );
	nAttr.setHidden( false );
	nAttr.setChannelBox( true );

	s = addAttribute( aDrawRootBound );
	
	aDrawChildBounds = nAttr.create( "drawChildBounds", "dchd", MFnNumericData::kBoolean, 0, &s );
	nAttr.setReadable( true );
	nAttr.setWritable( true );
	nAttr.setStorable( true );
	nAttr.setConnectable( true );
	nAttr.setHidden( false );
	nAttr.setChannelBox( true );

	s = addAttribute( aDrawChildBounds );

	aDrawTagsFilter = tAttr.create( "drawTagsFilter", "dtf", MFnData::kString, MFnStringData().create( "" ), &s );
	assert( s );
	s = addAttribute( aDrawTagsFilter );
	assert( s );

	aQuerySpace = eAttr.create( "querySpace", "qsp", 0);
	eAttr.addField( "World", World );
	eAttr.addField( "Local", Local );
	
	s = addAttribute( aQuerySpace );
    
	aTime = uAttr.create( "time", "tim", MFnUnitAttribute::kTime, 0.0, &s );
	uAttr.setConnectable( true );
	uAttr.setHidden( false );
	uAttr.setReadable( true );
	uAttr.setWritable( true );
	uAttr.setStorable( true );
	
	s = addAttribute( aTime );

	aOutTime = uAttr.create( "outTime", "otm", MFnUnitAttribute::kTime, 0.0, &s );
	uAttr.setReadable( true );
	uAttr.setWritable( false );
	uAttr.setStorable( false );
	
	s = addAttribute( aOutTime );
	
	// Queries
	
	aSceneQueries = tAttr.create( "queryPaths", "qpa", MFnData::kString, &s );
	tAttr.setReadable( true );
	tAttr.setWritable( true );
	tAttr.setStorable( true );
	tAttr.setConnectable( true );
	tAttr.setHidden( false );
	tAttr.setArray( true );
	tAttr.setIndexMatters( true );

	s = addAttribute( aSceneQueries );
	
	aAttributeQueries = tAttr.create( "queryAttributes", "qat", MFnData::kString, &s );
	tAttr.setReadable( true );
	tAttr.setWritable( true );
	tAttr.setStorable( true );
	tAttr.setConnectable( true );
	tAttr.setHidden( false );
	tAttr.setArray( true );
	tAttr.setIndexMatters( true );

	s = addAttribute( aAttributeQueries );

	// Output objects
	aOutputObjects = gAttr.create( "outObjects", "oob", &s );
	gAttr.addDataAccept( MFnMeshData::kMesh );
	gAttr.addDataAccept( MFnNurbsCurveData::kNurbsCurve );
	gAttr.addNumericDataAccept( MFnNumericData::k3Double );
	gAttr.setReadable( true );
	gAttr.setWritable( false );
	gAttr.setArray( true );
	gAttr.setIndexMatters( true );
	gAttr.setUsesArrayDataBuilder( true );
	gAttr.setStorable( false );
	s = addAttribute( aOutputObjects );
	
	// A Maya bug causes mesh attributes to compute on scene open, even when nothing should be
	// pulling on them. While dynamic attributes can work around this issue for single meshes,
	// arrays of meshes suffer from the erroneous compute as either static or dynamic attributes.
	//
	// We work around the problem by adding a PostLoadCallback if the node was created while the
	// scene is being read from disk. This callback is used to short circuit the compute which
	// was triggered on scene open. Since the compute may have actually been necessary, we use
	// this dummy dependency attribute to dirty the output object attributes immediately following
	// load. At this point the callback deletes itself, and the newly dirtied output objects will
	// re-compute if something was actually pulling on them.
	aObjectDependency = nAttr.create( "objectDependency", "objDep", MFnNumericData::kInt, 0 );
	nAttr.setStorable( false );
	nAttr.setHidden( true );
	s = addAttribute( aObjectDependency );
	
	// Transform

	aTranslateX = nAttr.create( "outTranslateX", "otx", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
		
	aTranslateY = nAttr.create( "outTranslateY", "oty", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aTranslateZ = nAttr.create( "outTranslateZ", "otz", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );	
		
	aTranslate = nAttr.create( "outTranslate", "obt", aTranslateX, aTranslateY, aTranslateZ, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aRotateX = uAttr.create( "outRotateX", "orx", MFnUnitAttribute::kAngle, 0.0, &s );
	uAttr.setWritable( false );
	uAttr.setStorable( false );
		
	aRotateY = uAttr.create( "outRotateY", "ory", MFnUnitAttribute::kAngle, 0.0, &s );
	uAttr.setWritable( false );
	uAttr.setStorable( false );
	
	aRotateZ = uAttr.create( "outRotateZ", "orz", MFnUnitAttribute::kAngle, 0.0, &s );
	uAttr.setWritable( false );
	uAttr.setStorable( false );	
		
	aRotate = nAttr.create( "outRotate", "obr", aRotateX, aRotateY, aRotateZ, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aScaleX = nAttr.create( "outScaleX", "osx", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
		
	aScaleY = nAttr.create( "outScaleY", "osy", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aScaleZ = nAttr.create( "outScaleZ", "osz", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );	
		
	aScale = nAttr.create( "outScale", "obs", aScaleX, aScaleY, aScaleZ, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aTransform = cAttr.create( "outTransform", "otr" );
	cAttr.addChild( aTranslate );
	cAttr.addChild( aRotate );
	cAttr.addChild( aScale );
	cAttr.setReadable( true );
	cAttr.setWritable( false );
	cAttr.setArray( true );
	cAttr.setIndexMatters( true );
	cAttr.setUsesArrayDataBuilder( true );
	cAttr.setStorable( false );

	s = addAttribute( aTransform );
	
	// Bounding box
	
	aBoundMinX = nAttr.create( "outBoundMinX", "obminx", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
		
	aBoundMinY = nAttr.create( "outBoundMinY", "cobminy", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundMinZ = nAttr.create( "outBoundMinZ", "obminz", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundMin = nAttr.create( "outBoundMin", "obmin", aBoundMinX, aBoundMinY, aBoundMinZ, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundMaxX = nAttr.create( "outBoundMaxX", "obmaxx", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
		
	aBoundMaxY = nAttr.create( "outBoundMaxY", "cobmaxy", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundMaxZ = nAttr.create( "outBoundMaxZ", "obmaxz", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundMax = nAttr.create( "outBoundMax", "obmax", aBoundMaxX, aBoundMaxY, aBoundMaxZ, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundCenterX = nAttr.create( "outBoundCenterX", "obcx", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
		
	aBoundCenterY = nAttr.create( "outBoundCenterY", "obcy", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundCenterZ = nAttr.create( "outBoundCenterZ", "obcz", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundCenter = nAttr.create( "outBoundCenter", "obc", aBoundCenterX, aBoundCenterY, aBoundCenterZ, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBound = cAttr.create( "outBound", "obd" );
	cAttr.addChild( aBoundMin );
	cAttr.addChild( aBoundMax );
	cAttr.addChild( aBoundCenter );
	cAttr.setReadable( true );
	cAttr.setWritable( false );
	cAttr.setArray( true );
	cAttr.setIndexMatters( true );
	cAttr.setUsesArrayDataBuilder( true );
	cAttr.setStorable( false );
	
	s = addAttribute( aBound );
	assert( s );
	
	// Attributes
	
	MFnGenericAttribute genAttr;
	aAttributeValues = genAttr.create( "attributeValues", "atv", &s );
	genAttr.addNumericDataAccept( MFnNumericData::kBoolean );
	genAttr.addNumericDataAccept( MFnNumericData::kInt );
	genAttr.addNumericDataAccept( MFnNumericData::kFloat );
	genAttr.addDataAccept( MFnData::kString );
	genAttr.setReadable( true );
	genAttr.setWritable( false );
	genAttr.setStorable( false );
	genAttr.setConnectable( true );
	genAttr.setArray( true );
	genAttr.setIndexMatters( true );
	genAttr.setUsesArrayDataBuilder( true );

	addAttribute( aAttributeValues );

	aAttributes = cAttr.create( "attributes", "ott" );
	cAttr.addChild( aAttributeValues );
	cAttr.setReadable( true );
	cAttr.setWritable( false );
	cAttr.setArray( true );
	cAttr.setIndexMatters( true );
	cAttr.setUsesArrayDataBuilder( true );
	cAttr.setReadable( true );
	cAttr.setStorable( false );
	
	s = addAttribute( aAttributes );
	
	attributeAffects( aSceneQueries, aTransform );
	attributeAffects( aSceneQueries, aBound );
	attributeAffects( aSceneQueries, aOutputObjects );
	attributeAffects( aSceneQueries, aAttributes );
	
	attributeAffects( aAttributeQueries, aAttributes );
	
	attributeAffects( aQuerySpace, aTransform );
	attributeAffects( aQuerySpace, aBound );
	attributeAffects( aQuerySpace, aOutputObjects );
	
	attributeAffects( aTime, aOutTime );

	return s;
}

ConstSceneInterfacePtr SceneShapeInterface::getSceneInterface( )
{
	throw Exception( "SceneShapeInterface: getSceneInterface not implemented!" );
}

bool SceneShapeInterface::isBounded() const
{
	return true;
}

double SceneShapeInterface::time() const
{
	MPlug pTime( thisMObject(), aTime );
	MTime time;
	pTime.getValue( time );
	return time.as( MTime::kSeconds );
}

MBoundingBox SceneShapeInterface::boundingBox() const
{
	MBoundingBox bound( MPoint( -1, -1, -1 ), MPoint( 1, 1, 1 ) );
	
	ConstSceneInterfacePtr scn = const_cast<SceneShapeInterface*>(this)->getSceneInterface();
	
	if( scn )
	{
		try
		{
			MPlug pTime( thisMObject(), aTime );
			MTime time;
			pTime.getValue( time );
			
			// Check what bounding box we need. If objectOnly we want the objectBound, else the scene bound.
			// If objectOnly and no object, return the scene bound to have something sensible. It won't be drawn.
			
			MPlug pObjectOnly( thisMObject(), aObjectOnly );
			bool objectOnly;
			pObjectOnly.getValue( objectOnly );
			
			bool objectBound = objectOnly;
			if( objectOnly )
			{
				// Check for an object
				if( scn->hasObject() )
				{
					ConstObjectPtr object = scn->readObject( time.as( MTime::kSeconds ) );
					const VisibleRenderable *obj = runTimeCast< const VisibleRenderable >( object.get() );
					if( obj )
					{
						Box3f objBox = obj->bound();
						if( !objBox.isEmpty() )
						{
							bound = convert<MBoundingBox>( objBox );
						}
					}
				}
				else
				{
					objectBound = false;
				}
				
			}
			
			if( !objectBound )
			{
				Box3d b = scn->readBound( time.as( MTime::kSeconds ) );
				if( !b.isEmpty() )
				{
					bound = convert<MBoundingBox>( b );
				}
			}
		}
		catch( std::exception &e )
		{
			msg( Msg::Error, "SceneShapeInterface::boundingBox", e.what() );
		}
		catch( ... )
		{
			msg( Msg::Error, "SceneShapeInterface::boundingBox", "Exception thrown in SceneShapeInterface::bound" );
		}
	}

	return bound;
}

MStatus SceneShapeInterface::setDependentsDirty( const MPlug &plug, MPlugArray &plugArray )
{
	if( plug == aTime )
	{
		ConstSampledSceneInterfacePtr sceneInterface = runTimeCast< const SampledSceneInterface >(getSceneInterface());
		
		// Check if sceneInterface is animated. If not, plugs are not dependent on time
		if( !sceneInterface || ( sceneInterface && sceneInterface -> numBoundSamples() > 1 ) )
		{
			m_previewSceneDirty = true;
			childChanged( kBoundingBoxChanged ); // needed to tell maya the bounding box has changed
			
			// Need to go through all output plugs, since Maya doesn't propagate dirtiness from parent plug to the children in setDependentsDirty
			MPlug pObjects( thisMObject(), aOutputObjects );
			for( unsigned i=0; i<pObjects.numElements(); i++ )
			{
				MPlug p = pObjects[i];
				plugArray.append( p );
			}
		
			MPlug pTransform( thisMObject(), aTransform );
			for( unsigned i=0; i<pTransform.numElements(); i++ )
			{
				MPlug p = pTransform[i];
				for( unsigned j=0; j<p.numChildren(); j++ )
				{
					plugArray.append( p.child( j ) );
					plugArray.append( p.child( j ).child( 0 ) );
					plugArray.append( p.child( j ).child( 1 ) );
					plugArray.append( p.child( j ).child( 2 ) );
				}
			}
			
			MPlug pBound( thisMObject(), aBound);
			for( unsigned i=0; i<pBound.numElements(); i++ )
			{
				MPlug p = pBound[i];
				for( unsigned j=0; j<p.numChildren(); j++ )
				{
					plugArray.append( p.child( j ) );
					plugArray.append( p.child( j ).child( 0 ) );
					plugArray.append( p.child( j ).child( 1 ) );
					plugArray.append( p.child( j ).child( 2 ) );
				}
			}
			
			MPlug pAttributes( thisMObject(), aAttributes);
			for( unsigned i=0; i<pAttributes.numElements(); i++ )
			{
				MPlug p = pAttributes[i];
				MPlug pChild = p.child(0);
				plugArray.append( pChild );
				for( unsigned j=0; j<pChild.numElements(); j++ )
				{
					plugArray.append( pChild[j] );
				}
			}
		}
	}
	else if( plug == aDrawGeometry || plug == aDrawChildBounds || plug == aObjectOnly || plug == aDrawTagsFilter )
	{
		// Preview plug values have changed, GL Scene is dirty
		m_previewSceneDirty = true;
	}
	else if ( plug == aObjectDependency )
	{
		MPlug pObjects( thisMObject(), aOutputObjects );
		for( unsigned i=0; i<pObjects.numElements(); i++ )
		{
			MPlug p = pObjects[i];
			plugArray.append( p );
		}
	}
	
	return MS::kSuccess;
}


MStatus SceneShapeInterface::compute( const MPlug &plug, MDataBlock &dataBlock )
{
	MStatus s;
	MPlug topLevelPlug = plug;
	int index = -1;
	// Look for parent plug index
	while( topLevelPlug.isChild() || topLevelPlug.isElement() )
	{
		if( topLevelPlug.isChild() )
		{
			topLevelPlug = topLevelPlug.parent();
		}
		if( topLevelPlug.isElement() )
		{
			index = topLevelPlug.logicalIndex();
			topLevelPlug = topLevelPlug.array();
		}
	}
	
	if ( topLevelPlug == aOutputObjects && m_postLoadCallback )
	{
		return MS::kFailure;
	}
	
	if( topLevelPlug == aOutputObjects || topLevelPlug == aTransform || topLevelPlug == aBound || topLevelPlug == aAttributes )
	{
		if( index == -1 )
		{
			// Couldn't find input index
			MFnDagNode dag(thisMObject());
			msg( Msg::Warning, dag.fullPathName().asChar(),  boost::format( "Could not find queried index for '%s' " ) % plug.name().asChar() );
			return MS::kFailure;
		}
		
		MDataHandle timeHandle = dataBlock.inputValue( aTime );
		MTime time = timeHandle.asTime();

		MPlug pQuerySpace( thisMObject(), aQuerySpace );
		int querySpace;
		pQuerySpace.getValue( querySpace );

		MPlug pSceneQueries( thisMObject(), aSceneQueries );
		MPlug pQuery = pSceneQueries.elementByLogicalIndex( index );
		MString name;
		pQuery.getValue( name );

		ConstSceneInterfacePtr sc = getSceneInterface();
		if( !sc )
		{
			// Scene Interface isn't valid
			MFnDagNode dag(thisMObject());
			msg( Msg::Error, dag.fullPathName().asChar(),  "Input values are invalid." );
			return MS::kFailure;
		}

		SceneInterface::Path path;
		path = fullPathName( name.asChar() );
		// Get sceneInterface for query path
		ConstSceneInterfacePtr scene = sc->scene( path,  SceneInterface::NullIfMissing );
		
		if( !scene )
		{
			// Queried element doesn't exist
			MFnDagNode dag(thisMObject());
			msg( Msg::Warning, dag.fullPathName().asChar(),  boost::format( "Queried element '%s' at index '%s' does not exist " ) % name.asChar() % index );
			return MS::kFailure;
		}
		if( topLevelPlug == aTransform )
		{
			MArrayDataHandle transformHandle = dataBlock.outputArrayValue( aTransform );
			MArrayDataBuilder transformBuilder = transformHandle.builder();
			
			M44d transformd;
			if( querySpace == World )
			{
				// World Space (starting from scene root)
				transformd = worldTransform( scene, time.as( MTime::kSeconds ) );
			}
			else if( querySpace == Local )
			{
				// Local space
				transformd = scene->readTransformAsMatrix( time.as( MTime::kSeconds ) );
			}
			
			V3f translate( 0 ), shear( 0 ), rotate( 0 ), scale( 1 );
			extractSHRT( convert<M44f>( transformd ), scale, shear, rotate, translate );

			MDataHandle transformElementHandle = transformBuilder.addElement( index );
			transformElementHandle.child( aTranslate ).set3Float( translate[0], translate[1], translate[2] );
			transformElementHandle.child( aRotate ).child( aRotateX ).setMAngle( MAngle( rotate[0] ) );
			transformElementHandle.child( aRotate ).child( aRotateY ).setMAngle( MAngle( rotate[1] ) );
			transformElementHandle.child( aRotate ).child( aRotateZ ).setMAngle( MAngle( rotate[2] ) );
			transformElementHandle.child( aScale ).set3Float( scale[0], scale[1], scale[2] );
		}
		else if( topLevelPlug == aOutputObjects && scene->hasObject() )
		{
			MArrayDataHandle outputDataHandle = dataBlock.outputArrayValue( aOutputObjects, &s );
			MArrayDataBuilder outputBuilder = outputDataHandle.builder();

			ConstObjectPtr object = scene->readObject( time.as( MTime::kSeconds ) );

			if( querySpace == World )
			{
				// If world space, need to transform the object using the concatenated matrix from the sceneInterface path to the query path
				M44d transformd;
				transformd = worldTransform( scene, time.as( MTime::kSeconds ) );
				
				TransformOpPtr transformer = new TransformOp();
				transformer->inputParameter()->setValue( const_cast< Object *>(object.get()) );		/// safe const_cast because the op will duplicate the Object.
				transformer->copyParameter()->setTypedValue( true );
				transformer->matrixParameter()->setValue( new M44dData( transformd ) );
				object = transformer->operate();
			}

			IECore::TypeId type = object->typeId();
			if( type == CoordinateSystemTypeId )
			{
				IECore::ConstCoordinateSystemPtr coordSys = IECore::runTimeCast<const CoordinateSystem>( object );
				Imath::M44f m = coordSys->getTransform()->transform();
				Imath::V3f s,h,r,t;
				Imath::extractSHRT(m, s, h, r, t);

				MFnNumericData fnData;
				MObject data = fnData.create( MFnNumericData::k3Double );
				fnData.setData( (double)t[0], (double)t[1], (double)t[2] );

				MDataHandle handle = outputBuilder.addElement( index );
				handle.set( data );
			}
			else
			{
				ToMayaObjectConverterPtr converter = ToMayaObjectConverter::create( object );

				if( converter )
				{
					MObject data;
					// Check the type for now, because a dag node is created if you pass an empty MObject to the converter
					// Won't be needed anymore when the related todo is addressed in the converter
					
					if( type == MeshPrimitiveTypeId )
					{
						MFnMeshData fnData;
						data = fnData.create();
					}
					else if( type == CurvesPrimitiveTypeId )
					{
						MFnNurbsCurveData fnData;
						data = fnData.create();
					}

					if( !data.isNull() )
					{
						bool conversionSuccess = converter->convert( data );
						if ( conversionSuccess )
						{
							MDataHandle h = outputBuilder.addElement( index, &s );
							s = h.set( data );
						}
					}
				}
			}

			

		}
		else if( topLevelPlug == aBound )
		{
			MArrayDataHandle boundHandle = dataBlock.outputArrayValue( aBound );
			MArrayDataBuilder boundBuilder = boundHandle.builder();
			
			Box3d bboxd = scene->readBound( time.as( MTime::kSeconds ) );
			
			if( querySpace == World )
			{
				// World Space (from root path)
				M44d transformd = worldTransform( scene, time.as( MTime::kSeconds ) );
				bboxd = transform( bboxd, transformd );
			}
			Box3f bound( bboxd.min, bboxd.max );

			MDataHandle boundElementHandle = boundBuilder.addElement( index );
			boundElementHandle.child( aBoundMin ).set3Float( bound.min[0],  bound.min[1],  bound.min[2] );
			boundElementHandle.child( aBoundMax ).set3Float( bound.max[0],  bound.max[1],  bound.max[2] );
			V3f boundCenter = bound.center();
			boundElementHandle.child( aBoundCenter ).set3Float( boundCenter[0], boundCenter[1], boundCenter[2] );
		}
		else if( topLevelPlug == aAttributes )
		{

			int attrIndex = -1;
			// Look for attribute query index
			if( plug.isElement() && !plug.isCompound() )
			{
				attrIndex = plug.logicalIndex();
			}
			else
			{
				MFnDagNode dag(thisMObject());
				msg( Msg::Warning, dag.fullPathName().asChar(),  boost::format( "Cannot find index for queried attribute '%s'" ) % plug.name() );
				return MS::kFailure;
			}

			MPlug pAttributeQueries( thisMObject(), aAttributeQueries );
			MPlug pAttributeQuery = pAttributeQueries.elementByLogicalIndex( attrIndex );
			MString attrName;
			pAttributeQuery.getValue( attrName );

			if( !scene->hasAttribute( attrName.asChar() ) )
			{
				// Queried attribute doesn't exist
				MFnDagNode dag(thisMObject());
				msg( Msg::Warning, dag.fullPathName().asChar(),  boost::format( "Queried attribute '%s' at index '%s' does not exist " ) % attrName.asChar() % attrIndex );
				return MS::kFailure;
			}
			
			// Attribute query results are returned as attributes[ sceneQuery index ].attributeValues[ attributeQuery index ]
			
			MArrayDataHandle attributesHandle = dataBlock.outputArrayValue( aAttributes );
			MArrayDataBuilder builder = attributesHandle.builder();
			
			MDataHandle elementHandle = builder.addElement( index );
			
			MDataHandle attributeValuesHandle = elementHandle.child( aAttributeValues );
			
			MArrayDataHandle arrayHandle( attributeValuesHandle );
			arrayHandle.jumpToElement( attrIndex );
			MDataHandle currentElement = arrayHandle.outputValue();
			
			ConstObjectPtr attrValue = scene->readAttribute( attrName.asChar(), time.as( MTime::kSeconds ) );
			IECore::TypeId type = attrValue->typeId();
			if( type == BoolDataTypeId )
			{
				bool value = static_cast< const BoolData * >(attrValue.get())->readable();
				currentElement.setGenericBool( value, true);
			}
			else if( type == FloatDataTypeId )
			{
				float value = static_cast< const FloatData * >(attrValue.get())->readable();
				currentElement.setGenericFloat( value, true);
			}
			else if( type == IntDataTypeId )
			{
				int value = static_cast< const IntData * >(attrValue.get())->readable();
				currentElement.setGenericInt( value, true);
			}
			else if( type == StringDataTypeId )
			{
				MString value( static_cast< const StringData * >(attrValue.get())->readable().c_str() );
				currentElement.setString( value );
			}
		}
	}
	else if( topLevelPlug == aOutTime )
	{
		MDataHandle timeHandle = dataBlock.inputValue( aTime );
		MTime time = timeHandle.asTime();
		
		MDataHandle outTimeHandle = dataBlock.outputValue( aOutTime );
		outTimeHandle.setMTime( time );
	}

	return MS::kSuccess;
}

M44d SceneShapeInterface::worldTransform( ConstSceneInterfacePtr scene, double time )
{
	SceneInterface::Path p;
	scene->path( p );
	
	ConstSceneInterfacePtr tmpScene = getSceneInterface();
	SceneInterface::Path pRoot;
	tmpScene->path( pRoot );
	M44d result;
	
	for ( SceneInterface::Path::const_iterator it = p.begin()+pRoot.size(); tmpScene && it != p.end(); ++it )
	{
		tmpScene = tmpScene->child( *it, SceneInterface::NullIfMissing );
		if ( !tmpScene )
		{
			break;
		}

		result = tmpScene->readTransformAsMatrix( time ) * result;
	}
	
	return result;
}

MPxSurfaceShape::MatchResult SceneShapeInterface::matchComponent( const MSelectionList &item, const MAttributeSpecArray &spec, MSelectionList &list )
{
	if( spec.length() == 1 )
	{
		MAttributeSpec attrSpec = spec[0];
		MStatus s;

		int dim = attrSpec.dimensions();

		if ( (dim > 0) && attrSpec.name() == "f" )
		{
			int numElements = m_nameToGroupMap.size();

			MAttributeIndex attrIndex = attrSpec[0];

			if ( attrIndex.type() != MAttributeIndex::kInteger )
			{
				return MPxSurfaceShape::kMatchInvalidAttributeRange;
			}

			int upper = numElements - 1;
			int lower = 0;
			if ( attrIndex.hasLowerBound() )
			{
				attrIndex.getLower( lower );
			}
			if ( attrIndex.hasUpperBound() )
			{
				attrIndex.getUpper( upper );
			}

			// Check the attribute index range is valid
			if ( (attrIndex.hasRange() && !attrIndex.hasValidRange() ) || (upper >= numElements) || (lower < 0 ) )
			{
				return MPxSurfaceShape::kMatchInvalidAttributeRange;
			}

			MDagPath path;
			item.getDagPath( 0, path );

			MFnSingleIndexedComponent fnComp;
			MObject comp = fnComp.create( MFn::kMeshPolygonComponent, &s );

			for ( int i=lower; i<=upper; i++ )
			{
				fnComp.addElement( i );
			}

			list.add( path, comp );

			return MPxSurfaceShape::kMatchOk;
		}
	}

	return MPxSurfaceShape::matchComponent( item, spec, list );
}

/// Blank implementation of this method. This is to avoid a crash when you try and use the rotation manipulator maya gives
/// you when you've selected procedural components in rotation mode (maya 2013)
void SceneShapeInterface::transformUsing( const MMatrix &mat, const MObjectArray &componentList, MPxSurfaceShape::MVertexCachingMode cachingMode, MPointArray *pointCache )
{
}

/// This method is overridden to supply a geometry iterator, which maya uses to work out
/// the bounding boxes of the components you've selected in the viewport
MPxGeometryIterator* SceneShapeInterface::geometryIteratorSetup( MObjectArray& componentList, MObject& components, bool forReadOnly )
{
	if ( components.isNull() )
	{
		return new SceneShapeInterfaceComponentBoundIterator( this, componentList );
	}
	else
	{
		return new SceneShapeInterfaceComponentBoundIterator( this, components );
	}
}

Imath::Box3d SceneShapeInterface::componentBound( int idx )
{
	// get relative path name from index
	IECore::InternedString name = selectionName( idx );
	
	SceneInterface::Path path;
	path = fullPathName( name.value() );
	ConstSceneInterfacePtr scene = getSceneInterface()->scene( path,  SceneInterface::NullIfMissing );
	
	MPlug pTime( thisMObject(), aTime );
	MTime time;
	pTime.getValue( time );
	
	// read bound from scene interface and return it in world space
	Imath::Box3d componentBound = scene->readBound( time.as( MTime::kSeconds ) );
	M44d transformd = worldTransform( scene, time.as( MTime::kSeconds ) );

	componentBound = transform( componentBound, transformd );

	return componentBound;
}

void SceneShapeInterface::buildScene( IECoreGL::RendererPtr renderer, ConstSceneInterfacePtr subSceneInterface )
{
	// Grab plug values to know what we need to render
	MPlug pTime( thisMObject(), aTime );
	MTime time;
	pTime.getValue( time );
	
	MPlug pDrawBounds( thisMObject(), aDrawChildBounds );
	bool drawBounds;
	pDrawBounds.getValue( drawBounds );
	
	MPlug pDrawGeometry( thisMObject(), aDrawGeometry );
	bool drawGeometry;
	pDrawGeometry.getValue( drawGeometry );
	
	MPlug pObjectOnly( thisMObject(), aObjectOnly );
	bool objectOnly;
	pObjectOnly.getValue( objectOnly );
	
	if( !drawGeometry && !drawBounds )
	{
		return;
	}

	MPlug pDrawTagsFilter( thisMObject(), aDrawTagsFilter );
	MString drawTagsFilter;
	pDrawTagsFilter.getValue( drawTagsFilter );

	typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
	std::string txt(drawTagsFilter.asChar());
	Tokenizer tokens( txt, boost::char_separator<char>(" "));
	Tokenizer::iterator t = tokens.begin();
	SceneInterface::NameList drawTags;
	for ( ; t != tokens.end(); t++ )
	{
		if ( t->size() )
		{
			/// test if the scene location has the tag to start with..
			if ( subSceneInterface->hasTag( *t, SceneInterface::EveryTag ) )
			{
				drawTags.push_back( *t );
			}
			else
			{
				msg( Msg::Warning, name().asChar(), std::string("Tag '") + *t + "'does not exist in scene. Ignoring it for filtering.");
			}
		}
	}

	renderer->concatTransform( convert<M44f>( worldTransform( subSceneInterface, time.as( MTime::kSeconds ) ) ) );

	recurseBuildScene( renderer.get(), subSceneInterface.get(), time.as( MTime::kSeconds ), drawBounds, drawGeometry, objectOnly, drawTags );
}

void SceneShapeInterface::recurseBuildScene( IECoreGL::Renderer * renderer, const SceneInterface *subSceneInterface, double time, bool drawBounds, bool drawGeometry, bool objectOnly, const SceneInterface::NameList &drawTags )
{
	if ( drawTags.size() )
	{
		SceneInterface::NameList::const_iterator it;
		for ( it = drawTags.begin(); it != drawTags.end(); it++ )
		{
			if ( subSceneInterface->hasTag( *it, SceneInterface::EveryTag ) )
			{
				break;
			}
		}
		/// stop drawing if the current scene location does not have the required tags.
		if ( it == drawTags.end() )
		{
			return;
		}
	}
	
	if ( subSceneInterface->hasAttribute( "scene:visible" ) )
	{
		if( ConstBoolDataPtr vis = runTimeCast<const BoolData>( subSceneInterface->readAttribute( "scene:visible", time ) ) )
		{
			if( !vis->readable() )
			{
				return;
			}
		}
	}
	
	AttributeBlock a(renderer);
	SceneInterface::Path pathName;
	subSceneInterface->path( pathName );
	std::string pathStr = relativePathName( pathName );
	renderer->setAttribute( "name", new StringData( pathStr ) );
	renderer->setAttribute( "gl:curvesPrimitive:useGLLines", new BoolData( true ) );
	
	if(pathStr != "/")
	{
		// Path space
		renderer->concatTransform( convert<M44f>( subSceneInterface->readTransformAsMatrix( time ) ) );
	}
	
	// Need to add this attribute block to get a parent group with that name that includes the object and/or bound
	AttributeBlock aNew(renderer);

	if ( subSceneInterface->hasAttribute( LinkedScene::fileNameLinkAttribute ) )
	{
		// we are at a link location... create a hash to uniquely identify it.
		MurmurHash hash;
		subSceneInterface->readAttribute( LinkedScene::fileNameLinkAttribute, time )->hash( hash );
		subSceneInterface->readAttribute( LinkedScene::rootLinkAttribute, time )->hash( hash );
		subSceneInterface->readAttribute( LinkedScene::timeLinkAttribute, time )->hash( hash );
		/// register the hash mapped to the name of the group
		InternedString pathInternedStr(pathStr);
		std::pair< HashToName::iterator, bool > ret = m_hashToName.insert( HashToName::value_type( hash, pathInternedStr ) );
		if ( !ret.second )
		{
			/// the same location was already rendered, so we store the current location for instanting later...
			m_instances.push_back( InstanceInfo(pathInternedStr, ret.first->second) );
			return;
		}
	}


	if( drawGeometry && subSceneInterface->hasObject() )
	{
		ConstObjectPtr object = subSceneInterface->readObject( time );
		const Renderable *o = runTimeCast< const Renderable >(object.get());
		if( o )
		{
			o->render(renderer);
		}
	}

	if( drawBounds && pathStr != "/" )
	{
		Box3d b = subSceneInterface->readBound( time );
		Box3f bbox( b.min, b.max );
		if( !bbox.isEmpty() )
		{
			CurvesPrimitive::createBox( bbox )->render( renderer );    
		}
	}
	
	if( !objectOnly )
	{
		// We need the entire hierarchy, iterate through all the sceneInterface children
		SceneInterface::NameList childNames;
		subSceneInterface->childNames( childNames );
		for ( SceneInterface::NameList::const_iterator it = childNames.begin(); it != childNames.end(); ++it )
		{
			ConstSceneInterfacePtr childScene = subSceneInterface->child( *it );
			recurseBuildScene( renderer, childScene.get(), time, drawBounds, drawGeometry, objectOnly, drawTags );
		}
	}
}

void SceneShapeInterface::createInstances()
{
	for ( InstanceArray::iterator it = m_instances.begin(); it != m_instances.end(); it++ )
	{
		const InternedString &instanceName = it->first;
		const InternedString &instanceSourceName = it->second;

		NameToGroupMap::const_iterator srcIt = m_nameToGroupMap.find( instanceSourceName );

		assert ( srcIt != m_nameToGroupMap.end() );

		const IECoreGL::Group *srcGroup = srcIt->second.second.get();
		
		NameToGroupMap::iterator trgIt = m_nameToGroupMap.find( instanceName );
		IECoreGL::Group *trgGroup = trgIt->second.second.get();

		// copy the src group to the trg group (empty instance group)
		recurseCopyGroup( srcGroup, trgGroup, instanceName.value() );
	}

	/// clear the maps we don't need them.
	m_hashToName.clear();
	m_instances.clear();
}

void SceneShapeInterface::recurseCopyGroup( const IECoreGL::Group *srcGroup, IECoreGL::Group *trgGroup, const std::string &namePrefix )
{
	const IECoreGL::Group::ChildContainer &children = srcGroup->children();

	for ( IECoreGL::Group::ChildContainer::const_iterator it = children.begin(); it != children.end(); ++it )
	{
		IECoreGL::Group *group = runTimeCast< IECoreGL::Group >( *it );

		if ( group )
		{
			IECoreGL::GroupPtr newGroup = new IECoreGL::Group();
			 // copy state, including the name state component
			IECoreGL::StatePtr newState = new IECoreGL::State( *group->getState() );
			newGroup->setState( newState );
			std::string newName;
			const IECoreGL::NameStateComponent *nameState = newState->get< IECoreGL::NameStateComponent >();
			/// now override the name state component
			if ( nameState )
			{
				const std::string &srcName = nameState->name();
				size_t found = srcName.rfind('/');
				if (found!=std::string::npos)
				{
					// we take the current "directory" and prepend it with the namePrefix
					newName = namePrefix;
					newName.append( srcName, found, std::string::npos );
					newState->add( new IECoreGL::NameStateComponent( newName ) );
					// we also need to register the group in the selection maps...
					registerGroup( newName, newGroup );
				}
			}
			newGroup->setTransform( group->getTransform() );
			recurseCopyGroup( group, newGroup, newName.size() ? newName.c_str() : namePrefix );
			trgGroup->addChild( newGroup );
		}
		else
		{
			trgGroup->addChild( *it );
		}
	}

}

IECoreGL::ConstScenePtr SceneShapeInterface::glScene()
{
	if(!m_previewSceneDirty)
	{
		return m_scene;
	}

	ConstSceneInterfacePtr sceneInterface = getSceneInterface();

	if( sceneInterface )
	{
		SceneInterface::NameList childNames;
		sceneInterface->childNames( childNames );

		IECoreGL::RendererPtr renderer = new IECoreGL::Renderer();
		renderer->setOption( "gl:mode", new StringData( "deferred" ) );
		// Always draw locators. They can be hidden by using tags.
		renderer->setOption( "gl:drawCoordinateSystems", new BoolData( true ) );
		
		renderer->worldBegin();
		{
			buildScene( renderer, sceneInterface );
		}
		renderer->worldEnd();
	
		m_scene = renderer->scene();
		m_scene->setCamera( 0 );
	}

	// Update component name to group map
	m_nameToGroupMap.clear();
	m_indexToNameMap.clear();
	IECoreGL::ConstStatePtr defaultState = IECoreGL::State::defaultState();
	buildGroups( defaultState->get<const IECoreGL::NameStateComponent>(), m_scene->root() );
	createInstances();

	m_previewSceneDirty = false;

	return m_scene;
}

void SceneShapeInterface::registerGroup( const std::string &name, IECoreGL::GroupPtr &group )
{
		int index = m_nameToGroupMap.size();
		std::pair< NameToGroupMap::iterator, bool> ret;
		ret = m_nameToGroupMap.insert( std::pair< InternedString, NameToGroupMap::mapped_type > (name,  NameToGroupMap::mapped_type( index, group )) );
		if( ret.second )
		{
			m_indexToNameMap.push_back( name );
		}
}

void SceneShapeInterface::buildGroups( IECoreGL::ConstNameStateComponentPtr nameState, IECoreGL::GroupPtr group )
{
	assert( nameState );
	assert( group );
	assert( group->getState() );

	if (  group->getState()->get< IECoreGL::NameStateComponent >() )
	{
		nameState = group->getState()->get< IECoreGL::NameStateComponent >();
	}

	const std::string &name = nameState->name();
	if( name != "unnamed" )
	{
		registerGroup( name, group );
	}

	const IECoreGL::Group::ChildContainer &children = group->children();
	for ( IECoreGL::Group::ChildContainer::const_iterator it = children.begin(); it != children.end(); ++it )
	{
		assert( *it );
		group = runTimeCast< IECoreGL::Group >( *it );
		if ( group )
		{
			buildGroups( nameState, group );
		}
	}
}

void SceneShapeInterface::setDirty()
{
	m_previewSceneDirty = true;
}

IECoreGL::GroupPtr SceneShapeInterface::glGroup( const IECore::InternedString &name )
{
	NameToGroupMap::const_iterator elementIt = m_nameToGroupMap.find( name );
	if( elementIt != m_nameToGroupMap.end() )
	{
		return elementIt->second.second;
	}
	else
	{
		return 0;
	}
}

int SceneShapeInterface::selectionIndex( const IECore::InternedString &name )
{
	NameToGroupMap::const_iterator elementIt = m_nameToGroupMap.find( name );
	if( elementIt != m_nameToGroupMap.end() )
	{
		return elementIt->second.first;
	}
	else
	{
		return -1;
	}
}

IECore::InternedString SceneShapeInterface::selectionName( int index )
{
	return m_indexToNameMap[index];
}

const std::vector< InternedString > & SceneShapeInterface::componentNames() const
{
	return m_indexToNameMap;
}

std::string SceneShapeInterface::relativePathName( SceneInterface::Path path )
{
	SceneInterface::Path root;
	getSceneInterface()->path( root );
	assert( root );
	assert( root.size() <= path.size() );
	
	if( root == path )
	{
		return "/";
	}
	
	std::string pathName;
	
	SceneInterface::Path::const_iterator it = path.begin();
	it += root.size();

	for ( ; it != path.end(); it++ )
	{
		pathName += '/';
		pathName += it->value();
	}
	
	return pathName;
}

SceneInterface::Path SceneShapeInterface::fullPathName( std::string relativeName )
{
	SceneInterface::Path root;
	getSceneInterface()->path( root );
	assert( root );

	SceneInterface::Path relativePath;
	SceneInterface::stringToPath( relativeName, relativePath );

	SceneInterface::Path fullPath( root );
	fullPath.insert( fullPath.end(), relativePath.begin(), relativePath.end() );

	return fullPath;
}

