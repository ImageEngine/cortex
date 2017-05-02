//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

#include "boost/format.hpp"

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
typename Spline<X,Y>::PointContainer::const_iterator Spline<X,Y>::lastValidSegment() const
{
	const int coefficientsNeeded = basis.numCoefficients();
	int validBasis = ( points.size() - ( coefficientsNeeded - basis.step ) ) / basis.step;
	int lastBasisOffset = ( validBasis - 1 ) * basis.step;

	typename PointContainer::const_iterator itEnd = points.end();
	for ( unsigned i = lastBasisOffset; i < points.size(); i++ )
		itEnd--;

	return itEnd;
}

template<typename X, typename Y>
typename Spline<X,Y>::XInterval Spline<X,Y>::interval() const
{
	const unsigned int coefficientsNeeded = basis.numCoefficients();
	if( points.size() < coefficientsNeeded )
	{
		return XInterval::empty();
	}
	else
	{
		typedef typename PointContainer::const_iterator It;
		X cc[4];
		X xp[4] = { X(0), X(0), X(0), X(0) };

		// collect first 4 control points
		It xIt = points.begin();
		for( unsigned i=0; i<coefficientsNeeded; i++, xIt++ )
		{
			xp[i] = xIt->first;
		}
		basis.coefficients( X( 0 ), cc );
		X xStart = cc[0]*xp[0]+cc[1]*xp[1]+cc[2]*xp[2]+cc[3]*xp[3];

		// collect last 4 control points (ignoring malformed basis)
		xIt = lastValidSegment();
		for( unsigned i=0; i<coefficientsNeeded; i++, xIt++ )
		{
			xp[i] = xIt->first;
		}
		basis.coefficients( X( 1 ), cc );
		X xEnd = cc[0]*xp[0]+cc[1]*xp[1]+cc[2]*xp[2]+cc[3]*xp[3];

		return XInterval( xStart, xEnd );
	}
}

