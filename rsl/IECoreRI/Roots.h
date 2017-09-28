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

#ifndef IECORERI_ROOTS_H
#define IECORERI_ROOTS_H

// Solves a * x + b == 0
float ieSolveLinear( float a; float b; output float root )
{
	float rootCount = -1;
	if (a != 0)
	{
		root = -b / a;
		rootCount = 1;
	}
	else if (b != 0)
	{
		rootCount = 0;
	}
	return rootCount;
}

float ieCubicRoot( float v )
{
	return sign(v)*pow( abs(v), 1/3 );
}

float ieSolveQuadratic( float a; float b; float c; output float roots[] )
{
	uniform float epsilon = 1e-16;
	float rootCount = 0;
	if (abs(a) < epsilon)
	{
		rootCount = ieSolveLinear( b, c, roots[0] );
	}
	else
	{
		float D = b*b-4*a*c;

		if (abs(D) < epsilon)
		{
			roots[0] = -b/(2*a);
			rootCount = 1;
		}
		else if (D > 0)
		{
			float s = sqrt(D);
			roots[0] = (-b + s) / (2 * a);
			roots[1] = (-b - s) / (2 * a);
		    rootCount = 2;
		}
	}
	return rootCount;
}

// Computes real roots for a given cubic polynomial (x^3+Ax^2+Bx+C = 0).
// \todo: make sure it returns the same number of roots as in OpenEXR/ImathRoot.h
float ieSolveNormalizedCubic( float A; float B; float C; output float roots[] )
{
	uniform float epsilon = 1e-16;
	float rootCount = 0;
	if ( abs(C) < epsilon)
	{
		// We're solving x^3 + A x^2 + Bx = 0. That's got a root where x = 0,
		// and potentially two more where x^2 + A x + B = 0:

		// find quadratic roots, if they exist:
		rootCount = ieSolveQuadratic( 1, A, B, roots );

		// add x = 0 root:
		roots[ rootCount ] = 0;
		rootCount = rootCount + 1;
	}
	else
	{
		float Q = (3*B - A*A)/9;
		float R = (9*A*B - 27*C - 2*A*A*A)/54;
		float D = Q*Q*Q + R*R;	// polynomial discriminant

		if (D > 0) // complex or duplicate roots
		{
			float sqrtD = sqrt(D);
			float S = ieCubicRoot( R + sqrtD );
			float T = ieCubicRoot( R - sqrtD );
			roots[0] = (-A/3 + (S + T));   // one real root
			rootCount = 1;
		}
		else  // 3 real roots
		{
			float th = acos( R/sqrt(-(Q*Q*Q)) );
			float sqrtQ = sqrt(-Q);
			roots[0] = (2*sqrtQ*cos(th/3) - A/3);
			roots[1] = (2*sqrtQ*cos((th + 2*PI)/3) - A/3);
			roots[2] = (2*sqrtQ*cos((th + 4*PI)/3) - A/3);
			rootCount = 3;
		}
	}
	return rootCount;
}

float ieSolveCubic( float a; float b; float c; float d; output float roots[] )
{
	float epsilon = 1e-16;
	float rootCount;
	if (abs(a) < epsilon)
	{
		rootCount = ieSolveQuadratic (b, c, d, roots);
	}
	else
	{
		rootCount = ieSolveNormalizedCubic (b / a, c / a, d / a, roots);
	}
	return rootCount;
}


#endif // IECORERI_ROOTS_H
