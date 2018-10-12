//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreNuke/FromNukeCameraConverter.h"

#include "IECoreNuke/Convert.h"

#include "IECoreScene/Camera.h"
#include "IECoreScene/MatrixTransform.h"

#include "IECore/AngleConversion.h"
#include "IECore/CompoundParameter.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

using namespace IECoreNuke;
using namespace IECore;
using namespace IECoreScene;
using namespace Imath;

FromNukeCameraConverter::FromNukeCameraConverter( const DD::Image::CameraOp *camera )
	:	FromNukeConverter( "Converts nuke cameras to IECore cameras." ), m_camera( camera )
{
}

FromNukeCameraConverter::~FromNukeCameraConverter()
{
}

IECore::ObjectPtr FromNukeCameraConverter::doConversion( IECore::ConstCompoundObjectPtr operands ) const
{
	CameraPtr result = new IECoreScene::Camera();

	// clipping planes
	result->setClippingPlanes( V2f( m_camera->Near(), m_camera->Far() ) );

	// projection
	switch( m_camera->projection_mode() )
	{
		case DD::Image::CameraOp::LENS_PERSPECTIVE :
		{
			result->setProjection( "perspective" );
			result->setFocalLength( m_camera->focal_length() );
			break;
		}
		case DD::Image::CameraOp::LENS_UV :
		case DD::Image::CameraOp::LENS_SPHERICAL :
		case DD::Image::CameraOp::LENS_RENDER_CAMERA :
			msg( Msg::Warning, "FromNukeCameraConverter::doConversion", "Unsupported projection type - reverting to orthographic" );
			// fall through to orthographic code
		case DD::Image::CameraOp::LENS_ORTHOGRAPHIC :
		{
			result->setProjection( "orthographic" );
			break;
		}
	}

	// TODO - I haven't tested any of this - I'm not sure how to use it, because it doesn't appear to
	// be used anywhere.  Why does it even exist?
	V2f screenWindowScale = IECore::convert<Imath::V2f>( m_camera->win_scale() );
	V2f screenWindowTranslate = IECore::convert<Imath::V2f>( m_camera->win_scale() );
	result->setAperture( V2f( m_camera->film_width(), m_camera->film_height() ) * screenWindowScale );
	result->setApertureOffset( screenWindowTranslate );

	// we don't currently support window roll
	if( m_camera->win_roll() != 0.0 )
	{
		msg( Msg::Warning, "FromNukeCameraConverter::doConversion", "Window roll is not supported" );
	}

	return result;
}
