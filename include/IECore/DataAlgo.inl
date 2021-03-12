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
typename std::result_of<F( Args&&... )>::type dispatch( IECore::TypeId typeId, F &&functor, Args&&... args )
{
	switch( typeId )
	{
		case BoolDataTypeId :
			return functor.template operator()<IECore::BoolData>( std::forward<Args>( args )... );
		case FloatDataTypeId :
			return functor.template operator()<FloatData>( std::forward<Args>( args )... );
		case DoubleDataTypeId :
			return functor.template operator()<DoubleData>( std::forward<Args>( args )... );
		case IntDataTypeId :
			return functor.template operator()<IntData>( std::forward<Args>( args )... );
		case UIntDataTypeId :
			return functor.template operator()<UIntData>( std::forward<Args>( args )... );
		case CharDataTypeId :
			return functor.template operator()<CharData>( std::forward<Args>( args )... );
		case UCharDataTypeId :
			return functor.template operator()<UCharData>( std::forward<Args>( args )... );
		case ShortDataTypeId :
			return functor.template operator()<ShortData>( std::forward<Args>( args )... );
		case UShortDataTypeId :
			return functor.template operator()<UShortData>( std::forward<Args>( args )... );
		case Int64DataTypeId :
			return functor.template operator()<Int64Data>( std::forward<Args>( args )... );
		case UInt64DataTypeId :
			return functor.template operator()<UInt64Data>( std::forward<Args>( args )... );
		case StringDataTypeId :
			return functor.template operator()<StringData>( std::forward<Args>( args )... );
		case InternedStringDataTypeId :
			return functor.template operator()<InternedStringData>( std::forward<Args>( args )... );
		case HalfDataTypeId :
			return functor.template operator()<HalfData>( std::forward<Args>( args )... );
		case V2iDataTypeId :
			return functor.template operator()<V2iData>( std::forward<Args>( args )... );
		case V3iDataTypeId :
			return functor.template operator()<V3iData>( std::forward<Args>( args )... );
		case V2fDataTypeId :
			return functor.template operator()<V2fData>( std::forward<Args>( args )... );
		case V3fDataTypeId :
			return functor.template operator()<V3fData>( std::forward<Args>( args )... );
		case V2dDataTypeId :
			return functor.template operator()<V2dData>( std::forward<Args>( args )... );
		case V3dDataTypeId :
			return functor.template operator()<V3dData>( std::forward<Args>( args )... );
		case Color3fDataTypeId :
			return functor.template operator()<Color3fData>( std::forward<Args>( args )... );
		case Color4fDataTypeId :
			return functor.template operator()<Color4fData>( std::forward<Args>( args )... );
		case Box2iDataTypeId :
			return functor.template operator()<Box2iData>( std::forward<Args>( args )... );
		case Box2fDataTypeId :
			return functor.template operator()<Box2fData>( std::forward<Args>( args )... );
		case Box3fDataTypeId :
			return functor.template operator()<Box3fData>( std::forward<Args>( args )... );
		case Box2dDataTypeId :
			return functor.template operator()<Box2dData>( std::forward<Args>( args )... );
		case Box3dDataTypeId :
			return functor.template operator()<Box3dData>( std::forward<Args>( args )... );
		case M33fDataTypeId :
			return functor.template operator()<M33fData>( std::forward<Args>( args )... );
		case M33dDataTypeId :
			return functor.template operator()<M33dData>( std::forward<Args>( args )... );
		case M44fDataTypeId :
			return functor.template operator()<M44fData>( std::forward<Args>( args )... );
		case M44dDataTypeId :
			return functor.template operator()<M44dData>( std::forward<Args>( args )... );
		case TransformationMatrixfDataTypeId :
			return functor.template operator()<TransformationMatrixfData>( std::forward<Args>( args )... );
		case TransformationMatrixdDataTypeId :
			return functor.template operator()<TransformationMatrixdData>( std::forward<Args>( args )... );
		case QuatfDataTypeId :
			return functor.template operator()<QuatfData>( std::forward<Args>( args )... );
		case QuatdDataTypeId :
			return functor.template operator()<QuatdData>( std::forward<Args>( args )... );
		case SplineffDataTypeId :
			return functor.template operator()<SplineffData>( std::forward<Args>( args )... );
		case SplineddDataTypeId :
			return functor.template operator()<SplineddData>( std::forward<Args>( args )... );
		case SplinefColor3fDataTypeId :
			return functor.template operator()<SplinefColor3fData>( std::forward<Args>( args )... );
		case SplinefColor4fDataTypeId :
			return functor.template operator()<SplinefColor4fData>( std::forward<Args>( args )... );
		case DateTimeDataTypeId :
			return functor.template operator()<DateTimeData>( std::forward<Args>( args )... );
		case BoolVectorDataTypeId :
			return functor.template operator()<BoolVectorData>( std::forward<Args>( args )... );
		case FloatVectorDataTypeId :
			return functor.template operator()<FloatVectorData>( std::forward<Args>( args )... );
		case DoubleVectorDataTypeId :
			return functor.template operator()<DoubleVectorData>( std::forward<Args>( args )... );
		case HalfVectorDataTypeId :
			return functor.template operator()<HalfVectorData>( std::forward<Args>( args )... );
		case IntVectorDataTypeId :
			return functor.template operator()<IntVectorData>( std::forward<Args>( args )... );
		case UIntVectorDataTypeId :
			return functor.template operator()<UIntVectorData>( std::forward<Args>( args )... );
		case CharVectorDataTypeId :
			return functor.template operator()<CharVectorData>( std::forward<Args>( args )... );
		case UCharVectorDataTypeId :
			return functor.template operator()<UCharVectorData>( std::forward<Args>( args )... );
		case ShortVectorDataTypeId :
			return functor.template operator()<ShortVectorData>( std::forward<Args>( args )... );
		case UShortVectorDataTypeId :
			return functor.template operator()<UShortVectorData>( std::forward<Args>( args )... );
		case Int64VectorDataTypeId :
			return functor.template operator()<Int64VectorData>( std::forward<Args>( args )... );
		case UInt64VectorDataTypeId :
			return functor.template operator()<UInt64VectorData>( std::forward<Args>( args )... );
		case StringVectorDataTypeId :
			return functor.template operator()<StringVectorData>( std::forward<Args>( args )... );
		case InternedStringVectorDataTypeId :
			return functor.template operator()<InternedStringVectorData>( std::forward<Args>( args )... );
		case V2iVectorDataTypeId :
			return functor.template operator()<V2iVectorData>( std::forward<Args>( args )... );
		case V2fVectorDataTypeId :
			return functor.template operator()<V2fVectorData>( std::forward<Args>( args )... );
		case V2dVectorDataTypeId :
			return functor.template operator()<V2dVectorData>( std::forward<Args>( args )... );
		case V3iVectorDataTypeId :
			return functor.template operator()<V3iVectorData>( std::forward<Args>( args )... );
		case V3fVectorDataTypeId :
			return functor.template operator()<V3fVectorData>( std::forward<Args>( args )... );
		case V3dVectorDataTypeId :
			return functor.template operator()<V3dVectorData>( std::forward<Args>( args )... );
		case Box3fVectorDataTypeId :
			return functor.template operator()<Box3fVectorData>( std::forward<Args>( args )... );
		case Box3dVectorDataTypeId :
			return functor.template operator()<Box3dVectorData>( std::forward<Args>( args )... );
		case M33fVectorDataTypeId :
			return functor.template operator()<M33fVectorData>( std::forward<Args>( args )... );
		case M33dVectorDataTypeId :
			return functor.template operator()<M33dVectorData>( std::forward<Args>( args )... );
		case M44fVectorDataTypeId :
			return functor.template operator()<M44fVectorData>( std::forward<Args>( args )... );
		case M44dVectorDataTypeId :
			return functor.template operator()<M44dVectorData>( std::forward<Args>( args )... );
		case QuatfVectorDataTypeId :
			return functor.template operator()<QuatfVectorData>( std::forward<Args>( args )... );
		case QuatdVectorDataTypeId :
			return functor.template operator()<QuatdVectorData>( std::forward<Args>( args )... );
		case Color3fVectorDataTypeId :
			return functor.template operator()<Color3fVectorData>( std::forward<Args>( args )... );
		case Color4fVectorDataTypeId :
			return functor.template operator()<Color4fVectorData>( std::forward<Args>( args )... );
		default :
			throw InvalidArgumentException( boost::str ( boost::format( "Unknown data type '%1%'" ) % typeId ) );
	}
}

