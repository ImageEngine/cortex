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

#ifndef IECOREGL_WINDOW_H
#define IECOREGL_WINDOW_H

#include "IECore/RefCounted.h"

#include "OpenEXR/ImathVec.h"

#include <map>
#include <string>

namespace IECoreGL
{

/// The Window class provides a simple object based
/// wrapper around the GLUT windowing functionality.
/// GLUT callback functions are implemented by
/// implementing a virtual function for each callback.
/// Before using this class you must either have initialised
/// GLUT yourself or called IECoreGL::init( false );
class Window : public IECore::RefCounted
{

	public :
	
		/// Creates a new window with the specified
		/// title.
		Window( const std::string &title );
		virtual ~Window();

		/// Returns the window title.
		const std::string &getTitle() const;
		/// Sets the window title.
		void setTitle( const std::string &title );
		
		/// Sets whether the window is shown or
		/// hidden.
		void setVisibility( bool visible );
		/// Returns whether the window is shown or
		/// hidden.
		bool getVisibility() const;
		
		/// Enters the main GLUT display loop. Once
		/// you're in that there's no getting back.
		static void start();
		
	protected :
	
		//! @name Callbacks
		/// These correspond directly to the glut
		/// callback functions for the window.
		////////////////////////////////////////////////
		//@{
		virtual void reshape( int width, int height );
		virtual void display();
		/// Implemented to store the status of each button,
		/// so that it can be queried with the mouseDown()
		/// function.
		virtual void mouse( int button, int state, int x, int y );
		virtual void motion( int x, int y );
		virtual void passiveMotion( int x, int y );
		virtual void keyboard( unsigned char key, int x, int y );
		//@}
		
		/// Calls glutPostRedisplay() for this window.
		void postRedisplay() const;
		
		/// Returns true if the specified mouse button is pressed.
		/// button is in the same format as passed to the mouse()
		/// callback function - ie one of the GLUT_*_BUTTON
		/// enumerations. This is only reliable if any overrides
		/// to mouse() also call the base class mouse().
		bool mouseDown( int button ) const;
		/// Returns the position of the last mouse click. This is
		/// only reliable if any overrides to mouse() also call the
		/// base class mouse().
		Imath::V2i lastMouseClickPosition() const;
		/// Returns the position of the last mouse drag position. This is
		/// only reliable if any overrides to motion() and mouse() also
		/// call the base class equivalents
		Imath::V2i lastMouseDragPosition() const;
		
	private :
	
		int m_window;
		std::string m_title;
		bool m_visible;
	
		bool m_mouseLeftDown;
		bool m_mouseMiddleDown;
		bool m_mouseRightDown;
		Imath::V2i m_lastMouseClickPosition;
		Imath::V2i m_lastMouseDragPosition;
				
		typedef std::map<int, Window *> WindowMap;
		static WindowMap m_windows;
		static void displayFunc();
		static void reshapeFunc( int width, int height );
		static void mouseFunc( int button, int state, int x, int y );
		static void motionFunc( int x, int y );
		static void passiveMotionFunc( int x, int y );
		static void keyboardFunc( unsigned char key, int x, int y );

};

IE_CORE_DECLAREPTR( Window );

} // namespace IECoreGL

#endif // IECOREGL_WINDOW_H
