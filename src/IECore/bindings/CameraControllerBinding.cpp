//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECore/bindings/CameraControllerBinding.h"
#include "IECore/CameraController.h"
#include "IECore/MatrixTransform.h"
#include "IECore/Camera.h"

using namespace boost::python;

namespace IECore
{

static tuple unproject( CameraController &c, Imath::V2i &p )
{
	Imath::V3f near, far;
	c.unproject( p, near, far );
	return make_tuple( near, far );
}

void bindCameraController()
{

	scope s = class_<CameraController>( "CameraController", init<CameraPtr>() )
		.def( "setCamera", &CameraController::setCamera )
		.def( "getCamera", &CameraController::getCamera )
		.def( "setCentreOfInterest", &CameraController::setCentreOfInterest )
		.def( "getCentreOfInterest", &CameraController::getCentreOfInterest )
		.def( "setResolution", &CameraController::setResolution )
		.def( "getResolution", &CameraController::getResolution, return_value_policy<copy_const_reference>() )
		.def( "frame", (void (CameraController::*)( const Imath::Box3f & ))&CameraController::frame )
		.def( "frame", (void (CameraController::*)( const Imath::Box3f &, const Imath::V3f &, const Imath::V3f & ))&CameraController::frame )
		.def( "unproject", &unproject )
		.def( "motionStart", &CameraController::motionStart )
		.def( "motionUpdate", &CameraController::motionUpdate )
		.def( "motionEnd", &CameraController::motionEnd )
	;

	enum_<CameraController::MotionType>( "MotionType" )
		.value( "None", CameraController::None )
		.value( "Track", CameraController::Track )
		.value( "Tumble", CameraController::Tumble )
		.value( "Dolly", CameraController::Dolly )
	;

}

}
