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

//! \file MatrixOps.h
/// Defines useful functions for dealing with any types which define an
/// appropriate MatrixTraits struct. Although it's much prettier to use
/// the built in operators for a typical matrix implementation, it's not
/// particularly practical in much templated code as different vector types
/// define different syntax or semantics for such operations. These functions
/// give less intuitive syntax but are compatible with any classes for which
/// a valid MatrixTraits specialisation exists.

#ifndef IE_CORE_MATRIXOPS_H
#define IE_CORE_MATRIXOPS_H

#include "IECore/MatrixTraits.h"

namespace IECore
{

/// Sets the specified component of m to the value x.
template<typename T>
inline void matSet( T &m, unsigned int i, unsigned int j, typename MatrixTraits<T>::BaseType x );

/// Sets all components of m to the value x.
template<typename T>
inline void matSetAll( T &m, typename MatrixTraits<T>::BaseType x );

/// Returns the value of the specified component of v.
template<typename T>
inline typename MatrixTraits<T>::BaseType matGet( const T &m, unsigned int i, unsigned int j );

/// Converts from one matrix type to another.
template<typename T, typename S>
inline S matConvert( const T &m );

/// Converts from one matrix type to another.
template<typename T, typename S>
inline void matConvert( const T &m1, S &m2 );

/// A functor suitable for use with stl algorithms such as transform(), allowing
/// the copying of a container of matrices of type T into a container of matrices of type S.
template<typename T, typename S>
struct MatConvert
{
	inline S operator()( const T &m ) const;
};

} // namespace IECore

#include "IECore/MatrixOps.inl"

#endif // IE_CORE_MATRIXOPS_H
