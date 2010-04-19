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

#ifndef IECOREGL_PERSPECTIVECAMERA_H
#define IECOREGL_PERSPECTIVECAMERA_H

#include "IECoreGL/Camera.h"

namespace IECoreGL
{

class PerspectiveCamera : public Camera
{

	public :

		PerspectiveCamera( const Imath::M44f &transform = Imath::M44f(),
			const Imath::V2i &resolution = Imath::V2i( 640, 480 ),
			const Imath::Box2f &screenWindow = Imath::Box2f( Imath::V2f( -1 ), Imath::V2f( 1 ) ),
			const Imath::V2f &clippingPlanes = Imath::V2f( 0.1, 1000 ),
			float horizontalFOV = 90.0f
		);

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::PerspectiveCamera, PerspectiveCameraTypeId, Camera );

		void setFOV( float fov );
		float getFOV() const;

		/// \todo Should the render() method actually draw a representation of the camera,
		/// and some other method be used for setting the camera up?
		virtual void render( ConstStatePtr state ) const;

	protected :

		float m_fov;

};

IE_CORE_DECLAREPTR( PerspectiveCamera );

};

#endif // IECOREGL_PERSPECTIVECAMERA_H
