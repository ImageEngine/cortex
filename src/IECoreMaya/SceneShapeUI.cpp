//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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
#include <limits>

#include "IECoreGL/GL.h" // must come first so glew.h is included before gl.h

#include "maya/MTypes.h"
#include "maya/MGlobal.h"
#include "maya/MDrawData.h"
#include "maya/MDagPath.h"
#include "maya/MSelectionList.h"
#include "maya/MSelectionMask.h"
#include "maya/MMaterial.h"
#include "maya/MFnSingleIndexedComponent.h"
#include "maya/MObjectArray.h"
#include "maya/MIntArray.h"
#include "maya/MPointArray.h"
#include "maya/MDoubleArray.h"
#include "maya/MFnCamera.h"
#include "maya/MFnStringArrayData.h"
#include "maya/MStringArray.h"

#include "IECore/MessageHandler.h"
#include "IECore/MeshPrimitive.h"
#include "IECore/RunTimeTyped.h"
#include "IECore/MeshPrimitiveEvaluator.h"

#include "IECoreGL/Scene.h"
#include "IECoreGL/State.h"
#include "IECoreGL/StateComponent.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/NameStateComponent.h"
#include "IECoreGL/BoxPrimitive.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/Group.h"

#include "IECoreMaya/SceneShapeUI.h"
#include "IECoreMaya/SceneShape.h"
#include "IECoreMaya/Convert.h"

#include <limits>

using namespace IECoreMaya;
using namespace std;

SceneShapeUI::SceneShapeUI()
{
	IECoreGL::init( true );
}

SceneShapeUI::~SceneShapeUI()
{
}

void *SceneShapeUI::creator()
{
	return new SceneShapeUI;
}


void SceneShapeUI::getDrawRequests( const MDrawInfo &info, bool objectAndActiveOnly, MDrawRequestQueue &requests )
{
	// it's easy if no one want to look at us
	if( !info.objectDisplayStatus( M3dView::kDisplayMeshes ) )
	{
		return;
	}
	// the node we're meant to be drawing
	SceneShape *sceneShape = (SceneShape *)surfaceShape();

	if( !sceneShape->getSceneInterface() )
	{
		return;
	}
	
	// draw data encapsulating that node
	MDrawData drawData;
	getDrawData( sceneShape, drawData );

	// a request for the bound if necessary
	MPlug pDrawBound( sceneShape->thisMObject(), SceneShape::aDrawRootBound );
	bool drawBound;
	pDrawBound.getValue( drawBound );
	
	if( drawBound )
	{
		bool doDrawBound = true;
		
		// If objectOnly is true, check for an object. If none found, no need to add the bound request.
		MPlug pObjectOnly( sceneShape->thisMObject(), SceneShape::aObjectOnly );
		bool objectOnly;
		pObjectOnly.getValue( objectOnly );
		if( objectOnly && !sceneShape->getSceneInterface()->hasObject() )
		{
			doDrawBound = false;
		}
		
		if( doDrawBound )
		{
			MDrawRequest request = info.getPrototype( *this );
			request.setDrawData( drawData );
			request.setToken( BoundDrawMode );
			request.setDisplayStyle( M3dView::kWireFrame );
			setWireFrameColors( request, info.displayStatus() );
			requests.add( request );
		}

	}

	MPlug pDrawAllBounds( sceneShape->thisMObject(), SceneShape::aDrawChildBounds );
	bool drawAllBounds = false;
	pDrawAllBounds.getValue( drawAllBounds );
	
	// requests for the scene if necessary
	MPlug pGLPreview( sceneShape->thisMObject(), SceneShape::aDrawGeometry );
	bool glPreview;
	pGLPreview.getValue( glPreview );
	
	if( glPreview || drawAllBounds )
	{
		if( info.displayStyle()==M3dView::kGouraudShaded || info.displayStyle()==M3dView::kFlatShaded )
		{
			// make a request for solid drawing with a material
			MDrawRequest solidRequest = info.getPrototype( *this );
			solidRequest.setDrawData( drawData );

			MDagPath path = info.multiPath();
			M3dView view = info.view();
			MMaterial material = MPxSurfaceShapeUI::material( path );
			if( !material.evaluateMaterial( view, path ) )
			{
				MString pathName = path.fullPathName();
				IECore::msg( IECore::Msg::Warning, "SceneShapeUI::getDrawRequests", boost::format( "Failed to evaluate material for \"%s\"." ) % pathName.asChar() );
			}
			if( material.materialIsTextured() )
			{
				material.evaluateTexture( drawData );
			}
			solidRequest.setMaterial( material );
			// set the transparency request. we don't have a decent way of finding out
			// if shaders applied by the procedural are transparent, so we've got a transparency
			// attribute on the procedural holder for users to use. maya materials may also say
			// they're transparent. if either asks for transparency then we'll ask for it here
			bool transparent = false;
			material.getHasTransparency( transparent );
			solidRequest.setIsTransparent( transparent );
			solidRequest.setToken( SceneDrawMode );
			requests.add( solidRequest );

			if( info.displayStatus()==M3dView::kActive || info.displayStatus()==M3dView::kLead || info.displayStatus()==M3dView::kHilite )
			{
				MDrawRequest wireRequest = info.getPrototype( *this );
				wireRequest.setDrawData( drawData );
				wireRequest.setDisplayStyle( M3dView::kWireFrame );
				wireRequest.setToken( SceneDrawMode );
				setWireFrameColors( wireRequest, info.displayStatus() );
				wireRequest.setComponent( MObject::kNullObj );

				if ( !objectAndActiveOnly )
				{
					if ( sceneShape->hasActiveComponents() )
					{
						MObjectArray components = sceneShape->activeComponents();
						MObject component = components[0];
						wireRequest.setComponent( component );
					}
				}

				requests.add( wireRequest );
			}
			
		}
		else
		{
			MDrawRequest request = info.getPrototype( *this );
			request.setDrawData( drawData );
			setWireFrameColors( request, info.displayStatus() );
			request.setToken( SceneDrawMode );

			request.setComponent( MObject::kNullObj );

			if ( !objectAndActiveOnly )
			{
				if ( sceneShape->hasActiveComponents() )
				{
					MObjectArray components = sceneShape->activeComponents();
		    			MObject component = components[0];
		   			request.setComponent( component );
				}
			}
			requests.add( request );
		}
	}
	
	
}

