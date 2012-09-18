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

#ifndef IECORERI_DISTANCE_H
#define IECORERI_DISTANCE_H

#define IEDISTANCE_LINEAR			0
#define IEDISTANCE_LINEARSQUARED	1
#define IEDISTANCE_CHEBYSHEV		2
#define IEDISTANCE_MANHATTAN		3
#define IEDISTANCE_MINKOWSKY		4	// requires minkowsky number passed as metric parameter

float ieDistance( point a; point b; uniform float metric )
{
	return ieDistance( a, b, metric, 0 );
}

float ieDistance( point a; point b; uniform float metric; uniform float metricParam )
{
	float v0 = a[0] - b[0];
	float v1 = a[1] - b[1];
	float v2 = a[2] - b[2];
	float result = 0;
	if ( metric == IEDISTANCE_LINEAR )
	{
		result = sqrt(v0*v0+v1*v1+v2*v2);
	}
	else if ( metric == IEDISTANCE_LINEARSQUARED )
	{
		result = v0*v0+v1*v1+v2*v2;
	}
	else if ( metric == IEDISTANCE_CHEBYSHEV )
	{
		result = max( abs(v0), abs(v1), abs(v2) );
	}
	else if ( metric == IEDISTANCE_MANHATTAN )
	{
		result = abs(v0) + abs(v1) + abs(v2);
	}
	else if ( metric == IEDISTANCE_MINKOWSKY )
	{
		uniform float minkowskiNumber = metricParam;
		result = pow( pow(abs(v0),minkowskiNumber) + pow( abs(v1), minkowskiNumber )+ pow( abs(v2), minkowskiNumber ), 1/minkowskiNumber );
	}
	return result;
}

#endif // IECORERI_DISTANCE_H
