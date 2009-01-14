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

#ifndef IECORERI_ARRAYALGO_H
#define IECORERI_ARRAYALGO_H

/// Returns the length of the array. This is simply arraylength(), but
/// is provided so as to have a more complete set of functions with
/// the same naming convention.
float ieArrayLength( float a[] )
{
	return arraylength( a );
}

float ieArrayLength( string a[] )
{
	return arraylength( a );
}

float ieArrayLength( color a[] )
{
	return arraylength( a );
}

/// Returns the sum of all the values in the array.
float ieArraySum( float a[] )
{
	float l = ieArrayLength( a );
	float s = 0;
	float i = 0;
	for( i=0; i<l; i+=1 )
	{
		s += a[i];
	}
	return s;
}

/// Returns the number of times v is present in a.
float ieArrayCount( float a[]; float v )
{
	float l = ieArrayLength( a );
	float c = 0;
	float i = 0;
	for( i=0; i<l; i+=1 )
	{
		if( a[i]==v )
		{
			c += 1;
		}
	}
	return c;
}

/// Checks that array is the specified length, returning 1 if it is. If it isn't, returns 0 and
/// outputs an informative error using context and arrayName.
float ieArrayLengthCheck( float array[]; float length; string context; string arrayName )
{
	uniform float l = ieArrayLength( array );
	if( l!=length )
	{
		printf( "ERROR : %s : Incorrect length for %s (%f but should be %f).\n", context, arrayName, l, length );
		return 0;
	}
	return 1;
}

float ieArrayLengthCheck( string array[]; float length; string context; string arrayName )
{
	uniform float l = ieArrayLength( array );
	if( l!=length )
	{
		printf( "ERROR : %s : Incorrect length for %s (%f but should be %f).\n", context, arrayName, l, length );
		return 0;
	}
	return 1;
}

float ieArrayLengthCheck( color array[]; float length; string context; string arrayName )
{
	uniform float l = ieArrayLength( array );
	if( l!=length )
	{
		printf( "ERROR : %s : Incorrect length for %s (%f but should be %f).\n", context, arrayName, l, length );
		return 0;
	}
	return 1;
}

/// Multiplies all the elements of an array in place.
void ieArrayMultiply( output color array[]; color multiplier )
{
	uniform float l = ieArrayLength( array );
	uniform float i = 0;
	for( i=0; i<l; i+=1 )
	{
		array[i] *= multiplier;
	}
}

/// Multiplies all the elements of an array in place.
void ieArrayMultiply( output float array[]; float multiplier )
{
	uniform float l = ieArrayLength( array );
	uniform float i = 0;
	for( i=0; i<l; i+=1 )
	{
		array[i] *= multiplier;
	}
}

#endif // IECORERI_ARRAYALGO_H
