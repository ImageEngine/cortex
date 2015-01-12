//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
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

#include "boost/math/special_functions/factorials.hpp"

#include "IECore/SphericalHarmonicsTensor.h"
#include "IECore/Math.h"

using namespace IECore;

SphericalHarmonicsTensor &SphericalHarmonicsTensor::tensor()
{
	static SphericalHarmonicsTensor singleTensor;
	return singleTensor;
}

SphericalHarmonicsTensor::SphericalHarmonicsTensor() : m_bands(0)
{
}

void SphericalHarmonicsTensor::compute( unsigned int bands )
{
	if ( m_bands < bands )
	{
		// get exclusive access
		tbb::queuing_rw_mutex::scoped_lock lock( m_mutex, true );

		// got the mutex, must check it again cause another compute() could have done it already.
		if ( m_bands < bands )
		{
			m_bands = bands;

			// \todo: computing tensors from scratch. Can we reuse old vector?
			m_tensors.clear();
			unsigned int maxIndex = bands * bands;
			
			// \todo: brute force algorithm. Could use some knowledge to avoid when coefficients are zero...
			int Ji, Mi;
			Ji = Mi = 0;
			for ( unsigned int i = 0; i < maxIndex; i++ )
			{
				int Jj = Ji;
				int Mj = Mi;
				for ( unsigned int j = i; j < maxIndex; j++ )
				{
					int Jk = Jj;
					int Mk = Mj;
					for ( unsigned int k = j; k < maxIndex; k++ )
					{
						IndexPermutation perm = IJK;
						if ( i == j )
						{
							if ( j == k )
							{
								perm = III;
							}
							else
							{
								perm = IIK;
							}
						}
						else if ( j == k )
						{
							perm = IJJ;
						}
						double t = realGaunt( Ji, Mi, Jj, Mj, Jk, Mk );
						if ( t != 0 )
						{
							m_tensors.push_back( boost::make_tuple( i, j, k, perm, t ) );
						}
						Mk++;
						if ( Mk > Jk )
						{
							Jk++;
							Mk = -Jk;
						}
					}
					Mj++;
					if ( Mj > Jj )
					{
						Jj++;
						Mj = -Jj;
					}
				}
				Mi++;
				if ( Mi > Ji )
				{
					Ji++;
					Mi = -Ji;
				}
			}
		}
	}
}

inline double factorial( int v )
{
	return boost::math::factorial< double >( v );
}

double SphericalHarmonicsTensor::wigner3j0( int Ji, int Jj, int Jk )
{
	int J = Ji + Jj + Jk;

	if ( J & 1 )
		return 0;

	int g = J / 2;
	int f1 = J - 2*Ji;
	int f2 = J - 2*Jj;
	int f3 = J - 2*Jk;

	if ( f1 < 0 || f2 < 0 || f3 < 0 )
		return 0;

	double firstHalf = ( factorial( f1 ) * factorial( f2 ) * factorial( f3 ) ) / factorial( J + 1 );
	double secondHalf = factorial( g ) / ( factorial( g - Ji ) * factorial( g - Jj ) * factorial( g - Jk ) );

	return ( g & 1 ? -1 : 1 ) * sqrt( firstHalf ) * secondHalf;
}

double SphericalHarmonicsTensor::wigner3j( int Ji, int Mi, int Jj, int Mj, int Jk, int Mk )
{
	if ( Mi + Mj != -Mk )
		return 0;

	if ( Ji - Jj > Jk || Jj - Ji > Jk )
		return 0;

	if ( Jk > Ji + Jj )
		return 0;

	double signW = ( (Ji - Jj - Mk) & 1 ? -1 : 1 );
	double triangleCoeff = ( factorial( Ji + Jj - Jk ) * factorial( Ji - Jj + Jk ) * factorial( -Ji + Jj + Jk ) ) / factorial( Ji + Jj + Jk + 1 );
	double sqrtW = sqrt( factorial( Ji + Mi ) * factorial( Ji - Mi ) * factorial( Jj + Mj ) * factorial( Jj - Mj ) * factorial( Jk + Mk ) * factorial( Jk - Mk ) );
	int v = std::min( Ji + Mi, std::min( Ji - Mi, std::min( Jj + Mj, std::min( Jj - Mj, std::min( Jk + Mk, std::min( Jk - Mk, std::min( Ji + Jj - Jk, std::min( Jj + Jk - Ji, Jk + Ji - Jj ) ) ) ) ) ) ) );
	int firstT = std::max(0, -std::min( Jk - Jj + Mi, Jk - Ji - Mj ) );

	double sum = 0;
	double signSum = ( firstT & 1 ? -1 : 1 );
	for ( int t = firstT; t <= firstT + v; t++, signSum = -signSum )
	{
		double x = factorial( t ) * factorial( Jk - Jj + t + Mi ) * factorial( Jk - Ji + t - Mj ) * 
					factorial( Ji + Jj - Jk - t ) * factorial( Ji - t - Mi ) * factorial( Jj - t + Mj );
		sum += signSum / x;
	}
	return signW * sqrt(triangleCoeff) * sqrtW * sum;
}

