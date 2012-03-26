//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_LINEARTOALEXALOGCDATACONVERSION_INL
#define IE_CORE_LINEARTOALEXALOGCDATACONVERSION_INL

#include <cassert>

#include "OpenEXR/ImathMath.h"

namespace IECore
{

template<typename F, typename T>
T LinearToAlexaLogcDataConversion<F, T>::operator()( F f ) const
{
	const float kCut = 0.010591f;
	const float kA = 5.555556f;
	const float kB = 0.052272f;
	const float kC = 0.247190f;
	const float kD = 0.385537f;
	const float kE = 5.367655f;
	const float kF = 0.092809f;
	
	if ( f <= kCut )
	{
		return T( kE*f + kF );
	}
	else
	{
		float v = kC * Imath::Math<float>::log10( kA*f + kB ) + kD;
		return T( v );
	}
}

template<typename F, typename T>
typename LinearToAlexaLogcDataConversion<F, T>::InverseType LinearToAlexaLogcDataConversion<F, T>::inverse() const
{
	return InverseType();
}

}

#endif // IE_CORE_LINEARTOALEXALOGCDATACONVERSION_INL
