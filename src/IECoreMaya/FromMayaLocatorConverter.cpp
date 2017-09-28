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

#include "IECoreMaya/FromMayaLocatorConverter.h"
#include "IECoreMaya/Convert.h"

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathMatrix.h"
#include "IECore/CoordinateSystem.h"
#include "IECore/MatrixTransform.h"

#include "maya/MPlug.h"
#include "maya/MString.h"
#include "maya/MDagPath.h"
#include "maya/MFnDagNode.h"

#include "OpenEXR/ImathMath.h"

using namespace IECoreMaya;
using namespace IECore;
using namespace Imath;

IE_CORE_DEFINERUNTIMETYPED( FromMayaLocatorConverter );

FromMayaDagNodeConverter::Description<FromMayaLocatorConverter> FromMayaLocatorConverter::m_description( MFn::kLocator, CoordinateSystemTypeId, true );

FromMayaLocatorConverter::FromMayaLocatorConverter( const MDagPath &dagPath )
	:	FromMayaDagNodeConverter( "Converts maya locator shape nodes into IECore::CoordinateSystem objects.", dagPath )
{
}

IECore::ObjectPtr FromMayaLocatorConverter::doConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus st;

	bool hasLocator = dagPath.hasFn( MFn::kLocator, &st );
	if (!st || !hasLocator)
	{
		throw Exception( "Could not find locator!" );
	}

	MObject locatorObj = dagPath.node();
	if ( !locatorObj.hasFn( MFn::kLocator ) )
	{
		throw Exception( "Not a locator!" );
	}

	MFnDagNode fnLocator( locatorObj );

	CoordinateSystemPtr result = new CoordinateSystem;
	result->setName( IECore::convert<std::string>( fnLocator.name() ) );

	/// obtain local position and scale from locator
	Imath::V3f position(0), scale(0);

	MPlug positionPlug = fnLocator.findPlug( "localPositionX", &st );
	if ( !st )
		throw Exception("Could not find 'localPositionX' plug!");
	positionPlug.getValue(position[0]);

	positionPlug = fnLocator.findPlug( "localPositionY", &st );
	if ( !st )
		throw Exception("Could not find 'localPositionY' plug!");
	positionPlug.getValue(position[1]);

	positionPlug = fnLocator.findPlug( "localPositionZ", &st );
	if ( !st )
		throw Exception("Could not find 'localPositionZ' plug!");
	positionPlug.getValue(position[2]);

	MPlug scalePlug = fnLocator.findPlug( "localScaleX", &st );
	if ( !st )
		throw Exception("Could not find 'localScaleX' plug!");
	scalePlug.getValue(scale[0]);

	scalePlug = fnLocator.findPlug( "localScaleY", &st );
	if ( !st )
		throw Exception("Could not find 'localScaleY' plug!");
	scalePlug.getValue(scale[1]);

	scalePlug = fnLocator.findPlug( "localScaleZ", &st );
	if ( !st )
		throw Exception("Could not find 'localScaleZ' plug!");
	scalePlug.getValue(scale[2]);

	Imath::M44f scaleM,translateM;
	scaleM.scale(scale);
	translateM.translate(position);
	result->setTransform( new MatrixTransform( scaleM * translateM ) );

	return result;
}
