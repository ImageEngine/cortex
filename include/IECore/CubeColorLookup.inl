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

#ifndef IECORE_CUBECOLORLOOKUP_INL
#define IECORE_CUBECOLORLOOKUP_INL

#include <cassert>

#include "boost/multi_array.hpp"

#include "IECore/Exception.h"

#include "OpenEXR/ImathLimits.h"
#include "OpenEXR/ImathMath.h"
#include "OpenEXR/ImathBoxAlgo.h"

namespace IECore
{

template<typename T>
CubeColorLookup<T>::CubeColorLookup() : m_dimension( 2, 2, 2 ), m_domain( VecType( 0, 0, 0 ), VecType( 1, 1, 1 ) ), m_interpolation( Linear )
{
	m_data.reserve( 2 * 2 * 2 );
	
	m_data.push_back( ColorType( 0, 0, 0 ) );
	m_data.push_back( ColorType( 0, 0, 1 ) );
	m_data.push_back( ColorType( 0, 1, 0 ) );
	m_data.push_back( ColorType( 0, 1, 1 ) );
	m_data.push_back( ColorType( 1, 0, 0 ) );
	m_data.push_back( ColorType( 1, 0, 1 ) );
	m_data.push_back( ColorType( 1, 1, 0 ) );
	m_data.push_back( ColorType( 1, 1, 1 ) );
	
	assert( (int)( m_data.size() ) == m_dimension.x * m_dimension.y * m_dimension.z );
}

template<typename T>
CubeColorLookup<T>::CubeColorLookup( const Imath::V3i &dimension, const DataType &data, const BoxType &domain, Interpolation interpolation )
{
	setInterpolation( interpolation );
	setCube( dimension, data, domain );
}

template<typename T>
CubeColorLookup<T>::~CubeColorLookup()
{
}

template<typename T>
void CubeColorLookup<T>::setCube( const Imath::V3i &dimension, const DataType &data, const BoxType &domain )
{
	if ( (int)(data.size()) != dimension.x * dimension.y * dimension.z )
	{
		throw InvalidArgumentException( "CubeColorLookup: Data of invalid length given for specified dimension" );
	}
	
	if ( dimension.x < 2 || dimension.y < 2 || dimension.z < 2 )
	{
		throw InvalidArgumentException( "CubeColorLookup: Dimension must be at least 2 in every axis" );
	}
	
	if ( domain.isEmpty() )
	{
		throw InvalidArgumentException( "CubeColorLookup: Cannot specify empty domain" );
	}
	
	m_dimension = dimension;
	m_data = data;	
	m_domain = domain;	
	
	assert( m_data.size() > 0 );
}

template<typename T>
inline int CubeColorLookup<T>::clamp( int v, int min, int max ) const
{
	if (v < min) return min;
	if (v > max) return max;
	return v;
}

template<typename T>
typename CubeColorLookup<T>::ColorType CubeColorLookup<T>::operator() ( const ColorType &color ) const
{
	assert( m_data.size() > 0 );
	boost::const_multi_array_ref< ColorType, 3 > colorArray( &m_data[0], boost::extents[ m_dimension.x ][ m_dimension.y ][ m_dimension.z ] );
	
	const ColorType clampedColor = Imath::closestPointInBox( color, m_domain );

	switch ( m_interpolation )
	{
		case NoInterpolation :
			{
				const VecType idx = normalizedCoordinates( clampedColor );
				
				return colorArray[ (int)( idx.x + 0.5 ) ][ (int)( idx.y + 0.5 ) ][ (int)( idx.z + 0.5 ) ];
			}
			break;
			
		case Linear:
			{
				LinearInterpolator<ColorType> interp;
				
				const VecType idx = normalizedCoordinates( clampedColor );
				
				int ix = (int)floor( idx.x );
				int iy = (int)floor( idx.y );
				int iz = (int)floor( idx.z );
				
				assert( ix >= 0 );
				assert( iy >= 0 );				
				assert( iz >= 0 );				
				
				T fx = idx.x - (T)ix;
				T fy = idx.y - (T)iy;
				T fz = idx.z - (T)iz;
				
				assert( fx >= -Imath::limits<T>::epsilon() );
				assert( fx <= T(1.0) + Imath::limits<T>::epsilon() );
				
				assert( fy >= -Imath::limits<T>::epsilon() );
				assert( fy <= T(1.0) + Imath::limits<T>::epsilon() );
				
				assert( fz >= -Imath::limits<T>::epsilon() );
				assert( fz <= T(1.0) + Imath::limits<T>::epsilon() );
				
				ColorType tmp1[2];
				ColorType tmp2[2];
				ColorType tmp3[2];
				
				for ( int x = 0; x <= 1; x ++ )
				{
					int xidx = clamp( ix + x, 0, m_dimension.x - 1 );
					
					for ( int y = 0; y <= 1; y ++ )
					{
						int yidx = clamp( iy + y, 0, m_dimension.y - 1 );
						
						for ( int z = 0; z <= 1; z ++ )
						{
							int zidx = clamp( iz + z, 0, m_dimension.z - 1 );
							
							tmp1[ z ] = colorArray[ xidx ][ yidx ][ zidx ];
						}
						
						interp( tmp1[0], tmp1[1], fz, tmp2[ y ] );
					}
					
					 interp( tmp2[ 0 ], tmp2[ 1 ], fy, tmp3[ x ] );				
				}
				
				ColorType result;
				interp( tmp3[ 0 ], tmp3[ 1 ], fx, result );				
				return result;
			}
			break;
		default:
			assert( false );
	}

	return color;
}

template<typename T>
const Imath::V3i &CubeColorLookup<T>::dimension() const
{
	return m_dimension;
}

template<typename T>
const typename CubeColorLookup<T>::BoxType &CubeColorLookup<T>::domain() const
{
	return m_domain;
}

template<typename T>
const typename CubeColorLookup<T>::DataType &CubeColorLookup<T>::data() const
{
	return m_data;
}

template<typename T>
typename CubeColorLookup<T>::Interpolation CubeColorLookup<T>::getInterpolation() const
{
	return m_interpolation;
}

template<typename T>
void CubeColorLookup<T>::setInterpolation( Interpolation i )
{
	m_interpolation = i;
}

template<typename T>
typename CubeColorLookup<T>::VecType CubeColorLookup<T>::normalizedCoordinates( const ColorType &color ) const
{
	return VecType(	
		( color.x - m_domain.min.x ) / ( m_domain.max.x - m_domain.min.x ) * (T)( m_dimension.x - 1 ),	
		( color.y - m_domain.min.y ) / ( m_domain.max.y - m_domain.min.y ) * (T)( m_dimension.y - 1 ),
		( color.z - m_domain.min.z ) / ( m_domain.max.z - m_domain.min.z ) * (T)( m_dimension.z - 1 )
	);
}

template<typename T>
bool CubeColorLookup<T>::operator==( const CubeColorLookup<T> &rhs ) const
{
	return m_dimension == rhs.m_dimension && m_domain == rhs.m_domain && m_data == rhs.m_data && m_interpolation == rhs.m_interpolation;

}

template<typename T>
bool CubeColorLookup<T>::operator!=( const CubeColorLookup<T> &rhs ) const
{
	return m_dimension != rhs.m_dimension || m_domain != rhs.m_domain || m_data != rhs.m_data || m_interpolation != rhs.m_interpolation;
}

} // namespace IECore

#endif // IECORE_CUBECOLORLOOKUP_INL
