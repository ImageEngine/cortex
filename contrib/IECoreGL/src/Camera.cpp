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

#include "IECoreGL/Camera.h"
#include "IECoreGL/GL.h"

#include "OpenEXR/ImathMatrix.h"

using namespace IECoreGL;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( Camera );

Camera::Camera( const Imath::M44f &transform,
	const Imath::V2i &resolution,
	const Imath::Box2f &screenWindow,
	const Imath::V2f &clippingPlanes
)
	:	m_transform( transform ), m_resolution( resolution ), m_screenWindow( screenWindow ), m_clippingPlanes( clippingPlanes )
{
	if( m_screenWindow.isEmpty() )
	{
		m_screenWindow.min.x = -1.0f;
		m_screenWindow.max.x = 1.0f;
		float y = (float)resolution.y / (float)resolution.x;
		m_screenWindow.min.y = -y;
		m_screenWindow.max.y = y;
	}
}

void Camera::setTransform( const Imath::M44f &transform )
{
	m_transform = transform;
}

const Imath::M44f &Camera::getTransform() const
{
	return m_transform;
}

void Camera::setResolution( const Imath::V2i &resolution )
{
	m_resolution = resolution;
}

const Imath::V2i &Camera::getResolution() const
{
	return m_resolution;
}

void Camera::setScreenWindow( const Imath::Box2f &screenWindow )
{
	m_screenWindow = screenWindow;
}

const Imath::Box2f &Camera::getScreenWindow() const
{
	return m_screenWindow;
}

void Camera::setClippingPlanes( const Imath::V2f &clippingPlanes )
{
	m_clippingPlanes = clippingPlanes;
}

const Imath::V2f &Camera::getClippingPlanes() const
{
	return m_clippingPlanes;
}

Imath::Box3f Camera::bound() const
{
	return Box3f();
}

Imath::M44f Camera::matrix()
{
	Imath::M44f obj2Camera;
	glGetFloatv( GL_MODELVIEW_MATRIX, obj2Camera.getValue() );
	return obj2Camera;
}

Imath::M44f Camera::projectionMatrix()
{
	Imath::M44f projection;
	glGetFloatv( GL_PROJECTION_MATRIX, projection.getValue() );
	return projection;
}

bool Camera::perspectiveProjection()
{
	M44f p = projectionMatrix();
	return p[2][3] != 0.0f;
}

Imath::V3f Camera::positionInObjectSpace()
{
	Imath::M44f obj2Camera = matrix();
	obj2Camera.invert();
	return V3f( 0 ) * obj2Camera;
}

Imath::V3f Camera::viewDirectionInObjectSpace()
{
	Imath::M44f obj2Camera = matrix();
	obj2Camera.invert();
	V3f view( 0, 0, -1 );
	obj2Camera.multDirMatrix( view, view );
	return view;
}

Imath::V3f Camera::upInObjectSpace()
{
	Imath::M44f obj2Camera = matrix();
	obj2Camera.invert();
	V3f up( 0, 1, 0 );
	obj2Camera.multDirMatrix( up, up );
	return up;
}

void Camera::setModelViewMatrix() const
{
	M44f inverseMatrix = m_transform.inverse();
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glMultMatrixf( inverseMatrix.getValue() );
}
