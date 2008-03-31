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
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIG4HT OWNER OR
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
/// TypedData instances when given only a DataPtr. This file, and the
/// functions/classes declared within it are now deprecated. See
/// DespatchTypedData.h for a replacement.

#ifndef IE_CORE_TYPEDDATADESPATCH_H
#define IE_CORE_TYPEDDATADESPATCH_H

#include <cassert>

#include "boost/format.hpp"

#include "IECore/Exception.h"
#include "IECore/VectorTypedData.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/TransformationMatrixData.h"

#include "IECore/TypeTraits.h"
#include "IECore/DespatchTypedData.h"

namespace IECore
{

namespace Detail
{
/// \todo This can be removed once despatchVectorTypedDataFn and despatchSimpleTypedDataFn are.
template< typename R, template<typename> class Functor, class A >
struct DespatchTypedDataFnAdapter
{
	struct Func
	{
		typedef R ReturnType;				
		const A &m_args;
		
		Func( const A &args ) : m_args( args )
		{
		}
		
		template<typename T>
		R operator()( typename T::Ptr data )
		{
			return Functor<T>()( data, m_args );
		}
	};
};

/// \todo This can be removed once despatchVectorTypedDataFn and despatchSimpleTypedDataFn are.
struct DespatchTypedDataFnErrorHandler
{
	std::string m_error;
	
	template<typename T, typename F>
	void operator()( typename T::ConstPtr data, const F& functor )
	{
		throw InvalidArgumentException( m_error );
	}
};

} // namespace Detail

/// \deprecated
template<typename ReturnType, template<typename> class Functor, class Args>
ReturnType despatchVectorTypedDataFn( const DataPtr &data, const Args &functorArgs )
{
	typedef typename Detail::DespatchTypedDataFnAdapter<ReturnType, Functor, Args>::Func F;

	F f( functorArgs );
	
	Detail::DespatchTypedDataFnErrorHandler errorHandler;
	errorHandler.m_error = "Data supplied is not of a known VectorTypedData type.";
	
	return despatchTypedData< F, TypeTraits::IsVectorTypedData, Detail::DespatchTypedDataFnErrorHandler >( data, f, errorHandler );
}

/// \deprecated
template<typename ReturnType, template<typename> class Functor, class Args>
ReturnType despatchSimpleTypedDataFn( const DataPtr &data, const Args &functorArgs )
{
	typedef typename Detail::DespatchTypedDataFnAdapter<ReturnType, Functor, Args>::Func F;

	F f( functorArgs );
	
	Detail::DespatchTypedDataFnErrorHandler errorHandler;
	errorHandler.m_error = "Data supplied is not of a known SimpleTypedData type.";
	
	return despatchTypedData< F, TypeTraits::IsSimpleTypedData, Detail::DespatchTypedDataFnErrorHandler >( data, f, errorHandler );
}

/// Arguments for the VectorTypedDataSize functor.
/// \deprecated
struct VectorTypedDataSizeArgs
{
};

/// A function for use with despatchVectorTypedDataFn(). It simply
/// returns the size of the held vector.
/// \deprecated
template<typename T>
struct VectorTypedDataSize
{
	size_t operator() ( boost::intrusive_ptr<T> data, VectorTypedDataSizeArgs )
	{
		return data->readable().size();
	}
};	

/// Arguments for the VectorTypedDataSize functor.
/// \deprecated
struct SimpleTypedDataAddressArgs
{
};

/// A function for use with despatchVectorTypedDataFn(). It simply
/// returns the address of the held type.
/// \deprecated
template<typename T>
struct SimpleTypedDataAddress
{
	const void *operator() ( boost::intrusive_ptr<T> data, SimpleTypedDataAddressArgs )
	{
		return &(data->readable());
	}
};

/// Arguments for the VectorTypedDataAddress functor.
/// \deprecated
struct VectorTypedDataAddressArgs
{
};

/// A function for use with despatchVectorTypedDataFn(). It simply
/// returns the address of the first element of the vector.
/// \deprecated
template<typename T>
struct VectorTypedDataAddress
{
	const void *operator() ( boost::intrusive_ptr<T> data, VectorTypedDataAddressArgs )
	{
		return &*(data->readable().begin());
	}
};

/// Arguments for the VectorTypedDataClear functor.
/// \deprecated
struct VectorTypedDataClearArgs
{
};

/// A function for use with despatchVectorTypedDataFn(). It simply
/// clears the underlying vector.
/// \deprecated
template<typename T>
struct VectorTypedDataClear
{
	void operator() ( boost::intrusive_ptr<T> data, VectorTypedDataClearArgs args )
	{
		data->writable().clear();
	}
};

}; // namespace IECore

#endif // IE_CORE_TYPEDDATADESPATCH_H
