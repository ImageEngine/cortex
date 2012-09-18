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

#ifndef IECORERI_COLORALGO_H
#define IECORERI_COLORALGO_H

float ieSRGBToLin( float sRGB )
{
	float k0 = 0.04045;
	float phi = 12.92;

	float result;
	if( sRGB <= k0 )
	{
		result = (sRGB/phi);
	}
	else
	{
		float alpha = 0.055;
		float exponent = 2.4;
		result = pow( ( sRGB + alpha ) / ( 1.0 + alpha ), exponent );
	}
	return result;
}

color ieSRGBToLin( color sRGB )
{
	color result;
	float i;
	for( i=0; i<3; i+=1 )
	{
		result[i] = ieSRGBToLin( sRGB[i] );
	}
	return result;
}

float ieLuminance( color c; color weights )
{
	return vector( c ).vector( weights );
}

float ieLuminance( color c )
{
	return ieLuminance( c, color( 0.212671, 0.715160, 0.072169 ) );
}

color ieAdjustSaturation( color c; float saturation )
{
	float l = ieLuminance( c );
	return mix( color( l ), c, saturation );
}

/// Arguments of 0 leave color unchanged.
color ieAdjustHSV( color c; float hue; float saturation; float value )
{
	color cc = ctransform( "rgb", "hsv", c );
	cc[0] += hue;
	cc[1] *= max( 0, 1 + saturation);
	cc[2] *= max( 0, 1 + value);
	cc = ctransform( "hsv", "rgb", cc );
	return clamp( cc, 0, 1 );
}

#endif // IECORERI_COLORALGO_H
