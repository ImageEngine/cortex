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

#ifndef IECORESCENE_PRIMITIVEALGOUTILS_H
#define IECORESCENE_PRIMITIVEALGOUTILS_H

#include <numeric>

#include "boost/mpl/and.hpp"
#include "IECore/TypeTraits.h"

namespace IECoreScene
{
namespace Detail
{

template< typename T > struct IsArithmeticVectorTypedData
	: boost::mpl::and_
	<
		IECore::TypeTraits::IsNumericBasedVectorTypedData<T>,
		boost::mpl::not_< IECore::TypeTraits::IsBox<typename IECore::TypeTraits::VectorValueType<T>::type > >,
		boost::mpl::not_< IECore::TypeTraits::IsQuat<typename IECore::TypeTraits::VectorValueType<T>::type > >
	>
{};

struct AverageValueFromVector
{
	typedef IECore::DataPtr ReturnType;

	template<typename From> ReturnType operator()( typename From::ConstPtr data )
	{
		const typename From::ValueType &src = data->readable();
		if ( src.size() )
		{
			return new IECore::TypedData< typename From::ValueType::value_type >( std::accumulate( src.begin() + 1, src.end(), *src.begin() ) / src.size() );
		}
		return nullptr;
	}
};


inline IECore::DataPtr createArrayData( PrimitiveVariable& primitiveVariable, const Primitive *primitive, PrimitiveVariable::Interpolation interpolation )
{
	if ( primitiveVariable.interpolation != PrimitiveVariable::Constant )
		return nullptr;

	size_t len = primitive->variableSize( interpolation );
	switch( primitiveVariable.data->typeId() )
	{
		case IECore::IntDataTypeId:
		{
			IECore::IntVectorDataPtr newData = new IECore::IntVectorData();
			newData->writable().resize( len, static_cast< const IECore::IntData * >( primitiveVariable.data.get() )->readable() );
			return newData;
		}
			break;
		case IECore::FloatDataTypeId:
		{
			IECore::FloatVectorDataPtr newData = new IECore::FloatVectorData();
			newData->writable().resize( len, static_cast< const IECore::FloatData * >( primitiveVariable.data.get() )->readable() );
			return newData;
		}
			break;
		case IECore::V2fDataTypeId:
		{
			IECore::V2fVectorDataPtr newData = new IECore::V2fVectorData();
			newData->writable().resize( len, static_cast< const IECore::V2fData * >( primitiveVariable.data.get() )->readable() );
			return newData;
		}
			break;
		case IECore::V3fDataTypeId:
		{
			IECore::V3fVectorDataPtr newData = new IECore::V3fVectorData();
			newData->writable().resize( len, static_cast< const IECore::V3fData * >( primitiveVariable.data.get() )->readable() );
			return newData;
		}
			break;
		case IECore::Color3fDataTypeId:
		{
			IECore::Color3fVectorDataPtr newData = new IECore::Color3fVectorData();
			newData->writable().resize( len, static_cast< const IECore::Color3fData * >( primitiveVariable.data.get() )->readable() );
			primitiveVariable = PrimitiveVariable(interpolation, newData);
		}
			break;
		default:
			return nullptr;
	}

	return nullptr;
}

} // namespace Detail
} // namespace IECoreScene

#endif // IECORESCENE_PRIMITIVEALGOUTILS_H
