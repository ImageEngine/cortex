//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORERI_TURBULENCE_H
#define IECORERI_TURBULENCE_H

#include "IECoreRI/Noise.h"

float ieTurbulence( point x; uniform float octaves; uniform float lacunarity; uniform float gain; varying float filterWidth )
{
	float sum = 0;
	point xx = x;
	float a = 1;
	float fw = filterWidth;
	uniform float i;
	uniform float aSum = 0;
	for( i=0; i<octaves; i+=1 )
	{
		sum += a * ieFilteredAbs( ieFilteredSNoise( xx, fw ), fw );
		aSum += a;
		a *= gain;
		xx *= lacunarity;
		fw *= lacunarity;
	}

	return sum / aSum;
}

color ieTurbulence( point x; uniform float octaves; uniform float lacunarity; uniform float gain; varying float filterWidth )
{
	color sum = 0;
	point xx = x;
	float a = 1;
	float fw = filterWidth;
	uniform float i;
	uniform float aSum = 0;
	color n;
	for( i=0; i<octaves; i+=1 )
	{
		n = color ieFilteredSNoise( xx, fw );
		n[0] = ieFilteredAbs( n[0], fw );
		n[1] = ieFilteredAbs( n[1], fw );
		n[2] = ieFilteredAbs( n[2], fw );
		sum += a * n;
		aSum += a;
		a *= gain;
		xx *= lacunarity;
		fw *= lacunarity;
	}

	return sum / aSum;
}

float ieTurbulence( point x; uniform float octaves; uniform float lacunarity; uniform float gain )
{
	return ieTurbulence( x, octaves, lacunarity, gain, ieFilterWidth(x) );
}

color ieTurbulence( point x; uniform float octaves; uniform float lacunarity; uniform float gain )
{
	return ieTurbulence( x, octaves, lacunarity, gain, ieFilterWidth(x) );
}


#endif // IECORERI_TURBULENCE_H
