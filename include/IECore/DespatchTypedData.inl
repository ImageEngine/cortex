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
#include "IECore/PrimitiveVariable.h"
#include "IECore/VectorTypedData.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TransformationMatrixData.h"
#include "IECore/SplineData.h"
#include "IECore/CubeColorLookupData.h"
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

		ReturnType operator()( typename DataType::Ptr data, Functor &functor, ErrorHandler &errorHandler )
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

		ReturnType operator()( typename DataType::Ptr data, Functor &functor, ErrorHandler & )
		{
			assert( data );

			return functor.template operator()<DataType>( data );
		}
	};
};

struct DespatchTypedDataExceptionError
{
	template<typename T, typename F>
	void operator()( typename T::ConstPtr data, const F& functor )
	{
		throw InvalidArgumentException( ( boost::format( "Unhandled data of type %s encountered by DespatchTypedData" ) % data->typeName() ).str() );
	}
};

} // namespace Detail


struct DespatchTypedDataIgnoreError
{
	template<typename T, typename F>
	void operator()( typename T::ConstPtr data, const F& functor )
	{
	}
};

template< class Functor, template<typename> class Enabler, typename ErrorHandler >
typename Functor::ReturnType despatchTypedData( const DataPtr &data, Functor &functor, ErrorHandler &errorHandler )
{
	assert( data );

	switch( data->typeId() )
	{
		case BoolDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, BoolData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<BoolData>( data ), functor, errorHandler );
		case FloatDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, FloatData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<FloatData>( data ), functor, errorHandler );
		case DoubleDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, DoubleData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<DoubleData>( data ), functor, errorHandler );
		case IntDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, IntData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<IntData>( data ), functor, errorHandler );
		case UIntDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UIntData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<UIntData>( data ), functor, errorHandler );
		case CharDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, CharData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<CharData>( data ), functor, errorHandler );
		case UCharDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UCharData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<UCharData>( data ), functor, errorHandler );
		case ShortDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, ShortData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<ShortData>( data ), functor, errorHandler );
		case UShortDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UShortData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<UShortData>( data ), functor, errorHandler );
		case Int64DataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Int64Data, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Int64Data>( data ), functor, errorHandler );
		case UInt64DataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UInt64Data, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<UInt64Data>( data ), functor, errorHandler );
		case StringDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, StringData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<StringData>( data ), functor, errorHandler );
		case InternedStringDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, InternedStringData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<InternedStringData>( data ), functor, errorHandler );
		case HalfDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, HalfData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<HalfData>( data ), functor, errorHandler );
		case V2iDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V2iData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<V2iData>( data ), functor, errorHandler );
		case V3iDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V3iData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<V3iData>( data ), functor, errorHandler );
		case V2fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V2fData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<V2fData>( data ), functor, errorHandler );
		case V3fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V3fData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<V3fData>( data ), functor, errorHandler );
		case V2dDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V2dData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<V2dData>( data ), functor, errorHandler );
		case V3dDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V3dData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<V3dData>( data ), functor, errorHandler );
		case Color3fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Color3fData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Color3fData>( data ), functor, errorHandler );
		case Color4fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Color4fData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Color4fData>( data ), functor, errorHandler );
		case Color3dDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Color3dData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Color3dData>( data ), functor, errorHandler );
		case Color4dDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Color4dData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Color4dData>( data ), functor, errorHandler );
		case Box2iDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Box2iData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Box2iData>( data ), functor, errorHandler );
		case Box2fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Box2fData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Box2fData>( data ), functor, errorHandler );
		case Box3fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Box3fData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Box3fData>( data ), functor, errorHandler );
		case Box2dDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Box2dData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Box2dData>( data ), functor, errorHandler );
		case Box3dDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Box3dData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Box3dData>( data ), functor, errorHandler );
		case M33fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M33fData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<M33fData>( data ), functor, errorHandler );
		case M33dDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M33dData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<M33dData>( data ), functor, errorHandler );
		case M44fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M44fData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<M44fData>( data ), functor, errorHandler );
		case M44dDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M44dData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<M44dData>( data ), functor, errorHandler );
		case TransformationMatrixfDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, TransformationMatrixfData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<TransformationMatrixfData>( data ), functor, errorHandler );
		case TransformationMatrixdDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, TransformationMatrixdData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<TransformationMatrixdData>( data ), functor, errorHandler );
		case QuatfDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, QuatfData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<QuatfData>( data ), functor, errorHandler );
		case QuatdDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, QuatdData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<QuatdData>( data ), functor, errorHandler );
		case SplineffDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, SplineffData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<SplineffData>( data ), functor, errorHandler );
		case SplineddDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, SplineddData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<SplineddData>( data ), functor, errorHandler );
		case SplinefColor3fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, SplinefColor3fData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<SplinefColor3fData>( data ), functor, errorHandler );
		case SplinefColor4fDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, SplinefColor4fData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<SplinefColor4fData>( data ), functor, errorHandler );
		case CubeColorLookupfDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, CubeColorLookupfData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<CubeColorLookupfData>( data ), functor, errorHandler );
		case CubeColorLookupdDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, CubeColorLookupdData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<CubeColorLookupdData>( data ), functor, errorHandler );
		case DateTimeDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, DateTimeData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<DateTimeData>( data ), functor, errorHandler );
		case TimePeriodDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, TimePeriodData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<TimePeriodData>( data ), functor, errorHandler );
		case TimeDurationDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, TimeDurationData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<TimeDurationData>( data ), functor, errorHandler );

		case BoolVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, BoolVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<BoolVectorData>( data ), functor, errorHandler );
		case FloatVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, FloatVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<FloatVectorData>( data ), functor, errorHandler );
		case DoubleVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, DoubleVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<DoubleVectorData>( data ), functor, errorHandler );
		case HalfVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, HalfVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<HalfVectorData>( data ), functor, errorHandler );
		case IntVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, IntVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<IntVectorData>( data ), functor, errorHandler );
		case UIntVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UIntVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<UIntVectorData>( data ), functor, errorHandler );
		case CharVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, CharVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<CharVectorData>( data ), functor, errorHandler );
		case UCharVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UCharVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<UCharVectorData>( data ), functor, errorHandler );
		case ShortVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, ShortVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<ShortVectorData>( data ), functor, errorHandler );
		case UShortVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UShortVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<UShortVectorData>( data ), functor, errorHandler );
		case Int64VectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Int64VectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Int64VectorData>( data ), functor, errorHandler );
		case UInt64VectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, UInt64VectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<UInt64VectorData>( data ), functor, errorHandler );
		case StringVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, StringVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<StringVectorData>( data ), functor, errorHandler );
		case InternedStringVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, InternedStringVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<InternedStringVectorData>( data ), functor, errorHandler );
		case V2iVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V2iVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<V2iVectorData>( data ), functor, errorHandler );
		case V2fVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V2fVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<V2fVectorData>( data ), functor, errorHandler );
		case V2dVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V2dVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<V2dVectorData>( data ), functor, errorHandler );
		case V3iVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V3iVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<V3iVectorData>( data ), functor, errorHandler );
		case V3fVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V3fVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<V3fVectorData>( data ), functor, errorHandler );
		case V3dVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, V3dVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<V3dVectorData>( data ), functor, errorHandler );
		case Box3fVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Box3fVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Box3fVectorData>( data ), functor, errorHandler );
		case Box3dVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Box3dVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Box3dVectorData>( data ), functor, errorHandler );
		case M33fVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M33fVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<M33fVectorData>( data ), functor, errorHandler );
		case M33dVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M33dVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<M33dVectorData>( data ), functor, errorHandler );
		case M44fVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M44fVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<M44fVectorData>( data ), functor, errorHandler );
		case M44dVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, M44dVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<M44dVectorData>( data ), functor, errorHandler );
		case QuatfVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, QuatfVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<QuatfVectorData>( data ), functor, errorHandler );
		case QuatdVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, QuatdVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<QuatdVectorData>( data ), functor, errorHandler );
		case Color3fVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Color3fVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Color3fVectorData>( data ), functor, errorHandler );
		case Color4fVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Color4fVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Color4fVectorData>( data ), functor, errorHandler );
		case Color3dVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Color3dVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Color3dVectorData>( data ), functor, errorHandler );
		case Color4dVectorDataTypeId :
			return
			typename Detail::DespatchTypedData< Functor, Color4dVectorData, ErrorHandler >
			::template Func<Enabler>()( staticPointerCast<Color4dVectorData>( data ), functor, errorHandler );

		default :
			throw InvalidArgumentException( "Data supplied is not of a known TypedData type." );
	}
}

