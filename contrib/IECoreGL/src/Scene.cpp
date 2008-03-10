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
#include "IECoreGL/Group.h"
#include "IECoreGL/State.h"
#include "IECoreGL/Camera.h"
#include "IECoreGL/NameStateComponent.h"
#include "IECoreGL/Exception.h"

#include "IECore/MessageHandler.h"

#include "OpenEXR/ImathFun.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

Scene::Scene()
	:	m_root( new Group ), m_camera( 0 )
{
}

Scene::~Scene()
{
}

void Scene::render( ConstStatePtr state ) const
{
	if( m_camera )
	{
		m_camera->render( state );
	}

	glPushAttrib( GL_ALL_ATTRIB_BITS );

		State::bindBaseState();
		state->bind();
		root()->render( state );
	
	glPopAttrib();
}

void Scene::render() const
{
	render( State::defaultState() );
}

Imath::Box3f Scene::bound() const
{
	return root()->bound();
}

unsigned Scene::select( const Imath::Box2f &region, std::list<HitRecord> &hits ) const
{
	ConstStatePtr state = State::defaultState();
	
	if( m_camera )
	{
		m_camera->render( state );
	}
	
	// perform the region framing
	GLdouble projectionMatrix[16];
	glGetDoublev( GL_PROJECTION_MATRIX, projectionMatrix );
	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
		
	V2f regionCenter = region.center();
	V2f regionSize = region.size();
	regionCenter.x = viewport[0] + viewport[2] * regionCenter.x;
	regionCenter.y = viewport[1] + viewport[3] * (1.0f - regionCenter.y);
	regionSize.x *= viewport[2];
	regionSize.y *= viewport[3];
		
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPickMatrix( regionCenter.x, regionCenter.y, regionSize.x, regionSize.y, viewport );
	glMultMatrixd( projectionMatrix );
	glMatrixMode( GL_MODELVIEW );
	
	// do the selection render
	static const unsigned int selectBufferSize = 20000; // enough to select 5000 distinct objects
	static GLuint selectBuffer[selectBufferSize];
	glSelectBuffer( selectBufferSize, selectBuffer );
	glRenderMode( GL_SELECT );
	
		glInitNames();
		glPushName( 0 );
	
		glPushAttrib( GL_ALL_ATTRIB_BITS );

			State::bindBaseState();
			state->bind();
			root()->render( state );

		glPopAttrib();	

	int numHits = glRenderMode( GL_RENDER );
	if( numHits < 0 ) 
	{
		IECore::msg( IECore::Msg::Warning, "IECoreGL::Scene::select", "Selection buffer overflow." );
		numHits *= -1;
	}
	
	// get the hits out of the select buffer.
	GLuint *hitRecord = selectBuffer;
	for( int i=0; i<numHits; i++ )
	{
		HitRecord h( hitRecord );
		hits.push_back( h );
		hitRecord += h.offsetToNext();
	}
	
	return numHits;
}

void Scene::setCamera( CameraPtr camera )
{
	m_camera = camera;
}

CameraPtr Scene::getCamera()
{
	return m_camera;
}

ConstCameraPtr Scene::getCamera() const
{
	return m_camera;
}

GroupPtr Scene::root()
{
	return m_root;
}

ConstGroupPtr Scene::root() const
{
	return m_root;
}
