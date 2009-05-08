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

#ifndef IECORE_POLYGONALGO_INL
#define IECORE_POLYGONALGO_INL

#include "IECore/CircularIterator.h"

#include "OpenEXR/ImathVecAlgo.h"
#include "OpenEXR/ImathMath.h"

namespace IECore
{

template<typename Iterator>
typename std::iterator_traits<Iterator>::value_type polygonNormal( Iterator first, Iterator last )
{
	// Newell's method.
	typedef typename std::iterator_traits<Iterator>::value_type Vec;
	typedef CircularIterator<Iterator> CircularIt;

	Vec n( 0 );

	CircularIt v0It( first, last, first );
	CircularIt v1It( v0It ); v1It++;
	do
	{
		const Vec &v0 = *v0It;
		const Vec &v1 = *v1It;

		n.x += (v0.y - v1.y) * (v0.z + v1.z);
		n.y += (v0.z - v1.z) * (v0.x + v1.x);
		n.z += (v0.x - v1.x) * (v0.y + v1.y);

		v0It++;
		v1It++;
	}
	while( v0It != first );

	return n.normalized();
}

template<typename Iterator>
Winding polygonWinding( Iterator first, Iterator last )
{
	// 2d version.
	// calculate just the z coordinate of the normal using
	// newell's method.
	typedef typename std::iterator_traits<Iterator>::value_type Vec;
	typedef typename Vec::BaseType Real;
	typedef CircularIterator<Iterator> CircularIt;

	Real z( 0 );

	CircularIt v0It( first, last, first );
	CircularIt v1It( v0It ); v1It++;
	do
	{
		const Vec &v0 = *v0It;
		const Vec &v1 = *v1It;

		z += (v0.x - v1.x) * (v0.y + v1.y);

		v0It++;
		v1It++;
	}
	while( v0It != first );

	return z < Real( 0 ) ? ClockwiseWinding : CounterClockwiseWinding;
}

template<typename Iterator>
Winding polygonWinding( Iterator first, Iterator last, const typename std::iterator_traits<Iterator>::value_type &viewVector )
{
	typedef typename std::iterator_traits<Iterator>::value_type Vec;
	typedef typename Vec::BaseType Real;
	Real f = polygonNormal( first, last ).dot( viewVector );
	return f < Real( 0 ) ? CounterClockwiseWinding : ClockwiseWinding;
}

template<typename Iterator>
Imath::Box<typename std::iterator_traits<Iterator>::value_type> polygonBound( Iterator first, Iterator last )
{
	Imath::Box<typename std::iterator_traits<Iterator>::value_type> result;
	for( Iterator it=first; it!=last; it++ )
	{
		result.extendBy( *it );
	}
	return result;
}

} // namespace IECore

#endif // IECORE_POLYGONALGO_INL
