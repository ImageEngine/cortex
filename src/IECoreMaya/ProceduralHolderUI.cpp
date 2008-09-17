//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
#include "IECoreMaya/DisplayStyle.h"

#include "IECore/MessageHandler.h"
#include "IECore/ClassData.h"

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

using namespace IECoreMaya;
using namespace std;


struct ProceduralHolderUI::MemberData
{
	mutable StateMap m_stateMap;
	DisplayStyle m_displayStyle;
};

static IECore::ClassData< ProceduralHolderUI, ProceduralHolderUI::MemberData > g_classData;

ProceduralHolderUI::ProceduralHolderUI()
	:	m_boxPrimitive( new IECoreGL::BoxPrimitive( Imath::Box3f() ) )
{
	g_classData.create( this );
}

ProceduralHolderUI::~ProceduralHolderUI()
{
	g_classData.erase( this );
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
				IECoreGL::ConstStatePtr wireframeState = g_classData[this].m_displayStyle.baseState( M3dView::kWireFrame );
				m_boxPrimitive->setBox( IECore::convert<Imath::Box3f>( proceduralHolder->boundingBox() ) );
				glPushAttrib( wireframeState->mask() );
					(boost::static_pointer_cast<IECoreGL::Renderable>( m_boxPrimitive ))->render( wireframeState );
				glPopAttrib();
			}

			// draw the scene if asked
			if( request.token()==SceneDrawMode )
			{
				resetHilites();
			
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

					IECoreGL::ConstStatePtr displayState = g_classData[this].m_displayStyle.baseState( (M3dView::DisplayStyle)request.displayStyle() );

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

							assert( proceduralHolder->componentToGroupMap().find( compId ) != proceduralHolder->componentToGroupMap().end() );

							hiliteGroups( 
								proceduralHolder->componentToGroupMap()[compId], 
								hilite,
								boost::const_pointer_cast<IECoreGL::WireframeColorStateComponent>( displayState->get< IECoreGL::WireframeColorStateComponent >() )
							);								
						} 
					}
					
					scene->render( displayState );
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
	MStatus s;
	
	M3dView view = selectInfo.view();
	
	ProceduralHolder *proceduralHolder = static_cast<ProceduralHolder *>( surfaceShape() );
	assert( proceduralHolder );
		
	bool hilited = (selectInfo.displayStatus() == M3dView::kHilite);
				
	static const unsigned int selectBufferSize = 20000; // enough to select 5000 distinct objects
	static GLuint selectBuffer[selectBufferSize];

	IECoreGL::ConstScenePtr scene = proceduralHolder->scene();
	if( scene )
	{		
		view.beginSelect( &selectBuffer[0], selectBufferSize );
		glInitNames();
		glPushName( 0 );
		scene->render( g_classData[this].m_displayStyle.baseState( selectInfo.displayStyle() ) );
	}
	else
	{
		return false;
	}

	int numHits = view.endSelect();

	// Get the hits out of the select buffer.
	typedef std::list<IECoreGL::HitRecord> HitRecordList;		
	HitRecordList hits;
	GLuint *hitRecord = selectBuffer;
	for( int i=0; i<numHits; i++ )
	{
		IECoreGL::HitRecord h( hitRecord );
		hits.push_back( h );
		hitRecord += h.offsetToNext();
	}

	/// Process the hits
	bool selected = false;
	MFnSingleIndexedComponent fnComponent;
	MObject component = fnComponent.create( MFn::kMeshPolygonComponent, &s );	
	assert( s );
	bool foundClosest = false;
	int closestCompId = -1;
	float depthMin = std::numeric_limits<float>::max();
	for ( HitRecordList::const_iterator it = hits.begin(); it != hits.end(); ++it )
	{
		const std::string &hitName = it->name.value();
		selected = true;
		
		ProceduralHolder::ComponentsMap::const_iterator compIt = proceduralHolder->componentsMap().find( hitName );
		assert( compIt != proceduralHolder->componentsMap().end() );
				
		int compId = compIt->second;

		if ( selectInfo.singleSelection()  )
		{
			if ( !foundClosest )
			{
				closestCompId = compId;
				foundClosest = true;
				depthMin = it->depthMin;
			}
			else
			{
				if ( it->depthMin < depthMin )
				{
					closestCompId = compId;
					depthMin = it->depthMin;
				}
			}
		}
		else
		{
			assert( proceduralHolder->componentToGroupMap().find( compId ) != proceduralHolder->componentToGroupMap().end() );
			
			const ProceduralHolder::ComponentToGroupMap::mapped_type &groups = proceduralHolder->componentToGroupMap()[compId];			
			for ( ProceduralHolder::ComponentToGroupMap::mapped_type::const_iterator jit = groups.begin(); jit != groups.end(); ++jit )
			{			
				const IECoreGL::GroupPtr &group = jit->second;
				
				MPoint pt = IECore::convert< MPoint > ( group->bound().center() );
				pt *= selectInfo.selectPath().inclusiveMatrix();
						
				worldSpaceSelectPts.append( pt );				
			}
			
			fnComponent.addElement( compId );
		}		
	}
	
	if ( !selected )
	{
		return false;
	}
	
	MPoint selectionPoint( 0, 0, 0, 1 );
	if ( hilited )
	{
		if ( selectInfo.singleSelection() )
		{
			assert( foundClosest );
			assert( closestCompId >= 0 );
			
			assert( proceduralHolder->componentToGroupMap().find( closestCompId ) != proceduralHolder->componentToGroupMap().end() );
			
			const ProceduralHolder::ComponentToGroupMap::mapped_type &groups = proceduralHolder->componentToGroupMap()[closestCompId];
			for ( ProceduralHolder::ComponentToGroupMap::mapped_type::const_iterator jit = groups.begin(); jit != groups.end(); ++jit )
			{			
				const IECoreGL::GroupPtr &group = jit->second;
				
				MPoint pt = IECore::convert< MPoint > ( group->bound().center() );
				pt *= selectInfo.selectPath().inclusiveMatrix();
				
				worldSpaceSelectPts.append( pt );				
			}
			
			fnComponent.addElement( closestCompId );
		}
	
		const MDagPath &path = selectInfo.multiPath();
			
		MSelectionList item;
		item.add( path, component );

		MSelectionMask mask( MSelectionMask::kSelectComponentsMask );
		selectInfo.addSelection(
			item, selectionPoint,
			selectionList, worldSpaceSelectPts,
			MSelectionMask::kSelectObjectsMask, 
			true );
	}
	else
	{
		MSelectionList item;		
		item.add( selectInfo.selectPath() );
		
		if ( selectInfo.singleSelection() )
		{
			/// \todo Find a way of creating a PrimitiveEvaluator to fire the selection ray at
			selectionPoint = proceduralHolder->boundingBox().center();
			selectionPoint *= selectInfo.selectPath().inclusiveMatrix();
		}
		
		selectInfo.addSelection( 
			item, selectionPoint, 
			selectionList, worldSpaceSelectPts, 
			MSelectionMask::kSelectObjectsMask,
			false );
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
	
	StateMap &m_stateMap = g_classData[this].m_stateMap;
		
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
	
	StateMap &m_stateMap = g_classData[this].m_stateMap;	
	
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

void ProceduralHolderUI::resetHilites() const
{
	StateMap &m_stateMap = g_classData[this].m_stateMap;
	
	for ( StateMap::iterator it = m_stateMap.begin(); it != m_stateMap.end(); ++it )
	{
		it->first->setState( it->second );
	}
	
	m_stateMap.clear();
}

