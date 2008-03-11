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

#include "IECoreGL/CameraController.h"
#include "IECoreGL/PerspectiveCamera.h"

#include "OpenEXR/ImathBoxAlgo.h"
#include "OpenEXR/ImathMatrixAlgo.h"

using namespace IECoreGL;
using namespace Imath;
using namespace boost;
using namespace std;

CameraController::CameraController( CameraPtr camera, float centreOfInterest )
{
	setCamera( camera );
	setCentreOfInterest( centreOfInterest );
}

void CameraController::setCamera( CameraPtr camera )
{
	m_camera = camera;
}

CameraPtr CameraController::getCamera()
{
	return m_camera;
}

void CameraController::setCentreOfInterest( float centreOfInterest )
{
	m_centreOfInterest = centreOfInterest;
}

float CameraController::getCentreOfInterest()
{
	return m_centreOfInterest;
}

void CameraController::reshape( int resolutionX, int resolutionY )
{
	V2i oldRes = m_camera->getResolution();
	float oldAspect = (float)oldRes.x/(float)oldRes.y;
	float badAspect = (float)resolutionX/(float)resolutionY;
	float yScale = oldAspect / badAspect;
	
	m_camera->setResolution( V2i( resolutionX, resolutionY ) );
	Box2f screenWindow = m_camera->getScreenWindow();
	screenWindow.min.y *= yScale;
	screenWindow.max.y *= yScale;
	m_camera->setScreenWindow( screenWindow );
}

void CameraController::frame( const Imath::Box3f &box )
{
	V3f z( 0, 0, 1 );
	V3f y( 0, 1, 0 );
	M44f t = m_camera->getTransform();
	t.multDirMatrix( z, z );
	t.multDirMatrix( y, y );
	frame( box, z, y );
}

void CameraController::frame( const Imath::Box3f &box, const Imath::V3f &viewDirection, const Imath::V3f &upVector )
{
	// make a matrix to centre the camera on the box, with the appropriate view direction
	M44f cameraMatrix = rotationMatrixWithUpDir( V3f( 0, 0, 1 ), viewDirection, upVector );
	M44f translationMatrix;
	translationMatrix.translate( box.center() );
	cameraMatrix *= translationMatrix;

	// translate the camera back until the box is completely visible
	M44f inverseCameraMatrix = cameraMatrix.inverse();
	Box3f cBox = transform( box, inverseCameraMatrix );
	
	Box2f screenWindow = m_camera->getScreenWindow();
	if( m_camera->isInstanceOf( PerspectiveCamera::staticTypeId() ) )
	{
		// perspective. leave the field of view and screen window as is and translate
		// back till the box is wholly visible. this currently assumes the screen window
		// is centred about the camera axis.
		PerspectiveCameraPtr perspCamera = static_pointer_cast<PerspectiveCamera>( m_camera );
		
		float z0 = cBox.size().x / screenWindow.size().x;
		float z1 = cBox.size().y / screenWindow.size().y;
		
		m_centreOfInterest = max( z0, z1 ) / tan( M_PI * perspCamera->getFOV() / 360.0 ) + cBox.size().z / 2.0f;
		
		cameraMatrix.translate( V3f( 0.0f, 0.0f, -m_centreOfInterest ) );
	}
	else
	{
		// orthographic. translate to front of box and set screen window
		// to frame the box, maintaining the aspect ratio of the screen window.
		m_centreOfInterest = cBox.size().z / 2.0f + m_camera->getClippingPlanes()[0];
		cameraMatrix.translate( V3f( 0.0f, 0.0f, -m_centreOfInterest ) );
		
		float xScale = cBox.size().x / screenWindow.size().x;
		float yScale = cBox.size().y / screenWindow.size().y;
		float scale = max( xScale, yScale );
		
		V2f newSize = screenWindow.size() * scale;
		screenWindow.min.x = cBox.center().x - newSize.x / 2.0f;
		screenWindow.min.y = cBox.center().y - newSize.y / 2.0f;
		screenWindow.max.x = cBox.center().x + newSize.x / 2.0f;
		screenWindow.max.y = cBox.center().y + newSize.y / 2.0f;
	}
	
	m_camera->setTransform( cameraMatrix );
	m_camera->setScreenWindow( screenWindow );
}
		
void CameraController::track( int dx, int dy )
{
	V2i resolution = m_camera->getResolution();
	Box2f screenWindow = m_camera->getScreenWindow();
	
	V3f translate( 0.0f );
	translate.x = -screenWindow.size().x * (float)dx/(float)resolution.x;
	translate.y = screenWindow.size().y * (float)dy/(float)resolution.y;
	if( m_camera->isInstanceOf( PerspectiveCamera::staticTypeId() ) )
	{
		PerspectiveCameraPtr persp = static_pointer_cast<PerspectiveCamera>( m_camera );
		translate *= tan( M_PI * persp->getFOV() / 360.0f ) * (float)m_centreOfInterest;
	}

	M44f t = m_camera->getTransform();
	t.translate( translate );
	m_camera->setTransform( t );
}

void CameraController::tumble( int dx, int dy )
{
	M44f t = m_camera->getTransform();
	M44f ti = t.inverse();
	V3f yAxis( 0.0f, 1.0f, 0.f );
	ti.multDirMatrix( yAxis, yAxis );
	
	t.translate( V3f( 0.0f, 0.0f, m_centreOfInterest ) );
	
		t.rotate( V3f( dy, 0.0f, 0.0f ) / 100.0f );
		M44f yRotate;
		yRotate.setAxisAngle( yAxis, dx / 100.0f );
		t = yRotate * t;

	t.translate( V3f( 0.0f, 0.0f, -m_centreOfInterest ) );
	
	m_camera->setTransform( t );
}

void CameraController::dolly( int dx, int dy )
{
	V2i resolution = m_camera->getResolution();
	float d = (float)dx/(float)resolution.x + (float)dy/(float)resolution.y;
	
	if( m_camera->isInstanceOf( PerspectiveCamera::staticTypeId() ) )
	{
		M44f t = m_camera->getTransform();
		d *= 2.5f * m_centreOfInterest; // 2.5 is a magic number that just makes the speed nice
		t.translate( V3f( 0, 0, d ) );
		m_centreOfInterest -= d;
		m_camera->setTransform( t );
	}
	else
	{
		// orthographic
		Box2f screenWindow = m_camera->getScreenWindow();
		V2f dd = screenWindow.size() * d;
		screenWindow.min += dd;
		screenWindow.max -= dd;
		m_camera->setScreenWindow( screenWindow );
	}
}
