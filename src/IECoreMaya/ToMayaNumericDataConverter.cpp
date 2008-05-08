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

#include "IECoreMaya/ToMayaNumericDataConverter.h"

#include "IECore/SimpleTypedData.h"

#include "maya/MFn.h"
#include "maya/MFnNumericData.h"

#include <algorithm>

using namespace IECoreMaya;
using namespace IECore;
using namespace std;
using namespace Imath;
using namespace boost;

// 3d vector conversion
ToMayaObjectConverter::ToMayaObjectConverterDescription<ToMayaNumericDataConverter> ToMayaNumericDataConverter::g_3Double( V3dDataTypeId, MFn::kData3Double );
ToMayaObjectConverter::ToMayaObjectConverterDescription<ToMayaNumericDataConverter> ToMayaNumericDataConverter::g_3Float( V3fDataTypeId, MFn::kData3Float );
ToMayaObjectConverter::ToMayaObjectConverterDescription<ToMayaNumericDataConverter> ToMayaNumericDataConverter::g_3Int( V3iDataTypeId, MFn::kData3Int );
ToMayaObjectConverter::ToMayaObjectConverterDescription<ToMayaNumericDataConverter> ToMayaNumericDataConverter::g_3Short( V3iDataTypeId, MFn::kData3Short );

// 2d vector conversion
ToMayaObjectConverter::ToMayaObjectConverterDescription<ToMayaNumericDataConverter> ToMayaNumericDataConverter::g_2Double( V2dDataTypeId, MFn::kData2Double );
ToMayaObjectConverter::ToMayaObjectConverterDescription<ToMayaNumericDataConverter> ToMayaNumericDataConverter::g_2Float( V2fDataTypeId, MFn::kData2Float );
ToMayaObjectConverter::ToMayaObjectConverterDescription<ToMayaNumericDataConverter> ToMayaNumericDataConverter::g_2Int( V2iDataTypeId, MFn::kData2Int );
ToMayaObjectConverter::ToMayaObjectConverterDescription<ToMayaNumericDataConverter> ToMayaNumericDataConverter::g_2Short( V2iDataTypeId, MFn::kData2Short );


ToMayaNumericDataConverter::ToMayaNumericDataConverter( ConstObjectPtr object )
	:	ToMayaObjectConverter( "ToMayaNumericDataConverter", "Converts IECore::Data types to maya numeric data objects.", object )
{
}
		
bool ToMayaNumericDataConverter::doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus s;
	MFnNumericData fnData( to, &s );
	
	switch ( fnData.numericType() )
	{
		case MFnNumericData::k2Short:
		{
			ConstV2iDataPtr data = runTimeCast<const V2iData>( from );
			if (data == 0)
			{
				return false;
			}			

			s = fnData.setData( (short)data->readable().x, (short)data->readable().y );
			return (s);			
		}	
		case MFnNumericData::k3Short:
		{
			ConstV3iDataPtr data = runTimeCast<const V3iData>( from );
			if (data == 0)
			{
				return false;
			}			

			s = fnData.setData( (short)data->readable().x, (short)data->readable().y, (short)data->readable().z );
			return (s);			
		}	
		case MFnNumericData::k2Int:
		{
			ConstV2iDataPtr data = runTimeCast<const V2iData>( from );
			if (data == 0)
			{
				return false;
			}			

			s = fnData.setData( (int)data->readable().x, (int)data->readable().y );
			return (s);			
		}	
		case MFnNumericData::k3Int:
		{
			ConstV3iDataPtr data = runTimeCast<const V3iData>( from );
			if (data == 0)
			{
				return false;
			}			

			s = fnData.setData( (int)data->readable().x, (int)data->readable().y, (int)data->readable().z );
			return (s);		
		}	
		case MFnNumericData::k2Float:
		{
			ConstV2fDataPtr data = runTimeCast<const V2fData>( from );
			if (data == 0)
			{
				return false;
			}			

			s = fnData.setData( data->readable().x, data->readable().y );
			return (s);
		}
		case MFnNumericData::k3Float:
		{
			ConstV3fDataPtr data = runTimeCast<const V3fData>( from );
			if (data == 0)
			{
				return false;
			}			

			s = fnData.setData( data->readable().x, data->readable().y, data->readable().z );
			return (s);
		}
		case MFnNumericData::k2Double:
		{
			ConstV2dDataPtr data = runTimeCast<const V2dData>( from );
			if (data == 0)
			{
				return false;
			}			

			s = fnData.setData( data->readable().x, data->readable().y );
			return (s);
		}		
		case MFnNumericData::k3Double:
		{
			ConstV3dDataPtr data = runTimeCast<const V3dData>( from );
			if (data == 0)
			{
				return false;
			}			

			s = fnData.setData( data->readable().x, data->readable().y, data->readable().z );
			return (s);
		}		
		case MFnNumericData::kDouble:		
		case MFnNumericData::kFloat:		
		case MFnNumericData::kInt:		
		case MFnNumericData::kBoolean:
		case MFnNumericData::kByte:
		case MFnNumericData::kChar:
		case MFnNumericData::kShort:		
		default:
			return false;
	}	
}
