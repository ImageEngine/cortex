//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_QUATALGO_H
#define IECORE_QUATALGO_H

#include "OpenEXR/ImathQuat.h"

namespace IECore
{

/// This is copied from the Imath corresponding to OpenEXR 1.6.1. It (and the other
/// copied functions below) can be removed when we're no longer building against 1.4.0.
template <class T>
inline T
sinx_over_x (T x)
{
    if (x * x < Imath::limits<T>::epsilon())
	return T (1);
    else
	return Imath::Math<T>::sin (x) / x;
}

/// This is copied from the Imath corresponding to OpenEXR 1.6.1. It (and the other
/// copied functions below) can be removed when we're no longer building against 1.4.0.
template<class T>
T
angle4D (const Imath::Quat<T> &q1, const Imath::Quat<T> &q2)
{
    //
    // Compute the angle between two quaternions,
    // interpreting the quaternions as 4D vectors.
    //

    Imath::Quat<T> d = q1 - q2;
    T lengthD = Imath::Math<T>::sqrt (d ^ d);

    Imath::Quat<T> s = q1 + q2;
    T lengthS = Imath::Math<T>::sqrt (s ^ s);

    return 2 * Imath::Math<T>::atan2 (lengthD, lengthS);
}

/// This is copied from the Imath corresponding to OpenEXR 1.6.1. It
/// is much more stable than the one in OpenEXR 1.4.0 so we take a copy of
/// the preferred one while we're still building with the old OpenEXR.
template<class T>
Imath::Quat<T>
slerp(const Imath::Quat<T> &q1,const Imath::Quat<T> &q2, T t)
{

 	//
    // Spherical linear interpolation.
    // Assumes q1 and q2 are normalized and that q1 != -q2.
    //
    // This method does *not* interpolate along the shortest
    // arc between q1 and q2.  If you desire interpolation
    // along the shortest arc, and q1^q2 is negative, then
    // consider flipping the second quaternion explicitly.
    //
    // The implementation of squad() depends on a slerp()
    // that interpolates as is, without the automatic
    // flipping.
    //
    // Don Hatch explains the method we use here on his
    // web page, The Right Way to Calculate Stuff, at
    // http://www.plunk.org/~hatch/rightway.php
    //

    T a = IECore::angle4D (q1, q2);
    T s = 1 - t;

    Imath::Quat<T> q = IECore::sinx_over_x (s * a) / IECore::sinx_over_x (a) * s * q1 +
	        IECore::sinx_over_x (t * a) / IECore::sinx_over_x (a) * t * q2;

    return q.normalized();
}

/// This is copied from revision 1.7 of IlmBase/Imath/ImathQuat.h in the
/// OpenEXR cvs repository. It's useful and it's not available in any of
/// the official OpenEXR releases yet.
template<class T>
Imath::Quat<T>
slerpShortestArc (const Imath::Quat<T> &q1, const Imath::Quat<T> &q2, T t)
{
    //
    // Spherical linear interpolation along the shortest
    // arc from q1 to either q2 or -q2, whichever is closer.
    // Assumes q1 and q2 are unit quaternions.
    //

    if ((q1 ^ q2) >= 0)
        return IECore::slerp (q1, q2, t);
    else
        return IECore::slerp (q1, -q2, t);
}

} // namespace IECore

#endif // IECORE_QUATALGO_H
