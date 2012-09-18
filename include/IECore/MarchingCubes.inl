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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "OpenEXR/ImathLimits.h"

#include "IECore/MarchingCubes.h"

namespace IECore
{

template< typename ImplicitFn, typename MeshBuilder >
MarchingCubes<ImplicitFn, MeshBuilder>::MarchingCubes( typename ImplicitFn::Ptr fn, typename MeshBuilder::Ptr builder ) :
		m_bound(),		
		m_fn( fn ),
		m_builder( builder ),
		m_resolution( -1, -1, -1 ),
		m_numVerts(0)
{
	assert( m_fn );
	assert( m_builder );	
}

template< typename ImplicitFn, typename MeshBuilder >
MarchingCubes<ImplicitFn, MeshBuilder>::~MarchingCubes()
{
}

template< typename ImplicitFn, typename MeshBuilder >
void MarchingCubes<ImplicitFn, MeshBuilder>::march( const MarchingCubes<ImplicitFn, MeshBuilder>::BoxType &bound, const Imath::V3i &res, typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType iso )
{
	m_resolution = res;
	m_bound = bound;
	m_numVerts = 0;
	m_P = new V3xVectorData();
	m_N = new V3xVectorData();	
		
	m_verts = new V3iVectorData();
	m_verts->writable().resize( m_resolution.x * m_resolution.y * m_resolution.z, Imath::V3i( -1, -1, -1 ) );
	
	computeIntersectionPoints( iso ) ;

	for ( m_currentGridPos.z = 0 ; m_currentGridPos.z < m_resolution.z-1 ; m_currentGridPos.z++ )
	{
		for ( m_currentGridPos.y = 0 ; m_currentGridPos.y < m_resolution.y-1 ; m_currentGridPos.y++ )
		{
			for ( m_currentGridPos.x = 0 ; m_currentGridPos.x < m_resolution.x-1 ; m_currentGridPos.x++ )
			{
				m_lutEntry = 0 ;

				for ( int p = 0 ; p < 8 ; ++p )
				{
					m_currentCubeValues[p] = getIsoValue( m_currentGridPos.x+((p^(p>>1))&1), m_currentGridPos.y+((p>>1)&1), m_currentGridPos.z+((p>>2)&1) ) - iso ;

					if ( fabs( m_currentCubeValues[p] ) < Imath::limits<typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType>::epsilon() )
					{
						m_currentCubeValues[p] = Imath::limits<typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType>::epsilon() ;
					}

					if ( m_currentCubeValues[p] > 0 )
					{
						m_lutEntry += 1 << p ;
					}
				}

				processCube( ) ;
			}
		}
	}
}

template< typename ImplicitFn, typename MeshBuilder >
typename MarchingCubes<ImplicitFn, MeshBuilder>::Point MarchingCubes<ImplicitFn, MeshBuilder>::gridToWorld( const typename MarchingCubes<ImplicitFn, MeshBuilder>::PointBaseType i, const typename MarchingCubes<ImplicitFn, MeshBuilder>::PointBaseType j, const typename MarchingCubes<ImplicitFn, MeshBuilder>::PointBaseType k ) const     	      
{										           	      
	const typename MarchingCubes<ImplicitFn, MeshBuilder>::PointBaseType fx = i / m_resolution.x;					           	      
	const typename MarchingCubes<ImplicitFn, MeshBuilder>::PointBaseType fy = j / m_resolution.y;					           	      
	const typename MarchingCubes<ImplicitFn, MeshBuilder>::PointBaseType fz = k / m_resolution.z;					           	      
										           	      
  	return typename MarchingCubes<ImplicitFn, MeshBuilder>::Vector(						           	      
		m_bound.min.x + fx * ( m_bound.max.x - m_bound.min.x ), 	           	      
		m_bound.min.y + fy * ( m_bound.max.y - m_bound.min.y ), 	           	      
		m_bound.min.z + fz * ( m_bound.max.z - m_bound.min.z )		           	      
	);									           	      
}

template< typename ImplicitFn, typename MeshBuilder >
typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType MarchingCubes<ImplicitFn, MeshBuilder>::getIsoValue( const int i, const int j, const int k )
{				
	assert( i >= 0 );
	assert( i < m_resolution.x );
	assert( j >= 0 );
	assert( j < m_resolution.y );
	assert( k >= 0 );
	assert( k < m_resolution.z );

	return m_fn->operator()( gridToWorld( i, j, k ) );
}										           	      

template< typename ImplicitFn, typename MeshBuilder >
void MarchingCubes<ImplicitFn, MeshBuilder>::computeIntersectionPoints( typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType iso )
{
	for ( m_currentGridPos.z = 0 ; m_currentGridPos.z < m_resolution.z ; m_currentGridPos.z++ )
	{
		for ( m_currentGridPos.y = 0 ; m_currentGridPos.y < m_resolution.y ; m_currentGridPos.y++ )
		{
			for ( m_currentGridPos.x = 0 ; m_currentGridPos.x < m_resolution.x ; m_currentGridPos.x++ )
			{
				m_currentCubeValues[0] = getIsoValue( m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z ) - iso ;
				if ( m_currentGridPos.x < m_resolution.x - 1 )
				{
					m_currentCubeValues[1] = getIsoValue(m_currentGridPos.x+1, m_currentGridPos.y, m_currentGridPos.z ) - iso ;
				}
				else
				{
					m_currentCubeValues[1] = m_currentCubeValues[0] ;
				}

				if ( m_currentGridPos.y < m_resolution.y - 1 )
				{
					m_currentCubeValues[3] = getIsoValue( m_currentGridPos.x, m_currentGridPos.y+1, m_currentGridPos.z ) - iso ;
				}
				else
				{
					m_currentCubeValues[3] = m_currentCubeValues[0] ;
				}

				if ( m_currentGridPos.z < m_resolution.z - 1 )
				{
					m_currentCubeValues[4] = getIsoValue( m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z+1) - iso ;
				}
				else
				{
					m_currentCubeValues[4] = m_currentCubeValues[0] ;
				}

				if ( fabs( m_currentCubeValues[0] ) < Imath::limits<typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType>::epsilon() )
				{
					m_currentCubeValues[0] = Imath::limits<typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType>::epsilon() ;
				}
				if ( fabs( m_currentCubeValues[1] ) < Imath::limits<typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType>::epsilon() )
				{
					m_currentCubeValues[1] = Imath::limits<typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType>::epsilon() ;
				}
				if ( fabs( m_currentCubeValues[3] ) < Imath::limits<typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType>::epsilon() )
				{
					m_currentCubeValues[3] = Imath::limits<typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType>::epsilon() ;
				}
				if ( fabs( m_currentCubeValues[4] ) < Imath::limits<typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType>::epsilon() )
				{
					m_currentCubeValues[4] = Imath::limits<typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType>::epsilon() ;
				}

				if ( m_currentCubeValues[0] < 0 )
				{
					if ( m_currentCubeValues[1] > 0 )
					{
						setVertX( addVertexX( ), m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z ) ;
					}
					if ( m_currentCubeValues[3] > 0 )
					{
						setVertY( addVertexY( ), m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z ) ;
					}
					if ( m_currentCubeValues[4] > 0 )
					{
						setVertZ( addVertexZ( ), m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z ) ;
					}
				}
				else
				{
					if ( m_currentCubeValues[1] < 0 )
					{
						setVertX( addVertexX( ), m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z ) ;
					}
					if ( m_currentCubeValues[3] < 0 )
					{
						setVertY( addVertexY( ), m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z ) ;
					}
					if ( m_currentCubeValues[4] < 0 )
					{
						setVertZ( addVertexZ( ), m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z ) ;
					}
				}
			}
		}
	}
}

template< typename ImplicitFn, typename MeshBuilder >
bool MarchingCubes<ImplicitFn, MeshBuilder>::testFace( signed char face )
{
	typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType A,B,C,D ;

	switch ( face )
	{
	case -1 :
	case 1 :
		A = m_currentCubeValues[0] ;
		B = m_currentCubeValues[4] ;
		C = m_currentCubeValues[5] ;
		D = m_currentCubeValues[1] ;
		break ;
	case -2 :
	case 2 :
		A = m_currentCubeValues[1] ;
		B = m_currentCubeValues[5] ;
		C = m_currentCubeValues[6] ;
		D = m_currentCubeValues[2] ;
		break ;
	case -3 :
	case 3 :
		A = m_currentCubeValues[2] ;
		B = m_currentCubeValues[6] ;
		C = m_currentCubeValues[7] ;
		D = m_currentCubeValues[3] ;
		break ;
	case -4 :
	case 4 :
		A = m_currentCubeValues[3] ;
		B = m_currentCubeValues[7] ;
		C = m_currentCubeValues[4] ;
		D = m_currentCubeValues[0] ;
		break ;
	case -5 :
	case 5 :
		A = m_currentCubeValues[0] ;
		B = m_currentCubeValues[3] ;
		C = m_currentCubeValues[2] ;
		D = m_currentCubeValues[1] ;
		break ;
	case -6 :
	case 6 :
		A = m_currentCubeValues[4] ;
		B = m_currentCubeValues[7] ;
		C = m_currentCubeValues[6] ;
		D = m_currentCubeValues[5] ;
		break ;
	default :

		/// Invalid face code
		assert(false);
		A = B = C = D = 0 ;
	};

	if ( fabs( A*C - B*D ) < Imath::limits<typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType>::epsilon() )
	{
		return face >= 0 ;
	}
	return face * A * ( A*C - B*D ) >= 0  ;  // face and A invert signs
}

template< typename ImplicitFn, typename MeshBuilder >
bool MarchingCubes<ImplicitFn, MeshBuilder>::testInterior( signed char s )
{
	typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType t, At=0, Bt=0, Ct=0, Dt=0, a, b ;
	char  test =  0 ;
	char  edge = -1 ; // reference edge of the triangulation

	switch ( m_currentCase )
	{
	case  4 :
	case 10 :
		a = ( m_currentCubeValues[4] - m_currentCubeValues[0] ) * ( m_currentCubeValues[6] - m_currentCubeValues[2] ) - ( m_currentCubeValues[7] - m_currentCubeValues[3] ) * ( m_currentCubeValues[5] - m_currentCubeValues[1] ) ;
		b =  m_currentCubeValues[2] * ( m_currentCubeValues[4] - m_currentCubeValues[0] ) + m_currentCubeValues[0] * ( m_currentCubeValues[6] - m_currentCubeValues[2] )
		     - m_currentCubeValues[1] * ( m_currentCubeValues[7] - m_currentCubeValues[3] ) - m_currentCubeValues[3] * ( m_currentCubeValues[5] - m_currentCubeValues[1] ) ;
		t = - b / (2*a) ;
		if ( t<0 || t>1 )
		{
			return s>0 ;
		}

		At = m_currentCubeValues[0] + ( m_currentCubeValues[4] - m_currentCubeValues[0] ) * t ;
		Bt = m_currentCubeValues[3] + ( m_currentCubeValues[7] - m_currentCubeValues[3] ) * t ;
		Ct = m_currentCubeValues[2] + ( m_currentCubeValues[6] - m_currentCubeValues[2] ) * t ;
		Dt = m_currentCubeValues[1] + ( m_currentCubeValues[5] - m_currentCubeValues[1] ) * t ;
		break ;

	case  6 :
	case  7 :
	case 12 :
	case 13 :
		switch ( m_currentCase )
		{
		case  6 :

			edge = g_test6 [m_currentConfig][2] ;
			break ;
		case  7 :
			edge = g_test7 [m_currentConfig][4] ;
			break ;
		case 12 :
			edge = g_test12[m_currentConfig][3] ;
			break ;
		case 13 :
			edge = g_tiling13_5_1[m_currentConfig][m_currentSubConfig][0] ;
			break ;
		}
		switch ( edge )
		{
		case  0 :
			t  = m_currentCubeValues[0] / ( m_currentCubeValues[0] - m_currentCubeValues[1] ) ;
			At = 0 ;
			Bt = m_currentCubeValues[3] + ( m_currentCubeValues[2] - m_currentCubeValues[3] ) * t ;
			Ct = m_currentCubeValues[7] + ( m_currentCubeValues[6] - m_currentCubeValues[7] ) * t ;
			Dt = m_currentCubeValues[4] + ( m_currentCubeValues[5] - m_currentCubeValues[4] ) * t ;
			break ;
		case  1 :
			t  = m_currentCubeValues[1] / ( m_currentCubeValues[1] - m_currentCubeValues[2] ) ;
			At = 0 ;
			Bt = m_currentCubeValues[0] + ( m_currentCubeValues[3] - m_currentCubeValues[0] ) * t ;
			Ct = m_currentCubeValues[4] + ( m_currentCubeValues[7] - m_currentCubeValues[4] ) * t ;
			Dt = m_currentCubeValues[5] + ( m_currentCubeValues[6] - m_currentCubeValues[5] ) * t ;
			break ;
		case  2 :
			t  = m_currentCubeValues[2] / ( m_currentCubeValues[2] - m_currentCubeValues[3] ) ;
			At = 0 ;
			Bt = m_currentCubeValues[1] + ( m_currentCubeValues[0] - m_currentCubeValues[1] ) * t ;
			Ct = m_currentCubeValues[5] + ( m_currentCubeValues[4] - m_currentCubeValues[5] ) * t ;
			Dt = m_currentCubeValues[6] + ( m_currentCubeValues[7] - m_currentCubeValues[6] ) * t ;
			break ;
		case  3 :
			t  = m_currentCubeValues[3] / ( m_currentCubeValues[3] - m_currentCubeValues[0] ) ;
			At = 0 ;
			Bt = m_currentCubeValues[2] + ( m_currentCubeValues[1] - m_currentCubeValues[2] ) * t ;
			Ct = m_currentCubeValues[6] + ( m_currentCubeValues[5] - m_currentCubeValues[6] ) * t ;
			Dt = m_currentCubeValues[7] + ( m_currentCubeValues[4] - m_currentCubeValues[7] ) * t ;
			break ;
		case  4 :
			t  = m_currentCubeValues[4] / ( m_currentCubeValues[4] - m_currentCubeValues[5] ) ;
			At = 0 ;
			Bt = m_currentCubeValues[7] + ( m_currentCubeValues[6] - m_currentCubeValues[7] ) * t ;
			Ct = m_currentCubeValues[3] + ( m_currentCubeValues[2] - m_currentCubeValues[3] ) * t ;
			Dt = m_currentCubeValues[0] + ( m_currentCubeValues[1] - m_currentCubeValues[0] ) * t ;
			break ;
		case  5 :
			t  = m_currentCubeValues[5] / ( m_currentCubeValues[5] - m_currentCubeValues[6] ) ;
			At = 0 ;
			Bt = m_currentCubeValues[4] + ( m_currentCubeValues[7] - m_currentCubeValues[4] ) * t ;
			Ct = m_currentCubeValues[0] + ( m_currentCubeValues[3] - m_currentCubeValues[0] ) * t ;
			Dt = m_currentCubeValues[1] + ( m_currentCubeValues[2] - m_currentCubeValues[1] ) * t ;
			break ;
		case  6 :
			t  = m_currentCubeValues[6] / ( m_currentCubeValues[6] - m_currentCubeValues[7] ) ;
			At = 0 ;
			Bt = m_currentCubeValues[5] + ( m_currentCubeValues[4] - m_currentCubeValues[5] ) * t ;
			Ct = m_currentCubeValues[1] + ( m_currentCubeValues[0] - m_currentCubeValues[1] ) * t ;
			Dt = m_currentCubeValues[2] + ( m_currentCubeValues[3] - m_currentCubeValues[2] ) * t ;
			break ;
		case  7 :
			t  = m_currentCubeValues[7] / ( m_currentCubeValues[7] - m_currentCubeValues[4] ) ;
			At = 0 ;
			Bt = m_currentCubeValues[6] + ( m_currentCubeValues[5] - m_currentCubeValues[6] ) * t ;
			Ct = m_currentCubeValues[2] + ( m_currentCubeValues[1] - m_currentCubeValues[2] ) * t ;
			Dt = m_currentCubeValues[3] + ( m_currentCubeValues[0] - m_currentCubeValues[3] ) * t ;
			break ;
		case  8 :
			t  = m_currentCubeValues[0] / ( m_currentCubeValues[0] - m_currentCubeValues[4] ) ;
			At = 0 ;
			Bt = m_currentCubeValues[3] + ( m_currentCubeValues[7] - m_currentCubeValues[3] ) * t ;
			Ct = m_currentCubeValues[2] + ( m_currentCubeValues[6] - m_currentCubeValues[2] ) * t ;
			Dt = m_currentCubeValues[1] + ( m_currentCubeValues[5] - m_currentCubeValues[1] ) * t ;
			break ;
		case  9 :
			t  = m_currentCubeValues[1] / ( m_currentCubeValues[1] - m_currentCubeValues[5] ) ;
			At = 0 ;
			Bt = m_currentCubeValues[0] + ( m_currentCubeValues[4] - m_currentCubeValues[0] ) * t ;
			Ct = m_currentCubeValues[3] + ( m_currentCubeValues[7] - m_currentCubeValues[3] ) * t ;
			Dt = m_currentCubeValues[2] + ( m_currentCubeValues[6] - m_currentCubeValues[2] ) * t ;
			break ;
		case 10 :
			t  = m_currentCubeValues[2] / ( m_currentCubeValues[2] - m_currentCubeValues[6] ) ;
			At = 0 ;
			Bt = m_currentCubeValues[1] + ( m_currentCubeValues[5] - m_currentCubeValues[1] ) * t ;
			Ct = m_currentCubeValues[0] + ( m_currentCubeValues[4] - m_currentCubeValues[0] ) * t ;
			Dt = m_currentCubeValues[3] + ( m_currentCubeValues[7] - m_currentCubeValues[3] ) * t ;
			break ;
		case 11 :
			t  = m_currentCubeValues[3] / ( m_currentCubeValues[3] - m_currentCubeValues[7] ) ;
			At = 0 ;
			Bt = m_currentCubeValues[2] + ( m_currentCubeValues[6] - m_currentCubeValues[2] ) * t ;
			Ct = m_currentCubeValues[1] + ( m_currentCubeValues[5] - m_currentCubeValues[1] ) * t ;
			Dt = m_currentCubeValues[0] + ( m_currentCubeValues[4] - m_currentCubeValues[0] ) * t ;
			break ;
		default :

			/// Invalid edge
			assert(false);
			break;
		}
		break ;

	default :

		/// Invalid ambiguous case
		assert(false);
	}

	if ( At >= 0 ) test ++ ;
	if ( Bt >= 0 ) test += 2 ;
	if ( Ct >= 0 ) test += 4 ;
	if ( Dt >= 0 ) test += 8 ;
	switch ( test )
	{
	case  0 :
		return s>0 ;
	case  1 :
		return s>0 ;
	case  2 :
		return s>0 ;
	case  3 :
		return s>0 ;
	case  4 :
		return s>0 ;
	case  5 :
		if ( At * Ct - Bt * Dt <  Imath::limits<typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType>::epsilon() )
		{
			return s>0 ;
		}
		break ;
	case  6 :
		return s>0 ;
	case  7 :
		return s<0 ;
	case  8 :
		return s>0 ;
	case  9 :
		return s>0 ;
	case 10 :
		if ( At * Ct - Bt * Dt >= Imath::limits<typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType>::epsilon() )
		{
			return s>0 ;
		}
		break ;
	case 11 :
		return s<0 ;
	case 12 :
		return s>0 ;
	case 13 :
		return s<0 ;
	case 14 :
		return s<0 ;
	case 15 :
		return s<0 ;
	}

	return s<0 ;
}

template< typename ImplicitFn, typename MeshBuilder >
void MarchingCubes<ImplicitFn, MeshBuilder>::processCube( )
{
	int v12 = -1 ;
	m_currentCase   = g_cases[m_lutEntry][0] ;
	m_currentConfig = g_cases[m_lutEntry][1] ;
	m_currentSubConfig = 0 ;
	
	switch ( m_currentCase )
	{
	case  0 :
		break ;

	case  1 :
		addTriangle( g_tiling1[m_currentConfig], 1 ) ;
		break ;

	case  2 :
		addTriangle( g_tiling2[m_currentConfig], 2 ) ;
		break ;

	case  3 :
		if ( testFace( g_test3[m_currentConfig]) )
		{
			addTriangle( g_tiling3_2[m_currentConfig], 4 ) ; // 3.2
		}
		else
		{
			addTriangle( g_tiling3_1[m_currentConfig], 2 ) ; // 3.1
		}
		break ;

	case  4 :
		if ( testInterior( g_test4[m_currentConfig]) )
		{
			addTriangle( g_tiling4_1[m_currentConfig], 2 ) ; // 4.1.1
		}
		else
		{
			addTriangle( g_tiling4_2[m_currentConfig], 6 ) ; // 4.1.2
		}
		break ;

	case  5 :
		addTriangle( g_tiling5[m_currentConfig], 3 ) ;
		break ;

	case  6 :
		if ( testFace( g_test6[m_currentConfig][0]) )
		{
			addTriangle( g_tiling6_2[m_currentConfig], 5 ) ; // 6.2
		}
		else
		{
			{
				if ( testInterior( g_test6[m_currentConfig][1]) )
				{
					addTriangle( g_tiling6_1_1[m_currentConfig], 3 ) ; // 6.1.1
				}
				else
				{
					addTriangle( g_tiling6_1_2[m_currentConfig], 7 ) ; // 6.1.2
				}
			}
		}
		break ;

	case  7 :
		if ( testFace( g_test7[m_currentConfig][0] ) )
		{
			m_currentSubConfig +=  1 ;
		}
		if ( testFace( g_test7[m_currentConfig][1] ) )
		{
			m_currentSubConfig +=  2 ;
		}
		if ( testFace( g_test7[m_currentConfig][2] ) )
		{
			m_currentSubConfig +=  4 ;
		}
		switch ( m_currentSubConfig )
		{
		case 0 :
			addTriangle( g_tiling7_1[m_currentConfig], 3 ) ;
			break ;
		case 1 :
			addTriangle( g_tiling7_2[m_currentConfig][0], 5 ) ;
			break ;
		case 2 :
			addTriangle( g_tiling7_2[m_currentConfig][1], 5 ) ;
			break ;
		case 3 :
			v12 = addVertexC() ;
			addTriangle( g_tiling7_3[m_currentConfig][0], 9, v12 ) ;
			break ;
		case 4 :
			addTriangle( g_tiling7_2[m_currentConfig][2], 5 ) ;
			break ;
		case 5 :
			v12 = addVertexC() ;
			addTriangle( g_tiling7_3[m_currentConfig][1], 9, v12 ) ;
			break ;
		case 6 :
			v12 = addVertexC() ;
			addTriangle( g_tiling7_3[m_currentConfig][2], 9, v12 ) ;
			break ;
		case 7 :
			if ( testInterior( g_test7[m_currentConfig][3]) )
			{
				addTriangle( g_tiling7_4_2[m_currentConfig], 9 ) ;
			}
			else
			{
				addTriangle( g_tiling7_4_1[m_currentConfig], 5 ) ;
			}
			break ;
		};
		break ;

	case  8 :
		addTriangle( g_tiling8[m_currentConfig], 2 ) ;
		break ;

	case  9 :
		addTriangle( g_tiling9[m_currentConfig], 4 ) ;
		break ;

	case 10 :
		if ( testFace( g_test10[m_currentConfig][0]) )
		{
			if ( testFace( g_test10[m_currentConfig][1]) )
				addTriangle( g_tiling10_1_1_[m_currentConfig], 4 ) ; // 10.1.1
			else
			{
				v12 = addVertexC() ;
				addTriangle( g_tiling10_2[m_currentConfig], 8, v12 ) ; // 10.2
			}
		}
		else
		{
			if ( testFace( g_test10[m_currentConfig][1]) )
			{
				v12 = addVertexC() ;
				addTriangle( g_tiling10_2_[m_currentConfig], 8, v12 ) ; // 10.2
			}
			else
			{
				if ( testInterior( g_test10[m_currentConfig][2]) )
				{
					addTriangle( g_tiling10_1_1[m_currentConfig], 4 ) ; // 10.1.1
				}
				else
				{
					addTriangle( g_tiling10_1_2[m_currentConfig], 8 ) ; // 10.1.2
				}
			}
		}
		break ;

	case 11 :
		addTriangle( g_tiling11[m_currentConfig], 4 ) ;
		break ;

	case 12 :
		if ( testFace( g_test12[m_currentConfig][0]) )
		{
			if ( testFace( g_test12[m_currentConfig][1]) )
				addTriangle( g_tiling12_1_1_[m_currentConfig], 4 ) ; // 12.1.1
			else
			{
				v12 = addVertexC() ;
				addTriangle( g_tiling12_2[m_currentConfig], 8, v12 ) ; // 12.2
			}
		}
		else
		{
			if ( testFace( g_test12[m_currentConfig][1]) )
			{
				v12 = addVertexC() ;
				addTriangle( g_tiling12_2_[m_currentConfig], 8, v12 ) ; // 12.2
			}
			else
			{
				if ( testInterior( g_test12[m_currentConfig][2]) )
				{
					addTriangle( g_tiling12_1_1[m_currentConfig], 4 ) ; // 12.1.1
				}
				else
				{
					addTriangle( g_tiling12_1_2[m_currentConfig], 8 ) ; // 12.1.2
				}
			}
		}
		break ;

	case 13 :
		if ( testFace( g_test13[m_currentConfig][0] ) )
		{
			m_currentSubConfig +=  1 ;
		}
		if ( testFace( g_test13[m_currentConfig][1] ) )
		{
			m_currentSubConfig +=  2 ;
		}
		if ( testFace( g_test13[m_currentConfig][2] ) )
		{
			m_currentSubConfig +=  4 ;
		}
		if ( testFace( g_test13[m_currentConfig][3] ) )
		{
			m_currentSubConfig +=  8 ;
		}
		if ( testFace( g_test13[m_currentConfig][4] ) )
		{
			m_currentSubConfig += 16 ;
		}
		if ( testFace( g_test13[m_currentConfig][5] ) )
		{
			m_currentSubConfig += 32 ;
		}
		switch ( g_subconfig13[m_currentSubConfig] )
		{
		case 0 :/* 13.1 */
			addTriangle( g_tiling13_1[m_currentConfig], 4 ) ;
			break ;

		case 1 :/* 13.2 */
			addTriangle( g_tiling13_2[m_currentConfig][0], 6 ) ;
			break ;
		case 2 :/* 13.2 */
			addTriangle( g_tiling13_2[m_currentConfig][1], 6 ) ;
			break ;
		case 3 :/* 13.2 */
			addTriangle( g_tiling13_2[m_currentConfig][2], 6 ) ;
			break ;
		case 4 :/* 13.2 */
			addTriangle( g_tiling13_2[m_currentConfig][3], 6 ) ;
			break ;
		case 5 :/* 13.2 */
			addTriangle( g_tiling13_2[m_currentConfig][4], 6 ) ;
			break ;
		case 6 :/* 13.2 */
			addTriangle( g_tiling13_2[m_currentConfig][5], 6 ) ;
			break ;

		case 7 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3[m_currentConfig][0], 10, v12 ) ;
			break ;
		case 8 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3[m_currentConfig][1], 10, v12 ) ;
			break ;
		case 9 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3[m_currentConfig][2], 10, v12 ) ;
			break ;
		case 10 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3[m_currentConfig][3], 10, v12 ) ;
			break ;
		case 11 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3[m_currentConfig][4], 10, v12 ) ;
			break ;
		case 12 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3[m_currentConfig][5], 10, v12 ) ;
			break ;
		case 13 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3[m_currentConfig][6], 10, v12 ) ;
			break ;
		case 14 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3[m_currentConfig][7], 10, v12 ) ;
			break ;
		case 15 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3[m_currentConfig][8], 10, v12 ) ;
			break ;
		case 16 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3[m_currentConfig][9], 10, v12 ) ;
			break ;
		case 17 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3[m_currentConfig][10], 10, v12 ) ;
			break ;
		case 18 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3[m_currentConfig][11], 10, v12 ) ;
			break ;

		case 19 :/* 13.4 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_4[m_currentConfig][0], 12, v12 ) ;
			break ;
		case 20 :/* 13.4 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_4[m_currentConfig][1], 12, v12 ) ;
			break ;
		case 21 :/* 13.4 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_4[m_currentConfig][2], 12, v12 ) ;
			break ;
		case 22 :/* 13.4 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_4[m_currentConfig][3], 12, v12 ) ;
			break ;

		case 23 :/* 13.5 */
			m_currentSubConfig = 0 ;
			if ( testInterior( g_test13[m_currentConfig][6] ) )
			{
				addTriangle( g_tiling13_5_1[m_currentConfig][0], 6 ) ;
			}
			else
			{
				addTriangle( g_tiling13_5_2[m_currentConfig][0], 10 ) ;
			}
			break ;
		case 24 :/* 13.5 */
			m_currentSubConfig = 1 ;
			if ( testInterior( g_test13[m_currentConfig][6] ) )
			{
				addTriangle( g_tiling13_5_1[m_currentConfig][1], 6 ) ;
			}
			else
			{
				addTriangle( g_tiling13_5_2[m_currentConfig][1], 10 ) ;
			}
			break ;
		case 25 :/* 13.5 */
			m_currentSubConfig = 2 ;
			if ( testInterior( g_test13[m_currentConfig][6] ) )
			{
				addTriangle( g_tiling13_5_1[m_currentConfig][2], 6 ) ;
			}
			else
			{
				addTriangle( g_tiling13_5_2[m_currentConfig][2], 10 ) ;
			}
			break ;
		case 26 :/* 13.5 */
			m_currentSubConfig = 3 ;
			if ( testInterior( g_test13[m_currentConfig][6] ) )
			{
				addTriangle( g_tiling13_5_1[m_currentConfig][3], 6 ) ;
			}
			else
			{
				addTriangle( g_tiling13_5_2[m_currentConfig][3], 10 ) ;
			}
			break ;

		case 27 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3_[m_currentConfig][0], 10, v12 ) ;
			break ;
		case 28 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3_[m_currentConfig][1], 10, v12 ) ;
			break ;
		case 29 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3_[m_currentConfig][2], 10, v12 ) ;
			break ;
		case 30 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3_[m_currentConfig][3], 10, v12 ) ;
			break ;
		case 31 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3_[m_currentConfig][4], 10, v12 ) ;
			break ;
		case 32 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3_[m_currentConfig][5], 10, v12 ) ;
			break ;
		case 33 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3_[m_currentConfig][6], 10, v12 ) ;
			break ;
		case 34 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3_[m_currentConfig][7], 10, v12 ) ;
			break ;
		case 35 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3_[m_currentConfig][8], 10, v12 ) ;
			break ;
		case 36 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3_[m_currentConfig][9], 10, v12 ) ;
			break ;
		case 37 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3_[m_currentConfig][10], 10, v12 ) ;
			break ;
		case 38 :/* 13.3 */
			v12 = addVertexC() ;
			addTriangle( g_tiling13_3_[m_currentConfig][11], 10, v12 ) ;
			break ;

		case 39 :/* 13.2 */
			addTriangle( g_tiling13_2_[m_currentConfig][0], 6 ) ;
			break ;
		case 40 :/* 13.2 */
			addTriangle( g_tiling13_2_[m_currentConfig][1], 6 ) ;
			break ;
		case 41 :/* 13.2 */
			addTriangle( g_tiling13_2_[m_currentConfig][2], 6 ) ;
			break ;
		case 42 :/* 13.2 */
			addTriangle( g_tiling13_2_[m_currentConfig][3], 6 ) ;
			break ;
		case 43 :/* 13.2 */
			addTriangle( g_tiling13_2_[m_currentConfig][4], 6 ) ;
			break ;
		case 44 :/* 13.2 */
			addTriangle( g_tiling13_2_[m_currentConfig][5], 6 ) ;
			break ;

		case 45 :/* 13.1 */
			addTriangle( g_tiling13_1_[m_currentConfig], 4 ) ;
			break ;

		default :
			// Impossible case 13?
			assert(false);
		}
		break ;

	case 14 :
		addTriangle( g_tiling14[m_currentConfig], 4 ) ;
		break ;
	};
}

