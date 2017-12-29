//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_DESPATCHTYPEDDATA_INL
#define IE_CORE_DESPATCHTYPEDDATA_INL

#include "IECore/TypeTraits.h"
#include "IECore/VectorTypedData.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TransformationMatrixData.h"
#include "IECore/SplineData.h"
#include "IECore/DateTimeData.h"
#include "IECore/TimePeriodData.h"
#include "IECore/TimeDurationData.h"

namespace IECore
{

namespace Detail
{

template< class Functor, typename DataType, typename ErrorHandler >
struct DespatchTypedData
{
	template< template< typename > class Enabler, typename Enable = void >
	struct Func
	{
		typedef typename Functor::ReturnType ReturnType;

		ReturnType operator()( DataType *data, Functor &functor, ErrorHandler &errorHandler )
		{
			assert( data );

			errorHandler.template operator()< DataType, Functor >( data, functor );

			return ReturnType();
		}
	};

	template< template< typename > class Enabler >
	struct Func< Enabler, typename boost::enable_if< Enabler<DataType> >::type >
	{
		typedef typename Functor::ReturnType ReturnType;

		ReturnType operator()( DataType *data, Functor &functor, ErrorHandler & )
		{
			assert( data );

			return functor.template operator()<DataType>( data );
		}
	};
};

struct DespatchTypedDataExceptionError
{
	template<typename T, typename F>
	void operator()( const T *data, const F& functor )
	{
		throw InvalidArgumentException( ( boost::format( "Unhandled data of type %s encountered by DespatchTypedData" ) % data->typeName() ).str() );
	}
};

} // namespace Detail


struct DespatchTypedDataIgnoreError
{
	template<typename T, typename F>
	void operator()( const T *data, const F& functor )
	{
	}
};

template< class Functor, template<typename> class Enabler, typename ErrorHandler >
typename Functor::ReturnType despatchTypedData( Data *data, Functor &functor, ErrorHandler &errorHandler )
{
	assert( data );

	switch( data->typeId() )
	{
		case BoolDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, BoolData, ErrorHandler >
			::template Func<Enabler>()( static_cast<BoolData *>( data ), functor, errorHandler );
		case FloatDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, FloatData, ErrorHandler >
			::template Func<Enabler>()( static_cast<FloatData *>( data ), functor, errorHandler );
		case DoubleDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, DoubleData, ErrorHandler >
			::template Func<Enabler>()( static_cast<DoubleData *>( data ), functor, errorHandler );
		case IntDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, IntData, ErrorHandler >
			::template Func<Enabler>()( static_cast<IntData *>( data ), functor, errorHandler );
		case UIntDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UIntData, ErrorHandler >
			::template Func<Enabler>()( static_cast<UIntData *>( data ), functor, errorHandler );
		case CharDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, CharData, ErrorHandler >
			::template Func<Enabler>()( static_cast<CharData *>( data ), functor, errorHandler );
		case UCharDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UCharData, ErrorHandler >
			::template Func<Enabler>()( static_cast<UCharData *>( data ), functor, errorHandler );
		case ShortDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, ShortData, ErrorHandler >
			::template Func<Enabler>()( static_cast<ShortData *>( data ), functor, errorHandler );
		case UShortDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UShortData, ErrorHandler >
			::template Func<Enabler>()( static_cast<UShortData *>( data ), functor, errorHandler );
		case Int64DataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Int64Data, ErrorHandler >
			::template Func<Enabler>()( static_cast<Int64Data *>( data ), functor, errorHandler );
		case UInt64DataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UInt64Data, ErrorHandler >
			::template Func<Enabler>()( static_cast<UInt64Data *>( data ), functor, errorHandler );
		case StringDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, StringData, ErrorHandler >
			::template Func<Enabler>()( static_cast<StringData *>( data ), functor, errorHandler );
		case InternedStringDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, InternedStringData, ErrorHandler >
			::template Func<Enabler>()( static_cast<InternedStringData *>( data ), functor, errorHandler );
		case HalfDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, HalfData, ErrorHandler >
			::template Func<Enabler>()( static_cast<HalfData *>( data ), functor, errorHandler );
		case V2iDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V2iData, ErrorHandler >
			::template Func<Enabler>()( static_cast<V2iData *>( data ), functor, errorHandler );
		case V3iDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V3iData, ErrorHandler >
			::template Func<Enabler>()( static_cast<V3iData *>( data ), functor, errorHandler );
		case V2fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V2fData, ErrorHandler >
			::template Func<Enabler>()( static_cast<V2fData *>( data ), functor, errorHandler );
		case V3fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V3fData, ErrorHandler >
			::template Func<Enabler>()( static_cast<V3fData *>( data ), functor, errorHandler );
		case V2dDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V2dData, ErrorHandler >
			::template Func<Enabler>()( static_cast<V2dData *>( data ), functor, errorHandler );
		case V3dDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V3dData, ErrorHandler >
			::template Func<Enabler>()( static_cast<V3dData *>( data ), functor, errorHandler );
		case Color3fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Color3fData, ErrorHandler >
			::template Func<Enabler>()( static_cast<Color3fData *>( data ), functor, errorHandler );
		case Color4fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Color4fData, ErrorHandler >
			::template Func<Enabler>()( static_cast<Color4fData *>( data ), functor, errorHandler );
		case Box2iDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Box2iData, ErrorHandler >
			::template Func<Enabler>()( static_cast<Box2iData *>( data ), functor, errorHandler );
		case Box2fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Box2fData, ErrorHandler >
			::template Func<Enabler>()( static_cast<Box2fData *>( data ), functor, errorHandler );
		case Box3fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Box3fData, ErrorHandler >
			::template Func<Enabler>()( static_cast<Box3fData *>( data ), functor, errorHandler );
		case Box2dDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Box2dData, ErrorHandler >
			::template Func<Enabler>()( static_cast<Box2dData *>( data ), functor, errorHandler );
		case Box3dDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Box3dData, ErrorHandler >
			::template Func<Enabler>()( static_cast<Box3dData *>( data ), functor, errorHandler );
		case M33fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M33fData, ErrorHandler >
			::template Func<Enabler>()( static_cast<M33fData *>( data ), functor, errorHandler );
		case M33dDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M33dData, ErrorHandler >
			::template Func<Enabler>()( static_cast<M33dData *>( data ), functor, errorHandler );
		case M44fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M44fData, ErrorHandler >
			::template Func<Enabler>()( static_cast<M44fData *>( data ), functor, errorHandler );
		case M44dDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M44dData, ErrorHandler >
			::template Func<Enabler>()( static_cast<M44dData *>( data ), functor, errorHandler );
		case TransformationMatrixfDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, TransformationMatrixfData, ErrorHandler >
			::template Func<Enabler>()( static_cast<TransformationMatrixfData *>( data ), functor, errorHandler );
		case TransformationMatrixdDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, TransformationMatrixdData, ErrorHandler >
			::template Func<Enabler>()( static_cast<TransformationMatrixdData *>( data ), functor, errorHandler );
		case QuatfDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, QuatfData, ErrorHandler >
			::template Func<Enabler>()( static_cast<QuatfData *>( data ), functor, errorHandler );
		case QuatdDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, QuatdData, ErrorHandler >
			::template Func<Enabler>()( static_cast<QuatdData *>( data ), functor, errorHandler );
		case SplineffDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, SplineffData, ErrorHandler >
			::template Func<Enabler>()( static_cast<SplineffData *>( data ), functor, errorHandler );
		case SplineddDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, SplineddData, ErrorHandler >
			::template Func<Enabler>()( static_cast<SplineddData *>( data ), functor, errorHandler );
		case SplinefColor3fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, SplinefColor3fData, ErrorHandler >
			::template Func<Enabler>()( static_cast<SplinefColor3fData *>( data ), functor, errorHandler );
		case SplinefColor4fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, SplinefColor4fData, ErrorHandler >
			::template Func<Enabler>()( static_cast<SplinefColor4fData *>( data ), functor, errorHandler );
		case DateTimeDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, DateTimeData, ErrorHandler >
			::template Func<Enabler>()( static_cast<DateTimeData *>( data ), functor, errorHandler );
		case TimePeriodDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, TimePeriodData, ErrorHandler >
			::template Func<Enabler>()( static_cast<TimePeriodData *>( data ), functor, errorHandler );
		case TimeDurationDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, TimeDurationData, ErrorHandler >
			::template Func<Enabler>()( static_cast<TimeDurationData *>( data ), functor, errorHandler );

		case BoolVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, BoolVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<BoolVectorData *>( data ), functor, errorHandler );
		case FloatVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, FloatVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<FloatVectorData *>( data ), functor, errorHandler );
		case DoubleVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, DoubleVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<DoubleVectorData *>( data ), functor, errorHandler );
		case HalfVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, HalfVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<HalfVectorData *>( data ), functor, errorHandler );
		case IntVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, IntVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<IntVectorData *>( data ), functor, errorHandler );
		case UIntVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UIntVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<UIntVectorData *>( data ), functor, errorHandler );
		case CharVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, CharVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<CharVectorData *>( data ), functor, errorHandler );
		case UCharVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UCharVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<UCharVectorData *>( data ), functor, errorHandler );
		case ShortVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, ShortVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<ShortVectorData *>( data ), functor, errorHandler );
		case UShortVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UShortVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<UShortVectorData *>( data ), functor, errorHandler );
		case Int64VectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Int64VectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<Int64VectorData *>( data ), functor, errorHandler );
		case UInt64VectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UInt64VectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<UInt64VectorData *>( data ), functor, errorHandler );
		case StringVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, StringVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<StringVectorData *>( data ), functor, errorHandler );
		case InternedStringVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, InternedStringVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<InternedStringVectorData *>( data ), functor, errorHandler );
		case V2iVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V2iVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<V2iVectorData *>( data ), functor, errorHandler );
		case V2fVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V2fVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<V2fVectorData *>( data ), functor, errorHandler );
		case V2dVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V2dVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<V2dVectorData *>( data ), functor, errorHandler );
		case V3iVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V3iVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<V3iVectorData *>( data ), functor, errorHandler );
		case V3fVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V3fVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<V3fVectorData *>( data ), functor, errorHandler );
		case V3dVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V3dVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<V3dVectorData *>( data ), functor, errorHandler );
		case Box3fVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Box3fVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<Box3fVectorData *>( data ), functor, errorHandler );
		case Box3dVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Box3dVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<Box3dVectorData *>( data ), functor, errorHandler );
		case M33fVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M33fVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<M33fVectorData *>( data ), functor, errorHandler );
		case M33dVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M33dVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<M33dVectorData *>( data ), functor, errorHandler );
		case M44fVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M44fVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<M44fVectorData *>( data ), functor, errorHandler );
		case M44dVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M44dVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<M44dVectorData *>( data ), functor, errorHandler );
		case QuatfVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, QuatfVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<QuatfVectorData *>( data ), functor, errorHandler );
		case QuatdVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, QuatdVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<QuatdVectorData *>( data ), functor, errorHandler );
		case Color3fVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Color3fVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<Color3fVectorData *>( data ), functor, errorHandler );
		case Color4fVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Color4fVectorData, ErrorHandler >
			::template Func<Enabler>()( static_cast<Color4fVectorData *>( data ), functor, errorHandler );
		default :
			throw InvalidArgumentException( "Data supplied is not of a known TypedData type." );
	}
}

