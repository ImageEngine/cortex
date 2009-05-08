//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/Parameter.h"
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

MStatus StringParameterHandler::update( IECore::ConstParameterPtr parameter, MObject &attribute ) const
{
	IECore::ConstStringParameterPtr p = IECore::runTimeCast<const IECore::StringParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	const IECore::ConstCompoundObjectPtr userData = parameter->userData();
	const IECore::ConstCompoundObjectPtr maya = userData->member<const IECore::CompoundObject>("maya");

	if (maya)
	{
		const IECore::ConstStringDataPtr valueProvider = maya->member<const IECore::StringData>("valueProvider");

		if (valueProvider && valueProvider->readable() == "connectedNodeName")
		{
			/// Nothing to do.
			return MS::kSuccess;
		}
	}

	// we'd like to be setting the default value here, but as maya doesn't save the default value
	// for dynamic string attributes in scene files, it'll be lost when the scene is reloaded. it's
	// best therefore that we don't set the default at all, so that the default is "", which is what
	// it'll be when we reload the scene - this ensures that any values set in the attribute later
	// will be saved correctly (if we set the default to "X" and the value was "X", maya won't save the
	// default or the value at all, and we end up with a value of "" on scene reload).

	return MS::kSuccess;
}

MObject StringParameterHandler::create( IECore::ConstParameterPtr parameter, const MString &attributeName ) const
{
	IECore::ConstStringParameterPtr p = IECore::runTimeCast<const IECore::StringParameter>( parameter );
	if( !p )
	{
		return MObject::kNullObj;
	}

	const IECore::ConstCompoundObjectPtr userData = parameter->userData();
	const IECore::ConstCompoundObjectPtr maya = userData->member<const IECore::CompoundObject>("maya");

	if (maya)
	{
		const IECore::ConstStringDataPtr valueProvider = maya->member<const IECore::StringData>("valueProvider");

		if (valueProvider && valueProvider->readable() == "connectedNodeName")
		{
			MFnMessageAttribute fnMAttr;
			MObject result = fnMAttr.create( attributeName, attributeName );

			return result;

		}
	}

	MFnTypedAttribute fnTAttr;
	MObject result = fnTAttr.create( attributeName, attributeName, MFnData::kString /* see comments in stringUpdate for why we don't specify a default here */ );
	update( parameter, result );
	return result;
}

MStatus StringParameterHandler::setValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	IECore::ConstStringParameterPtr p = IECore::runTimeCast<const IECore::StringParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	const IECore::ConstCompoundObjectPtr userData = parameter->userData();
	const IECore::ConstCompoundObjectPtr maya = userData->member<const IECore::CompoundObject>("maya");

	if (maya)
	{
		const IECore::ConstStringDataPtr valueProvider = maya->member<const IECore::StringData>("valueProvider");

		if (valueProvider && valueProvider->readable() == "connectedNodeName")
		{
			return MS::kSuccess;
		}
	}

	return plug.setValue( p->getTypedValue().c_str() );
}

MStatus StringParameterHandler::setValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	IECore::StringParameterPtr p = IECore::runTimeCast<IECore::StringParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}

	MString v = "";
	MStatus result;
	const IECore::ConstCompoundObjectPtr userData = parameter->userData();
	const IECore::ConstCompoundObjectPtr maya = userData->member<const IECore::CompoundObject>("maya");

	bool hasValueProvider = false;
	if (maya)
	{
		const IECore::ConstStringDataPtr valueProvider = maya->member<const IECore::StringData>("valueProvider");

		if (valueProvider && valueProvider->readable() == "nodeName")
		{
			MStatus s;
			MObject node = plug.node();

			if (node.hasFn( MFn::kDagNode) )
			{
				MDagPath path;
				s = MDagPath::getAPathTo( node, path );
				if (!s)
				{
					return s;
				}
				v = path.fullPathName();
			}
			else
			{
				MFnDependencyNode fnDN( node, &s );
				if (!s)
				{
					return s;
				}
				v = fnDN.name();
			}

			hasValueProvider = true;
		}
		else if (valueProvider && valueProvider->readable() == "connectedNodeName")
		{
			assert( plug.attribute().hasFn( MFn::kMessageAttribute ) );

			MStatus s;

			MPlugArray connections;
			bool connected = plug.connectedTo( connections, true, false, &s);

			if (connected)
			{
				MObject node = connections[0].node();

				if (node.hasFn( MFn::kDagNode) )
				{
					MDagPath path;
					s = MDagPath::getAPathTo( node, path );
					if (!s)
					{
						return s;
					}
					v = path.fullPathName();
				}
				else
				{
					MFnDependencyNode fnDN( node, &s );
					if (!s)
					{
						return s;
					}
					v = fnDN.name();
				}
			}

			hasValueProvider = true;
		}
	}

	if (!hasValueProvider)
	{
		result = plug.getValue( v );
	}
	if( result )
	{
		p->setTypedValue( v.asChar() );
	}
	return result;
}