template< typename ImplicitFn, typename MeshBuilder >
void MarchingCubes<ImplicitFn, MeshBuilder>::addTriangle( const char* trig, char n, int v12 )
{
	assert( trig );

	int tv[3] ;

	for ( int t = 0 ; t < 3*n ; t++ )
	{
		switch ( trig[t] )
		{
		case  0 :
			tv[ t % 3 ] = getVertX( m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z ) ;
			break ;
		case  1 :
			tv[ t % 3 ] = getVertY(m_currentGridPos.x+1, m_currentGridPos.y, m_currentGridPos.z ) ;
			break ;
		case  2 :
			tv[ t % 3 ] = getVertX( m_currentGridPos.x, m_currentGridPos.y+1, m_currentGridPos.z ) ;
			break ;
		case  3 :
			tv[ t % 3 ] = getVertY( m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z ) ;
			break ;
		case  4 :
			tv[ t % 3 ] = getVertX( m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z+1) ;
			break ;
		case  5 :
			tv[ t % 3 ] = getVertY(m_currentGridPos.x+1, m_currentGridPos.y, m_currentGridPos.z+1) ;
			break ;
		case  6 :
			tv[ t % 3 ] = getVertX( m_currentGridPos.x, m_currentGridPos.y+1, m_currentGridPos.z+1) ;
			break ;
		case  7 :
			tv[ t % 3 ] = getVertY( m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z+1) ;
			break ;
		case  8 :
			tv[ t % 3 ] = getVertZ( m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z ) ;
			break ;
		case  9 :
			tv[ t % 3 ] = getVertZ(m_currentGridPos.x+1, m_currentGridPos.y, m_currentGridPos.z ) ;
			break ;
		case 10 :
			tv[ t % 3 ] = getVertZ(m_currentGridPos.x+1, m_currentGridPos.y+1, m_currentGridPos.z ) ;
			break ;
		case 11 :
			tv[ t % 3 ] = getVertZ( m_currentGridPos.x, m_currentGridPos.y+1, m_currentGridPos.z ) ;
			break ;
		case 12 :
			tv[ t % 3 ] = v12 ;
			break ;
		default :
			break ;
		}

		assert( tv[t%3] != -1 );

		if ( t%3 == 2 )
		{
			m_builder->addTriangle(tv[0], tv[1], tv[2] );
		}
	}
}

template< typename ImplicitFn, typename MeshBuilder >
typename MarchingCubes<ImplicitFn, MeshBuilder>::Vector MarchingCubes<ImplicitFn, MeshBuilder>::getGradient( const int i, const int j, const int k )
{
	assert( i >= 0 );
	assert( i < m_resolution.x );
	assert( j >= 0 );
	assert( j < m_resolution.y);
	assert( k >= 0 );
	assert( k < m_resolution.z );

	return typename MarchingCubes<ImplicitFn, MeshBuilder>::Vector(
	           getGradientX( i, j, k ),
	           getGradientY( i, j, k ),
	           getGradientZ( i, j, k )
	       );
}

template< typename ImplicitFn, typename MeshBuilder >
typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType MarchingCubes<ImplicitFn, MeshBuilder>::getGradientX( const int i, const int j, const int k )
{
	assert( i >= 0 );
	assert( i < m_resolution.x );
	assert( j >= 0 );
	assert( j < m_resolution.y);
	assert( k >= 0 );
	assert( k < m_resolution.z );

	if ( i > 0 )
	{
		if ( i < m_resolution.x - 1 )
		{
			return ( getIsoValue( i+1, j, k ) - getIsoValue( i-1, j, k ) ) / 2 ;
		}
		else
		{
			return getIsoValue( i, j, k ) - getIsoValue( i-1, j, k ) ;
		}
	}
	else
	{
		return getIsoValue( i+1, j, k ) - getIsoValue( i, j, k ) ;
	}
}

template< typename ImplicitFn, typename MeshBuilder >
typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType MarchingCubes<ImplicitFn, MeshBuilder>::getGradientY( const int i, const int j, const int k )
{
	assert( i >= 0 );
	assert( i < m_resolution.x );
	assert( j >= 0 );
	assert( j < m_resolution.y);
	assert( k >= 0 );
	assert( k < m_resolution.z );

	if ( j > 0 )
	{
		if ( j < m_resolution.y - 1 )
		{
			return ( getIsoValue( i, j+1, k ) - getIsoValue( i, j-1, k ) ) / 2 ;
		}
		else
		{
			return getIsoValue( i, j, k ) - getIsoValue( i, j-1, k ) ;
		}
	}
	else
	{
		return getIsoValue( i, j+1, k ) - getIsoValue( i, j, k ) ;
	}
}

template< typename ImplicitFn, typename MeshBuilder >
typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType MarchingCubes<ImplicitFn, MeshBuilder>::getGradientZ( const int i, const int j, const int k )
{
	assert( i >= 0 );
	assert( i < m_resolution.x );
	assert( j >= 0 );
	assert( j < m_resolution.y);
	assert( k >= 0 );
	assert( k < m_resolution.z );

	if ( k > 0 )
	{
		if ( k < m_resolution.z - 1 )
		{
			return ( getIsoValue( i, j, k+1 ) - getIsoValue( i, j, k-1 ) ) / 2 ;
		}
		else
		{
			return getIsoValue( i, j, k ) - getIsoValue( i, j, k-1 ) ;
		}
	}
	else
	{
		return getIsoValue( i, j, k+1 ) - getIsoValue( i, j, k ) ;
	}
}

template< typename ImplicitFn, typename MeshBuilder >
int MarchingCubes<ImplicitFn, MeshBuilder>::getVertX( const int i, const int j, const int k ) const
{
	return m_verts->readable()[ i + j*m_resolution.x + k*m_resolution.x*m_resolution.y].x ;
}

template< typename ImplicitFn, typename MeshBuilder >
int MarchingCubes<ImplicitFn, MeshBuilder>::getVertY( const int i, const int j, const int k ) const
{
	return m_verts->readable()[ i + j*m_resolution.x + k*m_resolution.x*m_resolution.y].y ;
}

template< typename ImplicitFn, typename MeshBuilder >
int MarchingCubes<ImplicitFn, MeshBuilder>::getVertZ( const int i, const int j, const int k ) const
{
	return m_verts->readable()[ i + j*m_resolution.x + k*m_resolution.x*m_resolution.y].z ;
}

