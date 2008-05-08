//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/ToMayaObjectConverter.h"
#include "IECoreMaya/ToMayaPlugConverter.h"

using namespace IECoreMaya;
using namespace IECore;
using namespace boost;

ToMayaPlugConverter::ToMayaPlugConverter( ConstObjectPtr object )
	:	ToMayaConverter( "ToMayaPlugConverter", "Places values in plugs.", IECore::ObjectTypeId )
{
	srcParameter()->setValue( const_pointer_cast<Object>( object ) );
}

ToMayaPlugConverterPtr ToMayaPlugConverter::create( const IECore::ObjectPtr src )
{
	return new ToMayaPlugConverter( src );
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

				ConstFloatDataPtr data = runTimeCast<const FloatData>( toConvert );
				if (data == 0) 
				{
					return false;
				}

				MTime t;
				t.setUnit( MTime::kSeconds );
				t.setValue( data->readable() );
				
				s = plug.setValue(t);
				return (s);
			}
			case MFnUnitAttribute::kAngle:
			{
				ConstFloatDataPtr data = runTimeCast<const FloatData>( toConvert );
				if (data == 0) 
				{
					return false;
				}

				MAngle a;
				a.setUnit( MAngle::kRadians );
				a.setValue( data->readable() );
				
				s = plug.setValue(a);
				return (s);
			}
			case MFnUnitAttribute::kDistance:
			{
				ConstFloatDataPtr data = runTimeCast<const FloatData>( toConvert );
				if (data == 0) 
				{
					return false;
				}

				MDistance d;
				d.setUnit( MDistance::kCentimeters );
				d.setValue( data->readable() );
				
				s = plug.setValue(d);
				return (s);
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
				ConstDoubleDataPtr data = runTimeCast<const DoubleData>( toConvert );
				if (data == 0)
				{
					return false;
				}

				s = plug.setValue( data->readable() );
				return (s);
			}		
			case MFnNumericData::kFloat:
			{
				ConstFloatDataPtr data = runTimeCast<const FloatData>( toConvert );
				if (data == 0) 
				{
					return false;
				}

				s = plug.setValue( data->readable() );
				return (s);
			}		
			case MFnNumericData::kInt:
			{
				ConstIntDataPtr data = runTimeCast<const IntData>( toConvert );
				if (data == 0)
				{
					return false;
				}

				s = plug.setValue( data->readable() );
				return (s);
			}
			case MFnNumericData::kBoolean:
			{
				ConstIntDataPtr data = runTimeCast<const IntData>( toConvert );
				if (data == 0)
				{
					return false;
				}

				s = plug.setValue( static_cast<bool>( data->readable() ) );
				return (s);
			}
			case MFnNumericData::kChar:
			{
				ConstIntDataPtr data = runTimeCast<const IntData>( toConvert );
				if (data == 0)
				{
					return false;
				}

				s = plug.setValue( static_cast<char>( data->readable() ) );
				return (s);
			}			
			case MFnNumericData::kShort:
			case MFnNumericData::kByte:
			{
				ConstIntDataPtr data = runTimeCast<const IntData>( toConvert );
				if (data == 0)
				{
					return false;
				}

				s = plug.setValue( static_cast<short>( data->readable() ) );
				return (s);
			}	
			default:
				// Fall-through to MObject conversion.
				break;
		}
		
	}
	else if (attr.hasFn(MFn::kEnumAttribute))
	{
		ConstIntDataPtr data = runTimeCast<const IntData>( toConvert );
		if (data == 0)
		{
			return false;
		}

		s = plug.setValue( (short) data->readable() );
		return (s);
	}
	
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
