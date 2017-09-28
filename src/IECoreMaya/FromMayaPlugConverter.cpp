//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#include <iostream>

#include "maya/MFnAttribute.h"
#include "maya/MFnNumericAttribute.h"
#include "maya/MFnUnitAttribute.h"
#include "maya/MFnTypedAttribute.h"

#include "IECore/MessageHandler.h"

#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/FromMayaPlugConverter.h"

using namespace IECoreMaya;
using namespace IECore;

IE_CORE_DEFINERUNTIMETYPED( FromMayaPlugConverter );

FromMayaPlugConverter::FromMayaPlugConverter( const MPlug &plug )
	:	FromMayaConverter( "Converts the value held in a maya plug." ), m_plug( plug )
{
}

const MPlug &FromMayaPlugConverter::plug() const
{
	return m_plug;
}

FromMayaPlugConverter::NumericTypesToFnsMap &FromMayaPlugConverter::numericTypesToFns()
{
	static NumericTypesToFnsMap m;
	return m;
}

FromMayaPlugConverter::NumericDefaultConvertersMap &FromMayaPlugConverter::numericDefaultConverters()
{
	static NumericDefaultConvertersMap m;
	return m;
}

void FromMayaPlugConverter::registerConverter( const MFnNumericData::Type fromType, IECore::TypeId resultType, bool isDefaultConverter, CreatorFn creator )
{
	NumericTypesToFnsMap &m = numericTypesToFns();
	NumericTypesToFnsMap::const_iterator it = m.insert( NumericTypesToFnsMap::value_type( NumericTypePair( fromType, resultType ), creator ) ).first;
	if( isDefaultConverter )
	{
		NumericDefaultConvertersMap &dc = numericDefaultConverters();
		if( ! dc.insert( NumericDefaultConvertersMap::value_type( fromType, it ) ).second )
		{
			IECore::msg( IECore::Msg::Error, "FromMayaPlugConverter::registerConverter", boost::format( "Default conversion for MFnNumericData::Type %d already registered - ignoring second registration." ) % fromType );
		}
	}
}

FromMayaPlugConverter::TypedTypesToFnsMap &FromMayaPlugConverter::typedTypesToFns()
{
	static TypedTypesToFnsMap m;
	return m;
}

FromMayaPlugConverter::TypedDefaultConvertersMap &FromMayaPlugConverter::typedDefaultConverters()
{
	static TypedDefaultConvertersMap m;
	return m;
}

void FromMayaPlugConverter::registerConverter( const MFnData::Type fromType, IECore::TypeId resultType, bool isDefaultConverter, CreatorFn creator )
{
	TypedTypesToFnsMap &m = typedTypesToFns();
	TypedTypesToFnsMap::const_iterator it = m.insert( TypedTypesToFnsMap::value_type( TypedTypePair( fromType, resultType ), creator ) ).first;
	if( isDefaultConverter )
	{
		TypedDefaultConvertersMap &dc = typedDefaultConverters();
		if( ! dc.insert( TypedDefaultConvertersMap::value_type( fromType, it ) ).second )
		{
			IECore::msg( IECore::Msg::Error, "FromMayaPlugConverter::registerConverter", boost::format( "Default conversion for MFnData::Type %d already registered - ignoring second registration." ) % fromType );
		}
	}
}

FromMayaPlugConverter::UnitTypesToFnsMap &FromMayaPlugConverter::unitTypesToFns()
{
	static UnitTypesToFnsMap m;
	return m;
}

FromMayaPlugConverter::UnitDefaultConvertersMap &FromMayaPlugConverter::unitDefaultConverters()
{
	static UnitDefaultConvertersMap m;
	return m;
}

void FromMayaPlugConverter::registerConverter( const MFnUnitAttribute::Type fromType, IECore::TypeId resultType, bool isDefaultConverter, CreatorFn creator )
{
	UnitTypesToFnsMap &m = unitTypesToFns();
	UnitTypesToFnsMap::const_iterator it = m.insert(UnitTypesToFnsMap::value_type( UnitTypePair( fromType, resultType ), creator ) ).first;
	if( isDefaultConverter )
	{
		UnitDefaultConvertersMap &dc = unitDefaultConverters();
		if( ! dc.insert( UnitDefaultConvertersMap::value_type( fromType, it ) ).second )
		{
			IECore::msg( IECore::Msg::Error, "FromMayaPlugConverter::registerConverter", boost::format( "Default conversion for MFnUnitAttribute::Type %d already registered - ignoring second registration." ) % fromType );
		}
	}
}

FromMayaConverterPtr FromMayaPlugConverter::create( const MPlug &plug, IECore::TypeId resultType )
{
	MObject attribute = plug.attribute();

	if( attribute.hasFn( MFn::kUnitAttribute ) )
	{
		MFnUnitAttribute fnUAttr( attribute );
		const UnitTypesToFnsMap &m = unitTypesToFns();
		UnitTypesToFnsMap::const_iterator it = m.find( UnitTypePair( fnUAttr.unitType(), resultType ) );
		if( it!=m.end() )
		{
			return it->second( plug );
		}

		UnitDefaultConvertersMap &dc = unitDefaultConverters();
		UnitDefaultConvertersMap::const_iterator dcIt = dc.find( fnUAttr.unitType() );
		if( dcIt != dc.end() )
		{
			if( resultType==IECore::InvalidTypeId || RunTimeTyped::inheritsFrom( dcIt->second->first.second, resultType ) )
			{
				return dcIt->second->second( plug );
			}
		}
	}

	if( attribute.hasFn( MFn::kNumericAttribute ) )
	{
		MFnNumericAttribute fnNAttr( attribute );
		const NumericTypesToFnsMap &m = numericTypesToFns();
		NumericTypesToFnsMap::const_iterator it = m.find( NumericTypePair( fnNAttr.unitType(), resultType ) );
		if( it!=m.end() )
		{
			return it->second( plug );
		}

		NumericDefaultConvertersMap &dc = numericDefaultConverters();
		NumericDefaultConvertersMap::const_iterator dcIt = dc.find( fnNAttr.unitType() );
		if( dcIt != dc.end() )
		{
			if( resultType==IECore::InvalidTypeId || RunTimeTyped::inheritsFrom( dcIt->second->first.second, resultType ) )
			{
				return dcIt->second->second( plug );
			}
		}
	}

	if( attribute.hasFn( MFn::kTypedAttribute ) )
	{
		MFnTypedAttribute fnTAttr( attribute );
		const TypedTypesToFnsMap &m = typedTypesToFns();
		TypedTypesToFnsMap::const_iterator it = m.find( TypedTypePair( fnTAttr.attrType(), resultType ) );
		if( it!=m.end() )
		{
			return it->second( plug );
		}

		TypedDefaultConvertersMap &dc = typedDefaultConverters();
		TypedDefaultConvertersMap::const_iterator dcIt = dc.find( fnTAttr.attrType() );
		if( dcIt != dc.end() )
		{
			if( resultType==IECore::InvalidTypeId || RunTimeTyped::inheritsFrom( dcIt->second->first.second, resultType ) )
			{
				return dcIt->second->second( plug );
			}
		}
	}

	MObject o;
	plug.getValue( o );
	if( resultType==IECore::InvalidTypeId )
	{
		return FromMayaObjectConverter::create( o );
	}
	else
	{
		return FromMayaObjectConverter::create( o, resultType );
	}
}