template<typename X, typename Y>
inline X Spline<X,Y>::solve( X x, typename PointContainer::const_iterator &segment ) const
{
	const unsigned int coefficientsNeeded = basis.numCoefficients();
	
	size_t numPoints = points.size();
	if( numPoints < coefficientsNeeded )
	{
		throw( Exception( boost::str( boost::format( "Spline has less than %i points." ) % coefficientsNeeded ) ) );
	}
	if( (numPoints - coefficientsNeeded) % basis.step )
	{
		throw( Exception( "Spline has excess points (but not enough for an extra segment)." ) );
	}

	typedef typename PointContainer::const_iterator It;

	// find the first segment where seg( 0 ) > x. the segment before that is the one we're interested in.
	// this is just a linear search right now - it should be possible to optimise this using points.lower_bound
	// to quickly find a better start point for the search.
	X co[4];
	basis.coefficients( X( 0 ), co );
	X xp[4] = { X(0), X(0), X(0), X(0) };

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
		for( unsigned i=0; i<coefficientsNeeded; i++ )
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
	for( unsigned i=0; i<coefficientsNeeded; i++ )
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
	X t = solve( x, s );
	const unsigned int coefficientsNeeded = basis.numCoefficients();
	segment[0] = segment[1] = segment[2] = segment[3] = Y(0);
	for( unsigned i=0; i<coefficientsNeeded; i++ )
	{
		segment[i] = (*s++).second;
	}
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
inline Y Spline<X,Y>::derivative( X x ) const
{
	X xc[4] = { X(0), X(0), X(0), X(0) };
	Y yc[4] = { Y(0), Y(0), Y(0), Y(0) };
	typename PointContainer::const_iterator s;
	X t = solve( x, s );
	const unsigned int coefficientsNeeded = basis.numCoefficients();
	for( unsigned i=0; i<coefficientsNeeded; i++ )
	{
		xc[i] = (*s).first;
		yc[i] = (*s++).second;
	}
	return basis.derivative( t, yc ) / basis.derivative( t, xc );
}

template<typename X, typename Y>
inline Y Spline<X,Y>::integral( X t0, X t1, typename Spline<X,Y>::PointContainer::const_iterator segment ) const
{
	X xp[4] = { X(0), X(0), X(0), X(0) };
	Y yp[4] = { Y(0), Y(0), Y(0), Y(0) };

	const unsigned int coefficientsNeeded = basis.numCoefficients();
	for( unsigned i=0; i<coefficientsNeeded; i++ )
	{
		if( segment==points.end() )
		{
			throw( Exception( "Not enough control points to evaluate integral." ) );
		}
		xp[i] = segment->first;
		yp[i] = segment->second;
		segment++;
	}

	X cX[4];
	Y cY[4];

	cX[0] = basis.matrix[0][0]*xp[0] + basis.matrix[0][1]*xp[1] + basis.matrix[0][2]*xp[2] + basis.matrix[0][3]*xp[3];
	cX[1] = basis.matrix[1][0]*xp[0] + basis.matrix[1][1]*xp[1] + basis.matrix[1][2]*xp[2] + basis.matrix[1][3]*xp[3];
	cX[2] = basis.matrix[2][0]*xp[0] + basis.matrix[2][1]*xp[1] + basis.matrix[2][2]*xp[2] + basis.matrix[2][3]*xp[3];
	cX[3] = basis.matrix[3][0]*xp[0] + basis.matrix[3][1]*xp[1] + basis.matrix[3][2]*xp[2] + basis.matrix[3][3]*xp[3];

	cY[0] = basis.matrix[0][0]*yp[0] + basis.matrix[0][1]*yp[1] + basis.matrix[0][2]*yp[2] + basis.matrix[0][3]*yp[3];
	cY[1] = basis.matrix[1][0]*yp[0] + basis.matrix[1][1]*yp[1] + basis.matrix[1][2]*yp[2] + basis.matrix[1][3]*yp[3];
	cY[2] = basis.matrix[2][0]*yp[0] + basis.matrix[2][1]*yp[1] + basis.matrix[2][2]*yp[2] + basis.matrix[2][3]*yp[3];
	cY[3] = basis.matrix[3][0]*yp[0] + basis.matrix[3][1]*yp[1] + basis.matrix[3][2]*yp[2] + basis.matrix[3][3]*yp[3];

	// integral polynomial coefficients
	Y C1 = ( cY[0]*cX[0] ) / 2;
	Y C2 = ( cY[0]*cX[1]*2 + cY[1]*cX[0]*3 ) / 5;
	Y C3 = ( cY[0]*cX[2] + cY[1]*cX[1]*2 + cY[2]*cX[0]*3 ) / 4;
	Y C4 = ( cY[1]*cX[2] + cY[2]*cX[1]*2 + cY[3]*cX[0]*3 ) / 3;
	Y C5 = ( cY[2]*cX[2] + cY[3]*cX[1]*2 ) / 2;
	Y C6 = cY[3]*cX[2];

	// powers of t0 and t1
	X t02 = t0 * t0;
	X t03 = t02 * t0;
	X t04 = t02 * t02;
	X t05 = t04 * t0;
	X t06 = t03 * t03;
	X t12 = t1 * t1;
	X t13 = t12 * t1;
	X t14 = t12 * t12;
	X t15 = t14 * t1;
	X t16 = t13 * t13;

	return C1*(t16 - t06) + C2*(t15 - t05) + C3*(t14 - t04) + C4*(t13 - t03) + C5*(t12 - t02) + C6*(t1 - t0);
}

template<typename X, typename Y>
inline Y Spline<X,Y>::integral( X t0, typename Spline<X,Y>::PointContainer::const_iterator segment0, X t1, typename Spline<X,Y>::PointContainer::const_iterator segment1 ) const
{
	X initT = t0;
	X endT = 1;
	Y accum(0);
	typename PointContainer::const_iterator it = segment0;
	typename PointContainer::const_iterator itEnd = segment1;
	itEnd++;
	for ( it = segment0; it != itEnd; ) 
	{
		if ( it == segment1 )
		{
			endT = t1;
		}
		accum += integral( initT, endT, it );
		initT = 0;
		for ( unsigned i = 0; i < basis.step && it != itEnd; i++ )
		{
			it++;
		}
	}
	return accum;
}

template<typename X, typename Y>
inline Y Spline<X,Y>::integral( X x0, X x1 ) const
{
	typename PointContainer::const_iterator s0;
	X t0 = solve( x0, s0 );
	typename PointContainer::const_iterator s1;
	X t1 = solve( x1, s1 );
	return integral( t0, s0, t1, s1 );
}

template<typename X, typename Y>
inline Y Spline<X,Y>::integral() const
{
	const unsigned int coefficientsNeeded = basis.numCoefficients();
	if ( points.size() < coefficientsNeeded )
		return Y(0);

	return integral( X(0), points.begin(), X(1), lastValidSegment() );
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
