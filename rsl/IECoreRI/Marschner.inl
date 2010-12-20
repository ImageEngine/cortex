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

#include "IECoreRI/Gaussian.h"
#include "IECoreRI/Roots.h"

// converts a given refraction index ( eta ) to work on a 2d plane that is a cross section of the hair.
// the theta parameter is the angle from the incident light to the cross section plane.
float ieBravaisIndex( varying float theta; varying float eta )
{
	float sinTheta = sin( theta );
	float result = sqrt( eta*eta - sinTheta*sinTheta ) / cos( theta );
	return result;
}

// Computes reflectance fresnel with different index (eta) of refractions for perpendicular and parallel polarized light.
// Assumes the source media is vaccuum ( n = 1 ). If invert is non-zero, then assumes the target media is vaccuum.
float ieMarschnerFresnel( varying float incidenceAngle; varying float etaPerp; varying float etaParal; uniform float invert )
{
	float n1, n2;
	float rPerp = 1;
	float rParal = 1;

	float angle = abs(incidenceAngle);
	if ( angle > PI/2 )
	{
		angle = PI - angle;
	}

	if ( invert )
	{
		n1 = etaPerp;
		n2 = 1;
	}
	else
	{
		n1 = 1;
		n2 = etaPerp;
	}

	// Perpendicular light reflectance
	float a = (n1/n2)*sin(angle);
	a *= a;
	if ( a <= 1 )
	{
		float b = n2*sqrt(1-a);
		float c = n1*cos(angle);
		rPerp =  ( c - b ) / ( c + b );
		rPerp *= rPerp;
		rPerp = min( 1, rPerp );
	}
	if ( invert )
	{
		n1 = etaParal;
		n2 = 1;
	}
	else
	{
		n1 = 1;
		n2 = etaParal;
	}
	// Parallel light reflectance
	float d = (n1/n2)*sin(angle);
	d *= d;
	if ( d <= 1 )
	{
		float e = n1*sqrt(1-d);
		float f = n2*cos(angle);
		rParal = ( e - f ) / ( e + f );
		rParal *= rParal;
		rParal = min( 1, rParal );
	}
	return 0.5 * (rPerp + rParal);
}

// computes a new refraction index based on the hair eccentricity and the azimuth distance.
float ieMarschnerEccentricityRefraction( varying float eccentricity; varying float refraction; varying float averageAzimuth )
{
	float n1 = 2 * ( refraction - 1 ) * eccentricity * eccentricity - refraction + 2;
	float n2 = 2 * ( refraction - 1 ) / ( eccentricity * eccentricity ) - refraction + 2;
	return ( (n1 + n2) + cos( 2 * averageAzimuth ) * ( n1 - n2 ) ) / 2;
}

float ieMarschnerExitAnglePolynomial( uniform float p; float eta; float h )
{
	// use polynomial that approximates original equation.
	uniform float pi3 = PI*PI*PI;
	float gamma = asin(h);
	float c = asin(1/eta);
	return (6*p*c/PI - 2)*gamma-8*(p*c/pi3)*gamma*gamma*gamma + p*PI;
}

float ieMarschnerDExitAnglePolynomial( uniform float p; varying float eta; varying float h )
{
	// computes the derivative of the polynomial relative to h.
	float gamma = asin( h );
	uniform float pi3 = PI*PI*PI;
	float c = asin(1/eta);
	float dGamma = (6*p*c/PI-2) - 3*8*(p*c/pi3)*gamma*gamma;
	float denom = sqrt(1-h*h);
	return dGamma/max(1e-5,denom);
}

float ieMarschnerDDExitAnglePolynomial( uniform float p; varying float eta; varying float h )
{
	// computes the second derivative of the polynomial relative to h.
	float gamma = asin( h );
	uniform float pi3 = PI*PI*PI;
	float c = asin(1/eta);
	float dGamma = -2*3*8*(p*c/pi3)*gamma;
	float denom = pow( 1-h*h, 3/2 );
	return (dGamma*h)/max(1e-5,denom);
}

color ieMarschnerA( varying color absorption; varying vector lightVec; uniform float p; varying float gammaI; varying float refraction; varying float etaPerp; varying float etaParal )
{
	if ( p == 0 )
	{
		return color ieMarschnerFresnel( gammaI, etaPerp, etaParal, 0 );
	}

	float h = sin( gammaI );			// from [1] right before equation 3.
	float gammaT = asin( clamp(h / etaPerp,-1,1) );	// from [1] right before equation 3.
	float thetaT = acos( (etaPerp/refraction)*cos( lightVec[1] ) );	// definition for equation 20 in [2].
	float cosTheta = cos(thetaT);
	float l;
	// equation 20 in [2]
	l = 2*cos(gammaT)/max(1e-5,cosTheta);
	color segmentAbsorption = color( exp( -absorption[0]*l*p ), exp( -absorption[1]*l*p ), exp( -absorption[2]*l*p ) );

	float fresnel;
	// equations 24-28 in [2]
	float invFresnel = ieMarschnerFresnel( gammaT, etaPerp, etaParal, 1 );
	fresnel = ( 1 - ieMarschnerFresnel( gammaI, etaPerp, etaParal, 0 ) )*( 1 - invFresnel );
	if ( p > 1 )
	{
		fresnel = fresnel * invFresnel;
	}
	return fresnel * segmentAbsorption;
}

