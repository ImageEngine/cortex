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

#ifndef IECOREGL_CAMERA_H
#define IECOREGL_CAMERA_H

#include "IECoreGL/Renderable.h"

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathMatrix.h"

namespace IECoreGL
{

class Camera : public Renderable
{

	public :

		/// If screenWindow is specified as an empty box (the default),
		/// then it'll be calculated to be -1,1 in width and to preserve
		/// aspect ratio (based on resolution) in height.
		/// \todo Should make default screenWindow behaviour match that
		/// in IECore::Camera::addStandardParameters().
		Camera( const Imath::M44f &transform = Imath::M44f(),
			const Imath::V2i &resolution = Imath::V2i( 640, 480 ),
			const Imath::Box2f &screenWindow = Imath::Box2f(),
			const Imath::V2f &clippingPlanes = Imath::V2f( 0.1, 1000 )
		);

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( IECoreGL::Camera, CameraTypeId, Renderable );

		/// Specifies the transform of the camera relative to the world.
		void setTransform( const Imath::M44f &transform );
		const Imath::M44f &getTransform() const;

		void setResolution( const Imath::V2i &resolution );
		const Imath::V2i &getResolution() const;

		void setScreenWindow( const Imath::Box2f &screenWindow );
		const Imath::Box2f &getScreenWindow() const;

		void setClippingPlanes( const Imath::V2f &clippingPlanes );
		const Imath::V2f &getClippingPlanes() const;

		/// Returns an empty box.
		virtual Imath::Box3f bound() const;

		//! @name OpenGL query functions
		/// These functions provide basic information about the
		/// current OpenGL camera based entirely on the current
		/// graphics state.
		///////////////////////////////////////////////////////////
		//@{
		/// The matrix that converts from the current GL object space
		/// to camera space (the GL_MODELVIEW_MATRIX).
		static Imath::M44f matrix();
		/// The projection matrix (the GL_PROJECTION_MATRIX).
		static Imath::M44f projectionMatrix();
		/// True if the camera performs a perspective projection.
		static bool perspectiveProjection();
		/// Calculates the position of the current GL camera
		/// relative to the current GL transform.
		static Imath::V3f positionInObjectSpace();
		/// Calculates the view direction of the current GL camera
		/// relative to the current GL transform.
		static Imath::V3f viewDirectionInObjectSpace();
		/// Calculates the up vector of the current GL camera
		/// relative to the current GL transform.
		static Imath::V3f upInObjectSpace();
		//@}

	protected :

		void setModelViewMatrix() const;

		Imath::M44f m_transform;
		Imath::V2i m_resolution;
		Imath::Box2f m_screenWindow;
		Imath::V2f m_clippingPlanes;

};

IE_CORE_DECLAREPTR( Camera );

};

#endif // IECOREGL_CAMERA_H
