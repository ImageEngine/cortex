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

//! \file VectorOps.h
/// Defines useful functions for dealing with any types which define an
/// appropriate VectorTraits struct. Although it's much prettier to use
/// the built in operators for a typical vector implementation, it's not
/// particularly practical in much templated code as different vector types
/// define different syntax or semantics for such operations. These functions
/// give less intuitive syntax but are compatible with any classes for which
/// a valid VectorTraits specialisation exists.

#ifndef IE_CORE_VECTOROPS_H
#define IE_CORE_VECTOROPS_H

#include <IECore/VectorTraits.h>

namespace IECore
{

/// Sets the ith component of v to the value x.
template<typename T>
inline void vecSet( T &v, unsigned int i, typename VectorTraits<T>::BaseType x );

/// Sets all components of v to the value x.
template<typename T>
inline void vecSetAll( T &v, typename VectorTraits<T>::BaseType x );

/// Returns the value of the ith component of v.
template<typename T>
inline typename VectorTraits<T>::BaseType vecGet( const T &v, unsigned int i );

/// Adds v1 to v2 returning a new vector.
template<typename T>
inline T vecAdd( const T &v1, const T &v2 );

/// Adds v1 to v2, placing the result in result. It is safe for result
/// to be the same as either v1 or v2 to peform addition in place.
template<typename T>
inline void vecAdd( const T &v1, const T &v2, T &result );

/// Subtracts v2 from v1, returning a new vector.
template<typename T>
inline T vecSub( const T &v1, const T &v2 );

/// Subtracts v2 from v1, placing the result in result. It is safe for
/// result to be the same as either v1 or v2 to perform subtraction in place.
template<typename T>
inline void vecSub( const T &v1, const T &v2, T &result );

/// Perform multiplication of v1 by scalar value v2, returning a new vector.
template<typename T>
inline T vecMul( const T& v1, typename VectorTraits<T>::BaseType v2);

/// Perform multiplication of v1 by scalar value v2, placing the result in result.
/// It is safe for result to be the same as v1 to perform multiplication in place.
template<typename T>
inline void vecMul( const T &v1, typename VectorTraits<T>::BaseType v2, T &result );

/// Multiplies v1 by v2, returning a new vector.
template<typename T>
inline T vecMul( const T &v1, const T &v2 );

/// Multiplies v1 by v2, placing the result in result. It is safe for result
/// to be the same as either v1 or v2 to perform multiplication in place.
template<typename T>
inline void vecMul( const T &v1, const T &v2, T &result);

/// Division by a scalar returning a new vector.
template<typename T>
inline T vecDiv( const T &v1, typename VectorTraits<T>::BaseType v2 );

/// Division by a scalar placing the result in result. It is safe
/// for result to be the same as v1 to perform division in place.
template<typename T>
inline void vecDiv( const T &v1, typename VectorTraits<T>::BaseType v2, T &result );

/// Component-wise division of v1 by v2 returning a new vector.
template<typename T>
inline T vecDiv( const T &v1, const T &v2 );

/// Component-wise division of v1 by v2 placing the result in result. It is safe
/// for result to be the same as either v1 or v2 to perform division in place.
template<typename T>
inline void vecDiv( const T &v1, const T &v2, T &result );

/// Returns the dot product of v1 and v2.
template<typename T>
inline typename VectorTraits<T>::BaseType vecDot( const T &v1, const T &v2 );

/// Returns the squared length of v.
template<typename T>
inline typename VectorTraits<T>::BaseType vecLength2( const T &v );

/// Returns the length of v.
template<typename T>
inline typename VectorTraits<T>::BaseType vecLength( const T &v );

/// Normalizes v in place. If the length of v is zero then has no effect.
template<typename T>
inline void vecNormalize( T &v );

/// Returns the distance squared between v1 and v2.
template<typename T>
inline typename VectorTraits<T>::BaseType vecDistance2( const T &v1, const T &v2 );

/// Returns the distance between v1 and v2.
template<typename T>
inline typename VectorTraits<T>::BaseType vecDistance( const T &v1, const T &v2 );

/// Converts from one vector type to another.
template<typename T, typename S>
inline S vecConvert( const T &v );

/// Converts from one vector type to another.
template<typename T, typename S>
inline void vecConvert( const T &v1, S &v2 );

/// A functor suitable for use with stl algorithms such as transform(), allowing
/// the copying of a container of vectors of type T into a container of vectors of type S.
template<typename T, typename S>
struct VecConvert
{
	inline S operator()( const T &v ) const;
};

/// Constructs a new vector and returns it. The components array must be at
/// least VectorTraits<T>::dimensions() long.
template<typename T>
inline T vecConstruct( const typename VectorTraits<T>::BaseType *components );

/// Returns the cross product of v1 and v2, which must be 3-dimensional vectors
template<typename T>
inline T vecCross( const T &v1, const T &v2 );

} // namespace IECore

#include "IECore/VectorOps.inl"

#endif // IE_CORE_VECTOROPS_H
