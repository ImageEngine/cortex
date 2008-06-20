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

#ifndef IECORE_RANDOM_INL
#define IECORE_RANDOM_INL

#include "OpenEXR/ImathMath.h"
#include "OpenEXR/ImathVec.h"

namespace IECore
{

template<class Vec, class Rand>
Vec barycentricRand( Rand &rand )
{
	Vec result;
	result[0] = rand.nextf( 0, 1 );
	result[1] = rand.nextf( 0, 1 );
	if( result[0] + result[1] > 1 )
	{
		result[0] = 1 - result[0];
		result[1] = 1 - result[1];
	}
	result[2] = 1 - result[0] - result[1];
	return result;
}

template<class Vec, class Rand>
Vec triangleRand( const Vec &v0, const Vec &v1, const Vec &v2, Rand &rand )
{
	Vec b = barycentricRand( rand );
	return v0 * b[0] + v1 * b[1] + v2 * b[2];
}

template<class Vec, class Rand>
Vec cosineHemisphereRand( Rand &rand )
{
	typedef typename Vec::BaseType BaseType;
	typedef Imath::Vec2<BaseType> V2;
	V2 d = Imath::solidSphereRand<V2>( rand );
	Vec result;
	result[0] = d[0];
	result[1] = d[1];
	result[2] = Imath::Math<BaseType>::sqrt( std::max( BaseType( 0 ), BaseType( 1 ) - d.x*d.x - d.y*d.y ) );
	return result;
}

} // namespace IECore

#endif // IECORE_RANDOM_INL
