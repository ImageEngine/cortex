//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/NumericTraits.h"
#include "IECoreMaya/ToMayaObjectConverter.h"
#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/BoolParameterHandler.h"

#include "IECore/TypedParameter.h"
#include "IECore/CompoundObject.h"

#include "maya/MFnNumericAttribute.h"

using namespace IECoreMaya;
using namespace Imath;
using namespace boost;

static ParameterHandler::Description< BoolParameterHandler > registrar( IECore::BoolParameter::staticTypeId() );

MStatus BoolParameterHandler::doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	IECore::ConstBoolParameterPtr p = IECore::runTimeCast<const IECore::BoolParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MObject attribute = plug.attribute();
	MFnNumericAttribute fnNAttr( attribute );
	if( !fnNAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}

	fnNAttr.setDefault( p->typedDefaultValue() );

	bool keyable = true;
	bool channelBox = true;

	const IECore::ConstCompoundObjectPtr userData = parameter->userData();
	assert( userData );

	/// \todo This stuff could be consolidated into one place as it appears to be duplicated in several
	/// ParameterHandlers
	const IECore::ConstCompoundObjectPtr maya = userData->member<const IECore::CompoundObject>("maya");
	if (maya)
	{
		const IECore::ConstBoolDataPtr keyableData = maya->member<const IECore::BoolData>("keyable");
		if (keyableData)
		{
			keyable = keyableData->readable();
		}

		const IECore::ConstBoolDataPtr channelBoxData = maya->member<const IECore::BoolData>("channelBox");
		if (channelBoxData)
		{
			channelBox = channelBoxData->readable();
		}
	}

	fnNAttr.setKeyable( keyable );

	// Calling setChannelBox(true) disables keying
	if (!keyable)
	{
		fnNAttr.setChannelBox( channelBox );
	}

	return finishUpdating( parameter, plug );
}

MPlug BoolParameterHandler::doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const
{
	IECore::ConstBoolParameterPtr p = IECore::runTimeCast<const IECore::BoolParameter>( parameter );
	if( !p )
	{
		return MPlug();
	}

	MFnNumericAttribute fnNAttr;
	MObject attribute = fnNAttr.create( plugName, plugName, MFnNumericData::kBoolean, p->typedDefaultValue() );

	MPlug result = finishCreating( parameter, attribute, node );
	doUpdate( parameter, result );
	return result;
}

MStatus BoolParameterHandler::doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	IECore::ConstBoolParameterPtr p = IECore::runTimeCast<const IECore::BoolParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	return plug.setValue( p->getTypedValue() );
}

MStatus BoolParameterHandler::doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	IECore::BoolParameterPtr p = IECore::runTimeCast<IECore::BoolParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	bool v;
	MStatus result;
	result = plug.getValue( v );
	if( result )
	{
		p->setTypedValue( v );
	}
	return result;
}
