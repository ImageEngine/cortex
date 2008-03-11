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

#include "IECoreGL/SceneViewer.h"
#include "IECoreGL/PerspectiveCamera.h"
#include "IECoreGL/CameraController.h"
#include "IECoreGL/Scene.h"
#include "IECoreGL/State.h"
#include "IECoreGL/GL.h"
#include "IECoreGL/GLUT.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

SceneViewer::SceneViewer( const std::string &title, ScenePtr scene )
	:	Window( title ), m_scene( scene )
{
	if( !m_scene->getCamera() )
	{
		m_scene->setCamera( new PerspectiveCamera );
	}
	m_cameraController = new CameraController( m_scene->getCamera() );
}

SceneViewer::~SceneViewer()
{
}

void SceneViewer::reshape( int width, int height )
{
	glViewport( 0, 0, width, height );
	m_cameraController->reshape( width, height );
}

void SceneViewer::display()
{
	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glClearDepth( 1.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
	m_scene->render();
	glutSwapBuffers();
}

void SceneViewer::motion( int x, int y )
{
	if( mouseDown( GLUT_LEFT_BUTTON ) && mouseDown( GLUT_MIDDLE_BUTTON ) )
	{
		V2i lastDrag = lastMouseDragPosition();
		m_cameraController->dolly( x - lastDrag.x, y - lastDrag.y );	
		postRedisplay();
	}
	else if( mouseDown( GLUT_MIDDLE_BUTTON ) )
	{
		V2i lastDrag = lastMouseDragPosition();
		m_cameraController->track( x - lastDrag.x, y - lastDrag.y );
		postRedisplay();
	}
	else if( mouseDown( GLUT_LEFT_BUTTON ) )
	{
		V2i lastDrag = lastMouseDragPosition();
		m_cameraController->tumble( x - lastDrag.x, y - lastDrag.y );
		postRedisplay();		
	}
	
	Window::motion( x, y );
}

void SceneViewer::keyboard( unsigned char key, int x, int y )
{
	if( key=='f' || key=='F' )
	{
		m_cameraController->frame( m_scene->bound() );
		postRedisplay();
	}
}
