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
template<class S>
bool LineSegment<T>::operator==( const S &other ) const
{
	return p0==other.p0 && p1==other.p1;
}

template<class T>
template<class S>
bool LineSegment<T>::operator!=( const S &other ) const
{
	return p0!=other.p0 || p1!=other.p1;
}

template<class T>
template<class S>
const LineSegment<T> &LineSegment<T>::operator *=( const S &m )
{
	p0 *= m;
	p1 *= m;
	return *this;
}

template<class T>
template<class S>
LineSegment<T> LineSegment<T>::operator *( const S &m ) const
{
	LineSegment r( *this );
	r *= m;
	return r;
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
typename LineSegment<T>::BaseType LineSegment<T>::distance2To( const T &point ) const
{
	return (closestPointTo( point ) - point).length2();
}

template<class T>
typename LineSegment<T>::BaseType LineSegment<T>::distanceTo( const LineSegment &line ) const
{
	T a, b;
	a = closestPoints( line, b );
	return (a-b).length();
}

template<class T>
typename LineSegment<T>::BaseType LineSegment<T>::distance2To( const LineSegment &line ) const
{
	T a, b;
	a = closestPoints( line, b );
	return (a-b).length2();
}

template<class T>
template<class S>
bool LineSegment<T>::intersect( const Imath::Plane3<S> &plane, T &intersection ) const
{
	BaseType t;
	if( intersectT( plane, t ) )
	{
		intersection = (*this)( t );
		return true;
	}
	return false;
}

template<class T>
template<class S>
bool LineSegment<T>::intersectT( const Imath::Plane3<S> &plane, BaseType &t ) const
{
	T dir = direction();
	BaseType d = plane.normal.dot( dir );
	if( d==0.0 )
	{
		return false;
	}

	t = (plane.distance - plane.normal.dot( p0 )) / d;

	if( t >= 0 && t<=1 )
	{
		return true;
	}
	else
	{
		return false;
	}
}

template<class T>
std::ostream &operator << ( std::ostream &o, const LineSegment<T> &lineSegment )
{
    return o << "(" << lineSegment.p0 << ", " << lineSegment.p1 << ")";
}

} // namespace IECore

#endif // IECORE_LINESEGMENT_INL
