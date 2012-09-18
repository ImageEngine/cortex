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

#include <boost/python.hpp>

#include "IECoreGL/Camera.h"
#include "IECoreGL/bindings/CameraBinding.h"

#include "IECorePython/RunTimeTypedBinding.h"

using namespace boost::python;

namespace IECoreGL
{

void bindCamera()
{
	IECorePython::RunTimeTypedClass<Camera>()
		.def( "setTransform", &Camera::setTransform )
		.def( "getTransform", &Camera::getTransform, return_value_policy<copy_const_reference>() )
		.def( "setResolution", &Camera::setResolution )
		.def( "getResolution", &Camera::getResolution, return_value_policy<copy_const_reference>() )
		.def( "setScreenWindow", &Camera::setScreenWindow )
		.def( "getScreenWindow", &Camera::getScreenWindow, return_value_policy<copy_const_reference>() )
		.def( "setClippingPlanes", &Camera::setClippingPlanes )
		.def( "getClippingPlanes", &Camera::getClippingPlanes, return_value_policy<copy_const_reference>() )
		.def( "matrix", &Camera::matrix ).staticmethod( "matrix" )
		.def( "projectionMatrix", &Camera::projectionMatrix ).staticmethod( "projectionMatrix" )
		.def( "perspectiveProjection", &Camera::perspectiveProjection ).staticmethod( "perspectiveProjection" )
		.def( "positionInObjectSpace", &Camera::positionInObjectSpace ).staticmethod( "positionInObjectSpace" )
		.def( "viewDirectionInObjectSpace", &Camera::viewDirectionInObjectSpace ).staticmethod( "viewDirectionInObjectSpace" )
		.def( "upInObjectSpace", &Camera::upInObjectSpace ).staticmethod( "upInObjectSpace" )
	;
}

}
