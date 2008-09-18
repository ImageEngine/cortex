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

#ifndef IECOREGL_LIGHT_H
#define IECOREGL_LIGHT_H

/// \todo All the functions in the glsl source need the ie prefix on them.
vec3 light( vec3 p, int lightIndex, out vec3 L )
{
	vec3 Cl = gl_LightSource[lightIndex].diffuse.rgb;
	
	if( gl_LightSource[lightIndex].position.w==0.0 )
	{
		// directional light
		L = normalize( gl_LightSource[lightIndex].position.xyz );
	}
	else
	{
		// pointlight or spotlight
		
		L = gl_LightSource[lightIndex].position.xyz - p;
		float d = length( L );
		vec3 Ln = L/d;
		
		float falloff = 1.0 /
			(	gl_LightSource[lightIndex].constantAttenuation +
				gl_LightSource[lightIndex].linearAttenuation * d +
				gl_LightSource[lightIndex].quadraticAttenuation * d * d );
		
		if( gl_LightSource[lightIndex].spotCutoff!=180.0 )
		{
			// spotlight
			float cosA = dot( -Ln, normalize( gl_LightSource[lightIndex].spotDirection.xyz ) );
			if( cosA < gl_LightSource[lightIndex].spotCosCutoff )
			{
				falloff = 0.0;
			}
			else
			{
				falloff *= pow( cosA, gl_LightSource[lightIndex].spotExponent );
			}
		}
		
		Cl *= falloff;
	}
	return Cl;
}


#endif // IECOREGL_LIGHT_H
