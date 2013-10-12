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

#include "IECore/MessageHandler.h"

#include "IECoreGL/Scene.h"
#include "IECoreGL/State.h"
#include "IECoreGL/StateComponent.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/NameStateComponent.h"
#include "IECoreGL/BoxPrimitive.h"
#include "IECoreGL/Exception.h"
#include "IECoreGL/Group.h"

#include "IECoreMaya/ProceduralHolderUI.h"
#include "IECoreMaya/ProceduralHolder.h"
#include "IECoreMaya/Convert.h"

using namespace IECoreMaya;
using namespace std;

ProceduralHolderUI::ProceduralHolderUI()
{
	IECoreGL::init( true );
}

ProceduralHolderUI::~ProceduralHolderUI()
{
}

void *ProceduralHolderUI::creator()
{
	return new ProceduralHolderUI;
}

/// \todo We should be firing off a separate drawRequest for the components, then we can be done with the "hiliteGroups" mechanism.
void ProceduralHolderUI::getDrawRequests( const MDrawInfo &info, bool objectAndActiveOnly, MDrawRequestQueue &requests )
{
	// it's easy if noone want to look at us
	if( !info.objectDisplayStatus( M3dView::kDisplayMeshes ) )
	{
		return;
	}

	// the node we're meant to be drawing
	ProceduralHolder *proceduralHolder = (ProceduralHolder *)surfaceShape();

	// draw data encapsulating that node
	MDrawData drawData;
	getDrawData( proceduralHolder, drawData );

	// a request for the bound if necessary
	MPlug pDrawBound( proceduralHolder->thisMObject(), ProceduralHolder::aDrawBound );
	bool drawBound = true;
	pDrawBound.getValue( drawBound );

	if( drawBound )
	{
		MDrawRequest request = info.getPrototype( *this );
		request.setDrawData( drawData );
		request.setToken( BoundDrawMode );
		request.setDisplayStyle( M3dView::kWireFrame );
		setWireFrameColors( request, info.displayStatus() );
		requests.add( request );
	}

	// requests for the scene if necessary
	MPlug pGLPreview( proceduralHolder->thisMObject(), ProceduralHolder::aGLPreview );
	bool glPreview = false;
	pGLPreview.getValue( glPreview );
	if( glPreview )
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
				IECore::msg( IECore::Msg::Warning, "ProceduralHolderUI::getDrawRequests", boost::format( "Failed to evaluate material for \"%s\"." ) % pathName.asChar() );
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
			if( !transparent )
			{
				MPlug pT( proceduralHolder->thisMObject(), ProceduralHolder::aTransparent );
				bool transparent = false;
				pT.getValue( transparent );
			}
			solidRequest.setIsTransparent( transparent );
			solidRequest.setToken( SceneDrawMode );
			requests.add( solidRequest );
			// and add another request for wireframe drawing if we're selected
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

					if ( proceduralHolder->hasActiveComponents() )
					{
						MObjectArray components = proceduralHolder->activeComponents();
		    				MObject component = components[0]; // Should filter list
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

				if ( proceduralHolder->hasActiveComponents() )
				{
					MObjectArray components = proceduralHolder->activeComponents();
		    			MObject component = components[0]; // Should filter list
		   			request.setComponent( component );
				}
			}

			requests.add( request );
		}
	}
}

void ProceduralHolderUI::setWireFrameColors( MDrawRequest &request, M3dView::DisplayStatus status )
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

