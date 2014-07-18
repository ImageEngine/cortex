//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2013, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2011-2012, John Haddon. All rights reserved.
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
#include "IECore/AngleConversion.h"

#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathMatrixAlgo.h"
#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathBoxAlgo.h"

using namespace IECore;
using namespace Imath;

class CameraController::MemberData : public IECore::RefCounted
{

	public :

		// parts of the camera we manipulate
		CameraPtr camera;
		V2iDataPtr resolution;
		Box2fDataPtr screenWindow;
		MatrixTransformPtr transform;
		ConstStringDataPtr projection;
		ConstFloatDataPtr fov;
		ConstV2fDataPtr clippingPlanes;
		float centreOfInterest;

		// motion state
		MotionType motionType;
		Imath::V2f motionStart;
		Imath::M44f motionMatrix;
		float motionCentreOfInterest;
		Imath::Box2f motionScreenWindow;
		
};
		
CameraController::CameraController( CameraPtr camera )
	:	m_data( new MemberData )
{
	setCamera( camera );
}

CameraController::~CameraController()
{
}

void CameraController::setCamera( CameraPtr camera )
{
	m_data->camera = camera;
	m_data->camera->addStandardParameters(); // subsequent casts are safe because of this
	m_data->resolution = boost::static_pointer_cast<V2iData>( m_data->camera->parameters()["resolution"] );
	m_data->screenWindow = boost::static_pointer_cast<Box2fData>( m_data->camera->parameters()["screenWindow"] );
	m_data->clippingPlanes = boost::static_pointer_cast<V2fData>( m_data->camera->parameters()["clippingPlanes"] );
	m_data->projection = boost::static_pointer_cast<StringData>( m_data->camera->parameters()["projection"] );
	if( m_data->projection->readable()=="perspective" )
	{
		m_data->fov = boost::static_pointer_cast<FloatData>( m_data->camera->parameters()["projection:fov"] );
	}
	else
	{
		m_data->fov = 0;
	}
	
	TransformPtr transform = m_data->camera->getTransform();
	m_data->transform = runTimeCast<MatrixTransform>( transform );
	if( !m_data->transform )
	{
		m_data->transform = new MatrixTransform( transform ? transform->transform() : M44f() );
		m_data->camera->setTransform( m_data->transform );
	}

	m_data->centreOfInterest = 1;
}

CameraPtr CameraController::getCamera()
{
	return m_data->camera;
}

ConstCameraPtr CameraController::getCamera() const
{
	return m_data->camera;
}

void CameraController::setCentreOfInterest( float centreOfInterest )
{
	m_data->centreOfInterest = centreOfInterest;
}

float CameraController::getCentreOfInterest()
{
	return m_data->centreOfInterest;
}

void CameraController::setResolution( const Imath::V2i &resolution )
{
	setResolution( resolution, ScaleScreenWindow );
}

void CameraController::setResolution( const Imath::V2i &resolution, ScreenWindowAdjustment adjustment )
{
	const V2i oldResolution = m_data->resolution->readable();
	const Box2f oldScreenWindow = m_data->screenWindow->readable();
	
	m_data->resolution->writable() = resolution;

	Box2f newScreenWindow;
	if( adjustment == ScaleScreenWindow )
	{
		const float oldAspect = (float)oldResolution.x/(float)oldResolution.y;
		const float badAspect = (float)resolution.x/(float)resolution.y;
		const float yScale = oldAspect / badAspect;

		newScreenWindow = oldScreenWindow;
		newScreenWindow.min.y *= yScale;
		newScreenWindow.max.y *= yScale;
	}
	else
	{
		const V2f screenWindowCenter = oldScreenWindow.center();
		const V2f scale = V2f( resolution ) / V2f( oldResolution );
		newScreenWindow.min = screenWindowCenter + (oldScreenWindow.min - screenWindowCenter) * scale;
		newScreenWindow.max = screenWindowCenter + (oldScreenWindow.max - screenWindowCenter) * scale;
	}
	
	m_data->screenWindow->writable() = newScreenWindow;
}

const Imath::V2i &CameraController::getResolution() const
{
	return m_data->resolution->readable();
}

