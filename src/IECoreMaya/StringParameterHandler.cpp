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

#include "IECoreMaya/ToMayaObjectConverter.h"
#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/StringParameterHandler.h"

#include "IECore/SimpleTypedParameter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/CompoundObject.h"
#include "IECore/FileNameParameter.h"
#include "IECore/DirNameParameter.h"
#include "IECore/ValidatedStringParameter.h"
#include "IECore/FileSequenceParameter.h"
#include "IECore/FrameListParameter.h"

#include "maya/MFnTypedAttribute.h"
#include "maya/MFnMessageAttribute.h"
#include "maya/MFnStringArrayData.h"
#include "maya/MStringArray.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MDagPath.h"
#include "maya/MPlugArray.h"
#include "maya/MFnSet.h"
#include "maya/MSelectionList.h"

#include "maya/MStringArray.h"

using namespace IECoreMaya;
using namespace Imath;
using namespace boost;

static ParameterHandler::Description< StringParameterHandler > stringRegistrar( IECore::StringParameter::staticTypeId() );
static ParameterHandler::Description< StringParameterHandler > pathRegistrar( IECore::PathParameter::staticTypeId() );
static ParameterHandler::Description< StringParameterHandler > fileNameRegistrar( IECore::FileNameParameter::staticTypeId() );
static ParameterHandler::Description< StringParameterHandler > dirNameRegistrar( IECore::DirNameParameter::staticTypeId() );
static ParameterHandler::Description< StringParameterHandler > validatedStringRegistrar( IECore::ValidatedStringParameter::staticTypeId() );
static ParameterHandler::Description< StringParameterHandler > fileSequenceRegistrar( IECore::FileSequenceParameter::staticTypeId() );
static ParameterHandler::Description< StringParameterHandler > frameListRegistrar( IECore::FrameListParameter::staticTypeId() );

MStatus StringParameterHandler::doUpdate( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	IECore::ConstStringParameterPtr p = IECore::runTimeCast<const IECore::StringParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MObject attribute = plug.attribute();

	switch( getValueProvider(parameter) )
	{
		case ConnectedNodeNameValueProvider :
		{
			MFnMessageAttribute fnMAttr( attribute );
			if( !fnMAttr.hasObj( attribute ) )
			{
				return MS::kFailure;
			}
			break;
		}

		default :
		{
			// we'd like to be setting the default value here, but as maya doesn't save the default value
			// for dynamic string attributes in scene files, it'll be lost when the scene is reloaded. it's
			// best therefore that we don't set the default at all, so that the default is "", which is what
			// it'll be when we reload the scene - this ensures that any values set in the attribute later
			// will be saved correctly (if we set the default to "X" and the value was "X", maya won't save the
			// default or the value at all, and we end up with a value of "" on scene reload).

			MFnTypedAttribute fnTAttr( attribute );
			if( !fnTAttr.hasObj( attribute ) || fnTAttr.attrType() != MFnData::kString )
			{
				return MS::kFailure;
			}

			// Should this fail if getPlugValue fails?
			MString v = "";
			if( getPlugValue( plug, p, v ) )
			{
				IECore::ObjectPtr d = new IECore::StringData( v.asChar() );
				if( !parameter->valueValid( d.get() ) )
				{
					return MS::kFailure;
				}
			}
		}
	}

	return finishUpdating( parameter, plug );
}

MPlug StringParameterHandler::doCreate( IECore::ConstParameterPtr parameter, const MString &plugName, MObject &node ) const
{
	IECore::ConstStringParameterPtr p = IECore::runTimeCast<const IECore::StringParameter>( parameter );
	if( !p )
	{
		return MPlug();
	}

	MObject attribute;

	switch( getValueProvider(parameter) )
	{
		case ConnectedNodeNameValueProvider :
		{
			MFnMessageAttribute fnMAttr;
			attribute = fnMAttr.create( plugName, plugName );
			break;
		}

		default :
		{
			MFnTypedAttribute fnTAttr;
			attribute = fnTAttr.create( plugName, plugName, MFnData::kString /* see comments in stringUpdate for why we don't specify a default here */ );
		}
	}

	MPlug thePlug = finishCreating( parameter, attribute, node );
	if( finishUpdating( parameter, thePlug ) )
	{
		return thePlug;
	}

	return MPlug();
}

MStatus StringParameterHandler::doSetValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	IECore::ConstStringParameterPtr p = IECore::runTimeCast<const IECore::StringParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	switch( getValueProvider(parameter) )
	{
		case ConnectedNodeNameValueProvider :
		{
			return MS::kSuccess;
		}

		default :
			return plug.setValue( p->getTypedValue().c_str() );
	}
}

MStatus StringParameterHandler::doSetValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{

	IECore::StringParameterPtr p = IECore::runTimeCast<IECore::StringParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MString v = "";
	MStatus result = getPlugValue( plug, p, v );
	if( result )
	{
		p->setTypedValue( v.asChar() );
	}

	return result;
}

MStatus StringParameterHandler::getPlugValue( const MPlug &plug, IECore::ConstStringParameterPtr parameter, MString &value ) const
{
	MStatus result;

	switch( getValueProvider(parameter) )
	{
		case NodeNameValueProvider :
		{
			MObject node = plug.node();
			return getPathOrNameFromNode( node, value );
		}

		case ConnectedNodeNameValueProvider :
		{
			MPlugArray connections;
			bool connected = plug.connectedTo( connections, true, false, &result);
			if( !connected )
			{
				value = "";
				return MS::kSuccess;
			}

			MObject node = connections[0].node();
			return getPathOrNameFromNode( node, value );
		}

		default :
			return plug.getValue( value );
	}

	return MS::kFailure;
}

MStatus StringParameterHandler::getPathOrNameFromNode( const MObject &node, MString &name )
{
	MStatus result;

	if( node.hasFn(MFn::kDagNode) )
	{
		MDagPath path;
		result = MDagPath::getAPathTo( node, path );
		if( !result )
		{
			return MS::kFailure;
		}
		name = path.fullPathName();
	}
	else
	{
		MFnDependencyNode fnDN( node, &result );
		if( !result )
		{
			return MS::kFailure;
		}
		name = fnDN.name();
	}
	return MS::kSuccess;
}

StringParameterHandler::ValueProvider StringParameterHandler::getValueProvider( const IECore::ConstParameterPtr parameter )
{
	const IECore::ConstCompoundObjectPtr userData = parameter->userData();
	const IECore::ConstCompoundObjectPtr maya = userData->member<const IECore::CompoundObject>("maya");
	if (!maya)
	{
		return InvalidValueProvider;
	}

	const IECore::ConstStringDataPtr valueProvider = maya->member<const IECore::StringData>("valueProvider");
	if (!valueProvider)
	{
		return InvalidValueProvider;
	}

	if( valueProvider->readable() == "nodeName" )
	{
		return NodeNameValueProvider;
	}
	else if( valueProvider->readable() == "connectedNodeName" )
	{
		return ConnectedNodeNameValueProvider;
	}

	return InvalidValueProvider;
}