void ProceduralHolderUI::draw( const MDrawRequest &request, M3dView &view ) const
{
	MStatus s;
	MDrawData drawData = request.drawData();
	ProceduralHolder *proceduralHolder = (ProceduralHolder *)drawData.geometry();
	assert( proceduralHolder );
	
	view.beginGL();
	
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
			IECoreGL::BoxPrimitive::renderWireframe( IECore::convert<Imath::Box3f>( proceduralHolder->boundingBox() ) );
		}

		// draw the scene if asked
		if( request.token()==SceneDrawMode )
		{
			resetHilites();

			IECoreGL::ConstScenePtr scene = proceduralHolder->scene();
			if( scene )
			{
				IECoreGL::State *displayState = m_displayStyle.baseState( (M3dView::DisplayStyle)request.displayStyle() );

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
					for ( int j = 0; j < len; j++ )
					{
						int compId = fnComp.element(j);

						assert( proceduralHolder->m_componentToGroupMap.find( compId ) != proceduralHolder->m_componentToGroupMap.end() );

						hiliteGroups(
							proceduralHolder->m_componentToGroupMap[compId],
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
		IECore::msg( IECore::Msg::Error, "ProceduralHolderUI::draw", boost::format( "IECoreGL Exception : %s" ) % e.what() );
	}

	if( restoreLightState )
	{
		restoreLights( &lightingState );	
	}
	
	view.endGL();
}

bool ProceduralHolderUI::select( MSelectInfo &selectInfo, MSelectionList &selectionList, MPointArray &worldSpaceSelectPts ) const
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
	ProceduralHolder *proceduralHolder = static_cast<ProceduralHolder *>( surfaceShape() );
	IECoreGL::ConstScenePtr scene = proceduralHolder->scene();
	if( !scene )
	{
		return false;
	}
	
	// we want to perform the selection using an IECoreGL::Selector, so we
	// can avoid the performance penalty associated with using GL_SELECT mode.
	// that means we don't really want to call view.beginSelect(), but we have to
	// call it just to get the projection matrix for our own selection, because as far
	// as i can tell, there is no other way of getting it reliably.
	
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
				// we're not in component selection mode. we'd like to be able to select the procedural
				// object using the bounding box so we draw it too.
				MPlug pDrawBound( proceduralHolder->thisMObject(), ProceduralHolder::aDrawBound );
				bool drawBound = true;
				pDrawBound.getValue( drawBound );
				if( drawBound )
				{
					IECoreGL::BoxPrimitive::renderWireframe( IECore::convert<Imath::Box3f>( proceduralHolder->boundingBox() ) );
				}
			}
		}
						
	view.endGL();
	
	if( !hits.size() )
	{
		return false;
	}

	// iterate over the hits, converting them into components and also finding
	// the closest one.
	MIntArray componentIndices;
	float depthMin = std::numeric_limits<float>::max();
	int depthMinIndex = -1;
	for( int i=0, e = hits.size(); i < e; i++ )
	{		
		if( hits[i].depthMin < depthMin )
		{
			depthMin = hits[i].depthMin;
			depthMinIndex = componentIndices.length();
		}
		
		ProceduralHolder::ComponentsMap::const_iterator compIt = proceduralHolder->m_componentsMap.find( hits[i].name.value() );
		assert( compIt != proceduralHolder->m_componentsMap.end() );
		componentIndices.append( compIt->second.first );		
	}
	
	assert( depthMinIndex >= 0 );

	// figure out the world space location of the closest hit
	
	MDagPath camera;
	view.getCamera( camera );
	MFnCamera fnCamera( camera.node() );
	float nearf = fnCamera.nearClippingPlane();
	float farf = fnCamera.farClippingPlane();
	
	float z = -1;
	if( fnCamera.isOrtho() )
	{
		z = Imath::lerp( nearf, farf, depthMin );
	}
	else
	{
		// perspective camera - depth isn't linear so linearise to get z
		float a = farf / ( farf - nearf );
		float b = farf * nearf / ( nearf - farf );
		z = b / ( depthMin - a );
	}	
	
	MPoint localRayOrigin;
	MVector localRayDirection;
	selectInfo.getLocalRay( localRayOrigin, localRayDirection );
	MMatrix localToCamera = selectInfo.selectPath().inclusiveMatrix() * camera.inclusiveMatrix().inverse();	
	MPoint cameraRayOrigin = localRayOrigin * localToCamera;
	MVector cameraRayDirection = localRayDirection * localToCamera;
	
	MPoint cameraIntersectionPoint = cameraRayOrigin + cameraRayDirection * ( -( z - nearf ) / cameraRayDirection.z );
	MPoint worldIntersectionPoint = cameraIntersectionPoint * camera.inclusiveMatrix();
	
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
		
		selectInfo.addSelection(
			items, worldIntersectionPoint,
			selectionList, worldSpaceSelectPts,
			MSelectionMask::kSelectMeshFaces,
			true
		);		
	}
	else
	{
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

void ProceduralHolderUI::hiliteGroups( const ProceduralHolder::ComponentToGroupMap::mapped_type &groups, IECoreGL::StateComponentPtr hilite, IECoreGL::StateComponentPtr base ) const
{
	assert( base );
	for( ProceduralHolder::ComponentToGroupMap::mapped_type::const_iterator it = groups.begin(); it != groups.end(); ++it )
	{
		unhiliteGroupChildren( it->first, it->second, base );
	}

	for( ProceduralHolder::ComponentToGroupMap::mapped_type::const_iterator it = groups.begin(); it != groups.end(); ++it )
	{
		IECoreGL::GroupPtr group = it->second;
		assert( group );

		if ( m_stateMap.find( group.get() ) == m_stateMap.end() )
		{
			IECoreGL::StatePtr oldState = new IECoreGL::State( *(group->getState()) );
			assert( oldState );
			m_stateMap[ group.get() ] = oldState;
		}

		group->getState()->add( hilite );
	}
}

void ProceduralHolderUI::unhiliteGroupChildren( const std::string &name, IECoreGL::GroupPtr group, IECoreGL::StateComponentPtr base ) const
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
		
		// don't bother if the group's already been explicitly highlighted
		if( !group->getState()->get( IECoreGL::WireframeColorStateComponent::staticTypeId() ) )
		{
			group->getState()->add( base );
		}
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

void ProceduralHolderUI::resetHilites() const
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
bool ProceduralHolderUI::cleanupLights( const MDrawRequest &request, M3dView &view, LightingState *s ) const
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

void ProceduralHolderUI::restoreLights( LightingState *s ) const
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

