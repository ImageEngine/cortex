//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/FromMayaCameraConverter.h"
#include "IECoreMaya/Convert.h"

#include "IECore/CompoundParameter.h"
#include "IECore/Camera.h"
#include "IECore/MatrixTransform.h"
#include "IECore/BoxOperators.h"
#include "IECore/AngleConversion.h"

#include "maya/MFnCamera.h"
#include "maya/MString.h"
#include "maya/MRenderUtil.h"
#include "maya/MCommonRenderSettingsData.h"
#include "maya/MDagPath.h"

#include "OpenEXR/ImathMath.h"

using namespace IECoreMaya;
using namespace IECore;
using namespace Imath;

static const MFn::Type fromTypes[] = { MFn::kCamera, MFn::kInvalid };
static const IECore::TypeId toTypes[] = { BlindDataHolderTypeId, RenderableTypeId, PreWorldRenderableTypeId, CameraTypeId, InvalidTypeId };

FromMayaDagNodeConverter::Description<FromMayaCameraConverter> FromMayaCameraConverter::m_description( fromTypes, toTypes );

FromMayaCameraConverter::FromMayaCameraConverter( const MDagPath &dagPath )
	:	FromMayaDagNodeConverter( staticTypeName(), "Converts maya camera shape nodes into IECore::Camera objects.", dagPath )
{

	IntParameter::PresetsMap resolutionModePresets;
	resolutionModePresets["renderGlobals"] = RenderGlobals;
	resolutionModePresets["specified"] = Specified;
	
	m_resolutionMode = new IntParameter(
		"resolutionMode",
		"Determines how the resolution of the camera is decided.",
		RenderGlobals,
		RenderGlobals,
		Specified,
		resolutionModePresets,
		true
	);
	
	parameters()->addParameter( m_resolutionMode );
	
	V2iParameter::PresetsMap resolutionPresets;
	resolutionPresets["2K"] = Imath::V2i( 2048, 1556 );
	resolutionPresets["1K"] = Imath::V2i( 1024, 778 );
	
	m_resolution = new V2iParameter(
		"resolution",
		"Specifies the resolution of the camera when mode is set to \"Specified\".",
		Imath::V2i( 2048, 1556 ),
		resolutionPresets
	);
	
	parameters()->addParameter( m_resolution );
	
}

IECore::ObjectPtr FromMayaCameraConverter::doConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnCamera fnCamera( dagPath );

	// convert things that are required by the IECore::Renderer specification

	CameraPtr result = new Camera;
	result->setName( IECore::convert<std::string>( fnCamera.name() ) );
	
	result->setTransform( new MatrixTransform( IECore::convert<Imath::M44f>( dagPath.inclusiveMatrix() ) ) );

	V2i resolution;
	if( operands->member<IntData>( "resolutionMode" )->readable()==RenderGlobals )
	{
		MCommonRenderSettingsData renderSettings;
		MRenderUtil::getCommonRenderSettings( renderSettings );
		resolution = Imath::V2i( renderSettings.width, renderSettings.height );
	}
	else
	{
		resolution = operands->member<V2iData>( "resolution" )->readable();
	}
	result->parameters()["resolution"] = new V2iData( resolution );

	Imath::V2f clippingPlanes = Imath::V2f( fnCamera.nearClippingPlane(), fnCamera.farClippingPlane() );
	result->parameters()["clippingPlanes"] = new V2fData( clippingPlanes );
	
	Imath::Box2d frustum;
	fnCamera.getRenderingFrustum( (float)resolution.x / (float)resolution.y, frustum.min.x, frustum.max.x, frustum.min.y, frustum.max.y );
	
	if( fnCamera.isOrtho() )
	{
		// orthographic
		result->parameters()["projection"] = new StringData( "orthographic" );
		result->parameters()["screenWindow"] = new Box2fData( Box2f( frustum.min, frustum.max ) );
	}
	else
	{
		// perspective
		result->parameters()["projection"] = new StringData( "perspective" );
		
		// derive horizontal field of view from the viewing frustum
		float fov = Math<double>::atan( frustum.max.x / clippingPlanes[0] ) * 2.0f;
		fov = radiansToDegrees( fov );
		result->parameters()["projection:fov"] = new FloatData( fov );
		
		// scale the frustum so that it's -1,1 in x and that gives us the screen window
		float frustumScale = 2.0f/(frustum.max.x - frustum.min.x);
		Box2f screenWindow( V2f( -1, frustum.min.y * frustumScale ), V2f( 1, frustum.max.y * frustumScale ) );
		result->parameters()["screenWindow"] = new Box2fData( screenWindow );
	}
	
	// and add on other bits and bobs from maya attributes as blind data
	CompoundDataPtr maya = new CompoundData;
	result->blindData()->writable()["maya"] = maya;
	maya->writable()["aperture"] = new V2fData( Imath::V2f( fnCamera.horizontalFilmAperture(), fnCamera.verticalFilmAperture() ) );
	
	return result;
}
