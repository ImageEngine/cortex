//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013-2015, Image Engine Design Inc. All rights reserved.
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

#include "maya/MCommonRenderSettingsData.h"
#include "maya/MDagPath.h"
#include "maya/MDagModifier.h"
#include "maya/MFnCamera.h"
#include "maya/MFnTransform.h"
#include "maya/MRenderUtil.h"
#include "maya/MSelectionList.h"

#include "IECore/AngleConversion.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"
#include "IECoreScene/Camera.h"
#include "IECoreScene/MatrixTransform.h"

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/ToMayaCameraConverter.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreMaya;

namespace {

// It's awesome that not only does Maya bake this random mm-to-inch conversion into their
// camera, but they use the pre 1959 definition of the inch
const float INCH_TO_MM = 25.400051;

}

ToMayaCameraConverter::Description ToMayaCameraConverter::g_description( IECoreScene::Camera::staticTypeId(), MFn::kCamera );

ToMayaCameraConverter::ToMayaCameraConverter( IECore::ConstObjectPtr object )
: ToMayaObjectConverter( "Converts IECoreScene::Camera objects to Maya cameras.", object )
{
}

bool ToMayaCameraConverter::doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const
{
	ConstCameraPtr camera = IECore::runTimeCast<const Camera>( from );
	if ( !camera )
	{
		IECore::msg( IECore::Msg::Warning, "ToMayaCameraConverter::doConversion",  "The source object is not an IECoreScene::Camera." );
		return false;
	}

	// check if incoming object is a cameraShape itself
	MObject camObj;
	MFnCamera fnCamera;
	if ( fnCamera.hasObj( to ) )
	{
		camObj = to;
	}

	// check if incoming object is a parent of an existing cameraShape
	if ( camObj.isNull() )
	{
		MFnDagNode fnTo( to );
		for ( unsigned i=0; i < fnTo.childCount(); ++i )
		{
			MObject child = fnTo.child( i );
			if ( fnCamera.hasObj( child ) )
			{
				camObj = child;
				break;
			}
		}
	}

	// make a new cameraShape and parent it to the incoming object
	if ( camObj.isNull() )
	{
		if ( !MFnTransform().hasObj( to ) )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaCameraConverter::doConversion",  "Unable to create a camera as a child of the input object." );
			return false;
		}

		MDagModifier dagMod;
		camObj = dagMod.createNode( "camera", to );
		if ( !dagMod.doIt() )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaCameraConverter::doConversion",  "Unable to modify the DAG correctly." );
			dagMod.undoIt();
			return false;
		}
	}

	if ( camObj.isNull() )
	{
		IECore::msg( IECore::Msg::Warning, "ToMayaCameraConverter::doConversion",  "Unable to find or create a camera from the input object." );
		return false;
	}

	MDagPath camDag;
	MFnDagNode( camObj ).getPath( camDag );
	fnCamera.setObject( camDag );

	Imath::V2f clippingPlanes = camera->getClippingPlanes();
	fnCamera.setNearClippingPlane( clippingPlanes[0] );
	fnCamera.setFarClippingPlane( clippingPlanes[1] );

	if ( camera->getProjection() == "perspective" )
	{
		fnCamera.setIsOrtho( false );


		float focalLength = camera->getFocalLength();

		// The factor of 0.1 here means we assume focal length units are in tenths of world units
		// ( ie. focal length in millimeters, world units in centimeters ).  World units may not
		// really be centimeters, but using this as a default ( matching Alembic/USD ) preserves
		// the focal length in the common case where we're not using DOF and don't care about
		// focalLengthWorldScale
		float focalLengthScale = camera->getFocalLengthWorldScale() / 0.1f;

		if( focalLengthScale * focalLength < 2.5f )
		{
			// Maya arbitrarily disallows cameras with focal lengths less than 2.5 in whatever your
			// focal length units are.  If we encounter a camera smaller than this, I guess the best
			// we can do is arbitrarily scale the units of focal length in order to preserve the ratios
			// of focal length and aperture, so that we get the correct projection
			focalLengthScale = 2.5f / focalLength;
		}

		// setting field of view last as some of the other commands alter it
		fnCamera.setFocalLength( std::max( 2.5f, focalLengthScale * focalLength ) );
		fnCamera.setHorizontalFilmAperture( focalLengthScale * camera->getAperture()[0] * ( 1.0f / INCH_TO_MM ) );
		fnCamera.setVerticalFilmAperture( focalLengthScale * camera->getAperture()[1] * ( 1.0f / INCH_TO_MM ) );
		fnCamera.setHorizontalFilmOffset( focalLengthScale * camera->getApertureOffset()[0] * ( 1.0f / INCH_TO_MM ) );
		fnCamera.setVerticalFilmOffset( focalLengthScale * camera->getApertureOffset()[1] * ( 1.0f / INCH_TO_MM ) );
	}
	else
	{
		fnCamera.setIsOrtho( true );

		// Maya appears to only support a square aperture for ortho cameras.
		// This means that if the stored aperture is rectangular, and you perform a vertical fit,
		// you will will get an incorrect result.  Doesn't look like I can do anything about this.
		fnCamera.setOrthoWidth( camera->getAperture()[0] );

		// Maya doesn't support aperture offsets on ortho cameras.  In theory, we could try to translate
		// into a transform on the camera, but this would make things more complicated
	}

	if( camera->getFStop() != 0.0f )
	{
		fnCamera.setDepthOfField( true );
		fnCamera.setFStop( std::max( 1.0f, camera->getFStop() ) );
	}

	fnCamera.setFocusDistance( camera->getFocusDistance() );

	return true;
}
