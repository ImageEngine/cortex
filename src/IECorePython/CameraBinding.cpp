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

#include "IECore/Camera.h"
#include "IECore/Transform.h"
#include "IECorePython/CameraBinding.h"
#include "IECorePython/RunTimeTypedBinding.h"

using namespace boost::python;
using boost::python::arg;
using namespace IECore;

namespace IECorePython
{

void bindCamera()
{
	RunTimeTypedClass<Camera>()
		.def( init< optional< const std::string &, TransformPtr, CompoundDataPtr > >
			(
				(
					arg( "name" ) = std::string( "default" ),
					arg( "transform" ) = TransformPtr(),

					/// We need to explicitly make this a CompoundData::Ptr so that boost.python finds the correct to_python converter
					arg( "parameters" ) = CompoundData::Ptr( new CompoundData() )
				)
			)
		)
		.def( "setName", &Camera::setName )
		.def( "getName", &Camera::getName, return_value_policy<copy_const_reference>() )
		.def( "setTransform", &Camera::setTransform )
		.def( "getTransform", (Transform *(Camera::*)())&Camera::getTransform, return_value_policy<CastToIntrusivePtr>() )
		.def( "parameters", (CompoundData *(Camera::*)())&Camera::parametersData, return_value_policy<CastToIntrusivePtr>() )
		.def( "addStandardParameters", &Camera::addStandardParameters )
	;
}

}
