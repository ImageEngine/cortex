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

#include "IECoreMaya/FromMayaNumericPlugConverter.h"
#include "IECoreMaya/NumericTraits.h"

#include "IECore/SimpleTypedData.h"
#include "IECore/MessageHandler.h"

#include "boost/type_traits/is_same.hpp"

using namespace IECoreMaya;
using namespace IECore;
using namespace Imath;
using namespace boost;

template<typename F, typename T>
FromMayaPlugConverter::Description<FromMayaNumericPlugConverter<F,T> > FromMayaNumericPlugConverter<F,T>::m_description( NumericTraits<F>::dataType(), T::staticTypeId(), is_same<F, typename T::ValueType>::value );

template<typename F, typename T>
FromMayaNumericPlugConverter<F,T>::FromMayaNumericPlugConverter( const MPlug &plug )
	:	FromMayaPlugConverter( plug )
{
}

template<typename F, typename T>
IECore::ObjectPtr FromMayaNumericPlugConverter<F,T>::doConversion( IECore::ConstCompoundObjectPtr operands ) const
{
	F v;
	plug().getValue( v );
	return new T( (typename T::ValueType)v );
}

/// Explicit instantiations.
template class FromMayaNumericPlugConverter<bool, BoolData>;
template class FromMayaNumericPlugConverter<bool, IntData>;
template class FromMayaNumericPlugConverter<int, IntData>;
template class FromMayaNumericPlugConverter<int, FloatData>;
template class FromMayaNumericPlugConverter<int, DoubleData>;
template class FromMayaNumericPlugConverter<float, IntData>;
template class FromMayaNumericPlugConverter<float, FloatData>;
template class FromMayaNumericPlugConverter<float, DoubleData>;
template class FromMayaNumericPlugConverter<double, IntData>;
template class FromMayaNumericPlugConverter<double, FloatData>;
template class FromMayaNumericPlugConverter<double, DoubleData>;
