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

#ifndef IECORE_HENYEYGREENSTEIN_INL
#define IECORE_HENYEYGREENSTEIN_INL

#include "OpenEXR/ImathMath.h"

namespace IECore
{

template<typename Vec>
inline typename Vec::BaseType henyeyGreenstein( typename Vec::BaseType g, const Vec &incident, const Vec &outgoing )
{
	return henyeyGreensteinCT( g, incident.dot( outgoing ) );
}

template<typename T>
inline T henyeyGreenstein( T g, T theta )
{
	return henyeyGreensteinCT( g, Imath::Math<T>::cos( theta ) );
}

template<typename T>
inline T henyeyGreensteinCT( T g, T cosTheta )
{
	T g2 = g * g;
	T top = T( 1 ) - g2;
	T bottom = T( 1 ) + g2 - ( T( 2 ) * g * cosTheta );
	bottom = T( 4 * M_PI ) * Imath::Math<T>::pow( bottom, T( 1.5 ) );
	return top / bottom;
}

} // namespace IECore

#endif IECORE_HENYEYGREENSTEIN_INL
