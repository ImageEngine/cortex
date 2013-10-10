//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/GeometricTypedData.h"

namespace IECore
{

// Partially specialise for GeometricTypedData
template<typename T>
struct LinearInterpolator< GeometricTypedData< T > >
{
	void operator()(const GeometricTypedData< T > *y0,
			const GeometricTypedData< T > *y1,
			double x,
			typename GeometricTypedData< T >::Ptr &result) const
	{
		LinearInterpolator<T>()( y0->readable(), y1->readable(), x, result->writable());
		result->setInterpretation( y0->getInterpretation() );
	}
};

// Partially specialise for GeometricTypedData
template<typename T>
struct CubicInterpolator< GeometricTypedData< T > >
{
	void operator()(const GeometricTypedData< T > *y0,
			const GeometricTypedData< T > *y1,
			const GeometricTypedData< T > *y2,
			const GeometricTypedData< T > *y3,
			double x,
			typename GeometricTypedData< T >::Ptr &result) const
	{
		CubicInterpolator<T>()( y0->readable(), y1->readable(), y2->readable(), y3->readable(), x, result->writable());
		result->setInterpretation( y0->getInterpretation() );
	}
};

} // namespace IECore
