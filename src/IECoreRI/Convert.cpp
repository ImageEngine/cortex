//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#include "IECoreRI/Convert.h"

#include "IECore/SimpleTypedData.h"

namespace IECore
{

template<>
Imath::V3f convert( const RtPoint &from )
{
	return Imath::V3f( from[0], from[1], from[2] );
}

template<>
Imath::Color3f convert( const RtColor &from )
{
	return Imath::Color3f( from[0], from[1], from[2] );
}

template<>
Imath::Box3f convert( const RtBound &from )
{
	return Imath::Box3f( Imath::V3f( from[0], from[2], from[4] ), Imath::V3f( from[1], from[3], from[5] ) );
}


void convert( const Imath::M44f &m, RtMatrix mm )
{
	for( unsigned int i=0; i<4; i++ )
	{
		for( unsigned int j=0; j<4; j++ )
		{
			mm[i][j] = m[i][j];
		}
	}
}

void convert( const Imath::Box3f &from, RtBound to )
{
	to[0] = from.min.x;
	to[1] = from.max.x;
	to[2] = from.min.y;
	to[3] = from.max.y;
	to[4] = from.min.z;
	to[5] = from.max.z;
}

DataPtr convert( const char *data, RxInfoType_t type, RtInt count )
{
	switch( type )
	{
		case RxInfoFloat :
			assert( count==1 );
			return new FloatData( *(float *)data );
		case RxInfoInteger :
			assert( count==1 );
			return new IntData( *(int *)data );
		case RxInfoStringV :
			assert( count==1 );
			return new StringData( *(char **)data );
		case RxInfoColor :
			{
				assert( count==3 );
				const float *fData = (const float *)data;
				return new Color3fData( Imath::Color3f( fData[0], fData[1], fData[2] ) );
			}
		case RxInfoNormal :
		case RxInfoVector :
		case RxInfoPoint :
			{
				assert( count==3 );
				const float *fData = (const float *)data;
				return new V3fData( Imath::V3f( fData[0], fData[1], fData[2] ) );
			}
		case RxInfoMPoint :
		case RxInfoMatrix :
			{
				assert( count==16 );
				const float *fd = (const float *)data;
				return new M44fData( Imath::M44f(
					fd[0], fd[1], fd[2], fd[3],
					fd[4], fd[5], fd[6], fd[7],
					fd[8], fd[9], fd[10], fd[11],
					fd[12], fd[13], fd[14], fd[15]
				) );
			}
		default :
			return 0;
	}
}

} // namespace IECore
