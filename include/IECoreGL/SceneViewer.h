//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_SCENEVIEWER_H
#define IECOREGL_SCENEVIEWER_H

#include "IECoreGL/Window.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Scene );
IE_CORE_FORWARDDECLARE( Camera );
IE_CORE_FORWARDDECLARE( CameraController );

/// A simple utility Window class to allow
/// the viewing of Scene instances.
class SceneViewer : public Window
{

	public :

		IE_CORE_DECLAREMEMBERPTR( SceneViewer );

		/// Creates a new window with a view onto the
		/// specified scene.
		SceneViewer( const std::string &title, ScenePtr scene );
		virtual ~SceneViewer();

	protected :

		//! @name Callbacks
		////////////////////////////////////////////////
		//@{
		virtual void reshape( int width, int height );
		virtual void display();
		virtual void motion( int x, int y );
		virtual void keyboard( unsigned char key, int x, int y );
		//@}

	private :

		ScenePtr m_scene;
		CameraControllerPtr m_cameraController;

};

IE_CORE_DECLAREPTR( SceneViewer );

} // namespace IECoreGL

#endif // IECOREGL_SCENEVIEWER_H
