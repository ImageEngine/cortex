//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2012, Image Engine Design Inc. All rights reserved.
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

#include "IECoreGL/ToGLCameraConverter.h"

#include "IECoreGL/Camera.h"

#include "IECoreScene/Camera.h"

#include "IECore/CompoundParameter.h"
#include "IECore/Exception.h"
#include "IECore/ObjectParameter.h"
#include "IECore/SimpleTypedData.h"

#include "boost/format.hpp"

using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( ToGLCameraConverter );

ToGLConverter::ConverterDescription<ToGLCameraConverter> ToGLCameraConverter::g_description;

ToGLCameraConverter::ToGLCameraConverter( IECoreScene::ConstCameraPtr toConvert )
	:	ToGLConverter( "Converts IECoreScene::Camera objects to IECoreGL::Camera objects.", IECoreScene::Camera::staticTypeId() )
{
	srcParameter()->setValue( boost::const_pointer_cast<IECoreScene::Camera>( toConvert ) );
}

ToGLCameraConverter::~ToGLCameraConverter()
{
}

IECore::RunTimeTypedPtr ToGLCameraConverter::doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const
{
	IECoreScene::ConstCameraPtr camera = boost::static_pointer_cast<const IECoreScene::Camera>( src );
	return new Camera(
		Imath::M44f(),
		camera->getProjection() == "orthographic",
		camera->getResolution(),
		camera->frustum(),
		camera->getClippingPlanes()
	);
}
