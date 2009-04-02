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

#ifndef IE_CORE_CINEONTOLINEARDATACONVERSION_INL
#define IE_CORE_CINEONTOLINEARDATACONVERSION_INL

#include <iterator>
#include <algorithm>
#include <vector>
#include <cassert>

#include "OpenEXR/ImathLimits.h"
#include "OpenEXR/ImathMath.h"

namespace IECore
{

template<typename F, typename T>
CineonToLinearDataConversion<F, T>::CineonToLinearDataConversion() : m_LUT( 1024 )
{
	m_filmGamma = 0.6f;
	m_refWhiteVal = 685;
	m_refBlackVal = 95;
		
	m_LUTValid = false;
}

template<typename F, typename T>
CineonToLinearDataConversion<F, T>::CineonToLinearDataConversion( float filmGamma, int refWhiteVal, int refBlackVal ) : m_LUT( 1024 )
{
	m_filmGamma = filmGamma;
	m_refWhiteVal = refWhiteVal;
	m_refBlackVal = refBlackVal;	
	
	m_LUTValid = false;
}

template<typename F, typename T>
T CineonToLinearDataConversion<F, T>::operator()( F f ) const
{
	std::vector<float>::size_type i = static_cast<std::vector<float>::size_type>( f );

	assert( i >= 0 );
	assert( i < 1024 );

	return lookupTable()[ i ];
}

template<typename F, typename T>
const std::vector<float> &CineonToLinearDataConversion<F, T>::lookupTable() const
{
	if ( ! m_LUTValid )
	{	
		float refMult = 0.002f / m_filmGamma;
		float blackOffset = Imath::Math<float>::pow( 10.0f, ( m_refBlackVal - m_refWhiteVal ) * refMult );	
		
		assert( m_LUT.size() == 1024 );
		for ( unsigned i = 0; i < 1024; ++i )
		{
			m_LUT[i] = ( Imath::Math<float>::pow( 10.0f, ( (float)i - m_refWhiteVal ) * refMult ) - blackOffset ) / ( 1.0f - blackOffset );
		}
		m_LUTValid = true;	
	}

	assert( m_LUT.size() == 1024 );
	return m_LUT;
}	

template<typename F, typename T>
typename CineonToLinearDataConversion<F, T>::InverseType CineonToLinearDataConversion<F, T>::inverse() const
{
	return InverseType( m_filmGamma, m_refWhiteVal, m_refBlackVal );
}

} // namespace IECore

#endif // IE_CORE_CINEONTOLINEARDATACONVERSION_INL
