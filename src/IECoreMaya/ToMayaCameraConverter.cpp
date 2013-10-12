//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore\CompoundObject.h"
#include "maya/MCommonRenderSettingsData.h"
#include "maya/MDagPath.h"
#include "maya/MDagModifier.h"
#include "maya/MFnCamera.h"
#include "maya/MFnTransform.h"
#include "maya/MRenderUtil.h"
#include "maya/MSelectionList.h"

#include "IECore/AngleConversion.h"
#include "IECore/Camera.h"
#include "IECore/MatrixTransform.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/ToMayaCameraConverter.h"

using namespace IECore;
using namespace IECoreMaya;

ToMayaCameraConverter::Description ToMayaCameraConverter::g_description( IECore::Camera::staticTypeId(), MFn::kCamera );

ToMayaCameraConverter::ToMayaCameraConverter( IECore::ConstObjectPtr object )
: ToMayaObjectConverter( "Converts IECore::Camera objects to Maya cameras.", object )
{
}

bool ToMayaCameraConverter::doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const
{
	ConstCameraPtr camera = IECore::runTimeCast<const Camera>( from );
	if ( !camera )
	{
		IECore::msg( IECore::Msg::Warning, "ToMayaCameraConverter::doConversion",  "The source object is not an IECore::Camera." );
		return false;
	}
	
	// check if incoming object is a cameraShape itself
	MObject camObj;
	MFnCamera fnCamera;
	if ( fnCamera.hasObj( to ) )
	{
		camObj = to;
	}
	
	// check if incoming object is a parent of an existing cameraShape
	if ( camObj.isNull() )
	{
		MFnDagNode fnTo( to );
		for ( unsigned i=0; i < fnTo.childCount(); ++i )
		{
			MObject child = fnTo.child( i );
			if ( fnCamera.hasObj( child ) )
			{
				camObj = child;
				break;
			}
		}
	}
	
	// make a new cameraShape and parent it to the incoming object
	if ( camObj.isNull() )
	{
		if ( !MFnTransform().hasObj( to ) )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaCameraConverter::doConversion",  "Unable to create a camera as a child of the input object." );
			return false;
		}
		
		MDagModifier dagMod;
		camObj = dagMod.createNode( "camera", to );
		dagMod.renameNode( camObj, camera->getName().c_str() );
		if ( !dagMod.doIt() )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaCameraConverter::doConversion",  "Unable to modify the DAG correctly." );
			dagMod.undoIt();
			return false;
		}
	}
	
	if ( camObj.isNull() )
	{
		IECore::msg( IECore::Msg::Warning, "ToMayaCameraConverter::doConversion",  "Unable to find or create a camera from the input object." );
		return false;
	}
	
	MDagPath camDag;
	MFnDagNode( camObj ).getPath( camDag );
	fnCamera.setObject( camDag );
	
	double fov = fnCamera.horizontalFieldOfView();
	
	MCommonRenderSettingsData renderSettings;
	MRenderUtil::getCommonRenderSettings( renderSettings );
	Imath::V2i resolution( renderSettings.width, renderSettings.height );
	CompoundDataMap::const_iterator resIt = camera->parameters().find( "resolution" );
	if ( resIt != camera->parameters().end() && resIt->second->isInstanceOf( V2iDataTypeId ) )
	{
		resolution = staticPointerCast<V2iData>( resIt->second )->readable();
	}
	double aspectRatio = (double)resolution.x / (double)resolution.y;
	
	const MatrixTransform *transform = IECore::runTimeCast<const MatrixTransform>( camera->getTransform() );
	if ( transform )
	{
		Imath::M44f mat = transform->transform();
		MPoint pos = IECore::convert<MPoint>( mat.translation() );
		MVector view = IECore::convert<MVector>( -Imath::V3f( mat[2][0], mat[2][1], mat[2][2] ) );
		MVector up = IECore::convert<MVector>( Imath::V3f( mat[1][0], mat[1][1], mat[1][2] ) );
		
		if ( !fnCamera.set( pos, view, up, fov, aspectRatio ) )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaCameraConverter::doConversion",  "Unable to modify the Camera settings." );
			return false;
		}
	}
	
	const CompoundData *mayaData = camera->blindData()->member<CompoundData>( "maya" );
	if ( mayaData )
	{
		const V2fData *apertureData = mayaData->member<V2fData>( "aperture" );
		if ( apertureData )
		{
			Imath::V2f aperture = apertureData->readable();
			fnCamera.setHorizontalFilmAperture( aperture[0] );
			fnCamera.setVerticalFilmAperture( aperture[1] );
		}
	}
	
	CompoundDataMap::const_iterator clippingIt = camera->parameters().find( "clippingPlanes" );
	if ( clippingIt != camera->parameters().end() && clippingIt->second->isInstanceOf( V2fDataTypeId ) )
	{
		Imath::V2f clippingPlanes = staticPointerCast<V2fData>( clippingIt->second )->readable();
		fnCamera.setNearClippingPlane( clippingPlanes[0] );
		fnCamera.setFarClippingPlane( clippingPlanes[1] );
	}
	
	std::string projection = "";
	CompoundDataMap::const_iterator projectionIt = camera->parameters().find( "projection" );
	if ( projectionIt != camera->parameters().end() && projectionIt->second->isInstanceOf( StringDataTypeId ) )
	{
		projection = staticPointerCast<StringData>( projectionIt->second )->readable();
	}
	if ( projection == "perspective" )
	{
		fnCamera.setIsOrtho( false );
	}
	else
	{
		fnCamera.setIsOrtho( true );
		
		Imath::Box2f screenWindow;
		CompoundDataMap::const_iterator screenWindowIt = camera->parameters().find( "screenWindow" );
		if ( screenWindowIt != camera->parameters().end() && screenWindowIt->second->isInstanceOf( Box2fDataTypeId ) )
		{
			screenWindow = staticPointerCast<Box2fData>( screenWindowIt->second )->readable();
		}
		
		if ( !screenWindow.isEmpty() )
		{
			fnCamera.setOrthoWidth( screenWindow.max.x - screenWindow.min.x );
		}
	}
	
	CompoundDataMap::const_iterator fovIt = camera->parameters().find( "projection:fov" );
	if ( fovIt != camera->parameters().end() && fovIt->second->isInstanceOf( FloatDataTypeId ) )
	{
		fov = degreesToRadians( staticPointerCast<FloatData>( fovIt->second )->readable() );
	}
	
	// setting field of view last as some of the other commands alter it
	fnCamera.setHorizontalFieldOfView( fov );
	
	return true;
}
