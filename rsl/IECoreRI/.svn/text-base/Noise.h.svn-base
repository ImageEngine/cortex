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

#ifndef IECORERI_NOISE_H
#define IECORERI_NOISE_H

#include "IECoreRI/Filter.h"

// ieSNoise

float ieSNoise( float x )
{
	return 2 * ( float noise( x ) ) - 1;
}

float ieSNoise( float x; float y )
{
	return 2 * ( float noise( x, y ) ) - 1;
}

float ieSNoise( point x )
{
	return 2 * ( float noise( x ) ) - 1;
}

float ieSNoise( point x; float t )
{
	return 2 * ( float noise( x, t ) ) - 1;
}

color ieSNoise( float x )
{
	return 2 * ( color noise( x ) ) - 1;
}

color ieSNoise( float x; float y )
{
	return 2 * ( color noise( x, y ) ) - 1;
}

color ieSNoise( point x )
{
	return 2 * ( color noise( x ) ) - 1;
}

color ieSNoise( point x; float t )
{
	return 2 * ( color noise( x, t ) ) - 1;
}

// ieFilteredSNoise

float ieFilteredSNoise( float x; float filterWidth )
{
	return mix( ieSNoise( x ), 0, smoothstep( 0.2, 0.6, filterWidth ) );
}

float ieFilteredSNoise( float x )
{
	return ieFilteredSNoise( x, ieFilterWidth( x ) );
}

float ieFilteredSNoise( point x; float filterWidth )
{
	return mix( ieSNoise( x ), 0, smoothstep( 0.2, 0.6, filterWidth ) );
}

float ieFilteredSNoise( point x )
{
	return ieFilteredSNoise( x, ieFilterWidth( x ) );
}

color ieFilteredSNoise( float x; float filterWidth )
{
	return mix( color ieSNoise( x ), 0, smoothstep( 0.2, 0.6, filterWidth ) );
}

color ieFilteredSNoise( float x )
{
	return color ieFilteredSNoise( x, ieFilterWidth( x ) );
}

color ieFilteredSNoise( point x; float filterWidth )
{
	return mix( color ieSNoise( x ), 0, smoothstep( 0.2, 0.6, filterWidth ) );
}

color ieFilteredSNoise( point x )
{
	return color ieFilteredSNoise( x, ieFilterWidth( x ) );
}

#endif // IECORERI_NOISE_H
