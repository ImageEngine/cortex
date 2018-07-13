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

template<class F, typename... Args>
typename std::result_of<F( Data *, Args&&... )>::type dispatch( Data *data, F &&functor, Args&&... args )
{
	IECore::TypeId typeId = data->typeId();

	switch( typeId )
	{
		case BoolDataTypeId :
			return functor( static_cast<BoolData *>( data ), std::forward<Args>( args )... );
		case FloatDataTypeId :
			return functor( static_cast<FloatData *>( data ), std::forward<Args>( args )... );
		case DoubleDataTypeId :
			return functor( static_cast<DoubleData *>( data ), std::forward<Args>( args )... );
		case IntDataTypeId :
			return functor( static_cast<IntData *>( data ), std::forward<Args>( args )... );
		case UIntDataTypeId :
			return functor( static_cast<UIntData *>( data ), std::forward<Args>( args )... );
		case CharDataTypeId :
			return functor( static_cast<CharData *>( data ), std::forward<Args>( args )... );
		case UCharDataTypeId :
			return functor( static_cast<UCharData *>( data ), std::forward<Args>( args )... );
		case ShortDataTypeId :
			return functor( static_cast<ShortData *>( data ), std::forward<Args>( args )... );
		case UShortDataTypeId :
			return functor( static_cast<UShortData *>( data ), std::forward<Args>( args )... );
		case Int64DataTypeId :
			return functor( static_cast<Int64Data *>( data ), std::forward<Args>( args )... );
		case UInt64DataTypeId :
			return functor( static_cast<UInt64Data *>( data ), std::forward<Args>( args )... );
		case StringDataTypeId :
			return functor( static_cast<StringData *>( data ), std::forward<Args>( args )... );
		case InternedStringDataTypeId :
			return functor( static_cast<InternedStringData *>( data ), std::forward<Args>( args )... );
		case HalfDataTypeId :
			return functor( static_cast<HalfData *>( data ), std::forward<Args>( args )... );
		case V2iDataTypeId :
			return functor( static_cast<V2iData *>( data ), std::forward<Args>( args )... );
		case V3iDataTypeId :
			return functor( static_cast<V3iData *>( data ), std::forward<Args>( args )... );
		case V2fDataTypeId :
			return functor( static_cast<V2fData *>( data ), std::forward<Args>( args )... );
		case V3fDataTypeId :
			return functor( static_cast<V3fData *>( data ), std::forward<Args>( args )... );
		case V2dDataTypeId :
			return functor( static_cast<V2dData *>( data ), std::forward<Args>( args )... );
		case V3dDataTypeId :
			return functor( static_cast<V3dData *>( data ), std::forward<Args>( args )... );
		case Color3fDataTypeId :
			return functor( static_cast<Color3fData *>( data ), std::forward<Args>( args )... );
		case Color4fDataTypeId :
			return functor( static_cast<Color4fData *>( data ), std::forward<Args>( args )... );
		case Box2iDataTypeId :
			return functor( static_cast<Box2iData *>( data ), std::forward<Args>( args )... );
		case Box2fDataTypeId :
			return functor( static_cast<Box2fData *>( data ), std::forward<Args>( args )... );
		case Box3fDataTypeId :
			return functor( static_cast<Box3fData *>( data ), std::forward<Args>( args )... );
		case Box2dDataTypeId :
			return functor( static_cast<Box2dData *>( data ), std::forward<Args>( args )... );
		case Box3dDataTypeId :
			return functor( static_cast<Box3dData *>( data ), std::forward<Args>( args )... );
		case M33fDataTypeId :
			return functor( static_cast<M33fData *>( data ), std::forward<Args>( args )... );
		case M33dDataTypeId :
			return functor( static_cast<M33dData *>( data ), std::forward<Args>( args )... );
		case M44fDataTypeId :
			return functor( static_cast<M44fData *>( data ), std::forward<Args>( args )... );
		case M44dDataTypeId :
			return functor( static_cast<M44dData *>( data ), std::forward<Args>( args )... );
		case TransformationMatrixfDataTypeId :
			return functor( static_cast<TransformationMatrixfData *>( data ), std::forward<Args>( args )... );
		case TransformationMatrixdDataTypeId :
			return functor( static_cast<TransformationMatrixdData *>( data ), std::forward<Args>( args )... );
		case QuatfDataTypeId :
			return functor( static_cast<QuatfData *>( data ), std::forward<Args>( args )... );
		case QuatdDataTypeId :
			return functor( static_cast<QuatdData *>( data ), std::forward<Args>( args )... );
		case SplineffDataTypeId :
			return functor( static_cast<SplineffData *>( data ), std::forward<Args>( args )... );
		case SplineddDataTypeId :
			return functor( static_cast<SplineddData *>( data ), std::forward<Args>( args )... );
		case SplinefColor3fDataTypeId :
			return functor( static_cast<SplinefColor3fData *>( data ), std::forward<Args>( args )... );
		case SplinefColor4fDataTypeId :
			return functor( static_cast<SplinefColor4fData *>( data ), std::forward<Args>( args )... );
		case DateTimeDataTypeId :
			return functor( static_cast<DateTimeData *>( data ), std::forward<Args>( args )... );
		case BoolVectorDataTypeId :
			return functor( static_cast<BoolVectorData *>( data ), std::forward<Args>( args )... );
		case FloatVectorDataTypeId :
			return functor( static_cast<FloatVectorData *>( data ), std::forward<Args>( args )... );
		case DoubleVectorDataTypeId :
			return functor( static_cast<DoubleVectorData *>( data ), std::forward<Args>( args )... );
		case HalfVectorDataTypeId :
			return functor( static_cast<HalfVectorData *>( data ), std::forward<Args>( args )... );
		case IntVectorDataTypeId :
			return functor( static_cast<IntVectorData *>( data ), std::forward<Args>( args )... );
		case UIntVectorDataTypeId :
			return functor( static_cast<UIntVectorData *>( data ), std::forward<Args>( args )... );
		case CharVectorDataTypeId :
			return functor( static_cast<CharVectorData *>( data ), std::forward<Args>( args )... );
		case UCharVectorDataTypeId :
			return functor( static_cast<UCharVectorData *>( data ), std::forward<Args>( args )... );
		case ShortVectorDataTypeId :
			return functor( static_cast<ShortVectorData *>( data ), std::forward<Args>( args )... );
		case UShortVectorDataTypeId :
			return functor( static_cast<UShortVectorData *>( data ), std::forward<Args>( args )... );
		case Int64VectorDataTypeId :
			return functor( static_cast<Int64VectorData *>( data ), std::forward<Args>( args )... );
		case UInt64VectorDataTypeId :
			return functor( static_cast<UInt64VectorData *>( data ), std::forward<Args>( args )... );
		case StringVectorDataTypeId :
			return functor( static_cast<StringVectorData *>( data ), std::forward<Args>( args )... );
		case InternedStringVectorDataTypeId :
			return functor( static_cast<InternedStringVectorData *>( data ), std::forward<Args>( args )... );
		case V2iVectorDataTypeId :
			return functor( static_cast<V2iVectorData *>( data ), std::forward<Args>( args )... );
		case V2fVectorDataTypeId :
			return functor( static_cast<V2fVectorData *>( data ), std::forward<Args>( args )... );
		case V2dVectorDataTypeId :
			return functor( static_cast<V2dVectorData *>( data ), std::forward<Args>( args )... );
		case V3iVectorDataTypeId :
			return functor( static_cast<V3iVectorData *>( data ), std::forward<Args>( args )... );
		case V3fVectorDataTypeId :
			return functor( static_cast<V3fVectorData *>( data ), std::forward<Args>( args )... );
		case V3dVectorDataTypeId :
			return functor( static_cast<V3dVectorData *>( data ), std::forward<Args>( args )... );
		case Box3fVectorDataTypeId :
			return functor( static_cast<Box3fVectorData *>( data ), std::forward<Args>( args )... );
		case Box3dVectorDataTypeId :
			return functor( static_cast<Box3dVectorData *>( data ), std::forward<Args>( args )... );
		case M33fVectorDataTypeId :
			return functor( static_cast<M33fVectorData *>( data ), std::forward<Args>( args )... );
		case M33dVectorDataTypeId :
			return functor( static_cast<M33dVectorData *>( data ), std::forward<Args>( args )... );
		case M44fVectorDataTypeId :
			return functor( static_cast<M44fVectorData *>( data ), std::forward<Args>( args )... );
		case M44dVectorDataTypeId :
			return functor( static_cast<M44dVectorData *>( data ), std::forward<Args>( args )... );
		case QuatfVectorDataTypeId :
			return functor( static_cast<QuatfVectorData *>( data ), std::forward<Args>( args )... );
		case QuatdVectorDataTypeId :
			return functor( static_cast<QuatdVectorData *>( data ), std::forward<Args>( args )... );
		case Color3fVectorDataTypeId :
			return functor( static_cast<Color3fVectorData *>( data ), std::forward<Args>( args )... );
		case Color4fVectorDataTypeId :
			return functor( static_cast<Color4fVectorData *>( data ), std::forward<Args>( args )... );
		default :
			throw InvalidArgumentException( boost::str ( boost::format( "Data has unknown type '%1%' / '%2%' " ) % typeId % data->typeName() ) );
	}
}