void SceneShapeUI::setWireFrameColors( MDrawRequest &request, M3dView::DisplayStatus status )
{
	// yes, this does use magic numbers for the color indices, and
	// no, i don't know how to get them properly. the quadricShape api
	// example is just doing this too.
	switch( status )
	{
		case M3dView::kLead :
			request.setColor( 18, M3dView::kActiveColors );
			break;
		case M3dView::kActive :
			request.setColor( 15, M3dView::kActiveColors );
			break;
		case M3dView::kActiveAffected :
			request.setColor( 8, M3dView::kActiveColors );
			break;
		case M3dView::kHilite :
			request.setColor( 17, M3dView::kActiveColors );
			break;
		case M3dView::kTemplate :
			request.setColor( 2, M3dView::kDormantColors );
			break;
		case M3dView::kActiveTemplate :
			request.setColor( 19, M3dView::kActiveColors );
			break;
		default :
			// dormant
			request.setColor( 4, M3dView::kDormantColors );
			break;
	}
}

void SceneShapeUI::draw( const MDrawRequest &request, M3dView &view ) const
{
	MStatus s;
	MDrawData drawData = request.drawData();
	SceneShape *sceneShape = (SceneShape *)drawData.geometry();
	assert( sceneShape );

	view.beginGL();
	
	M3dView::LightingMode lightingMode;
	view.getLightingMode( lightingMode );
	
	LightingState lightingState;
	bool restoreLightState = cleanupLights( request, view, &lightingState );
	
	// maya can sometimes leave an error from it's own code,
	// and we don't want that to confuse us in our drawing code.
	while( glGetError()!=GL_NO_ERROR )
	{
	}

	try
	{
		// draw the bound if asked
		if( request.token()==BoundDrawMode )
		{
			IECoreGL::BoxPrimitive::renderWireframe( IECore::convert<Imath::Box3f>( sceneShape->boundingBox() ) );
		}
		
		// draw the scene if asked
		if( request.token()==SceneDrawMode )
		{
			resetHilites();
			
			IECoreGL::ConstScenePtr scene = sceneShape->glScene();
			if( scene )
			{
				IECoreGL::State *displayState = m_displayStyle.baseState( (M3dView::DisplayStyle)request.displayStyle(), lightingMode );
				
				if ( request.component() != MObject::kNullObj )
				{
					MDoubleArray col;
					s = MGlobal::executeCommand( "colorIndex -q 21", col );
					assert( s );
					IECoreGL::WireframeColorStateComponentPtr hilite = new IECoreGL::WireframeColorStateComponent( Imath::Color4f( col[0], col[1], col[2], 1.0f ) );

					MFnSingleIndexedComponent fnComp( request.component(), &s );
					assert( s );
					
					int len = fnComp.elementCount( &s );
					assert( s );
					std::vector<IECore::InternedString> groupNames;
					for ( int j = 0; j < len; j++ )
					{
						int index = fnComp.element(j);
						groupNames.push_back( sceneShape->selectionName( index ) );
					}
					// Sort by name to make sure we don't unhilite selected items that are further down the hierarchy
					std::sort( groupNames.begin(), groupNames.end() );
					
					for ( std::vector<IECore::InternedString>::iterator it = groupNames.begin(); it!= groupNames.end(); ++it)
					{
						IECoreGL::GroupPtr group = sceneShape->glGroup( *it );

						hiliteGroups(
								group,
								hilite,
								const_cast<IECoreGL::WireframeColorStateComponent *>( displayState->get< IECoreGL::WireframeColorStateComponent >() )
							);
					}
				}
				
				scene->render( displayState );
			}
		}
	}
	catch( const IECoreGL::Exception &e )
	{
		// much better to catch and report this than to let the application die
		IECore::msg( IECore::Msg::Error, "SceneShapeUI::draw", boost::format( "IECoreGL Exception : %s" ) % e.what() );
	}
	
	if( restoreLightState )
	{
		restoreLights( &lightingState );	
	}
	
	view.endGL();
}

