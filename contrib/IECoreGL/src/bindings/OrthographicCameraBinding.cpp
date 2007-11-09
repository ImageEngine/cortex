//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/OrthographicCamera.h"
#include "IECoreGL/bindings/OrthographicCameraBinding.h"

#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace boost::python;

namespace IECoreGL
{

void bindOrthographicCamera()
{
	typedef class_< OrthographicCamera, OrthographicCameraPtr, boost::noncopyable, Camera> OrthographicCameraPyClass;
	OrthographicCameraPyClass( "OrthographicCamera", no_init )
		.def( init<const Imath::M44f &, const Imath::V2i &, const Imath::Box2f &, const Imath::V2f &>( (
				arg( "transform" ) = Imath::M44f(),
				arg( "resolution" ) = Imath::V2i( 640, 480 ),
				arg( "screenWindow" ) = Imath::Box2f(),
				arg( "clippingPlanes" ) = Imath::V2f( 0.1, 1000 )
			)
		) )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS( OrthographicCamera )
	;

	INTRUSIVE_PTR_PATCH( OrthographicCamera, OrthographicCameraPyClass );
	implicitly_convertible<OrthographicCameraPtr, CameraPtr>();
}
	
}
