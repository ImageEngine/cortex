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

//! \file DespatchTypedData.h
/// Defines useful functions for calling template functions to manipulate
/// TypedData instances when given only a DataPtr.


#ifndef IE_CORE_DESPATCHTYPEDDATA_H
#define IE_CORE_DESPATCHTYPEDDATA_H

#include <cassert>

#include "boost/format.hpp"

#include "IECore/Exception.h"
#include "IECore/Data.h"

namespace IECore
{

/// Given a DataPtr which points to one a instance of the TypedData classes, call the operator() member function on an
/// instance of Functor, passing the DataPtr downcast to the correct type. A template parameter, Enabler, allows the
/// function to run only on certain subsets of TypedData, for example, only VectorTypeData. The template specified
/// here should be compatible with boost::mpl - see examples in TypeTraits.h.
///
/// When an instance of a type not supported by the Enabler is encountered, the ErrorHandler is called. An ErrorHandler
/// class should take the following form:
///
/// \code
/// struct EH
/// {
///     template<typename DataType, typename Functor>
///     void operator()( typename DataType::ConstPtr , const Functor& )
///     {
///         // Handle error here
///     }
/// };
/// \endcode
///
/// A Functor should look this like:
///
/// \code
/// struct F
///{
///	typedef unspecified-type ReturnType;
///
///	template<typename T>
///	ReturnType operator()( typename T::Ptr data )
///	{
///         // Deal with the data and, optionally, return some value
///	}
///};
/// \endcode
///
/// Example uses can be found it the ImageWriter-derived classes
template< class Functor, template<typename> class Enabler, typename ErrorHandler >
typename Functor::ReturnType despatchTypedData( const DataPtr &data, Functor &functor, ErrorHandler &errorHandler );

/// Convenience version of despatchTypedData which constructs an ErrorHandler using its default constructor
template< class Functor, template<typename> class Enabler, typename ErrorHandler >
typename Functor::ReturnType despatchTypedData( const DataPtr &data, Functor &functor );

/// Convenience version of despatchTypedData which constructs the ErrorHandler and Functor using their default constructors
template< class Functor, template<typename> class Enabler, typename ErrorHandler >
typename Functor::ReturnType despatchTypedData( const DataPtr &data );

/// Convenience version of despatchTypedData, which throws an InvalidArgumentException when data which doesn't match the Enabler
/// is encountered
template< class Functor, template<typename> class Enabler >
typename Functor::ReturnType despatchTypedData( const DataPtr &data, Functor &functor );

/// Convenience version of despatchTypedData, which throws an InvalidArgumentException when data which doesn't match the Enabler
/// is encountered.
template< class Functor, template<typename> class Enabler >
typename Functor::ReturnType despatchTypedData( const DataPtr &data );

/// Convenience version of despatchTypedData which operates on all TypedData classes, and constructs an ErrorHandler
/// using its default constructor
template< class Functor >
typename Functor::ReturnType despatchTypedData( const DataPtr &data, Functor &functor );

/// Convenience version of despatchTypedData which operates on all TypedData classes, constructs the ErrorHandler
/// and Functor using their default constructors, and throws an InvalidArgumentException when data which isn't TypedData
/// is encountered.
template< class Functor >
typename Functor::ReturnType despatchTypedData( const DataPtr &data );

/// Simply returns the result of Trait - this can be used to check TypeTraits at runtime.
/// e.g. bool isSimple = despatchTraitsTest<TypeTraits::IsSimpleTypedData>( data ).
template<template<typename> class Trait>
bool despatchTraitsTest( const DataPtr &data );

/// An error handler which simply ignores any errors encountered
struct DespatchTypedDataIgnoreError;

/// A functor which can return the size (or "length") of any given TypedData. By definition, the size of a SimpleTypedData is 1,
/// and the size of a VectorTypedData is equal to the size of its contained vector.
struct TypedDataSize;

/// A functor which can return the address of the data held by a TypedData object.
struct TypedDataAddress;

/// A functor which returns PrimitiveVariable::Vertex for VectorTypedData, PrimitiveVariable::Constant for SimpleTypedData,
/// and PrimitiveVariable::Invalid otherwise (or if using the DespatchTypedDataIgnoreError error handler)
struct TypedDataInterpolation;

/// A functor which always returns true. This can be used to test data traits at runtime.
///
/// e.g. :
/// bool isSimple = despatchTypedData<TraitsTest, TypeTraits::IsSimpleTypedData, DespatchTypedDataIgnoreError>( data )
///
/// This is used by the despatchTraitsTest() convenience function (see above).
struct TraitsTest;

} // namespace IECore

#include "IECore/DespatchTypedData.inl"

#endif // IE_CORE_DESPATCHTYPEDDATA_H
