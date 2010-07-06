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

// Angles used for Marschner reflectance
varying float sinThetaO;
varying float sinThetaI;
varying float cosPhiD;

// Angles used for diffuse shading
varying float cosThetaI;
varying float cosHalfPhi;

// Marschner lookup texture inputs, as computed
// by the IECore.MarschnerLookupTableOp
uniform sampler2D lookupM;
uniform sampler2D lookupN;
uniform sampler2D lookupNTRT;

uniform float diffuseFalloff;
uniform float diffuseAzimuthFalloff;

uniform float scaleDiffuse;
uniform float scaleR;
uniform float scaleTT;
uniform float scaleTRT;

void main()
{	
	// NOTE: We need to 1 - the 't' coodinate for the texture lookup, as its the 
	// other way up here

	// Get MR, MTT, MTRT, cosThetaD from lookupM.
	vec2 lookupcoord1 = vec2( sinThetaI * 0.5 + 0.5, 1.0 - ( sinThetaO * 0.5 + 0.5 ) );
	vec4 scales = vec4( 30.0, 30.0, 30.0, 1.0 );
	vec4 comps1 = texture2D( lookupM, lookupcoord1 ) * scales;

	// Get NTT & NR from lookupN and NTRT from lookupNRT.
	// cosThetaD from Step 1 is already normalised.
	//                                                   cosThetaD
	vec2 lookupcoord2 = vec2( cosPhiD * 0.5 + 0.5, 1.0 - comps1.a );
	vec4 comps2 = texture2D( lookupN, lookupcoord2 );
	vec4 comps3 = texture2D( lookupNTRT, lookupcoord2 );
	
	//                       MR         NR                                     MTT         NTT                                      MTRT        NTRT
	vec3 marschner = vec3( comps1.x * comps2.a * scaleR ) + ( gl_Color.xyz * comps1.y * comps2.xyz * scaleTT ) + ( gl_Color.xyz * comps1.z * comps3.xyz * scaleTRT );
	vec3 diffuse = gl_Color.xyz * mix( 1.0, cosThetaI, diffuseFalloff ) * mix( 1.0, cosHalfPhi, diffuseAzimuthFalloff ); 

	gl_FragColor = vec4( marschner * gl_LightSource[0].specular.rgb + ( diffuse * scaleDiffuse * gl_LightSource[0].diffuse.rgb ), 1.0 );	
}