namespace Detail
{
	template<class F, typename... Args>
	struct DispatchDataWrapper
	{
		DispatchDataWrapper( F &&functor ) : m_functor( std::forward<F>( functor ) )
		{
		}

		template<class T>
		typename std::result_of<F( Data *, Args&&... )>::type operator()( Data *data, Args&&... args )
		{
			return m_functor( static_cast<T *>( data ), std::forward<Args>( args )... );
		}

		typename std::result_of<F( Data *, Args&&... )>::type operator()( Data *data, Args&&... args )
		{
			return m_functor( data, std::forward<Args>( args )... );
		}

		F &&m_functor;
	};
} // namespace Detail

template<class F, typename... Args>
typename std::result_of<F( Data *, Args&&... )>::type dispatch( Data *data, F &&functor, Args&&... args )
{
	Detail::DispatchDataWrapper<F, Args&&...> functorWrapper( std::forward<F>( functor ) );
	return dispatch( data->typeId(), functorWrapper, data, std::forward<Args>( args )... );
}

namespace Detail
{
	template<class F, typename... Args>
	struct DispatchConstDataWrapper
	{
		DispatchConstDataWrapper( F &&functor ) : m_functor( std::forward<F>( functor ) )
		{
		}

		template<class T>
		typename std::result_of<F( const Data *, Args&&... )>::type operator()( const Data *data, Args&&... args )
		{
			return m_functor( static_cast<const T *>( data ), std::forward<Args>( args )... );
		}

		typename std::result_of<F( const Data *, Args&&... )>::type operator()( const Data *data, Args&&... args )
		{
			return m_functor( data, std::forward<Args>( args )... );
		}

		F &&m_functor;
	};
} // namespace Detail

template<class F, typename... Args>
typename std::result_of<F( const Data *, Args&&... )>::type dispatch( const Data *data, F &&functor, Args&&... args )
{
	Detail::DispatchConstDataWrapper<F, Args&&...> functorWrapper( std::forward<F>( functor ) );
	return dispatch( data->typeId(), functorWrapper, data, std::forward<Args>( args )... );
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
