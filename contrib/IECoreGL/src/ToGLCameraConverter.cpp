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

#include "IECoreGL/ToGLCameraConverter.h"
#include "IECoreGL/PerspectiveCamera.h"
#include "IECoreGL/OrthographicCamera.h"

#include "IECore/ObjectParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/Camera.h"
#include "IECore/Exception.h"
#include "IECore/Transform.h"

#include <boost/format.hpp>

using namespace IECoreGL;

IE_CORE_DEFINERUNTIMETYPED( ToGLCameraConverter );

ToGLCameraConverter::ToGLCameraConverter( IECore::ConstCameraPtr toConvert )
	:	ToGLConverter( staticTypeName(), "Converts IECore::Camera objects to IECoreGL::Camera objects.", IECore::CameraTypeId )
{
	srcParameter()->setValue( boost::const_pointer_cast<IECore::Camera>( toConvert ) );
}

ToGLCameraConverter::~ToGLCameraConverter()
{
}

IECore::RunTimeTypedPtr ToGLCameraConverter::doConversion( IECore::ConstObjectPtr src, IECore::ConstCompoundObjectPtr operands ) const
{
	IECore::CameraPtr camera = boost::static_pointer_cast<const IECore::Camera>( src )->copy(); // safe because the parameter validated it for us
	camera->addStandardParameters(); // now all parameters should be there and have appropriate types - so we can avoid performing checks below

	CameraPtr result = 0;
	const std::string &projection = boost::static_pointer_cast<const IECore::StringData>( camera->parameters()["projection"] )->readable();
	if( projection=="orthographic" )
	{
		result = new OrthographicCamera;
	}
	else if( projection=="perspective" )
	{
		PerspectiveCameraPtr p = new PerspectiveCamera;
		float fov = boost::static_pointer_cast<const IECore::FloatData>( camera->parameters()["projection:fov"] )->readable();
		p->setFOV( fov );
		result = p;
	}
	else
	{
		throw IECore::Exception( ( boost::format( "Unsupported projection type \"%s\"" ) % projection ).str() );
	}

	result->setResolution( boost::static_pointer_cast<const IECore::V2iData>( camera->parameters()["resolution"] )->readable() );
	result->setScreenWindow( boost::static_pointer_cast<const IECore::Box2fData>( camera->parameters()["screenWindow"] )->readable() );
	result->setClippingPlanes( boost::static_pointer_cast<const IECore::V2fData>( camera->parameters()["clippingPlanes"] )->readable() );

	IECore::TransformPtr t = camera->getTransform();
	if( t )
	{
		result->setTransform( t->transform() );
	}

	return result;
}
