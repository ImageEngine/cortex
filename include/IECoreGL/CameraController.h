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

#ifndef IECOREGL_CAMERACONTROLLER_H
#define IECOREGL_CAMERACONTROLLER_H

#include "IECore/RefCounted.h"

#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathBox.h"

namespace IECoreGL
{

IE_CORE_FORWARDDECLARE( Camera )

/// \deprecated Use IECore::CameraController (possibly in conjunction
/// with ToGLCameraConverter) instead.
class CameraController : public IECore::RefCounted
{

	public :

		IE_CORE_DECLAREMEMBERPTR( CameraController );

		CameraController( CameraPtr camera, float centreOfInterest = 5.0f );

		void setCamera( CameraPtr camera );
		CameraPtr getCamera();

		void setCentreOfInterest( float centreOfInterest );
		float getCentreOfInterest();

		/// Changes the camera resolution, modifying the screen window
		/// to preserve the horizontal framing and change the vertical
		/// framing to maintain aspect ratio.
		void reshape( int resolutionX, int resolutionY );
		/// Translates the camera to frame the specified box, keeping the
		/// current viewing direction unchanged.
		void frame( const Imath::Box3f &box );
		/// Moves the camera to frame the specified box, viewing it from the
		/// specified direction, and with the specified up vector.
		void frame( const Imath::Box3f &box, const Imath::V3f &viewDirection,
			const Imath::V3f &upVector = Imath::V3f( 0, 1, 0 ) );
		void track( int dx, int dy );
		void tumble( int dx, int dy );
		void dolly( int dx, int dy );

	private :

		CameraPtr m_camera;
		float m_centreOfInterest;

};

IE_CORE_DECLAREPTR( CameraController );

};

#endif // IECOREGL_CAMERACONTROLLER_H