template< class Functor, template<typename> class Enabler, typename ErrorHandler >
typename Functor::ReturnType despatchTypedData( const DataPtr &data )
{
	ErrorHandler e;
	Functor functor;
	return despatchTypedData<Functor, Enabler, ErrorHandler>( data, functor, e );
}

template< class Functor, template<typename> class Enabler, typename ErrorHandler >
typename Functor::ReturnType despatchTypedData( const DataPtr &data, Functor &functor )
{
	ErrorHandler e;
	return despatchTypedData<Functor, Enabler, ErrorHandler>( data, functor, e );
}

template< class Functor, template<typename> class Enabler >
typename Functor::ReturnType despatchTypedData( const DataPtr &data, Functor &functor )
{
	return despatchTypedData< Functor, Enabler, Detail::DespatchTypedDataExceptionError>( data, functor );
}

template< class Functor, template<typename> class Enabler >
typename Functor::ReturnType despatchTypedData( const DataPtr &data )
{
	Functor functor;
	return despatchTypedData< Functor, Enabler, Detail::DespatchTypedDataExceptionError>( data, functor );
}

template< class Functor >
typename Functor::ReturnType despatchTypedData( const DataPtr &data, Functor &functor )
{
	return despatchTypedData< Functor, TypeTraits::IsTypedData, Detail::DespatchTypedDataExceptionError>( data, functor );
}

