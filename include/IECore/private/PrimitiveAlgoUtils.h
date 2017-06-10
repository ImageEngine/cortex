//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2017, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_PRIMITIVEALGOUTILS_H
#define IECORE_PRIMITIVEALGOUTILS_H

#include <numeric>

#include "boost/mpl/and.hpp"
#include "IECore/TypeTraits.h"

namespace IECore
{
namespace Detail
{
//@formatter:off
template< typename T > struct IsArithmeticVectorTypedData
	: boost::mpl::and_
	<
		IECore::TypeTraits::IsNumericBasedVectorTypedData<T>,
		boost::mpl::not_< IECore::TypeTraits::IsBox<typename TypeTraits::VectorValueType<T>::type > >,
		boost::mpl::not_< IECore::TypeTraits::IsQuat<typename TypeTraits::VectorValueType<T>::type > >
	>
{};
//@formatter:on

struct  AverageValueFromVector
{
	typedef DataPtr ReturnType;

	template<typename From> ReturnType operator()( typename From::ConstPtr data )
	{
		const typename From::ValueType &src = data->readable();
		if ( src.size() )
		{
			return new TypedData< typename From::ValueType::value_type >( std::accumulate( src.begin() + 1, src.end(), *src.begin() ) / src.size() );
		}
		return NULL;
	}
};


inline DataPtr createArrayData(PrimitiveVariable& primitiveVariable, const Primitive *primitive, PrimitiveVariable::Interpolation interpolation)
{
	if ( primitiveVariable.interpolation != PrimitiveVariable::Constant )
		return NULL;

	size_t len = primitive->variableSize( interpolation );
	switch( primitiveVariable.data->typeId() )
	{
		case IntDataTypeId:
		{
			IntVectorDataPtr newData = new IntVectorData();
			newData->writable().resize( len, static_cast< const IntData * >( primitiveVariable.data.get() )->readable() );
			return newData;
		}
			break;
		case FloatDataTypeId:
		{
			FloatVectorDataPtr newData = new FloatVectorData();
			newData->writable().resize( len, static_cast< const FloatData * >( primitiveVariable.data.get() )->readable() );
			return newData;
		}
			break;
		case V2fDataTypeId:
		{
			V2fVectorDataPtr newData = new V2fVectorData();
			newData->writable().resize( len, static_cast< const V2fData * >( primitiveVariable.data.get() )->readable() );
			return newData;
		}
			break;
		case V3fDataTypeId:
		{
			V3fVectorDataPtr newData = new V3fVectorData();
			newData->writable().resize( len, static_cast< const V3fData * >( primitiveVariable.data.get() )->readable() );
			return newData;
		}
			break;
		case Color3fDataTypeId:
		{
			Color3fVectorDataPtr newData = new Color3fVectorData();
			newData->writable().resize( len, static_cast< const Color3fData * >( primitiveVariable.data.get() )->readable() );
			primitiveVariable = PrimitiveVariable(interpolation, newData);
		}
			break;
		default:
			return NULL;
	}

	return NULL;
}

} // namespace Detail
} // namespace IECore

#endif // IECORE_PRIMITIVEALGOUTILS_H
