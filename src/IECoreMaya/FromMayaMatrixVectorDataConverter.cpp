//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#include "IECore\CompoundObject.h"
#include "IECoreMaya/FromMayaMatrixVectorDataConverter.h"

#include "IECore/VectorTypedData.h"

#include "maya/MDoubleArray.h"
#include "maya/MFnDoubleArrayData.h"

using namespace IECoreMaya;
using namespace IECore;

template<>
FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaMatrixVectorDataConverter<M44dVectorData> > FromMayaMatrixVectorDataConverter<M44dVectorData>::m_description( MFn::kDoubleArrayData, M44dVectorData::staticTypeId(), false );
template<>
FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaMatrixVectorDataConverter<M44fVectorData> > FromMayaMatrixVectorDataConverter<M44fVectorData>::m_description( MFn::kDoubleArrayData, M44fVectorData::staticTypeId(), false );

template<typename T>
FromMayaMatrixVectorDataConverter<T>::FromMayaMatrixVectorDataConverter( const MObject &object )
	:	FromMayaObjectConverter( "Converts maya double array data to IECore::M44*VectorData types.", object )
{
}

template<typename T>
IECore::ObjectPtr FromMayaMatrixVectorDataConverter<T>::doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	MFnDoubleArrayData fnArrayData( object );
	if( !fnArrayData.hasObj( object ) )
	{
		return 0;
	}
	if( ( fnArrayData.length() % 16 ) != 0 )
	{
		return 0;
	}
	
	MDoubleArray array = fnArrayData.array();
	typename T::Ptr resultData = new T;
	typename T::ValueType &resultArray = resultData->writable();
	
	resultArray.resize( array.length() / 16 );
	for ( unsigned int i=0; i<resultArray.size(); i++ )
	{
		for ( unsigned int j=0; j < 4; j++ )
		{
			for ( unsigned int k=0; k < 4; k++ )
			{
				resultArray[i][j][k] = array[ i*16 + j*4 + k ];
			}
		}
	}

	return resultData;
}

/// Explicit instantiations.
template class FromMayaMatrixVectorDataConverter<M44fVectorData>;
template class FromMayaMatrixVectorDataConverter<M44dVectorData>;

