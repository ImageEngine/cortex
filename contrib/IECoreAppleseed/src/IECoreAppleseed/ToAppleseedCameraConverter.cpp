//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2014, Esteban Tovagliari. All rights reserved.
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

#include <sstream>

#include "renderer/api/camera.h"

#include "IECore/Camera.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreAppleseed/ToAppleseedCameraConverter.h"

using namespace IECoreAppleseed;
using namespace IECore;

namespace asf = foundation;
namespace asr = renderer;

IE_CORE_DEFINERUNTIMETYPED( ToAppleseedCameraConverter );

ToAppleseedCameraConverter::ConverterDescription<ToAppleseedCameraConverter> ToAppleseedCameraConverter::g_description;

ToAppleseedCameraConverter::ToAppleseedCameraConverter( IECore::CameraPtr toConvert )
    :	ToAppleseedConverter( "Converts IECore::Cameras to appleseed camera nodes", IECore::Camera::staticTypeId() )
{
    srcParameter()->setValue( toConvert );
}

ToAppleseedCameraConverter::~ToAppleseedCameraConverter()
{
}

asr::Entity *ToAppleseedCameraConverter::doConversion( IECore::ConstObjectPtr from, IECore::ConstCompoundObjectPtr operands ) const
{
    CameraPtr camera = boost::static_pointer_cast<const Camera>( from )->copy();
    camera->addStandardParameters();

    asr::ParamArray cameraParams;

    // set shutter
    const Imath::V2f &shutter = camera->parametersData()->member<V2fData>( "shutter", true )->readable();
    cameraParams.insert( "shutter_open_time", shutter.x );
    cameraParams.insert( "shutter_close_time", shutter.y );

    const Imath::V2i &resolution = camera->parametersData()->member<V2iData>( "resolution", true )->readable();
    {
        foundation::Vector2d film_dims( resolution.x, resolution.y );
        film_dims /= 10000.0;
        std::stringstream ss;
        ss << film_dims.x << " " << film_dims.y;
        cameraParams.insert( "film_dimensions", ss.str().c_str() );
    }

    asr::CameraFactoryRegistrar cameraFactories;
    const asr::ICameraFactory *cameraFactory = 0;

    const std::string &projection = camera->parametersData()->member<StringData>( "projection", true )->readable();

    if( projection=="perspective" )
    {
        double horizontal_fov = camera->parametersData()->member<FloatData>( "projection:fov", true )->readable();

        // adjust fov.
        if( resolution.x > resolution.y )
        {
            horizontal_fov *= static_cast<float>( resolution.x ) / resolution.y;
        }

        cameraParams.insert( "horizontal_fov", horizontal_fov );
        cameraFactory = cameraFactories.lookup( "pinhole_camera" );
    }
    else if( projection=="spherical" )
    {
        cameraFactory = cameraFactories.lookup( "spherical_camera" );
    }
    else
    {
         IECore::msg( Msg::Warning, "ToAppleseedCameraConverter", "unsupported projection type. Creating a default camera" );
         cameraFactory = cameraFactories.lookup( "pinhole_camera" );
    }

    asf::auto_release_ptr<asr::Camera> result( cameraFactory->create( "camera", cameraParams ) );
    return result.release();
}
