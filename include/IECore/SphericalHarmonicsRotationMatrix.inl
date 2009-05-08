//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

namespace IECore
{

template < typename V >
SphericalHarmonicsRotationMatrix<V>::SphericalHarmonicsRotationMatrix() :
		m_bands(0),
		m_newRotation(true)
{
}

template < typename V >
template <class S>
SphericalHarmonicsRotationMatrix<V>::SphericalHarmonicsRotationMatrix( const Imath::Vec3<S>& r ) :
		m_bands(0),
		m_newRotation(true)
{
	m_3dRotation.setEulerAngles( r );
}

template < typename V >
template <class S>
void SphericalHarmonicsRotationMatrix<V>::setEulerAngles( const Imath::Vec3<S>& r )
{
	m_3dRotation.setEulerAngles( r );
	m_newRotation = true;
}

template < typename V >
template <class S>
void SphericalHarmonicsRotationMatrix<V>::setAxisAngle ( const Imath::Vec3<S>& ax, S ang)
{
	m_3dRotation.setAxisAngle( ax, ang );
	m_newRotation = true;
}

template < typename V >
template <class S>
void SphericalHarmonicsRotationMatrix<V>::setQuaternion( const Imath::Quat<S>& q )
{
	m_3dRotation = q.toMatrix44();
	m_newRotation = true;
}

template < typename V >
template <class S>
void SphericalHarmonicsRotationMatrix<V>::setRotation( const Imath::Matrix44<S>& m )
{
	m_3dRotation = m;
	m_newRotation = true;
}

template < typename V >
template< typename U >
void SphericalHarmonicsRotationMatrix<V>::transform( SphericalHarmonics<U> &sh ) const
{
	if ( sh.bands() != m_bands || m_newRotation )
	{
		m_bands = sh.bands();
		m_newRotation = false;
		computeSquareRoots( );
		computeComplexRotation( );
		computeRealRotation( );
	}
	applyRotation( sh );
}

template < typename V >
template< typename U >
void SphericalHarmonicsRotationMatrix<V>::applyRotation( SphericalHarmonics<U> &sh ) const
{
	int offset;
	typename SphericalHarmonics<U>::CoefficientVector::iterator cv = sh.coefficients().begin();
	std::vector<U> tmp( sh.coefficients() );
	typename std::vector<U>::iterator it = tmp.begin();

	for ( unsigned int band = 0; band < sh.bands(); band++ )
	{
		offset = band*band;
		for ( int j = -static_cast<int>(band); j <= static_cast<int>(band); j++, cv++ )
		{
			it = tmp.begin() + offset;
			U acc(0);
			for ( int i = -static_cast<int>(band); i <= static_cast<int>(band); i++, it++ )
			{
				acc += (*it) * bandMatrixValue( m_R, band, i, j );
			}
			*cv = acc;
		}
	}
}

template < typename V >
int SphericalHarmonicsRotationMatrix<V>::bandMatrixOffset( int band )
{
	return band*band + band;
}

template < typename V >
V SphericalHarmonicsRotationMatrix<V>::bandMatrixValue( const std::vector< std::vector<V> > &mat, int band, int m1, int m2 )
{
	int offset = bandMatrixOffset( band );
	assert( m1 >= -band && m1 <= band );
	assert( m2 >= -band && m2 <= band );
	return mat[ m1 + offset ][ m2 + offset ];
}

template < typename V >
void SphericalHarmonicsRotationMatrix<V>::setBandMatrixValue( std::vector< std::vector<V> > &mat, int band, int m1, int m2, V value )
{
	int offset = bandMatrixOffset( band );
	assert( m1 >= -band && m1 <= band );
	assert( m2 >= -band && m2 <= band );
	mat[ m1 + offset ][ m2 + offset ] = value;
}

template < typename V >
V SphericalHarmonicsRotationMatrix<V>::F( unsigned int band, int i, int j ) const
{
	return bandMatrixValue( m_F, band, i, j );
}

template < typename V >
V SphericalHarmonicsRotationMatrix<V>::G( unsigned int band, int i, int j ) const
{
	return bandMatrixValue( m_G, band, i, j );
}

template < typename V >
void SphericalHarmonicsRotationMatrix<V>::setF( unsigned int band, int i, int j, V value ) const
{
	setBandMatrixValue( m_F, band, i, j, value );
}

template < typename V >
void SphericalHarmonicsRotationMatrix<V>::setG( unsigned int band, int i, int j, V value ) const
{
	setBandMatrixValue( m_G, band, i, j, value );
}

template < typename V >
void SphericalHarmonicsRotationMatrix<V>::setFG( unsigned int band, int m1, int m2, V realValue, V complexValue ) const
{
	setF( band, m1, m2, realValue );
	setG( band, m1, m2, complexValue );

	if ( m1 || m2 )
	{
		if ( (m1 + m2) & 1 )
		{
			setF( band, -m1, -m2, -realValue );
			setG( band, -m1, -m2, complexValue );
		}
		else
		{
			setF( band, -m1, -m2, realValue );
			setG( band, -m1, -m2, -complexValue );
		}
	}
}

template < typename V >
void SphericalHarmonicsRotationMatrix<V>::computeHK( unsigned int band, int m1, int m2, int i, int j, V &H, V &K ) const
{
	V f1, g1, fN, gN;
	f1 = F(1, i,j);
	g1 = G(1, i,j);
	fN = F(band - 1, m1, m2);
	gN = G(band - 1, m1, m2);
	H = f1 * fN - g1 * gN;
	K = f1 * gN + g1 * fN;
}

template < typename V >
V SphericalHarmonicsRotationMatrix<V>::compute_a( unsigned int l, int m1, int m2 ) const
{
	if ( m1 == static_cast<int>(l) || m1 == -static_cast<int>(l) )
	{
		return 0;
	}
	return (m_squareRoots[l+m1] * m_squareRoots[l-m1]) / ( m_squareRoots[l+m2] * m_squareRoots[l-m2] );
}

template < typename V >
V SphericalHarmonicsRotationMatrix<V>::compute_b( unsigned int l, int m1, int m2 ) const
{
	if ( m1 == -static_cast<int>(l) || m1 == -static_cast<int>(l) + 1 )
	{
		return 0;
	}
	return (m_squareRoots[l+m1] * m_squareRoots[l+m1-1]) / ( m_squareRoots[2] * m_squareRoots[l+m2] * m_squareRoots[l-m2] );
}

template < typename V >
V SphericalHarmonicsRotationMatrix<V>::compute_c( unsigned int l, int m1, int m2 ) const
{
	if ( m1 == -static_cast<int>(l) || m1 == static_cast<int>(l) )
	{
		return 0;
	}
	return (m_squareRoots[2] * m_squareRoots[l+m1] * m_squareRoots[l-m1]) / ( m_squareRoots[l+m2] * m_squareRoots[l+m2-1] );
}

template < typename V >
V SphericalHarmonicsRotationMatrix<V>::compute_d( unsigned int l, int m1, int m2 ) const
{
	if ( m1 == -static_cast<int>(l) || m1 == -static_cast<int>(l) + 1 )
	{
		return 0;
	}
	return (m_squareRoots[l+m1] * m_squareRoots[l+m1-1]) / ( m_squareRoots[l+m2] * m_squareRoots[l+m2-1] );
}

template < typename V >
void SphericalHarmonicsRotationMatrix<V>::computeRecurrenceRelation1( unsigned int band, int m1, int m2 ) const
{
	V realValue, complexValue;
	V H01, H11, H21;
	V K01, K11, K21;
	V a, b, bOpposite;

	realValue = complexValue = 0;

	if ( m1 != -static_cast<int>(band) && m1 != static_cast<int>(band) )
	{
		computeHK( band, m1,   m2,  0, 0, H11, K11 );
		a = compute_a( band, m1, m2 );
		realValue = a * H11;
		complexValue = a * K11;
	}

	if ( m1+1 < static_cast<int>(band) )
	{
		computeHK( band, m1+1, m2, -1, 0, H01, K01 );
		bOpposite = compute_b( band, -m1, m2 );
		realValue += bOpposite * H01;
		complexValue += bOpposite * K01;
	}

	if ( m1-1 > -static_cast<int>(band) )
	{
		computeHK( band, m1-1, m2,  1, 0, H21, K21 );
		b = compute_b( band, m1, m2 );
		realValue += b * H21;
		complexValue += b * K21;
	}

	setFG( band, m1, m2, realValue, complexValue );
}

template < typename V >
void SphericalHarmonicsRotationMatrix<V>::computeRecurrenceRelation2( unsigned int band, int m1, int m2 ) const
{
	V realValue, complexValue;
	V H00, H10, H20;
	V K00, K10, K20;
	V c, d, dOpposite;

	realValue = complexValue = 0;

	if ( m1 != -static_cast<int>(band) && m1 != static_cast<int>(band) )
	{
		computeHK( band, m1,   m2+1,  0, -1, H10, K10 );
		c = compute_c( band, m1, -m2 );
		realValue = c * H10;
		complexValue = c * K10;
	}

	if ( m1+1 < static_cast<int>(band) )
	{
		computeHK( band, m1+1, m2+1, -1, -1, H00, K00 );
		dOpposite = compute_d( band, -m1, -m2 );
		realValue += dOpposite * H00;
		complexValue += dOpposite * K00;
	}

	if ( m1-1 > -static_cast<int>(band) )
	{
		computeHK( band, m1-1, m2+1,  1, -1, H20, K20 );
		d = compute_d( band, m1, -m2 );
		realValue += d * H20;
		complexValue += d * K20;
	}

	setFG( band, m1, m2, realValue, complexValue );
}

template < typename V >
void SphericalHarmonicsRotationMatrix<V>::computeRecurrenceRelation3( unsigned int band, int m1, int m2 ) const
{
	V realValue, complexValue;
	V H02, H12, H22;
	V K02, K12, K22;
	V c, d, dOpposite;

	realValue = complexValue = 0;

	if ( m1 != -static_cast<int>(band) && m1 != static_cast<int>(band) )
	{
		computeHK( band, m1,   m2-1,  0, 1, H12, K12 );
		c = compute_c( band, m1, m2 );
		realValue = c * H12;
		complexValue = c * K12;
	}

	if ( m1+1 < static_cast<int>(band) )
	{
		computeHK( band, m1+1, m2-1, -1, 1, H02, K02 );
		dOpposite = compute_d( band, -m1, m2 );
		realValue += dOpposite * H02;
		complexValue += dOpposite * K02;
	}

	if ( m1-1 > -static_cast<int>(band) )
	{
		computeHK( band, m1-1, m2-1,  1, 1, H22, K22 );
		d = compute_d( band, m1, m2 );
		realValue += d * H22;
		complexValue += d * K22;
	}
	setFG( band, m1, m2, realValue, complexValue );
}

template < typename V >
void SphericalHarmonicsRotationMatrix<V>::computeSquareRoots( ) const
{
	unsigned int rootCount = (m_bands - 1) * 2 + 2;
	if ( m_squareRoots.size() >= rootCount )
	{
		return;
	}
	m_squareRoots.resize( rootCount );
	for (unsigned int i = 0; i < rootCount; i++ )
	{
		m_squareRoots[i] = sqrt( static_cast<V>(i) );
	}
}

template < typename V >
void SphericalHarmonicsRotationMatrix<V>::computeComplexRotation() const
{
	unsigned int rows = m_bands*m_bands;
	unsigned int cols = rows;
	if ( m_F.size() != rows )
	{
		m_F.resize( rows );
		m_G.resize( rows );
		typename std::vector< std::vector<V> >::iterator it;
		for ( it = m_F.begin(); it != m_F.end(); it++ )
		{
			it->resize( cols, 0 );
		}
		for ( it = m_G.begin(); it != m_G.end(); it++ )
		{
			it->resize( cols, 0 );
		}
	}

	// compute band 0
	setF( 0, 0, 0, 1 );
	setG( 0, 0, 0, 1 );

	if ( m_bands == 1 )
	{
		return;
	}

	// compute band 1 from rotation matrix
	setF( 1, -1, -1, (m_3dRotation[1][1] + m_3dRotation[0][0]) / 2 );
	setF( 1, -1,  0, m_3dRotation[0][2] / m_squareRoots[2] );
	setF( 1, -1,  1, (m_3dRotation[1][1] - m_3dRotation[0][0]) / 2 );
	setF( 1,  0, -1, m_3dRotation[2][0] / m_squareRoots[2] );
	setF( 1,  0,  0, m_3dRotation[2][2] );
	setF( 1,  0,  1, -F(1,0,-1) );
	setF( 1,  1, -1, F(1,-1,1) );
	setF( 1,  1,  0, -F(1,-1,0) );
	setF( 1,  1,  1, F(1,-1,-1) );

	setG( 1, -1, -1, (m_3dRotation[1][0] - m_3dRotation[0][1]) / 2 );
	setG( 1, -1,  0, m_3dRotation[1][2] / m_squareRoots[2] );
	setG( 1, -1,  1, -(m_3dRotation[1][0] + m_3dRotation[0][1]) / 2 );
	setG( 1,  0, -1, -m_3dRotation[2][1] / m_squareRoots[2] );
	setG( 1,  0,  0, 0 );
	setG( 1,  0,  1, G(1,0,-1) );
	setG( 1,  1, -1, -G(1,-1,1) );
	setG( 1,  1,  0, G(1,-1,0) );
	setG( 1,  1,  1, -G(1,-1,-1) );

	// compute recurrent bands
	for ( unsigned int band = 2; band < m_bands; band++ )
	{
		for (int m1 = 0; m1 <= static_cast<int>(band); m1++ )
		{
			for (int m2 = 0; m2 < static_cast<int>(band); m2++ )
			{
				computeRecurrenceRelation1( band, m1, m2 );
				if ( m1 && m2 )
				{
					computeRecurrenceRelation1( band, -m1, m2 );
				}
			}
			computeRecurrenceRelation3( band, m1, band );
			if ( m1 )
			{
				computeRecurrenceRelation3( band, -m1, band );
			}
		}
	}
}

template < typename V >
void SphericalHarmonicsRotationMatrix<V>::computeRealRotation() const
{
	unsigned int rows = m_bands*m_bands;
	unsigned int cols = rows;
	V F1, F2, G1, G2, v;

	if ( m_R.size() != rows )
	{
		m_R.resize( rows );
		typename std::vector< std::vector<V> >::iterator it;
		for ( it = m_R.begin(); it != m_R.end(); it++ )
		{
			it->resize( cols, 0 );
		}
	}

	// compute band 0
	setBandMatrixValue( m_R, 0, 0, 0, 1 );

	// compute recurrent bands
	for ( unsigned int band = 1; band < m_bands; band++ )
	{
		//  i=0,j=0: R(i,j) = F(0,0)
		F1 = F( band, 0, 0 );
		setBandMatrixValue( m_R, band,  0,  0, F1 );

		for ( int i = 1; i <= static_cast<int>(band); i++ )
		{
			v = m_squareRoots[2];
			// i=0,j>0 or i>0,j=0: R(i,j) = sqrt(2)*F(-|i|,-|j|)
			setBandMatrixValue( m_R, band,  i,  0, v * F( band, -i, 0 ) );
			setBandMatrixValue( m_R, band,  0,  i, v * F( band, 0, -i ) );

			// i=0,j<0 or i<0,j=0: R(i,j) = sign(j+.5)*sqrt(2)*G(-|i|,-|j|)
			setBandMatrixValue( m_R, band,  -i,  0, v * G( band, -i, 0 ) );
			setBandMatrixValue( m_R, band,  0,  -i, -v * G( band, 0, -i ) );

			for ( int j = 1; j <= static_cast<int>(band); j++ )
			{
				//  i>0,j>0 or i<0,j<0: R(i,j) = sign(j)*((-1)^|j|)*F(-|i|,|j|) + F(-|i|,-|j|)
				F1 = F( band, -i, j );
				F2 = F( band, -i, -j );
				v = ( j & 1 ? -1 : 1 ) * F1;
				setBandMatrixValue( m_R, band,  i,  j, v + F2 );
				setBandMatrixValue( m_R, band,  -i,  -j, -v + F2 );

				//  i<0,j>0 or i>0,j<0: R(i,j) = ((-1)^|j|*G(-|i|,|j|) + sign(j)*G(-|i|,-|j|)
				G1 = G( band, -i, j );
				G2 = G( band, -i, -j );
				v = ( j & 1 ? -1 : 1 ) * G1;
				setBandMatrixValue( m_R, band, -i,  j, v + G2 );
				setBandMatrixValue( m_R, band,  i, -j, v - G2 );
			}
		}
	}
}

} // namespace IECore
