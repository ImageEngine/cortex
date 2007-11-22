//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/Scene.h"
#include "IECoreGL/State.h"
#include "IECoreGL/TypedStateComponent.h"
#include "IECoreGL/BoxPrimitive.h"
#include "IECoreGL/Exception.h"

#include "IECoreMaya/ProceduralHolderUI.h"
#include "IECoreMaya/ProceduralHolder.h"
#include "IECoreMaya/Convert.h"

#include "IECore/MessageHandler.h"

#include "maya/MDrawData.h"
#include "maya/MDagPath.h"
#include "maya/MSelectionList.h"
#include "maya/MSelectionMask.h"
#include "maya/MMaterial.h"

using namespace IECoreMaya;

using namespace std;
ProceduralHolderUI::ProceduralHolderUI()
	:	m_boxPrimitive( new IECoreGL::BoxPrimitive( Imath::Box3f() ) )
{
}

ProceduralHolderUI::~ProceduralHolderUI()
{
}

void *ProceduralHolderUI::creator()
{
	return new ProceduralHolderUI;
}

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
			if( info.displayStatus()==M3dView::kActive || info.displayStatus()==M3dView::kLead )
			{
				MDrawRequest wireRequest = info.getPrototype( *this );
				wireRequest.setDrawData( drawData );
				wireRequest.setDisplayStyle( M3dView::kWireFrame );
				wireRequest.setToken( SceneDrawMode );
				setWireFrameColors( wireRequest, info.displayStatus() );
				requests.add( wireRequest );
			}
		}
		else
		{
			MDrawRequest request = info.getPrototype( *this );
			request.setDrawData( drawData );
			setWireFrameColors( request, info.displayStatus() );
			request.setToken( SceneDrawMode );
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
	MDrawData drawData = request.drawData();
	ProceduralHolder *proceduralHolder = (ProceduralHolder *)drawData.geometry();
	
	view.beginGL();
	
		// maya can sometimes leave an error from it's own code,
		// and we don't want that to confuse us in our drawing code.
		while( glGetError()!=GL_NO_ERROR )
		{
		}
	
		try
		{
		
			// maya has already set the color based on the request created
			// in getDrawRequests(), but we need to transfer that into the base state
			// we use to draw the scene and the bounding box.
			Imath::Color4f wc; glGetFloatv( GL_CURRENT_COLOR, wc.getValue() );
			switch( request.displayStyle() )
			{
				case M3dView::kWireFrame :
					baseState( M3dView::kWireFrame )->add( new IECoreGL::WireframeColorStateComponent( convert<Imath::Color4f>( wc ) ) );
					break;
				case M3dView::kPoints :
					baseState( M3dView::kPoints )->add( new IECoreGL::PointColorStateComponent( convert<Imath::Color4f>( wc ) ) );
					break;
				case M3dView::kBoundingBox :
					baseState( M3dView::kBoundingBox )->add( new IECoreGL::BoundColorStateComponent( convert<Imath::Color4f>( wc ) ) );
					break;	
				default :
					break;
			}

			// draw the bound if asked
			if( request.token()==BoundDrawMode )
			{
				IECoreGL::StatePtr wireframeState = baseState( M3dView::kWireFrame );
				m_boxPrimitive->setBox( convert<Imath::Box3f>( proceduralHolder->boundingBox() ) );
				glPushAttrib( wireframeState->mask() );
					(boost::static_pointer_cast<IECoreGL::Renderable>( m_boxPrimitive ))->render( wireframeState );
				glPopAttrib();
			}

			// draw the scene if asked
			if( request.token()==SceneDrawMode )
			{
				IECoreGL::ConstScenePtr scene = proceduralHolder->scene();
				if( scene )
				{
					bool popTexture = false;
					if( request.displayStyle()==M3dView::kGouraudShaded || request.displayStyle()==M3dView::kFlatShaded )
					{
						glPushAttrib( GL_TEXTURE_BIT );
						popTexture = true;
						// set up the material. we probably need to do some work to prevent the base state passed to
						// the scene render from overriding aspects of this
						MMaterial material = request.material();
						material.setMaterial( request.multiPath(), request.isTransparent() );
						if( material.materialIsTextured() )
						{
							glEnable( GL_TEXTURE_2D );
							material.applyTexture( view, drawData );
						}
					}
						scene->render( baseState( (M3dView::DisplayStyle)request.displayStyle() ) );
					if( popTexture )
					{
						glPopAttrib();
					}
				}
			}
				
		}
		catch( const IECoreGL::Exception &e )
		{
			// much better to catch and report this than to let the application die
			IECore::msg( IECore::Msg::Error, "ProceduralHolderUI::draw", boost::format( "IECoreGL Exception : %s" ) % e.what() );
		}
		
	view.endGL();
}

bool ProceduralHolderUI::select( MSelectInfo &selectInfo, MSelectionList &selectionList, MPointArray &worldSpaceSelectPts ) const
{
	MSelectionList toSelect;
	toSelect.add( selectInfo.selectPath() );
	selectInfo.addSelection( toSelect, MPoint(), selectionList, worldSpaceSelectPts, MSelectionMask::kSelectObjectsMask, false );
	return true;
}

IECoreGL::StatePtr ProceduralHolderUI::baseState( M3dView::DisplayStyle style ) const
{
	static bool init = false;
	static IECoreGL::StatePtr wireframe = new IECoreGL::State( true );
	static IECoreGL::StatePtr shaded = new IECoreGL::State( true );
	static IECoreGL::StatePtr points = new IECoreGL::State( true );
	static IECoreGL::StatePtr bounds = new IECoreGL::State( true );
	if( !init )
	{
		wireframe->add( new IECoreGL::PrimitiveSolid( false ) );
		wireframe->add( new IECoreGL::PrimitiveWireframe( true ) );
		
		points->add( new IECoreGL::PrimitiveSolid( false ) );
		points->add( new IECoreGL::PrimitivePoints( true ) );
		points->add( new IECoreGL::PrimitivePointWidth( 2.0f ) );
		
		bounds->add( new IECoreGL::PrimitiveSolid( false ) );
		bounds->add( new IECoreGL::PrimitiveBound( true ) );
		init = true;
	}
		
	switch( style )
	{
		case M3dView::kBoundingBox :
			return bounds;
			
		case M3dView::kFlatShaded :
		case M3dView::kGouraudShaded :
			return shaded;
			
		case M3dView::kWireFrame :
			return wireframe;
			
		case M3dView::kPoints :
			return points;
			
		default :
			return shaded;
	}
	
}
