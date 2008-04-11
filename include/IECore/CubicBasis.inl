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

#ifndef IECORE_CUBICBASIS_INL
#define IECORE_CUBICBASIS_INL

#include "IECore/CubicBasis.h"

namespace IECore
{

template<typename T>
CubicBasis<T>::CubicBasis( const MatrixType &m, unsigned s )
	:	matrix( m ), step( s )
{
}

template<typename T>
template<class S>
inline void CubicBasis<T>::coefficients( S t, S &c0, S &c1, S &c2, S &c3 ) const
{
	S t2 = t * t;
	S t3 = t2 * t;
	c0 = matrix[0][0] * t3 + matrix[1][0] * t2 + matrix[2][0] * t + matrix[3][0];
	c1 = matrix[0][1] * t3 + matrix[1][1] * t2 + matrix[2][1] * t + matrix[3][1];
	c2 = matrix[0][2] * t3 + matrix[1][2] * t2 + matrix[2][2] * t + matrix[3][2];
	c3 = matrix[0][3] * t3 + matrix[1][3] * t2 + matrix[2][3] * t + matrix[3][3];
}

template<typename T>
template<class S>
inline S CubicBasis<T>::operator() ( typename S::BaseType t, const S &p0, const S &p1, const S &p2, const S &p3 ) const
{
	typename S::BaseType c0, c1, c2, c3;
	coefficients( t, c0, c1, c2, c3 );
	return c0 * p0 + c1 * p1 + c2 * p2 + c3 * p3;
}


template<typename T>
bool CubicBasis<T>::operator==( const CubicBasis &rhs ) const
{
	return step==rhs.step && matrix==rhs.matrix;
}

template<typename T>
bool CubicBasis<T>::operator!=( const CubicBasis &rhs ) const
{
	return step!=rhs.step || matrix!=rhs.matrix;
}

template<typename T>
const CubicBasis<T> &CubicBasis<T>::linear()
{
	static CubicBasis<T> m(
		MatrixType(
			 0,  0, -1,  1,
			 0,  0,  1,  0,
			 0,  0,  0,  0, 
			 0,  0,  0,  0
		),
		1
	);
	return m;
}
		
template<typename T>
const CubicBasis<T> &CubicBasis<T>::bezier()
{
	static CubicBasis<T> m(
		MatrixType(
			-1,  3, -3,  1,
			 3, -6,  3,  0,
			-3,  3,  0,  0, 
			 1,  0,  0,  0
		),
		3
	);
	return m;
}

template<typename T>
const CubicBasis<T> &CubicBasis<T>::bSpline()
{
	static CubicBasis<T> m(
		MatrixType(
			-1/6.0f,  3/6.0f, -3/6.0f,  1/6.0f,
			 3/6.0f, -6/6.0f,  3/6.0f,       0,
			-3/6.0f,       0,  3/6.0f,       0,
			 1/6.0f,  4/6.0f,  1/6.0f,       0 
		),
		1
	);
	return m;
}

template<typename T>
const CubicBasis<T> &CubicBasis<T>::catmullRom()
{
	static CubicBasis<T> m(
		MatrixType(
			-1/2.0f,  3/2.0f, -3/2.0f,  1/2.0f,
			 2/2.0f, -5/2.0f,  4/2.0f, -1/2.0f,
			-1/2.0f,       0,  1/2.0f,       0,
			      0,  2/2.0f,       0,       0 
		),
		1
	);
	return m;
}
	
} // namespace IECore

#endif // IECORE_CUBICBASIS_INL