template< class Functor, template<typename> class Enabler, typename ErrorHandler >
typename Functor::ReturnType despatchTypedData( Data *data )
{
	ErrorHandler e;
	Functor functor;
	return despatchTypedData<Functor, Enabler, ErrorHandler>( data, functor, e );
}

template< class Functor, template<typename> class Enabler, typename ErrorHandler >
typename Functor::ReturnType despatchTypedData( Data *data, Functor &functor )
{
	ErrorHandler e;
	return despatchTypedData<Functor, Enabler, ErrorHandler>( data, functor, e );
}

template< class Functor, template<typename> class Enabler >
typename Functor::ReturnType despatchTypedData( Data *data, Functor &functor )
{
	return despatchTypedData< Functor, Enabler, Detail::DespatchTypedDataExceptionError>( data, functor );
}

template< class Functor, template<typename> class Enabler >
typename Functor::ReturnType despatchTypedData( Data *data )
{
	Functor functor;
	return despatchTypedData< Functor, Enabler, Detail::DespatchTypedDataExceptionError>( data, functor );
}

template< class Functor >
typename Functor::ReturnType despatchTypedData( Data *data, Functor &functor )
{
	return despatchTypedData< Functor, TypeTraits::IsTypedData, Detail::DespatchTypedDataExceptionError>( data, functor );
}

