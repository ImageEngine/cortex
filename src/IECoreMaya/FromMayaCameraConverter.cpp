//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2015, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/FromMayaCameraConverter.h"
#include "IECoreMaya/Convert.h"

#include "IECore/CompoundParameter.h"
#include "IECore/AngleConversion.h"
#include "IECoreScene/Camera.h"
#include "IECoreScene/MatrixTransform.h"

#include "maya/MFnCamera.h"
#include "maya/MFnEnumAttribute.h"
#include "maya/MString.h"
#include "maya/MRenderUtil.h"
#include "maya/MCommonRenderSettingsData.h"
#include "maya/MDagPath.h"
#include "maya/MPlug.h"

#include "OpenEXR/ImathMath.h"

using namespace IECoreMaya;
using namespace IECore;
using namespace IECoreScene;
using namespace Imath;

namespace {

// It's awesome that not only does Maya bake this random mm-to-inch conversion into their
// camera, but they use the pre 1959 definition of the inch
const float INCH_TO_MM = 25.400051;

}

IE_CORE_DEFINERUNTIMETYPED( FromMayaCameraConverter );

FromMayaDagNodeConverter::Description<FromMayaCameraConverter> FromMayaCameraConverter::m_description( MFn::kCamera, Camera::staticTypeId(), true );

FromMayaCameraConverter::FromMayaCameraConverter( const MDagPath &dagPath )
	:	FromMayaDagNodeConverter( "Converts maya camera shape nodes into IECoreScene::Camera objects.", dagPath )
{
}

IECore::ObjectPtr FromMayaCameraConverter::doConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnCamera fnCamera( dagPath );

	// convert things that are required by the IECore::Renderer specification

	CameraPtr result = new Camera;

	if( fnCamera.hasAttribute( "ieCamera_overrideResolution" ) )
	{
		MStatus success1, success2;
		MPlug resPlug( fnCamera.object(), fnCamera.attribute( "ieCamera_overrideResolution" ));
		if( resPlug.numChildren() == 2 )
		{
#if MAYA_API_VERSION >= 20180000
			int x = resPlug.child(0).asInt( &success1 );
			int y = resPlug.child(1).asInt( &success2 );
#else
			int x = resPlug.child(0).asInt( MDGContext::fsNormal, &success1 );
			int y = resPlug.child(1).asInt( MDGContext::fsNormal, &success2 );
#endif
			if( success1 == MS::kSuccess && success2 == MS::kSuccess )
			{
				result->setResolution( Imath::V2i( x, y ) );
			}
		}
	}

	if( fnCamera.hasAttribute( "ieCamera_overridePixelAspectRatio" ) )
	{
		MStatus success;
		MPlug overridePixelAspectRatioPlug( fnCamera.object(), fnCamera.attribute( "ieCamera_overridePixelAspectRatio" ) );
#if MAYA_API_VERSION >= 20180000
		float overridePixelAspectRatio = overridePixelAspectRatioPlug.asFloat( &success );
#else
		float overridePixelAspectRatio = overridePixelAspectRatioPlug.asFloat( MDGContext::fsNormal, &success );
#endif

		if( success == MS::kSuccess )
		{
			result->setPixelAspectRatio( overridePixelAspectRatio );
		}
	}

	if( fnCamera.hasAttribute( "ieCamera_overrideFilmFit" ) )
	{
		MStatus success;
		MPlug overrideFilmFitPlug( fnCamera.object(), fnCamera.attribute( "ieCamera_overrideFilmFit" ) );
		MFnEnumAttribute enumAttr( overrideFilmFitPlug.attribute(), &success );
		if( success == MS::kSuccess )
		{
#if MAYA_API_VERSION >= 20180000
			MString overrideFilmFit = enumAttr.fieldName( overrideFilmFitPlug.asInt( &success) );
#else
			MString overrideFilmFit = enumAttr.fieldName( overrideFilmFitPlug.asInt( MDGContext::fsNormal, &success) );
#endif
			if( success == MS::kSuccess )
			{
				if( overrideFilmFit == "Horizontal" )
				{
					result->setFilmFit( Camera::FilmFit::Horizontal );
				}
				else if( overrideFilmFit == "Vertical" )
				{
					result->setFilmFit( Camera::FilmFit::Vertical );
				}
				else if( overrideFilmFit == "Fit" )
				{
					result->setFilmFit( Camera::FilmFit::Fit );
				}
				else if( overrideFilmFit == "Fill" )
				{
					result->setFilmFit( Camera::FilmFit::Fill );
				}
				else if( overrideFilmFit == "Distort" )
				{
					result->setFilmFit( Camera::FilmFit::Distort );
				}
			}
		}
	}

	result->setClippingPlanes( Imath::V2f( fnCamera.nearClippingPlane(), fnCamera.farClippingPlane() ) );

	if( fnCamera.isOrtho() )
	{
		// orthographic
		result->setProjection( "orthographic" );
		result->setAperture( V2f( fnCamera.orthoWidth() ) );
	}
	else
	{
		// perspective
		result->setProjection( "perspective" );

		result->setAperture( INCH_TO_MM * V2f( fnCamera.horizontalFilmAperture(), fnCamera.verticalFilmAperture() ) );
		result->setApertureOffset( INCH_TO_MM * V2f( fnCamera.horizontalFilmOffset(), fnCamera.verticalFilmOffset() ) );
		result->setFocalLength( fnCamera.focalLength() );
	}
	if( fnCamera.isDepthOfField() )
	{
		result->setFStop( fnCamera.fStop() );
	}
	result->setFocusDistance( fnCamera.focusDistance() );

	return result;
}
