//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

//! \file TypedDataDespatch.h
/// Defines useful functions for calling template functions to manipulate
/// TypedData instances when given only a DataPtr.

#ifndef IE_CORE_TYPEDDATADESPATCH_H
#define IE_CORE_TYPEDDATADESPATCH_H

#include "IECore/VectorTypedData.h"
#include "IECore/SimpleTypedData.h"

namespace IECore
{
	

template<typename ReturnType, template<typename T> class Functor, class Args>
ReturnType despatchSimpleTypedDataFn( const DataPtr &data, const Args &functorArgs )
{
	switch( data->typeId() )
	{
		case BoolDataTypeId :
			return Functor<BoolData>()( boost::static_pointer_cast<BoolData>( data ), functorArgs );
		case FloatDataTypeId :
			return Functor<FloatData>()( boost::static_pointer_cast<FloatData>( data ), functorArgs );
		case DoubleDataTypeId :
			return Functor<DoubleData>()( boost::static_pointer_cast<DoubleData>( data ), functorArgs );
		case IntDataTypeId :
			return Functor<IntData>()( boost::static_pointer_cast<IntData>( data ), functorArgs );
		case LongDataTypeId :
			return Functor<LongData>()( boost::static_pointer_cast<LongData>( data ), functorArgs );
		case UIntDataTypeId :
			return Functor<UIntData>()( boost::static_pointer_cast<UIntData>( data ), functorArgs );
		case CharDataTypeId :
			return Functor<CharData>()( boost::static_pointer_cast<CharData>( data ), functorArgs );
		case UCharDataTypeId :
			return Functor<UCharData>()( boost::static_pointer_cast<UCharData>( data ), functorArgs );
		case StringDataTypeId :
			return Functor<StringData>()( boost::static_pointer_cast<StringData>( data ), functorArgs );
		case HalfDataTypeId :
			return Functor<HalfData>()( boost::static_pointer_cast<HalfData>( data ), functorArgs );
		case V2iDataTypeId :
			return Functor<V2iData>()( boost::static_pointer_cast<V2iData>( data ), functorArgs );
		case V3iDataTypeId :
			return Functor<V3iData>()( boost::static_pointer_cast<V3iData>( data ), functorArgs );
		case V2fDataTypeId :
			return Functor<V2fData>()( boost::static_pointer_cast<V2fData>( data ), functorArgs );
		case V3fDataTypeId :
			return Functor<V3fData>()( boost::static_pointer_cast<V3fData>( data ), functorArgs );
		case V2dDataTypeId :
			return Functor<V2dData>()( boost::static_pointer_cast<V2dData>( data ), functorArgs );
		case V3dDataTypeId :
			return Functor<V3dData>()( boost::static_pointer_cast<V3dData>( data ), functorArgs );
		case Color3fDataTypeId :
			return Functor<Color3fData>()( boost::static_pointer_cast<Color3fData>( data ), functorArgs );
		case Color4fDataTypeId :
			return Functor<Color4fData>()( boost::static_pointer_cast<Color4fData>( data ), functorArgs );
		case Color3dDataTypeId :
			return Functor<Color3dData>()( boost::static_pointer_cast<Color3dData>( data ), functorArgs );
		case Color4dDataTypeId :
			return Functor<Color4dData>()( boost::static_pointer_cast<Color4dData>( data ), functorArgs );
		case Box2iDataTypeId :
			return Functor<Box2iData>()( boost::static_pointer_cast<Box2iData>( data ), functorArgs );
		case Box2fDataTypeId :
			return Functor<Box2fData>()( boost::static_pointer_cast<Box2fData>( data ), functorArgs );
		case Box3fDataTypeId :
			return Functor<Box3fData>()( boost::static_pointer_cast<Box3fData>( data ), functorArgs );
		case Box2dDataTypeId :
			return Functor<Box2dData>()( boost::static_pointer_cast<Box2dData>( data ), functorArgs );
		case Box3dDataTypeId :
			return Functor<Box3dData>()( boost::static_pointer_cast<Box3dData>( data ), functorArgs );
		case M33fDataTypeId :
			return Functor<M33fData>()( boost::static_pointer_cast<M33fData>( data ), functorArgs );
		case M33dDataTypeId :
			return Functor<M33dData>()( boost::static_pointer_cast<M33dData>( data ), functorArgs );
		case M44fDataTypeId :
			return Functor<M44fData>()( boost::static_pointer_cast<M44fData>( data ), functorArgs );
		case M44dDataTypeId :
			return Functor<M44dData>()( boost::static_pointer_cast<M44dData>( data ), functorArgs );
		case QuatfDataTypeId :
			return Functor<QuatfData>()( boost::static_pointer_cast<QuatfData>( data ), functorArgs );
		case QuatdDataTypeId :
			return Functor<QuatdData>()( boost::static_pointer_cast<QuatdData>( data ), functorArgs );
		default :
			throw InvalidArgumentException( "Data supplied is not of a known SimpleTypedData type." );
	}
}

template<typename ReturnType, template<typename T> class Functor, class Args>
ReturnType despatchVectorTypedDataFn( const DataPtr &data, const Args &functorArgs )
{
	switch( data->typeId() )
	{
		case FloatVectorDataTypeId :
			return Functor<FloatVectorData>()( boost::static_pointer_cast<FloatVectorData>( data ), functorArgs );
		case DoubleVectorDataTypeId :
			return Functor<DoubleVectorData>()( boost::static_pointer_cast<DoubleVectorData>( data ), functorArgs );
		case HalfVectorDataTypeId :
			return Functor<HalfVectorData>()( boost::static_pointer_cast<HalfVectorData>( data ), functorArgs );	
		case IntVectorDataTypeId :
			return Functor<IntVectorData>()( boost::static_pointer_cast<IntVectorData>( data ), functorArgs );
		case UIntVectorDataTypeId :
			return Functor<UIntVectorData>()( boost::static_pointer_cast<UIntVectorData>( data ), functorArgs );
		case LongVectorDataTypeId :
			return Functor<LongVectorData>()( boost::static_pointer_cast<LongVectorData>( data ), functorArgs );
		case CharVectorDataTypeId :
			return Functor<CharVectorData>()( boost::static_pointer_cast<CharVectorData>( data ), functorArgs );
		case UCharVectorDataTypeId :
			return Functor<UCharVectorData>()( boost::static_pointer_cast<UCharVectorData>( data ), functorArgs );
		case StringVectorDataTypeId :
			return Functor<StringVectorData>()( boost::static_pointer_cast<StringVectorData>( data ), functorArgs );
		case V2fVectorDataTypeId :
			return Functor<V2fVectorData>()( boost::static_pointer_cast<V2fVectorData>( data ), functorArgs );
		case V2dVectorDataTypeId :
			return Functor<V2dVectorData>()( boost::static_pointer_cast<V2dVectorData>( data ), functorArgs );
		case V3fVectorDataTypeId :
			return Functor<V3fVectorData>()( boost::static_pointer_cast<V3fVectorData>( data ), functorArgs );
		case V3dVectorDataTypeId :
			return Functor<V3dVectorData>()( boost::static_pointer_cast<V3dVectorData>( data ), functorArgs );
		case Box3fVectorDataTypeId :
			return Functor<Box3fVectorData>()( boost::static_pointer_cast<Box3fVectorData>( data ), functorArgs );
		case Box3dVectorDataTypeId :
			return Functor<Box3dVectorData>()( boost::static_pointer_cast<Box3dVectorData>( data ), functorArgs );
		case M33fVectorDataTypeId :
			return Functor<M33fVectorData>()( boost::static_pointer_cast<M33fVectorData>( data ), functorArgs );
		case M33dVectorDataTypeId :
			return Functor<M33dVectorData>()( boost::static_pointer_cast<M33dVectorData>( data ), functorArgs );
		case M44fVectorDataTypeId :
			return Functor<M44fVectorData>()( boost::static_pointer_cast<M44fVectorData>( data ), functorArgs );
		case M44dVectorDataTypeId :
			return Functor<M44dVectorData>()( boost::static_pointer_cast<M44dVectorData>( data ), functorArgs );
		case QuatfVectorDataTypeId :
			return Functor<QuatfVectorData>()( boost::static_pointer_cast<QuatfVectorData>( data ), functorArgs );
		case QuatdVectorDataTypeId :
			return Functor<QuatdVectorData>()( boost::static_pointer_cast<QuatdVectorData>( data ), functorArgs );
		case Color3fVectorDataTypeId :
			return Functor<Color3fVectorData>()( boost::static_pointer_cast<Color3fVectorData>( data ), functorArgs );
		case Color4fVectorDataTypeId :
			return Functor<Color4fVectorData>()( boost::static_pointer_cast<Color4fVectorData>( data ), functorArgs );
		case Color3dVectorDataTypeId :
			return Functor<Color3dVectorData>()( boost::static_pointer_cast<Color3dVectorData>( data ), functorArgs );
		case Color4dVectorDataTypeId :
			return Functor<Color4dVectorData>()( boost::static_pointer_cast<Color4dVectorData>( data ), functorArgs );
		default :
			throw InvalidArgumentException( "Data supplied is not of a known VectorTypedData type." );
	}
}

/// Arguments for the VectorTypedDataSize functor.
struct VectorTypedDataSizeArgs
{
};

/// A function for use with despatchVectorTypedDataFn(). It simply
/// returns the size of the held vector.
template<typename T>
struct VectorTypedDataSize
{
	size_t operator() ( boost::intrusive_ptr<T> data, VectorTypedDataSizeArgs )
	{
		return data->readable().size();
	}
};	

/// Arguments for the VectorTypedDataSize functor.
struct SimpleTypedDataAddressArgs
{
};

/// A function for use with despatchVectorTypedDataFn(). It simply
/// returns the address of the held type.
template<typename T>
struct SimpleTypedDataAddress
{
	const void *operator() ( boost::intrusive_ptr<T> data, SimpleTypedDataAddressArgs )
	{
		return &(data->readable());
	}
};

/// Arguments for the VectorTypedDataAddress functor.
struct VectorTypedDataAddressArgs
{
};

/// A function for use with despatchVectorTypedDataFn(). It simply
/// returns the address of the first element of the vector.
template<typename T>
struct VectorTypedDataAddress
{
	const void *operator() ( boost::intrusive_ptr<T> data, VectorTypedDataAddressArgs )
	{
		return &*(data->readable().begin());
	}
};

/// Arguments for the VectorTypedDataClear functor.
struct VectorTypedDataClearArgs
{
};

/// A function for use with despatchVectorTypedDataFn(). It simply
/// clears the underlying vector.
template<typename T>
struct VectorTypedDataClear
{
	int operator() ( boost::intrusive_ptr<T> data, VectorTypedDataClearArgs args )
	{
		data->writable().clear();
		
		/// We have to return something, unforunately.
		return 0;
	}
};

}; // namespace IECore

#endif // IE_CORE_TYPEDDATADESPATCH_H
