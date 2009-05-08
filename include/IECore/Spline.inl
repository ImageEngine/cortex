//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_SPLINE_INL
#define IECORE_SPLINE_INL

#include "IECore/Exception.h"

#include "OpenEXR/ImathLimits.h"
#include "OpenEXR/ImathMath.h"

namespace IECore
{

template<typename X, typename Y>
Spline<X,Y>::Spline( const Basis &b )
	:	basis( b )
{
}

template<typename X, typename Y>
Spline<X,Y>::Spline( const Basis &b, const PointContainer &p )
	:	basis( b ), points( p )
{
}

template<typename X, typename Y>
typename Spline<X,Y>::XInterval Spline<X,Y>::interval() const
{
	if( !points.size() )
	{
		return XInterval::empty();
	}
	else
	{
		return XInterval( points.begin()->first, points.rbegin()->first );
	}
}

template<typename X, typename Y>
inline X Spline<X,Y>::solve( X x, typename PointContainer::const_iterator &segment ) const
{
	size_t numPoints = points.size();
	if( numPoints < 4 )
	{
		throw( Exception( "Spline has less than 4 points." ) );
	}
	if( (numPoints - 4) % basis.step )
	{
		throw( Exception( "Spline has excess points (but not enough for an extra segment)." ) );
	}

	typedef typename PointContainer::const_iterator It;

	// find the first segment where seg( 0 ) > x. the segment before that is the one we're interested in.
	// this is just a linear search right now - it should be possible to optimise this using points.lower_bound
	// to quickly find a better start point for the search.
	X co[4];
	basis.coefficients( X( 0 ), co );
	X xp[4];

	It testSegment = points.begin();
	do
	{
		segment = testSegment;
		for( unsigned i=0; i<basis.step; i++ )
		{
			testSegment++;
		}

		bool overrun = false;
		It xIt( testSegment );
		for( unsigned i=0; i<4; i++ )
		{
			if( xIt==points.end() )
			{
				overrun = true;
				break;
			}
			xp[i] = xIt->first;
			xIt++;
		}
		if( overrun )
		{
			break;
		}

	} while( xp[0] * co[0] + xp[1] * co[1] + xp[2] * co[2] + xp[3] * co[3] < x );
	// get the x values of the control values for the segment in question
	It xIt( segment );
	for( unsigned i=0; i<4; i++ )
	{
		xp[i] = (*xIt++).first;
	}
	// find the appropriate parametric position within that segment. we tried
	// doing this directly with ImathRoots.h but precision problems prevented this
	// working well. now we do a sort of bisection thing instead.
	X tMin = 0;
	X tMax = 1;
	X tMid, xMid;
	X epsilon = Imath::limits<X>::epsilon(); // might be nice to present this as a parameter
	do
	{
		tMid = ( tMin + tMax ) / X( 2 );
		xMid = basis( tMid, xp );
		if( xMid > x )
		{
			tMax = tMid;
		}
		else
		{
			tMin = tMid;
		}
	} while( Imath::Math<X>::fabs( tMin - tMax ) > epsilon );

	return tMid;
}

template<typename X, typename Y>
inline X Spline<X,Y>::solve( X x, Y segment[4] ) const
{
	typename PointContainer::const_iterator s;
	float t = solve( x, s );
	segment[0] = (*s++).second;
	segment[1] = (*s++).second;
	segment[2] = (*s++).second;
	segment[3] = (*s++).second;
	return t;
}

template<typename X, typename Y>
inline Y Spline<X,Y>::operator() ( X x ) const
{
	Y y[4];
	X t = solve( x, y );
	X c[4];
	basis.coefficients( t, c[0], c[1], c[2], c[3] );
	return c[0] * y[0] + c[1] * y[1] + c[2] * y[2] + c[3] * y[3];
}

template<typename X, typename Y>
inline bool Spline<X,Y>::operator==( const Spline &rhs ) const
{
	return basis==rhs.basis && points==rhs.points;
}

template<typename X, typename Y>
inline bool Spline<X,Y>::operator!=( const Spline &rhs ) const
{
	return basis!=rhs.basis || points!=rhs.points;
}

} // namespace IECore

#endif // IECORE_SPLINE_INL
