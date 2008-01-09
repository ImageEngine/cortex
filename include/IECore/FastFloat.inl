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
//	     other contributors to this software may be used to endorse or
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

#ifndef IE_CORE_FASTFLOAT_INL
#define IE_CORE_FASTFLOAT_INL

namespace IECore
{
#define IECORE_DOUBLEMAGICROUNDEPS	(.5-1.4e-11)

#if (defined(__linux__) && defined(__i386__)) || defined(WIN32)	
	#define IECORE_DOUBLEMAGIC			double (6755399441055744.0)

	inline int fastFloat2Int( double v )
	{
		 return ( v < 0.0 ) ? fastFloatRound( v + IECORE_DOUBLEMAGICROUNDEPS ) : fastFloatRound(v-IECORE_DOUBLEMAGICROUNDEPS );
	}

	union FastFloatDoubleLong
	{
		double d;
		long l;
	};

	inline int fastFloatRound( double v )
	{
		FastFloatDoubleLong vv;
		vv.d = v + IECORE_DOUBLEMAGIC;
		return vv.l;
	}

	inline int fastFloatFloor( double v )
	{
		return fastFloatRound( v - IECORE_DOUBLEMAGICROUNDEPS );
	}

	inline int fastFloatCeil( double v )
	{
		return fastFloatRound( v + IECORE_DOUBLEMAGICROUNDEPS );
	}
	
	// From: http://www.beyond3d.com/articles/fastinvsqrt/
	inline float fastFloatInvSqrt( float x )
	{
		float xhalf = 0.5f * x;
		int i = *(int*)(&x);
		i = 0x5f3759df - (i >> 1 );
		x = *(float*)&i;
		x = x*(1.5f - xhalf*x*x);
		return x;
	}

#else

	inline int fastFloat2Int( double v )
	{
		 return (int)v;
	}

	inline int fastFloatRound( double v )
	{
		return int( v+IECORE_DOUBLEMAGICROUNDEPS );
	}

	inline int fastFloatFloor( double v )
	{
		return (int)floorf( v );
	}

	inline int fastFloatCeil( double v )
	{
		return (int)ceilf( v );
	}
	
	
	inline float fastFloatInvSqrt( float x )
	{
		return 1.0f / sqrtf(x);
	}

#endif

} // namespace IECore

#endif // IE_CORE_FASTFLOAT_INL
