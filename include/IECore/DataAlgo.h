//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2015, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_DATAALGO_H
#define IECORE_DATAALGO_H

#include "IECore/Data.h"
#include "IECore/GeometricTypedData.h"

namespace IECore
{

/// Try to get the geometric interpretation of the given data.  Returns None if the data is not geometric.
IECORE_API GeometricData::Interpretation getGeometricInterpretation( const IECore::Data *data );

/// Try to set the geometric interpretation of the given data.  Has no effect if the data is not geometric.
IECORE_API void setGeometricInterpretation( IECore::Data *data, GeometricData::Interpretation interpretation );

/// Calculate the unique values in the TypedVectorData data
IECORE_API IECore::DataPtr uniqueValues(const IECore::Data *data);

/// For VectorTypedData, returns the size of the vector.
/// For SimpleTypedData, returns 1. For all other types
/// returns 0.
IECORE_API size_t size( const IECore::Data *data );

/// For VectorTypedData, returns the address of the first
/// element in the vector. For SimpleTypedData, returns
/// the address of the held type. For all other types,
/// returns nullptr.
IECORE_API void *address( IECore::Data *data );
IECORE_API const void *address( const IECore::Data *data );

/// Downcasts `data` to its true derived type and returns the result of calling `functor( derived )`.
/// Functors may define arbitrary numbers of overloads to treat each type in a different way :
///
/// ```
/// struct MyFunctor
/// {
///
///     template<typename T>
///     string operator()( const TypedData<vector<T>> *data )
///     {
///         return "Dispatched VectorTypedData of some sort";
///     }
///
///     string operator()( const FloatData *data )
///     {
///         return "Dispatched FloatData";
///     }
///
///     ...
///
///     string operator()( const Data *data )
///     {
///         // Didn't expect to have to handle any other types.
///         throw exception( "Dispatched unknown type" );
///     }
///
/// };
/// ```
template<typename F>
typename std::result_of<F( Data * )>::type dispatch( Data *data, F &&functor );
template<typename F>
typename std::result_of<F( const Data * )>::type dispatch( const Data *data, F &&functor );

} // namespace IECore

#include "IECore/DataAlgo.inl"

#endif // IECORE_DATAALGO_H
