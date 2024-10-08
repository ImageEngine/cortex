//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

//! \file BoxAlgo.h
/// Defines algorithms and operators which ideally would be already defined in ImathBox.h
/// \ingroup mathGroup

#ifndef IE_CORE_BOXALGO_H
#define IE_CORE_BOXALGO_H

#include "IECore/Export.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "Imath/ImathBox.h"
IECORE_POP_DEFAULT_VISIBILITY

#include <iostream>

namespace IECore
{

namespace BoxAlgo
{

/// Streaming for Imath::Box types
template<class T>
std::ostream &operator <<( std::ostream &os, const Imath::Box<T> &obj );

/// Closest point in box for 2D box types
template <class T>
Imath::Vec2<T> closestPointInBox(const Imath::Vec2<T>& p, const Imath::Box< Imath::Vec2<T> >& box );

/// Returns true if the box contains containee.
template <typename T>
bool contains( const Imath::Box<T> &box, const Imath::Box<T> &containee );

/// Splits the box into two across the specified axis.
template<typename T>
void split( const Imath::Box<T> &box, Imath::Box<T> &low, Imath::Box<T> &high, unsigned int axis );

/// Splits the box into two across the major axis.
template<typename T>
void split( const Imath::Box<T> &box, Imath::Box<T> &low, Imath::Box<T> &high );

} // namespace BoxAlgo

} // namespace IECore

#include "IECore/BoxAlgo.inl"

#endif // IE_CORE_BOXALGO_H
