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

#ifndef IE_CORE_MATRIXALGO_INL
#define IE_CORE_MATRIXALGO_INL

namespace IECore
{

template<typename T>
Imath::Matrix44<T> matrixFromBasis( const Imath::Vec3<T> &x, const Imath::Vec3<T> &y, const Imath::Vec3<T> &z, const Imath::Vec3<T> &o )
{
	Imath::Matrix44<T> result;
	result[0][0] = x[0];
	result[0][1] = x[1];
	result[0][2] = x[2];
	result[1][0] = y[0];
	result[1][1] = y[1];
	result[1][2] = y[2];
	result[2][0] = z[0];
	result[2][1] = z[1];
	result[2][2] = z[2];	
	result[3][0] = o[0];
	result[3][1] = o[1];
	result[3][2] = o[2];
	return result;
}

template<class T>
float determinant( const Imath::Matrix33<T> &m )
{
	// a(ei-fh) - b(di-fg) + c(dh-eg)
	return
		
		m[0][0] * ( m[1][1] * m[2][2] - m[1][2] * m[2][1] ) - 
		m[0][1] * ( m[1][0] * m[2][2] - m[1][2] * m[2][0] ) + 
		m[0][2] * ( m[1][0] * m[2][1] - m[1][1] * m[2][0] );
		
}

template<class T>
float determinant( const Imath::Matrix44<T> &m )
{
	// a * | f g h |
	//     | j k l |
	//     | n o p | -
	//
	// b * | e g h |
	//     | i k l |
	//     | m o p | +
	//
	// c * | e f h |
	//     | i j l |
	//     | m n p | -
	//
	// d * | e f g |
	//     | i j k |
	//     | m n o |
	
	return
	
		m[0][0] * determinant(
			Imath::M33f( 
				m[1][1], m[1][2], m[1][3],
				m[2][1], m[2][2], m[2][3],
				m[3][1], m[3][2], m[3][3]
			)
		) - 
		
		m[0][1] * determinant(
			Imath::M33f( 
				m[1][0], m[1][2], m[1][3],
				m[2][0], m[2][2], m[2][3],
				m[3][0], m[3][2], m[3][3]
			)
		) +
		
		 
		m[0][2] * determinant(
			Imath::M33f( 
				m[1][0], m[1][1], m[1][3],
				m[2][0], m[2][1], m[2][3],
				m[3][0], m[3][1], m[3][3]
			)
		) -
		
		m[0][3] * determinant(
			Imath::M33f( 
				m[1][0], m[1][1], m[1][2],
				m[2][0], m[2][1], m[2][2],
				m[3][0], m[3][1], m[3][2]
			)
		)
	
	;
}

} // namespace IECore

#endif // IE_CORE_MATRIXALGO_INL
