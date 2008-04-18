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
#include <vector>

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

template <typename T>
bool boxContains( const T &box, const T &containee )
{
	typename BoxTraits<T>::BaseType min = BoxTraits<T>::min( box );
	typename BoxTraits<T>::BaseType max = BoxTraits<T>::max( box );
	
	typename BoxTraits<T>::BaseType cMin = BoxTraits<T>::min( containee );
	typename BoxTraits<T>::BaseType cMax = BoxTraits<T>::max( containee );
	
	for( unsigned int d=0; d<BoxTraits<T>::BaseType::dimensions(); d++ )
	{
		if( vecGet( cMin, d ) < vecGet( min, d ) )
		{
			return false;
		}
		if( vecGet( cMax, d ) > vecGet( max, d ) )
		{
			return false;
		}
	}
	return true;
}

/// Based on "Fast Ray-Box Intersection", by Andrew Woo, "Graphics Gems", Academic Press, 1990
template<typename T>
bool boxIntersects(
        const T &box,
        const typename BoxTraits<T>::BaseType &origin,
        const typename BoxTraits<T>::BaseType &direction,
        typename BoxTraits<T>::BaseType &result
)
{
	const char right = 0;
	const char left = 1;
	const char middle = 2;

	typedef typename BoxTraits<T>::BaseType Vec;
	typedef typename VectorTraits<Vec>::BaseType Real;

	Vec minB = BoxTraits<T>::min( box );
	Vec maxB = BoxTraits<T>::max( box );
	
	unsigned int dimension = VectorTraits<Vec>::dimensions();
	assert( dimension >= 2 );

	bool inside = true;
	std::vector<char> quadrant;
	quadrant.resize(dimension);
	
	Vec maxT;	
	Vec candidatePlane;

	for ( unsigned int i = 0; i < dimension; i++ )
	{
		if ( vecGet( origin, i ) < vecGet( minB, i ))
		{
			quadrant[i] = left;			
			vecSet( candidatePlane, i, vecGet( minB, i ) );
			inside = false;
		}
		else if (vecGet( origin, i ) > vecGet( maxB, i ))
		{
			quadrant[i] = right;
			vecSet( candidatePlane, i, vecGet( maxB, i ) );
			inside = false;
		}
		else
		{
			quadrant[i] = middle;
		}
	}

	if ( inside )
	{
		result = origin;
		return true;
	}

	for ( unsigned int i = 0; i < dimension; i++ )
	{
		if ( quadrant[i] != middle && vecGet( direction, i ) != 0. )
		{
			vecSet( maxT, i, ( vecGet( candidatePlane, i ) - vecGet( origin, i ) ) / vecGet( direction, i ) );
		}
		else
		{
			vecSet( maxT, i, -1. );
		}
	}

	unsigned int whichPlane = 0;

	for ( unsigned int i = 1; i < dimension; i++ )
	{
		if ( vecGet( maxT, whichPlane ) < vecGet( maxT, i ) )
		{
			whichPlane = i;
		}
	}

	if ( vecGet( maxT, whichPlane ) < 0. )
	{
		return false;
	}

	for ( unsigned int i = 0; i < dimension; i++ )
	{
		if ( whichPlane != i )
		{
			vecSet( result, i, vecGet( origin, i ) + vecGet( maxT, whichPlane ) * vecGet( direction, i ) );

			if ( vecGet( result, i ) < vecGet( minB, i ) || vecGet( result, i ) > vecGet( maxB, i ) )
			{
				return false;
			}
		}
		else
		{
			vecSet( result, i, vecGet( candidatePlane, i ) );
		}
	}

	return true;

}


} // namespace IECore


#endif // IE_CORE_BOXOPS_INL

