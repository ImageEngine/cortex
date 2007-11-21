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

#ifndef IECORE_RANDOMROTATIONOP_H
#define IECORE_RANDOMROTATIONOP_H

#include "OpenEXR/ImathRandom.h"
#include "OpenEXR/ImathQuat.h"

namespace IECore
{

template<typename SeedIt, typename OutputIt>
void RandomRotationOp::generate(
	SeedIt seedBegin, SeedIt seedEnd,
	typename std::iterator_traits<OutputIt>::value_type::BaseType time,
	typename std::iterator_traits<OutputIt>::value_type::BaseType speedMin,
	typename std::iterator_traits<OutputIt>::value_type::BaseType speedMax,
	OutputIt result
)
{
	typedef typename std::iterator_traits<OutputIt>::value_type Vec;
	typedef typename Vec::BaseType BaseType;
	typedef Imath::Quat<BaseType> Quat;
	Imath::Rand32 r;

	Vec up( 0.0, 1.0, 0.0 );
	Quat rotator;
	for( SeedIt sIt=seedBegin; sIt!=seedEnd; sIt++ )
	{
		r.init( (unsigned long int)*sIt );
		Vec v = Imath::hollowSphereRand<Vec>( r ); // random axis to rotate around
		Quat toRotate( 0.0, v.cross( up ).normalized() ); // a point to rotate around the axis
		rotator.setAxisAngle( v, time * r.nextf( speedMin, speedMax ) + r.nextf( 0.0, M_PI * 2.0 ) );
		Quat rotated = rotator * toRotate * ~rotator;
		*result++ = rotated.v;
	}
}
		
} // namespace IECore

#endif // IECORE_RANDOMROTATIONOP_H
