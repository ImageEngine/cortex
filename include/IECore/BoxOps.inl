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

#ifndef IE_CORE_BOXOPS_INL
#define IE_CORE_BOXOPS_INL

#include <cassert>

#include <IECore/VectorOps.h>

namespace IECore
{

template<typename T>
inline typename BoxTraits<T>::BaseType boxSize( const T &box )
{
	return vecSub( BoxTraits<T>::max( box ), BoxTraits<T>::min( box ) );
}

template<typename T>
inline typename BoxTraits<T>::BaseType boxCenter( const T &box )
{
	return vecMul(
		vecAdd( BoxTraits<T>::max( box ), BoxTraits<T>::min( box ) ),
		2.0
	);
}

template <typename T>
T boxIntersection( const T &box, const T &box2 )
{
	typename BoxTraits<T>::BaseType min, max;
	
        for( unsigned int d = 0; d < BoxTraits<T>::BaseType::dimensions(); ++d )
        {
                vecSet( min, d, 
			std::max( 
				vecGet( BoxTraits<T>::min( box ), d ),
				vecGet( BoxTraits<T>::min( box2 ), d )
			)
		);
		
		vecSet( max, d, 
			std::min( 
				vecGet( BoxTraits<T>::max( box ), d ),
				vecGet( BoxTraits<T>::max( box2 ), d )
			)
		);
		
		/// Check for no intersection
                if( vecGet( max, d ) < vecGet( min, d ) )
                {
                        return BoxTraits<T>::create();
                }
        }
                
        return BoxTraits<T>::create( min, max );
}

template<typename T>
inline void boxExtend( T &box, const typename BoxTraits<T>::BaseType &p )
{
	typename BoxTraits<T>::BaseType min = BoxTraits<T>::min( box );
	typename BoxTraits<T>::BaseType max = BoxTraits<T>::max( box );	
	
	for( unsigned int d = 0; d < BoxTraits<T>::BaseType::dimensions(); ++d )
        {
		if ( vecGet( p, d ) < vecGet( min, d ) )
		{
			vecSet( min, d, vecGet( p, d ) );
		}
		if ( vecGet( p, d ) > vecGet( max, d ) )
		{
			vecSet( max, d, vecGet( p, d ) );
		}
	}
	
	BoxTraits<T>::setMin( box, min );
	BoxTraits<T>::setMax( box, max );	
}

template<typename T>
inline void boxExtend( T &box, const T &box2 )
{
	typename BoxTraits<T>::BaseType min = BoxTraits<T>::min( box );
	typename BoxTraits<T>::BaseType max = BoxTraits<T>::max( box );	
	
	typename BoxTraits<T>::BaseType min2 = BoxTraits<T>::min( box2 );
	typename BoxTraits<T>::BaseType max2 = BoxTraits<T>::max( box2 );		
	
	for( unsigned int d = 0; d < BoxTraits<T>::BaseType::dimensions(); ++d )
        {
		if ( vecGet( min2, d ) < vecGet( min, d ) )
		{
			vecSet( min, d, vecGet( min2, d ) );
		}
		if ( vecGet( max2, d ) > vecGet( max, d ) )
		{
			vecSet( max, d, vecGet( max2, d ) );
		}
	}
	
	BoxTraits<T>::setMin( box, min );
	BoxTraits<T>::setMax( box, max );	
}

template <typename T>
bool boxIntersects(const T &box, const typename BoxTraits<T>::BaseType &p)
{
	typename BoxTraits<T>::BaseType min = BoxTraits<T>::min( box );
	typename BoxTraits<T>::BaseType max = BoxTraits<T>::max( box );	
	
	for( unsigned int d = 0; d < BoxTraits<T>::BaseType::dimensions(); ++d )
        {
		if ( vecGet( p, d ) < vecGet( min, d ) )
		{
			return false;
		}
		if ( vecGet( p, d ) > vecGet( max, d ) )
		{
			return false;
		}
	}
	
	return true;
}

template <typename T>
bool boxIntersects( const T &box, const T &box2 )
{
	typename BoxTraits<T>::BaseType min = BoxTraits<T>::min( box );
	typename BoxTraits<T>::BaseType max = BoxTraits<T>::max( box );
	
	typename BoxTraits<T>::BaseType min2 = BoxTraits<T>::min( box2 );
	typename BoxTraits<T>::BaseType max2 = BoxTraits<T>::max( box2 );		
	
	for( unsigned int d = 0; d < BoxTraits<T>::BaseType::dimensions(); ++d )
        {
		if ( vecGet( max2, d ) < vecGet( min, d ) )
		{
			return false;
		}
		if ( vecGet( min2, d ) > vecGet( max, d ) )
		{
			return false;
		}
	}
	
	return true;
}

} // namespace IECore


#endif // IE_CORE_BOXOPS_INL

