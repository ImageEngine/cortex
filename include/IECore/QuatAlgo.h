//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2011, Image Engine Design Inc. All rights reserved.
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

//! \file QuatAlgo.h
/// Defines algorithms for quaternions.
/// \ingroup mathGroup

#ifndef IECORE_QUATALGO_H
#define IECORE_QUATALGO_H

#include "IECore/Export.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "Imath/ImathQuat.h"
#include "Imath/ImathMath.h"
IECORE_POP_DEFAULT_VISIBILITY

namespace IECore
{

/// We copied these imath functions into our code before they were released.  Now that we're
/// using OpenEXR 2, we no longer need them.  I've left these as aliases until we make sure
/// we remove any use of them in our namespace
template <class T>
inline T
sinx_over_x (T x)
{
	return Imath::sinx_over_x( x );
}

template<class T>
T
angle4D (const Imath::Quat<T> &q1, const Imath::Quat<T> &q2)
{
	return Imath::angle4D( q1, q2 );
}

template<class T>
Imath::Quat<T>
slerp(const Imath::Quat<T> &q1,const Imath::Quat<T> &q2, T t)
{
    return Imath::slerp( q1, q2, t );
}

template<class T>
Imath::Quat<T>
slerpShortestArc (const Imath::Quat<T> &q1, const Imath::Quat<T> &q2, T t)
{
    return Imath::slerpShortestArc( q1, q2, t );
}

} // namespace IECore

#endif // IECORE_QUATALGO_H
