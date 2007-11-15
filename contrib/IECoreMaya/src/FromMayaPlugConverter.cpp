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

#include <iostream>

#include "maya/MFnAttribute.h"
#include "maya/MFnNumericAttribute.h"
#include "maya/MFnUnitAttribute.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MString.h"
#include "maya/MTime.h"
#include "maya/MAngle.h"
#include "maya/MDistance.h"

#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"

#include "IECoreMaya/FromMayaObjectConverter.h"
#include "IECoreMaya/FromMayaPlugConverter.h"

using namespace IECoreMaya;
using namespace IECore;

FromMayaPlugConverter::FromMayaPlugConverter( const MPlug &plug )
	:	FromMayaConverter( "FromMayaPlugConverter", "Converts the value held in a maya plug." ), m_plug( plug )
{
}

const MPlug &FromMayaPlugConverter::plug() const
{
	return m_plug;
}

FromMayaPlugConverter::NumericTypesToFnsMap *FromMayaPlugConverter::numericTypesToFns()
{
	static NumericTypesToFnsMap *m = new NumericTypesToFnsMap;
	return m;
}

void FromMayaPlugConverter::registerConverter( const MFnNumericData::Type fromType, IECore::TypeId resultType, CreatorFn creator )
{
	NumericTypesToFnsMap *m = numericTypesToFns();
	(*m)[NumericTypePair( fromType, resultType )] = creator;
}

FromMayaPlugConverter::TypedTypesToFnsMap *FromMayaPlugConverter::typedTypesToFns()
{
	static TypedTypesToFnsMap *m = new TypedTypesToFnsMap;
	return m;
}

void FromMayaPlugConverter::registerConverter( const MFnData::Type fromType, IECore::TypeId resultType, CreatorFn creator )
{
	TypedTypesToFnsMap *m = typedTypesToFns();
	(*m)[TypedTypePair( fromType, resultType )] = creator;
}
		
FromMayaConverterPtr FromMayaPlugConverter::create( const MPlug &plug )
{
	return create( plug, IECore::InvalidTypeId );
}
		
FromMayaConverterPtr FromMayaPlugConverter::create( const MPlug &plug, IECore::TypeId resultType )
{
	MObject attribute = plug.attribute();
	if( attribute.hasFn( MFn::kNumericAttribute ) )
	{
		MFnNumericAttribute fnNAttr( attribute );
		const NumericTypesToFnsMap *m = numericTypesToFns();
		NumericTypesToFnsMap::const_iterator it = m->find( NumericTypePair( fnNAttr.unitType(), resultType ) );
		if( it!=m->end() )
		{
			return it->second( plug );
		}
	}
	
	if( attribute.hasFn( MFn::kTypedAttribute ) )
	{
		MFnTypedAttribute fnTAttr( attribute );
		const TypedTypesToFnsMap *m = typedTypesToFns();
		TypedTypesToFnsMap::const_iterator it = m->find( TypedTypePair( fnTAttr.attrType(), resultType ) );
		if( it!=m->end() )
		{
			return it->second( plug );
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

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// The code below is deprecated and due for removal in the next major version
////////////////////////////////////////////////////////////////////////////////////////////////////////

IECore::ObjectPtr FromMayaPlugConverter::doConversion( IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus s;
	
	if (m_plug.isNull())
	{
		return 0;
	}
	
	MObject attr = m_plug.attribute();
		
	if (attr.hasFn(MFn::kUnitAttribute))
	{
		MFnUnitAttribute fnAttr( attr, &s );
		
		switch (fnAttr.unitType())
		{
			case MFnUnitAttribute::kTime:
			{
				MTime t;
				
				s = m_plug.getValue(t);
				assert(s);
				
				FloatDataPtr data = new FloatData();				
				data->writable() = t.as( MTime::kSeconds );				
				return data;
			}
			case MFnUnitAttribute::kAngle:
			{
				MAngle a;
				
				s = m_plug.getValue(a);
				assert(s);
				
				FloatDataPtr data = new FloatData();				
				data->writable() = a.as( MAngle::kRadians );				
				return data;				
			}
			case MFnUnitAttribute::kDistance:
			{
				MDistance d;
				
				s = m_plug.getValue(d);
				assert(s);
				
				FloatDataPtr data = new FloatData();				
				data->writable() = d.as( MDistance::kCentimeters );				
				return data;				
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
				double d;
				s = m_plug.getValue(d);
				assert(s);
				
				DoubleDataPtr data = new DoubleData();
				data->writable() = d;
				return data;
			}		
			case MFnNumericData::kFloat:
			{
				float f;
				s = m_plug.getValue(f);
				assert(s);
				
				FloatDataPtr data = new FloatData();
				data->writable() = f;
				return data;
			}		
			case MFnNumericData::kInt:
			{
				int i;
				s = m_plug.getValue(i);
				assert(s);
				
				IntDataPtr data = new IntData();
				data->writable() = i;
				return data;
			}
			case MFnNumericData::kBoolean:
			{
				bool b;
				s = m_plug.getValue(b);
				assert(s);
				
				IntDataPtr data = new IntData();
				data->writable() = b ? 1 : 0;
				return data;
			}
			case MFnNumericData::kChar:
			{
				char c;
				s = m_plug.getValue(c);
				assert(s);
				
				IntDataPtr data = new IntData();
				data->writable() = static_cast<int>(c);
				return data;
			}			
			case MFnNumericData::kShort:
			case MFnNumericData::kByte:
			{
				short s1;
				s = m_plug.getValue(s1);
				assert(s);
				
				IntDataPtr data = new IntData();
				data->writable() = static_cast<int>(s1);
				return data;
			}	
			default:
				// Fall-through to MObject conversion.
				break;
		}
		
	}
	else if (attr.hasFn( MFn::kCompoundAttribute ) )
	{
		assert( m_plug.isCompound() );
		
		/// \todo Recurse through attribute conveting all children, storing in a CompoundData.
		/// Finally go through and  combine into more complex types, as appropriate. e.g.
		/// Turn 3 x FloatData into 1 x V3fData
	}
	else if (attr.hasFn(MFn::kEnumAttribute))
	{
		short s1;
		
		s = m_plug.getValue(s1);
		assert(s);
				
		IntDataPtr data = new IntData();
		data->writable() = static_cast<int>(s1);
		return data;
	}
	
	try
	{
		MObject value;
		s = m_plug.getValue( value );
		assert(s);
		
		FromMayaObjectConverterPtr objectConverter = FromMayaObjectConverter::create( value );

		if (objectConverter)
		{
			return objectConverter->convert();
		}
	}
	catch (...)
	{
	}
	
	return 0;
}