template< class Functor >
typename Functor::ReturnType despatchTypedData( const DataPtr &data )
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
		ReturnType operator()( typename T::ConstPtr data ) const
		{
			BOOST_STATIC_ASSERT( sizeof(T) == 0 );
			return 0;
		}
	};

	template<typename T>
	ReturnType operator()( typename T::ConstPtr data ) const
	{
		assert( data );
		return TypedDataSizeHelper<T>()( data );
	}
};

template<typename T>
struct TypedDataSize::TypedDataSizeHelper< T, typename boost::enable_if< TypeTraits::IsSimpleTypedData<T> >::type >
{
	size_t operator()( typename T::ConstPtr data ) const
	{
		assert( data );
		return 1;
	}
};

template<typename T>
struct TypedDataSize::TypedDataSizeHelper< T, typename boost::enable_if< TypeTraits::IsVectorTypedData<T> >::type >
{
	size_t operator()( typename T::ConstPtr data ) const
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
		ReturnType operator()( typename T::ConstPtr data ) const
		{
			BOOST_STATIC_ASSERT( sizeof(T) == 0 );
			return 0;
		}
	};
 	
	template<typename T>
	ReturnType operator()( typename T::ConstPtr data ) const
	{
		assert( data );
		return TypedDataAddressHelper<T>()( data );
	}
};

template<typename T>
struct TypedDataAddress::TypedDataAddressHelper< T, typename boost::enable_if< TypeTraits::IsSimpleTypedData<T> >::type >
{
	const void *operator()( typename T::ConstPtr data ) const
	{
		assert( data );
		return &( data->readable() );
	}
};

template<typename T>
struct TypedDataAddress::TypedDataAddressHelper< T, typename boost::enable_if< boost::mpl::and_< TypeTraits::IsVectorTypedData<T>, boost::mpl::not_< boost::is_same< typename TypeTraits::VectorValueType<T>::type, bool > > > >::type >
{
	const void *operator()( typename T::ConstPtr data ) const
	{
		assert( data );
		return &*(data->readable().begin());
	}
};

template<typename T>
struct TypedDataAddress::TypedDataAddressHelper< T, typename boost::enable_if< boost::mpl::and_< TypeTraits::IsVectorTypedData<T>, boost::is_same< typename TypeTraits::VectorValueType<T>::type, bool > > >::type >
{
	const void *operator()( typename T::ConstPtr data ) const
	{
		assert( data );
		return data->baseReadable();
	}
};

struct TypedDataInterpolation
{
	typedef PrimitiveVariable::Interpolation ReturnType;

	template< typename T, typename Enable = void >
	struct TypedDataInterpolationHelper
	{
		ReturnType operator()( typename T::ConstPtr d ) const
		{
			return PrimitiveVariable::Invalid;
		}
	};

	template< typename T >
	ReturnType operator()( typename T::ConstPtr d ) const
	{
		return TypedDataInterpolationHelper<T>()( d );
	}
};

template< typename T >
struct TypedDataInterpolation::TypedDataInterpolationHelper<T, typename boost::enable_if< TypeTraits::IsVectorTypedData<T> >::type >
{
	ReturnType operator()( typename T::ConstPtr d ) const
	{
		return PrimitiveVariable::Vertex;
	}
};

template< typename T >
struct TypedDataInterpolation::TypedDataInterpolationHelper<T, typename boost::enable_if< TypeTraits::IsSimpleTypedData<T> >::type >
{
	ReturnType operator()( typename T::ConstPtr d ) const
	{
		return PrimitiveVariable::Constant;
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
	ReturnType operator()( typename T::ConstPtr d ) const
	{
		return true;
	}
};

}

#endif // IE_CORE_DESPATCHTYPEDDATA_INL
