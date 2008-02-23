//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_TURBULENCE_INL
#define IE_CORE_TURBULENCE_INL

namespace IECore 
{

template<typename N>
Turbulence<N>::Turbulence( const unsigned int octaves, const Value &gain,
	const Point &lacunarity, bool turbulent, const N &noise )
	:	m_octaves( octaves ), m_gain( gain ), m_lacunarity( lacunarity ), m_turbulent( turbulent ), m_noise( noise )
{
	calculateScaleAndOffset();
}

template<typename N>
Turbulence<N>::Turbulence( const Turbulence &other )
	:	m_octaves( other.m_octaves ), m_gain( other.m_gain ), m_lacunarity( other.m_lacunarity ),
		m_turbulent( other.m_turbulent ), m_noise( other.m_noise )
{
}

template<typename N>
void Turbulence<N>::setOctaves( unsigned int octaves )
{
	m_octaves = octaves;
	calculateScaleAndOffset();
}

template<typename N>
unsigned int Turbulence<N>::getOctaves() const
{
	return m_octaves;
}

template<typename N>
void Turbulence<N>::setGain( const Value &gain )
{
	m_gain = gain;
	calculateScaleAndOffset();
}

template<typename N>
const typename Turbulence<N>::Value &Turbulence<N>::getGain() const
{
	return m_gain;
}

template<typename N>
void Turbulence<N>::setLacunarity( const Point &lacunarity )
{
	m_lacunarity = lacunarity;
}

template<typename N>
const typename Turbulence<N>::Point &Turbulence<N>::getLacunarity() const
{
	return m_lacunarity;
}

template<typename N>
void Turbulence<N>::setTurbulent( bool turbulent )
{
	m_turbulent = turbulent;
	calculateScaleAndOffset();
}

template<typename N>
bool Turbulence<N>::getTurbulent() const
{
	return m_turbulent;
}

template<typename N>
void Turbulence<N>::setNoise( const Noise &n )
{
	m_noise = n;
}

template<typename N>
const typename Turbulence<N>::Noise &Turbulence<N>::getNoise() const
{
	return m_noise;
}

template<typename N>
void Turbulence<N>::calculateScaleAndOffset()
{
	Value scale; vecSetAll( scale, 0.5 );
	Value result; vecSetAll( result, 0 );
	for( unsigned int i=0; i<m_octaves; i++ )
	{
		vecAdd( result, scale, result );
		vecMul( scale, m_gain, scale );
	}

	if( m_turbulent )
	{
		vecSetAll( m_scale, 1 );
		vecDiv( m_scale, result, m_scale );
		vecSetAll( m_offset, -0.5 );
	}
	else
	{
		vecSetAll( m_scale, 1 );
		vecDiv( m_scale, result, m_scale );
		vecDiv( m_scale, 2, m_scale );
		vecSetAll( m_offset, 0 );
	}
}
		
template<typename N>
typename Turbulence<N>::Value Turbulence<N>::turbulence( const Point &p ) const
{
	Value result; vecSetAll( result, 0 );
	Point frequency; vecSetAll( frequency, 1 );
	Value scale; vecSetAll( scale, 1 );
	for( unsigned int i=0; i<m_octaves; i++ )
	{
		Point pp; vecMul( p, frequency, pp );
		Value v = m_noise.noise( pp );
		vecMul( v, scale, v );
		if( m_turbulent )
		{
			for( unsigned int i=0; i<VectorTraits<Value>::dimensions(); i++ )
			{
				vecSet( v, i, Imath::Math<ValueBaseType>::fabs( vecGet( v, i ) ) );
			}
		}
		vecAdd( result, v, result );
		vecMul( scale, m_gain, scale );
		vecMul( frequency, m_lacunarity, frequency );
	}
	vecMul( result, m_scale, result );
	vecAdd( result, m_offset, result );
	return result;
}

} // namespace IECore

#endif // IE_CORE_TURBULENCE_INL
