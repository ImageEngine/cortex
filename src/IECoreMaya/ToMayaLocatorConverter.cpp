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

#include "maya/MDagPath.h"
#include "maya/MDagModifier.h"
#include "maya/MFnDagNode.h"
#include "maya/MSelectionList.h"
#include "maya/MFnTransform.h"
#include "maya/MPlug.h"

#include "OpenEXR/ImathMatrixAlgo.h"
#include "IECore/AngleConversion.h"
#include "IECore/CoordinateSystem.h"
#include "IECore/MatrixTransform.h"
#include "IECore/MessageHandler.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/ToMayaLocatorConverter.h"

using namespace IECore;
using namespace IECoreMaya;

ToMayaLocatorConverter::Description ToMayaLocatorConverter::g_description( IECore::CoordinateSystem::staticTypeId(), MFn::kLocator );

ToMayaLocatorConverter::ToMayaLocatorConverter( IECore::ConstObjectPtr object )
: ToMayaObjectConverter( "Converts IECore::CoordinateSystem objects to Maya locators.", object )
{
}

bool ToMayaLocatorConverter::doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const
{
	ConstCoordinateSystemPtr coordSys = IECore::runTimeCast<const CoordinateSystem>( from );
	if ( !coordSys )
	{
		IECore::msg( IECore::Msg::Warning, "ToMayaLocatorConverter::doConversion",  "The source object is not an IECore::CoordinateSystem." );
		return false;
	}
	
	// check if incoming object is a locator itself
	MObject locatorObj;
	if ( to.hasFn( MFn::kLocator ) )
	{
		locatorObj = to;
	}
	
	// check if incoming object is a parent of an existing locator
	if ( locatorObj.isNull() )
	{
		MFnDagNode fnTo( to );
		for ( unsigned i=0; i < fnTo.childCount(); ++i )
		{
			MObject child = fnTo.child( i );
			if ( child.hasFn( MFn::kLocator ) )
			{
				locatorObj = child;
				break;
			}
		}
	}
	
	// make a new locator and parent it to the incoming object
	if ( locatorObj.isNull() )
	{
		if ( !MFnTransform().hasObj( to ) )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaLocatorConverter::doConversion",  "Unable to create a locator as a child of the input object." );
			return false;
		}
		
		MDagModifier dagMod;
		locatorObj = dagMod.createNode( "locator", to );
		dagMod.renameNode( locatorObj, coordSys->getName().c_str() );
		if ( !dagMod.doIt() )
		{
			IECore::msg( IECore::Msg::Warning, "ToMayaLocatorConverter::doConversion",  "Unable to modify the DAG correctly." );
			dagMod.undoIt();
			return false;
		}
	}
	
	if ( locatorObj.isNull() )
	{
		IECore::msg( IECore::Msg::Warning, "ToMayaLocatorConverter::doConversion",  "Unable to find or create a locator from the input object." );
		return false;
	}
	
	MFnDagNode fnLocator( locatorObj );
	
	Imath::M44f m;
	const IECore::Transform* transform = coordSys->getTransform();
	if( transform )
	{
		m = transform->transform();
	}
	Imath::V3f s,h,r,t;
	Imath::extractSHRT(m, s, h, r, t);

	/// obtain local position and scale from locator
	MStatus st;
	MPlug positionPlug = fnLocator.findPlug( "localPositionX", &st );
	if ( !st ) return false;
	positionPlug.setValue(t[0]);
	positionPlug = fnLocator.findPlug( "localPositionY", &st );
	if ( !st ) return false;
	positionPlug.setValue(t[1]);
	positionPlug = fnLocator.findPlug( "localPositionZ", &st );
	if ( !st ) return false;
	positionPlug.setValue(t[2]);

	MPlug scalePlug = fnLocator.findPlug( "localScaleX", &st );
	if ( !st ) return false;
	scalePlug.setValue(s[0]);
	scalePlug = fnLocator.findPlug( "localScaleY", &st );
	if ( !st ) return false;
	scalePlug.setValue(s[1]);
	scalePlug = fnLocator.findPlug( "localScaleZ", &st );
	if ( !st ) return false;
	scalePlug.setValue(s[2]);

	return true;
}
