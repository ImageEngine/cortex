//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREGL_POINTSPRIMITIVE_H
#define IECOREGL_POINTSPRIMITIVE_H

#include "IECoreGL/MatrixAlgo.h"
#include "IECoreGL/VertexShader.h"

#define IECOREGL_POINTSPRIMITIVE_DECLAREVERTEXPARAMETERS \
	\
	IECOREGL_VERTEXSHADER_IN vec3 vertexP;\
	IECOREGL_VERTEXSHADER_IN float vertexwidth;\
	IECOREGL_VERTEXSHADER_IN float vertexpatchaspectratio;\
	IECOREGL_VERTEXSHADER_IN float vertexpatchrotation;\
	uniform bool useWidth;\
	uniform bool useAspectRatio;\
	uniform bool useRotation;\
	uniform float constantwidth;

#define IECOREGL_POINTSPRIMITIVE_INSTANCEMATRIX \
	iePointsPrimitiveInstanceMatrix(\
		vertexP,\
		useWidth ? vertexwidth * constantwidth : constantwidth,\
		useAspectRatio ? vertexpatchaspectratio : 1.0,\
		useRotation ? vertexpatchrotation : 0.0\
	)

mat4 iePointsPrimitiveInstanceMatrix( in vec3 P, in float width, in float aspectRatio, in float rotation )
{
	vec3 pCam = (gl_ModelViewMatrix * vec4( P, 1.0 )).xyz;

	vec3 Az;
	if( gl_ProjectionMatrix[2][3] != 0.0 )
	{
		// perspective
		Az = normalize( -pCam.xyz );
	}
	else
	{
		// orthographic
		Az = vec3( 0, 0, 1 );

	}

	vec3 up = vec3( sin( radians( rotation ) ), cos( radians( rotation ) ), 0 );

	vec3 Ax = normalize( cross( up, Az ) );
	vec3 Ay = normalize( cross( Az, Ax ) );

	mat4 placementMatrix = ieMatrixFromBasis( Ax * width, Ay * width / aspectRatio, Az * width, pCam );

	return placementMatrix;
}

#endif // IECOREGL_POINTSPRIMITIVE_H
