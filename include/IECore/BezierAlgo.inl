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

#ifndef IECORE_BEZIERALGO_INL
#define IECORE_BEZIERALGO_INL

#include "IECore/LineSegment.h"

#include "OpenEXR/ImathFun.h"

namespace IECore
{

namespace Detail
{

template<typename Vec, typename F>
void bezierSubdivideWalk( const Vec &v0, const Vec &v1, const Vec &v2, const Vec &v3, typename Vec::BaseType tolerance2, F &f )
{
	LineSegment<Vec> line( v0, v3 );
	if( line.distance2To( v1 ) < tolerance2 && line.distance2To( v2 ) < tolerance2 )
	{
		f( v0 );
	}
	else
	{
		Vec p01 = Imath::lerp( v0, v1, 0.5 );
		Vec p12 = Imath::lerp( v1, v2, 0.5 );
		Vec p23 = Imath::lerp( v2, v3, 0.5 );
		Vec p0112 = Imath::lerp( p01, p12, 0.5 );
		Vec p1223 = Imath::lerp( p12, p23, 0.5 );
		Vec p01121223 = Imath::lerp( p0112, p1223, 0.5 );
		bezierSubdivideWalk( v0, p01, p0112, p01121223, tolerance2, f );
		bezierSubdivideWalk( p01121223, p1223, p23, v3, tolerance2, f );	
	}
}

} // namespace Detail

template<typename Vec, typename F>
void bezierSubdivide( const Vec &v0, const Vec &v1, const Vec &v2, const Vec &v3, typename Vec::BaseType tolerance, F &f )
{
	typename Vec::BaseType t2 = tolerance * tolerance;
	Detail::bezierSubdivideWalk( v0, v1, v2, v3, t2, f );
	f( v3 );
}

} // namespace IECore

#endif // IECORE_BEZIERALGO_INL
