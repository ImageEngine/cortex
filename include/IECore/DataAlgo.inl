//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2018, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_DATAALGO_INL
#define IECORE_DATAALGO_INL

#include "IECore/DateTimeData.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/SplineData.h"
#include "IECore/TransformationMatrixData.h"
#include "IECore/VectorTypedData.h"

#include <type_traits>

namespace IECore
{

template<class F>
typename std::result_of<F( Data * )>::type dispatch( Data *data, F &&functor )
{
	switch( data->typeId() )
	{
		case BoolDataTypeId :
			return functor( static_cast<BoolData *>( data ) );
		case FloatDataTypeId :
			return functor( static_cast<FloatData *>( data ) );
		case DoubleDataTypeId :
			return functor( static_cast<DoubleData *>( data ) );
		case IntDataTypeId :
			return functor( static_cast<IntData *>( data ) );
		case UIntDataTypeId :
			return functor( static_cast<UIntData *>( data ) );
		case CharDataTypeId :
			return functor( static_cast<CharData *>( data ) );
		case UCharDataTypeId :
			return functor( static_cast<UCharData *>( data ) );
		case ShortDataTypeId :
			return functor( static_cast<ShortData *>( data ) );
		case UShortDataTypeId :
			return functor( static_cast<UShortData *>( data ) );
		case Int64DataTypeId :
			return functor( static_cast<Int64Data *>( data ) );
		case UInt64DataTypeId :
			return functor( static_cast<UInt64Data *>( data ) );
		case StringDataTypeId :
			return functor( static_cast<StringData *>( data ) );
		case InternedStringDataTypeId :
			return functor( static_cast<InternedStringData *>( data ) );
		case HalfDataTypeId :
			return functor( static_cast<HalfData *>( data ) );
		case V2iDataTypeId :
			return functor( static_cast<V2iData *>( data ) );
		case V3iDataTypeId :
			return functor( static_cast<V3iData *>( data ) );
		case V2fDataTypeId :
			return functor( static_cast<V2fData *>( data ) );
		case V3fDataTypeId :
			return functor( static_cast<V3fData *>( data ) );
		case V2dDataTypeId :
			return functor( static_cast<V2dData *>( data ) );
		case V3dDataTypeId :
			return functor( static_cast<V3dData *>( data ) );
		case Color3fDataTypeId :
			return functor( static_cast<Color3fData *>( data ) );
		case Color4fDataTypeId :
			return functor( static_cast<Color4fData *>( data ) );
		case Box2iDataTypeId :
			return functor( static_cast<Box2iData *>( data ) );
		case Box2fDataTypeId :
			return functor( static_cast<Box2fData *>( data ) );
		case Box3fDataTypeId :
			return functor( static_cast<Box3fData *>( data ) );
		case Box2dDataTypeId :
			return functor( static_cast<Box2dData *>( data ) );
		case Box3dDataTypeId :
			return functor( static_cast<Box3dData *>( data ) );
		case M33fDataTypeId :
			return functor( static_cast<M33fData *>( data ) );
		case M33dDataTypeId :
			return functor( static_cast<M33dData *>( data ) );
		case M44fDataTypeId :
			return functor( static_cast<M44fData *>( data ) );
		case M44dDataTypeId :
			return functor( static_cast<M44dData *>( data ) );
		case TransformationMatrixfDataTypeId :
			return functor( static_cast<TransformationMatrixfData *>( data ) );
		case TransformationMatrixdDataTypeId :
			return functor( static_cast<TransformationMatrixdData *>( data ) );
		case QuatfDataTypeId :
			return functor( static_cast<QuatfData *>( data ) );
		case QuatdDataTypeId :
			return functor( static_cast<QuatdData *>( data ) );
		case SplineffDataTypeId :
			return functor( static_cast<SplineffData *>( data ) );
		case SplineddDataTypeId :
			return functor( static_cast<SplineddData *>( data ) );
		case SplinefColor3fDataTypeId :
			return functor( static_cast<SplinefColor3fData *>( data ) );
		case SplinefColor4fDataTypeId :
			return functor( static_cast<SplinefColor4fData *>( data ) );
		case DateTimeDataTypeId :
			return functor( static_cast<DateTimeData *>( data ) );
		case BoolVectorDataTypeId :
			return functor( static_cast<BoolVectorData *>( data ) );
		case FloatVectorDataTypeId :
			return functor( static_cast<FloatVectorData *>( data ) );
		case DoubleVectorDataTypeId :
			return functor( static_cast<DoubleVectorData *>( data ) );
		case HalfVectorDataTypeId :
			return functor( static_cast<HalfVectorData *>( data ) );
		case IntVectorDataTypeId :
			return functor( static_cast<IntVectorData *>( data ) );
		case UIntVectorDataTypeId :
			return functor( static_cast<UIntVectorData *>( data ) );
		case CharVectorDataTypeId :
			return functor( static_cast<CharVectorData *>( data ) );
		case UCharVectorDataTypeId :
			return functor( static_cast<UCharVectorData *>( data ) );
		case ShortVectorDataTypeId :
			return functor( static_cast<ShortVectorData *>( data ) );
		case UShortVectorDataTypeId :
			return functor( static_cast<UShortVectorData *>( data ) );
		case Int64VectorDataTypeId :
			return functor( static_cast<Int64VectorData *>( data ) );
		case UInt64VectorDataTypeId :
			return functor( static_cast<UInt64VectorData *>( data ) );
		case StringVectorDataTypeId :
			return functor( static_cast<StringVectorData *>( data ) );
		case InternedStringVectorDataTypeId :
			return functor( static_cast<InternedStringVectorData *>( data ) );
		case V2iVectorDataTypeId :
			return functor( static_cast<V2iVectorData *>( data ) );
		case V2fVectorDataTypeId :
			return functor( static_cast<V2fVectorData *>( data ) );
		case V2dVectorDataTypeId :
			return functor( static_cast<V2dVectorData *>( data ) );
		case V3iVectorDataTypeId :
			return functor( static_cast<V3iVectorData *>( data ) );
		case V3fVectorDataTypeId :
			return functor( static_cast<V3fVectorData *>( data ) );
		case V3dVectorDataTypeId :
			return functor( static_cast<V3dVectorData *>( data ) );
		case Box3fVectorDataTypeId :
			return functor( static_cast<Box3fVectorData *>( data ) );
		case Box3dVectorDataTypeId :
			return functor( static_cast<Box3dVectorData *>( data ) );
		case M33fVectorDataTypeId :
			return functor( static_cast<M33fVectorData *>( data ) );
		case M33dVectorDataTypeId :
			return functor( static_cast<M33dVectorData *>( data ) );
		case M44fVectorDataTypeId :
			return functor( static_cast<M44fVectorData *>( data ) );
		case M44dVectorDataTypeId :
			return functor( static_cast<M44dVectorData *>( data ) );
		case QuatfVectorDataTypeId :
			return functor( static_cast<QuatfVectorData *>( data ) );
		case QuatdVectorDataTypeId :
			return functor( static_cast<QuatdVectorData *>( data ) );
		case Color3fVectorDataTypeId :
			return functor( static_cast<Color3fVectorData *>( data ) );
		case Color4fVectorDataTypeId :
			return functor( static_cast<Color4fVectorData *>( data ) );
		default :
			throw InvalidArgumentException( "Data has unknown type" );
	}
}

template<class F>
typename std::result_of<F( const Data * )>::type dispatch( const Data *data, F &&functor )
{
	switch( data->typeId() )
	{
		case BoolDataTypeId :
			return functor( static_cast<const BoolData *>( data ) );
		case FloatDataTypeId :
			return functor( static_cast<const FloatData *>( data ) );
		case DoubleDataTypeId :
			return functor( static_cast<const DoubleData *>( data ) );
		case IntDataTypeId :
			return functor( static_cast<const IntData *>( data ) );
		case UIntDataTypeId :
			return functor( static_cast<const UIntData *>( data ) );
		case CharDataTypeId :
			return functor( static_cast<const CharData *>( data ) );
		case UCharDataTypeId :
			return functor( static_cast<const UCharData *>( data ) );
		case ShortDataTypeId :
			return functor( static_cast<const ShortData *>( data ) );
		case UShortDataTypeId :
			return functor( static_cast<const UShortData *>( data ) );
		case Int64DataTypeId :
			return functor( static_cast<const Int64Data *>( data ) );
		case UInt64DataTypeId :
			return functor( static_cast<const UInt64Data *>( data ) );
		case StringDataTypeId :
			return functor( static_cast<const StringData *>( data ) );
		case InternedStringDataTypeId :
			return functor( static_cast<const InternedStringData *>( data ) );
		case HalfDataTypeId :
			return functor( static_cast<const HalfData *>( data ) );
		case V2iDataTypeId :
			return functor( static_cast<const V2iData *>( data ) );
		case V3iDataTypeId :
			return functor( static_cast<const V3iData *>( data ) );
		case V2fDataTypeId :
			return functor( static_cast<const V2fData *>( data ) );
		case V3fDataTypeId :
			return functor( static_cast<const V3fData *>( data ) );
		case V2dDataTypeId :
			return functor( static_cast<const V2dData *>( data ) );
		case V3dDataTypeId :
			return functor( static_cast<const V3dData *>( data ) );
		case Color3fDataTypeId :
			return functor( static_cast<const Color3fData *>( data ) );
		case Color4fDataTypeId :
			return functor( static_cast<const Color4fData *>( data ) );
		case Box2iDataTypeId :
			return functor( static_cast<const Box2iData *>( data ) );
		case Box2fDataTypeId :
			return functor( static_cast<const Box2fData *>( data ) );
		case Box3fDataTypeId :
			return functor( static_cast<const Box3fData *>( data ) );
		case Box2dDataTypeId :
			return functor( static_cast<const Box2dData *>( data ) );
		case Box3dDataTypeId :
			return functor( static_cast<const Box3dData *>( data ) );
		case M33fDataTypeId :
			return functor( static_cast<const M33fData *>( data ) );
		case M33dDataTypeId :
			return functor( static_cast<const M33dData *>( data ) );
		case M44fDataTypeId :
			return functor( static_cast<const M44fData *>( data ) );
		case M44dDataTypeId :
			return functor( static_cast<const M44dData *>( data ) );
		case TransformationMatrixfDataTypeId :
			return functor( static_cast<const TransformationMatrixfData *>( data ) );
		case TransformationMatrixdDataTypeId :
			return functor( static_cast<const TransformationMatrixdData *>( data ) );
		case QuatfDataTypeId :
			return functor( static_cast<const QuatfData *>( data ) );
		case QuatdDataTypeId :
			return functor( static_cast<const QuatdData *>( data ) );
		case SplineffDataTypeId :
			return functor( static_cast<const SplineffData *>( data ) );
		case SplineddDataTypeId :
			return functor( static_cast<const SplineddData *>( data ) );
		case SplinefColor3fDataTypeId :
			return functor( static_cast<const SplinefColor3fData *>( data ) );
		case SplinefColor4fDataTypeId :
			return functor( static_cast<const SplinefColor4fData *>( data ) );
		case DateTimeDataTypeId :
			return functor( static_cast<const DateTimeData *>( data ) );
		case BoolVectorDataTypeId :
			return functor( static_cast<const BoolVectorData *>( data ) );
		case FloatVectorDataTypeId :
			return functor( static_cast<const FloatVectorData *>( data ) );
		case DoubleVectorDataTypeId :
			return functor( static_cast<const DoubleVectorData *>( data ) );
		case HalfVectorDataTypeId :
			return functor( static_cast<const HalfVectorData *>( data ) );
		case IntVectorDataTypeId :
			return functor( static_cast<const IntVectorData *>( data ) );
		case UIntVectorDataTypeId :
			return functor( static_cast<const UIntVectorData *>( data ) );
		case CharVectorDataTypeId :
			return functor( static_cast<const CharVectorData *>( data ) );
		case UCharVectorDataTypeId :
			return functor( static_cast<const UCharVectorData *>( data ) );
		case ShortVectorDataTypeId :
			return functor( static_cast<const ShortVectorData *>( data ) );
		case UShortVectorDataTypeId :
			return functor( static_cast<const UShortVectorData *>( data ) );
		case Int64VectorDataTypeId :
			return functor( static_cast<const Int64VectorData *>( data ) );
		case UInt64VectorDataTypeId :
			return functor( static_cast<const UInt64VectorData *>( data ) );
		case StringVectorDataTypeId :
			return functor( static_cast<const StringVectorData *>( data ) );
		case InternedStringVectorDataTypeId :
			return functor( static_cast<const InternedStringVectorData *>( data ) );
		case V2iVectorDataTypeId :
			return functor( static_cast<const V2iVectorData *>( data ) );
		case V2fVectorDataTypeId :
			return functor( static_cast<const V2fVectorData *>( data ) );
		case V2dVectorDataTypeId :
			return functor( static_cast<const V2dVectorData *>( data ) );
		case V3iVectorDataTypeId :
			return functor( static_cast<const V3iVectorData *>( data ) );
		case V3fVectorDataTypeId :
			return functor( static_cast<const V3fVectorData *>( data ) );
		case V3dVectorDataTypeId :
			return functor( static_cast<const V3dVectorData *>( data ) );
		case Box3fVectorDataTypeId :
			return functor( static_cast<const Box3fVectorData *>( data ) );
		case Box3dVectorDataTypeId :
			return functor( static_cast<const Box3dVectorData *>( data ) );
		case M33fVectorDataTypeId :
			return functor( static_cast<const M33fVectorData *>( data ) );
		case M33dVectorDataTypeId :
			return functor( static_cast<const M33dVectorData *>( data ) );
		case M44fVectorDataTypeId :
			return functor( static_cast<const M44fVectorData *>( data ) );
		case M44dVectorDataTypeId :
			return functor( static_cast<const M44dVectorData *>( data ) );
		case QuatfVectorDataTypeId :
			return functor( static_cast<const QuatfVectorData *>( data ) );
		case QuatdVectorDataTypeId :
			return functor( static_cast<const QuatdVectorData *>( data ) );
		case Color3fVectorDataTypeId :
			return functor( static_cast<const Color3fVectorData *>( data ) );
		case Color4fVectorDataTypeId :
			return functor( static_cast<const Color4fVectorData *>( data ) );
		default :
			throw InvalidArgumentException( "Data has unknown type" );
	}
}

namespace Detail
{

template<template<typename> class Trait>
struct TestTrait
{

	template<typename T>
	bool operator()( const T *data, typename std::enable_if<Trait<T>::value>::type *enabler = nullptr )
	{
		return true;
	}

	bool operator()( const Data *data )
	{
		return false;
	}

};

} // namespace Detail

template<template<typename> class Trait>
bool trait( const IECore::Data *data )
{
	return dispatch( data, Detail::TestTrait<Trait>() );
}

} // namespace IECore

#endif // IECORE_DATAALGO_INL