void CameraController::frame( const Imath::Box3f &box )
{
	V3f z( 0, 0, -1 );
	V3f y( 0, 1, 0 );
	M44f t = m_data->transform->matrix;
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

	Box2f screenWindow = m_data->screenWindow->readable();
	if( m_data->projection->readable()=="perspective" )
	{
		// perspective. leave the field of view and screen window as is and translate
		// back till the box is wholly visible. this currently assumes the screen window
		// is centred about the camera axis.
		float z0 = cBox.size().x / screenWindow.size().x;
		float z1 = cBox.size().y / screenWindow.size().y;

		m_data->centreOfInterest = std::max( z0, z1 ) / tan( M_PI * m_data->fov->readable() / 360.0 ) + cBox.max.z +
			m_data->clippingPlanes->readable()[0];

		cameraMatrix.translate( V3f( 0.0f, 0.0f, m_data->centreOfInterest ) );
	}
	else
	{
		// orthographic. translate to front of box and set screen window
		// to frame the box, maintaining the aspect ratio of the screen window.
		m_data->centreOfInterest = cBox.max.z + m_data->clippingPlanes->readable()[0] + 0.1; // 0.1 is a fudge factor
		cameraMatrix.translate( V3f( 0.0f, 0.0f, m_data->centreOfInterest ) );

		float xScale = cBox.size().x / screenWindow.size().x;
		float yScale = cBox.size().y / screenWindow.size().y;
		float scale = std::max( xScale, yScale );

		V2f newSize = screenWindow.size() * scale;
		screenWindow.min.x = cBox.center().x - newSize.x / 2.0f;
		screenWindow.min.y = cBox.center().y - newSize.y / 2.0f;
		screenWindow.max.x = cBox.center().x + newSize.x / 2.0f;
		screenWindow.max.y = cBox.center().y + newSize.y / 2.0f;
	}

	m_data->transform->matrix = cameraMatrix;
	m_data->screenWindow->writable() = screenWindow;
}

void CameraController::unproject( const Imath::V2f rasterPosition, Imath::V3f &near, Imath::V3f &far )
{
	V2f ndc = V2f( rasterPosition ) / m_data->resolution->readable();
	const Box2f &screenWindow = m_data->screenWindow->readable();
	V2f screen(
		lerp( screenWindow.min.x, screenWindow.max.x, ndc.x ),
		lerp( screenWindow.max.y, screenWindow.min.y, ndc.y )
	);

	const V2f &clippingPlanes = m_data->clippingPlanes->readable();
	if( m_data->projection->readable()=="perspective" )
	{
		float fov = m_data->fov->readable();
		float d = tan( degreesToRadians( fov / 2.0f ) ); // camera x coordinate at screen window x==1
		V3f camera( screen.x * d, screen.y * d, -1.0f );
		near = camera * clippingPlanes[0];
		far = camera * clippingPlanes[1];
	}
	else
	{
		near = V3f( screen.x, screen.y, -clippingPlanes[0] );
		far = V3f( screen.x, screen.y, -clippingPlanes[1] );
	}

	near = near * m_data->transform->matrix;
	far = far * m_data->transform->matrix;
}

Imath::V2f CameraController::project( const Imath::V3f &worldPosition ) const
{
	M44f inverseCameraMatrix = m_data->transform->matrix.inverse();
	V3f cameraPosition = worldPosition * inverseCameraMatrix;
	
	const V2i &resolution = m_data->resolution->readable();
	const Box2f &screenWindow = m_data->screenWindow->readable();
	if( m_data->projection->readable() == "perspective" )
	{
		V3f screenPosition = cameraPosition / cameraPosition.z;
		float fov = m_data->fov->readable();
		float d = tan( degreesToRadians( fov / 2.0f ) ); // camera x coordinate at screen window x==1
		screenPosition /= d;
		V2f ndcPosition(
			lerpfactor( screenPosition.x, screenWindow.max.x, screenWindow.min.x ),
			lerpfactor( screenPosition.y, screenWindow.min.y, screenWindow.max.y )	
		);
		return V2f(
			ndcPosition.x * resolution.x,
			ndcPosition.y * resolution.y
		);
	}
	else
	{
		V2f ndcPosition(
			lerpfactor( cameraPosition.x, screenWindow.min.x, screenWindow.max.x ),
			lerpfactor( cameraPosition.y, screenWindow.max.y, screenWindow.min.y )	
		);
		return V2f(
			ndcPosition.x * resolution.x,
			ndcPosition.y * resolution.y
		);
	}
}

