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

#include "maya/MVectorArray.h"
#include "maya/MPointArray.h"
#include "maya/MStringArray.h"

#include "IECore/VectorTypedData.h"

#include "IECoreMaya/Convert.h"
#include "IECoreMaya/MArrayTraits.h"
#include "IECoreMaya/ToMayaArrayDataConverter.h"

using namespace IECoreMaya;

template<typename F, typename T>
ToMayaObjectConverter::ToMayaObjectConverterDescription< ToMayaArrayDataConverter<F, T> > ToMayaArrayDataConverter<F, T>::g_description( F::staticTypeId(), MArrayTraits<T>::dataType() );

template<typename F, typename T>
ToMayaArrayDataConverter<F, T>::ToMayaArrayDataConverter( IECore::ConstObjectPtr object ) 
: ToMayaObjectConverter( "ToMayaArrayDataConverter", "Converts IECore::VectorData objects to a Maya object.", object)
{
}

template<typename F, typename T>
bool ToMayaArrayDataConverter<F, T>::doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const
{
	MStatus s;
	
	boost::intrusive_ptr< const F > dataPtr = IECore::runTimeCast<const F>(from);
	assert( dataPtr );
	
	typename MArrayTraits<T>::DataFn fnData;
	
	const typename F::ValueType &v = dataPtr->readable();
	
	T array;
	array.setLength( v.size() );
	
	for (unsigned i = 0; i < v.size(); i++)
	{
		array[i] = IECore::convert< typename MArrayTraits<T>::ValueType, typename F::ValueType::value_type >( v[i] );
	}
		
	to = fnData.create( array, &s );
	
	return s;
}

template class ToMayaArrayDataConverter< IECore::V3fVectorData, MVectorArray >;
template class ToMayaArrayDataConverter< IECore::V3dVectorData, MVectorArray >;
template class ToMayaArrayDataConverter< IECore::V3fVectorData, MPointArray >;
template class ToMayaArrayDataConverter< IECore::V3dVectorData, MPointArray >;
template class ToMayaArrayDataConverter< IECore::StringVectorData, MStringArray >;
template class ToMayaArrayDataConverter< IECore::DoubleVectorData, MDoubleArray >;
