//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

//! \file BoxOps.h
/// Defines useful functions for dealing with any types which define an
/// appropriate BoxTraits struct. Although it's much prettier to use
/// the built in operators and members for a typical box implementation, it's not
/// particularly practical in much templated code as different box types
/// define different syntax or semantics for such operations. These functions
/// give less intuitive syntax but are compatible with any classes for which
/// a valid BoxTraits specialisation exists.

#ifndef IE_CORE_BOXOPS_H
#define IE_CORE_BOXOPS_H

#include <IECore/BoxTraits.h>
#include <IECore/VectorTraits.h>

namespace IECore
{

/// Returns a vector representing the length of each side of the box
template<typename T>
inline typename BoxTraits<T>::BaseType boxSize( const T &box );

/// Returns the center point of the box
template<typename T>
inline typename BoxTraits<T>::BaseType boxCenter( const T &box );

/// Extends the box by the given point
template<typename T>
inline void boxExtend( T &box, const typename BoxTraits<T>::BaseType &p );

/// Extends the box by the given box
template<typename T>
inline void boxExtend( T &box, const T &box2 );

/// Returns the intersection between two boxes, or the empty box if there is no intersection.
template <typename T>
T boxIntersection( const T &box, const T &box2 );

/// Returns true if the box intersects (contains) the given point
template <typename T>
bool boxIntersects( const T &box, const typename BoxTraits<T>::BaseType &p );

/// Returns true if the box intersects the given box
template <typename T>
bool boxIntersects( const T &box, const T &box2 );

/// Returns true if box contains containee.
template <typename T>
bool boxContains( const T &box, const T &containee );

/// Intersects the box with the given ray. The direction vector must be normalissed. Returns true if there was an intersection, setting
/// the "result" argument with the point of intersection accordingly.
template<typename T>
bool boxIntersects(
        const T &box,
        const typename BoxTraits<T>::BaseType &origin,
        const typename BoxTraits<T>::BaseType &direction,
        typename BoxTraits<T>::BaseType &result
);

} // namespace IECore

#include "IECore/BoxOps.inl"

#endif // IE_CORE_BOXOPS_H
