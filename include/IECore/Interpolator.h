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

#ifndef IE_CORE_INTERPOLATOR_H
#define IE_CORE_INTERPOLATOR_H

#include <cassert>
#include <vector>
#include <math.h>
#include <OpenEXR/ImathQuat.h>

#include <IECore/TypedData.h>

namespace IECore
{
/// A function object which performs linear interpolation

template<typename T>
struct LinearInterpolator;

template<typename T>
struct CosineInterpolator;

template<typename T>
struct CubicInterpolator;


template<typename T>
struct LinearInterpolator
{
		/// Interpolate between y0 and y1
		void operator()(const T &y0, const T &y1, double x, T &result ) const;

};

/// A function object which performs cosine interpolation
template<typename T>
struct CosineInterpolator
{
		/// Interpolate between y0 and y1
		void operator()(const T &y0, const T &y1, double x, T &result ) const;

};

/// A function object which performs cubic interpolation
template<typename T>
struct CubicInterpolator
{
		/// Interpolate between y1 and y2. Requires additional data points on either side.
		void operator()(const T &y0, const T &y1, const T &y2, T const &y3, double x, T &result ) const;

};

#include "Interpolator.inl"
#include "QuatInterpolator.inl"

} // namespace IECore

#endif // IE_CORE_INTERPOLATOR_H
