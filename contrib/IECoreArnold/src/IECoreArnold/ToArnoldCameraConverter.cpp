//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2012, Image Engine Design Inc. All rights reserved.
//  Copyright (c) 2012, John Haddon. All rights reserved.
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

// This must come before the Cortex includes, because on OSX headers included
// by TBB define macros which conflict with the inline functions in ai_types.h.
#include "ai.h"

#include "IECore/Camera.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreArnold/ToArnoldCameraConverter.h"

using namespace IECoreArnold;
using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( ToArnoldCameraConverter );

ToArnoldCameraConverter::ConverterDescription<ToArnoldCameraConverter> ToArnoldCameraConverter::g_description;

ToArnoldCameraConverter::ToArnoldCameraConverter( IECore::CameraPtr toConvert )
	:	ToArnoldConverter( "Converts IECore::Cameras to arnold camera nodes", IECore::Camera::staticTypeId() )
{
	srcParameter()->setValue( toConvert );
}

ToArnoldCameraConverter::~ToArnoldCameraConverter()
{
}

AtNode *ToArnoldCameraConverter::doConversion( IECore::ConstObjectPtr from, IECore::ConstCompoundObjectPtr operands ) const
{
	CameraPtr camera = boost::static_pointer_cast<const Camera>( from )->copy();
	camera->addStandardParameters();

	// use projection to decide what sort of camera node to create
	const std::string &projection = camera->parametersData()->member<StringData>( "projection", true )->readable();
	AtNode *result = 0;
	if( projection=="perspective" )
	{
		result = AiNode( "persp_camera" );
		AiNodeSetFlt( result, "fov", camera->parametersData()->member<FloatData>( "projection:fov", true )->readable() );
	}
	else if( projection=="orthographic" )
	{
		result = AiNode( "ortho_camera" );
	}
	else
	{
		result = AiNode( projection.c_str() );
	}
	
	// set clipping planes
	const Imath::V2f &clippingPlanes = camera->parametersData()->member<V2fData>( "clippingPlanes", true )->readable();
	AiNodeSetFlt( result, "near_clip", clippingPlanes[0] );
	AiNodeSetFlt( result, "far_clip", clippingPlanes[1] );
	
	// set shutter
	const Imath::V2f &shutter = camera->parametersData()->member<V2fData>( "shutter", true )->readable();
	AiNodeSetFlt( result, "shutter_start", shutter[0] );
	AiNodeSetFlt( result, "shutter_end", shutter[1] );
	
	// set screen window
	const Imath::Box2f &screenWindow = camera->parametersData()->member<Box2fData>( "screenWindow", true )->readable();
	const Imath::V2i &resolution = camera->parametersData()->member<V2iData>( "resolution", true )->readable();
	float aspect = (float)resolution.x / (float)resolution.y;
	AiNodeSetPnt2( result, "screen_window_min", screenWindow.min.x, screenWindow.min.y * aspect );
	AiNodeSetPnt2( result, "screen_window_max", screenWindow.max.x, screenWindow.max.y * aspect );
	
	return result;
}
