//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2016, Image Engine Design Inc. All rights reserved.
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

#include "IECoreArnold/CameraAlgo.h"

#include "IECoreArnold/NodeAlgo.h"
#include "IECoreArnold/ParameterAlgo.h"

#include "IECoreScene/Camera.h"

#include "IECore/SimpleTypedData.h"

#include "ai.h"

using namespace IECore;
using namespace IECoreScene;
using namespace IECoreArnold;

namespace
{

NodeAlgo::ConverterDescription<Camera> g_description( CameraAlgo::convert );

const AtString g_perspCameraArnoldString("persp_camera");
const AtString g_orthoCameraArnoldString("ortho_camera");
const AtString g_fovArnoldString("fov");
const AtString g_nearClipArnoldString("near_clip");
const AtString g_farClipArnoldString("far_clip");
const AtString g_shutterStartArnoldString("shutter_start");
const AtString g_shutterEndArnoldString("shutter_end");
const AtString g_screenWindowMinArnoldString("screen_window_min");
const AtString g_screenWindowMaxArnoldString("screen_window_max");
const AtString g_apertureSizeArnoldString("aperture_size");
const AtString g_focusDistanceArnoldString("focus_distance");

} // namespace

AtNode *CameraAlgo::convert( const IECoreScene::Camera *camera, const std::string &nodeName, const AtNode *parentNode )
{
	// Use projection to decide what sort of camera node to create
	const std::string &projection = camera->getProjection();

	AtNode *result = nullptr;
	if( projection=="perspective" )
	{
		result = AiNode( g_perspCameraArnoldString, AtString( nodeName.c_str() ), parentNode );
		AiNodeSetFlt( result, g_fovArnoldString, 90.0f );

		if( camera->getFStop() > 0.0f )
		{
			// Note the factor of 0.5 because Arnold stores aperture as radius, not diameter
			AiNodeSetFlt( result, g_apertureSizeArnoldString,
				0.5f * camera->getFocalLength() * camera->getFocalLengthWorldScale() / camera->getFStop()
			);
			AiNodeSetFlt( result, g_focusDistanceArnoldString, camera->getFocusDistance() );
		}
	}
	else if( projection=="orthographic" )
	{
		result = AiNode( g_orthoCameraArnoldString, AtString( nodeName.c_str() ), parentNode );
	}
	else
	{
		result = AiNode( AtString( projection.c_str() ), AtString( nodeName.c_str() ), parentNode );
	}

	// Set clipping planes
	const Imath::V2f &clippingPlanes = camera->getClippingPlanes();
	AiNodeSetFlt( result, g_nearClipArnoldString, clippingPlanes[0] );
	AiNodeSetFlt( result, g_farClipArnoldString, clippingPlanes[1] );

	// Set shutter
	const Imath::V2f &shutter = camera->getShutter();
	AiNodeSetFlt( result, g_shutterStartArnoldString, shutter[0] );
	AiNodeSetFlt( result, g_shutterEndArnoldString, shutter[1] );

	const Imath::Box2f &frustum = camera->frustum();

	// Arnold automatically adjusts the vertical screenWindow to compensate for the resolution and pixel aspect.
	// This is handy when hand-editing .ass files, but since we already take care of this ourselves, we have
	// reverse their correction by multiplying the y values by aspect.
	const Imath::V2i &resolution = camera->getResolution();
	float aspect = camera->getPixelAspectRatio() * (float)resolution.x / (float)resolution.y;

	AiNodeSetVec2( result, g_screenWindowMinArnoldString, frustum.min.x, frustum.min.y * aspect );
	AiNodeSetVec2( result, g_screenWindowMaxArnoldString, frustum.max.x, frustum.max.y * aspect );

	// Set any Arnold-specific parameters
	const AtNodeEntry *nodeEntry = AiNodeGetNodeEntry( result );
	for( CompoundDataMap::const_iterator it = camera->parameters().begin(), eIt = camera->parameters().end(); it != eIt; ++it )
	{
		AtString paramNameArnold( it->first.c_str() );
		if( AiNodeEntryLookUpParameter( nodeEntry, paramNameArnold ) )
		{
			ParameterAlgo::setParameter( result, paramNameArnold, it->second.get() );
		}
	}

	return result;
}
