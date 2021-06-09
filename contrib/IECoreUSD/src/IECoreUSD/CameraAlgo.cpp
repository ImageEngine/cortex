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
// Reading
//////////////////////////////////////////////////////////////////////////

namespace
{

IECore::ObjectPtr readCamera( pxr::UsdGeomCamera &camera, pxr::UsdTimeCode time, const Canceller *canceller )
{
	CameraPtr result = new Camera;

	pxr::TfToken projection;
	camera.GetProjectionAttr().Get( &projection, time );
	result->setProjection( projection.GetString() );

	Imath::V2f aperture;
	camera.GetHorizontalApertureAttr().Get( &aperture.x, time );
	camera.GetVerticalApertureAttr().Get( &aperture.y, time );
	Imath::V2f apertureOffset;
	camera.GetHorizontalApertureOffsetAttr().Get( &apertureOffset.x, time );
	camera.GetVerticalApertureOffsetAttr().Get( &apertureOffset.y, time );

	if( projection == pxr::UsdGeomTokens->orthographic )
	{
		// USD uses tenths of world units, we use world units.
		result->setAperture( aperture / 10.0f );
		result->setApertureOffset( apertureOffset / 10.0f );
	}
	else if( projection == pxr::UsdGeomTokens->perspective )
	{
		// USD specifies focal length in tenths of world units.
		result->setFocalLengthWorldScale( 0.1 );
		result->setAperture( aperture );
		result->setApertureOffset( apertureOffset );

		float focalLength;
		camera.GetFocalLengthAttr().Get( &focalLength, time );
		result->setFocalLength( focalLength );
	}
	else
	{
		IECore::msg(
			IECore::Msg::Warning, "IECoreUSD::CameraAlgo",
			boost::format( "Unsupported projection \"%1%\" reading \"%2%\" at time %3%" )
				% projection
				% camera.GetPrim().GetPath()
				% ( time.GetValue() / camera.GetPrim().GetStage()->GetTimeCodesPerSecond() )
		);
	}

	pxr::GfVec2f clippingRange;
	camera.GetClippingRangeAttr().Get( &clippingRange, time );
	result->setClippingPlanes( DataAlgo::fromUSD( clippingRange ) );

	float fStop;
	camera.GetFStopAttr().Get( &fStop, time );
	result->setFStop( fStop );

	float focusDistance;
	camera.GetFocusDistanceAttr().Get( &focusDistance, time );
	result->setFocusDistance( focusDistance );

	Imath::V2d shutter;
	camera.GetShutterOpenAttr().Get( &shutter[0], time );
	camera.GetShutterCloseAttr().Get( &shutter[1], time );
	result->setShutter( shutter );

	return result;
}

bool cameraMightBeTimeVarying( pxr::UsdGeomCamera &camera )
{
	return
		camera.GetProjectionAttr().ValueMightBeTimeVarying() ||
		camera.GetHorizontalApertureAttr().ValueMightBeTimeVarying() ||
		camera.GetVerticalApertureAttr().ValueMightBeTimeVarying() ||
		camera.GetHorizontalApertureOffsetAttr().ValueMightBeTimeVarying() ||
		camera.GetVerticalApertureOffsetAttr().ValueMightBeTimeVarying() ||
		camera.GetFocalLengthAttr().ValueMightBeTimeVarying() ||
		camera.GetClippingRangeAttr().ValueMightBeTimeVarying() ||
		camera.GetFStopAttr().ValueMightBeTimeVarying() ||
		camera.GetFocusDistanceAttr().ValueMightBeTimeVarying() ||
		camera.GetShutterOpenAttr().ValueMightBeTimeVarying() ||
		camera.GetShutterCloseAttr().ValueMightBeTimeVarying()
	;
}

ObjectAlgo::ReaderDescription<pxr::UsdGeomCamera> g_cameraReaderDescription( pxr::TfToken( "Camera" ), readCamera, cameraMightBeTimeVarying );

} // namespace

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
		usdCamera.GetProjectionAttr().Set( pxr::UsdGeomTokens->orthographic );

		// For ortho cameras, USD uses aperture units of tenths of scene units
		usdCamera.GetHorizontalApertureAttr().Set( 10.0f * camera->getAperture()[0] );
		usdCamera.GetVerticalApertureAttr().Set( 10.0f * camera->getAperture()[1] );
		usdCamera.GetHorizontalApertureOffsetAttr().Set( 10.0f * camera->getApertureOffset()[0] );
		usdCamera.GetVerticalApertureOffsetAttr().Set( 10.0f * camera->getApertureOffset()[1] );
	}
	else if( camera->getProjection() == "perspective" )
	{
		usdCamera.GetProjectionAttr().Set( pxr::UsdGeomTokens->perspective );

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
		IECore::msg(
			IECore::Msg::Warning, "IECoreUSD::CameraAlgo",
			boost::format( "Unsupported projection \"%1%\" writing \"%2%\" at time %3%" )
				% camera->getProjection()
				% path
				% ( time.GetValue() / stage->GetTimeCodesPerSecond() )
		);
	}

	usdCamera.GetClippingRangeAttr().Set( pxr::GfVec2f( camera->getClippingPlanes().getValue() ) );
	usdCamera.GetFStopAttr().Set( camera->getFStop() );
	usdCamera.GetFocusDistanceAttr().Set( camera->getFocusDistance() );

	/// \todo This is documented as being specified in UsdTimeCode units,
	/// in which case I think we should be converting from seconds using
	/// `stage->GetTimeCodesPerSecond()`. Having looked at both the Maya
	/// and Houdini plugin sources, I've been unable to find evidence for
	/// anyone else doing this though, so maybe it's one of those things
	/// everyone is just getting wrong?
	usdCamera.GetShutterOpenAttr().Set( (double)camera->getShutter()[0] );
	usdCamera.GetShutterCloseAttr().Set( (double)camera->getShutter()[1] );

	return true;
}

ObjectAlgo::WriterDescription<Camera> g_cameraWriterDescription( writeCamera );

} // namespace
