//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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
//	     other contributors to this software may be used to endorse or
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

#include "IECore/Export.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathVec.h"
IECORE_POP_DEFAULT_VISIBILITY

#include "OpenEXR/ImathMatrixAlgo.h"

namespace IECore
{

// Partially specialize for Imath::Matrix44<T>. Interpolates through the shortest rotation path and linearly on the shear,translate and scale.
// If one of the matrices cannot be decomposed it raises an exception.
template<typename T>
struct LinearInterpolator< Imath::Matrix44<T> >
{
	void operator()(const Imath::Matrix44<T> &y0,
			const Imath::Matrix44<T> &y1,
			double x,
			Imath::Matrix44<T> &result) const
	{
		if ( x <= 0 )
		{
			result = y0;
			return;
		}

		if ( x >= 1 )
		{
			result = y1;
			return;
		}

		Imath::Vec3<T> s0, s1, sx;
		Imath::Vec3<T> h0, h1, hx;
		Imath::Vec3<T> r0, r1, rx;
		Imath::Vec3<T> t0, t1, tx;

		extractSHRT(y0, s0, h0, r0, t0);
		extractSHRT(y1, s1, h1, r1, t1);

		Imath::Quat<T> q0, q1, qx;
		q0 = Imath::Euler<T>(r0).toQuat();
		q1 = Imath::Euler<T>(r1).toQuat();

		LinearInterpolator< Imath::Vec3<T> >()( s0, s1, x, sx );
		LinearInterpolator< Imath::Vec3<T> >()( h0, h1, x, hx );
		LinearInterpolator< Imath::Quat<T> >()( q0, q1, x, qx );
		LinearInterpolator< Imath::Vec3<T> >()( t0, t1, x, tx );

		result.makeIdentity();
		result *= qx.toMatrix44();
		result.shear( hx );
		result.scale( sx );
		result[3][0] = tx.x;
		result[3][1] = tx.y;
		result[3][2] = tx.z;
	}
};

// Partially specialize for Imath::Matrix44<T>. Interpolates through the shortest rotation path and linearly on the shear,translate and scale.
// If one of the matrices cannot be decomposed it raises an exception.
template<typename T>
struct CubicInterpolator< Imath::Matrix44< T > >
{
	void operator()(const Imath::Matrix44< T > &y0,
			const Imath::Matrix44< T > &y1,
			const Imath::Matrix44< T > &y2,
			const Imath::Matrix44< T > &y3,
			double x,
			Imath::Matrix44< T > &result) const
	{

		if ( x <= 0 )
		{
			result = y1;
			return;
		}

		if ( x >= 1 )
		{
			result = y2;
			return;
		}

		Imath::Vec3<T> s0, s1, s2, s3, sx;
		Imath::Vec3<T> h0, h1, h2, h3, hx;
		Imath::Vec3<T> r0, r1, r2, r3, rx;
		Imath::Vec3<T> t0, t1, t2, t3, tx;

		extractSHRT(y0, s0, h0, r0, t0);
		extractSHRT(y1, s1, h1, r1, t1);
		extractSHRT(y2, s2, h2, r2, t2);
		extractSHRT(y3, s3, h3, r3, t3);

		Imath::Quat<T> q0, q1, q2, q3, qx;
		q0 = Imath::Euler<T>(r0).toQuat();
		q1 = Imath::Euler<T>(r1).toQuat();
		q2 = Imath::Euler<T>(r2).toQuat();
		q3 = Imath::Euler<T>(r3).toQuat();

		CubicInterpolator< Imath::Vec3<T> >()( s0, s1, s2, s3, x, sx );
		CubicInterpolator< Imath::Vec3<T> >()( h0, h1, h2, h3, x, hx );
		CubicInterpolator< Imath::Quat<T> >()( q0, q1, q2, q3, x, qx );
		CubicInterpolator< Imath::Vec3<T> >()( t0, t1, t2, t3, x, tx );

		result.makeIdentity();
		result *= qx.toMatrix44();
		result.shear( hx );
		result.scale( sx );
		result[3][0] = tx.x;
		result[3][1] = tx.y;
		result[3][2] = tx.z;
	}
};

} // namespace IECore