void CameraController::motionStart( MotionType motion, const Imath::V2f &startPosition )
{
	m_data->motionType = motion;
	m_data->motionStart = startPosition;
	m_data->motionMatrix = m_data->transform->transform();
	m_data->motionScreenWindow = m_data->screenWindow->readable();
	m_data->motionCentreOfInterest = m_data->centreOfInterest;
}

void CameraController::motionUpdate( const Imath::V2f &newPosition )
{
	switch( m_data->motionType )
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

void CameraController::motionEnd( const Imath::V2f &endPosition )
{
	switch( m_data->motionType )
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
	m_data->motionType = None;
}

void CameraController::track( const Imath::V2f &p )
{
	V2i resolution = m_data->resolution->readable();
	Box2f screenWindow = m_data->screenWindow->readable();

	V2f d = p - m_data->motionStart;
	V3f translate( 0.0f );
	translate.x = -screenWindow.size().x * d.x/(float)resolution.x;
	translate.y = screenWindow.size().y * d.y/(float)resolution.y;
	if( m_data->projection->readable()=="perspective" && m_data->fov )
	{
		translate *= tan( M_PI * m_data->fov->readable() / 360.0f ) * (float)m_data->centreOfInterest;
	}
	M44f t = m_data->motionMatrix;
	t.translate( translate );
	m_data->transform->matrix = t;
}

void CameraController::tumble( const Imath::V2f &p )
{
	V2f d = p - m_data->motionStart;

	V3f centreOfInterestInWorld = V3f( 0, 0, -m_data->centreOfInterest ) * m_data->motionMatrix;
	V3f xAxisInWorld = V3f( 1, 0, 0 );
	m_data->motionMatrix.multDirMatrix( xAxisInWorld, xAxisInWorld );
	xAxisInWorld.normalize();

	M44f t;
	t.translate( centreOfInterestInWorld );

		t.rotate( V3f( 0, -d.x / 100.0f, 0 ) );

		M44f xRotate;
		xRotate.setAxisAngle( xAxisInWorld, -d.y / 100.0f );

		t = xRotate * t;

	t.translate( -centreOfInterestInWorld );

	m_data->transform->matrix = m_data->motionMatrix * t;
}

void CameraController::dolly( const Imath::V2f &p )
{
	V2i resolution = m_data->resolution->readable();
	V2f dv = V2f( (p - m_data->motionStart) ) / resolution;
	float d = dv.x - dv.y;

	if( m_data->projection->readable()=="perspective" )
	{
		// perspective
		m_data->centreOfInterest = m_data->motionCentreOfInterest * expf( -1.9f * d );
		
		M44f t = m_data->motionMatrix;
		t.translate( V3f( 0, 0, m_data->centreOfInterest - m_data->motionCentreOfInterest ) );
		
		m_data->transform->matrix = t;
	}
	else
	{
		// orthographic
		Box2f screenWindow = m_data->motionScreenWindow;

		V2f centreNDC = V2f( m_data->motionStart ) / resolution;
		V2f centre(
			lerp( screenWindow.min.x, screenWindow.max.x, centreNDC.x ),
			lerp( screenWindow.max.y, screenWindow.min.y, centreNDC.y )
		);

		float newWidth = m_data->motionScreenWindow.size().x * expf( -1.9f * d );
		newWidth = std::max( newWidth, 0.01f );

		float scale = newWidth / screenWindow.size().x;
		
		screenWindow.min = (screenWindow.min - centre) * scale + centre;
		screenWindow.max = (screenWindow.max - centre) * scale + centre;
		m_data->screenWindow->writable() = screenWindow;
	}	
}

