//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_SPECULAR_H
#define IECOREGL_SPECULAR_H

#define M_PI 3.1415926535897932384626433832795

vec3 ieSpecular( vec3 P, vec3 N, vec3 V, float roughness, vec3 lightColors[gl_MaxLights], vec3 lightDirs[gl_MaxLights], int nLights )
{
	float n = pow( roughness, -3.5 ) - 1;

	// Blinn normalization
	float normalization = ( n + 2.0 ) / ( 4.0 * M_PI * (2.0 - pow( 2.0, -n * 0.5 ) ) );

	// Combine the visibility term from the denominator of the microfacet BRDF with the normalization
	normalization /= dot( N, V );

	// We use Schlick's approximation to the Smith G masking/shadowing function
	float k = 0.2 * roughness * sqrt( 2.0 / M_PI );

	// Eye Component of G : The masking term
	float G1V = max( 0, dot( N, V ) ) / ( dot( N, V ) * ( 1 - k ) + k );




	vec3 result = vec3( 0 );
	for( int i=0 ; i<nLights; i++ )
	{
		vec3 L = normalize( lightDirs[i] );
		vec3 H = normalize( L + V );

		// Light Component of G : The shadowing term
		float G1L = max( 0, dot( N, L ) ) / ( dot( N, L ) * ( 1 - k ) + k );

		// Blinn microfacet distribution
		float D = pow( max( 0.0, dot( N, H ) ), n ) * normalization;
		result += lightColors[i] * D * G1L * G1V;
	}
	return result;
}

#endif // IECOREGL_SPECULAR_H
