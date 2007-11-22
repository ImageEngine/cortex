//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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
#include "IECoreMaya/StringVectorParameterHandler.h"

#include "IECore/TypedParameter.h"

#include "maya/MFnTypedAttribute.h"
#include "maya/MFnMessageAttribute.h"
#include "maya/MFnStringArrayData.h"
#include "maya/MStringArray.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MDagPath.h"
#include "maya/MPlugArray.h"
#include "maya/MFnSet.h"
#include "maya/MSelectionList.h"


using namespace IECoreMaya;
using namespace Imath;
using namespace boost;

static ParameterHandler::Description< StringVectorParameterHandler > registrar( IECore::StringVectorParameter::staticTypeId() );

MStatus StringVectorParameterHandler::update( IECore::ConstParameterPtr parameter, MObject &attribute ) const
{
	intrusive_ptr<const IECore::StringVectorParameter> p = IECore::runTimeCast<const IECore::StringVectorParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}
	
	MFnTypedAttribute fnTAttr( attribute );
	if( !fnTAttr.hasObj( attribute ) )
	{
		return MS::kFailure;
	}
	
	const IECore::StringVectorParameter::ValueType &value = p->typedDefaultValue();
	MStringArray defaultValue;
	
	for (IECore::StringVectorParameter::ValueType::const_iterator it = value.begin(); it != value.end(); ++it)
	{
		defaultValue.append( it->c_str() );
	}
	
	fnTAttr.setDefault( MFnStringArrayData().create( defaultValue ) );		
	
	return MS::kSuccess;
}

MObject StringVectorParameterHandler::create( IECore::ConstParameterPtr parameter, const MString &attributeName ) const
{
	intrusive_ptr<const IECore::StringVectorParameter> p = IECore::runTimeCast<const IECore::StringVectorParameter>( parameter );
	if( !p )
	{
		return MObject::kNullObj;
	}
	
	const IECore::StringVectorParameter::ValueType &value = p->typedDefaultValue();
	MStringArray defaultValue;
	
	for (IECore::StringVectorParameter::ValueType::const_iterator it = value.begin(); it != value.end(); ++it)
	{
		defaultValue.append( it->c_str() );
	}
	
	MFnTypedAttribute fnTAttr;
	MObject result = fnTAttr.create( attributeName, attributeName, MFnData::kStringArray, MFnStringArrayData().create( defaultValue ) );
	update( parameter, result );
	return result;
}
		
MStatus StringVectorParameterHandler::setValue( IECore::ConstParameterPtr parameter, MPlug &plug ) const
{
	intrusive_ptr<const IECore::StringVectorParameter> p = IECore::runTimeCast<const IECore::StringVectorParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}		
	
	const IECore::StringVectorParameter::ValueType &value = p->getTypedValue();
	MStringArray arr;
	
	for (IECore::StringVectorParameter::ValueType::const_iterator it = value.begin(); it != value.end(); ++it)
	{
		arr.append( it->c_str() );
	}
		
	
	MObject data = MFnStringArrayData().create( arr );
		
	return plug.setValue( data );
}

MStatus StringVectorParameterHandler::setValue( const MPlug &plug, IECore::ParameterPtr parameter ) const
{
	intrusive_ptr<IECore::StringVectorParameter> p = IECore::runTimeCast<IECore::StringVectorParameter>( parameter );
	if( !p )
	{
		return MS::kFailure;
	}
	
	MStringArray arr;
	MStatus result;
	const IECore::ConstCompoundObjectPtr userData = parameter->userData();
	const IECore::ConstCompoundObjectPtr maya = userData->member<const IECore::CompoundObject>("maya");
		
	bool hasValueProvider = false;
	
	if (maya)
	{
		const IECore::ConstStringDataPtr valueProvider = maya->member<const IECore::StringData>("valueProvider");
		
		if (valueProvider && valueProvider->readable() == "setMembers")
		{
			MStatus s;
			MFnSet fnSet( plug.node(), &s );
			if (!s)
			{
				return s;
			}
			
			MSelectionList members;
			s = fnSet.getMembers( members, true );
			if (!s)
			{
				return s;
			}
			
			for (unsigned i = 0; i < members.length(); ++i)
			{
				MObject node;
				MPlug plug;
				MDagPath path;
				
				if (members.getPlug(i, plug))
				{
					arr.append( plug.name() );
				} 
				else if (members.getDagPath(i, path))
				{
					arr.append( path.fullPathName() );
				}
				else if (members.getDependNode(i, node))
				{
					MFnDependencyNode fnDN(node);
					arr.append( fnDN.name() );
				}
				else
				{
					return MS::kFailure;
				}
				
			} 
			
			hasValueProvider = true;
		}	
	}	
	
	if (!hasValueProvider)
	{
		MObject data;
		result = plug.getValue( data );
		
		if( result )
		{	
			MStatus s;
			MFnStringArrayData fnData( data, &s );
			if (!s)
			{
				return s;
			}
		
			arr = fnData.array();	
		}
	}
		
	IECore::StringVectorParameter::ValueType value;
	
	for (unsigned i = 0; i < arr.length(); i++)
	{
		value.push_back( arr[i].asChar() );
	}	
		
	p->setTypedValue( value );
	
	return result;
}