template< typename ImplicitFn, typename MeshBuilder >
void MarchingCubes<ImplicitFn, MeshBuilder>::setVertX( const int val, const int i, const int j, const int k )
{
	m_verts->writable()[ i + j*m_resolution.x + k*m_resolution.x*m_resolution.y].x = val ;
}

template< typename ImplicitFn, typename MeshBuilder >
void MarchingCubes<ImplicitFn, MeshBuilder>::setVertY( const int val, const int i, const int j, const int k )
{
	m_verts->writable()[ i + j*m_resolution.x + k*m_resolution.x*m_resolution.y].y = val ;
}

template< typename ImplicitFn, typename MeshBuilder >
void MarchingCubes<ImplicitFn, MeshBuilder>::setVertZ( const int val, const int i, const int j, const int k )
{
	m_verts->writable()[ i + j*m_resolution.x + k*m_resolution.x*m_resolution.y].z = val ;
}

template< typename ImplicitFn, typename MeshBuilder >
int MarchingCubes<ImplicitFn, MeshBuilder>::addVertexX( )
{
	typedef typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType ValueBaseType;
	ValueBaseType u = ( m_currentCubeValues[0] ) / ( m_currentCubeValues[0] - m_currentCubeValues[1] ) ;
	
	assert( u >=   -Imath::limits<ValueBaseType>::epsilon() );
	assert( u <= 1.+Imath::limits<ValueBaseType>::epsilon() );

	typename MarchingCubes<ImplicitFn, MeshBuilder>::Vector p = gridToWorld( m_currentGridPos.x+u, m_currentGridPos.y, m_currentGridPos.z );
	typename MarchingCubes<ImplicitFn, MeshBuilder>::Vector n = getGradient(m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z)*(1-u) + getGradient(m_currentGridPos.x+1, m_currentGridPos.y, m_currentGridPos.z) *u;

	m_builder->addVertex( p, n );
	
	m_P->writable().push_back( p );
	m_N->writable().push_back( n );		
	
	return m_numVerts++;
}

template< typename ImplicitFn, typename MeshBuilder >
int MarchingCubes<ImplicitFn, MeshBuilder>::addVertexY( )
{
	typedef typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType ValueBaseType;
	ValueBaseType u = ( m_currentCubeValues[0] ) / ( m_currentCubeValues[0] - m_currentCubeValues[3] ) ;
	
	assert( u >=   -Imath::limits<ValueBaseType>::epsilon() );
	assert( u <= 1.+Imath::limits<ValueBaseType>::epsilon() );
	
	typename MarchingCubes<ImplicitFn, MeshBuilder>::Vector p = gridToWorld( m_currentGridPos.x, m_currentGridPos.y+u, m_currentGridPos.z );
	typename MarchingCubes<ImplicitFn, MeshBuilder>::Vector n = getGradient(m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z)*(1-u) + getGradient(m_currentGridPos.x, m_currentGridPos.y+1, m_currentGridPos.z)*u ;

	m_builder->addVertex( p, n );
	
	m_P->writable().push_back( p );
	m_N->writable().push_back( n );	
	
	return m_numVerts++;
}

template< typename ImplicitFn, typename MeshBuilder >
int MarchingCubes<ImplicitFn, MeshBuilder>::addVertexZ( )
{
	typedef typename MarchingCubes<ImplicitFn, MeshBuilder>::ValueBaseType ValueBaseType;
	ValueBaseType u = ( m_currentCubeValues[0] ) / ( m_currentCubeValues[0] - m_currentCubeValues[4] ) ;

	assert( u >=   -Imath::limits<ValueBaseType>::epsilon() );
	assert( u <= 1.+Imath::limits<ValueBaseType>::epsilon() );
	
	typename MarchingCubes<ImplicitFn, MeshBuilder>::Vector p = gridToWorld( m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z+u );
	typename MarchingCubes<ImplicitFn, MeshBuilder>::Vector n = getGradient(m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z)*(1-u) + getGradient(m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z+1) * u;

	m_builder->addVertex( p, n );
	
	m_P->writable().push_back( p );
	m_N->writable().push_back( n );	
	
	return m_numVerts++;
}

template< typename ImplicitFn, typename MeshBuilder >
int MarchingCubes<ImplicitFn, MeshBuilder>::addVertexC( )
{
	typename MarchingCubes<ImplicitFn, MeshBuilder>::PointBaseType u = 0.0 ;

	typename MarchingCubes<ImplicitFn, MeshBuilder>::Vector p(0.0, 0.0, 0.0), n(0.0,0.0,0.0);
	const typename V3xVectorData::ValueType &P = m_P->readable();
	const typename V3xVectorData::ValueType &N = m_N->readable();
	
	// Computes the average of the intersection points of the cube
	int vid = getVertX( m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z ) ;
	if ( vid != -1 )
	{
		++u ;
		p += P[vid];
		n += N[vid];
	}
	vid = getVertY(m_currentGridPos.x+1, m_currentGridPos.y, m_currentGridPos.z ) ;
	if ( vid != -1 )
	{
		++u ;
		p += P[vid];
		n += N[vid];
	}
	vid = getVertX( m_currentGridPos.x, m_currentGridPos.y+1, m_currentGridPos.z ) ;
	if ( vid != -1 )
	{
		++u ;
		p += P[vid];
		n += N[vid];
	}
	vid = getVertY( m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z ) ;
	if ( vid != -1 )
	{
		++u ;
		p += P[vid];
		n += N[vid];
	}
	vid = getVertX( m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z+1) ;
	if ( vid != -1 )
	{
		++u ;
		p += P[vid];
		n += N[vid];
	}
	vid = getVertY(m_currentGridPos.x+1, m_currentGridPos.y, m_currentGridPos.z+1) ;
	if ( vid != -1 )
	{
		++u ;
		p += P[vid];
		n += N[vid];
	}
	vid = getVertX( m_currentGridPos.x, m_currentGridPos.y+1, m_currentGridPos.z+1) ;
	if ( vid != -1 )
	{
		++u ;
		p += P[vid];
		n += N[vid];
	}
	vid = getVertY( m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z+1) ;
	if ( vid != -1 )
	{
		++u ;
		p += P[vid];
		n += N[vid];
	}
	vid = getVertZ( m_currentGridPos.x, m_currentGridPos.y, m_currentGridPos.z ) ;
	if ( vid != -1 )
	{
		++u ;
		p += P[vid];
		n += N[vid];
	}
	vid = getVertZ(m_currentGridPos.x+1, m_currentGridPos.y, m_currentGridPos.z ) ;
	if ( vid != -1 )
	{
		++u ;
		p += P[vid];
		n += N[vid];
	}
	vid = getVertZ(m_currentGridPos.x+1, m_currentGridPos.y+1, m_currentGridPos.z ) ;
	if ( vid != -1 )
	{
		++u ;
		p += P[vid];
		n += N[vid];
	}
	vid = getVertZ( m_currentGridPos.x, m_currentGridPos.y+1, m_currentGridPos.z ) ;
	if ( vid != -1 )
	{
		++u ;
		p += P[vid];
		n += N[vid];
	}

	assert( u > 0.0 );
	p /= u;
	n /= u;

	m_builder->addVertex( p, n );
	
	m_P->writable().push_back( p );
	m_N->writable().push_back( n );		
	
	return m_numVerts++;
}

