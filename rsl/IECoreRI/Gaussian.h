//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORERI_GAUSSIAN_H
#define IECORERI_GAUSSIAN_H

/// Evaluates the one dimensional gaussian function defined by a,b and c
/// at position x. a is the height of the peak, b is the centre of the peak
/// and c controls the width of the bell.
float ieGaussian( float a; float b; float c; float x )
{
	float o = x - b;
	return a * exp( - o * o  / ( 2 * c * c ) );
}

/// Computes the a, b and c parameters for a normalized gaussian pdf with the mean
/// specified by mu and a variance corresponding to sigma squared. This can then be
/// evaluated using the ieGaussian function above.
void ieGaussianPDF( float mu; float sigma; output float a; output float b; output float c )
{
	a = 1 / ( sigma * sqrt( 2 * PI ) );
	b = mu;
	c = sigma;
}

/// Computes the gaussian which is the product of the two gaussians a1,b1,c1 and a2,b2,c2. The parameters
/// for the result are placed in a,b and c, and may then be evaluated using the ieGaussian method above.
/// Taken from http://ccrma.stanford.edu/~jos/sasp/Gaussians_Closed_under_Multiplication.html.
void ieGaussianProduct( float a1; float b1; float c1; float a2; float b2; float c2; output float a; output float b; output float c )
{
	float C1 = -b1;
	float C2 = -b2;
	float P1 = 1 / ( 2 * c1 * c1 );
	float P2 = 1 / ( 2 * c2 * c2 );
	
	float P = P1 + P2;
	float C = (P1*C1 + P2*C2) / P;
	float CC = C1 - C2;
	
	a = a1 * a2 * exp( -P1 * P2 * CC * CC / P );
	b = -C;
	c = sqrt( 1 / ( 2 * P ) );
}

/// Returns the area under the specified gaussian.
float ieGaussianIntegral( float a; float b; float c )
{
	return a * c * sqrt( 2 * PI );
}

#endif // IECORERI_GAUSSIAN_H
