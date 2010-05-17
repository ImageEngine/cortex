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

#ifndef IECORERI_BROWNIAN_H
#define IECORERI_BROWNIAN_H

#include "IECoreRI/Noise.h"

// ieSBrownian

float ieSBrownian( point x; uniform float octaves; uniform float lacunarity; uniform float gain )
{
	float sum = 0;
	point xx = x;
	float fw = ieFilterWidth( x );
	float a = 1;
	uniform float i;
	uniform float aSum = 0;
	for( i=0; i<octaves; i+=1 )
	{
		sum += a * ieFilteredSNoise( xx, fw );
		aSum += a;
		a *= gain;
		xx *= lacunarity;
		fw *= lacunarity;
	}

	return sum / aSum; // try to stop different gains giving wildly different ranges of output.
}

color ieSBrownian( point x; uniform float octaves; uniform float lacunarity; uniform float gain )
{
	color sum = 0;
	point xx = x;
	float fw = ieFilterWidth( x );
	float a = 1;
	uniform float i;
	uniform float aSum = 0;
	for( i=0; i<octaves; i+=1 )
	{
		sum += a * (color ieFilteredSNoise( xx, fw ));
		aSum += a;
		a *= gain;
		xx *= lacunarity;
		fw *= lacunarity;
	}

	return sum / aSum; // try to stop different gains giving wildly different ranges of output.
}

// ieBrownian

float ieBrownian( point x; uniform float octaves; uniform float lacunarity; uniform float gain )
{
	return ieSBrownian( x, octaves, lacunarity, gain ) / 2.0 + 0.5;
}

color ieBrownian( point x; uniform float octaves; uniform float lacunarity; uniform float gain )
{
	return color ieSBrownian( x, octaves, lacunarity, gain ) / 2.0 + 0.5;
}

#endif // IECORERI_BROWNIAN_H
