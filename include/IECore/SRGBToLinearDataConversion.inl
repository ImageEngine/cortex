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

#ifndef IE_CORE_SRGBTOLINEARDATACONVERSION_INL
#define IE_CORE_SRGBTOLINEARDATACONVERSION_INL

#include "OpenEXR/ImathMath.h"

namespace IECore
{

/// See http://en.wikipedia.org/wiki/SRGB_color_space for implementation details	
template<typename F, typename T>
T SRGBToLinearDataConversion<F, T>::operator()( F f ) const
{
	const float k0 = 0.04045f;
	const float phi = 12.92f;

	if ( f <= k0 )
	{
		return T(f / phi);
	}
	else
	{
		const float alpha = 0.055;
		const float exponent = 2.4;

		float v = Imath::Math<float>::pow( ( f + alpha ) / ( 1.0f + alpha ), exponent );

		return T( v );
	}
}

template<typename F, typename T>
typename SRGBToLinearDataConversion<F, T>::InverseType SRGBToLinearDataConversion<F, T>::inverse() const
{
	return InverseType();
}

} // namespace IECore

#endif // IE_CORE_SRGBTOLINEARDATACONVERSION_INL
