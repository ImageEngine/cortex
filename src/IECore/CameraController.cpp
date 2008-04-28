//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/CameraController.h"
#include "IECore/Camera.h"
#include "IECore/MatrixTransform.h"
#include "IECore/Exception.h"

#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathMatrixAlgo.h"
#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathBoxAlgo.h"

using namespace IECore;
using namespace Imath;

CameraController::CameraController( CameraPtr camera )
{
	setCamera( camera );
}

void CameraController::setCamera( CameraPtr camera )
{
	m_camera = camera;
	m_camera->addStandardParameters(); // subsequent casts are safe because of this
	m_resolution = boost::static_pointer_cast<V2iData>( m_camera->parameters()["resolution"] );
	m_screenWindow = boost::static_pointer_cast<Box2fData>( m_camera->parameters()["screenWindow"] );
	m_clippingPlanes = boost::static_pointer_cast<V2fData>( m_camera->parameters()["clippingPlanes"] );
	m_projection = boost::static_pointer_cast<StringData>( m_camera->parameters()["projection"] );
	if( m_projection->readable()=="perspective" )
	{
		m_fov = boost::static_pointer_cast<FloatData>( m_camera->parameters()["projection:fov"] );
	}
	else
	{
		m_fov = 0;
	}
	TransformPtr transform = m_camera->getTransform();
	if( !transform )
	{
		m_transform = new MatrixTransform;
		m_camera->setTransform( m_transform );
	}
	else if( !transform->isInstanceOf( MatrixTransform::staticTypeId() ) )
	{
		m_transform = new MatrixTransform( transform->transform() );
		m_camera->setTransform( m_transform );
	}
	
	m_centreOfInterest = 1;
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

void CameraController::setResolution( const Imath::V2i &resolution )
{
	V2i oldRes = m_resolution->readable();
	float oldAspect = (float)oldRes.x/(float)oldRes.y;
	float badAspect = (float)resolution.x/(float)resolution.y;
	float yScale = oldAspect / badAspect;
	
	m_resolution->writable() = V2i( resolution.x, resolution.y );
	Box2f screenWindow = m_screenWindow->readable();
	screenWindow.min.y *= yScale;
	screenWindow.max.y *= yScale;
	m_screenWindow->writable() = screenWindow;
}

const Imath::V2i &CameraController::getResolution() const
{
	return m_resolution->readable();
}

void CameraController::frame( const Imath::Box3f &box )
{
	V3f z( 0, 0, -1 );
	V3f y( 0, 1, 0 );
	M44f t = m_transform->matrix;
	t.multDirMatrix( z, z );
	t.multDirMatrix( y, y );
	frame( box, z, y );
}

void CameraController::frame( const Imath::Box3f &box, const Imath::V3f &viewDirection, const Imath::V3f &upVector )
{
	// make a matrix to centre the camera on the box, with the appropriate view direction
	M44f cameraMatrix = rotationMatrixWithUpDir( V3f( 0, 0, -1 ), viewDirection, upVector );
	M44f translationMatrix;
	translationMatrix.translate( box.center() );
	cameraMatrix *= translationMatrix;

	// translate the camera back until the box is completely visible
	M44f inverseCameraMatrix = cameraMatrix.inverse();
	Box3f cBox = transform( box, inverseCameraMatrix );
	
	Box2f screenWindow = m_screenWindow->readable();
	if( m_projection->readable()=="perspective" )
	{
		// perspective. leave the field of view and screen window as is and translate
		// back till the box is wholly visible. this currently assumes the screen window
		// is centred about the camera axis.		
		float z0 = cBox.size().x / screenWindow.size().x;
		float z1 = cBox.size().y / screenWindow.size().y;
				
		m_centreOfInterest = std::max( z0, z1 ) / tan( M_PI * m_fov->readable() / 360.0 ) + cBox.max.z +
			m_clippingPlanes->readable()[0];
		
		cameraMatrix.translate( V3f( 0.0f, 0.0f, m_centreOfInterest ) );
	}
	else
	{
		// orthographic. translate to front of box and set screen window
		// to frame the box, maintaining the aspect ratio of the screen window.
		m_centreOfInterest = cBox.max.z + m_clippingPlanes->readable()[0] + 0.1; // 0.1 is a fudge factor
		cameraMatrix.translate( V3f( 0.0f, 0.0f, m_centreOfInterest ) );
		
		float xScale = cBox.size().x / screenWindow.size().x;
		float yScale = cBox.size().y / screenWindow.size().y;
		float scale = std::max( xScale, yScale );
		
		V2f newSize = screenWindow.size() * scale;
		screenWindow.min.x = cBox.center().x - newSize.x / 2.0f;
		screenWindow.min.y = cBox.center().y - newSize.y / 2.0f;
		screenWindow.max.x = cBox.center().x + newSize.x / 2.0f;
		screenWindow.max.y = cBox.center().y + newSize.y / 2.0f;
	}
	
	m_transform->matrix = cameraMatrix;
	m_screenWindow->writable() = screenWindow;
}

void CameraController::unproject( const Imath::V2i rasterPosition, Imath::V3f &near, Imath::V3f &far )
{
	V2f ndc = V2f( rasterPosition ) / m_resolution->readable();
	const Box2f &screenWindow = m_screenWindow->readable();
	V2f screen(
		lerp( screenWindow.min.x, screenWindow.max.x, ndc.x ),
		lerp( screenWindow.max.y, screenWindow.min.y, ndc.y )
	);
		
	const V2f &clippingPlanes = m_clippingPlanes->readable();
	if( m_projection->readable()=="perspective" )
	{
		float fov = m_fov->readable();
		float d = tan( M_PI * fov/180.0f ); // camera x coordinate at screen window x==1
		V3f camera( screen.x * d, screen.y * d, 1.0f );
		near = camera * clippingPlanes[0];
		far = camera * clippingPlanes[0];
	}
	else
	{
		near = V3f( screen.x, screen.y, clippingPlanes[0] );
		far = V3f( screen.x, screen.y, clippingPlanes[1] );
	}

	near = near * m_transform->matrix;
	far = far * m_transform->matrix;
}

void CameraController::motionStart( MotionType motion, const Imath::V2i &startPosition )
{
	m_motionType = motion;
	m_motionStart = startPosition;
	m_motionMatrix = m_transform->transform();
	m_motionScreenWindow = m_screenWindow->readable();
	m_motionCentreOfInterest = m_centreOfInterest;
}

void CameraController::motionUpdate( const Imath::V2i &newPosition )
{
	switch( m_motionType )
	{
		case Track :
			track( newPosition );
			break;
		case Tumble :
			tumble( newPosition );
			break;
		case Dolly :
			dolly( newPosition );
			break;				
		default :
			throw Exception( "CameraController not in motion." );
	}
}

void CameraController::motionEnd( const Imath::V2i &endPosition )
{
	switch( m_motionType )
	{
		case Track :
			track( endPosition );
			break;
		case Tumble :
			tumble( endPosition );
			break;
		case Dolly :
			dolly( endPosition );
			break;				
		default :
			break;
	}
	m_motionType = None;
}

void CameraController::track( const Imath::V2i &p )
{
	V2i resolution = m_resolution->readable();
	Box2f screenWindow = m_screenWindow->readable();
	
	V2i d = p - m_motionStart;
	V3f translate( 0.0f );
	translate.x = -screenWindow.size().x * (float)d.x/(float)resolution.x;
	translate.y = screenWindow.size().y * (float)d.y/(float)resolution.y;
	if( m_projection->readable()=="perspective" && m_fov )
	{
		translate *= tan( M_PI * m_fov->readable() / 360.0f ) * (float)m_centreOfInterest;
	}
	M44f t = m_motionMatrix;
	t.translate( translate );
	m_transform->matrix = t;
}

void CameraController::tumble( const Imath::V2i &p )
{
	V2i d = p - m_motionStart;
	
	V3f centreOfInterestInWorld = V3f( 0, 0, -m_centreOfInterest ) * m_motionMatrix;
	V3f xAxisInWorld = V3f( 1, 0, 0 );
	m_motionMatrix.multDirMatrix( xAxisInWorld, xAxisInWorld );
	xAxisInWorld.normalize();
	
	M44f t;
	t.translate( -centreOfInterestInWorld );
	
		
		t.rotate( V3f( 0, -d.x / 100.0f, 0 ) );

		M44f xRotate;
		xRotate.setAxisAngle( xAxisInWorld, -d.y / 100.0f );
	
		t = xRotate * t;
	
	t.translate( centreOfInterestInWorld );
	
	m_transform->matrix = m_motionMatrix * t;
}

void CameraController::dolly( const Imath::V2i &p )
{
	V2i resolution = m_resolution->readable();
	V2f dv = V2f( (p - m_motionStart) ) / resolution;
	float d = dv.x - dv.y;
		
	if( m_projection->readable()=="perspective" )
	{
		M44f t = m_motionMatrix;
		d *= 2.5f * m_motionCentreOfInterest; // 2.5 is a magic number that just makes the speed nice
		t.translate( V3f( 0, 0, -d ) );
		m_centreOfInterest = m_motionCentreOfInterest - d;
		m_transform->matrix = t;
	}
	else
	{
		// orthographic
		Box2f screenWindow = m_motionScreenWindow;

		V2f centreNDC = V2f( m_motionStart ) / resolution;
		V2f centre(
			lerp( screenWindow.min.x, screenWindow.max.x, centreNDC.x ),
			lerp( screenWindow.max.y, screenWindow.min.y, centreNDC.y )
		);
		
		float scale = 1.0f - d;
		if( scale > 0.001 )
		{
			screenWindow.min = (screenWindow.min - centre) * scale + centre;
			screenWindow.max = (screenWindow.max - centre) * scale + centre;
			m_screenWindow->writable() = screenWindow;
		}
	}
}