#if MAYA_API_VERSION >= 201300

bool SceneShapeUI::snap( MSelectInfo &snapInfo ) const
{
	MStatus s;

	if( snapInfo.displayStatus() != M3dView::kHilite )
	{
		MSelectionMask meshMask( MSelectionMask::kSelectMeshes );
		if( !snapInfo.selectable( meshMask ) )
		{
			return false;
		}
	}

	// early out if we have no scene to draw
	SceneShape *sceneShape = static_cast<SceneShape *>( surfaceShape() );
	const IECore::SceneInterface *sceneInterface = sceneShape->getSceneInterface().get();
	if( !sceneInterface )
	{
		return false;
	}

	IECoreGL::ConstScenePtr scene = sceneShape->glScene();
	if( !scene )
	{
		return false;
	}

	// Get the viewport that the snapping operation is taking place in.
	M3dView view = snapInfo.view();

	// Use an IECoreGL::Selector to find the point in world space that we wish to snap to.
	// We do this by first getting the origin of the selection ray and transforming it into
	// NDC space using the OpenGL projection and transformation matrices. Once we have the
	// point in NDC we can use it to define the viewport that the IECoreGL::Selector will use.

	MPoint localRayOrigin;
	MVector localRayDirection;
	snapInfo.getLocalRay( localRayOrigin, localRayDirection );
	
	Imath::V3d org( localRayOrigin[0], localRayOrigin[1], localRayOrigin[2] );
	MDagPath camera;
	view.getCamera( camera );
	MMatrix localToCamera = snapInfo.selectPath().inclusiveMatrix() * camera.inclusiveMatrix().inverse();
	
	view.beginSelect();
		Imath::M44d projectionMatrix;
		glGetDoublev( GL_PROJECTION_MATRIX, projectionMatrix.getValue() );
	view.endSelect();

	double v[4][4];
	localToCamera.get( v ); 
	Imath::M44d cam( v );
	Imath::V3d ndcPt3d = ( (org * cam ) * projectionMatrix + Imath::V3d( 1. ) ) * Imath::V3d( .5 );
	Imath::V2d ndcPt( std::max( std::min( ndcPt3d[0], 1. ), 0. ), 1. - std::max( std::min( ndcPt3d[1], 1. ), 0. ) );

	view.beginGL();
	
		glMatrixMode( GL_PROJECTION );
		glLoadMatrixd( projectionMatrix.getValue() );
		
		float radius = .001; // The radius of the selection area in NDC.
		double aspect = double( view.portWidth() ) / view.portHeight();
		Imath::V2f selectionWH( radius, radius * aspect );
		
		std::vector<IECoreGL::HitRecord> hits;
		{
			IECoreGL::Selector selector( Imath::Box2f( ndcPt - selectionWH, ndcPt + selectionWH ), IECoreGL::Selector::IDRender, hits );
				
			IECoreGL::State::bindBaseState();
			selector.baseState()->bind();
			scene->render( selector.baseState() );			
		}
				
	view.endGL();

	if( hits.empty() )
	{
		return false;
	}

	// Get the closest mesh hit.	
	float depthMin = std::numeric_limits<float>::max();
	int depthMinIndex = -1;
	for( unsigned int i=0, e = hits.size(); i < e; i++ )
	{		
		if( hits[i].depthMin < depthMin )
		{
			depthMin = hits[i].depthMin;
			depthMinIndex = i;
		}
	}

	// Get the absolute path of the hit object.
	IECore::SceneInterface::Path objPath;
	std::string objPathStr;
	sceneInterface->path( objPath );
	IECore::SceneInterface::pathToString( objPath, objPathStr );
	
	objPathStr += IECoreGL::NameStateComponent::nameFromGLName( hits[depthMinIndex].name );
	IECore::SceneInterface::stringToPath( objPathStr, objPath );

	// Validate the hit selection.
	IECore::ConstSceneInterfacePtr childInterface;
	try
	{
		childInterface = sceneInterface->scene( objPath );
	}
	catch(...)
	{
		return false;
	}

	if( !childInterface )
	{
		return false;
	}
	
	if( !childInterface->hasObject() )
	{
		return false;
	}

	// Get the mesh primitive so that we can query it's vertices.
	double time = sceneShape->time();
	IECore::ConstObjectPtr object = childInterface->readObject( time );
	IECore::ConstMeshPrimitivePtr meshPtr = IECore::runTimeCast<const IECore::MeshPrimitive>( object.get() );
	
	if ( !meshPtr )
	{
		return false;
	}
	
	// Calculate the snap point in object space.
	MPoint worldIntersectionPoint;
	selectionRayToWorldSpacePoint( camera, snapInfo, depthMin, worldIntersectionPoint );
	Imath::V3f pt( worldIntersectionPoint[0], worldIntersectionPoint[1], worldIntersectionPoint[2] );
	Imath::M44f objToWorld( worldTransform( childInterface.get(), time ) );
	pt = pt * objToWorld.inverse();

	// Get the list of vertices in the mesh.
	IECore::V3fVectorData::ConstPtr pointData( meshPtr->variableData<IECore::V3fVectorData>( "P", IECore::PrimitiveVariable::Vertex ) ); 
	const std::vector<Imath::V3f> &vertices( pointData->readable() ); 
	
	// Find the vertex that is closest to the snap point.
	Imath::V3d closestVertex;
	float closestDistance = std::numeric_limits<float>::max(); 
	
	for( std::vector<Imath::V3f>::const_iterator it( vertices.begin() ); it != vertices.end(); ++it )
	{
		Imath::V3d vert( *it );
		float d( ( pt - vert ).length() ); // Calculate the distance between the vertex and the snap point.
		if( d < closestDistance )
		{
			closestDistance = d;
			closestVertex = vert;
		}
	}

	// Snap to the vertex.
	closestVertex *= objToWorld;
	snapInfo.setSnapPoint( MPoint( closestVertex[0], closestVertex[1], closestVertex[2] ) );
	return true;
}