float ieMarschnerTargetAngle( uniform float p; varying float relativeAzimuth )
{
	float targetAngle = abs(relativeAzimuth);

	// set right range to match polynomial representation of the real curve.
	if ( p != 1 )
	{
		// converts angles to range [-PI,PI]
		if ( targetAngle > PI )
			targetAngle -= 2*PI;

		// offset center
		targetAngle += p*PI;
	}
	return targetAngle;
}

float ieMarschnerRoots( uniform float p; varying float eta; varying float targetAngle; output varying float roots[] )
{
	float rootCount;
	// Computes the roots of: o(p,y) - targetAngle = 0
	// by using the polynomial approximation: o(p,y) = (6pc / PI - 2)y - 8(pc/PI^3)y^3 + pPI where c = asin( 1/eta )^-1
	uniform float pi3 = PI*PI*PI;
	float c = asin(1/eta);
	rootCount = ieSolveCubic( -8*(p*c/pi3), 0, (6*p*c / PI - 2), (p*PI-targetAngle), roots );
	return rootCount;
}

color ieMarschnerNP( varying color absorption; varying vector lightVec; uniform float p; varying float refraction; varying float etaPerp; varying float etaParal; varying float targetAngle )
{
	float roots[3] = { 0,0,0 };
	float rootCount = ieMarschnerRoots( p, etaPerp, targetAngle, roots );

	color result = 0;
	uniform float denomMin = 1e-5;
	float rootIndex;
	for ( rootIndex = 0; rootIndex < rootCount; rootIndex+=1 )
	{
		float gammaI = roots[rootIndex];
		if ( abs(gammaI) <= PI/2 )
		{
			float h = sin( gammaI );
			color finalAbsorption = ieMarschnerA( absorption, lightVec, p, gammaI, refraction, etaPerp, etaParal );
			float dexitAngle;
			dexitAngle = ieMarschnerDExitAnglePolynomial( p, etaPerp, h );
			float denom = max( denomMin, 2*abs( dexitAngle ) );
			result += (finalAbsorption / denom);
		}
	}
	return result;
}

color ieMarschnerNTRT( varying color absorption; varying vector lightVec; varying float refraction; varying float etaPerp; varying float etaParal; varying float targetAngle; uniform float causticLimit; uniform float causticWidth; uniform float glintScale; uniform float causticFade )
{
	float dH, t, hc, Oc1, Oc2;
	if ( etaPerp < 2 )
	{
		float ddexitAngle;
		// compute roots of the polynomials derivative
		float c = asin(1/etaPerp);
		uniform float pi3 = PI*PI*PI;
		float gammac = sqrt( (6*2*c/PI-2)/(3*8*(2*c/pi3)) );
		hc = abs(sin(gammac));
		ddexitAngle = ieMarschnerDDExitAnglePolynomial( 2, etaPerp, hc );
		dH = min( causticLimit, 2*sqrt( 2*causticWidth/abs( ddexitAngle ) ) );
		t = 1;
	}
	else
	{
		hc = 0;
		dH = causticLimit;
		t = 1 - smoothstep( 2, 2 + causticFade, etaPerp );
	}

	Oc1 = ieMarschnerExitAnglePolynomial( 2, etaPerp, hc );
	Oc2 = ieMarschnerExitAnglePolynomial( 2, etaPerp, -hc );

	uniform float a, b, c;
	ieGaussianPDF( 0, causticWidth, a, b, c );
	uniform float causticCenter = ieGaussian( a, b, c, 0 );
	float causticLeft = ieGaussian( a, b, c, targetAngle - Oc1 );
	float causticRight = ieGaussian( a, b, c, targetAngle - Oc2 );
	color glintAbsorption = ieMarschnerA( absorption, lightVec, 2, asin(hc), refraction, etaPerp, etaParal );

	color result = ieMarschnerNP( absorption, lightVec, 2, refraction, etaPerp, etaParal, targetAngle );
	result *= 1 - t*causticLeft/causticCenter;
	result *= 1 - t*causticRight/causticCenter;
	result += glintAbsorption*t*glintScale*dH*(causticLeft + causticRight);
	return result;
}

float ieMarschnerM( uniform float shift; uniform float width; uniform float normWidth; varying float x )
{
	uniform float a, b, c;
	uniform float norm = 1/( radians(normWidth) * sqrt( 2 * PI ) );
	ieGaussianPDF( shift, width, a, b, c );
	return (ieGaussian( a, b, c, x ) / a) * norm;
}
