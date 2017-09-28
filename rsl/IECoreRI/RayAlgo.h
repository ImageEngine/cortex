//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORERI_RAYALGO_H
#define IECORERI_RAYALGO_H

/// Intersections between a ray and a sphere centred at the origin.
float ieRaySphereIntersection(
	point rayOrigin;
	vector rayDirection; // normalised
	float sphereRadius;
	float epsilon;
	output float t0;
	output float t1;
)
{

	float b = 2 * ((vector rayOrigin) . rayDirection);
	float c = ((vector rayOrigin) . (vector rayOrigin)) - sphereRadius*sphereRadius;
	float discrim = b*b - 4*c;
	float solutions;
	if( discrim > 0 )
	{
		discrim = sqrt( discrim );
		t0 = (-discrim - b) / 2;
		if( t0 > epsilon )
		{
			t1 = (discrim - b) / 2;
			solutions = 2;
		}
		else
		{
			t0 = (discrim - b) / 2;
			solutions = (t0 > epsilon) ? 1 : 0;
		}
	}
	else if( discrim == 0 )
	{
		t0 = -b/2;
		solutions = (t0 > epsilon) ? 1 : 0;
	}
	else
	{
		solutions = 0;
	}
	return solutions;

}

/// Intersections between a ray and an XY plane at the origin.
float ieRayPlaneIntersection(
	point rayOrigin;
	vector rayDirection; // normalised
	float epsilon;
	output float t0
)
{
	float solutions = 0;

	if( rayDirection[2] == 0.0 )
	{
		solutions = 0;
	}
	else
	{
		t0 = - rayOrigin[2] / rayDirection[2];
		solutions = t0 > epsilon ? 1 : 0;
	}

	return solutions;
}


/// Intersections between a ray and a cone on the negative Z axis, with its apex at the origin and the specified cone angle.
float ieRayConeIntersection(
	point rayOrigin;
	vector rayDirection;
	float coneAngle;
	float epsilon;
	output float t0;
	output float t1;
)
{

	// multiply the z coordinate by this factor to get the desired cone angle:
	float k = tan( coneAngle / 2 );
	k = k * k;

	// ok - we're working out an intersection with the double cone defined by x^2 + y^2 - k^2 z^2 = 0
	float c = rayOrigin[0] * rayOrigin[0] + rayOrigin[1] * rayOrigin[1] - k * rayOrigin[2] * rayOrigin[2];
	float b = 2 * ( rayOrigin[0] * rayDirection[0] + rayOrigin[1] * rayDirection[1] - k * rayOrigin[2] * rayDirection[2] );
	float a = rayDirection[0] * rayDirection[0] + rayDirection[1] * rayDirection[1] - k * rayDirection[2] * rayDirection[2];

	if( a == 0 )
	{
		// b t + c == 0
		t0 = - c / b;

		if( t0 > epsilon )
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	// a t * t + b t + c == 0
    	float discrim = b * b - 4 * a * c;

	float solutions = 0;

	if( discrim < 0 )
	{
		return 0;
	}

	discrim = sqrt( discrim );

	t0 = (-discrim - b) / ( 2 * a );
	t1 = ( discrim - b) / ( 2 * a );

	if( a < 0 )
	{
		// This means k^2 dz^2 > dx^2 + d^2 - ie the ray's gonna hit the -z cone and the +z cone.

		// Get the parameter value of the single valid cone hit - ie the one that hit the -z cone:
		float t = ( rayOrigin[2] + t1 * rayDirection[2] > 0 ) ? t0 : t1;

		if( c < 0  && rayOrigin[2] < 0 )
		{
			// x^2 + y^2 < k^2 z^2, and z > 0: ray origin is inside the -z cone:
			if( t > epsilon )
			{
				// started inside the cone, the intersection is ahead of us:
				t0 = t;
				return 1;
			}
			else
			{
				// started inside the cone, the intersection is behind us, so we aint hit nuffink:
				return 0;
			}
		}
		else
		{
			// x^2 + y^2 > k^2 z^2, or z > 0: ray origin is outside the -z cone:
			if( t > epsilon )
			{
				// started outside the cone, the intersection is ahead of us:
				t0 = t;
				return 1;
			}
			else
			{
				// started outside the cone, the intersection is behind us, so we aint hit nuffink:
				return 0;
			}
		}

		//return t1 > epsilon ? 2 : 0;

	}
	else
	{
		// this means k^2 dz^2 < dx^2 + d^2, so the two hits are both gonna be on the +z cone, or both on the -z cone:
		if( rayOrigin[2] + t1 * rayDirection[2] > 0 )
		{
			// ok - we've hit the positive cone. Never mind then:
			return 0;
		}
		else
		{
			// we've hit the negative cone! Probably.
			if( t1 > epsilon )
			{
				// foremost hit is valid:
				if( t0 > epsilon )
				{
					// other hit is valid too:
					return 2;
				}
				else
				{
					// only the foremost hit is valid:
					t0 = t1;
					return 1;
				}
			}
			else
			{
				return 0;
			}
		}
	}


}


#endif // IECORERI_RAYALGO_H