template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_cases[256][2] =
{
	{ 0, -1 },
	{ 1, 0 },
	{ 1, 1 },
	{ 2, 0 },
	{ 1, 2 },
	{ 3, 0 },
	{ 2, 3 },
	{ 5, 0 },
	{ 1, 3 },
	{ 2, 1 },
	{ 3, 3 },
	{ 5, 1 },
	{ 2, 5 },
	{ 5, 4 },
	{ 5, 9 },
	{ 8, 0 },
	{ 1, 4 },
	{ 2, 2 },
	{ 3, 4 },
	{ 5, 2 },
	{ 4, 2 },
	{ 6, 2 },
	{ 6, 9 },
	{ 11, 0 },
	{ 3, 8 },
	{ 5, 5 },
	{ 7, 3 },
	{ 9, 1 },
	{ 6, 16 },
	{ 14, 3 },
	{ 12, 12 },
	{ 5, 24 },
	{ 1, 5 },
	{ 3, 1 },
	{ 2, 4 },
	{ 5, 3 },
	{ 3, 6 },
	{ 7, 0 },
	{ 5, 10 },
	{ 9, 0 },
	{ 4, 3 },
	{ 6, 4 },
	{ 6, 11 },
	{ 14, 1 },
	{ 6, 17 },
	{ 12, 4 },
	{ 11, 6 },
	{ 5, 25 },
	{ 2, 8 },
	{ 5, 7 },
	{ 5, 12 },
	{ 8, 1 },
	{ 6, 18 },
	{ 12, 5 },
	{ 14, 7 },
	{ 5, 28 },
	{ 6, 21 },
	{ 11, 4 },
	{ 12, 15 },
	{ 5, 30 },
	{ 10, 5 },
	{ 6, 32 },
	{ 6, 39 },
	{ 2, 12 },
	{ 1, 6 },
	{ 4, 0 },
	{ 3, 5 },
	{ 6, 0 },
	{ 2, 6 },
	{ 6, 3 },
	{ 5, 11 },
	{ 14, 0 },
	{ 3, 9 },
	{ 6, 5 },
	{ 7, 4 },
	{ 12, 1 },
	{ 5, 14 },
	{ 11, 3 },
	{ 9, 4 },
	{ 5, 26 },
	{ 3, 10 },
	{ 6, 6 },
	{ 7, 5 },
	{ 12, 2 },
	{ 6, 19 },
	{ 10, 1 },
	{ 12, 13 },
	{ 6, 24 },
	{ 7, 7 },
	{ 12, 9 },
	{ 13, 1 },
	{ 7, 9 },
	{ 12, 20 },
	{ 6, 33 },
	{ 7, 13 },
	{ 3, 12 },
	{ 2, 10 },
	{ 6, 7 },
	{ 5, 13 },
	{ 11, 2 },
	{ 5, 16 },
	{ 12, 7 },
	{ 8, 3 },
	{ 5, 29 },
	{ 6, 22 },
	{ 10, 2 },
	{ 12, 17 },
	{ 6, 27 },
	{ 14, 9 },
	{ 6, 34 },
	{ 5, 39 },
	{ 2, 14 },
	{ 5, 20 },
	{ 14, 5 },
	{ 9, 5 },
	{ 5, 32 },
	{ 11, 10 },
	{ 6, 35 },
	{ 5, 41 },
	{ 2, 16 },
	{ 12, 23 },
	{ 6, 37 },
	{ 7, 14 },
	{ 3, 16 },
	{ 6, 46 },
	{ 4, 6 },
	{ 3, 21 },
	{ 1, 8 },
	{ 1, 7 },
	{ 3, 2 },
	{ 4, 1 },
	{ 6, 1 },
	{ 3, 7 },
	{ 7, 1 },
	{ 6, 10 },
	{ 12, 0 },
	{ 2, 7 },
	{ 5, 6 },
	{ 6, 12 },
	{ 11, 1 },
	{ 5, 15 },
	{ 9, 2 },
	{ 14, 6 },
	{ 5, 27 },
	{ 2, 9 },
	{ 5, 8 },
	{ 6, 13 },
	{ 14, 2 },
	{ 6, 20 },
	{ 12, 6 },
	{ 10, 3 },
	{ 6, 25 },
	{ 5, 18 },
	{ 8, 2 },
	{ 12, 16 },
	{ 5, 31 },
	{ 11, 9 },
	{ 5, 34 },
	{ 6, 40 },
	{ 2, 13 },
	{ 3, 11 },
	{ 7, 2 },
	{ 6, 14 },
	{ 12, 3 },
	{ 7, 6 },
	{ 13, 0 },
	{ 12, 14 },
	{ 7, 8 },
	{ 6, 23 },
	{ 12, 10 },
	{ 10, 4 },
	{ 6, 28 },
	{ 12, 21 },
	{ 7, 10 },
	{ 6, 41 },
	{ 3, 13 },
	{ 5, 21 },
	{ 9, 3 },
	{ 11, 8 },
	{ 5, 33 },
	{ 12, 22 },
	{ 7, 11 },
	{ 6, 42 },
	{ 3, 14 },
	{ 14, 11 },
	{ 5, 36 },
	{ 6, 44 },
	{ 2, 17 },
	{ 6, 47 },
	{ 3, 18 },
	{ 4, 7 },
	{ 1, 9 },
	{ 2, 11 },
	{ 6, 8 },
	{ 6, 15 },
	{ 10, 0 },
	{ 5, 17 },
	{ 12, 8 },
	{ 11, 7 },
	{ 6, 26 },
	{ 5, 19 },
	{ 14, 4 },
	{ 12, 18 },
	{ 6, 29 },
	{ 8, 4 },
	{ 5, 35 },
	{ 5, 40 },
	{ 2, 15 },
	{ 5, 22 },
	{ 11, 5 },
	{ 12, 19 },
	{ 6, 30 },
	{ 14, 10 },
	{ 6, 36 },
	{ 6, 43 },
	{ 4, 4 },
	{ 9, 7 },
	{ 5, 37 },
	{ 7, 15 },
	{ 3, 17 },
	{ 5, 44 },
	{ 2, 19 },
	{ 3, 22 },
	{ 1, 10 },
	{ 5, 23 },
	{ 12, 11 },
	{ 14, 8 },
	{ 6, 31 },
	{ 9, 6 },
	{ 7, 12 },
	{ 5, 42 },
	{ 3, 15 },
	{ 11, 11 },
	{ 6, 38 },
	{ 6, 45 },
	{ 4, 5 },
	{ 5, 45 },
	{ 3, 19 },
	{ 2, 21 },
	{ 1, 11 },
	{ 8, 5 },
	{ 5, 38 },
	{ 5, 43 },
	{ 2, 18 },
	{ 5, 46 },
	{ 3, 20 },
	{ 2, 22 },
	{ 1, 12 },
	{ 5, 47 },
	{ 2, 20 },
	{ 3, 23 },
	{ 1, 13 },
	{ 2, 23 },
	{ 1, 14 },
	{ 1, 15 },
	{ 0, -1 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling1[16][3] =
{
	{ 0, 8, 3 },
	{ 0, 1, 9 },
	{ 1, 2, 10 },
	{ 3, 11, 2 },
	{ 4, 7, 8 },
	{ 9, 5, 4 },
	{ 10, 6, 5 },
	{ 7, 6, 11 },
	{ 7, 11, 6 },
	{ 10, 5, 6 },
	{ 9, 4, 5 },
	{ 4, 8, 7 },
	{ 3, 2, 11 },
	{ 1, 10, 2 },
	{ 0, 9, 1 },
	{ 0, 3, 8 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling2[24][6] =
{
	{ 1, 8, 3, 9, 8, 1 },
	{ 0, 11, 2, 8, 11, 0 },
	{ 4, 3, 0, 7, 3, 4 },
	{ 9, 2, 10, 0, 2, 9 },
	{ 0, 5, 4, 1, 5, 0 },
	{ 3, 10, 1, 11, 10, 3 },
	{ 1, 6, 5, 2, 6, 1 },
	{ 7, 2, 3, 6, 2, 7 },
	{ 9, 7, 8, 5, 7, 9 },
	{ 6, 8, 4, 11, 8, 6 },
	{ 10, 4, 9, 6, 4, 10 },
	{ 11, 5, 10, 7, 5, 11 },
	{ 11, 10, 5, 7, 11, 5 },
	{ 10, 9, 4, 6, 10, 4 },
	{ 6, 4, 8, 11, 6, 8 },
	{ 9, 8, 7, 5, 9, 7 },
	{ 7, 3, 2, 6, 7, 2 },
	{ 1, 5, 6, 2, 1, 6 },
	{ 3, 1, 10, 11, 3, 10 },
	{ 0, 4, 5, 1, 0, 5 },
	{ 9, 10, 2, 0, 9, 2 },
	{ 4, 0, 3, 7, 4, 3 },
	{ 0, 2, 11, 8, 0, 11 },
	{ 1, 3, 8, 9, 1, 8 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_test3[24] =
{
	5,
	1,
	4,
	5,
	1,
	2,
	2,
	3,
	4,
	3,
	6,
	6,
	-6,
	-6,
	-3,
	-4,
	-3,
	-2,
	-2,
	-1,
	-5,
	-4,
	-1,
	-5
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling3_1[24][6] =
{
	{ 0, 8, 3, 1, 2, 10 },
	{ 9, 5, 4, 0, 8, 3 },
	{ 3, 0, 8, 11, 7, 6 },
	{ 1, 9, 0, 2, 3, 11 },
	{ 0, 1, 9, 8, 4, 7 },
	{ 9, 0, 1, 5, 10, 6 },
	{ 1, 2, 10, 9, 5, 4 },
	{ 10, 1, 2, 6, 11, 7 },
	{ 8, 4, 7, 3, 11, 2 },
	{ 2, 3, 11, 10, 6, 5 },
	{ 5, 10, 6, 4, 7, 8 },
	{ 4, 9, 5, 7, 6, 11 },
	{ 5, 9, 4, 11, 6, 7 },
	{ 6, 10, 5, 8, 7, 4 },
	{ 11, 3, 2, 5, 6, 10 },
	{ 7, 4, 8, 2, 11, 3 },
	{ 2, 1, 10, 7, 11, 6 },
	{ 10, 2, 1, 4, 5, 9 },
	{ 1, 0, 9, 6, 10, 5 },
	{ 9, 1, 0, 7, 4, 8 },
	{ 0, 9, 1, 11, 3, 2 },
	{ 8, 0, 3, 6, 7, 11 },
	{ 4, 5, 9, 3, 8, 0 },
	{ 3, 8, 0, 10, 2, 1 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling3_2[24][12] =
{
	{ 10, 3, 2, 10, 8, 3, 10, 1, 0, 8, 10, 0 },
	{ 3, 4, 8, 3, 5, 4, 3, 0, 9, 5, 3, 9 },
	{ 6, 8, 7, 6, 0, 8, 6, 11, 3, 0, 6, 3 },
	{ 11, 0, 3, 11, 9, 0, 11, 2, 1, 9, 11, 1 },
	{ 7, 9, 4, 7, 1, 9, 7, 8, 0, 1, 7, 0 },
	{ 6, 1, 10, 6, 0, 1, 9, 0, 6, 9, 6, 5 },
	{ 4, 10, 5, 4, 2, 10, 4, 9, 1, 2, 4, 1 },
	{ 7, 2, 11, 7, 1, 2, 7, 6, 10, 1, 7, 10 },
	{ 2, 7, 11, 2, 4, 7, 2, 3, 8, 4, 2, 8 },
	{ 5, 11, 6, 5, 3, 11, 5, 10, 2, 3, 5, 2 },
	{ 8, 6, 7, 8, 10, 6, 8, 4, 5, 10, 8, 5 },
	{ 11, 5, 6, 11, 9, 5, 11, 7, 4, 9, 11, 4 },
	{ 6, 5, 11, 5, 9, 11, 4, 7, 11, 4, 11, 9 },
	{ 7, 6, 8, 6, 10, 8, 5, 4, 8, 5, 8, 10 },
	{ 6, 11, 5, 11, 3, 5, 2, 10, 5, 2, 5, 3 },
	{ 11, 7, 2, 7, 4, 2, 8, 3, 2, 8, 2, 4 },
	{ 11, 2, 7, 2, 1, 7, 10, 6, 7, 10, 7, 1 },
	{ 5, 10, 4, 10, 2, 4, 1, 9, 4, 1, 4, 2 },
	{ 10, 1, 6, 1, 0, 6, 6, 0, 9, 5, 6, 9 },
	{ 4, 9, 7, 9, 1, 7, 0, 8, 7, 0, 7, 1 },
	{ 3, 0, 11, 0, 9, 11, 1, 2, 11, 1, 11, 9 },
	{ 7, 8, 6, 8, 0, 6, 3, 11, 6, 3, 6, 0 },
	{ 8, 4, 3, 4, 5, 3, 9, 0, 3, 9, 3, 5 },
	{ 2, 3, 10, 3, 8, 10, 0, 1, 10, 0, 10, 8 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_test4[8] =
{
	7,
	7,
	7,
	7,
	-7,
	-7,
	-7,
	-7
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling4_1[8][6] =
{
	{ 0, 8, 3, 5, 10, 6 },
	{ 0, 1, 9, 11, 7, 6 },
	{ 1, 2, 10, 8, 4, 7 },
	{ 9, 5, 4, 2, 3, 11 },
	{ 4, 5, 9, 11, 3, 2 },
	{ 10, 2, 1, 7, 4, 8 },
	{ 9, 1, 0, 6, 7, 11 },
	{ 3, 8, 0, 6, 10, 5 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling4_2[8][18] =
{
	{ 8, 5, 0, 5, 8, 6, 3, 6, 8, 6, 3, 10, 0, 10, 3, 10, 0, 5 },
	{ 9, 6, 1, 6, 9, 7, 0, 7, 9, 7, 0, 11, 1, 11, 0, 11, 1, 6 },
	{ 10, 7, 2, 7, 10, 4, 1, 4, 10, 4, 1, 8, 2, 8, 1, 8, 2, 7 },
	{ 11, 4, 3, 4, 11, 5, 2, 5, 11, 5, 2, 9, 3, 9, 2, 9, 3, 4 },
	{ 3, 4, 11, 5, 11, 4, 11, 5, 2, 9, 2, 5, 2, 9, 3, 4, 3, 9 },
	{ 2, 7, 10, 4, 10, 7, 10, 4, 1, 8, 1, 4, 1, 8, 2, 7, 2, 8 },
	{ 1, 6, 9, 7, 9, 6, 9, 7, 0, 11, 0, 7, 0, 11, 1, 6, 1, 11 },
	{ 0, 5, 8, 6, 8, 5, 8, 6, 3, 10, 3, 6, 3, 10, 0, 5, 0, 10 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling5[48][9] =
{
	{ 2, 8, 3, 2, 10, 8, 10, 9, 8 },
	{ 1, 11, 2, 1, 9, 11, 9, 8, 11 },
	{ 4, 1, 9, 4, 7, 1, 7, 3, 1 },
	{ 8, 5, 4, 8, 3, 5, 3, 1, 5 },
	{ 0, 10, 1, 0, 8, 10, 8, 11, 10 },
	{ 11, 4, 7, 11, 2, 4, 2, 0, 4 },
	{ 7, 0, 8, 7, 6, 0, 6, 2, 0 },
	{ 9, 3, 0, 9, 5, 3, 5, 7, 3 },
	{ 3, 6, 11, 3, 0, 6, 0, 4, 6 },
	{ 3, 9, 0, 3, 11, 9, 11, 10, 9 },
	{ 5, 2, 10, 5, 4, 2, 4, 0, 2 },
	{ 9, 6, 5, 9, 0, 6, 0, 2, 6 },
	{ 0, 7, 8, 0, 1, 7, 1, 5, 7 },
	{ 10, 0, 1, 10, 6, 0, 6, 4, 0 },
	{ 6, 3, 11, 6, 5, 3, 5, 1, 3 },
	{ 10, 7, 6, 10, 1, 7, 1, 3, 7 },
	{ 1, 4, 9, 1, 2, 4, 2, 6, 4 },
	{ 11, 1, 2, 11, 7, 1, 7, 5, 1 },
	{ 8, 2, 3, 8, 4, 2, 4, 6, 2 },
	{ 2, 5, 10, 2, 3, 5, 3, 7, 5 },
	{ 7, 10, 6, 7, 8, 10, 8, 9, 10 },
	{ 6, 9, 5, 6, 11, 9, 11, 8, 9 },
	{ 5, 8, 4, 5, 10, 8, 10, 11, 8 },
	{ 4, 11, 7, 4, 9, 11, 9, 10, 11 },
	{ 4, 7, 11, 4, 11, 9, 9, 11, 10 },
	{ 5, 4, 8, 5, 8, 10, 10, 8, 11 },
	{ 6, 5, 9, 6, 9, 11, 11, 9, 8 },
	{ 7, 6, 10, 7, 10, 8, 8, 10, 9 },
	{ 2, 10, 5, 2, 5, 3, 3, 5, 7 },
	{ 8, 3, 2, 8, 2, 4, 4, 2, 6 },
	{ 11, 2, 1, 11, 1, 7, 7, 1, 5 },
	{ 1, 9, 4, 1, 4, 2, 2, 4, 6 },
	{ 10, 6, 7, 10, 7, 1, 1, 7, 3 },
	{ 6, 11, 3, 6, 3, 5, 5, 3, 1 },
	{ 10, 1, 0, 10, 0, 6, 6, 0, 4 },
	{ 0, 8, 7, 0, 7, 1, 1, 7, 5 },
	{ 9, 5, 6, 9, 6, 0, 0, 6, 2 },
	{ 5, 10, 2, 5, 2, 4, 4, 2, 0 },
	{ 3, 0, 9, 3, 9, 11, 11, 9, 10 },
	{ 3, 11, 6, 3, 6, 0, 0, 6, 4 },
	{ 9, 0, 3, 9, 3, 5, 5, 3, 7 },
	{ 7, 8, 0, 7, 0, 6, 6, 0, 2 },
	{ 11, 7, 4, 11, 4, 2, 2, 4, 0 },
	{ 0, 1, 10, 0, 10, 8, 8, 10, 11 },
	{ 8, 4, 5, 8, 5, 3, 3, 5, 1 },
	{ 4, 9, 1, 4, 1, 7, 7, 1, 3 },
	{ 1, 2, 11, 1, 11, 9, 9, 11, 8 },
	{ 2, 3, 8, 2, 8, 10, 10, 8, 9 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_test6[48][3] =
{
	{ 2, 7, 10 },
	{ 4, 7, 11 },
	{ 5, 7, 1 },
	{ 5, 7, 3 },
	{ 1, 7, 9 },
	{ 3, 7, 10 },
	{ 6, 7, 5 },
	{ 1, 7, 8 },
	{ 4, 7, 8 },
	{ 1, 7, 8 },
	{ 3, 7, 11 },
	{ 5, 7, 2 },
	{ 5, 7, 0 },
	{ 1, 7, 9 },
	{ 6, 7, 6 },
	{ 2, 7, 9 },
	{ 4, 7, 8 },
	{ 2, 7, 9 },
	{ 2, 7, 10 },
	{ 6, 7, 7 },
	{ 3, 7, 10 },
	{ 4, 7, 11 },
	{ 3, 7, 11 },
	{ 6, 7, 4 },
	{ -6, -7, 4 },
	{ -3, -7, 11 },
	{ -4, -7, 11 },
	{ -3, -7, 10 },
	{ -6, -7, 7 },
	{ -2, -7, 10 },
	{ -2, -7, 9 },
	{ -4, -7, 8 },
	{ -2, -7, 9 },
	{ -6, -7, 6 },
	{ -1, -7, 9 },
	{ -5, -7, 0 },
	{ -5, -7, 2 },
	{ -3, -7, 11 },
	{ -1, -7, 8 },
	{ -4, -7, 8 },
	{ -1, -7, 8 },
	{ -6, -7, 5 },
	{ -3, -7, 10 },
	{ -1, -7, 9 },
	{ -5, -7, 3 },
	{ -5, -7, 1 },
	{ -4, -7, 11 },
	{ -2, -7, 10 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling6_1_1[48][9] =
{
	{ 6, 5, 10, 3, 1, 8, 9, 8, 1 },
	{ 11, 7, 6, 9, 3, 1, 3, 9, 8 },
	{ 1, 2, 10, 7, 0, 4, 0, 7, 3 },
	{ 3, 0, 8, 5, 2, 6, 2, 5, 1 },
	{ 5, 4, 9, 2, 0, 11, 8, 11, 0 },
	{ 10, 6, 5, 8, 2, 0, 2, 8, 11 },
	{ 10, 6, 5, 0, 4, 3, 7, 3, 4 },
	{ 3, 0, 8, 6, 4, 10, 9, 10, 4 },
	{ 8, 3, 0, 10, 7, 5, 7, 10, 11 },
	{ 8, 4, 7, 10, 0, 2, 0, 10, 9 },
	{ 7, 6, 11, 0, 2, 9, 10, 9, 2 },
	{ 2, 3, 11, 4, 1, 5, 1, 4, 0 },
	{ 0, 1, 9, 6, 3, 7, 3, 6, 2 },
	{ 9, 0, 1, 11, 4, 6, 4, 11, 8 },
	{ 11, 7, 6, 1, 5, 0, 4, 0, 5 },
	{ 0, 1, 9, 7, 5, 11, 10, 11, 5 },
	{ 4, 7, 8, 1, 3, 10, 11, 10, 3 },
	{ 9, 5, 4, 11, 1, 3, 1, 11, 10 },
	{ 10, 1, 2, 8, 5, 7, 5, 8, 9 },
	{ 8, 4, 7, 2, 6, 1, 5, 1, 6 },
	{ 1, 2, 10, 4, 6, 8, 11, 8, 6 },
	{ 2, 3, 11, 5, 7, 9, 8, 9, 7 },
	{ 11, 2, 3, 9, 6, 4, 6, 9, 10 },
	{ 9, 5, 4, 3, 7, 2, 6, 2, 7 },
	{ 4, 5, 9, 2, 7, 3, 7, 2, 6 },
	{ 3, 2, 11, 4, 6, 9, 10, 9, 6 },
	{ 11, 3, 2, 9, 7, 5, 7, 9, 8 },
	{ 10, 2, 1, 8, 6, 4, 6, 8, 11 },
	{ 7, 4, 8, 1, 6, 2, 6, 1, 5 },
	{ 2, 1, 10, 7, 5, 8, 9, 8, 5 },
	{ 4, 5, 9, 3, 1, 11, 10, 11, 1 },
	{ 8, 7, 4, 10, 3, 1, 3, 10, 11 },
	{ 9, 1, 0, 11, 5, 7, 5, 11, 10 },
	{ 6, 7, 11, 0, 5, 1, 5, 0, 4 },
	{ 1, 0, 9, 6, 4, 11, 8, 11, 4 },
	{ 9, 1, 0, 7, 3, 6, 2, 6, 3 },
	{ 11, 3, 2, 5, 1, 4, 0, 4, 1 },
	{ 11, 6, 7, 9, 2, 0, 2, 9, 10 },
	{ 7, 4, 8, 2, 0, 10, 9, 10, 0 },
	{ 0, 3, 8, 5, 7, 10, 11, 10, 7 },
	{ 8, 0, 3, 10, 4, 6, 4, 10, 9 },
	{ 5, 6, 10, 3, 4, 0, 4, 3, 7 },
	{ 5, 6, 10, 0, 2, 8, 11, 8, 2 },
	{ 9, 4, 5, 11, 0, 2, 0, 11, 8 },
	{ 8, 0, 3, 6, 2, 5, 1, 5, 2 },
	{ 10, 2, 1, 4, 0, 7, 3, 7, 0 },
	{ 6, 7, 11, 1, 3, 9, 8, 9, 3 },
	{ 10, 5, 6, 8, 1, 3, 1, 8, 9 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling6_1_2[48][21] =
{
	{ 1, 10, 3, 6, 3, 10, 3, 6, 8, 5, 8, 6, 8, 5, 9, 1, 9, 5, 10, 1, 5 },
	{ 1, 11, 3, 11, 1, 6, 9, 6, 1, 6, 9, 7, 8, 7, 9, 7, 8, 3, 7, 3, 11 },
	{ 4, 1, 0, 1, 4, 10, 7, 10, 4, 10, 7, 2, 3, 2, 7, 2, 3, 0, 2, 0, 1 },
	{ 6, 3, 2, 3, 6, 8, 5, 8, 6, 8, 5, 0, 1, 0, 5, 0, 1, 2, 0, 2, 3 },
	{ 0, 9, 2, 5, 2, 9, 2, 5, 11, 4, 11, 5, 11, 4, 8, 0, 8, 4, 9, 0, 4 },
	{ 0, 10, 2, 10, 0, 5, 8, 5, 0, 5, 8, 6, 11, 6, 8, 6, 11, 2, 6, 2, 10 },
	{ 4, 5, 0, 10, 0, 5, 0, 10, 3, 6, 3, 10, 3, 6, 7, 4, 7, 6, 5, 4, 6 },
	{ 4, 8, 6, 3, 6, 8, 6, 3, 10, 0, 10, 3, 10, 0, 9, 4, 9, 0, 8, 4, 0 },
	{ 5, 8, 7, 8, 5, 0, 10, 0, 5, 0, 10, 3, 11, 3, 10, 3, 11, 7, 3, 7, 8 },
	{ 2, 8, 0, 8, 2, 7, 10, 7, 2, 7, 10, 4, 9, 4, 10, 4, 9, 0, 4, 0, 8 },
	{ 2, 11, 0, 7, 0, 11, 0, 7, 9, 6, 9, 7, 9, 6, 10, 2, 10, 6, 11, 2, 6 },
	{ 5, 2, 1, 2, 5, 11, 4, 11, 5, 11, 4, 3, 0, 3, 4, 3, 0, 1, 3, 1, 2 },
	{ 7, 0, 3, 0, 7, 9, 6, 9, 7, 9, 6, 1, 2, 1, 6, 1, 2, 3, 1, 3, 0 },
	{ 6, 9, 4, 9, 6, 1, 11, 1, 6, 1, 11, 0, 8, 0, 11, 0, 8, 4, 0, 4, 9 },
	{ 5, 6, 1, 11, 1, 6, 1, 11, 0, 7, 0, 11, 0, 7, 4, 5, 4, 7, 6, 5, 7 },
	{ 5, 9, 7, 0, 7, 9, 7, 0, 11, 1, 11, 0, 11, 1, 10, 5, 10, 1, 9, 5, 1 },
	{ 3, 8, 1, 4, 1, 8, 1, 4, 10, 7, 10, 4, 10, 7, 11, 3, 11, 7, 8, 3, 7 },
	{ 3, 9, 1, 9, 3, 4, 11, 4, 3, 4, 11, 5, 10, 5, 11, 5, 10, 1, 5, 1, 9 },
	{ 7, 10, 5, 10, 7, 2, 8, 2, 7, 2, 8, 1, 9, 1, 8, 1, 9, 5, 1, 5, 10 },
	{ 6, 7, 2, 8, 2, 7, 2, 8, 1, 4, 1, 8, 1, 4, 5, 6, 5, 4, 7, 6, 4 },
	{ 6, 10, 4, 1, 4, 10, 4, 1, 8, 2, 8, 1, 8, 2, 11, 6, 11, 2, 10, 6, 2 },
	{ 7, 11, 5, 2, 5, 11, 5, 2, 9, 3, 9, 2, 9, 3, 8, 7, 8, 3, 11, 7, 3 },
	{ 4, 11, 6, 11, 4, 3, 9, 3, 4, 3, 9, 2, 10, 2, 9, 2, 10, 6, 2, 6, 11 },
	{ 7, 4, 3, 9, 3, 4, 3, 9, 2, 5, 2, 9, 2, 5, 6, 7, 6, 5, 4, 7, 5 },
	{ 3, 4, 7, 4, 3, 9, 2, 9, 3, 9, 2, 5, 6, 5, 2, 5, 6, 7, 5, 7, 4 },
	{ 6, 11, 4, 3, 4, 11, 4, 3, 9, 2, 9, 3, 9, 2, 10, 6, 10, 2, 11, 6, 2 },
	{ 5, 11, 7, 11, 5, 2, 9, 2, 5, 2, 9, 3, 8, 3, 9, 3, 8, 7, 3, 7, 11 },
	{ 4, 10, 6, 10, 4, 1, 8, 1, 4, 1, 8, 2, 11, 2, 8, 2, 11, 6, 2, 6, 10 },
	{ 2, 7, 6, 7, 2, 8, 1, 8, 2, 8, 1, 4, 5, 4, 1, 4, 5, 6, 4, 6, 7 },
	{ 5, 10, 7, 2, 7, 10, 7, 2, 8, 1, 8, 2, 8, 1, 9, 5, 9, 1, 10, 5, 1 },
	{ 1, 9, 3, 4, 3, 9, 3, 4, 11, 5, 11, 4, 11, 5, 10, 1, 10, 5, 9, 1, 5 },
	{ 1, 8, 3, 8, 1, 4, 10, 4, 1, 4, 10, 7, 11, 7, 10, 7, 11, 3, 7, 3, 8 },
	{ 7, 9, 5, 9, 7, 0, 11, 0, 7, 0, 11, 1, 10, 1, 11, 1, 10, 5, 1, 5, 9 },
	{ 1, 6, 5, 6, 1, 11, 0, 11, 1, 11, 0, 7, 4, 7, 0, 7, 4, 5, 7, 5, 6 },
	{ 4, 9, 6, 1, 6, 9, 6, 1, 11, 0, 11, 1, 11, 0, 8, 4, 8, 0, 9, 4, 0 },
	{ 3, 0, 7, 9, 7, 0, 7, 9, 6, 1, 6, 9, 6, 1, 2, 3, 2, 1, 0, 3, 1 },
	{ 1, 2, 5, 11, 5, 2, 5, 11, 4, 3, 4, 11, 4, 3, 0, 1, 0, 3, 2, 1, 3 },
	{ 0, 11, 2, 11, 0, 7, 9, 7, 0, 7, 9, 6, 10, 6, 9, 6, 10, 2, 6, 2, 11 },
	{ 0, 8, 2, 7, 2, 8, 2, 7, 10, 4, 10, 7, 10, 4, 9, 0, 9, 4, 8, 0, 4 },
	{ 7, 8, 5, 0, 5, 8, 5, 0, 10, 3, 10, 0, 10, 3, 11, 7, 11, 3, 8, 7, 3 },
	{ 6, 8, 4, 8, 6, 3, 10, 3, 6, 3, 10, 0, 9, 0, 10, 0, 9, 4, 0, 4, 8 },
	{ 0, 5, 4, 5, 0, 10, 3, 10, 0, 10, 3, 6, 7, 6, 3, 6, 7, 4, 6, 4, 5 },
	{ 2, 10, 0, 5, 0, 10, 0, 5, 8, 6, 8, 5, 8, 6, 11, 2, 11, 6, 10, 2, 6 },
	{ 2, 9, 0, 9, 2, 5, 11, 5, 2, 5, 11, 4, 8, 4, 11, 4, 8, 0, 4, 0, 9 },
	{ 2, 3, 6, 8, 6, 3, 6, 8, 5, 0, 5, 8, 5, 0, 1, 2, 1, 0, 3, 2, 0 },
	{ 0, 1, 4, 10, 4, 1, 4, 10, 7, 2, 7, 10, 7, 2, 3, 0, 3, 2, 1, 0, 2 },
	{ 3, 11, 1, 6, 1, 11, 1, 6, 9, 7, 9, 6, 9, 7, 8, 3, 8, 7, 11, 3, 7 },
	{ 3, 10, 1, 10, 3, 6, 8, 6, 3, 6, 8, 5, 9, 5, 8, 5, 9, 1, 5, 1, 10 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling6_2[48][15] =
{
	{ 1, 10, 3, 6, 3, 10, 3, 6, 8, 5, 8, 6, 8, 5, 9 },
	{ 1, 11, 3, 11, 1, 6, 9, 6, 1, 6, 9, 7, 8, 7, 9 },
	{ 4, 1, 0, 1, 4, 10, 7, 10, 4, 10, 7, 2, 3, 2, 7 },
	{ 6, 3, 2, 3, 6, 8, 5, 8, 6, 8, 5, 0, 1, 0, 5 },
	{ 0, 9, 2, 5, 2, 9, 2, 5, 11, 4, 11, 5, 11, 4, 8 },
	{ 0, 10, 2, 10, 0, 5, 8, 5, 0, 5, 8, 6, 11, 6, 8 },
	{ 4, 5, 0, 10, 0, 5, 0, 10, 3, 6, 3, 10, 3, 6, 7 },
	{ 4, 8, 6, 3, 6, 8, 6, 3, 10, 0, 10, 3, 10, 0, 9 },
	{ 5, 8, 7, 8, 5, 0, 10, 0, 5, 0, 10, 3, 11, 3, 10 },
	{ 2, 8, 0, 8, 2, 7, 10, 7, 2, 7, 10, 4, 9, 4, 10 },
	{ 2, 11, 0, 7, 0, 11, 0, 7, 9, 6, 9, 7, 9, 6, 10 },
	{ 5, 2, 1, 2, 5, 11, 4, 11, 5, 11, 4, 3, 0, 3, 4 },
	{ 7, 0, 3, 0, 7, 9, 6, 9, 7, 9, 6, 1, 2, 1, 6 },
	{ 6, 9, 4, 9, 6, 1, 11, 1, 6, 1, 11, 0, 8, 0, 11 },
	{ 5, 6, 1, 11, 1, 6, 1, 11, 0, 7, 0, 11, 0, 7, 4 },
	{ 5, 9, 7, 0, 7, 9, 7, 0, 11, 1, 11, 0, 11, 1, 10 },
	{ 3, 8, 1, 4, 1, 8, 1, 4, 10, 7, 10, 4, 10, 7, 11 },
	{ 3, 9, 1, 9, 3, 4, 11, 4, 3, 4, 11, 5, 10, 5, 11 },
	{ 7, 10, 5, 10, 7, 2, 8, 2, 7, 2, 8, 1, 9, 1, 8 },
	{ 6, 7, 2, 8, 2, 7, 2, 8, 1, 4, 1, 8, 1, 4, 5 },
	{ 6, 10, 4, 1, 4, 10, 4, 1, 8, 2, 8, 1, 8, 2, 11 },
	{ 7, 11, 5, 2, 5, 11, 5, 2, 9, 3, 9, 2, 9, 3, 8 },
	{ 4, 11, 6, 11, 4, 3, 9, 3, 4, 3, 9, 2, 10, 2, 9 },
	{ 7, 4, 3, 9, 3, 4, 3, 9, 2, 5, 2, 9, 2, 5, 6 },
	{ 3, 4, 7, 4, 3, 9, 2, 9, 3, 9, 2, 5, 6, 5, 2 },
	{ 6, 11, 4, 3, 4, 11, 4, 3, 9, 2, 9, 3, 9, 2, 10 },
	{ 5, 11, 7, 11, 5, 2, 9, 2, 5, 2, 9, 3, 8, 3, 9 },
	{ 4, 10, 6, 10, 4, 1, 8, 1, 4, 1, 8, 2, 11, 2, 8 },
	{ 2, 7, 6, 7, 2, 8, 1, 8, 2, 8, 1, 4, 5, 4, 1 },
	{ 5, 10, 7, 2, 7, 10, 7, 2, 8, 1, 8, 2, 8, 1, 9 },
	{ 1, 9, 3, 4, 3, 9, 3, 4, 11, 5, 11, 4, 11, 5, 10 },
	{ 1, 8, 3, 8, 1, 4, 10, 4, 1, 4, 10, 7, 11, 7, 10 },
	{ 7, 9, 5, 9, 7, 0, 11, 0, 7, 0, 11, 1, 10, 1, 11 },
	{ 1, 6, 5, 6, 1, 11, 0, 11, 1, 11, 0, 7, 4, 7, 0 },
	{ 4, 9, 6, 1, 6, 9, 6, 1, 11, 0, 11, 1, 11, 0, 8 },
	{ 3, 0, 7, 9, 7, 0, 7, 9, 6, 1, 6, 9, 6, 1, 2 },
	{ 1, 2, 5, 11, 5, 2, 5, 11, 4, 3, 4, 11, 4, 3, 0 },
	{ 0, 11, 2, 11, 0, 7, 9, 7, 0, 7, 9, 6, 10, 6, 9 },
	{ 0, 8, 2, 7, 2, 8, 2, 7, 10, 4, 10, 7, 10, 4, 9 },
	{ 7, 8, 5, 0, 5, 8, 5, 0, 10, 3, 10, 0, 10, 3, 11 },
	{ 6, 8, 4, 8, 6, 3, 10, 3, 6, 3, 10, 0, 9, 0, 10 },
	{ 0, 5, 4, 5, 0, 10, 3, 10, 0, 10, 3, 6, 7, 6, 3 },
	{ 2, 10, 0, 5, 0, 10, 0, 5, 8, 6, 8, 5, 8, 6, 11 },
	{ 2, 9, 0, 9, 2, 5, 11, 5, 2, 5, 11, 4, 8, 4, 11 },
	{ 2, 3, 6, 8, 6, 3, 6, 8, 5, 0, 5, 8, 5, 0, 1 },
	{ 0, 1, 4, 10, 4, 1, 4, 10, 7, 2, 7, 10, 7, 2, 3 },
	{ 3, 11, 1, 6, 1, 11, 1, 6, 9, 7, 9, 6, 9, 7, 8 },
	{ 3, 10, 1, 10, 3, 6, 8, 6, 3, 6, 8, 5, 9, 5, 8 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_test7[16][5] =
{
	{ 1, 2, 5, 7, 1 },
	{ 3, 4, 5, 7, 3 },
	{ 4, 1, 6, 7, 4 },
	{ 4, 1, 5, 7, 0 },
	{ 2, 3, 5, 7, 2 },
	{ 1, 2, 6, 7, 5 },
	{ 2, 3, 6, 7, 6 },
	{ 3, 4, 6, 7, 7 },
	{ -3, -4, -6, -7, 7 },
	{ -2, -3, -6, -7, 6 },
	{ -1, -2, -6, -7, 5 },
	{ -2, -3, -5, -7, 2 },
	{ -4, -1, -5, -7, 0 },
	{ -4, -1, -6, -7, 4 },
	{ -3, -4, -5, -7, 3 },
	{ -1, -2, -5, -7, 1 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling7_1[16][9] =
{
	{ 9, 5, 4, 10, 1, 2, 8, 3, 0 },
	{ 11, 7, 6, 8, 3, 0, 10, 1, 2 },
	{ 3, 0, 8, 5, 4, 9, 7, 6, 11 },
	{ 8, 4, 7, 9, 0, 1, 11, 2, 3 },
	{ 10, 6, 5, 11, 2, 3, 9, 0, 1 },
	{ 0, 1, 9, 6, 5, 10, 4, 7, 8 },
	{ 1, 2, 10, 7, 6, 11, 5, 4, 9 },
	{ 2, 3, 11, 4, 7, 8, 6, 5, 10 },
	{ 11, 3, 2, 8, 7, 4, 10, 5, 6 },
	{ 10, 2, 1, 11, 6, 7, 9, 4, 5 },
	{ 9, 1, 0, 10, 5, 6, 8, 7, 4 },
	{ 5, 6, 10, 3, 2, 11, 1, 0, 9 },
	{ 7, 4, 8, 1, 0, 9, 3, 2, 11 },
	{ 8, 0, 3, 9, 4, 5, 11, 6, 7 },
	{ 6, 7, 11, 0, 3, 8, 2, 1, 10 },
	{ 4, 5, 9, 2, 1, 10, 0, 3, 8 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling7_2[16][3][15] =
{
	{
		{ 1, 2, 10, 3, 4, 8, 4, 3, 5, 0, 5, 3, 5, 0, 9 },
		{ 3, 0, 8, 9, 1, 4, 2, 4, 1, 4, 2, 5, 10, 5, 2 },
		{ 9, 5, 4, 0, 10, 1, 10, 0, 8, 10, 8, 2, 3, 2, 8 }
	},
	{
		{ 3, 0, 8, 1, 6, 10, 6, 1, 7, 2, 7, 1, 7, 2, 11 },
		{ 1, 2, 10, 11, 3, 6, 0, 6, 3, 6, 0, 7, 8, 7, 0 },
		{ 11, 7, 6, 2, 8, 3, 8, 2, 10, 8, 10, 0, 1, 0, 10 }
	},
	{
		{ 9, 5, 4, 11, 3, 6, 0, 6, 3, 6, 0, 7, 8, 7, 0 },
		{ 11, 7, 6, 3, 4, 8, 4, 3, 5, 0, 5, 3, 5, 0, 9 },
		{ 3, 0, 8, 4, 9, 7, 11, 7, 9, 5, 11, 9, 11, 5, 6 }
	},
	{
		{ 0, 1, 9, 2, 7, 11, 7, 2, 4, 3, 4, 2, 4, 3, 8 },
		{ 2, 3, 11, 8, 0, 7, 1, 7, 0, 7, 1, 4, 9, 4, 1 },
		{ 8, 4, 7, 3, 9, 0, 9, 3, 11, 9, 11, 1, 2, 1, 11 }
	},
	{
		{ 2, 3, 11, 0, 5, 9, 5, 0, 6, 1, 6, 0, 6, 1, 10 },
		{ 0, 1, 9, 10, 2, 5, 3, 5, 2, 5, 3, 6, 11, 6, 3 },
		{ 6, 5, 10, 1, 11, 2, 11, 1, 9, 11, 9, 3, 0, 3, 9 }
	},
	{
		{ 6, 5, 10, 8, 0, 7, 1, 7, 0, 7, 1, 4, 9, 4, 1 },
		{ 8, 4, 7, 0, 5, 9, 5, 0, 6, 1, 6, 0, 6, 1, 10 },
		{ 0, 1, 9, 5, 10, 4, 8, 4, 10, 6, 8, 10, 8, 6, 7 }
	},
	{
		{ 11, 7, 6, 9, 1, 4, 2, 4, 1, 4, 2, 5, 10, 5, 2 },
		{ 9, 5, 4, 1, 6, 10, 6, 1, 7, 2, 7, 1, 7, 2, 11 },
		{ 1, 2, 10, 6, 11, 5, 9, 5, 11, 7, 9, 11, 9, 7, 4 }
	},
	{
		{ 8, 4, 7, 10, 2, 5, 3, 5, 2, 5, 3, 6, 11, 6, 3 },
		{ 6, 5, 10, 2, 7, 11, 7, 2, 4, 3, 4, 2, 4, 3, 8 },
		{ 2, 3, 11, 7, 8, 6, 10, 6, 8, 4, 10, 8, 10, 4, 5 }
	},
	{
		{ 7, 4, 8, 5, 2, 10, 2, 5, 3, 6, 3, 5, 3, 6, 11 },
		{ 10, 5, 6, 11, 7, 2, 4, 2, 7, 2, 4, 3, 8, 3, 4 },
		{ 11, 3, 2, 6, 8, 7, 8, 6, 10, 8, 10, 4, 5, 4, 10 }
	},
	{
		{ 6, 7, 11, 4, 1, 9, 1, 4, 2, 5, 2, 4, 2, 5, 10 },
		{ 4, 5, 9, 10, 6, 1, 7, 1, 6, 1, 7, 2, 11, 2, 7 },
		{ 10, 2, 1, 5, 11, 6, 11, 5, 9, 11, 9, 7, 4, 7, 9 }
	},
	{
		{ 10, 5, 6, 7, 0, 8, 0, 7, 1, 4, 1, 7, 1, 4, 9 },
		{ 7, 4, 8, 9, 5, 0, 6, 0, 5, 0, 6, 1, 10, 1, 6 },
		{ 9, 1, 0, 4, 10, 5, 10, 4, 8, 10, 8, 6, 7, 6, 8 }
	},
	{
		{ 11, 3, 2, 9, 5, 0, 6, 0, 5, 0, 6, 1, 10, 1, 6 },
		{ 9, 1, 0, 5, 2, 10, 2, 5, 3, 6, 3, 5, 3, 6, 11 },
		{ 10, 5, 6, 2, 11, 1, 9, 1, 11, 3, 9, 11, 9, 3, 0 }
	},
	{
		{ 9, 1, 0, 11, 7, 2, 4, 2, 7, 2, 4, 3, 8, 3, 4 },
		{ 11, 3, 2, 7, 0, 8, 0, 7, 1, 4, 1, 7, 1, 4, 9 },
		{ 7, 4, 8, 0, 9, 3, 11, 3, 9, 1, 11, 9, 11, 1, 2 }
	},
	{
		{ 4, 5, 9, 6, 3, 11, 3, 6, 0, 7, 0, 6, 0, 7, 8 },
		{ 6, 7, 11, 8, 4, 3, 5, 3, 4, 3, 5, 0, 9, 0, 5 },
		{ 8, 0, 3, 7, 9, 4, 9, 7, 11, 9, 11, 5, 6, 5, 11 }
	},
	{
		{ 8, 0, 3, 10, 6, 1, 7, 1, 6, 1, 7, 2, 11, 2, 7 },
		{ 10, 2, 1, 6, 3, 11, 3, 6, 0, 7, 0, 6, 0, 7, 8 },
		{ 6, 7, 11, 3, 8, 2, 10, 2, 8, 0, 10, 8, 10, 0, 1 }
	},
	{
		{ 10, 2, 1, 8, 4, 3, 5, 3, 4, 3, 5, 0, 9, 0, 5 },
		{ 8, 0, 3, 4, 1, 9, 1, 4, 2, 5, 2, 4, 2, 5, 10 },
		{ 4, 5, 9, 1, 10, 0, 8, 0, 10, 2, 8, 10, 8, 2, 3 } }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling7_3[16][3][27] =
{
	{
		{ 12, 2, 10, 12, 10, 5, 12, 5, 4, 12, 4, 8, 12, 8, 3, 12, 3, 0, 12, 0, 9, 12, 9, 1, 12, 1, 2 },
		{ 12, 5, 4, 12, 4, 8, 12, 8, 3, 12, 3, 2, 12, 2, 10, 12, 10, 1, 12, 1, 0, 12, 0, 9, 12, 9, 5 },
		{ 5, 4, 12, 10, 5, 12, 2, 10, 12, 3, 2, 12, 8, 3, 12, 0, 8, 12, 1, 0, 12, 9, 1, 12, 4, 9, 12 }
	},
	{
		{ 12, 0, 8, 12, 8, 7, 12, 7, 6, 12, 6, 10, 12, 10, 1, 12, 1, 2, 12, 2, 11, 12, 11, 3, 12, 3, 0 },
		{ 12, 7, 6, 12, 6, 10, 12, 10, 1, 12, 1, 0, 12, 0, 8, 12, 8, 3, 12, 3, 2, 12, 2, 11, 12, 11, 7 },
		{ 7, 6, 12, 8, 7, 12, 0, 8, 12, 1, 0, 12, 10, 1, 12, 2, 10, 12, 3, 2, 12, 11, 3, 12, 6, 11, 12 }
	},
	{
		{ 9, 5, 12, 0, 9, 12, 3, 0, 12, 11, 3, 12, 6, 11, 12, 7, 6, 12, 8, 7, 12, 4, 8, 12, 5, 4, 12 },
		{ 3, 0, 12, 11, 3, 12, 6, 11, 12, 5, 6, 12, 9, 5, 12, 4, 9, 12, 7, 4, 12, 8, 7, 12, 0, 8, 12 },
		{ 12, 3, 0, 12, 0, 9, 12, 9, 5, 12, 5, 6, 12, 6, 11, 12, 11, 7, 12, 7, 4, 12, 4, 8, 12, 8, 3 }
	},
	{
		{ 12, 1, 9, 12, 9, 4, 12, 4, 7, 12, 7, 11, 12, 11, 2, 12, 2, 3, 12, 3, 8, 12, 8, 0, 12, 0, 1 },
		{ 12, 4, 7, 12, 7, 11, 12, 11, 2, 12, 2, 1, 12, 1, 9, 12, 9, 0, 12, 0, 3, 12, 3, 8, 12, 8, 4 },
		{ 4, 7, 12, 9, 4, 12, 1, 9, 12, 2, 1, 12, 11, 2, 12, 3, 11, 12, 0, 3, 12, 8, 0, 12, 7, 8, 12 }
	},
	{
		{ 12, 3, 11, 12, 11, 6, 12, 6, 5, 12, 5, 9, 12, 9, 0, 12, 0, 1, 12, 1, 10, 12, 10, 2, 12, 2, 3 },
		{ 12, 6, 5, 12, 5, 9, 12, 9, 0, 12, 0, 3, 12, 3, 11, 12, 11, 2, 12, 2, 1, 12, 1, 10, 12, 10, 6 },
		{ 6, 5, 12, 11, 6, 12, 3, 11, 12, 0, 3, 12, 9, 0, 12, 1, 9, 12, 2, 1, 12, 10, 2, 12, 5, 10, 12 }
	},
	{
		{ 10, 6, 12, 1, 10, 12, 0, 1, 12, 8, 0, 12, 7, 8, 12, 4, 7, 12, 9, 4, 12, 5, 9, 12, 6, 5, 12 },
		{ 0, 1, 12, 8, 0, 12, 7, 8, 12, 6, 7, 12, 10, 6, 12, 5, 10, 12, 4, 5, 12, 9, 4, 12, 1, 9, 12 },
		{ 12, 0, 1, 12, 1, 10, 12, 10, 6, 12, 6, 7, 12, 7, 8, 12, 8, 4, 12, 4, 5, 12, 5, 9, 12, 9, 0 }
	},
	{
		{ 11, 7, 12, 2, 11, 12, 1, 2, 12, 9, 1, 12, 4, 9, 12, 5, 4, 12, 10, 5, 12, 6, 10, 12, 7, 6, 12 },
		{ 1, 2, 12, 9, 1, 12, 4, 9, 12, 7, 4, 12, 11, 7, 12, 6, 11, 12, 5, 6, 12, 10, 5, 12, 2, 10, 12 },
		{ 12, 1, 2, 12, 2, 11, 12, 11, 7, 12, 7, 4, 12, 4, 9, 12, 9, 5, 12, 5, 6, 12, 6, 10, 12, 10, 1 }
	},
	{
		{ 8, 4, 12, 3, 8, 12, 2, 3, 12, 10, 2, 12, 5, 10, 12, 6, 5, 12, 11, 6, 12, 7, 11, 12, 4, 7, 12 },
		{ 2, 3, 12, 10, 2, 12, 5, 10, 12, 4, 5, 12, 8, 4, 12, 7, 8, 12, 6, 7, 12, 11, 6, 12, 3, 11, 12 },
		{ 12, 2, 3, 12, 3, 8, 12, 8, 4, 12, 4, 5, 12, 5, 10, 12, 10, 6, 12, 6, 7, 12, 7, 11, 12, 11, 2 }
	},
	{
		{ 12, 4, 8, 12, 8, 3, 12, 3, 2, 12, 2, 10, 12, 10, 5, 12, 5, 6, 12, 6, 11, 12, 11, 7, 12, 7, 4 },
		{ 12, 3, 2, 12, 2, 10, 12, 10, 5, 12, 5, 4, 12, 4, 8, 12, 8, 7, 12, 7, 6, 12, 6, 11, 12, 11, 3 },
		{ 3, 2, 12, 8, 3, 12, 4, 8, 12, 5, 4, 12, 10, 5, 12, 6, 10, 12, 7, 6, 12, 11, 7, 12, 2, 11, 12 }
	},
	{
		{ 12, 7, 11, 12, 11, 2, 12, 2, 1, 12, 1, 9, 12, 9, 4, 12, 4, 5, 12, 5, 10, 12, 10, 6, 12, 6, 7 },
		{ 12, 2, 1, 12, 1, 9, 12, 9, 4, 12, 4, 7, 12, 7, 11, 12, 11, 6, 12, 6, 5, 12, 5, 10, 12, 10, 2 },
		{ 2, 1, 12, 11, 2, 12, 7, 11, 12, 4, 7, 12, 9, 4, 12, 5, 9, 12, 6, 5, 12, 10, 6, 12, 1, 10, 12 }
	},
	{
		{ 12, 6, 10, 12, 10, 1, 12, 1, 0, 12, 0, 8, 12, 8, 7, 12, 7, 4, 12, 4, 9, 12, 9, 5, 12, 5, 6 },
		{ 12, 1, 0, 12, 0, 8, 12, 8, 7, 12, 7, 6, 12, 6, 10, 12, 10, 5, 12, 5, 4, 12, 4, 9, 12, 9, 1 },
		{ 1, 0, 12, 10, 1, 12, 6, 10, 12, 7, 6, 12, 8, 7, 12, 4, 8, 12, 5, 4, 12, 9, 5, 12, 0, 9, 12 }
	},
	{
		{ 11, 3, 12, 6, 11, 12, 5, 6, 12, 9, 5, 12, 0, 9, 12, 1, 0, 12, 10, 1, 12, 2, 10, 12, 3, 2, 12 },
		{ 5, 6, 12, 9, 5, 12, 0, 9, 12, 3, 0, 12, 11, 3, 12, 2, 11, 12, 1, 2, 12, 10, 1, 12, 6, 10, 12 },
		{ 12, 5, 6, 12, 6, 11, 12, 11, 3, 12, 3, 0, 12, 0, 9, 12, 9, 1, 12, 1, 2, 12, 2, 10, 12, 10, 5 }
	},
	{
		{ 9, 1, 12, 4, 9, 12, 7, 4, 12, 11, 7, 12, 2, 11, 12, 3, 2, 12, 8, 3, 12, 0, 8, 12, 1, 0, 12 },
		{ 7, 4, 12, 11, 7, 12, 2, 11, 12, 1, 2, 12, 9, 1, 12, 0, 9, 12, 3, 0, 12, 8, 3, 12, 4, 8, 12 },
		{ 12, 7, 4, 12, 4, 9, 12, 9, 1, 12, 1, 2, 12, 2, 11, 12, 11, 3, 12, 3, 0, 12, 0, 8, 12, 8, 7 }
	},
	{
		{ 12, 5, 9, 12, 9, 0, 12, 0, 3, 12, 3, 11, 12, 11, 6, 12, 6, 7, 12, 7, 8, 12, 8, 4, 12, 4, 5 },
		{ 12, 0, 3, 12, 3, 11, 12, 11, 6, 12, 6, 5, 12, 5, 9, 12, 9, 4, 12, 4, 7, 12, 7, 8, 12, 8, 0 },
		{ 0, 3, 12, 9, 0, 12, 5, 9, 12, 6, 5, 12, 11, 6, 12, 7, 11, 12, 4, 7, 12, 8, 4, 12, 3, 8, 12 }
	},
	{
		{ 8, 0, 12, 7, 8, 12, 6, 7, 12, 10, 6, 12, 1, 10, 12, 2, 1, 12, 11, 2, 12, 3, 11, 12, 0, 3, 12 },
		{ 6, 7, 12, 10, 6, 12, 1, 10, 12, 0, 1, 12, 8, 0, 12, 3, 8, 12, 2, 3, 12, 11, 2, 12, 7, 11, 12 },
		{ 12, 6, 7, 12, 7, 8, 12, 8, 0, 12, 0, 1, 12, 1, 10, 12, 10, 2, 12, 2, 3, 12, 3, 11, 12, 11, 6 }
	},
	{
		{ 10, 2, 12, 5, 10, 12, 4, 5, 12, 8, 4, 12, 3, 8, 12, 0, 3, 12, 9, 0, 12, 1, 9, 12, 2, 1, 12 },
		{ 4, 5, 12, 8, 4, 12, 3, 8, 12, 2, 3, 12, 10, 2, 12, 1, 10, 12, 0, 1, 12, 9, 0, 12, 5, 9, 12 },
		{ 12, 4, 5, 12, 5, 10, 12, 10, 2, 12, 2, 3, 12, 3, 8, 12, 8, 0, 12, 0, 1, 12, 1, 9, 12, 9, 4 } }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling7_4_1[16][15] =
{
	{ 3, 4, 8, 4, 3, 10, 2, 10, 3, 4, 10, 5, 9, 1, 0 },
	{ 1, 6, 10, 6, 1, 8, 0, 8, 1, 6, 8, 7, 11, 3, 2 },
	{ 11, 3, 6, 9, 6, 3, 6, 9, 5, 0, 9, 3, 7, 4, 8 },
	{ 2, 7, 11, 7, 2, 9, 1, 9, 2, 7, 9, 4, 8, 0, 3 },
	{ 0, 5, 9, 5, 0, 11, 3, 11, 0, 5, 11, 6, 10, 2, 1 },
	{ 8, 0, 7, 10, 7, 0, 7, 10, 6, 1, 10, 0, 4, 5, 9 },
	{ 9, 1, 4, 11, 4, 1, 4, 11, 7, 2, 11, 1, 5, 6, 10 },
	{ 10, 2, 5, 8, 5, 2, 5, 8, 4, 3, 8, 2, 6, 7, 11 },
	{ 5, 2, 10, 2, 5, 8, 4, 8, 5, 2, 8, 3, 11, 7, 6 },
	{ 4, 1, 9, 1, 4, 11, 7, 11, 4, 1, 11, 2, 10, 6, 5 },
	{ 7, 0, 8, 0, 7, 10, 6, 10, 7, 0, 10, 1, 9, 5, 4 },
	{ 9, 5, 0, 11, 0, 5, 0, 11, 3, 6, 11, 5, 1, 2, 10 },
	{ 11, 7, 2, 9, 2, 7, 2, 9, 1, 4, 9, 7, 3, 0, 8 },
	{ 6, 3, 11, 3, 6, 9, 5, 9, 6, 3, 9, 0, 8, 4, 7 },
	{ 10, 6, 1, 8, 1, 6, 1, 8, 0, 7, 8, 6, 2, 3, 11 },
	{ 8, 4, 3, 10, 3, 4, 3, 10, 2, 5, 10, 4, 0, 1, 9 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling7_4_2[16][27] =
{
	{ 9, 4, 8, 4, 9, 5, 10, 5, 9, 1, 10, 9, 10, 1, 2, 0, 2, 1, 2, 0, 3, 8, 3, 0, 9, 8, 0 },
	{ 11, 6, 10, 6, 11, 7, 8, 7, 11, 3, 8, 11, 8, 3, 0, 2, 0, 3, 0, 2, 1, 10, 1, 2, 11, 10, 2 },
	{ 11, 3, 8, 0, 8, 3, 8, 0, 9, 8, 9, 4, 5, 4, 9, 4, 5, 7, 6, 7, 5, 7, 6, 11, 7, 11, 8 },
	{ 8, 7, 11, 7, 8, 4, 9, 4, 8, 0, 9, 8, 9, 0, 1, 3, 1, 0, 1, 3, 2, 11, 2, 3, 8, 11, 3 },
	{ 10, 5, 9, 5, 10, 6, 11, 6, 10, 2, 11, 10, 11, 2, 3, 1, 3, 2, 3, 1, 0, 9, 0, 1, 10, 9, 1 },
	{ 8, 0, 9, 1, 9, 0, 9, 1, 10, 9, 10, 5, 6, 5, 10, 5, 6, 4, 7, 4, 6, 4, 7, 8, 4, 8, 9 },
	{ 9, 1, 10, 2, 10, 1, 10, 2, 11, 10, 11, 6, 7, 6, 11, 6, 7, 5, 4, 5, 7, 5, 4, 9, 5, 9, 10 },
	{ 10, 2, 11, 3, 11, 2, 11, 3, 8, 11, 8, 7, 4, 7, 8, 7, 4, 6, 5, 6, 4, 6, 5, 10, 6, 10, 11 },
	{ 11, 2, 10, 2, 11, 3, 8, 3, 11, 7, 8, 11, 8, 7, 4, 6, 4, 7, 4, 6, 5, 10, 5, 6, 11, 10, 6 },
	{ 10, 1, 9, 1, 10, 2, 11, 2, 10, 6, 11, 10, 11, 6, 7, 5, 7, 6, 7, 5, 4, 9, 4, 5, 10, 9, 5 },
	{ 9, 0, 8, 0, 9, 1, 10, 1, 9, 5, 10, 9, 10, 5, 6, 4, 6, 5, 6, 4, 7, 8, 7, 4, 9, 8, 4 },
	{ 9, 5, 10, 6, 10, 5, 10, 6, 11, 10, 11, 2, 3, 2, 11, 2, 3, 1, 0, 1, 3, 1, 0, 9, 1, 9, 10 },
	{ 11, 7, 8, 4, 8, 7, 8, 4, 9, 8, 9, 0, 1, 0, 9, 0, 1, 3, 2, 3, 1, 3, 2, 11, 3, 11, 8 },
	{ 8, 3, 11, 3, 8, 0, 9, 0, 8, 4, 9, 8, 9, 4, 5, 7, 5, 4, 5, 7, 6, 11, 6, 7, 8, 11, 7 },
	{ 10, 6, 11, 7, 11, 6, 11, 7, 8, 11, 8, 3, 0, 3, 8, 3, 0, 2, 1, 2, 0, 2, 1, 10, 2, 10, 11 },
	{ 8, 4, 9, 5, 9, 4, 9, 5, 10, 9, 10, 1, 2, 1, 10, 1, 2, 0, 3, 0, 2, 0, 3, 8, 0, 8, 9 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling8[6][6] =
{
	{ 9, 8, 10, 10, 8, 11 },
	{ 1, 5, 3, 3, 5, 7 },
	{ 0, 4, 2, 4, 6, 2 },
	{ 0, 2, 4, 4, 2, 6 },
	{ 1, 3, 5, 3, 7, 5 },
	{ 9, 10, 8, 10, 11, 8 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling9[8][12] =
{
	{ 2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8 },
	{ 4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1 },
	{ 10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8 },
	{ 3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5 },
	{ 3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9 },
	{ 10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0 },
	{ 4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2 },
	{ 2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_test10[6][3] =
{
	{ 2, 4, 7 },
	{ 5, 6, 7 },
	{ 1, 3, 7 },
	{ 1, 3, 7 },
	{ 5, 6, 7 },
	{ 2, 4, 7 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling10_1_1[6][12] =
{
	{ 5, 10, 7, 11, 7, 10, 8, 1, 9, 1, 8, 3 },
	{ 1, 2, 5, 6, 5, 2, 4, 3, 0, 3, 4, 7 },
	{ 11, 0, 8, 0, 11, 2, 4, 9, 6, 10, 6, 9 },
	{ 9, 0, 10, 2, 10, 0, 6, 8, 4, 8, 6, 11 },
	{ 7, 2, 3, 2, 7, 6, 0, 1, 4, 5, 4, 1 },
	{ 7, 9, 5, 9, 7, 8, 10, 1, 11, 3, 11, 1 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling10_1_1_[6][12] =
{
	{ 5, 9, 7, 8, 7, 9, 11, 1, 10, 1, 11, 3 },
	{ 3, 2, 7, 6, 7, 2, 4, 1, 0, 1, 4, 5 },
	{ 10, 0, 9, 0, 10, 2, 4, 8, 6, 11, 6, 8 },
	{ 8, 0, 11, 2, 11, 0, 6, 9, 4, 9, 6, 10 },
	{ 5, 2, 1, 2, 5, 6, 0, 3, 4, 7, 4, 3 },
	{ 7, 10, 5, 10, 7, 11, 9, 1, 8, 3, 8, 1 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling10_1_2[6][24] =
{
	{ 3, 11, 7, 3, 7, 8, 9, 8, 7, 5, 9, 7, 9, 5, 10, 9, 10, 1, 3, 1, 10, 11, 3, 10 },
	{ 7, 6, 5, 7, 5, 4, 0, 4, 5, 1, 0, 5, 0, 1, 2, 0, 2, 3, 7, 3, 2, 6, 7, 2 },
	{ 11, 2, 10, 6, 11, 10, 11, 6, 4, 11, 4, 8, 0, 8, 4, 9, 0, 4, 0, 9, 10, 0, 10, 2 },
	{ 11, 2, 10, 11, 10, 6, 4, 6, 10, 9, 4, 10, 4, 9, 0, 4, 0, 8, 11, 8, 0, 2, 11, 0 },
	{ 7, 6, 5, 4, 7, 5, 7, 4, 0, 7, 0, 3, 2, 3, 0, 1, 2, 0, 2, 1, 5, 2, 5, 6 },
	{ 7, 8, 3, 11, 7, 3, 7, 11, 10, 7, 10, 5, 9, 5, 10, 1, 9, 10, 9, 1, 3, 9, 3, 8 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling10_2[6][24] =
{
	{ 12, 5, 9, 12, 9, 8, 12, 8, 3, 12, 3, 1, 12, 1, 10, 12, 10, 11, 12, 11, 7, 12, 7, 5 },
	{ 12, 1, 0, 12, 0, 4, 12, 4, 7, 12, 7, 3, 12, 3, 2, 12, 2, 6, 12, 6, 5, 12, 5, 1 },
	{ 4, 8, 12, 6, 4, 12, 10, 6, 12, 9, 10, 12, 0, 9, 12, 2, 0, 12, 11, 2, 12, 8, 11, 12 },
	{ 12, 9, 4, 12, 4, 6, 12, 6, 11, 12, 11, 8, 12, 8, 0, 12, 0, 2, 12, 2, 10, 12, 10, 9 },
	{ 0, 3, 12, 4, 0, 12, 5, 4, 12, 1, 5, 12, 2, 1, 12, 6, 2, 12, 7, 6, 12, 3, 7, 12 },
	{ 10, 5, 12, 11, 10, 12, 3, 11, 12, 1, 3, 12, 9, 1, 12, 8, 9, 12, 7, 8, 12, 5, 7, 12 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling10_2_[6][24] =
{
	{ 8, 7, 12, 9, 8, 12, 1, 9, 12, 3, 1, 12, 11, 3, 12, 10, 11, 12, 5, 10, 12, 7, 5, 12 },
	{ 4, 5, 12, 0, 4, 12, 3, 0, 12, 7, 3, 12, 6, 7, 12, 2, 6, 12, 1, 2, 12, 5, 1, 12 },
	{ 12, 11, 6, 12, 6, 4, 12, 4, 9, 12, 9, 10, 12, 10, 2, 12, 2, 0, 12, 0, 8, 12, 8, 11 },
	{ 6, 10, 12, 4, 6, 12, 8, 4, 12, 11, 8, 12, 2, 11, 12, 0, 2, 12, 9, 0, 12, 10, 9, 12 },
	{ 12, 7, 4, 12, 4, 0, 12, 0, 1, 12, 1, 5, 12, 5, 6, 12, 6, 2, 12, 2, 3, 12, 3, 7 },
	{ 12, 7, 11, 12, 11, 10, 12, 10, 1, 12, 1, 3, 12, 3, 8, 12, 8, 9, 12, 9, 5, 12, 5, 7 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling11[12][12] =
{
	{ 2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4 },
	{ 1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6 },
	{ 8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10 },
	{ 0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6 },
	{ 9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11 },
	{ 5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0 },
	{ 5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3 },
	{ 9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7 },
	{ 0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11 },
	{ 8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1 },
	{ 1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7 },
	{ 2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_test12[24][4] =
{
	{ 4, 3, 7, 11 },
	{ 3, 2, 7, 10 },
	{ 2, 6, 7, 5 },
	{ 6, 4, 7, 7 },
	{ 2, 1, 7, 9 },
	{ 5, 2, 7, 1 },
	{ 5, 3, 7, 2 },
	{ 5, 1, 7, 0 },
	{ 5, 4, 7, 3 },
	{ 6, 3, 7, 6 },
	{ 1, 6, 7, 4 },
	{ 1, 4, 7, 8 },
	{ 4, 1, 7, 8 },
	{ 6, 1, 7, 4 },
	{ 3, 6, 7, 6 },
	{ 4, 5, 7, 3 },
	{ 1, 5, 7, 0 },
	{ 3, 5, 7, 2 },
	{ 2, 5, 7, 1 },
	{ 1, 2, 7, 9 },
	{ 4, 6, 7, 7 },
	{ 6, 2, 7, 5 },
	{ 2, 3, 7, 10 },
	{ 3, 4, 7, 11 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling12_1_1[24][12] =
{
	{ 7, 6, 11, 10, 3, 2, 3, 10, 8, 9, 8, 10 },
	{ 6, 5, 10, 9, 2, 1, 2, 9, 11, 8, 11, 9 },
	{ 10, 6, 5, 7, 9, 4, 9, 7, 1, 3, 1, 7 },
	{ 7, 6, 11, 4, 8, 5, 3, 5, 8, 5, 3, 1 },
	{ 5, 4, 9, 8, 1, 0, 1, 8, 10, 11, 10, 8 },
	{ 1, 2, 10, 0, 9, 3, 5, 3, 9, 3, 5, 7 },
	{ 10, 1, 2, 0, 11, 3, 11, 0, 6, 4, 6, 0 },
	{ 8, 3, 0, 2, 9, 1, 9, 2, 4, 6, 4, 2 },
	{ 3, 0, 8, 2, 11, 1, 7, 1, 11, 1, 7, 5 },
	{ 6, 5, 10, 7, 11, 4, 2, 4, 11, 4, 2, 0 },
	{ 9, 5, 4, 6, 8, 7, 8, 6, 0, 2, 0, 6 },
	{ 8, 3, 0, 7, 4, 11, 9, 11, 4, 11, 9, 10 },
	{ 4, 7, 8, 11, 0, 3, 0, 11, 9, 10, 9, 11 },
	{ 4, 7, 8, 5, 9, 6, 0, 6, 9, 6, 0, 2 },
	{ 11, 7, 6, 4, 10, 5, 10, 4, 2, 0, 2, 4 },
	{ 11, 2, 3, 1, 8, 0, 8, 1, 7, 5, 7, 1 },
	{ 0, 1, 9, 3, 8, 2, 4, 2, 8, 2, 4, 6 },
	{ 2, 3, 11, 1, 10, 0, 6, 0, 10, 0, 6, 4 },
	{ 9, 0, 1, 3, 10, 2, 10, 3, 5, 7, 5, 3 },
	{ 9, 0, 1, 4, 5, 8, 10, 8, 5, 8, 10, 11 },
	{ 8, 4, 7, 5, 11, 6, 11, 5, 3, 1, 3, 5 },
	{ 5, 4, 9, 6, 10, 7, 1, 7, 10, 7, 1, 3 },
	{ 10, 1, 2, 5, 6, 9, 11, 9, 6, 9, 11, 8 },
	{ 11, 2, 3, 6, 7, 10, 8, 10, 7, 10, 8, 9 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling12_1_1_[24][12] =
{
	{ 3, 2, 11, 10, 7, 6, 7, 10, 8, 9, 8, 10 },
	{ 2, 1, 10, 9, 6, 5, 6, 9, 11, 8, 11, 9 },
	{ 9, 4, 5, 7, 10, 6, 10, 7, 1, 3, 1, 7 },
	{ 7, 4, 8, 6, 11, 5, 3, 5, 11, 5, 3, 1 },
	{ 1, 0, 9, 8, 5, 4, 5, 8, 10, 11, 10, 8 },
	{ 1, 0, 9, 2, 10, 3, 5, 3, 10, 3, 5, 7 },
	{ 11, 3, 2, 0, 10, 1, 10, 0, 6, 4, 6, 0 },
	{ 9, 1, 0, 2, 8, 3, 8, 2, 4, 6, 4, 2 },
	{ 3, 2, 11, 0, 8, 1, 7, 1, 8, 1, 7, 5 },
	{ 6, 7, 11, 5, 10, 4, 2, 4, 10, 4, 2, 0 },
	{ 8, 7, 4, 6, 9, 5, 9, 6, 0, 2, 0, 6 },
	{ 8, 7, 4, 3, 0, 11, 9, 11, 0, 11, 9, 10 },
	{ 0, 3, 8, 11, 4, 7, 4, 11, 9, 10, 9, 11 },
	{ 4, 5, 9, 7, 8, 6, 0, 6, 8, 6, 0, 2 },
	{ 10, 5, 6, 4, 11, 7, 11, 4, 2, 0, 2, 4 },
	{ 8, 0, 3, 1, 11, 2, 11, 1, 7, 5, 7, 1 },
	{ 0, 3, 8, 1, 9, 2, 4, 2, 9, 2, 4, 6 },
	{ 2, 1, 10, 3, 11, 0, 6, 0, 11, 0, 6, 4 },
	{ 10, 2, 1, 3, 9, 0, 9, 3, 5, 7, 5, 3 },
	{ 9, 4, 5, 0, 1, 8, 10, 8, 1, 8, 10, 11 },
	{ 11, 6, 7, 5, 8, 4, 8, 5, 3, 1, 3, 5 },
	{ 5, 6, 10, 4, 9, 7, 1, 7, 9, 7, 1, 3 },
	{ 10, 5, 6, 1, 2, 9, 11, 9, 2, 9, 11, 8 },
	{ 11, 6, 7, 2, 3, 10, 8, 10, 3, 10, 8, 9 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling12_1_2[24][24] =
{
	{ 7, 3, 11, 3, 7, 8, 9, 8, 7, 6, 9, 7, 9, 6, 10, 2, 10, 6, 11, 2, 6, 2, 11, 3 },
	{ 6, 2, 10, 2, 6, 11, 8, 11, 6, 5, 8, 6, 8, 5, 9, 1, 9, 5, 10, 1, 5, 1, 10, 2 },
	{ 10, 9, 5, 9, 10, 1, 3, 1, 10, 6, 3, 10, 3, 6, 7, 4, 7, 6, 5, 4, 6, 4, 5, 9 },
	{ 7, 8, 11, 3, 11, 8, 11, 3, 1, 11, 1, 6, 5, 6, 1, 6, 5, 4, 6, 4, 7, 8, 7, 4 },
	{ 5, 1, 9, 1, 5, 10, 11, 10, 5, 4, 11, 5, 11, 4, 8, 0, 8, 4, 9, 0, 4, 0, 9, 1 },
	{ 1, 9, 10, 5, 10, 9, 10, 5, 7, 10, 7, 2, 3, 2, 7, 2, 3, 0, 2, 0, 1, 9, 1, 0 },
	{ 10, 11, 2, 11, 10, 6, 4, 6, 10, 1, 4, 10, 4, 1, 0, 3, 0, 1, 2, 3, 1, 3, 2, 11 },
	{ 8, 9, 0, 9, 8, 4, 6, 4, 8, 3, 6, 8, 6, 3, 2, 1, 2, 3, 0, 1, 3, 1, 0, 9 },
	{ 3, 11, 8, 7, 8, 11, 8, 7, 5, 8, 5, 0, 1, 0, 5, 0, 1, 2, 0, 2, 3, 11, 3, 2 },
	{ 6, 11, 10, 2, 10, 11, 10, 2, 0, 10, 0, 5, 4, 5, 0, 5, 4, 7, 5, 7, 6, 11, 6, 7 },
	{ 9, 8, 4, 8, 9, 0, 2, 0, 9, 5, 2, 9, 2, 5, 6, 7, 6, 5, 4, 7, 5, 7, 4, 8 },
	{ 8, 4, 0, 9, 0, 4, 0, 9, 10, 0, 10, 3, 11, 3, 10, 3, 11, 7, 3, 7, 8, 4, 8, 7 },
	{ 4, 0, 8, 0, 4, 9, 10, 9, 4, 7, 10, 4, 10, 7, 11, 3, 11, 7, 8, 3, 7, 3, 8, 0 },
	{ 4, 9, 8, 0, 8, 9, 8, 0, 2, 8, 2, 7, 6, 7, 2, 7, 6, 5, 7, 5, 4, 9, 4, 5 },
	{ 11, 10, 6, 10, 11, 2, 0, 2, 11, 7, 0, 11, 0, 7, 4, 5, 4, 7, 6, 5, 7, 5, 6, 10 },
	{ 11, 8, 3, 8, 11, 7, 5, 7, 11, 2, 5, 11, 5, 2, 1, 0, 1, 2, 3, 0, 2, 0, 3, 8 },
	{ 0, 8, 9, 4, 9, 8, 9, 4, 6, 9, 6, 1, 2, 1, 6, 1, 2, 3, 1, 3, 0, 8, 0, 3 },
	{ 2, 10, 11, 6, 11, 10, 11, 6, 4, 11, 4, 3, 0, 3, 4, 3, 0, 1, 3, 1, 2, 10, 2, 1 },
	{ 9, 10, 1, 10, 9, 5, 7, 5, 9, 0, 7, 9, 7, 0, 3, 2, 3, 0, 1, 2, 0, 2, 1, 10 },
	{ 9, 5, 1, 10, 1, 5, 1, 10, 11, 1, 11, 0, 8, 0, 11, 0, 8, 4, 0, 4, 9, 5, 9, 4 },
	{ 8, 11, 7, 11, 8, 3, 1, 3, 8, 4, 1, 8, 1, 4, 5, 6, 5, 4, 7, 6, 4, 6, 7, 11 },
	{ 5, 10, 9, 1, 9, 10, 9, 1, 3, 9, 3, 4, 7, 4, 3, 4, 7, 6, 4, 6, 5, 10, 5, 6 },
	{ 10, 6, 2, 11, 2, 6, 2, 11, 8, 2, 8, 1, 9, 1, 8, 1, 9, 5, 1, 5, 10, 6, 10, 5 },
	{ 11, 7, 3, 8, 3, 7, 3, 8, 9, 3, 9, 2, 10, 2, 9, 2, 10, 6, 2, 6, 11, 7, 11, 6 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling12_2[24][24] =
{
	{ 9, 8, 12, 10, 9, 12, 2, 10, 12, 3, 2, 12, 11, 3, 12, 6, 11, 12, 7, 6, 12, 8, 7, 12 },
	{ 8, 11, 12, 9, 8, 12, 1, 9, 12, 2, 1, 12, 10, 2, 12, 5, 10, 12, 6, 5, 12, 11, 6, 12 },
	{ 3, 1, 12, 7, 3, 12, 4, 7, 12, 9, 4, 12, 5, 9, 12, 6, 5, 12, 10, 6, 12, 1, 10, 12 },
	{ 12, 3, 1, 12, 1, 5, 12, 5, 6, 12, 6, 11, 12, 11, 7, 12, 7, 4, 12, 4, 8, 12, 8, 3 },
	{ 11, 10, 12, 8, 11, 12, 0, 8, 12, 1, 0, 12, 9, 1, 12, 4, 9, 12, 5, 4, 12, 10, 5, 12 },
	{ 12, 5, 7, 12, 7, 3, 12, 3, 2, 12, 2, 10, 12, 10, 1, 12, 1, 0, 12, 0, 9, 12, 9, 5 },
	{ 4, 6, 12, 0, 4, 12, 1, 0, 12, 10, 1, 12, 2, 10, 12, 3, 2, 12, 11, 3, 12, 6, 11, 12 },
	{ 6, 4, 12, 2, 6, 12, 3, 2, 12, 8, 3, 12, 0, 8, 12, 1, 0, 12, 9, 1, 12, 4, 9, 12 },
	{ 12, 7, 5, 12, 5, 1, 12, 1, 0, 12, 0, 8, 12, 8, 3, 12, 3, 2, 12, 2, 11, 12, 11, 7 },
	{ 12, 2, 0, 12, 0, 4, 12, 4, 5, 12, 5, 10, 12, 10, 6, 12, 6, 7, 12, 7, 11, 12, 11, 2 },
	{ 2, 0, 12, 6, 2, 12, 7, 6, 12, 8, 7, 12, 4, 8, 12, 5, 4, 12, 9, 5, 12, 0, 9, 12 },
	{ 12, 9, 10, 12, 10, 11, 12, 11, 7, 12, 7, 4, 12, 4, 8, 12, 8, 3, 12, 3, 0, 12, 0, 9 },
	{ 10, 9, 12, 11, 10, 12, 7, 11, 12, 4, 7, 12, 8, 4, 12, 3, 8, 12, 0, 3, 12, 9, 0, 12 },
	{ 12, 0, 2, 12, 2, 6, 12, 6, 7, 12, 7, 8, 12, 8, 4, 12, 4, 5, 12, 5, 9, 12, 9, 0 },
	{ 0, 2, 12, 4, 0, 12, 5, 4, 12, 10, 5, 12, 6, 10, 12, 7, 6, 12, 11, 7, 12, 2, 11, 12 },
	{ 5, 7, 12, 1, 5, 12, 0, 1, 12, 8, 0, 12, 3, 8, 12, 2, 3, 12, 11, 2, 12, 7, 11, 12 },
	{ 12, 4, 6, 12, 6, 2, 12, 2, 3, 12, 3, 8, 12, 8, 0, 12, 0, 1, 12, 1, 9, 12, 9, 4 },
	{ 12, 6, 4, 12, 4, 0, 12, 0, 1, 12, 1, 10, 12, 10, 2, 12, 2, 3, 12, 3, 11, 12, 11, 6 },
	{ 7, 5, 12, 3, 7, 12, 2, 3, 12, 10, 2, 12, 1, 10, 12, 0, 1, 12, 9, 0, 12, 5, 9, 12 },
	{ 12, 10, 11, 12, 11, 8, 12, 8, 0, 12, 0, 1, 12, 1, 9, 12, 9, 4, 12, 4, 5, 12, 5, 10 },
	{ 1, 3, 12, 5, 1, 12, 6, 5, 12, 11, 6, 12, 7, 11, 12, 4, 7, 12, 8, 4, 12, 3, 8, 12 },
	{ 12, 1, 3, 12, 3, 7, 12, 7, 4, 12, 4, 9, 12, 9, 5, 12, 5, 6, 12, 6, 10, 12, 10, 1 },
	{ 12, 11, 8, 12, 8, 9, 12, 9, 1, 12, 1, 2, 12, 2, 10, 12, 10, 5, 12, 5, 6, 12, 6, 11 },
	{ 12, 8, 9, 12, 9, 10, 12, 10, 2, 12, 2, 3, 12, 3, 11, 12, 11, 6, 12, 6, 7, 12, 7, 8 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling12_2_[24][24] =
{
	{ 12, 2, 11, 12, 11, 7, 12, 7, 6, 12, 6, 10, 12, 10, 9, 12, 9, 8, 12, 8, 3, 12, 3, 2 },
	{ 12, 1, 10, 12, 10, 6, 12, 6, 5, 12, 5, 9, 12, 9, 8, 12, 8, 11, 12, 11, 2, 12, 2, 1 },
	{ 12, 4, 5, 12, 5, 10, 12, 10, 6, 12, 6, 7, 12, 7, 3, 12, 3, 1, 12, 1, 9, 12, 9, 4 },
	{ 7, 6, 12, 8, 7, 12, 4, 8, 12, 5, 4, 12, 1, 5, 12, 3, 1, 12, 11, 3, 12, 6, 11, 12 },
	{ 12, 0, 9, 12, 9, 5, 12, 5, 4, 12, 4, 8, 12, 8, 11, 12, 11, 10, 12, 10, 1, 12, 1, 0 },
	{ 1, 2, 12, 9, 1, 12, 0, 9, 12, 3, 0, 12, 7, 3, 12, 5, 7, 12, 10, 5, 12, 2, 10, 12 },
	{ 12, 1, 2, 12, 2, 11, 12, 11, 3, 12, 3, 0, 12, 0, 4, 12, 4, 6, 12, 6, 10, 12, 10, 1 },
	{ 12, 3, 0, 12, 0, 9, 12, 9, 1, 12, 1, 2, 12, 2, 6, 12, 6, 4, 12, 4, 8, 12, 8, 3 },
	{ 3, 0, 12, 11, 3, 12, 2, 11, 12, 1, 2, 12, 5, 1, 12, 7, 5, 12, 8, 7, 12, 0, 8, 12 },
	{ 6, 5, 12, 11, 6, 12, 7, 11, 12, 4, 7, 12, 0, 4, 12, 2, 0, 12, 10, 2, 12, 5, 10, 12 },
	{ 12, 7, 4, 12, 4, 9, 12, 9, 5, 12, 5, 6, 12, 6, 2, 12, 2, 0, 12, 0, 8, 12, 8, 7 },
	{ 8, 7, 12, 0, 8, 12, 3, 0, 12, 11, 3, 12, 10, 11, 12, 9, 10, 12, 4, 9, 12, 7, 4, 12 },
	{ 12, 7, 8, 12, 8, 0, 12, 0, 3, 12, 3, 11, 12, 11, 10, 12, 10, 9, 12, 9, 4, 12, 4, 7 },
	{ 4, 7, 12, 9, 4, 12, 5, 9, 12, 6, 5, 12, 2, 6, 12, 0, 2, 12, 8, 0, 12, 7, 8, 12 },
	{ 12, 5, 6, 12, 6, 11, 12, 11, 7, 12, 7, 4, 12, 4, 0, 12, 0, 2, 12, 2, 10, 12, 10, 5 },
	{ 12, 0, 3, 12, 3, 11, 12, 11, 2, 12, 2, 1, 12, 1, 5, 12, 5, 7, 12, 7, 8, 12, 8, 0 },
	{ 0, 3, 12, 9, 0, 12, 1, 9, 12, 2, 1, 12, 6, 2, 12, 4, 6, 12, 8, 4, 12, 3, 8, 12 },
	{ 2, 1, 12, 11, 2, 12, 3, 11, 12, 0, 3, 12, 4, 0, 12, 6, 4, 12, 10, 6, 12, 1, 10, 12 },
	{ 12, 2, 1, 12, 1, 9, 12, 9, 0, 12, 0, 3, 12, 3, 7, 12, 7, 5, 12, 5, 10, 12, 10, 2 },
	{ 9, 0, 12, 5, 9, 12, 4, 5, 12, 8, 4, 12, 11, 8, 12, 10, 11, 12, 1, 10, 12, 0, 1, 12 },
	{ 12, 6, 7, 12, 7, 8, 12, 8, 4, 12, 4, 5, 12, 5, 1, 12, 1, 3, 12, 3, 11, 12, 11, 6 },
	{ 5, 4, 12, 10, 5, 12, 6, 10, 12, 7, 6, 12, 3, 7, 12, 1, 3, 12, 9, 1, 12, 4, 9, 12 },
	{ 10, 1, 12, 6, 10, 12, 5, 6, 12, 9, 5, 12, 8, 9, 12, 11, 8, 12, 2, 11, 12, 1, 2, 12 },
	{ 11, 2, 12, 7, 11, 12, 6, 7, 12, 10, 6, 12, 9, 10, 12, 8, 9, 12, 3, 8, 12, 2, 3, 12 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_test13[2][7] =
{
	{ 1,2,3,4,5,6,7 },
	{ 2,3,4,1,5,6,7 },
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_subconfig13[64] =
{
	0,
	1,
	2,
	7,
	3,
	-1,
	11,
	-1,
	4,
	8,
	-1,
	-1,
	14,
	-1,
	-1,
	-1,
	5,
	9,
	12,
	23,
	15,
	-1,
	21,
	38,
	17,
	20,
	-1,
	36,
	26,
	33,
	30,
	44,
	6,
	10,
	13,
	19,
	16,
	-1,
	25,
	37,
	18,
	24,
	-1,
	35,
	22,
	32,
	29,
	43,
	-1,
	-1,
	-1,
	34,
	-1,
	-1,
	28,
	42,
	-1,
	31,
	-1,
	41,
	27,
	40,
	39,
	45,
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling13_1[2][12] =
{
	{ 11, 7, 6, 1, 2, 10, 8, 3, 0, 9, 5, 4 },
	{ 8, 4, 7, 2, 3, 11, 9, 0, 1, 10, 6, 5 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling13_1_[2][12] =
{
	{ 7, 4, 8, 11, 3, 2, 1, 0, 9, 5, 6, 10 },
	{ 6, 7, 11, 10, 2, 1, 0, 3, 8, 4, 5, 9 }
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling13_2[2][6][18] =
{
	{
		{ 1, 2, 10, 11, 7, 6, 3, 4, 8, 4, 3, 5, 0, 5, 3, 5, 0, 9 },
		{ 8, 3, 0, 11, 7, 6, 9, 1, 4, 2, 4, 1, 4, 2, 5, 10, 5, 2 },
		{ 9, 5, 4, 8, 3, 0, 1, 6, 10, 6, 1, 7, 2, 7, 1, 7, 2, 11 },
		{ 9, 5, 4, 1, 2, 10, 11, 3, 6, 0, 6, 3, 6, 0, 7, 8, 7, 0 },
		{ 9, 5, 4, 11, 7, 6, 0, 10, 1, 10, 0, 8, 10, 8, 2, 3, 2, 8 },
		{ 1, 2, 10, 3, 0, 8, 4, 9, 7, 11, 7, 9, 5, 11, 9, 11, 5, 6 }
	},
	{
		{ 2, 3, 11, 8, 4, 7, 0, 5, 9, 5, 0, 6, 1, 6, 0, 6, 1, 10 },
		{ 9, 0, 1, 8, 4, 7, 10, 2, 5, 3, 5, 2, 5, 3, 6, 11, 6, 3 },
		{ 6, 5, 10, 9, 0, 1, 2, 7, 11, 7, 2, 4, 3, 4, 2, 4, 3, 8 },
		{ 6, 5, 10, 2, 3, 11, 8, 0, 7, 1, 7, 0, 7, 1, 4, 9, 4, 1 },
		{ 6, 5, 10, 8, 4, 7, 1, 11, 2, 11, 1, 9, 11, 9, 3, 0, 3, 9 },
		{ 2, 3, 11, 0, 1, 9, 5, 10, 4, 8, 4, 10, 6, 8, 10, 8, 6, 7 }
	}
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling13_2_[2][6][18] =
{
	{
		{ 10, 5, 6, 11, 3, 2, 7, 0, 8, 0, 7, 1, 4, 1, 7, 1, 4, 9 },
		{ 11, 3, 2, 7, 4, 8, 9, 5, 0, 6, 0, 5, 0, 6, 1, 10, 1, 6 },
		{ 1, 0, 9, 7, 4, 8, 5, 2, 10, 2, 5, 3, 6, 3, 5, 3, 6, 11 },
		{ 10, 5, 6, 1, 0, 9, 11, 7, 2, 4, 2, 7, 2, 4, 3, 8, 3, 4 },
		{ 10, 5, 6, 7, 4, 8, 2, 11, 1, 9, 1, 11, 3, 9, 11, 9, 3, 0 },
		{ 11, 3, 2, 9, 1, 0, 4, 10, 5, 10, 4, 8, 10, 8, 6, 7, 6, 8 }
	},
	{
		{ 6, 7, 11, 8, 0, 3, 4, 1, 9, 1, 4, 2, 5, 2, 4, 2, 5, 10 },
		{ 8, 0, 3, 4, 5, 9, 10, 6, 1, 7, 1, 6, 1, 7, 2, 11, 2, 7 },
		{ 2, 1, 10, 4, 5, 9, 6, 3, 11, 3, 6, 0, 7, 0, 6, 0, 7, 8 },
		{ 6, 7, 11, 2, 1, 10, 8, 4, 3, 5, 3, 4, 3, 5, 0, 9, 0, 5 },
		{ 6, 7, 11, 4, 5, 9, 3, 8, 2, 10, 2, 8, 0, 10, 8, 10, 0, 1 },
		{ 8, 0, 3, 10, 2, 1, 5, 11, 6, 11, 5, 9, 11, 9, 7, 4, 7, 9 }
	}
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling13_3[2][12][30] =
{
	{
		{ 11, 7, 6, 12, 2, 10, 12, 10, 5, 12, 5, 4, 12, 4, 8, 12, 8, 3, 12, 3, 0, 12, 0, 9, 12, 9, 1, 12, 1, 2 },
		{ 1, 2, 10, 9, 5, 12, 0, 9, 12, 3, 0, 12, 11, 3, 12, 6, 11, 12, 7, 6, 12, 8, 7, 12, 4, 8, 12, 5, 4, 12 },
		{ 11, 7, 6, 12, 5, 4, 12, 4, 8, 12, 8, 3, 12, 3, 2, 12, 2, 10, 12, 10, 1, 12, 1, 0, 12, 0, 9, 12, 9, 5 },
		{ 1, 2, 10, 12, 3, 0, 12, 0, 9, 12, 9, 5, 12, 5, 6, 12, 6, 11, 12, 11, 7, 12, 7, 4, 12, 4, 8, 12, 8, 3 },
		{ 8, 3, 0, 11, 7, 12, 2, 11, 12, 1, 2, 12, 9, 1, 12, 4, 9, 12, 5, 4, 12, 10, 5, 12, 6, 10, 12, 7, 6, 12 },
		{ 11, 7, 6, 5, 4, 12, 10, 5, 12, 2, 10, 12, 3, 2, 12, 8, 3, 12, 0, 8, 12, 1, 0, 12, 9, 1, 12, 4, 9, 12 },
		{ 8, 3, 0, 1, 2, 12, 9, 1, 12, 4, 9, 12, 7, 4, 12, 11, 7, 12, 6, 11, 12, 5, 6, 12, 10, 5, 12, 2, 10, 12 },
		{ 9, 5, 4, 12, 0, 8, 12, 8, 7, 12, 7, 6, 12, 6, 10, 12, 10, 1, 12, 1, 2, 12, 2, 11, 12, 11, 3, 12, 3, 0 },
		{ 9, 5, 4, 12, 7, 6, 12, 6, 10, 12, 10, 1, 12, 1, 0, 12, 0, 8, 12, 8, 3, 12, 3, 2, 12, 2, 11, 12, 11, 7 },
		{ 8, 3, 0, 12, 1, 2, 12, 2, 11, 12, 11, 7, 12, 7, 4, 12, 4, 9, 12, 9, 5, 12, 5, 6, 12, 6, 10, 12, 10, 1 },
		{ 9, 5, 4, 7, 6, 12, 8, 7, 12, 0, 8, 12, 1, 0, 12, 10, 1, 12, 2, 10, 12, 3, 2, 12, 11, 3, 12, 6, 11, 12 },
		{ 1, 2, 10, 3, 0, 12, 11, 3, 12, 6, 11, 12, 5, 6, 12, 9, 5, 12, 4, 9, 12, 7, 4, 12, 8, 7, 12, 0, 8, 12 }
	},
	{
		{ 8, 4, 7, 12, 3, 11, 12, 11, 6, 12, 6, 5, 12, 5, 9, 12, 9, 0, 12, 0, 1, 12, 1, 10, 12, 10, 2, 12, 2, 3 },
		{ 2, 3, 11, 10, 6, 12, 1, 10, 12, 0, 1, 12, 8, 0, 12, 7, 8, 12, 4, 7, 12, 9, 4, 12, 5, 9, 12, 6, 5, 12 },
		{ 8, 4, 7, 12, 6, 5, 12, 5, 9, 12, 9, 0, 12, 0, 3, 12, 3, 11, 12, 11, 2, 12, 2, 1, 12, 1, 10, 12, 10, 6 },
		{ 2, 3, 11, 12, 0, 1, 12, 1, 10, 12, 10, 6, 12, 6, 7, 12, 7, 8, 12, 8, 4, 12, 4, 5, 12, 5, 9, 12, 9, 0 },
		{ 0, 1, 9, 8, 4, 12, 3, 8, 12, 2, 3, 12, 10, 2, 12, 5, 10, 12, 6, 5, 12, 11, 6, 12, 7, 11, 12, 4, 7, 12 },
		{ 8, 4, 7, 6, 5, 12, 11, 6, 12, 3, 11, 12, 0, 3, 12, 9, 0, 12, 1, 9, 12, 2, 1, 12, 10, 2, 12, 5, 10, 12 },
		{ 9, 0, 1, 2, 3, 12, 10, 2, 12, 5, 10, 12, 4, 5, 12, 8, 4, 12, 7, 8, 12, 6, 7, 12, 11, 6, 12, 3, 11, 12 },
		{ 6, 5, 10, 12, 1, 9, 12, 9, 4, 12, 4, 7, 12, 7, 11, 12, 11, 2, 12, 2, 3, 12, 3, 8, 12, 8, 0, 12, 0, 1 },
		{ 6, 5, 10, 12, 4, 7, 12, 7, 11, 12, 11, 2, 12, 2, 1, 12, 1, 9, 12, 9, 0, 12, 0, 3, 12, 3, 8, 12, 8, 4 },
		{ 9, 0, 1, 12, 2, 3, 12, 3, 8, 12, 8, 4, 12, 4, 5, 12, 5, 10, 12, 10, 6, 12, 6, 7, 12, 7, 11, 12, 11, 2 },
		{ 6, 5, 10, 4, 7, 12, 9, 4, 12, 1, 9, 12, 2, 1, 12, 11, 2, 12, 3, 11, 12, 0, 3, 12, 8, 0, 12, 7, 8, 12 },
		{ 2, 3, 11, 0, 1, 12, 8, 0, 12, 7, 8, 12, 6, 7, 12, 10, 6, 12, 5, 10, 12, 4, 5, 12, 9, 4, 12, 1, 9, 12 }
	}
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling13_3_[2][12][30] =
{
	{
		{ 3, 2, 11, 8, 7, 12, 0, 8, 12, 1, 0, 12, 10, 1, 12, 6, 10, 12, 5, 6, 12, 9, 5, 12, 4, 9, 12, 7, 4, 12 },
		{ 5, 6, 10, 12, 2, 11, 12, 11, 7, 12, 7, 4, 12, 4, 9, 12, 9, 1, 12, 1, 0, 12, 0, 8, 12, 8, 3, 12, 3, 2 },
		{ 10, 5, 6, 12, 7, 4, 12, 4, 9, 12, 9, 1, 12, 1, 2, 12, 2, 11, 12, 11, 3, 12, 3, 0, 12, 0, 8, 12, 8, 7 },
		{ 11, 3, 2, 12, 1, 0, 12, 0, 8, 12, 8, 7, 12, 7, 6, 12, 6, 10, 12, 10, 5, 12, 5, 4, 12, 4, 9, 12, 9, 1 },
		{ 7, 4, 8, 11, 3, 12, 6, 11, 12, 5, 6, 12, 9, 5, 12, 0, 9, 12, 1, 0, 12, 10, 1, 12, 2, 10, 12, 3, 2, 12 },
		{ 7, 4, 8, 5, 6, 12, 9, 5, 12, 0, 9, 12, 3, 0, 12, 11, 3, 12, 2, 11, 12, 1, 2, 12, 10, 1, 12, 6, 10, 12 },
		{ 11, 3, 2, 1, 0, 12, 10, 1, 12, 6, 10, 12, 7, 6, 12, 8, 7, 12, 4, 8, 12, 5, 4, 12, 9, 5, 12, 0, 9, 12 },
		{ 1, 0, 9, 12, 4, 8, 12, 8, 3, 12, 3, 2, 12, 2, 10, 12, 10, 5, 12, 5, 6, 12, 6, 11, 12, 11, 7, 12, 7, 4 },
		{ 7, 4, 8, 12, 5, 6, 12, 6, 11, 12, 11, 3, 12, 3, 0, 12, 0, 9, 12, 9, 1, 12, 1, 2, 12, 2, 10, 12, 10, 5 },
		{ 1, 0, 9, 12, 3, 2, 12, 2, 10, 12, 10, 5, 12, 5, 4, 12, 4, 8, 12, 8, 7, 12, 7, 6, 12, 6, 11, 12, 11, 3 },
		{ 10, 5, 6, 7, 4, 12, 11, 7, 12, 2, 11, 12, 1, 2, 12, 9, 1, 12, 0, 9, 12, 3, 0, 12, 8, 3, 12, 4, 8, 12 },
		{ 9, 1, 0, 3, 2, 12, 8, 3, 12, 4, 8, 12, 5, 4, 12, 10, 5, 12, 6, 10, 12, 7, 6, 12, 11, 7, 12, 2, 11, 12 }
	},
	{
		{ 0, 3, 8, 9, 4, 12, 1, 9, 12, 2, 1, 12, 11, 2, 12, 7, 11, 12, 6, 7, 12, 10, 6, 12, 5, 10, 12, 4, 5, 12 },
		{ 11, 6, 7, 12, 3, 8, 12, 8, 4, 12, 4, 5, 12, 5, 10, 12, 10, 2, 12, 2, 1, 12, 1, 9, 12, 9, 0, 12, 0, 3 },
		{ 6, 7, 11, 12, 4, 5, 12, 5, 10, 12, 10, 2, 12, 2, 3, 12, 3, 8, 12, 8, 0, 12, 0, 1, 12, 1, 9, 12, 9, 4 },
		{ 8, 0, 3, 12, 2, 1, 12, 1, 9, 12, 9, 4, 12, 4, 7, 12, 7, 11, 12, 11, 6, 12, 6, 5, 12, 5, 10, 12, 10, 2 },
		{ 4, 5, 9, 8, 0, 12, 7, 8, 12, 6, 7, 12, 10, 6, 12, 1, 10, 12, 2, 1, 12, 11, 2, 12, 3, 11, 12, 0, 3, 12 },
		{ 4, 5, 9, 6, 7, 12, 10, 6, 12, 1, 10, 12, 0, 1, 12, 8, 0, 12, 3, 8, 12, 2, 3, 12, 11, 2, 12, 7, 11, 12 },
		{ 8, 0, 3, 2, 1, 12, 11, 2, 12, 7, 11, 12, 4, 7, 12, 9, 4, 12, 5, 9, 12, 6, 5, 12, 10, 6, 12, 1, 10, 12 },
		{ 2, 1, 10, 12, 5, 9, 12, 9, 0, 12, 0, 3, 12, 3, 11, 12, 11, 6, 12, 6, 7, 12, 7, 8, 12, 8, 4, 12, 4, 5 },
		{ 4, 5, 9, 12, 6, 7, 12, 7, 8, 12, 8, 0, 12, 0, 1, 12, 1, 10, 12, 10, 2, 12, 2, 3, 12, 3, 11, 12, 11, 6 },
		{ 2, 1, 10, 12, 0, 3, 12, 3, 11, 12, 11, 6, 12, 6, 5, 12, 5, 9, 12, 9, 4, 12, 4, 7, 12, 7, 8, 12, 8, 0 },
		{ 6, 7, 11, 4, 5, 12, 8, 4, 12, 3, 8, 12, 2, 3, 12, 10, 2, 12, 1, 10, 12, 0, 1, 12, 9, 0, 12, 5, 9, 12 },
		{ 10, 2, 1, 0, 3, 12, 9, 0, 12, 5, 9, 12, 6, 5, 12, 11, 6, 12, 7, 11, 12, 4, 7, 12, 8, 4, 12, 3, 8, 12 }
	}
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling13_4[2][4][36] =
{
	{
		{ 12, 2, 10, 12, 10, 5, 12, 5, 6, 12, 6, 11, 12, 11, 7, 12, 7, 4, 12, 4, 8, 12, 8, 3, 12, 3, 0, 12, 0, 9, 12, 9, 1, 12, 1, 2 },
		{ 11, 3, 12, 6, 11, 12, 7, 6, 12, 8, 7, 12, 4, 8, 12, 5, 4, 12, 9, 5, 12, 0, 9, 12, 1, 0, 12, 10, 1, 12, 2, 10, 12, 3, 2, 12 },
		{ 9, 1, 12, 4, 9, 12, 5, 4, 12, 10, 5, 12, 6, 10, 12, 7, 6, 12, 11, 7, 12, 2, 11, 12, 3, 2, 12, 8, 3, 12, 0, 8, 12, 1, 0, 12 },
		{ 12, 0, 8, 12, 8, 7, 12, 7, 4, 12, 4, 9, 12, 9, 5, 12, 5, 6, 12, 6, 10, 12, 10, 1, 12, 1, 2, 12, 2, 11, 12, 11, 3, 12, 3, 0 }
	},
	{
		{ 12, 3, 11, 12, 11, 6, 12, 6, 7, 12, 7, 8, 12, 8, 4, 12, 4, 5, 12, 5, 9, 12, 9, 0, 12, 0, 1, 12, 1, 10, 12, 10, 2, 12, 2, 3 },
		{ 8, 0, 12, 7, 8, 12, 4, 7, 12, 9, 4, 12, 5, 9, 12, 6, 5, 12, 10, 6, 12, 1, 10, 12, 2, 1, 12, 11, 2, 12, 3, 11, 12, 0, 3, 12 },
		{ 10, 2, 12, 5, 10, 12, 6, 5, 12, 11, 6, 12, 7, 11, 12, 4, 7, 12, 8, 4, 12, 3, 8, 12, 0, 3, 12, 9, 0, 12, 1, 9, 12, 2, 1, 12 },
		{ 12, 1, 9, 12, 9, 4, 12, 4, 5, 12, 5, 10, 12, 10, 6, 12, 6, 7, 12, 7, 11, 12, 11, 2, 12, 2, 3, 12, 3, 8, 12, 8, 0, 12, 0, 1 }
	}
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling13_5_1[2][4][18] =
{
	{
		{ 7, 6, 11, 1, 0, 9, 10, 3, 2, 3, 10, 5, 3, 5, 8, 4, 8, 5 },
		{ 1, 2, 10, 7, 4, 8, 3, 0, 11, 6, 11, 0, 9, 6, 0, 6, 9, 5 },
		{ 3, 0, 8, 5, 6, 10, 1, 2, 9, 4, 9, 2, 11, 4, 2, 4, 11, 7 },
		{ 5, 4, 9, 3, 2, 11, 8, 1, 0, 1, 8, 7, 1, 7, 10, 6, 10, 7 }
	},
	{
		{ 4, 7, 8, 2, 1, 10, 11, 0, 3, 0, 11, 6, 0, 6, 9, 5, 9, 6 },
		{ 2, 3, 11, 4, 5, 9, 0, 1, 8, 7, 8, 1, 10, 7, 1, 7, 10, 6 },
		{ 0, 1, 9, 6, 7, 11, 2, 3, 10, 5, 10, 3, 8, 5, 3, 5, 8, 4 },
		{ 6, 5, 10, 0, 3, 8, 9, 2, 1, 2, 9, 4, 2, 4, 11, 7, 11, 4 }
	}
};
template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling13_5_2[2][4][30] =
{
	{
		{ 1, 0, 9, 7, 4, 8, 7, 8, 3, 7, 3, 11, 2, 11, 3, 11, 2, 10, 11, 10, 6, 5, 6, 10, 6, 5, 7, 4, 7, 5 },
		{ 7, 4, 8, 11, 3, 2, 6, 11, 2, 10, 6, 2, 6, 10, 5, 9, 5, 10, 1, 9, 10, 9, 1, 0, 2, 0, 1, 0, 2, 3 },
		{ 5, 6, 10, 9, 1, 0, 4, 9, 0, 8, 4, 0, 4, 8, 7, 11, 7, 8, 3, 11, 8, 11, 3, 2, 0, 2, 3, 2, 0, 1 },
		{ 3, 2, 11, 5, 6, 10, 5, 10, 1, 5, 1, 9, 0, 9, 1, 9, 0, 8, 9, 8, 4, 4, 8, 7, 4, 7, 5, 6, 5, 7 }
	},
	{
		{ 2, 1, 10, 4, 5, 9, 4, 9, 0, 4, 0, 8, 3, 8, 0, 8, 3, 11, 8, 11, 7, 6, 7, 11, 7, 6, 4, 5, 4, 6 },
		{ 4, 5, 9, 8, 0, 3, 7, 8, 3, 11, 7, 3, 7, 11, 6, 10, 6, 11, 2, 10, 11, 10, 2, 1, 3, 1, 2, 1, 3, 0 },
		{ 6, 7, 11, 10, 2, 1, 5, 10, 1, 9, 5, 1, 5, 9, 4, 8, 4, 9, 0, 8, 9, 8, 0, 3, 1, 3, 0, 3, 1, 2 },
		{ 0, 3, 8, 6, 7, 11, 6, 11, 2, 6, 2, 10, 1, 10, 2, 10, 1, 9, 10, 9, 5, 5, 9, 4, 5, 4, 6, 7, 6, 4 }
	}
};

template< typename ImplicitFn, typename MeshBuilder >
const char MarchingCubes<ImplicitFn, MeshBuilder>::g_tiling14[12][12] =
{
	{ 5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8 },
	{ 2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5 },
	{ 9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6 },
	{ 1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4 },
	{ 8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5 },
	{ 0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10 },
	{ 0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7 },
	{ 8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2 },
	{ 1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11 },
	{ 9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3 },
	{ 2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8 },
	{ 5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2 }
};


}
