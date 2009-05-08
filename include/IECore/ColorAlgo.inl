//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_COLORALGO_INL
#define IECORE_COLORALGO_INL

#include "OpenEXR/ImathFun.h"

#include "IECore/SRGBToLinearDataConversion.h"
#include "IECore/LinearToSRGBDataConversion.h"

namespace IECore
{

template<typename T>
typename T::BaseType luminance( const T &color )
{
	return luminance<T, Imath::V3f>( color, Imath::V3f( 0.212671, 0.715160, 0.072169 ) );
}

template<class T, class S>
typename T::BaseType luminance( const T &color, const S &weights )
{
	return color[0] * weights[0] + color[1] * weights[1] + color[2] * weights[2];
}

template<typename T>
T adjustSaturation( const T &color, typename T::BaseType saturation )
{
	typename T::BaseType l = luminance( color );
	T desaturated( color );
	desaturated[0] = l;
	desaturated[1] = l;
	desaturated[2] = l;
	return Imath::lerp( desaturated, color, saturation );
}

template<typename T>
T linearToSRGB( const T &color )
{
	LinearToSRGBDataConversion<typename T::BaseType, typename T::BaseType> cv;
	T result;
	for( unsigned i=0; i<T::dimensions(); i++ )
	{
		result[i] = cv( color[i] );
	}
	return result;
}

template<typename T>
T sRGBToLinear( const T &color )
{
	SRGBToLinearDataConversion<typename T::BaseType, typename T::BaseType> cv;
	T result;
	for( unsigned i=0; i<T::dimensions(); i++ )
	{
		result[i] = cv( color[i] );
	}
	return result;
}

} // namespace IECore

#endif // IECORE_COLORALGO_INL
