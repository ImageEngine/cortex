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

#include "IECoreMaya/FromMayaNumericDataConverter.h"

#include "IECore/SimpleTypedData.h"

#include "maya/MFn.h"
#include "maya/MFnNumericData.h"

#include <algorithm>

using namespace IECoreMaya;
using namespace IECore;
using namespace std;
using namespace Imath;

// 3d vector conversion
FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaNumericDataConverter> FromMayaNumericDataConverter::g_3Double( MFn::kData3Double, V3dDataTypeId );
FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaNumericDataConverter> FromMayaNumericDataConverter::g_3Float( MFn::kData3Float, V3fDataTypeId );
FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaNumericDataConverter> FromMayaNumericDataConverter::g_3Int( MFn::kData3Int, V3iDataTypeId );
FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaNumericDataConverter> FromMayaNumericDataConverter::g_3Short( MFn::kData3Short, V3iDataTypeId );
// 2d vector conversion
FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaNumericDataConverter> FromMayaNumericDataConverter::g_2Double( MFn::kData2Double, V2dDataTypeId );
FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaNumericDataConverter> FromMayaNumericDataConverter::g_2Float( MFn::kData2Float, V2fDataTypeId );
FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaNumericDataConverter> FromMayaNumericDataConverter::g_2Int( MFn::kData2Int, V2iDataTypeId );
FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaNumericDataConverter> FromMayaNumericDataConverter::g_2Short( MFn::kData2Short, V3iDataTypeId );


FromMayaNumericDataConverter::FromMayaNumericDataConverter( const MObject &object )
	:	FromMayaObjectConverter( "FromMayaNumericDataConverter", "Converts types compatible with MFnNumericData.", object )
{
}
		
IECore::ObjectPtr FromMayaNumericDataConverter::doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus s;
	MFnNumericData fnData( object, &s );
	
	switch (fnData.numericType())
	{
		case MFnNumericData::k2Short:
		{
			short s1, s2;
			s = fnData.getData( s1, s2 );
			assert(s);			
			
			V2iDataPtr data = new V2iData();
			data->writable().x = s1;
			data->writable().y = s2;						
			
			return data;			
		}	
		case MFnNumericData::k3Short:
		{
			short s1, s2, s3;
			s = fnData.getData( s1, s2, s3 );
			assert(s);			
			
			V3iDataPtr data = new V3iData();
			data->writable().x = s1;
			data->writable().y = s2;
			data->writable().z = s3;
			
			return data;			
		}	
		case MFnNumericData::k2Int:
		{
			int i1, i2;
			s = fnData.getData( i1, i2 );
			assert(s);			
			
			V2iDataPtr data = new V2iData();
			data->writable().x = i1;
			data->writable().y = i2;						
			
			return data;			
		}	
		case MFnNumericData::k3Int:
		{
			int i1, i2, i3;
			s = fnData.getData( i1, i2, i3);
			assert(s);			
			
			V3iDataPtr data = new V3iData();
			data->writable().x = i1;
			data->writable().y = i2;
			data->writable().z = i3;
			
			return data;			
		}	
		case MFnNumericData::k2Float:
		{
			float f1, f2;
			s = fnData.getData( f1, f2 );
			assert(s);			
			
			V2fDataPtr data = new V2fData();
			data->writable().x = f1;
			data->writable().y = f2;						
			
			return data;			
		}
		case MFnNumericData::k3Float:
		{
			float f1, f2, f3;
			s = fnData.getData( f1, f2, f3 );
			assert(s);			
			
			V3fDataPtr data = new V3fData();
			data->writable().x = f1;
			data->writable().y = f2;
			data->writable().z = f3;
			
			return data;			
		}
		case MFnNumericData::k2Double:
		{
			double d1, d2;
			s = fnData.getData( d1, d2 );
			assert(s);			
			
			V2dDataPtr data = new V2dData();
			data->writable().x = d1;
			data->writable().y = d2;						
			
			return data;			
		}		
		case MFnNumericData::k3Double:
		{
			double d1, d2, d3;
			s = fnData.getData( d1, d2, d3 );
			assert(s);			
			
			V3dDataPtr data = new V3dData();
			data->writable().x = d1;
			data->writable().y = d2;
			data->writable().z = d3;
			
			return data;			
		}		
		case MFnNumericData::kDouble:		
		case MFnNumericData::kFloat:		
		case MFnNumericData::kInt:		
		case MFnNumericData::kBoolean:
		case MFnNumericData::kByte:
		case MFnNumericData::kChar:
		case MFnNumericData::kShort:		
		default:
			return 0;
	}	
}
