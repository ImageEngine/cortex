//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#include <cassert>

#include "IECore/VectorTypedData.h"
#include "IECore/SimpleTypedData.h"

#include "IECoreHoudini/Convert.h"

namespace IECore
{

template<>
UT_Vector3 convert( const Imath::V3f &from )
{
	return UT_Vector3( from.x, from.y, from.z );
}

template<>
Imath::V3f convert( const UT_Vector3 &from )
{
	return Imath::V3f( from[0], from[1], from[2] );
}

template<>
UT_Vector3 convert( const Imath::V3d &from )
{
	return UT_Vector3( from.x, from.y, from.z );
}

template<>
Imath::V3d convert( const UT_Vector3 &from )
{
	return Imath::V3d( from[0], from[1], from[2] );
}

template<>
UT_Vector4 convert( const Imath::V3f &from )
{
	return UT_Vector4( from.x, from.y, from.z );
}

template<>
Imath::V3f convert( const UT_Vector4 &from )
{
	return Imath::V3f( from[0], from[1], from[2] );
}

template<>
UT_Vector4 convert( const Imath::V3d &from )
{
	return UT_Vector4( from.x, from.y, from.z );
}

template<>
Imath::V3d convert( const UT_Vector4 &from )
{
	return Imath::V3d( from[0], from[1], from[2] );
}

template<>
Imath::Color3f convert( const UT_Color &from )
{
	float r, g, b;
	from.getRGB( &r, &g, &b );
	return Imath::Color3f( r, g, b );
}

template<>
UT_Color convert( const Imath::Color3f &from )
{
	return UT_Color( UT_RGB, from[0], from[1], from[2] );
}

template<>
Imath::Color4f convert( const UT_Color &from )
{
	float r, g, b;
	from.getRGB( &r, &g, &b );
	return Imath::Color4f( r, g, b, 1 );
}

template<>
UT_Color convert( const Imath::Color4f &from )
{
	return UT_Color( UT_RGB, from[0], from[1], from[2] );
}

template<>
UT_BoundingBox convert( const Imath::Box3f &from )
{
	if( from.isEmpty() )
	{
		return UT_BoundingBox();
	}
	return UT_BoundingBox( convert<UT_Vector3>( from.min ), convert<UT_Vector3>( from.max ) );
}

template<>
Imath::Box3f convert( const UT_BoundingBox &from )
{
	return Imath::Box3f( convert<Imath::V3f>( from.minvec() ), convert<Imath::V3f>( from.maxvec() ) );
}

template<>
UT_BoundingBox convert( const Imath::Box3d &from )
{
	if( from.isEmpty() )
	{
		return UT_BoundingBox();
	}
	return UT_BoundingBox( convert<UT_Vector3>( from.min ), convert<UT_Vector3>( from.max ) );
}

template<>
Imath::Box3d convert( const UT_BoundingBox &from )
{
	return Imath::Box3d( convert<Imath::V3d>( from.minvec() ), convert<Imath::V3d>( from.maxvec() ) );
}

template<>
UT_Matrix4T<double> convert( const Imath::M44d &from )
{
	return UT_Matrix4T<double>( from[0][0], from[0][1], from[0][2], from[0][3], from[1][0], from[1][1], from[1][2], from[1][3], from[2][0], from[2][1], from[2][2], from[2][3], from[3][0], from[3][1], from[3][2], from[3][3] );
}

template<>
Imath::M44d convert( const UT_Matrix4T<double> &from )
{
	return Imath::M44d( from[0][0], from[0][1], from[0][2], from[0][3], from[1][0], from[1][1], from[1][2], from[1][3], from[2][0], from[2][1], from[2][2], from[2][3], from[3][0], from[3][1], from[3][2], from[3][3] );
}

template<>
Imath::M44f convert( const UT_Matrix4T<double> &from )
{
	return Imath::M44f( from[0][0], from[0][1], from[0][2], from[0][3], from[1][0], from[1][1], from[1][2], from[1][3], from[2][0], from[2][1], from[2][2], from[2][3], from[3][0], from[3][1], from[3][2], from[3][3] );
}

template<>
UT_Matrix4T<float> convert( const Imath::M44f &from )
{
	return UT_Matrix4T<float>( from[0][0], from[0][1], from[0][2], from[0][3], from[1][0], from[1][1], from[1][2], from[1][3], from[2][0], from[2][1], from[2][2], from[2][3], from[3][0], from[3][1], from[3][2], from[3][3] );
}

template<>
Imath::M44f convert( const UT_Matrix4T<float> &from )
{
	return Imath::M44f( from[0][0], from[0][1], from[0][2], from[0][3], from[1][0], from[1][1], from[1][2], from[1][3], from[2][0], from[2][1], from[2][2], from[2][3], from[3][0], from[3][1], from[3][2], from[3][3] );
}

template<>
Imath::M44d convert( const UT_Matrix4T<float> &from )
{
	return Imath::M44d( from[0][0], from[0][1], from[0][2], from[0][3], from[1][0], from[1][1], from[1][2], from[1][3], from[2][0], from[2][1], from[2][2], from[2][3], from[3][0], from[3][1], from[3][2], from[3][3] );
}

} // namespace IECore
