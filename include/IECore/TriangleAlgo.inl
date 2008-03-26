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

#ifndef IECORE_TRIANGLEALGO_INL
#define IECORE_TRIANGLEALGO_INL

#include <cassert>

#include "OpenEXR/ImathLimits.h"

#include "IECore/VectorOps.h"

namespace IECore
{

template<class Vec>
typename VectorTraits<Vec>::BaseType triangleArea( const Vec &v0, const Vec &v1, const Vec &v2 )
{
	return (v1-v0).cross(v2-v0).length() / 2;
}

template<class Vec>
Vec triangleNormal( const Vec &v0, const Vec &v1, const Vec &v2 )
{
	Vec n = (v2-v0).cross(v1-v0);	
	vecNormalize( n );
	return n;
}

template<class Vec>
Vec trianglePoint( const Vec &v0, const Vec &v1, const Vec &v2, const Imath::Vec3<typename VectorTraits<Vec>::BaseType> &barycentric )
{
	return vecAdd(
		vecAdd(
			vecMul( v0, barycentric[0] ),
			vecMul( v1, barycentric[1] )
		),
		vecMul( v2, barycentric[2] )
	);
}

/// Implementation derived from Wild Magic (Version 2) Software Library, available
/// from http://www.geometrictools.com/Downloads/WildMagic2p5.zip under free license
template<class Vec>
typename VectorTraits<Vec>::BaseType triangleClosestBarycentric( const Vec &v0, const Vec &v1, const Vec &v2, const Vec &p, Imath::Vec3<typename VectorTraits<Vec>::BaseType> &barycentric )
{
	typedef typename VectorTraits<Vec>::BaseType Real;

	Vec triOrigin = v0;
	Vec triEdge0, triEdge1;
	
	vecSub( v1, v0, triEdge0 );
	vecSub( v2, v0, triEdge1 );
	
	Vec kDiff;
	vecSub( triOrigin, p, kDiff );	

	Real a00 = vecDot( triEdge0, triEdge0 );
	Real a01 = vecDot( triEdge0, triEdge1 );
	Real a11 = vecDot( triEdge1, triEdge1 );
	Real b0 = vecDot( kDiff, triEdge0 );
	Real b1 = vecDot( kDiff, triEdge1 );
	Real c = vecDot( kDiff, kDiff );
	Real det = Real(fabs(a00*a11-a01*a01));
	Real s = a01*b1-a11*b0;
	Real t = a01*b0-a00*b1;
	Real distSqrd;

	if ( s + t <= det )
	{
		if ( s < Real(0.0) )
		{
			if ( t < Real(0.0) )  // region 4
			{
				if ( b0 < Real(0.0) )
				{
					t = Real(0.0);
					if ( -b0 >= a00 )
					{
						s = Real(1.0);
						distSqrd = a00+Real(2.0)*b0+c;
					}
					else
					{
						s = -b0/a00;
						distSqrd = b0*s+c;
					}
				}
				else
				{
					s = Real(0.0);
					if ( b1 >= Real(0.0) )
					{
						t = Real(0.0);
						distSqrd = c;
					}
					else if ( -b1 >= a11 )
					{
						t = Real(1.0);
						distSqrd = a11+Real(2.0)*b1+c;
					}
					else
					{
						t = -b1/a11;
						distSqrd = b1*t+c;
					}
				}
			}
			else  // region 3
			{
				s = Real(0.0);
				if ( b1 >= Real(0.0) )
				{
					t = Real(0.0);
					distSqrd = c;
				}
				else if ( -b1 >= a11 )
				{
					t = Real(1.0);
					distSqrd = a11+Real(2.0)*b1+c;
				}
				else
				{
					t = -b1/a11;
					distSqrd = b1*t+c;
				}
			}
		}
		else if ( t < Real(0.0) )  // region 5
		{
			t = Real(0.0);
			if ( b0 >= Real(0.0) )
			{
				s = Real(0.0);
				distSqrd = c;
			}
			else if ( -b0 >= a00 )
			{
				s = Real(1.0);
				distSqrd = a00+Real(2.0)*b0+c;
			}
			else
			{
				s = -b0/a00;
				distSqrd = b0*s+c;
			}
		}
		else  // region 0
		{
			// minimum at interior point
			if ( det == Real(0.0) )
			{
				s = Real(0.0);
				t = Real(0.0);
				distSqrd = Imath::limits<Real>::max();
			}
			else
			{
				Real invDet = Real(1.0)/det;
				s *= invDet;
				t *= invDet;
				distSqrd = s*(a00*s+a01*t+Real(2.0)*b0) +
				           t*(a01*s+a11*t+Real(2.0)*b1)+c;
			}
		}
	}
	else
	{
		Real tmp0, tmp1, numer, denom;

		if ( s < Real(0.0) )  // region 2
		{
			tmp0 = a01 + b0;
			tmp1 = a11 + b1;
			if ( tmp1 > tmp0 )
			{
				numer = tmp1 - tmp0;
				denom = a00-Real(2.0)*a01+a11;
				if ( numer >= denom )
				{
					s = Real(1.0);
					t = Real(0.0);
					distSqrd = a00+Real(2.0)*b0+c;
				}
				else
				{
					s = numer/denom;
					t = Real(1.0) - s;
					distSqrd = s*(a00*s+a01*t+Real(2.0)*b0) +
					           t*(a01*s+a11*t+Real(2.0)*b1)+c;
				}
			}
			else
			{
				s = Real(0.0);
				if ( tmp1 <= Real(0.0) )
				{
					t = Real(1.0);
					distSqrd = a11+Real(2.0)*b1+c;
				}
				else if ( b1 >= Real(0.0) )
				{
					t = Real(0.0);
					distSqrd = c;
				}
				else
				{
					t = -b1/a11;
					distSqrd = b1*t+c;
				}
			}
		}
		else if ( t < Real(0.0) )  // region 6
		{
			tmp0 = a01 + b1;
			tmp1 = a00 + b0;
			if ( tmp1 > tmp0 )
			{
				numer = tmp1 - tmp0;
				denom = a00-Real(2.0)*a01+a11;
				if ( numer >= denom )
				{
					t = Real(1.0);
					s = Real(0.0);
					distSqrd = a11+Real(2.0)*b1+c;
				}
				else
				{
					t = numer/denom;
					s = Real(1.0) - t;
					distSqrd = s*(a00*s+a01*t+Real(2.0)*b0) +
					           t*(a01*s+a11*t+Real(2.0)*b1)+c;
				}
			}
			else
			{
				t = Real(0.0);
				if ( tmp1 <= Real(0.0) )
				{
					s = Real(1.0);
					distSqrd = a00+Real(2.0)*b0+c;
				}
				else if ( b0 >= Real(0.0) )
				{
					s = Real(0.0);
					distSqrd = c;
				}
				else
				{
					s = -b0/a00;
					distSqrd = b0*s+c;
				}
			}
		}
		else  // region 1
		{
			numer = a11 + b1 - a01 - b0;
			if ( numer <= Real(0.0) )
			{
				s = Real(0.0);
				t = Real(1.0);
				distSqrd = a11+Real(2.0)*b1+c;
			}
			else
			{
				denom = a00-Real(2.0)*a01+a11;
				if ( numer >= denom )
				{
					s = Real(1.0);
					t = Real(0.0);
					distSqrd = a00+Real(2.0)*b0+c;
				}
				else
				{
					s = numer/denom;
					t = Real(1.0) - s;
					distSqrd = s*(a00*s+a01*t+Real(2.0)*b0) +
					           t*(a01*s+a11*t+Real(2.0)*b1)+c;
				}
			}
		}
	}

	barycentric.y = s;
	barycentric.z = t;
	barycentric.x = 1 - s - t;

	return Real(fabs(distSqrd));
}

template<class Vec>
Vec triangleClosestPoint( const Vec &v0, const Vec &v1, const Vec &v2, const Vec &p, Imath::Vec3<typename VectorTraits<Vec>::BaseType> &barycentric )
{
	triangleClosestBarycentric( v0, v1, v2, p, barycentric );
	return trianglePoint( v0, v1, v2, barycentric );
}

template<class Vec>
int triangleBarycentricFeature( const Vec &barycentric, typename VectorTraits<Vec>::BaseType tolerance )
{
	typedef typename VectorTraits<Vec>::BaseType Real;
	
	bool bx = VectorTraits<Vec>::get( barycentric, 0 ) > tolerance;
	bool by = VectorTraits<Vec>::get( barycentric, 1 ) > tolerance;
	bool bz = VectorTraits<Vec>::get( barycentric, 2 ) > tolerance;
	
	if (bx && by && bz)
	{
		return 0;
	}
	else if (bx && by)
	{
		return 5;
	}
	else if (bx && bz)
	{
		return 3;
	}
	else if (by && bz)
	{
		return 1;
	}
	else if (bx)
	{
		return 4;
	}
	else if (by)
	{
		return 6;
	}
	else
	{
		assert(bz);
		return 2;
	}	
}

template<class Vec>
int triangleClosestFeature( const Vec &v0, const Vec &v1, const Vec &v2, const Vec &p )
{
	typedef typename VectorTraits<Vec>::BaseType Real;
	
	Imath::Vec3<Real> barycentric;	
	triangleClosestPoint( v0, v1, v2, p, barycentric);
	
	return triangleBarycentricFeature( barycentric );	
}


/// Derived from article at http://www.blackpawn.com/texts/pointinpoly/default.html
template<class Vec>
bool triangleContainsPoint( const Vec &v0, const Vec &v1, const Vec &v2, const Vec &p )
{
	typedef typename VectorTraits<Vec>::BaseType Real;
			
	Vec a = v2 - v0;
	Vec b = v1 - v0;
	Vec c = p - v0;

	Real dotAA = a.dot( a );
	Real dotAB = a.dot( b );
	Real dotAC = a.dot( c );
	Real dotBB = b.dot( b );
	Real dotBC = b.dot( c );

	Real d = Real( 1 ) / ( dotAA * dotBB - dotAB * dotAB );
	
	Real u = (dotBB * dotAC - dotAB * dotBC) * d;
	if( u < Real( 0 ) || u > Real( 1 ) )
	{
		return false;
	}

	Real v = (dotAA * dotBC - dotAB * dotAC) * d;
	if( v < Real( 0 ) || u + v > Real( 1 ) )
	{
		return false;
	} 
	
	return true;
}

} // namespace IECore

#endif // IECORE_TRIANGLEALGO_INL
