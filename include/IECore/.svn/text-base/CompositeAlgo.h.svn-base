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

//! \file CompositeAlgo.h
/// Defines fundamental compositing operations of two data values (A and B),
/// each with associated alpha. The values are expected to be premultiplied
/// by their corresponding alpha.


#ifndef IE_CORE_COMPOSITEALGO_H
#define IE_CORE_COMPOSITEALGO_H

namespace IECore
{

/// A over B
template<typename T>
inline T compositeOver( T aVal, T aAlpha, T bVal, T bAlpha );

/// max(A, B)
template<typename T>
inline T compositeMax( T aVal, T aAlpha, T bVal, T bAlpha );

/// min(A, B)
template<typename T>
inline T compositeMin( T aVal, T aAlpha, T bVal, T bAlpha );

/// A*B
template<typename T>
inline T compositeMultiply( T aVal, T aAlpha, T bVal, T bAlpha );

} // namespace IECore

#include "IECore/CompositeAlgo.inl"

#endif // IE_CORE_COMPOSITEALGO_H
