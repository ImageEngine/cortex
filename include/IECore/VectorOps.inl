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

#ifndef IE_CORE_VECTOROPS_INL
#define IE_CORE_VECTOROPS_INL

namespace IECore
{

template<typename T>
inline void vecSet( T &v, unsigned int i, typename VectorTraits<T>::BaseType x )
{
	VectorTraits<T>::set( v, i, x );
}

template<typename T>
inline void vecSetAll( T &v, typename VectorTraits<T>::BaseType x )
{
	for( unsigned int i=0; i<VectorTraits<T>::dimensions(); i++ )
	{
		VectorTraits<T>::set( v, i, x );
	}
}

template<typename T>
inline typename VectorTraits<T>::BaseType vecGet( const T &v, unsigned int i )
{
	return VectorTraits<T>::get( v, i );
}

template<typename T>
inline T vecAdd( const T &v1, const T &v2 )
{
	T result;
	vecAdd( v1, v2, result );
	return result;
}

template<typename T>
inline void vecAdd( const T &v1, const T &v2, T &result )
{
	for( unsigned int i=0; i<VectorTraits<T>::dimensions(); i++ )
	{
		vecSet( result, i, vecGet( v1, i ) + vecGet( v2, i ) );
	}
}

template<typename T>
inline T vecSub( const T &v1, const T &v2 )
{
	T result;
	vecSub( v1, v2, result );
	return result;
}

template<typename T>
inline void vecSub( const T &v1, const T &v2, T &result )
{
	for( unsigned int i=0; i<VectorTraits<T>::dimensions(); i++ )
	{
		vecSet( result, i, vecGet( v1, i ) - vecGet( v2, i ) );
	}
}

template<typename T>
inline T vecMul( const T& v1, typename VectorTraits<T>::BaseType v2)
{
	T result;
	vecMul( v1, v2, result );
	return result;	
}

template<typename T>
inline void vecMul( const T& v1, typename VectorTraits<T>::BaseType v2, T& result)
{
	for( unsigned int i=0; i<VectorTraits<T>::dimensions(); i++ )
	{
		vecSet( result, i, vecGet( v1, i ) * v2 );
	}
}

template<typename T>
inline T vecMul( const T &v1, const T &v2 )
{
	T result;
	vecMul( v1, v2, result );
	return result;
}

template<typename T>
inline void vecMul( const T &v1, const T &v2, T &result)
{
	for( unsigned char i=0; i<VectorTraits<T>::dimensions(); i++ )
	{
		vecSet( result, i, vecGet( v1, i ) * vecGet( v2, i ) );
	}
}

template<typename T>
inline T vecDiv( const T &v1, typename VectorTraits<T>::BaseType v2 )
{
	T result;
	vecDiv( v1, v2, result );
	return result;
}

template<typename T>
inline void vecDiv( const T &v1, typename VectorTraits<T>::BaseType v2, T &result )
{
	for( unsigned char i=0; i<VectorTraits<T>::dimensions(); i++ )
	{
		vecSet( result, i, vecGet( v1, i ) / v2 );
	}
}

template<typename T>
inline T vecDiv( const T &v1, const T &v2 )
{
	T result;
	vecDiv( v1, v2, result );
	return result;
}

template<typename T>
inline void vecDiv( const T &v1, const T &v2, T &result )
{
	for( unsigned char i=0; i<VectorTraits<T>::dimensions(); i++ )
	{
		vecSet( result, i, vecGet( v1, i ) / vecGet( v2, i ) );
	}
}

template<typename T>
inline typename VectorTraits<T>::BaseType vecDot( const T &v1, const T &v2 )
{
	typename VectorTraits<T>::BaseType result = 0;
	for( unsigned char i=0; i<VectorTraits<T>::dimensions(); i++ )
	{
		result += VectorTraits<T>::get( v1, i ) * VectorTraits<T>::get( v2, i );
	}
	return result;
}

template<typename T>
inline typename VectorTraits<T>::BaseType vecLength2( const T &v )
{
	return vecDot( v, v );
}

template<typename T>
inline typename VectorTraits<T>::BaseType vecLength( const T &v )
{
	return Imath::Math<typename VectorTraits<T>::BaseType>::sqrt( vecLength2( v ) );
}

template<typename T>
inline void vecNormalize( T &v )
{
	typename VectorTraits<T>::BaseType l = vecLength( v );
	if( l!=0 )
	{
		for( unsigned char i=0; i<VectorTraits<T>::dimensions(); i++ )
		{
			VectorTraits<T>::set( v, i, VectorTraits<T>::get( v, i ) / l );
		}
	}
}

template<typename T>
inline typename VectorTraits<T>::BaseType vecDistance2( const T &v1, const T &v2 )
{
	T d;
	vecSub( v1, v2, d );
	return vecLength2( d );
}

template<typename T>
inline typename VectorTraits<T>::BaseType vecDistance( const T &v1, const T &v2 )
{
	return Imath::Math<typename VectorTraits<T>::BaseType>::sqrt( vecDistance2( v1, v2 ) );
}

template<typename T, typename S>
inline S vecConvert( const T &v )
{
	S result;
	vecConvert( v, result );
	return result;
}

template<typename T, typename S>
inline void vecConvert( const T &v1, S &v2 )
{
	for( unsigned int i=0; i<VectorTraits<S>::dimensions(); i++ )
	{
		VectorTraits<S>::set( v2, i, VectorTraits<T>::get( v1, i ) );
	}
}

template<typename T, typename S>
inline S VecConvert<T,S>::operator()( const T &v ) const
{
	S result;
	vecConvert( v, result );
	return result;
}

template<typename T>
inline T vecConstruct( const typename VectorTraits<T>::BaseType *components )
{
	T result;
	for( unsigned int i=0; i<VectorTraits<T>::dimensions(); i++ )
	{
		VectorTraits<T>::set( result, i, components[i] );
	}
}

} // namespace IECore


#endif // IE_CORE_VECTOROPS_INL