#endif

void SceneShapeUI::selectionRayToWorldSpacePoint( const MDagPath &camera, const MSelectInfo &selectInfo, float depth, MPoint &worldIntersectionPoint ) const
{
	MPoint localRayOrigin;
	MVector localRayDirection;
	selectInfo.getLocalRay( localRayOrigin, localRayDirection );

	MFnCamera fnCamera( camera.node() );
	float near = fnCamera.nearClippingPlane();
	float far = fnCamera.farClippingPlane();
	float z = -1;

	if( fnCamera.isOrtho() )
	{
		z = Imath::lerp( near, far, depth );
	}
	else
	{
		// perspective camera - depth isn't linear so linearise to get z
		float a = far / ( far - near );
		float b = far * near / ( near - far );
		z = b / ( depth - a );
	}	

	MMatrix localToCamera = selectInfo.selectPath().inclusiveMatrix() * camera.inclusiveMatrix().inverse();	
	MPoint cameraRayOrigin = localRayOrigin * localToCamera;
	MVector cameraRayDirection = localRayDirection * localToCamera;

	MPoint cameraIntersectionPoint = cameraRayOrigin + cameraRayDirection * ( -( z - near ) / cameraRayDirection.z );
	worldIntersectionPoint = cameraIntersectionPoint * camera.inclusiveMatrix();
}

