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

#ifndef IECORE_LOOKUP_INL
#define IECORE_LOOKUP_INL

#include "OpenEXR/ImathFun.h"

#include "IECore/FastFloat.h"

namespace IECore
{

template<typename X, typename Y>
Lookup<X, Y>::Lookup()
	:	m_xMin( 0 ), m_xMax( 1 ), m_xMult( 1 )
{
	m_values.push_back( Y( 0 ) );
	m_values.push_back( Y( 1 ) );
}

template<typename X, typename Y>
template<class Function>
Lookup<X, Y>::Lookup( const Function &function, XType xMin, XType xMax, unsigned numSamples )
{
	init( function, xMin, xMax, numSamples );
}

template<typename X, typename Y>
template<class Function>
void Lookup<X, Y>::init( const Function &function, XType xMin, XType xMax, unsigned numSamples )
{
	m_values.resize( numSamples );
	X xStep = (xMax - xMin) / (numSamples -1 );
	X x = xMin;
	for( unsigned i=0; i<numSamples; i++ )
	{
		m_values[i] = function( x );
		x += xStep;
	}
	m_xMin = xMin;
	m_xMax = xMax;
	m_xMult = (numSamples-1) / (xMax - xMin);
}
		
template<typename X, typename Y>
inline Y Lookup<X, Y>::operator() ( X x ) const
{
	x = Imath::clamp( x, m_xMin, m_xMax );
	X f = (x - m_xMin) * m_xMult;
	int fi = fastFloatFloor( f );
	X ff = f - fi;
	return Imath::lerp( m_values[fi], m_values[fi+1], ff );
}
				
} // namespace IECore

#endif // IECORE_LOOKUP_INL