template<class F, typename... Args>
typename std::result_of<F( const Data *, Args&&... )>::type dispatch( const Data *data, F &&functor, Args&&... args )
{
	IECore::TypeId typeId = data->typeId();

	switch( typeId )
	{
		case BoolDataTypeId :
			return functor( static_cast<const BoolData *>( data ), std::forward<args>( args )... );
		case FloatDataTypeId :
			return functor( static_cast<const FloatData *>( data ), std::forward<args>( args )... );
		case DoubleDataTypeId :
			return functor( static_cast<const DoubleData *>( data ), std::forward<args>( args )... );
		case IntDataTypeId :
			return functor( static_cast<const IntData *>( data ), std::forward<args>( args )... );
		case UIntDataTypeId :
			return functor( static_cast<const UIntData *>( data ), std::forward<args>( args )... );
		case CharDataTypeId :
			return functor( static_cast<const CharData *>( data ), std::forward<args>( args )... );
		case UCharDataTypeId :
			return functor( static_cast<const UCharData *>( data ), std::forward<args>( args )... );
		case ShortDataTypeId :
			return functor( static_cast<const ShortData *>( data ), std::forward<args>( args )... );
		case UShortDataTypeId :
			return functor( static_cast<const UShortData *>( data ), std::forward<args>( args )... );
		case Int64DataTypeId :
			return functor( static_cast<const Int64Data *>( data ), std::forward<args>( args )... );
		case UInt64DataTypeId :
			return functor( static_cast<const UInt64Data *>( data ), std::forward<args>( args )... );
		case StringDataTypeId :
			return functor( static_cast<const StringData *>( data ), std::forward<args>( args )... );
		case InternedStringDataTypeId :
			return functor( static_cast<const InternedStringData *>( data ), std::forward<args>( args )... );
		case HalfDataTypeId :
			return functor( static_cast<const HalfData *>( data ), std::forward<args>( args )... );
		case V2iDataTypeId :
			return functor( static_cast<const V2iData *>( data ), std::forward<args>( args )... );
		case V3iDataTypeId :
			return functor( static_cast<const V3iData *>( data ), std::forward<args>( args )... );
		case V2fDataTypeId :
			return functor( static_cast<const V2fData *>( data ), std::forward<args>( args )... );
		case V3fDataTypeId :
			return functor( static_cast<const V3fData *>( data ), std::forward<args>( args )... );
		case V2dDataTypeId :
			return functor( static_cast<const V2dData *>( data ), std::forward<args>( args )... );
		case V3dDataTypeId :
			return functor( static_cast<const V3dData *>( data ), std::forward<args>( args )... );
		case Color3fDataTypeId :
			return functor( static_cast<const Color3fData *>( data ), std::forward<args>( args )... );
		case Color4fDataTypeId :
			return functor( static_cast<const Color4fData *>( data ), std::forward<args>( args )... );
		case Box2iDataTypeId :
			return functor( static_cast<const Box2iData *>( data ), std::forward<args>( args )... );
		case Box2fDataTypeId :
			return functor( static_cast<const Box2fData *>( data ), std::forward<args>( args )... );
		case Box3fDataTypeId :
			return functor( static_cast<const Box3fData *>( data ), std::forward<args>( args )... );
		case Box2dDataTypeId :
			return functor( static_cast<const Box2dData *>( data ), std::forward<args>( args )... );
		case Box3dDataTypeId :
			return functor( static_cast<const Box3dData *>( data ), std::forward<args>( args )... );
		case M33fDataTypeId :
			return functor( static_cast<const M33fData *>( data ), std::forward<args>( args )... );
		case M33dDataTypeId :
			return functor( static_cast<const M33dData *>( data ), std::forward<args>( args )... );
		case M44fDataTypeId :
			return functor( static_cast<const M44fData *>( data ), std::forward<args>( args )... );
		case M44dDataTypeId :
			return functor( static_cast<const M44dData *>( data ), std::forward<args>( args )... );
		case TransformationMatrixfDataTypeId :
			return functor( static_cast<const TransformationMatrixfData *>( data ), std::forward<args>( args )... );
		case TransformationMatrixdDataTypeId :
			return functor( static_cast<const TransformationMatrixdData *>( data ), std::forward<args>( args )... );
		case QuatfDataTypeId :
			return functor( static_cast<const QuatfData *>( data ), std::forward<args>( args )... );
		case QuatdDataTypeId :
			return functor( static_cast<const QuatdData *>( data ), std::forward<args>( args )... );
		case SplineffDataTypeId :
			return functor( static_cast<const SplineffData *>( data ), std::forward<args>( args )... );
		case SplineddDataTypeId :
			return functor( static_cast<const SplineddData *>( data ), std::forward<args>( args )... );
		case SplinefColor3fDataTypeId :
			return functor( static_cast<const SplinefColor3fData *>( data ), std::forward<args>( args )... );
		case SplinefColor4fDataTypeId :
			return functor( static_cast<const SplinefColor4fData *>( data ), std::forward<args>( args )... );
		case DateTimeDataTypeId :
			return functor( static_cast<const DateTimeData *>( data ), std::forward<args>( args )... );
		case BoolVectorDataTypeId :
			return functor( static_cast<const BoolVectorData *>( data ), std::forward<args>( args )... );
		case FloatVectorDataTypeId :
			return functor( static_cast<const FloatVectorData *>( data ), std::forward<args>( args )... );
		case DoubleVectorDataTypeId :
			return functor( static_cast<const DoubleVectorData *>( data ), std::forward<args>( args )... );
		case HalfVectorDataTypeId :
			return functor( static_cast<const HalfVectorData *>( data ), std::forward<args>( args )... );
		case IntVectorDataTypeId :
			return functor( static_cast<const IntVectorData *>( data ), std::forward<args>( args )... );
		case UIntVectorDataTypeId :
			return functor( static_cast<const UIntVectorData *>( data ), std::forward<args>( args )... );
		case CharVectorDataTypeId :
			return functor( static_cast<const CharVectorData *>( data ), std::forward<args>( args )... );
		case UCharVectorDataTypeId :
			return functor( static_cast<const UCharVectorData *>( data ), std::forward<args>( args )... );
		case ShortVectorDataTypeId :
			return functor( static_cast<const ShortVectorData *>( data ), std::forward<args>( args )... );
		case UShortVectorDataTypeId :
			return functor( static_cast<const UShortVectorData *>( data ), std::forward<args>( args )... );
		case Int64VectorDataTypeId :
			return functor( static_cast<const Int64VectorData *>( data ), std::forward<args>( args )... );
		case UInt64VectorDataTypeId :
			return functor( static_cast<const UInt64VectorData *>( data ), std::forward<args>( args )... );
		case StringVectorDataTypeId :
			return functor( static_cast<const StringVectorData *>( data ), std::forward<args>( args )... );
		case InternedStringVectorDataTypeId :
			return functor( static_cast<const InternedStringVectorData *>( data ), std::forward<args>( args )... );
		case V2iVectorDataTypeId :
			return functor( static_cast<const V2iVectorData *>( data ), std::forward<args>( args )... );
		case V2fVectorDataTypeId :
			return functor( static_cast<const V2fVectorData *>( data ), std::forward<args>( args )... );
		case V2dVectorDataTypeId :
			return functor( static_cast<const V2dVectorData *>( data ), std::forward<args>( args )... );
		case V3iVectorDataTypeId :
			return functor( static_cast<const V3iVectorData *>( data ), std::forward<args>( args )... );
		case V3fVectorDataTypeId :
			return functor( static_cast<const V3fVectorData *>( data ), std::forward<args>( args )... );
		case V3dVectorDataTypeId :
			return functor( static_cast<const V3dVectorData *>( data ), std::forward<args>( args )... );
		case Box3fVectorDataTypeId :
			return functor( static_cast<const Box3fVectorData *>( data ), std::forward<args>( args )... );
		case Box3dVectorDataTypeId :
			return functor( static_cast<const Box3dVectorData *>( data ), std::forward<args>( args )... );
		case M33fVectorDataTypeId :
			return functor( static_cast<const M33fVectorData *>( data ), std::forward<args>( args )... );
		case M33dVectorDataTypeId :
			return functor( static_cast<const M33dVectorData *>( data ), std::forward<args>( args )... );
		case M44fVectorDataTypeId :
			return functor( static_cast<const M44fVectorData *>( data ), std::forward<args>( args )... );
		case M44dVectorDataTypeId :
			return functor( static_cast<const M44dVectorData *>( data ), std::forward<args>( args )... );
		case QuatfVectorDataTypeId :
			return functor( static_cast<const QuatfVectorData *>( data ), std::forward<args>( args )... );
		case QuatdVectorDataTypeId :
			return functor( static_cast<const QuatdVectorData *>( data ), std::forward<args>( args )... );
		case Color3fVectorDataTypeId :
			return functor( static_cast<const Color3fVectorData *>( data ), std::forward<args>( args )... );
		case Color4fVectorDataTypeId :
			return functor( static_cast<const Color4fVectorData *>( data ), std::forward<args>( args )... );
		default :
			throw InvalidArgumentException( boost::str ( boost::format( "Data has unknown type '%1%' / '%2%' " ) % typeId % data->typeName() ) );
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
