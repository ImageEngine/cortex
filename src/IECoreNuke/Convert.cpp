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

#include "IECoreNuke/Convert.h"

namespace IECore
{

template<>
Imath::V2f convert( const DD::Image::Vector3 &from )
{
	return Imath::V2f( from.x, from.y );
}

template<>
Imath::V2d convert( const DD::Image::Vector3 &from )
{
	return Imath::V2d( from.x, from.y );
}

template<>
Imath::V3f convert( const DD::Image::Vector3 &from )
{
	return Imath::V3f( from.x, from.y, from.z );
}

template<>
Imath::V3d convert( const DD::Image::Vector3 &from )
{
	return Imath::V3d( from.x, from.y, from.z );
}

template<>
Imath::V2f convert( const DD::Image::Vector4 &from )
{
	return Imath::V2f( from.x, from.y );
}

template<>
Imath::V2d convert( const DD::Image::Vector4 &from )
{
	return Imath::V2d( from.x, from.y );
}

template<>
Imath::V3f convert( const DD::Image::Vector4 &from )
{
	return Imath::V3f( from.x, from.y, from.z );
}

template<>
Imath::V3d convert( const DD::Image::Vector4 &from )
{
	return Imath::V3d( from.x, from.y, from.z );
}

template<>
Imath::M44f convert( const DD::Image::Matrix4 &from )
{
	Imath::M44f result;
	for( unsigned int i=0; i<4; i++ )
	{
		for( unsigned int j=0; j<4; j++ )
		{
			result[j][i] = from[j][i];
		}
	}
	return result;
}

template<>
Imath::M44d convert( const DD::Image::Matrix4 &from )
{
	Imath::M44d result;
	for( unsigned int i=0; i<4; i++ )
	{
		for( unsigned int j=0; j<4; j++ )
		{
			result[j][i] = from[j][i];
		}
	}
	return result;
}


} // namespace IECore