double SphericalHarmonicsTensor::gaunt( int Ji, int Mi, int Jj, int Mj, int Jk, int Mk )
{
	double A = sqrt( (double)(2*Ji+1)*(2*Jj+1)*(2*Jk+1) / (4*M_PI) );
	return A * wigner3j0( Ji, Jj, Jk ) * wigner3j( Ji, Mi, Jj, Mj, Jk, Mk );
}

std::complex<double> SphericalHarmonicsTensor::U( int l, int m, int u )
{
	const static double invSqrt2 = 1/sqrt(2);
	double mS = (m&1?-invSqrt2:invSqrt2);
	if ( u == 0 )
	{
		if ( m == 0 )
			return 1;
	}
	else if ( u < 0 )
	{
		if ( m == u )
			return std::complex<double>( 0, mS );
		if ( m == -u )
			return std::complex<double>( 0, -invSqrt2 );
	}
	else if ( u > 0 )
	{
		if ( m == u )
			return invSqrt2;
		if ( m == -u )
			return mS;
	}
	return 0;
}

double SphericalHarmonicsTensor::realGaunt( int Ji, int Mi, int Jj, int Mj, int Jk, int Mk )
{
	// \todo Considering the change on equation 26, rethink how would be the special case equations and avoid the brute force approach below.
	double sum = 0;
	for ( int M1 = -Ji; M1 <= Ji; M1++ ) 
	{
		for ( int M2 = -Jj; M2 <= Jj; M2++ )
		{
			for ( int M3 = -Jk; M3 <= Jk; M3++ )
			{
				// \todo Reduce the number of iterations by considering the behavior of U.
				double tmp = ( U(Ji,M1,Mi)*U(Jj,M2,Mj)*U(Jk,M3,Mk) ).real();
				if ( tmp != 0 )
					sum += tmp * gaunt( Ji, M1, Jj, M2, Jk, M3 );
			}
		}
	}
	return sum;
}

void SphericalHarmonicsTensor::evaluate( size_t bands, boost::function< void ( unsigned int, unsigned int, unsigned int, double ) > functor )
{
	compute( bands );

	// get read-only access
	tbb::queuing_rw_mutex::scoped_lock lock( m_mutex, false );

	unsigned int maxIndex = bands * bands;

	// go on each tuple, filtering to not return higher bands coefficients.
	for ( TensorVector::const_iterator it = m_tensors.begin(); it != m_tensors.end(); it++ )
	{
		unsigned int i = boost::get<0>( *it );
		unsigned int j = boost::get<1>( *it );
		unsigned int k = boost::get<2>( *it );

		if ( i < maxIndex && j < maxIndex && k < maxIndex )
		{
			IndexPermutation indexPermutation = boost::get<3>( *it );
			double tensorValue = boost::get<4>( *it );

			switch( indexPermutation )
			{
			case IJK:
				functor( i, j, k, tensorValue );
				functor( j, i, k, tensorValue );
				functor( i, k, j, tensorValue );
				functor( j, k, i, tensorValue );
				functor( k, i, j, tensorValue );
				functor( k, j, i, tensorValue );
				break;
			case IIK:
				functor( i, i, k, tensorValue );
				functor( i, k, i, tensorValue );
				functor( k, i, i, tensorValue );
				break;
			case IJJ:
				functor( i, j, j, tensorValue );
				functor( j, i, j, tensorValue );
				functor( j, j, i, tensorValue );
				break;
			case III:
				functor( i, i, i, tensorValue );
				break;
			}
		}
	}
}
