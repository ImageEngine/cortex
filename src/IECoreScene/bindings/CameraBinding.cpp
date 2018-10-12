//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

// This include needs to be the very first to prevent problems with warnings
// regarding redefinition of _POSIX_C_SOURCE
#include "boost/python.hpp"

#include "CameraBinding.h"

#include "IECoreScene/Camera.h"

#include "IECorePython/RunTimeTypedBinding.h"

using namespace boost::python;
using boost::python::arg;
using namespace IECore;
using namespace IECorePython;
using namespace IECoreScene;

namespace IECoreSceneModule
{

void bindCamera()
{
	RunTimeTypedClass<Camera> cameraClass;

	{
		// then define all the nested types
		scope s( cameraClass );

		enum_< Camera::FilmFit > ("FilmFit")
			.value("Horizontal", Camera::Horizontal)
			.value("Vertical", Camera::Vertical)
			.value("Fit", Camera::Fit)
			.value("Fill", Camera::Fill)
			.value("Distort", Camera::Distort)
			.export_values()
		;
	}

	cameraClass.def( init< optional< CompoundDataPtr > >
			(
				(
					/// We need to explicitly make this a CompoundData::Ptr so that boost.python finds the correct to_python converter
					arg( "parameters" ) = CompoundData::Ptr( new CompoundData() )
				)
			)
		)
		.def( "parameters", (CompoundData *(Camera::*)())&Camera::parametersData, return_value_policy<CastToIntrusivePtr>() )
		.def( "setProjection", &Camera::setProjection )
		.def( "getProjection", &Camera::getProjection )
		.def( "setAperture", &Camera::setAperture )
		.def( "getAperture", &Camera::getAperture )
		.def( "setApertureOffset", &Camera::setApertureOffset )
		.def( "getApertureOffset", &Camera::getApertureOffset )
		.def( "setFocalLength", &Camera::setFocalLength )
		.def( "getFocalLength", &Camera::getFocalLength )
		.def( "setClippingPlanes", &Camera::setClippingPlanes )
		.def( "getClippingPlanes", &Camera::getClippingPlanes )
		.def( "setFStop", &Camera::setFStop )
		.def( "getFStop", &Camera::getFStop )
		.def( "setFocalLengthWorldScale", &Camera::setFocalLengthWorldScale )
		.def( "getFocalLengthWorldScale", &Camera::getFocalLengthWorldScale )
		.def( "setFocusDistance", &Camera::setFocusDistance )
		.def( "getFocusDistance", &Camera::getFocusDistance )

		.def( "hasFilmFit", &Camera::hasFilmFit )
		.def( "setFilmFit", &Camera::setFilmFit )
		.def( "getFilmFit", &Camera::getFilmFit )
		.def( "removeFilmFit", &Camera::removeFilmFit )

		.def( "hasResolution", &Camera::hasResolution )
		.def( "setResolution", &Camera::setResolution )
		.def( "getResolution", &Camera::getResolution )
		.def( "removeResolution", &Camera::removeResolution )

		.def( "hasPixelAspectRatio", &Camera::hasPixelAspectRatio )
		.def( "setPixelAspectRatio", &Camera::setPixelAspectRatio )
		.def( "getPixelAspectRatio", &Camera::getPixelAspectRatio )
		.def( "removePixelAspectRatio", &Camera::removePixelAspectRatio )

		.def( "hasResolutionMultiplier", &Camera::hasResolutionMultiplier )
		.def( "setResolutionMultiplier", &Camera::setResolutionMultiplier )
		.def( "getResolutionMultiplier", &Camera::getResolutionMultiplier )
		.def( "removeResolutionMultiplier", &Camera::removeResolutionMultiplier )

		.def( "hasOverscan", &Camera::hasOverscan )
		.def( "setOverscan", &Camera::setOverscan )
		.def( "getOverscan", &Camera::getOverscan )
		.def( "removeOverscan", &Camera::removeOverscan )

		.def( "hasOverscanLeft", &Camera::hasOverscanLeft )
		.def( "setOverscanLeft", &Camera::setOverscanLeft )
		.def( "getOverscanLeft", &Camera::getOverscanLeft )
		.def( "removeOverscanLeft", &Camera::removeOverscanLeft )

		.def( "hasOverscanRight", &Camera::hasOverscanRight )
		.def( "setOverscanRight", &Camera::setOverscanRight )
		.def( "getOverscanRight", &Camera::getOverscanRight )
		.def( "removeOverscanRight", &Camera::removeOverscanRight )

		.def( "hasOverscanTop", &Camera::hasOverscanTop )
		.def( "setOverscanTop", &Camera::setOverscanTop )
		.def( "getOverscanTop", &Camera::getOverscanTop )
		.def( "removeOverscanTop", &Camera::removeOverscanTop )

		.def( "hasOverscanBottom", &Camera::hasOverscanBottom )
		.def( "setOverscanBottom", &Camera::setOverscanBottom )
		.def( "getOverscanBottom", &Camera::getOverscanBottom )
		.def( "removeOverscanBottom", &Camera::removeOverscanBottom )

		.def( "hasCropWindow", &Camera::hasCropWindow )
		.def( "setCropWindow", &Camera::setCropWindow )
		.def( "getCropWindow", &Camera::getCropWindow )
		.def( "removeCropWindow", &Camera::removeCropWindow )

		.def( "hasShutter", &Camera::hasShutter )
		.def( "setShutter", &Camera::setShutter )
		.def( "getShutter", &Camera::getShutter )
		.def( "removeShutter", &Camera::removeShutter )

		.def( "fitWindow", &Camera::fitWindow ).staticmethod( "fitWindow" )
		.def<Imath::Box2f (Camera::*)() const>( "frustum", &Camera::frustum )
		.def<Imath::Box2f (Camera::*)(Camera::FilmFit) const>( "frustum", &Camera::frustum )
		.def<Imath::Box2f (Camera::*)(Camera::FilmFit, float) const>( "frustum", &Camera::frustum )
		.def( "renderResolution", &Camera::renderResolution )
		.def( "renderRegion", &Camera::renderRegion )

		.def( "calculateFieldOfView", &Camera::calculateFieldOfView )
		.def( "setFocalLengthFromFieldOfView", &Camera::setFocalLengthFromFieldOfView )
	;
}

}
