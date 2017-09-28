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

#ifndef IECOREGL_CURVESPRIMITIVE_H
#define IECOREGL_CURVESPRIMITIVE_H

#include "IECoreGL/VecAlgo.h"

#define IECOREGL_CURVESPRIMITIVE_DECLARE_VERTEX_PASS_THROUGH_PARAMETERS \
	in vec3 geometryCs[];\
	\
	out vec3 fragmentCs;

#define IECOREGL_CURVESPRIMITIVE_DECLARE_CUBIC_LINES_PARAMETERS \
	\
	layout( lines_adjacency ) in;\
	layout( line_strip, max_vertices = 10 ) out;\
	\
	uniform mat4x4 basis;\
	\
	IECOREGL_CURVESPRIMITIVE_DECLARE_VERTEX_PASS_THROUGH_PARAMETERS

#define IECOREGL_CURVESPRIMITIVE_DECLARE_LINEAR_RIBBONS_PARAMETERS \
	\
	layout( lines_adjacency ) in;\
	layout( triangle_strip, max_vertices = 4 ) out;\
	\
	uniform float width;\
	\
	IECOREGL_CURVESPRIMITIVE_DECLARE_VERTEX_PASS_THROUGH_PARAMETERS

#define IECOREGL_CURVESPRIMITIVE_DECLARE_CUBIC_RIBBONS_PARAMETERS \
	\
	layout( lines_adjacency ) in;\
	layout( triangle_strip, max_vertices = 20 ) out;\
	\
	uniform mat4x4 basis;\
	uniform float width;\
	\
	IECOREGL_CURVESPRIMITIVE_DECLARE_VERTEX_PASS_THROUGH_PARAMETERS

#define IECOREGL_CURVESPRIMITIVE_COEFFICIENTS( t, c0, c1, c2, c3 ) \
	ieCurvesPrimitiveCoefficients(\
		basis, t, c0, c1, c2, c3\
	)

#define IECOREGL_CURVESPRIMITIVE_POSITION( t )\
	ieCurvesPrimitivePosition( basis, t )

#define IECOREGL_CURVESPRIMITIVE_LINEARFRAME( i, p, normal, uTangent, vTangent ) \
	ieCurvesPrimitiveLinearFrame( i, p, normal, uTangent, vTangent )

#define IECOREGL_CURVESPRIMITIVE_CUBICFRAME( t, p, normal, uTangent, vTangent ) \
	ieCurvesPrimitiveCubicFrame( basis, t, p, normal, uTangent, vTangent )

# define IECOREGL_ASSIGN_VERTEX_PASS_THROUGH \
	fragmentCs = geometryCs[1];

void ieCurvesPrimitiveCoefficients( in mat4x4 basis, in float t, out float c0, out float c1, out float c2, out float c3 )
{
	float t2 = t * t;
	float t3 = t2 * t;

	c0 = basis[0][0] * t3 + basis[1][0] * t2 + basis[2][0] * t + basis[3][0];
	c1 = basis[0][1] * t3 + basis[1][1] * t2 + basis[2][1] * t + basis[3][1];
	c2 = basis[0][2] * t3 + basis[1][2] * t2 + basis[2][2] * t + basis[3][2];
	c3 = basis[0][3] * t3 + basis[1][3] * t2 + basis[2][3] * t + basis[3][3];
}

// As above but also computes d0-d3, the coefficients for computing tangents.
void ieCurvesPrimitiveCoefficients(
	in mat4x4 basis, in float t,
	out float c0, out float c1, out float c2, out float c3,
	out float d0, out float d1, out float d2, out float d3
)
{
	float t2 = t * t;
	float t3 = t2 * t;

	c0 = basis[0][0] * t3 + basis[1][0] * t2 + basis[2][0] * t + basis[3][0];
	c1 = basis[0][1] * t3 + basis[1][1] * t2 + basis[2][1] * t + basis[3][1];
	c2 = basis[0][2] * t3 + basis[1][2] * t2 + basis[2][2] * t + basis[3][2];
	c3 = basis[0][3] * t3 + basis[1][3] * t2 + basis[2][3] * t + basis[3][3];

	float twoT = 2.0 * t;
	float threeT2 = 3.0 * t2;

	d0 = basis[0][0] * threeT2 + basis[1][0] * twoT + basis[2][0];
	d1 = basis[0][1] * threeT2 + basis[1][1] * twoT + basis[2][1];
	d2 = basis[0][2] * threeT2 + basis[1][2] * twoT + basis[2][2];
	d3 = basis[0][3] * threeT2 + basis[1][3] * twoT + basis[2][3];
}

vec4 ieCurvesPrimitivePosition( in float c0, in float c1, in float c2, in float c3 )
{
	return

		gl_in[0].gl_Position * c0 +
		gl_in[1].gl_Position * c1 +
		gl_in[2].gl_Position * c2 +
		gl_in[3].gl_Position * c3;
}

vec4 ieCurvesPrimitivePosition( in mat4x4 basis, in float t )
{
	float c0, c1, c2, c3;
	ieCurvesPrimitiveCoefficients( basis, t, c0, c1, c2, c3 );
	return ieCurvesPrimitivePosition( c0, c1, c2, c3 );
}

void ieCurvesPrimitiveUTangentAndNormal( in vec4 p, in vec4 vTangent, out vec4 uTangent, out vec4 normal )
{
	vec3 view;
	if( gl_ProjectionMatrix[2][3] != 0.0 )
	{
		view = normalize( -p.xyz );
	}
	else
	{
		view = vec3( 0, 0, 1 );
	}

	uTangent = normalize( vec4( cross( view.xyz, vTangent.xyz ), 0 ) );
	normal = vec4( cross( uTangent.xyz, vTangent.xyz ), 0 );
}

void ieCurvesPrimitiveLinearFrame(
	in int i,
	out vec4 p, out vec4 n,
	out vec4 uTangent, out vec4 vTangent
)
{
	vec4 vBefore = ieNormalize( gl_in[i+1].gl_Position - gl_in[i].gl_Position );
	vec4 vAfter = ieNormalize( gl_in[i+2].gl_Position - gl_in[i+1].gl_Position );
	vTangent = normalize( vBefore + vAfter );

	ieCurvesPrimitiveUTangentAndNormal( gl_in[i+1].gl_Position, vTangent, uTangent, n );

	p = gl_in[i+1].gl_Position;

	float sinTheta = dot( uTangent, vBefore );
	float cosTheta = sqrt( 1.0 - sinTheta * sinTheta );
	uTangent /= cosTheta;
}

void ieCurvesPrimitiveCubicFrame(
	in mat4x4 basis, in float t,
	out vec4 p, out vec4 n,
	out vec4 uTangent, out vec4 vTangent
)
{
	float c0, c1, c2, c3, d0, d1, d2, d3;
	ieCurvesPrimitiveCoefficients( basis, t, c0, c1, c2, c3, d0, d1, d2, d3 );

	p =

		gl_in[0].gl_Position * c0 +
		gl_in[1].gl_Position * c1 +
		gl_in[2].gl_Position * c2 +
		gl_in[3].gl_Position * c3;

	vTangent =

		gl_in[0].gl_Position * d0 +
		gl_in[1].gl_Position * d1 +
		gl_in[2].gl_Position * d2 +
		gl_in[3].gl_Position * d3;

	vTangent = normalize( vTangent );

	ieCurvesPrimitiveUTangentAndNormal( p, vTangent, uTangent, n );
}

#endif // IECOREGL_CURVESPRIMITIVE_H
