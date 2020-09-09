//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2020, Cinesite VFX Ltd. All rights reserved.
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

#include "IECoreUSD/DataAlgo.h"
#include "IECoreUSD/ObjectAlgo.h"

#include "IECoreScene/Camera.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "pxr/usd/usdGeom/camera.h"
IECORE_POP_DEFAULT_VISIBILITY

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreUSD;

//////////////////////////////////////////////////////////////////////////
// Writing
//////////////////////////////////////////////////////////////////////////

namespace
{

bool writeCamera( const IECoreScene::Camera *camera, const pxr::UsdStagePtr &stage, const pxr::SdfPath &path, pxr::UsdTimeCode time )
{
	auto usdCamera = pxr::UsdGeomCamera::Define( stage, path );
	if( camera->getProjection() == "orthographic" )
	{
		usdCamera.GetProjectionAttr().Set( pxr::TfToken( "orthographic" ) );

		// For ortho cameras, USD uses aperture units of tenths of scene units
		usdCamera.GetHorizontalApertureAttr().Set( 10.0f * camera->getAperture()[0] );
		usdCamera.GetVerticalApertureAttr().Set( 10.0f * camera->getAperture()[1] );
		usdCamera.GetHorizontalApertureOffsetAttr().Set( 10.0f * camera->getApertureOffset()[0] );
		usdCamera.GetVerticalApertureOffsetAttr().Set( 10.0f * camera->getApertureOffset()[1] );
	}
	else if( camera->getProjection() == "perspective" )
	{
		usdCamera.GetProjectionAttr().Set( pxr::TfToken( "perspective" ) );

		// We store focalLength and aperture in arbitary units.  USD uses tenths
		// of scene units
		float scale = 10.0f * camera->getFocalLengthWorldScale();

		usdCamera.GetFocalLengthAttr().Set( camera->getFocalLength() * scale );
		usdCamera.GetHorizontalApertureAttr().Set( camera->getAperture()[0] * scale );
		usdCamera.GetVerticalApertureAttr().Set( camera->getAperture()[1] * scale );
		usdCamera.GetHorizontalApertureOffsetAttr().Set( camera->getApertureOffset()[0] * scale );
		usdCamera.GetVerticalApertureOffsetAttr().Set( camera->getApertureOffset()[1] * scale );
	}
	else
	{
		IECore::msg( IECore::Msg::Warning, "IECoreUSD::CameraAlgo", boost::format( "Unsupported projection \"%1%\"" ) % camera->getProjection() );
	}

	usdCamera.GetClippingRangeAttr().Set( pxr::GfVec2f( camera->getClippingPlanes().getValue() ) );
	usdCamera.GetFStopAttr().Set( camera->getFStop() );
	usdCamera.GetFocusDistanceAttr().Set( camera->getFocusDistance() );
	usdCamera.GetShutterOpenAttr().Set( (double)camera->getShutter()[0] );
	usdCamera.GetShutterCloseAttr().Set( (double)camera->getShutter()[1] );

	return true;
}

ObjectAlgo::WriterDescription<Camera> g_cameraWriterDescription( writeCamera );

} // namespace