template< class Functor >
typename Functor::ReturnType despatchTypedData( Data *data )
{
	Functor functor;
	return despatchTypedData< Functor, TypeTraits::IsTypedData, Detail::DespatchTypedDataExceptionError>( data, functor );
}

struct TypedDataSize
{
	typedef size_t ReturnType;

	template<typename T, typename Enable = void>
	struct TypedDataSizeHelper
	{
		ReturnType operator()( const T *data ) const
		{
			BOOST_STATIC_ASSERT( sizeof(T) == 0 );
			return 0;
		}
	};

	template<typename T>
	ReturnType operator()( const T *data ) const
	{
		assert( data );
		return TypedDataSizeHelper<T>()( data );
	}
};

template<typename T>
struct TypedDataSize::TypedDataSizeHelper< T, typename boost::enable_if< TypeTraits::IsSimpleTypedData<T> >::type >
{
	size_t operator()( const T *data ) const
	{
		assert( data );
		return 1;
	}
};

template<typename T>
struct TypedDataSize::TypedDataSizeHelper< T, typename boost::enable_if< TypeTraits::IsVectorTypedData<T> >::type >
{
	size_t operator()( const T *data ) const
	{
		assert( data );
		return data->readable().size();
	}
};

struct TypedDataAddress
{
	typedef const void *ReturnType;

