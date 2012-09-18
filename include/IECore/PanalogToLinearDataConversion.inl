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

#ifndef IE_CORE_PANALOGTOLINEARDATACONVERSION_INL
#define IE_CORE_PANALOGTOLINEARDATACONVERSION_INL

#include <iterator>
#include <algorithm>
#include <vector>
#include <cassert>

#include "OpenEXR/ImathLimits.h"
#include "OpenEXR/ImathMath.h"

namespace IECore
{

template<typename F, typename T>
PanalogToLinearDataConversion<F, T>::PanalogToLinearDataConversion()
{
	m_c1 = 0.066736f;
	m_c2 = 0.042784f;
	m_c3 = 4.971170f;
	m_c4 = 1.06674f;
}

template<typename F, typename T>
PanalogToLinearDataConversion<F, T>::PanalogToLinearDataConversion( float c1, float c2, float c3, float c4 )
{
	m_c1 = c1;
	m_c2 = c2;
	m_c3 = c3;
	m_c4 = c4;
}

template<typename F, typename T>
T PanalogToLinearDataConversion<F, T>::operator()( F f ) const
{
	return T(-m_c2 + m_c2 * Imath::Math<float>::exp(m_c3 * (f * m_c4 - m_c1)));
}

template<typename F, typename T>
typename PanalogToLinearDataConversion<F, T>::InverseType PanalogToLinearDataConversion<F, T>::inverse() const
{
	return InverseType();
}

} // namespace IECore

#endif // IE_CORE_PANALOGTOLINEARDATACONVERSION_INL
