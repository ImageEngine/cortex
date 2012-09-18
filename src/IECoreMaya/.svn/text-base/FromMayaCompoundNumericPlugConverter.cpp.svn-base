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

#include "IECoreMaya/FromMayaCompoundNumericPlugConverter.h"
#include "IECoreMaya/NumericTraits.h"

#include "IECore/SimpleTypedData.h"
#include "IECore/MessageHandler.h"

#include "boost/type_traits/is_same.hpp"

using namespace IECore;
using namespace Imath;
using namespace boost;

namespace IECoreMaya
{

template<typename F, typename T>
FromMayaPlugConverter::Description<FromMayaCompoundNumericPlugConverter<F,T> > FromMayaCompoundNumericPlugConverter<F,T>::m_description( NumericTraits<F>::dataType(), T::staticTypeId(), is_same<F, typename T::ValueType>::value );

template<typename F, typename T>
FromMayaCompoundNumericPlugConverter<F,T>::FromMayaCompoundNumericPlugConverter( const MPlug &plug )
	:	FromMayaPlugConverter( plug )
{
}

template<typename F, typename T>
IECore::ObjectPtr FromMayaCompoundNumericPlugConverter<F,T>::doConversion( IECore::ConstCompoundObjectPtr operands ) const
{
	typename F::BaseType v;
	typename T::ValueType vv;
	for( unsigned int i=0; i<F::dimensions(); i++ )
	{
		plug().child( i ).getValue( v );
		vv[i] = (typename T::ValueType::BaseType) v;
	}
	return new T( vv );
}

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaCompoundNumericPlugConverterV2fV2i, FromMayaCompoundNumericPlugConverterV2fV2iTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaCompoundNumericPlugConverterV2fV2f, FromMayaCompoundNumericPlugConverterV2fV2fTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaCompoundNumericPlugConverterV2fV2d, FromMayaCompoundNumericPlugConverterV2fV2dTypeId )

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaCompoundNumericPlugConverterV2dV2i, FromMayaCompoundNumericPlugConverterV2dV2iTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaCompoundNumericPlugConverterV2dV2f, FromMayaCompoundNumericPlugConverterV2dV2fTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaCompoundNumericPlugConverterV2dV2d, FromMayaCompoundNumericPlugConverterV2dV2dTypeId )

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaCompoundNumericPlugConverterV3fV3i, FromMayaCompoundNumericPlugConverterV3fV3iTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaCompoundNumericPlugConverterV3fV3f, FromMayaCompoundNumericPlugConverterV3fV3fTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaCompoundNumericPlugConverterV3fV3d, FromMayaCompoundNumericPlugConverterV3fV3dTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaCompoundNumericPlugConverterV3fC3f, FromMayaCompoundNumericPlugConverterV3fC3fTypeId )

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaCompoundNumericPlugConverterV3dV3i, FromMayaCompoundNumericPlugConverterV3dV3iTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaCompoundNumericPlugConverterV3dV3f, FromMayaCompoundNumericPlugConverterV3dV3fTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaCompoundNumericPlugConverterV3dV3d, FromMayaCompoundNumericPlugConverterV3dV3dTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaCompoundNumericPlugConverterV3dC3f, FromMayaCompoundNumericPlugConverterV3dC3fTypeId )

/// Explicit instantiations.
template class FromMayaCompoundNumericPlugConverter<V2f, V2iData>;
template class FromMayaCompoundNumericPlugConverter<V2f, V2fData>;
template class FromMayaCompoundNumericPlugConverter<V2f, V2dData>;

template class FromMayaCompoundNumericPlugConverter<V2d, V2iData>;
template class FromMayaCompoundNumericPlugConverter<V2d, V2fData>;
template class FromMayaCompoundNumericPlugConverter<V2d, V2dData>;

template class FromMayaCompoundNumericPlugConverter<V3f, V3iData>;
template class FromMayaCompoundNumericPlugConverter<V3f, V3fData>;
template class FromMayaCompoundNumericPlugConverter<V3f, V3dData>;
template class FromMayaCompoundNumericPlugConverter<V3f, Color3fData>;

template class FromMayaCompoundNumericPlugConverter<V3d, V3iData>;
template class FromMayaCompoundNumericPlugConverter<V3d, V3fData>;
template class FromMayaCompoundNumericPlugConverter<V3d, V3dData>;
template class FromMayaCompoundNumericPlugConverter<V3d, Color3fData>;

} // namespace IECoreMaya
