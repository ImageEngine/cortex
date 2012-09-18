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

#ifndef IE_CORE_LINEARTOREC709DATACONVERSION_INL
#define IE_CORE_LINEARTOREC709DATACONVERSION_INL

#include <cassert>

#include "OpenEXR/ImathMath.h"

namespace IECore
{

template<typename F, typename T>
T LinearToRec709DataConversion<F, T>::operator()( F f ) const
{
	const float phi = 4.5f;
	const float cutoff = (float)(0.081f / 4.5f);

	if ( f <= cutoff )
	{
		return T(f * phi);
	}
	else
	{
		const float alpha = 0.099f;
		const float exponent = 1/.45;

		float v = ( 1.0f + alpha ) * Imath::Math<float>::pow(f, 1.0f / exponent ) - alpha;

		return T( v );
	}
}

template<typename F, typename T>
typename LinearToRec709DataConversion<F, T>::InverseType LinearToRec709DataConversion<F, T>::inverse() const
{
	return InverseType();
}

}

#endif // IE_CORE_LINEARTOREC709DATACONVERSION_INL