	template<typename T, typename Enable = void>

	struct TypedDataAddressHelper
	{
		ReturnType operator()( const T *data ) const
		{
			BOOST_STATIC_ASSERT( sizeof(T) == 0 );
			return nullptr;
		}
	};
 	
	template<typename T>
	ReturnType operator()( const T *data ) const
	{
		assert( data );
		return TypedDataAddressHelper<T>()( data );
	}
};

template<typename T>
struct TypedDataAddress::TypedDataAddressHelper< T, typename boost::enable_if< TypeTraits::IsSimpleTypedData<T> >::type >
{
	const void *operator()( const T *data ) const
	{
		assert( data );
		return &( data->readable() );
	}
};

template<typename T>
struct TypedDataAddress::TypedDataAddressHelper< T, typename boost::enable_if< boost::mpl::and_< TypeTraits::IsVectorTypedData<T>, boost::mpl::not_< boost::is_same< typename TypeTraits::VectorValueType<T>::type, bool > > > >::type >
{
	const void *operator()( const T *data ) const
	{
		assert( data );
		return &*(data->readable().begin());
	}
};

template<typename T>
struct TypedDataAddress::TypedDataAddressHelper< T, typename boost::enable_if< boost::mpl::and_< TypeTraits::IsVectorTypedData<T>, boost::is_same< typename TypeTraits::VectorValueType<T>::type, bool > > >::type >
{
	const void *operator()( const T *data ) const
	{
		assert( data );
		return data->baseReadable();
	}
};


template<template<typename> class Trait>
bool despatchTraitsTest( const Data *data )
{
	return despatchTypedData<TraitsTest, Trait, DespatchTypedDataIgnoreError>( const_cast<Data *>( data ) );
}

struct TraitsTest
{
	typedef bool ReturnType;

	template< typename T >
	ReturnType operator()( const T *d ) const
	{
		return true;
	}
};

}

#endif // IE_CORE_DESPATCHTYPEDDATA_INL
