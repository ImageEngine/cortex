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

#ifndef IECORE_LINESEGMENT_H
#define IECORE_LINESEGMENT_H

#include "IECore/VectorTraits.h"

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathPlane.h"

namespace IECore
{

/// The LineSegment class represents the portion of a line bounded
/// by two endpoints - this is in contrast to the Imath::Line class
/// which represents a line with infinite extent.
template<class T>
class LineSegment
{

	public :
	
		typedef T Point;
		typedef typename VectorTraits<T>::BaseType BaseType;
	
		/// Line endpoints can be accessed directly.
		T p0;
		T p1;
		
		/// Uninitialised.
		LineSegment();
		LineSegment( const T &P0, const T &P1 );

		/// Equality
		template<class S>
		bool operator==( const S &other ) const;
		template<class S>
		bool operator!=( const S &other ) const;
		
		/// Matrix multiplication. These functions
		/// transform both endpoints.
		template<class S>
		const LineSegment &operator *=( const S &m );
		template<class S>
		LineSegment operator *( const S &m ) const;

		/// Returns the point on the line at parameter t.
		/// t ranges from 0 at p0 to 1 at p1. 
		T operator() ( BaseType t ) const;

		/// p0 - p1
		T direction() const;
		/// (p0 - p1).normalized()
		T normalizedDirection() const;
		
		/// Distance between p0 and p1
		BaseType length() const;
		/// Distance squared between p0 and p1
		BaseType length2() const;

		T closestPointTo( const T &point ) const;
		/// Returns the point on this LineSeqment which is closest
		/// to line, and places the corresponding point on line in
		/// otherPoint.
		T closestPoints( const LineSegment &line, T &otherPoint ) const;
		
		BaseType distanceTo( const T &point ) const;
		/// Returns the shortest squared distance to the point.
		BaseType distance2To( const T &point ) const;
		BaseType distanceTo( const LineSegment &line ) const;
		/// Returns the shortest squared distance to the line.
		BaseType distance2To( const LineSegment &line ) const;
		
		template<class S>
		bool intersect( const Imath::Plane3<S> &plane, T &intersection ) const;
		template<class S>
		bool intersectT( const Imath::Plane3<S> &plane, BaseType &t ) const;
		
};

typedef LineSegment<Imath::V3f> LineSegment3f;
typedef LineSegment<Imath::V3d> LineSegment3d;
typedef LineSegment<Imath::V2f> LineSegment2f;
typedef LineSegment<Imath::V2d> LineSegment2d;

template<class T>
std::ostream &operator << ( std::ostream &o, const LineSegment<T> &lineSegment );

} // namespace IECore

#include "IECore/LineSegment.inl"

#endif // IECORE_LINESEGMENT_H
