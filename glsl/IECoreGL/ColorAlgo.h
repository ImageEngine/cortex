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

#ifndef IECOREGL_COLORALGO_H
#define IECOREGL_COLORALGO_H

float ieLuminance( vec3 color, vec3 weights )
{
	return dot( color, weights );
}

float ieLuminance( vec3 color )
{
	return ieLuminance( color, vec3( 0.212671, 0.715160, 0.072169 ) );
}

/// Saturation value of 1 return c, 0 returns a fully desaturated
/// color and > 1 returns a color with increased saturation.
vec3 ieAdjustSaturation( vec3 c, float saturation )
{
	float l = ieLuminance( c );
	return mix( vec3( l, l, l ), c, saturation );
}

vec3 ieHsvToRGB( vec3 hsv )
{
	if( hsv.b == 0.0 )
	{
		return hsv.ggg;
	}

	float h = hsv.r * 6.0;
	int i = int( floor( h ) );
	float f = h - float( i );
	float p = hsv.b * ( 1.0 - hsv.g );
	float q = hsv.b * ( 1.0 - hsv.g * f );
	float t = hsv.b * ( 1.0 - hsv.g * ( 1.0 - f ) );

	if( i==0 )
	{
		return vec3( hsv.b, t, p );
	}
	else if( i==1 )
	{
		return vec3( q, hsv.b, p );
	}
	else if( i==2 )
	{
		return vec3( p, hsv.b, t );
	}
	else if( i==3 )
	{
		return vec3( p, q, hsv.b );
	}
	else if( i==4 )
	{
		return vec3( t, p, hsv.b );
	}

	return vec3( hsv.b, p, q );
}

float ieLinToSRGB( float f )
{
	const float phi = 12.92;
	const float cutoff = 0.003130805;

	if( f <= cutoff )
	{
		return f * phi;
	}
	else
	{
		const float alpha = 0.055;
		const float exponent = 2.4;
		return ( 1.0 + alpha ) * pow( f, 1.0 / exponent ) - alpha;
	}
}

vec3 ieLinToSRGB( vec3 f )
{
	return vec3( ieLinToSRGB( f.x ), ieLinToSRGB( f.y ), ieLinToSRGB( f.z ) );
}

vec3 ieRgbToHSV( vec3 rgb )
{
	vec3 result;

	float minc = min( min( rgb.r, rgb.g ), rgb.b );
	float maxc = max( max( rgb.r, rgb.g ), rgb.b );

	result.b = maxc; // v

	float delta = maxc - minc;
	if( maxc != 0.0 )
	{
		result.g = delta/maxc; // s
	}
	else
	{
		result.g = 0.0;
		result.r = 0.0;
		return result;
	}

	if( delta==0.0 )
	{
		result.r = 0.0;
	}
	else
	{
		if( rgb.r == maxc )
		{
			result.r = (rgb.g - rgb.b) / delta;
		}
		else if( rgb.g == maxc )
		{
			result.r = 2.0 + (rgb.b - rgb.r) / delta;
		}
		else
		{
			result.r = 4.0 + (rgb.r - rgb.g) / delta;
		}
	}

	result.r /= 6.0; // we'll keep hue in the 0-1 range

	if( result.r < 0.0 )
	{
		result.r += 1.0;
	}

	return result;
}

#endif // IECOREGL_COLORALGO_H
