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

#include "IECoreGL/PerspectiveCamera.h"
#include "IECoreGL/GL.h"

using namespace IECoreGL;
using namespace Imath;
using namespace std;

PerspectiveCamera::PerspectiveCamera( const Imath::M44f &transform,
	const Imath::V2i &resolution,
	const Imath::Box2f &screenWindow,
	const Imath::V2f &clippingPlanes,
	float horizontalFOV	)
	:	Camera( transform, resolution, screenWindow, clippingPlanes )
{
	setFOV( horizontalFOV );
}

void PerspectiveCamera::setFOV( float fov )
{
	m_fov = fov;
}

float PerspectiveCamera::getFOV() const
{
	return m_fov;
}

void PerspectiveCamera::render( ConstStatePtr state ) const
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	float r = m_clippingPlanes[0] * tan( M_PI * m_fov / 360.0 );
	
	glFrustum( r * m_screenWindow.min.x, r * m_screenWindow.max.x,
		r * m_screenWindow.min.y, r * m_screenWindow.max.y,
		m_clippingPlanes[0], m_clippingPlanes[1] );
		
	M44f inverseMatrix = m_transform.inverse();
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glMultMatrixf( inverseMatrix.getValue() );
}
