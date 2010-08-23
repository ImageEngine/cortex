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

#include "IECoreGL/Window.h"
#include "IECoreGL/GLUT.h"

#include <iostream>

using namespace IECoreGL;
using namespace Imath;
using namespace std;

Window::WindowMap Window::m_windows;

Window::Window( const std::string &title )
{
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
	glutInitWindowSize( 640, 480 );
	glutInitWindowPosition( 100, 100 );
	m_window = glutCreateWindow( title.c_str() );
	m_title = title;
	m_windows[m_window] = this;
	glutDisplayFunc( displayFunc );
	glutReshapeFunc( reshapeFunc );
	glutMouseFunc( mouseFunc );
	glutMotionFunc( motionFunc );
	glutPassiveMotionFunc( passiveMotionFunc );
	glutKeyboardFunc( keyboardFunc );
	m_mouseLeftDown = m_mouseMiddleDown = m_mouseRightDown = false;
}

Window::~Window()
{
	glutDestroyWindow( m_window );
	m_windows.erase( m_window );
}

const std::string &Window::getTitle() const
{
	return m_title;
}

void Window::setTitle( const std::string &title )
{
	m_title = title;
	glutSetWindow( m_window );
	glutSetWindowTitle( m_title.c_str() );
}

void Window::setVisibility( bool visible )
{
	glutSetWindow( m_window );
	if( visible )
	{
		glutShowWindow();
	}
	else
	{
		glutHideWindow();
	}
	m_visible = visible;
}

bool Window::getVisibility() const
{
	return m_visible;
}

void Window::reshape( int width, int height )
{
	glViewport( 0, 0, width, height );
}

void Window::display()
{
	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glClear( GL_COLOR_BUFFER_BIT );
	glFlush();
}

void Window::mouse( int button, int state, int x, int y )
{
	switch( button )
	{
		case GLUT_LEFT_BUTTON :
			m_mouseLeftDown = state == GLUT_DOWN;
			break;
		case GLUT_MIDDLE_BUTTON :
			m_mouseMiddleDown = state == GLUT_DOWN;
			break;
		case GLUT_RIGHT_BUTTON :
			m_mouseRightDown = state == GLUT_DOWN;
			break;
	}
	if( state==GLUT_DOWN )
	{
		m_lastMouseClickPosition = V2i( x, y );
		m_lastMouseDragPosition = V2i( x, y );
	}
}

void Window::keyboard( unsigned char key, int x, int y )
{
}

Imath::V2i Window::lastMouseClickPosition() const
{
	return m_lastMouseClickPosition;
}

Imath::V2i Window::lastMouseDragPosition() const
{
	return m_lastMouseDragPosition;
}

void Window::motion( int x, int y )
{
	m_lastMouseDragPosition = V2i( x, y );
}

void Window::passiveMotion( int x, int y )
{
}

void Window::postRedisplay() const
{
	int w = glutGetWindow();
	glutSetWindow( m_window );
		glutPostRedisplay();
	glutSetWindow( w );
}

bool Window::mouseDown( int button ) const
{
	switch( button )
	{
		case GLUT_LEFT_BUTTON :
			return m_mouseLeftDown;
		case GLUT_MIDDLE_BUTTON :
			return m_mouseMiddleDown;
		case GLUT_RIGHT_BUTTON :
			return m_mouseRightDown;
	};
	return false;
}

void Window::start()
{
	glutMainLoop();
}

void Window::displayFunc()
{
	WindowMap::iterator it = m_windows.find( glutGetWindow() );
	it->second->display();
}

void Window::reshapeFunc( int width, int height )
{
	WindowMap::iterator it = m_windows.find( glutGetWindow() );
	it->second->reshape( width, height );
}

void Window::mouseFunc( int button, int state, int x, int y )
{
	WindowMap::iterator it = m_windows.find( glutGetWindow() );
	it->second->mouse( button, state, x, y );
}

void Window::motionFunc( int x, int y )
{
	WindowMap::iterator it = m_windows.find( glutGetWindow() );
	it->second->motion( x, y );
}

void Window::passiveMotionFunc( int x, int y )
{
	WindowMap::iterator it = m_windows.find( glutGetWindow() );
	it->second->passiveMotion( x, y );
}

void Window::keyboardFunc( unsigned char key, int x, int y )
{
	WindowMap::iterator it = m_windows.find( glutGetWindow() );
	it->second->keyboard( key, x, y );
}
