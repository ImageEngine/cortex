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

#ifndef IECOREGL_HSVTORGB_H
#define IECOREGL_HSVTORGB_H

vec3 hsvToRGB( vec3 hsv )
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

#endif // IECOREGL_HSVTORGB_H
