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

#ifndef IECORE_TRIANGLEALGO_H
#define IECORE_TRIANGLEALGO_H

namespace IECore
{

template<class Vec>
typename Vec::BaseType triangleArea( const Vec &v0, const Vec &v1, const Vec &v2 );

template<class Vec>
Vec triangleNormal( const Vec &v0, const Vec &v1, const Vec &v2 );

template<class Vec>
Vec trianglePoint( const Vec &v0, const Vec &v1, const Vec &v2, const Imath::Vec3<typename Vec::BaseType> &barycentric );

/// Returns the squared-distance to the closest point on the triangle, and places that point's barycentric coordinates in the 4th argument.
template<class Vec>
typename Vec::BaseType triangleClosestBarycentric( const Vec &v0, const Vec &v1, const Vec &v2, const Vec &p, Imath::Vec3<typename Vec::BaseType> &barycentric );

/// Returns the closest point on the triangle, and places that point's barycentric coordinates in the 4th argument.
template<class Vec>
Vec triangleClosestPoint( const Vec &v0, const Vec &v1, const Vec &v2, const Vec &p, Imath::Vec3<typename Vec::BaseType> &barycentric );

/// Returns information regarding the closest feature on the triangle.
/// - 0 is the area within the triangle itself
/// - 1 is the edge connecting v1 and v2
/// - 2 is vertex v2
/// - 3 is the edge connecting v0 and v2
/// - 4 is vertex v0
/// - 5 is the edge connecting v0 and v1
/// - 6 is vertex v1
template<class Vec>
int triangleClosestFeature( const Vec &v0, const Vec &v1, const Vec &v2, const Vec &p );

} // namespace IECore

#include "IECore/TriangleAlgo.inl"

#endif // IECORE_TRIANGLEALGO_H
