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

#include "IECoreMaya/FromMayaArrayDataConverter.h"
#include "IECoreMaya/MArrayTraits.h"
#include "IECoreMaya/Convert.h"

#include "IECore/VectorTypedData.h"

using namespace IECore;

namespace IECoreMaya
{

template<typename F, typename T>
FromMayaObjectConverter::FromMayaObjectConverterDescription<FromMayaArrayDataConverter<F,T> > FromMayaArrayDataConverter<F,T>::m_description( MArrayTraits<F>::dataType(), T::staticTypeId() );

template<typename F, typename T>
FromMayaArrayDataConverter<F,T>::FromMayaArrayDataConverter( const MObject &object )
	:	FromMayaObjectConverter( "FromMayaArrayDataConverter", "Converts maya array data types to IECore::TypedVectorData types.", object )
{
}

template<typename F, typename T>
IECore::ObjectPtr FromMayaArrayDataConverter<F,T>::doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const
{
	typename MArrayTraits<F>::DataFn fnArrayData( object );
	if( !fnArrayData.hasObj( object ) )
	{
		return 0;
	}

	F array = fnArrayData.array();
	typename T::Ptr resultData = new T;
	typename T::ValueType &resultArray = resultData->writable();

	resultArray.resize( array.length() );
	for( unsigned int i=0; i<resultArray.size(); i++ )
	{
		resultArray[i] = IECore::convert<typename T::ValueType::value_type, typename MArrayTraits<F>::ValueType>( array[i] );
	}

	return resultData;
}

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaArrayDataConverterii, FromMayaArrayDataConverteriiTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaArrayDataConverterdd, FromMayaArrayDataConverterddTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaArrayDataConverterdf, FromMayaArrayDataConverterdfTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaArrayDataConverterss, FromMayaArrayDataConverterssTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaArrayDataConverterVV3f, FromMayaArrayDataConverterVV3fTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaArrayDataConverterVV3d, FromMayaArrayDataConverterVV3dTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaArrayDataConverterVC3f, FromMayaArrayDataConverterVC3fTypeId )

/// Explicit instantiations.
template class FromMayaArrayDataConverter<MIntArray, IntVectorData>;
template class FromMayaArrayDataConverter<MDoubleArray, DoubleVectorData>;
template class FromMayaArrayDataConverter<MDoubleArray, FloatVectorData>;
template class FromMayaArrayDataConverter<MStringArray, StringVectorData>;
template class FromMayaArrayDataConverter<MVectorArray, V3fVectorData>;
template class FromMayaArrayDataConverter<MVectorArray, V3dVectorData>;
template class FromMayaArrayDataConverter<MVectorArray, Color3fVectorData>;

} // namespace IECoreMaya
