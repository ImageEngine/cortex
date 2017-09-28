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

#include "maya/MDrawData.h"
#include "maya/MDagPath.h"
#include "maya/MSelectionList.h"
#include "maya/MSelectionMask.h"
#include "maya/MIntArray.h"
#include "maya/MPointArray.h"
#include "maya/MFnCamera.h"
#include "maya/MMatrix.h"

#include "IECore/MessageHandler.h"

#include "IECoreGL/Scene.h"
#include "IECoreGL/State.h"

#include "IECoreMaya/DrawableHolderUI.h"
#include "IECoreMaya/DrawableHolder.h"

using namespace IECoreMaya;
using namespace std;

DrawableHolderUI::DrawableHolderUI()
{
	IECoreGL::init( true );
}

DrawableHolderUI::~DrawableHolderUI()
{
}

void *DrawableHolderUI::creator()
{
	return new DrawableHolderUI;
}

void DrawableHolderUI::getDrawRequests( const MDrawInfo &info, bool objectAndActiveOnly, MDrawRequestQueue &requests )
{
	// it's easy if noone want to look at us
	if( !info.objectDisplayStatus( M3dView::kDisplayLocators ) )
	{
		return;
	}

	// the node we're meant to be drawing
	DrawableHolder *drawableHolder = dynamic_cast<DrawableHolder *>( surfaceShape() );
	if( !drawableHolder )
	{
		return;
	}

	// do we actually want to draw it?
	MPlug pDraw( drawableHolder->thisMObject(), DrawableHolder::aDraw );
	bool draw = true;
	pDraw.getValue( draw );

	if( !draw )
	{
		return;
	}

	// draw data encapsulating that node
	MDrawData drawData;
	getDrawData( drawableHolder, drawData );

	MDrawRequest request = info.getPrototype( *this );
	request.setDrawData( drawData );

	// set correct drawing colour:
	switch( info.displayStatus() )
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

	requests.add( request );
}

void DrawableHolderUI::draw( const MDrawRequest &request, M3dView &view ) const
{
	MDrawData drawData = request.drawData();
	DrawableHolder *drawableHolder = (DrawableHolder *)drawData.geometry();
	assert( drawableHolder );

	IECoreGL::ConstScenePtr s = drawableHolder->scene();
	if( !s )
	{
		return;
	}

	view.beginGL();

		// maya can sometimes leave an error from it's own code,
		// and we don't want that to confuse us in our drawing code.
		while( glGetError()!=GL_NO_ERROR )
		{
		}

		// if we're being drawn as part of a selection operation we need
		// to make sure there's a name on the name stack, as the IECoreGL::NameStateComponent
		// expects to be able to load a name into it (it fails with an invalid operation if
		// there's no name slot to load into).
		if( view.selectMode() )
		{
			view.pushName( 0 );
		}

		try
		{
			// do the main render
			s->render( m_displayStyle.baseState( request.displayStyle() ) );

			// do a wireframe render over the top if we're selected and we just did a solid
			// draw.
			bool selected = request.displayStatus()==M3dView::kActive || request.displayStatus()==M3dView::kLead;
			bool solid = request.displayStyle()==M3dView::kFlatShaded || request.displayStyle()==M3dView::kGouraudShaded;
			if( selected && solid )
			{
				s->render( m_displayStyle.baseState( M3dView::kWireFrame ) );
			}
		}
		catch( std::exception &e )
		{
			IECore::msg( IECore::Msg::Error, "DrawableHolderUI::draw", e.what() );
		}

	view.endGL();
}

bool DrawableHolderUI::select( MSelectInfo &selectInfo, MSelectionList &selectionList, MPointArray &worldSpaceSelectPts ) const
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
	DrawableHolder *drawableHolder = static_cast<DrawableHolder *>( surfaceShape() );
	IECoreGL::ConstScenePtr scene = drawableHolder->scene();
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
		}

	view.endGL();

	if( !hits.size() )
	{
		return false;
	}

	// find the depth of the closest hit:
	MIntArray componentIndices;
	float depthMin = std::numeric_limits<float>::max();
	for( int i=0, e = hits.size(); i < e; i++ )
	{
		if( hits[i].depthMin < depthMin )
		{
			depthMin = hits[i].depthMin;
		}
	}


	// figure out the world space location of the closest hit

	MDagPath camera;
	view.getCamera( camera );
	MFnCamera fnCamera( camera.node() );
	float near = fnCamera.nearClippingPlane();
	float far = fnCamera.farClippingPlane();

	float z = -1;
	if( fnCamera.isOrtho() )
	{
		z = Imath::lerp( near, far, depthMin );
	}
	else
	{
		// perspective camera - depth isn't linear so linearise to get z
		float a = far / ( far - near );
		float b = far * near / ( near - far );
		z = b / ( depthMin - a );
	}

	MPoint localRayOrigin;
	MVector localRayDirection;
	selectInfo.getLocalRay( localRayOrigin, localRayDirection );
	MMatrix localToCamera = selectInfo.selectPath().inclusiveMatrix() * camera.inclusiveMatrix().inverse();
	MPoint cameraRayOrigin = localRayOrigin * localToCamera;
	MVector cameraRayDirection = localRayDirection * localToCamera;

	MPoint cameraIntersectionPoint = cameraRayOrigin + cameraRayDirection * ( -( z - near ) / cameraRayDirection.z );
	MPoint worldIntersectionPoint = cameraIntersectionPoint * camera.inclusiveMatrix();

	MSelectionList item;
	item.add( selectInfo.selectPath() );

	selectInfo.addSelection(
		item, worldIntersectionPoint,
		selectionList, worldSpaceSelectPts,
		MSelectionMask::kSelectMeshes,
		false
	);

	return true;
}

