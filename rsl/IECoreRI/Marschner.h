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

#ifndef IECORERI_MARSCHNER_H
#define IECORERI_MARSCHNER_H

#include "IECoreRI/Marschner.inl"

// This function implements marschner's BCSDF according to:
// [1] "Light Scattering from Human Hair Fibers" by Marschner et al. 2003 and corrections suggested in
// [2] "Light Scattering from Filaments" by Arno Zinke and Andreas Weber
// I've done two additional changes from the original algorithm:
// a) The gaussian used for the M coefficients have a constant peak so the width parameter does not affect intensity. One can easily scale the resulting R,TT,TRT to account for intensity.
// b) The derivatives computed on the algorithm and the hc positions for Ntrt are computed from the approximating polynomial and not from the original equation. That guarantees that the caustic happens exactly were hc points to.
// It returns the sum of the three components R+TT+TRT but it also returns them individually for further customizations.
// The returned values are already multiplied by the cosine of the incidence angle on the cross section plane of the hair according to equation 1 in [1].
// The parameters for this function are the same as in table 1 at [1] but angles are in radians. 'eye' and 'light' vectors are spherical coordinates that should be computed by ieMarschnerLocalVector().
// Lucio Moser - October 2009
color ieMarschner( varying vector eye; varying vector lightVec; uniform float refraction; 
						varying color absorption; uniform float eccentricity;
						uniform float shiftR; uniform float shiftTT; uniform float shiftTRT; 
						uniform float widthR; uniform float widthTT; uniform float widthTRT; 
						// parameters for the caustic treatment
						uniform float causticLimit; uniform float causticWidth; uniform float glintScale; uniform float causticFade;
						// output factors R, TT and TRT
						output varying color outR; output varying color outTT; output varying color outTRT )
{
	float relativeTheta = abs( eye[1] - lightVec[1] ) / 2.;
	// get refraction indices as described in [1] for R and TT
	float etaPerp = ieBravaisIndex( relativeTheta, refraction );
	float etaParal = (refraction*refraction)/etaPerp;
	// get refraction indices modified by the eccentricity to use in TRT
	float refractionTRT = ieMarschnerEccentricityRefraction( eccentricity, refraction, (eye[0] + lightVec[0]) / 2 );
	float etaPerpTRT = ieBravaisIndex( relativeTheta, refractionTRT );
	float etaParalTRT = (refractionTRT*refractionTRT)/etaPerpTRT;
	float averageTheta = ( eye[1] + lightVec[1] ) / 2.;
	float relativeAzimuth = mod( abs( eye[0] - lightVec[0] ), 2*PI );
	float cosRelativeTheta = cos( relativeTheta );
	float invSqrCosRelativeTheta = 1/max(1e-3,cosRelativeTheta*cosRelativeTheta);
	float cosLight = cos(lightVec[1]);
	float finalScale = max(0,invSqrCosRelativeTheta*cosLight);

	uniform float rWidth = 5;
	float MR = ieMarschnerM( shiftR, widthR, rWidth, averageTheta );
	float MTT = ieMarschnerM( shiftTT, widthTT, rWidth/2, averageTheta );
	float MTRT = ieMarschnerM( shiftTRT, widthTRT, rWidth*2, averageTheta );

	color NR = ieMarschnerNP( absorption, lightVec, 0, refraction, etaPerp, etaParal, ieMarschnerTargetAngle(0,relativeAzimuth) );
	outR = MR*NR*finalScale;

	color NTT = ieMarschnerNP( absorption, lightVec, 1, refraction, etaPerp, etaParal, ieMarschnerTargetAngle(1,relativeAzimuth) );
	outTT = MTT*NTT*finalScale;

	color NTRT = ieMarschnerNTRT( absorption, lightVec, refractionTRT, etaPerpTRT, etaParalTRT, ieMarschnerTargetAngle(2,relativeAzimuth), causticLimit, causticWidth, glintScale, causticFade );
	outTRT = MTRT*NTRT*finalScale;
 
	return outR + outTT + outTRT;
}

// Returns spherical coordinates for any given vector according to [1].
// This function assumes the given euclidian vectors are already converted to the hair frame of reference as described below:
// The Z component is aligned to the hair and it goes from the root to the tip.
// The X component is the major axis for the cross section of the hair - it is important in case the hair is elliptical ( eccentricity != 1 ).
// The Y component completes the right-handed orthonormal basis.
vector ieMarschnerLocalVector( varying vector dir )
{
	// convert euclidian vector to spherical coordinates 
	// Mapping ranges on spherical coordinate Y from [0,pi] to [pi/2,-pi/2] according to convention addopted by [1] on section 2.2.
	return vector( atan( dir[1], dir[0] ), PI/2 - acos( dir[2] ), 0 );
}


#endif // IECORERI_MARSCHNER_H