bool SceneShapeUI::select( MSelectInfo &selectInfo, MSelectionList &selectionList, MPointArray &worldSpaceSelectPts ) const
{
	MStatus s;

	// early out if we're not selectable. we always allow components to be selected if we're highlighted,
	// but we don't allow ourselves to be selected as a whole unless meshes are in the selection mask.
	// it's not ideal that we act like a mesh, but it's at least consistent with the drawing mask we use.
	if( selectInfo.displayStatus() != M3dView::kHilite )
	{
		MSelectionMask meshMask( MSelectionMask::kSelectMeshes );
		if( !selectInfo.selectable( meshMask ) )
		{
			return false;
		}
	}

	// early out if we have no scene to draw
	SceneShape *sceneShape = static_cast<SceneShape *>( surfaceShape() );
	if( !sceneShape->getSceneInterface() )
	{
		return false;
	}

	IECoreGL::ConstScenePtr scene = sceneShape->glScene();
	if( !scene )
	{
		return false;
	}

	// we want to perform the selection using an IECoreGL::Selector, so we
	// can avoid the performance penalty associated with using GL_SELECT mode.
	// that means we don't really want to call view.beginSelect(), but we have to
	// call it just to get the projection matrix for our own selection, because as far
	// as I can tell, there is no other way of getting it reliably.

	M3dView view = selectInfo.view();
	view.beginSelect();
		Imath::M44d projectionMatrix;
		glGetDoublev( GL_PROJECTION_MATRIX, projectionMatrix.getValue() );
	view.endSelect();
		
	view.beginGL();
	
		glMatrixMode( GL_PROJECTION );
		glLoadMatrixd( projectionMatrix.getValue() );
		
		IECoreGL::Selector::Mode selectionMode = IECoreGL::Selector::IDRender;
		if( selectInfo.displayStatus() == M3dView::kHilite && !selectInfo.singleSelection() )
		{
			selectionMode = IECoreGL::Selector::OcclusionQuery;
		}

		std::vector<IECoreGL::HitRecord> hits;
		{
			IECoreGL::Selector selector( Imath::Box2f( Imath::V2f( 0 ), Imath::V2f( 1 ) ), selectionMode, hits );
				
			IECoreGL::State::bindBaseState();
			selector.baseState()->bind();
			scene->render( selector.baseState() );

			if( selectInfo.displayStatus() != M3dView::kHilite )
			{
				// We're not in component selection mode. We'd like to be able to select the scene shape
				// using the bounding box so we draw it too.
				IECoreGL::BoxPrimitive::renderWireframe( IECore::convert<Imath::Box3f>( sceneShape->boundingBox() ) );
			}
		}
						
	view.endGL();
	
	if( hits.empty() )
	{
		return false;
	}
	
	// iterate over the hits, converting them into components and also finding
	// the closest one.
	MIntArray componentIndices;
	
	float depthMin = std::numeric_limits<float>::max();
	int depthMinIndex = -1;
	for( unsigned int i=0, e = hits.size(); i < e; i++ )
	{		
		if( hits[i].depthMin < depthMin )
		{
			depthMin = hits[i].depthMin;
			depthMinIndex = componentIndices.length();
		}
		int index = sceneShape->selectionIndex( IECoreGL::NameStateComponent::nameFromGLName( hits[i].name ) );
		componentIndices.append( index );
	}
	
	assert( depthMinIndex >= 0 );

	// figure out the world space location of the closest hit	
	MDagPath camera;
	view.getCamera( camera );
	
	MPoint worldIntersectionPoint;
	selectionRayToWorldSpacePoint( camera, selectInfo, depthMin, worldIntersectionPoint );

	// turn the processed hits into appropriate changes to the current selection
	if( selectInfo.displayStatus() == M3dView::kHilite )
	{
		// selecting components
		MFnSingleIndexedComponent fnComponent;
		MObject component = fnComponent.create( MFn::kMeshPolygonComponent, &s ); assert( s );
	
		if( selectInfo.singleSelection() )
		{
			fnComponent.addElement( componentIndices[depthMinIndex] );
		}
		else
		{
			fnComponent.addElements( componentIndices );
		}
		
		MSelectionList items;
		items.add( selectInfo.multiPath(), component );
		
		MDagPath path = selectInfo.multiPath();

		selectInfo.addSelection(
			items, worldIntersectionPoint,
			selectionList, worldSpaceSelectPts,
			MSelectionMask::kSelectMeshFaces,
			true
		);
		
	}
	else
	{
		// Check if we should be able to select that object
		MPlug pObjectOnly( sceneShape->thisMObject(), SceneShape::aObjectOnly );
		bool objectOnly;
		pObjectOnly.getValue( objectOnly );
		if( objectOnly && !sceneShape->getSceneInterface()->hasObject() )
		{
			return true;
		}
		
		// selecting objects
		MSelectionList item;
		item.add( selectInfo.selectPath() );

		selectInfo.addSelection(
			item, worldIntersectionPoint,
			selectionList, worldSpaceSelectPts,
			MSelectionMask::kSelectMeshes,
			false
		);
	}
	
	return true;
}

