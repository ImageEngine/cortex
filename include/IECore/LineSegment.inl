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

#ifndef IECORE_LINESEGMENT_INL
#define IECORE_LINESEGMENT_INL

#include "OpenEXR/ImathFun.h"

namespace IECore
{

template<class T>
LineSegment<T>::LineSegment()
{
}

template<class T>
LineSegment<T>::LineSegment( const T &P0, const T &P1 )
	:	p0( P0 ), p1( P1 )
{
}

template<class T>
T LineSegment<T>::operator() ( BaseType t ) const
{
	return Imath::lerp( p0, p1, t );
}

template<class T>
T LineSegment<T>::direction() const
{
	return p1 - p0;
}

template<class T>
T LineSegment<T>::normalizedDirection() const
{
	return direction().normalized();
}

template<class T>
typename LineSegment<T>::BaseType LineSegment<T>::length() const
{
	return (p1-p0).length();
}

template<class T>
typename LineSegment<T>::BaseType LineSegment<T>::length2() const
{
	return (p1-p0).length2();
}

template<class T>
T LineSegment<T>::closestPointTo( const T &point ) const
{
	T d = direction();
	BaseType l2 = d.length2();
	if( l2==0 )
	{
		return p0;
	}
	
	BaseType t = (point-p0).dot( d ) / l2;
	return (*this)( Imath::clamp( t, BaseType( 0 ), BaseType( 1 ) ) );
}

template<class T>
T LineSegment<T>::closestPoints( const LineSegment &line, T &otherPoint ) const
{
	// first find the closest points on the infinite lines
	T a = p1 - p0;
	T b = line.p1 - line.p0;
	T c = line.p0 - p0;
	
	T ab = a.cross( b );
	BaseType abL2 = ab.length2();
	if( abL2 < 1e-6 )
	{
		// pretty much parallel
		otherPoint = line.p0;
		return p0;
	}
	
	T cb = c.cross( b );
	T ca = c.cross( a );
	T ba = -ab;
	
	BaseType s = cb.dot( ab ) / abL2;
	BaseType t = - ca.dot( ba ) / abL2;
	
	// clamp onto the segments
	s = Imath::clamp( s, BaseType( 0 ), BaseType( 1 ) );
	t = Imath::clamp( t, BaseType( 0 ), BaseType( 1 ) );
		
	otherPoint = line( t );
	return (*this)( s );
}

template<class T>
typename LineSegment<T>::BaseType LineSegment<T>::distanceTo( const T &point ) const
{
	return (closestPointTo( point ) - point).length();
}

template<class T>
typename LineSegment<T>::BaseType LineSegment<T>::distanceTo( const LineSegment &line ) const
{
	T a, b;
	a = closestPoints( line, b );
	return (a-b).length();
}

} // namespace IECore

#endif // IECORE_LINESEGMENT_INL
