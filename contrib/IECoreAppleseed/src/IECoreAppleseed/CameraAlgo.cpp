//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2016, Esteban Tovagliari. All rights reserved.
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

#include "IECoreAppleseed/CameraAlgo.h"

#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "foundation/math/scalar.h"
#include "foundation/utility/iostreamop.h"
#include "renderer/api/project.h"
#include "renderer/api/frame.h"

using namespace IECore;
using namespace IECoreScene;
using namespace Imath;

namespace asf = foundation;
namespace asr = renderer;

namespace IECoreAppleseed
{

namespace CameraAlgo
{

renderer::Camera *convert( const IECoreScene::Camera *camera )
{
	asr::ParamArray cameraParams;

	// set shutter
	const V2f &shutter = camera->getShutter();
	cameraParams.insert( "shutter_open_begin_time", shutter.x );
	cameraParams.insert( "shutter_open_end_time", shutter.x );
	cameraParams.insert( "shutter_close_begin_time", shutter.y );
	cameraParams.insert( "shutter_close_end_time", shutter.y );

	asr::CameraFactoryRegistrar cameraFactories;
	const asr::ICameraFactory *cameraFactory = nullptr;

	const std::string &projection = camera->getProjection();

	V2f apertureOffset = camera->getApertureOffset();

	const Box2f &screenWindow = camera->frustum();
	V2f fitAperture = screenWindow.size();

	if( projection=="perspective" )
	{
		float focalLengthScale = camera->getFocalLengthWorldScale();
		float focalLength = focalLengthScale * camera->getFocalLength();
		cameraParams.insert( "focal_length", focalLength );
		fitAperture = fitAperture * focalLength;
		apertureOffset *= focalLengthScale;

		if( camera->getFStop() == 0.0f )
		{
			cameraFactory = cameraFactories.lookup( "pinhole_camera" );
		}
		else
		{
			cameraFactory = cameraFactories.lookup( "thinlens_camera" );
			cameraParams.insert( "f_stop", camera->getFStop() );

			cameraParams.insert( "autofocus_enabled", false );
			cameraParams.insert( "focal_distance", camera->getFocusDistance() );
		}
	}
	else if( projection=="orthographic" )
	{
		cameraFactory = cameraFactories.lookup( "orthographic_camera" );
	}
	else
	{
		throw Exception( "Unknown camera projection" );
	}

	asf::Vector2f film_dims( fitAperture.x, fitAperture.y );
	cameraParams.insert( "film_dimensions", film_dims );


	cameraParams.insert( "shift_x", apertureOffset.x );
	cameraParams.insert( "shift_y", apertureOffset.y );

	asf::auto_release_ptr<asr::Camera> result( cameraFactory->create( "camera", cameraParams ) );

	return result.release();
}

} // namespace CameraAlgo

} // namespace IECoreAppleseed
