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

#include "IECoreMaya/SceneShapeInterface.h"

#include <boost/python.hpp>

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
#include "IECore/SceneCache.h"
#include "IECore/CurvesPrimitive.h"
#include "IECore/TransformOp.h"

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


using namespace Imath;
using namespace IECore;
using namespace IECoreMaya;

MTypeId SceneShapeInterface::id = SceneShapeInterfaceId;
MObject SceneShapeInterface::aObjectOnly;
MObject SceneShapeInterface::aDrawGeometry;
MObject SceneShapeInterface::aDrawRootBound;
MObject SceneShapeInterface::aDrawChildBounds;
MObject SceneShapeInterface::aQuerySpace;
MObject SceneShapeInterface::aTime;
MObject SceneShapeInterface::aSceneQueries;
MObject SceneShapeInterface::aOutputObjects;
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


SceneShapeInterface::SceneShapeInterface()
	: m_previewSceneDirty( true )
{
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

	s = addAttribute( aDrawGeometry );

	aDrawRootBound = nAttr.create( "drawRootBound", "drbd", MFnNumericData::kBoolean, 1, &s );
	nAttr.setReadable( true );
	nAttr.setWritable( true );
	nAttr.setStorable( true );
	nAttr.setConnectable( true );
	nAttr.setHidden( false );

	s = addAttribute( aDrawRootBound );
	
	aDrawChildBounds = nAttr.create( "drawChildBounds", "dchd", MFnNumericData::kBoolean, 0, &s );
	nAttr.setReadable( true );
	nAttr.setWritable( true );
	nAttr.setStorable( true );
	nAttr.setConnectable( true );
	nAttr.setHidden( false );

	s = addAttribute( aDrawChildBounds );
	
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

	// Queries
	
	aSceneQueries = tAttr.create( "sceneQueries", "sqy", MFnData::kString, &s );
	tAttr.setReadable( true );
	tAttr.setWritable( true );
	tAttr.setStorable( true );
	tAttr.setConnectable( true );
	tAttr.setHidden( false );
	tAttr.setArray( true );
	tAttr.setIndexMatters( true );

	s = addAttribute( aSceneQueries );

	// Output objects

	aOutputObjects = gAttr.create( "outputObjects", "oob", &s );
	gAttr.addDataAccept( MFnMeshData::kMesh );
	gAttr.addDataAccept( MFnNurbsCurveData::kNurbsCurve );
	gAttr.setReadable( true );
	gAttr.setWritable( false );
	gAttr.setStorable( false );
	gAttr.setConnectable( true );
	gAttr.setHidden( true );
	gAttr.setArray( true );
	gAttr.setIndexMatters( true );
	gAttr.setUsesArrayDataBuilder( true );

	s = addAttribute( aOutputObjects );

	// Transform

	aTranslateX = nAttr.create( "objectTranslateX", "otx", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
		
	aTranslateY = nAttr.create( "objectTranslateY", "oty", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aTranslateZ = nAttr.create( "objectTranslateZ", "otz", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );	
		
	aTranslate = nAttr.create( "objectTranslate", "obt", aTranslateX, aTranslateY, aTranslateZ, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aRotateX = nAttr.create( "objectRotateX", "orx", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
		
	aRotateY = nAttr.create( "objectRotateY", "ory", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aRotateZ = nAttr.create( "objectRotateZ", "orz", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );	
		
	aRotate = nAttr.create( "objectRotate", "obr", aRotateX, aRotateY, aRotateZ, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aScaleX = nAttr.create( "objectScaleX", "osx", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
		
	aScaleY = nAttr.create( "objectScaleY", "osy", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aScaleZ = nAttr.create( "objectScaleZ", "osz", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );	
		
	aScale = nAttr.create( "objectScale", "obs", aScaleX, aScaleY, aScaleZ, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );

	aTransform = cAttr.create( "objectTransform", "otr" );
	cAttr.addChild( aTranslate );
	cAttr.addChild( aRotate );
	cAttr.addChild( aScale );
	cAttr.setArray( true );
	cAttr.setIndexMatters( true );
	cAttr.setUsesArrayDataBuilder( true );
	
	s = addAttribute( aTransform );
	
	// Bounding box
	
	aBoundMinX = nAttr.create( "objectBoundMinX", "obminx", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
		
	aBoundMinY = nAttr.create( "objectBoundMinY", "cobminy", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundMinZ = nAttr.create( "objectBoundMinZ", "obminz", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundMin = nAttr.create( "objectBoundMin", "obmin", aBoundMinX, aBoundMinY, aBoundMinZ, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundMaxX = nAttr.create( "objectBoundMaxX", "obmaxx", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
		
	aBoundMaxY = nAttr.create( "objectBoundMaxY", "cobmaxy", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundMaxZ = nAttr.create( "objectBoundMaxZ", "obmaxz", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundMax = nAttr.create( "objectBoundMax", "obmax", aBoundMaxX, aBoundMaxY, aBoundMaxZ, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundCenterX = nAttr.create( "objectBoundCenterX", "obcx", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
		
	aBoundCenterY = nAttr.create( "objectBoundCenterY", "obcy", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundCenterZ = nAttr.create( "objectBoundCenterZ", "obcz", MFnNumericData::kFloat, 0, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBoundCenter = nAttr.create( "objectBoundCenter", "obc", aBoundCenterX, aBoundCenterY, aBoundCenterZ, &s );
	nAttr.setWritable( false );
	nAttr.setStorable( false );
	
	aBound = cAttr.create( "objectBound", "obb" );
	cAttr.addChild( aBoundMin );
	cAttr.addChild( aBoundMax );
	cAttr.addChild( aBoundCenter );
	cAttr.setArray( true );
	cAttr.setIndexMatters( true );
	cAttr.setUsesArrayDataBuilder( true );
	
	s = addAttribute( aBound );
	assert( s );
	
	attributeAffects( aSceneQueries, aTransform );
	attributeAffects( aSceneQueries, aBound );
	attributeAffects( aSceneQueries, aOutputObjects );
	
	return s;
}

IECore::SceneInterfacePtr SceneShapeInterface::getSceneInterface( )
{
	throw Exception( "SceneShapeInterface: getSceneInterface not implemented!" );
}

IECore::SceneInterface::Path SceneShapeInterface::getSceneRoot( )
{
	throw Exception( "SceneShapeInterface: getSceneRoot not implemented!" );
}


bool SceneShapeInterface::isBounded() const
{
	return true;
}

MBoundingBox SceneShapeInterface::boundingBox() const
{
	MBoundingBox bound( MPoint( -1, -1, -1 ), MPoint( 1, 1, 1 ) );
	
	SceneInterfacePtr scn = const_cast<SceneShapeInterface*>(this)->getSceneInterface();
	
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
					ObjectPtr object = scn->readObject( time.as( MTime::kSeconds ) );
					VisibleRenderablePtr obj = IECore::runTimeCast< VisibleRenderable >( object );
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
		IECore::SceneCachePtr sceneInterface = runTimeCast< IECore::SceneCache >(getSceneInterface());
		if( sceneInterface && sceneInterface -> numBoundSamples() > 1 )
		{
			m_previewSceneDirty = true;
			childChanged( kBoundingBoxChanged );
			getOutputPlugsArray( plugArray );
		}
	}
	else if( plug == aSceneQueries || plug == aQuerySpace )
	{
		getOutputPlugsArray( plugArray );
	}
	else if( plug == aDrawGeometry || plug == aDrawChildBounds || plug == aObjectOnly )
	{
		m_previewSceneDirty = true;
	}
	
	return MS::kSuccess;
}

void SceneShapeInterface::getOutputPlugsArray( MPlugArray &plugArray )
{
	MPlug pObjects( thisMObject(), aOutputObjects );
	for( unsigned i=0; i<pObjects.numElements(); i++ )
	{
		MPlug p = pObjects[i];
		plugArray.append( p );
	}

	MPlug pQueries( thisMObject(), aSceneQueries );

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
}


MStatus SceneShapeInterface::compute( const MPlug &plug, MDataBlock &dataBlock )
{
	MStatus s;

	MPlug topLevelPlug = plug;
	int index = -1;
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

	if( topLevelPlug == aOutputObjects || topLevelPlug == aTransform || topLevelPlug == aBound )
	{
		if( index == -1 )
		{
			// Couldn't find input index
			msg( Msg::Warning, "[SceneShapeInterface::compute] Could not find queried index for", plug.name().asChar() );
			return MS::kSuccess;
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
		
		SceneInterface::Path root = getSceneRoot();
		std::string rootName;
		SceneInterface::pathToString( root, rootName );
		std::string pathName = rootName+name.asChar();
		
		SceneInterface::Path path;
		SceneInterface::stringToPath( pathName, path );
		ConstSceneInterfacePtr scene = sc->scene( path,  SceneInterface::NullIfMissing );
		
		if( !scene )
		{
			// Queried element doesn't exist
			msg( Msg::Warning, "[SceneShapeInterface::compute] Queried element does not exist", name.asChar() );
			return MS::kSuccess;
		}
		if( topLevelPlug == aTransform )
		{
			MArrayDataHandle transformHandle = dataBlock.outputArrayValue( aTransform );
			MArrayDataBuilder transformBuilder = transformHandle.builder();
			
			Imath::M44d transformd;
			if( querySpace == World )
			{
				// World Space (starting from scene root)
				transformd = worldTransform( scene, getSceneRoot(), time.as( MTime::kSeconds ) );
			}
			else if( querySpace == Local )
			{
				// Local space
				transformd = scene->readTransformAsMatrix( time.as( MTime::kSeconds ) );
			}
			
			Imath::V3f translate( 0 ), shear( 0 ), rotate( 0 ), scale( 1 );
			Imath::M44f transform;
			transform.setValue( transformd );
			Imath::extractSHRT( convert<Imath::M44f>( transform ), scale, shear, rotate, translate );

			rotate = radiansToDegrees( rotate );

			MDataHandle transformElementHandle = transformBuilder.addElement( index );
			transformElementHandle.child( aTranslate ).set3Float( translate[0], translate[1], translate[2] );
			transformElementHandle.child( aRotate ).set3Float( rotate[0], rotate[1], rotate[2] );
			transformElementHandle.child( aScale ).set3Float( scale[0], scale[1], scale[2] );
		}
		else if( topLevelPlug == aOutputObjects && scene->hasObject() )
		{
			MArrayDataHandle outputDataHandle = dataBlock.outputValue( aOutputObjects, &s );
			MArrayDataBuilder outputBuilder = outputDataHandle.builder();

			IECore::ObjectPtr object = scene->readObject( time.as( MTime::kSeconds ) );
			
			if( querySpace == World )
			{
				Imath::M44d transformd;
				transformd = worldTransform( scene, getSceneRoot(), time.as( MTime::kSeconds ) );
				
				TransformOpPtr transformer = new TransformOp();
				transformer->inputParameter()->setValue( object );
				transformer->copyParameter()->setTypedValue( false );
				transformer->matrixParameter()->setValue( new M44dData( transformd ) );
				object = transformer->operate();
			}

			ToMayaObjectConverterPtr converter = ToMayaObjectConverter::create( object );

			if( converter )
			{
				MObject data;
				// Check the type for now, because a dag node is created if you pass an empty MObject to the converter
				// Won't be needed anymore when the related todo is addressed in the converter
				IECore::TypeId type = object->typeId();
				if( type == IECore::MeshPrimitiveTypeId )
				{
					MFnMeshData fnData;
					data = fnData.create();
				}
				else if( type == IECore::CurvesPrimitiveTypeId )
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
		else if( topLevelPlug == aBound )
		{
			MArrayDataHandle boundHandle = dataBlock.outputArrayValue( aBound );
			MArrayDataBuilder boundBuilder = boundHandle.builder();
			
			Imath::Box3d bboxd = scene->readBound( time.as( MTime::kSeconds ) );
			
			if( querySpace == World )
			{
				// World Space (from root path)
				Imath::M44d transformd = worldTransform( scene, getSceneRoot(), time.as( MTime::kSeconds ) );
				bboxd = transform( bboxd, transformd );
			}
			Imath::Box3f bound( bboxd.min, bboxd.max );

			MDataHandle boundElementHandle = boundBuilder.addElement( index );
			boundElementHandle.child( aBoundMin ).set3Float( bound.min[0],  bound.min[1],  bound.min[2] );
			boundElementHandle.child( aBoundMax ).set3Float( bound.max[0],  bound.max[1],  bound.max[2] );
			Imath::V3f boundCenter = bound.center();
			boundElementHandle.child( aBoundCenter ).set3Float( boundCenter[0], boundCenter[1], boundCenter[2] );
		}
	}

	return MS::kSuccess;
}

Imath::M44d SceneShapeInterface::worldTransform( IECore::ConstSceneInterfacePtr scene, IECore::SceneInterface::Path root, double time )
{
	IECore::SceneInterface::Path p;
	scene->path( p );

	IECore::ConstSceneInterfacePtr tmpScene = scene->scene( root );
	IECore::SceneInterface::Path pRoot;
	tmpScene->path( pRoot );
	Imath::M44d result;
	
	for ( IECore::SceneInterface::Path::const_iterator it = p.begin()+pRoot.size(); tmpScene && it != p.end(); ++it )
	{
		tmpScene = tmpScene->child( *it, IECore::SceneInterface::NullIfMissing );
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

void SceneShapeInterface::buildScene( IECoreGL::RendererPtr renderer, IECore::ConstSceneInterfacePtr subSceneInterface )
{
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

	IECore::AttributeBlock a(renderer);
	IECore::SceneInterface::Path pathName;
	subSceneInterface->path( pathName );
	std::string pathStr = getRelativePathName( pathName );
	renderer->setAttribute( "name", new StringData( pathStr ) );
	
	// Need to add this attribute block to get a parent group with that name that includes the object and/or bound
	IECore::AttributeBlock aNew(renderer);

	Imath::M44d transformd;
	if(pathStr != "/")
	{
		// Path space
		transformd = worldTransform( subSceneInterface, getSceneRoot(), time.as( MTime::kSeconds ) );
	}

	Imath::M44f transform;
	transform.setValue( transformd );
	renderer->setTransform( transform );
	
	if( drawGeometry && subSceneInterface->hasObject() )
	{
		IECore::ObjectPtr object = subSceneInterface->readObject( time.as( MTime::kSeconds ) );
		VisibleRenderablePtr vis = IECore::runTimeCast< VisibleRenderable >(object);
		if( vis )
		{
			vis->render(renderer);
		}
	}
	
	if( drawBounds && pathStr != "/" )
	{
		IECore::AttributeBlock aBox(renderer);
		renderer->setAttribute( "gl:primitive:wireframe", new BoolData( true ) );
		renderer->setAttribute( "gl:primitive:solid", new BoolData( false ) );
		renderer->setAttribute( "gl:curvesPrimitive:useGLLines", new BoolData( true ) );

		Box3d b = subSceneInterface->readBound( time.as( MTime::kSeconds ) );
		Box3f bbox( b.min, b.max );
		if( !bbox.isEmpty() )
		{
			IECore::CurvesPrimitive::createBox( bbox )->render( renderer );    
		}
	}
	
	if( !objectOnly )
	{
		SceneInterface::NameList childNames;
		subSceneInterface->childNames( childNames );
		for ( SceneInterface::NameList::const_iterator it = childNames.begin(); it != childNames.end(); ++it )
		{
			IECore::SceneInterface::Name name = *it;
			IECore::ConstSceneInterfacePtr childScene = subSceneInterface->child( name );
			buildScene( renderer, childScene );
		}
	}
}

IECoreGL::ConstScenePtr SceneShapeInterface::scene()
{
	if(!m_previewSceneDirty)
	{
		return m_scene;
	}

	IECore::ConstSceneInterfacePtr sceneInterface = getSceneInterface();
	m_sceneRoot = getSceneRoot();

	if( sceneInterface )
	{
		SceneInterface::NameList childNames;
		sceneInterface->childNames( childNames );
		
		IECoreGL::RendererPtr renderer = new IECoreGL::Renderer();
		renderer->setOption( "gl:mode", new StringData( "deferred" ) );
		
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

	m_previewSceneDirty = false;

	return m_scene;
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
		int index = m_nameToGroupMap.size();
		std::pair< NameToGroupMap::iterator, bool> ret;
		ret = m_nameToGroupMap.insert( std::pair< IECore::InternedString, NameToGroupMap::mapped_type > (name,  NameToGroupMap::mapped_type( index, group )) );
		if( ret.second )
		{
			IndexToNameMap::iterator idIt = m_indexToNameMap.begin() + index;
			m_indexToNameMap.insert( idIt, name ); 	
		}
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

IECoreGL::GroupPtr SceneShapeInterface::getGroup( std::string name )
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

int SceneShapeInterface::getIndex( std::string name )
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

std::string SceneShapeInterface::getName( int index )
{
	return m_indexToNameMap[index];
}

const std::vector< IECore::InternedString > & SceneShapeInterface::getChildrenNames() const
{
	return m_indexToNameMap;
}

std::string SceneShapeInterface::getRelativePathName( SceneInterface::Path path )
{
	assert( m_sceneRoot );
	assert( m_sceneRoot.size() <= path.size() );
	
	if( m_sceneRoot == path )
	{
		return "/";
	}
	
	std::string pathName;
	
	SceneInterface::Path::const_iterator it = path.begin();
	it += m_sceneRoot.size();

	for ( ; it != path.end(); it++ )
	{
		pathName += '/';
		pathName += it->value();
	}
	
	return pathName;
}