Imath::M44d SceneShapeUI::worldTransform( const IECore::SceneInterface *scene, double time ) const
{
	IECore::SceneInterface::Path p;
	scene->path( p );

	IECore::ConstSceneInterfacePtr tmpScene = scene->scene( IECore::SceneInterface::rootPath );
	Imath::M44d result;

	for ( IECore::SceneInterface::Path::const_iterator it = p.begin(); tmpScene && it != p.end(); ++it )
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

void SceneShapeUI::unhiliteGroupChildren( const std::string &name, IECoreGL::GroupPtr group, IECoreGL::StateComponentPtr base ) const
{
	assert( base );
	assert( group );

	/// Add state so that the group hilite state doesn't propogate down the hierarchy past the given name
	IECoreGL::ConstNameStateComponentPtr n = group->getState()->get< IECoreGL::NameStateComponent >();
	if ( n && n->name() != name )
	{
		if ( m_stateMap.find( group.get() ) == m_stateMap.end() )
		{
			IECoreGL::StatePtr oldState = new IECoreGL::State( *(group->getState()) );
			assert( oldState );
			m_stateMap[ group.get() ] = oldState;
		}

		group->getState()->add( base );
		return;
	}

	const IECoreGL::Group::ChildContainer &children = group->children();
	for ( IECoreGL::Group::ChildContainer::const_iterator it = children.begin(); it != children.end(); ++it )
	{
		assert( *it );

		IECoreGL::GroupPtr childGroup = IECore::runTimeCast< IECoreGL::Group >( *it );
		if ( childGroup )
		{
			unhiliteGroupChildren( name, childGroup, base );
		}
	}
}

void SceneShapeUI::hiliteGroups( IECoreGL::GroupPtr group, IECoreGL::StateComponentPtr hilite, IECoreGL::StateComponentPtr base ) const
{
	assert( base );
	
	if( group )
	{
		IECoreGL::ConstNameStateComponentPtr n = group->getState()->get< IECoreGL::NameStateComponent >();
		if( n )
		{
			const std::string &name = n->name();
			unhiliteGroupChildren( name, group, base );
		}
		
		if ( m_stateMap.find( group.get() ) == m_stateMap.end() )
		{
			IECoreGL::StatePtr oldState = new IECoreGL::State( *(group->getState()) );
			assert( oldState );
			m_stateMap[ group.get() ] = oldState;
		}
	
		group->getState()->add( hilite );
	}
}

void SceneShapeUI::resetHilites() const
{
	for ( StateMap::iterator it = m_stateMap.begin(); it != m_stateMap.end(); ++it )
	{
		it->first->setState( it->second );
	}

	m_stateMap.clear();
}


// Currently, Maya leaves lights in GL when you reduce the number of active lights in 
// your scene. It fills the GL light space from 0 with the visible lights, so, we simply 
// need to reset the potentially 'old' state of lights after the last one we know to be 
// visible. We'll put it all back as we found it though. For the moment, this assumes 
// Maya is filling GL consecutively, if they stop doing that, we'll need to get the 
// actual light indexes from the view. Its just a bit quicker to assume this, whilst we can.
bool SceneShapeUI::cleanupLights( const MDrawRequest &request, M3dView &view, LightingState *s ) const
{
		
	if( !(request.displayStyle()==M3dView::kFlatShaded || request.displayStyle()==M3dView::kGouraudShaded) )
	{
		return false;
	}
	
	M3dView::LightingMode mode;
	view.getLightingMode(mode);
	
	if (mode == M3dView::kLightDefault)
	{
		s->numMayaLights = 1;
	}
	else
	{
		view.getLightCount( s->numMayaLights );
	}
	
	int sGlMaxLights = 0;
	glGetIntegerv( GL_MAX_LIGHTS, &sGlMaxLights );
	s->numGlLights = sGlMaxLights;	

	if( s->numMayaLights >= s->numGlLights || s->numGlLights == 0 )
	{
		return false;
	}		
	
	unsigned int vectorSize = s->numGlLights - s->numMayaLights;
	
	s->diffuses.resize( vectorSize );
	s->specs.resize( vectorSize );
	s->ambients.resize( vectorSize );

	static float s_defaultColor[] = { 0.0, 0.0, 0.0, 1.0 };
	
	GLenum light;
	unsigned int j = 0;
	
	for( unsigned int i = s->numMayaLights; i < s->numGlLights; i++ )
	{		
		light = GL_LIGHT0 + i;
		
		glGetLightfv( light, GL_DIFFUSE, s->diffuses[j].getValue() );
		glLightfv( light, GL_DIFFUSE, s_defaultColor );
			
		glGetLightfv( light, GL_SPECULAR, s->specs[j].getValue() );
		glLightfv( light, GL_SPECULAR, s_defaultColor );
			
		glGetLightfv( light, GL_AMBIENT, s->ambients[j].getValue() );
		glLightfv( light, GL_AMBIENT, s_defaultColor );
		
		j++;
	}
	
	return true;
}

void SceneShapeUI::restoreLights( LightingState *s ) const
{
	GLenum light;
	unsigned int j = 0;
	
	for( unsigned int i = s->numMayaLights; i < s->numGlLights; i++ )
	{	
		light = GL_LIGHT0 + i;
		
		glLightfv( light, GL_DIFFUSE, s->diffuses[j].getValue() );
		glLightfv( light, GL_SPECULAR, s->specs[j].getValue() );
		glLightfv( light, GL_AMBIENT, s->ambients[j].getValue() );
		
		j++;
	}
}
