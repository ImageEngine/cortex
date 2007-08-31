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


// Partially specialize for Imath::Quat. Interpolates through the shortest path.
template<typename T>
struct LinearInterpolator< Imath::Quat<T> >
{
	void operator()(const Imath::Quat<T> &y0, 
			const Imath::Quat<T> &y1,
			double x, 
			Imath::Quat<T> &result) const
	{
		Imath::Quat< T > y0Tmp( y0.normalized() );
		Imath::Quat< T > y1Tmp( y1.normalized() );
		if ( (y0Tmp ^ y1Tmp) < 0.0 )
		{
			result = Imath::slerp< T >( y0Tmp, -y1Tmp, static_cast< T >(x) );
		}
		else
		{
			result = Imath::slerp< T >( y0Tmp, y1Tmp, static_cast< T >(x) );
		}
	}
};

// Partially specialize for Imath::Quat. Interpolates through the shortest path.
template<typename T>
struct CosineInterpolator< Imath::Quat<T> >
{
	void operator()(const Imath::Quat<T> &y0, 
			const Imath::Quat<T> &y1,
			double x, 
			Imath::Quat<T> &result) const
	{
		double cx = (1.0 - cos(x * M_PI)) / 2.0;
		LinearInterpolator< Imath::Quat<T> >()( y0, y1, static_cast< T >(cx), result );
	}
};

// Partially specialize for Imath::Quat. Interpolates through the shortest path.
template<typename T>
struct CubicInterpolator< Imath::Quat< T > >
{
	void operator()(const Imath::Quat< T > &y0, 
			const Imath::Quat< T > &y1,
			const Imath::Quat< T > &y2,
			const Imath::Quat< T > &y3,
			double x, 
			Imath::Quat< T > &result) const
	{
		Imath::Quat< T > y0Tmp( y0.normalized() );
		Imath::Quat< T > y1Tmp( y1.normalized() );
		Imath::Quat< T > y2Tmp( y2.normalized() );
		Imath::Quat< T > y3Tmp( y3.normalized() );

		if ( (y0Tmp ^ y1Tmp) < 0.0 )
		{
			y1Tmp = -y1Tmp;
		}
		if ( (y1Tmp ^ y2) < 0.0 )
		{
			y2Tmp = -y2Tmp;
		}
		if ( (y2Tmp ^ y3) < 0.0 )
		{
			y3Tmp = -y3Tmp;
		}
		result = Imath::spline< T >( y0Tmp, y1Tmp, y2Tmp, y3Tmp, static_cast< T >(x) );
	}
};
