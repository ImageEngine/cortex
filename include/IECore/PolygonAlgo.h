//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

//! \file PolygonAlgo.h
/// Defines algorithms for operating on polygons.
/// \ingroup mathGroup

#ifndef IECORE_POLYGONALGO_H
#define IECORE_POLYGONALGO_H

#include "OpenEXR/ImathBox.h"

namespace IECore
{

/// Returns the normalized normal for the polygon specified by the 3D vertices in the
/// given iterator range. Copes properly with concave polygons. Assumes a
/// righthanded (counter-clockwise) winding order, meaning that the normal will
/// face towards an observer who sees the loop from first to last as being
/// counter-clockwise.
template<typename Iterator>
typename std::iterator_traits<Iterator>::value_type polygonNormal( Iterator first, Iterator last );

/// As above, but only normalizes the normal if normalised==true.
template<typename Iterator>
typename std::iterator_traits<Iterator>::value_type polygonNormal( Iterator first, Iterator last, bool normalized );

/// An enum used to specify the winding order of
/// polygons.
enum Winding
{
	ClockwiseWinding = 0,
	CounterClockwiseWinding = 1
};

/// Returns the winding order for the polygon specified by the 2D vertices in the
/// given iterator range. Copes correctly with concave polygons.
template<typename Iterator>
Winding polygonWinding( Iterator first, Iterator last );

/// Returns the winding order for the polygon specified by the 3D vertices in the
/// given iterator range, when viewed with the specified view vector.
/// Copes correctly with concave polygons.
template<typename Iterator>
Winding polygonWinding( Iterator first, Iterator last, const typename std::iterator_traits<Iterator>::value_type &viewVector );

/// Returns the bounding box of the polygon specified by the vertices in the given iterator range.
template<typename Iterator>
Imath::Box<typename std::iterator_traits<Iterator>::value_type> polygonBound( Iterator first, Iterator last );

/// Returns the area of the polygon specified by the vertices in the given iterator range.
template<typename Iterator>
typename std::iterator_traits<Iterator>::value_type::BaseType polygonArea( Iterator first, Iterator last );

} // namespace IECore

#include "IECore/PolygonAlgo.inl"

#endif // IECORE_POLYGONALGO_H
