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

// The shaded objects need to have their vTangents set as a vertex Primitive Variable
attribute vec3 vTangent;

// Angle outputs for Marschner reflectance
varying float sinThetaO;
varying float sinThetaI;
varying float cosPhiD;

// Angle outputs for diffuse term
varying float cosThetaI;
varying float cosHalfPhi;

void main()
{
	gl_FrontColor = gl_Color;
	gl_BackColor = gl_Color;
	gl_Position = ftransform();
	
	// Work out light, and eye vectors
	vec3 light = normalize( ( gl_ModelViewMatrixInverse * gl_LightSource[0].position ).xyz - gl_Vertex.xyz );
	vec3 cam = vec3( gl_ModelViewMatrixInverse * vec4(0,0,0,1.0) );
	vec3 eye = normalize( cam  - gl_Vertex.xyz );

	// http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter23.html
	
	sinThetaI = dot( light, vTangent );
	sinThetaO = dot( eye, vTangent );
	
	vec3 lightPerp = light - ( sinThetaI * vTangent );
	vec3 eyePerp = eye - ( sinThetaO * vTangent );
	
	float lenLightPerp = dot( lightPerp, lightPerp );
	float lenEyePerp = dot( eyePerp, eyePerp );
	
	cosPhiD = dot( eyePerp, lightPerp  ) * pow( lenEyePerp * lenLightPerp, -0.5 );
	
	// We need these for our two axis diffuse model
	cosThetaI = sqrt( 1.0 - sinThetaI * sinThetaI );
	cosHalfPhi = cos( acos(cosPhiD.x) * 0.5 );
}
