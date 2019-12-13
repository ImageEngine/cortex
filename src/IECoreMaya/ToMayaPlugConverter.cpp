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
#include "maya/MString.h"
#include "maya/MTime.h"
#include "maya/MAngle.h"
#include "maya/MDistance.h"
#include "maya/MFnPluginData.h"

#include "IECore/CompoundData.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

#include "IECoreMaya/ObjectData.h"
#include "IECoreMaya/ToMayaObjectConverter.h"
#include "IECoreMaya/ToMayaPlugConverter.h"

#include "boost/format.hpp"

using namespace IECoreMaya;
using namespace IECore;
using namespace boost;

IE_CORE_DEFINERUNTIMETYPED( ToMayaPlugConverter );

ToMayaPlugConverter::ToMayaPlugConverter( ConstObjectPtr object )
	:	ToMayaConverter( "Places values in plugs.", IECore::ObjectTypeId )
{
	srcParameter()->setValue( boost::const_pointer_cast<Object>( object ) );
}

ToMayaPlugConverterPtr ToMayaPlugConverter::create( const IECore::ObjectPtr src )
{
	return new ToMayaPlugConverter( src );
}

template <typename DataType, typename UnitType>
MStatus ToMayaPlugConverter::setPlugValue(MPlug &plug, const DataType &data)
{
	return plug.setValue(data);
}


// specializations for Unit Attributes
namespace IECoreMaya
{

template <>
MStatus ToMayaPlugConverter::setPlugValue<float, MTime>(MPlug &plug, const float &data)
{
	MTime t;
	t.setUnit(MTime::kSeconds);
	t.setValue(data);
	return plug.setValue(t);
}

template <>
MStatus ToMayaPlugConverter::setPlugValue<float, MDistance>(MPlug &plug, const float &data)
{
	MDistance t;
	t.setUnit(MDistance::kCentimeters);
	t.setValue(data);
	return plug.setValue(t);
}

template <>
MStatus ToMayaPlugConverter::setPlugValue<float, MAngle>(MPlug &plug, const float &data)
{
	MAngle t;
	t.setUnit(MAngle::kRadians);
	t.setValue(data);
	return plug.setValue(t);
}

} // namespace IECoreMaya


template <typename DataType, typename UnitType>
MStatus ToMayaPlugConverter::convertAttr(MPlug &plug) const
{
	MStatus s;
	if (plug.isArray())
	{
		IECore::CompoundData::ConstPtr data = IECore::runTimeCast<const IECore::CompoundData>( srcParameter()->getValidatedValue() );
		if (!data)
		{
			return s;
		}

		typedef TypedData< std::vector<DataType> > VecDataType;
		const VecDataType *indexedData = data->member<VecDataType>( "data", true );
		const IntVectorData *indices = data->member<IntVectorData>( "indices", true );

		// set all indexed values

		const auto &dataReadable = indexedData->readable();
		const auto &indicesReadable = indices->readable();
		size_t dataSize = dataReadable.size();
		size_t indicesSize = indicesReadable.size();
		if ( dataSize != indicesSize )
		{
			throw Exception(boost::str(boost::format( "Error setting array data for\"%s\" . Length of indices and data array do not match." ) % plug.info() ));
		}

		for( size_t i=0; i < dataSize; ++i )
		{
			MPlug currentPlug = plug.elementByLogicalIndex(indicesReadable[i]);
			s = setPlugValue<DataType, UnitType>(currentPlug, dataReadable[i]);
			if (s != MStatus::kSuccess)	return s;
		}
	}
	else if( typename IECore::TypedData<DataType>::ConstPtr data = runTimeCast<const TypedData<DataType> >( srcParameter()->getValidatedValue() ) )
	{
		return setPlugValue<DataType, UnitType>( plug, data->readable() );
	}
	return s;
}

bool ToMayaPlugConverter::convert( MPlug &plug ) const
{
	ConstObjectPtr toConvert = srcParameter()->getValidatedValue();
	MStatus s;
	MObject attr = plug.attribute();

	// uses the same types used in FromMayaPlugConverter.
	if (attr.hasFn(MFn::kUnitAttribute))
	{
		MFnUnitAttribute fnAttr( attr, &s );

		switch (fnAttr.unitType())
		{
			case MFnUnitAttribute::kTime:
			{
				return convertAttr<double, MTime>(plug);
			}
			case MFnUnitAttribute::kAngle:
			{
				return convertAttr<double, MAngle>(plug);
			}
			case MFnUnitAttribute::kDistance:
			{
				return convertAttr<double, MDistance>(plug);
			}
			default:
				return 0;
		}
	}
	else if (attr.hasFn( MFn::kNumericAttribute ) )
	{
		MFnNumericAttribute fnAttr( attr, &s );
		assert(s);

		switch (fnAttr.unitType())
		{
			case MFnNumericData::kDouble:
			{
				return (convertAttr<double, void>(plug));
			}
			case MFnNumericData::kFloat:
			{
				return (convertAttr<float, void>(plug));
			}
			case MFnNumericData::kInt:
			{
				return (convertAttr<int, void>(plug));
			}
			case MFnNumericData::kBoolean:
			{
				return (convertAttr<bool, void>(plug));
			}
			case MFnNumericData::kChar:
			{
				return (convertAttr<char,void>(plug));
			}
			case MFnNumericData::kShort:
			case MFnNumericData::kByte:
			{
				return (convertAttr<short, void>(plug));
			}
			default:
				// Fall-through to MObject conversion.
				break;
		}

	}
	else if (attr.hasFn(MFn::kEnumAttribute))
	{
		return (convertAttr<short, void>(plug));
	}

	// simple types failed - see if it'll accept ObjectData
	MFnAttribute fnAttr( attr );
	if( fnAttr.accepts( ObjectData::id ) )
	{
		MFnPluginData fnData;
		MObject data = fnData.create( ObjectData::id );

		ObjectData *oData = dynamic_cast<ObjectData *>( fnData.data() );
		oData->setObject( toConvert->copy() );

		return plug.setValue( data );
	}

	// doesn't accept ObjectData, have a bash at storing
	// an arbitrary MObject.

	try
	{
		MObject value;
		s = plug.getValue( value );
		assert(s);

		ToMayaObjectConverterPtr objectConverter = ToMayaObjectConverter::create( toConvert );

		if (objectConverter)
		{
			if ( !objectConverter->convert( value ) )
			{
				return false;
			}
			s = plug.setValue( value );
			return (s);
		}
	}
	catch (...)
	{
	}

	return false;
}
