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

#ifndef IE_CORE_MATRIXOPS_INL
#define IE_CORE_MATRIXOPS_INL

namespace IECore
{

template<typename T>
inline void matSet( T &m, unsigned int i, unsigned int j, typename MatrixTraits<T>::BaseType x )
{
	MatrixTraits<T>::set( m, i, j, x );
}

template<typename T>
inline void matSetAll( T &m, typename MatrixTraits<T>::BaseType x )
{
	for( unsigned int i=0; i<MatrixTraits<T>::dimensions(); i++ )
	{
		for( unsigned int j=0; j<MatrixTraits<T>::dimensions(); j++ )
		{
			MatrixTraits<T>::set( m, i, j, x );
		}
	}
}

template<typename T>
inline typename MatrixTraits<T>::BaseType matGet( const T &m, unsigned int i, unsigned int j )
{
	return MatrixTraits<T>::get( m, i, j );
}

template<typename T, typename S>
inline S matConvert( const T &m )
{
	S result;
	matConvert( m, result );
	return result;
}

template<typename T, typename S>
inline void matConvert( const T &m1, S &m2 )
{
	for( unsigned int i=0; i<MatrixTraits<S>::dimensions(); i++ )
	{
		for( unsigned int j=0; j<MatrixTraits<T>::dimensions(); j++ )
		{
			MatrixTraits<S>::set( m2, i, j, (typename MatrixTraits<S>::BaseType)MatrixTraits<T>::get( m1, i, j ) );
		}
	}
}

template<typename T, typename S>
inline S MatConvert<T,S>::operator()( const T &m ) const
{
	S result;
	matConvert( m, result );
	return result;
}

} // namespace IECore


#endif // IE_CORE_MATRIXOPS_INL
