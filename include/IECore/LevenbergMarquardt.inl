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

#ifndef IE_CORE_LEVENBERGMARQUARDT_INL
#define IE_CORE_LEVENBERGMARQUARDT_INL

#include <cassert>

#include "OpenEXR/ImathMath.h"

#include "IECore/Exception.h"

namespace IECore
{

template<typename T>
struct DefaultLevenbergMarquardtTraits
{
	static T g_machinePrecision;
	static T g_sqrtMin;
	static T g_sqrtMax;

	static T machinePrecision()
	{
		return g_machinePrecision;
	}
	static T sqrtMin()
	{
		return g_sqrtMin;
	}
	static T sqrtMax()
	{
		return g_sqrtMax;
	}
	static T tolerance()
	{
		return T(30) * machinePrecision();
	}
};

template<typename T>
T DefaultLevenbergMarquardtTraits<T>::g_machinePrecision( Imath::limits<T>::epsilon() );

template<typename T>
T DefaultLevenbergMarquardtTraits<T>::g_sqrtMin( Imath::Math<T>::sqrt( Imath::limits<T>::smallest() ) );

template<typename T>
T DefaultLevenbergMarquardtTraits<T>::g_sqrtMax( Imath::Math<T>::sqrt( Imath::limits<T>::max() ) );

template<typename T, typename ErrorFn, template<typename> class Traits>
LevenbergMarquardt<T, ErrorFn, Traits>::LevenbergMarquardt()
{
	m_maxCalls = 100;
	m_epsilon = Traits<T>::tolerance();
	m_stepBound = T( 100 );
	m_ftol = Traits<T>::tolerance();
	m_xtol = Traits<T>::tolerance();
	m_gtol = Traits<T>::tolerance();
}


template<typename T, typename ErrorFn, template<typename> class Traits>
void LevenbergMarquardt<T, ErrorFn, Traits>::setParameters(
        T ftol,
        T xtol,
        T gtol,
        T epsilon,
        T stepBound
)
{
	m_ftol = ftol;
	m_xtol = xtol;
	m_gtol = gtol;
	m_epsilon = epsilon;
	m_stepBound = stepBound;
}

template<typename T, typename ErrorFn, template<typename> class Traits>
void LevenbergMarquardt<T, ErrorFn, Traits>::getParameters(
	T &ftol,
	T &xtol,
	T &gtol,
	T &epsilon,
	T &stepBound
) const
{
	ftol = m_ftol;
	xtol = m_xtol;	
	gtol = m_gtol;		
	epsilon = m_epsilon;
	stepBound = m_stepBound;
}

	
template<typename T, typename ErrorFn, template<typename> class Traits>
void LevenbergMarquardt<T, ErrorFn, Traits>::setMaxCalls( unsigned maxCalls )
{
	m_maxCalls = maxCalls;
}

template<typename T, typename ErrorFn, template<typename> class Traits>
unsigned LevenbergMarquardt<T, ErrorFn, Traits>::getMaxCalls() const
{
	return m_maxCalls;
}

template<typename T, typename ErrorFn, template<typename> class Traits>
typename LevenbergMarquardt<T, ErrorFn, Traits>::Status LevenbergMarquardt<T, ErrorFn, Traits>::solve( typename TypedData< std::vector<T> >::Ptr parameters, ErrorFn &fn )
{
	assert( parameters );
	m_n = parameters->readable().size();
	m_m = fn.numErrors();

	m_fvec = new TypedData< std::vector<T> >( );
	m_fvec->writable().resize( m_m );
	m_diag.resize( m_n );
	m_qtf.resize( m_n );
	m_fjac.resize( m_n * m_m );
	m_wa1.resize( m_n );
	m_wa2 = new TypedData< std::vector<T> >( );
	m_wa2->writable().resize( m_n );
	m_wa3.resize( m_n );
	m_wa4 = new TypedData< std::vector<T> >( );
	m_wa4->writable().resize( m_m );
	m_ipvt.resize( m_n );

	m_numCalls = 0;

	typename std::vector<T> &x = parameters->writable();
	typename std::vector<T> &fvec = m_fvec->writable();
	typename std::vector<T> &wa2 = m_wa2->writable();
	typename std::vector<T> &wa4 = m_wa4->writable();

	T actred, dirder,  fnorm, fnorm1, gnorm, pnorm,	prered, ratio, step, sum, temp, temp1, temp2, temp3;

	unsigned iter = 1;

	T delta = 0;
	T xnorm = 0;
	T eps = Imath::Math<T>::sqrt( std::max<T>( m_epsilon, Traits<T>::machinePrecision() ) );

	if (( m_n <= 0 ) || ( m_m < m_n ) || ( m_ftol < 0. )
	                || ( m_xtol < 0. ) || ( m_gtol < 0. ) || ( m_stepBound <= 0. ) )
	{
		throw InvalidArgumentException( "LevenbergMarquardt: Incorrect solver parameters" );
	}

	fn( parameters, m_fvec );
	m_numCalls++;

	fnorm = euclideanNorm( m_fvec->readable().begin(), m_fvec->readable().end() );

	do
	{
		for ( unsigned j = 0; j < m_n; j++ )
		{
			T temp = x[j];
			step = eps * Imath::Math<T>::fabs( temp );
			if ( step == 0. )
			{
				step = eps;
			}
			x[j] = temp + step;

			fn( parameters, m_wa4 );
			m_numCalls++;

			for ( unsigned i = 0; i < m_m; i++ )
			{
				m_fjac[j * m_m + i] = ( wa4[i] - fvec[i] ) / ( x[j] - temp );
			}
			x[j] = temp;
		}

		qrFactorize();

		if ( iter == 1 )
		{
			/// diag := norms of the columns of the initial jacobian
			for ( unsigned j = 0; j < m_n; j++ )
			{
				m_diag[j] = wa2[j];
				if ( wa2[j] == 0. )
				{
					m_diag[j] = 1.;
				}
			}

			/// use diag to scale x, then calculate the norm
			for ( unsigned j = 0; j < m_n; j++ )
			{
				m_wa3[j] = m_diag[j] * x[j];
			}
			assert( m_wa3.size() == m_n );
			xnorm = euclideanNorm( m_wa3.begin(), m_wa3.end() );
			/// initialize the step bound delta.
			delta = m_stepBound * xnorm;
			if ( delta == 0. )
			{
				delta = m_stepBound;
			}
		}

		for ( unsigned i = 0; i < m_m; i++ )
		{
			wa4[i] = fvec[i];
		}

		for ( unsigned j = 0; j < m_n; j++ )
		{
			temp3 = m_fjac[j * m_m + j];
			if ( temp3 != 0. )
			{
				sum = 0;
				for ( unsigned i = j; i < m_m; i++ )
				{
					sum += m_fjac[j * m_m + i] * wa4[i];
				}
				temp = -sum / temp3;
				for ( unsigned i = j; i < m_m; i++ )
				{
					wa4[i] += m_fjac[j * m_m + i] * temp;
				}
			}
			m_fjac[j * m_m + j] = m_wa1[j];
			m_qtf[j] = wa4[j];
		}

		gnorm = 0;
		if ( fnorm != 0 )
		{
			for ( unsigned j = 0; j < m_n; j++ )
			{
				if ( wa2[m_ipvt[j]] != 0 )
				{
					sum = 0.;
					for ( unsigned i = 0; i <= j; i++ )
					{
						sum += m_fjac[j * m_m + i] * m_qtf[i] / fnorm;
					}
					gnorm = std::max<T>( gnorm, Imath::Math<T>::fabs( sum / wa2[m_ipvt[j]] ) );
				}
			}
		}

		if ( gnorm <= m_gtol )
		{
			return Degenerate;
		}

		for ( unsigned j = 0; j < m_n; j++ )
		{
			m_diag[j] = std::max( m_diag[j], wa2[j] );
		}

		do
		{
			/// determine the levenberg-marquardt parameter.
			T par = computeLMParameter( m_wa1, m_wa2->writable(), delta );

			///store the direction p and x + p; calculate the norm of p.
			for ( unsigned j = 0; j < m_n; j++ )
			{
				m_wa1[j] = -m_wa1[j];
				wa2[j] = x[j] + m_wa1[j];
				m_wa3[j] = m_diag[j] * m_wa1[j];
			}
			assert( m_wa3.size() == m_n );
			pnorm = euclideanNorm( m_wa3.begin(), m_wa3.end() );

			/// on the first iteration, adjust the initial step bound.
			if ( m_numCalls <= 1 + m_n )
			{
				delta = std::min( delta, pnorm );
			}

			/// evaluate the function at x + p and calculate its norm.
			fn( m_wa2, m_wa4 );
			m_numCalls++;

			assert( wa4.size() == m_m );
			fnorm1 = euclideanNorm( wa4.begin(), wa4.end() );

			/// compute the scaled actual reduction.
			if ( 0.1 * fnorm1 < fnorm )
			{
				actred = 1 - sqr( fnorm1 / fnorm );
			}
			else
			{
				actred = -1;
			}

			/// compute the scaled predicted reduction and the scaled directional derivative.
			for ( unsigned j = 0; j < m_n; j++ )
			{
				m_wa3[j] = 0;
				for ( unsigned i = 0; i <= j; i++ )
				{
					m_wa3[i] += m_fjac[j * m_m + i] * m_wa1[m_ipvt[j]];
				}
			}
			assert( m_wa3.size() == m_n );
			temp1 = euclideanNorm( m_wa3.begin(), m_wa3.end() ) / fnorm;
			temp2 = Imath::Math<T>::sqrt( par ) * pnorm / fnorm;
			prered = sqr( temp1 ) + 2 * sqr( temp2 );
			dirder = -( sqr( temp1 ) + sqr( temp2 ) );

			/// compute the ratio of the actual to the predicted reduction.
			ratio = prered != 0 ? actred / prered : 0;

			/// update the step bound.
			if ( ratio <= 0.25 )
			{
				if ( actred >= 0. )
				{
					temp = 0.5;
				}
				else
				{
					temp = 0.5 * dirder / ( dirder + 0.5 * actred );
				}
				if ( 0.1 * fnorm1 >= fnorm || temp < 0.1 )
				{
					temp = 0.1;
				}
				delta = temp * std::min<T>( delta, pnorm / 0.1 );
				par /= temp;
			}
			else if ( par == 0. || ratio >= 0.75 )
			{
				delta = pnorm / 0.5;
				par *= 0.5;
			}

			/// test for successful iteration.
			if ( ratio >= 1.0e-4 )
			{
				//// yes, success: update x, fvec, and their norms.
				for ( unsigned j = 0; j < m_n; j++ )
				{
					x[j] = wa2[j];
					wa2[j] = m_diag[j] * x[j];
				}
				for ( unsigned i = 0; i < m_m; i++ )
				{
					fvec[i] = wa4[i];
				}
				assert( wa2.size() == m_n );
				xnorm = euclideanNorm( wa2.begin(), wa2.end() );
				fnorm = fnorm1;
				iter++;
			}

			/// tests for convergence
			if ( Imath::Math<T>::fabs( actred ) <= m_ftol && prered <= m_ftol && 0.5 * ratio <= 1 )
			{
				return Success;
			}

			if ( delta <= m_xtol * xnorm )
			{
				return Success;
			}

			/// tests for termination and stringent tolerances.
			if ( m_numCalls >= m_maxCalls )
			{
				return  CallLimit;
			}
			if ( Imath::Math<T>::fabs( actred ) <= Traits<T>::machinePrecision() &&
			                prered <= Traits<T>::machinePrecision() && 0.5 * ratio <= 1 )
			{
				return FailedFTol;
			}
			if ( delta <= Traits<T>::machinePrecision() * xnorm )
			{
				return FailedXTol;
			}
			if ( gnorm <= Traits<T>::machinePrecision() )
			{
				return FailedGTol;
			}
		}
		while ( ratio < 1.0e-4 );
	}
	while ( 1 );
}

template<typename T, typename ErrorFn, template<typename> class Traits>
void LevenbergMarquardt<T, ErrorFn, Traits>::qrFactorize()
{
	unsigned int i, j, k, kmax, minmn;
	T ajnorm, sum, temp;

	std::vector<T> &rdiag = m_wa1;
	std::vector<T> &acnorm = m_wa2->writable();
	for ( j = 0; j < m_n; j++ )
	{

		acnorm[j] = euclideanNorm( m_fjac.begin() + j * m_m, m_fjac.begin() + j * m_m + m_m );

		rdiag[j] = acnorm[j];
		m_wa3[j] = rdiag[j];
		m_ipvt[j] = j;
	}

	minmn = std::min( m_m, m_n );
	for ( j = 0; j < minmn; j++ )
	{
		kmax = j;
		for ( k = j + 1; k < m_n; k++ )
		{
			if ( rdiag[k] > rdiag[kmax] )
			{
				kmax = k;
			}
		}

		if ( kmax != j )
		{

			for ( i = 0; i < m_m; i++ )
			{
				temp = m_fjac[j * m_m + i];
				m_fjac[j * m_m + i] = m_fjac[kmax * m_m + i];
				m_fjac[kmax * m_m + i] = temp;
			}
			rdiag[kmax] = rdiag[j];
			m_wa3[kmax] = m_wa3[j];
			k = m_ipvt[j];
			m_ipvt[j] = m_ipvt[kmax];
			m_ipvt[kmax] = k;
		}
		ajnorm = euclideanNorm( m_fjac.begin() + j * m_m + j,  m_fjac.begin() + j * m_m + m_m );

		if ( ajnorm == 0. )
		{
			rdiag[j] = 0;
			continue;
		}

		if ( m_fjac[j * m_m + j] < 0. )
		{
			ajnorm = -ajnorm;
		}

		for ( i = j; i < m_m; i++ )
		{
			m_fjac[j * m_m + i] /= ajnorm;
		}

		m_fjac[j * m_m + j] += 1;

		for ( k = j + 1; k < m_n; k++ )
		{
			sum = 0;

			for ( i = j; i < m_m; i++ )
			{
				sum += m_fjac[j * m_m + i] * m_fjac[k * m_m + i];
			}

			temp = sum / m_fjac[j + m_m * j];

			for ( i = j; i < m_m; i++ )
			{
				m_fjac[k * m_m + i] -= temp * m_fjac[j * m_m + i];
			}

			if ( rdiag[k] != 0. )
			{
				temp = m_fjac[m_m * k + j] / rdiag[k];
				temp = std::max<T>( 0., 1 - temp * temp );
				rdiag[k] *= Imath::Math<T>::sqrt( temp );
				temp = rdiag[k] / m_wa3[k];
				if ( T( 0.05 ) * sqr( temp ) <= Traits<T>::machinePrecision() )
				{
					rdiag[k] = euclideanNorm( m_fjac.begin()+m_m * k + j + 1, m_fjac.begin()+m_m * k  + m_m );
					m_wa3[k] = rdiag[k];
				}
			}
		}

		rdiag[j] = -ajnorm;
	}
}

template<typename T, typename ErrorFn, template<typename> class Traits>
T LevenbergMarquardt<T, ErrorFn, Traits>::computeLMParameter( std::vector<T> &x, std::vector<T> &sdiag, T delta )
{
	T par;

	unsigned int i, iter, j, nsing;
	T dxnorm, fp, fp_old, gnorm, parc, parl, paru;
	T sum, temp;

	typename std::vector<T> &wa4 = m_wa4->writable();

	/// compute and store in x the gauss-newton direction. if the jacobian is rank-deficient, obtain a least squares solution.

	nsing = m_n;
	for ( j = 0; j < m_n; j++ )
	{
		m_wa3[j] = m_qtf[j];
		if ( m_fjac[j * m_m + j] == 0 && nsing == m_n )
		{
			nsing = j;
		}
		if ( nsing < m_n )
		{
			m_wa3[j] = 0;
		}
	}

	for ( int j = nsing - 1; j >= 0; j-- )
	{
		m_wa3[j] = m_wa3[j] / m_fjac[j + m_m * j];
		temp = m_wa3[j];
		for ( i = 0; i < ( unsigned )j; i++ )
		{
			m_wa3[i] -= m_fjac[j * m_m + i] * temp;
		}
	}

	for ( j = 0; j < m_n; j++ )
	{
		x[m_ipvt[j]] = m_wa3[j];
	}

	iter = 0;
	for ( j = 0; j < m_n; j++ )
	{
		wa4[j] = m_diag[j] * x[j];
	}
	dxnorm = euclideanNorm( wa4.begin(), wa4.begin() + m_n );
	fp = dxnorm - delta;

	if ( fp <= 0.1 * delta )
	{
		return 0;
	}

	parl = 0;
	if ( nsing >= m_n )
	{
		for ( j = 0; j < m_n; j++ )
		{
			m_wa3[j] = m_diag[m_ipvt[j]] * wa4[m_ipvt[j]] / dxnorm;
		}

		for ( j = 0; j < m_n; j++ )
		{
			sum = 0.;
			for ( i = 0; i < j; i++ )
			{
				sum += m_fjac[j * m_m + i] * m_wa3[i];
			}
			m_wa3[j] = ( m_wa3[j] - sum ) / m_fjac[j + m_m * j];
		}
		assert( m_wa3.size() == m_n );
		temp = euclideanNorm( m_wa3.begin(), m_wa3.end() );
		parl = fp / delta / temp / temp;
	}

	for ( j = 0; j < m_n; j++ )
	{
		sum = 0;
		for ( i = 0; i <= j; i++ )
		{
			sum += m_fjac[j * m_m + i] * m_qtf[i];
		}
		m_wa3[j] = sum / m_diag[m_ipvt[j]];
	}
	gnorm = euclideanNorm( m_wa3.begin(), m_wa3.end() );
	paru = gnorm / delta;
	if ( paru == 0. )
	{
		paru = Imath::limits<T>::smallest() / std::min<T>( delta, 0.1 );
	}

	par = std::max<T>( par, parl );
	par = std::min<T>( par, paru );

	if ( par == 0. )
	{
		par = gnorm / dxnorm;
	}

	for ( ;; iter++ )
	{
		/// evaluate the function at the current value of par.
		if ( par == 0. )
		{
			par = std::max<T>( Imath::limits<T>::smallest(), 0.001 * paru );
		}
		temp = Imath::Math<T>::sqrt( par );
		for ( j = 0; j < m_n; j++ )
		{
			m_wa3[j] = temp * m_diag[j];
		}
		qrSolve( m_fjac, m_wa3, m_qtf, x, sdiag );
		for ( j = 0; j < m_n; j++ )
		{
			wa4[j] = m_diag[j] * x[j];
		}
		dxnorm = euclideanNorm( wa4.begin(), wa4.begin() + m_n );
		fp_old = fp;
		fp = dxnorm - delta;

		/// if the function is small enough, accept the current value of par. Also test for the exceptional cases where parl
		/// is zero or the number of iterations has reached 10.
		if ( Imath::Math<T>::fabs( fp ) <= 0.1 * delta || ( parl == 0. && fp <= fp_old && fp_old < 0. ) || iter == 10 )
		{
			break;
		}

		/// compute the Newton correction.
		for ( j = 0; j < m_n; j++ )
		{
			m_wa3[j] = m_diag[m_ipvt[j]] * wa4[m_ipvt[j]] / dxnorm;
		}

		for ( j = 0; j < m_n; j++ )
		{
			m_wa3[j] = m_wa3[j] / sdiag[j];
			for ( i = j + 1; i < m_n; i++ )
			{
				m_wa3[i] -= m_fjac[j * m_m + i] * m_wa3[j];
			}
		}
		temp = euclideanNorm( m_wa3.begin(), m_wa3.end() );
		parc = fp / delta / temp / temp;
		if ( fp > 0 )
		{
			parl = std::max<T>( parl, par );
		}
		else if ( fp < 0 )
		{
			paru = std::min<T>( paru, par );
		}
		/// the case fp==0 is precluded by the break condition

		/// compute an improved estimate for par.
		par = std::max<T>( parl, par + parc );

	}

	return par;
}

template<typename T, typename ErrorFn, template<typename> class Traits>
void LevenbergMarquardt<T, ErrorFn, Traits>::qrSolve( std::vector<T> &r, std::vector<T> & diag,
                std::vector<T> & qtb, std::vector<T> &x, std::vector<T> &sdiag )
{
	std::vector<T> &wa4 = m_wa4->writable();

	/// copy r and (q transpose)*b to preserve input and initialize s in particular, save the diagonal elements of r in x.
	for ( unsigned j = 0; j < m_n; j++ )
	{
		for ( unsigned i = j; i < m_n; i++ )
		{
			r[j * m_m + i] = r[i * m_m + j];
		}
		x[j] = r[j * m_m + j];
		wa4[j] = qtb[j];
	}

	/// eliminate the diagonal matrix d using a Givens rotation. ***/
	for ( unsigned j = 0; j < m_n; j++ )
	{

		/// prepare the row of d to be eliminated, locating the diagonal element using p from the qr factorization.
		if ( diag[m_ipvt[j]] != 0. )
		{
			for ( unsigned k = j; k < m_n; k++ )
			{
				sdiag[k] = 0.;
			}
			sdiag[j] = diag[m_ipvt[j]];

			T qtbpj = 0.;
			for ( unsigned k = j; k < m_n; k++ )
			{

				/// determine a givens rotation which eliminates the appropriate element in the current row of d.
				if ( sdiag[k] != 0. )
				{

					unsigned kk = k + m_m * k;
					T sinTheta, cosTheta;

					if ( Imath::Math<T>::fabs( r[kk] ) < Imath::Math<T>::fabs( sdiag[k] ) )
					{
						T cotTheta = r[kk] / sdiag[k];
						sinTheta = 0.5 / Imath::Math<T>::sqrt( 0.25 + 0.25 * sqr( cotTheta ) );
						cosTheta = sinTheta * cotTheta;
					}
					else
					{
						T tanTheta = sdiag[k] / r[kk];
						cosTheta = 0.5 / Imath::Math<T>::sqrt( 0.25 + 0.25 * sqr( tanTheta ) );
						sinTheta = cosTheta * tanTheta;
					}

					/// compute the modified diagonal element of r and the modified element of ((q transpose)*b,0).
					r[kk] = cosTheta * r[kk] + sinTheta * sdiag[k];
					T temp = cosTheta * wa4[k] + sinTheta * qtbpj;
					qtbpj = -sinTheta * wa4[k] + cosTheta * qtbpj;
					wa4[k] = temp;

					/// accumulate the tranformation in the row of s.
					for ( unsigned i = k + 1; i < m_n; i++ )
					{
						temp = cosTheta * r[k * m_m + i] + sinTheta * sdiag[i];
						sdiag[i] = -sinTheta * r[k * m_m + i] + cosTheta * sdiag[i];
						r[k * m_m + i] = temp;
					}
				}
			}

		}

		/// store the diagonal element of s and restore the corresponding diagonal element of r.
		sdiag[j] = r[j * m_m + j];
		r[j * m_m + j] = x[j];
	}

	unsigned nsing = m_n;
	for ( unsigned j = 0; j < m_n; j++ )
	{
		if ( sdiag[j] == 0. && nsing == m_n )
		{
			nsing = j;
		}
		if ( nsing < m_n )
		{
			wa4[j] = 0;
		}
	}

	for ( int j = nsing - 1; j >= 0; j-- )
	{
		T sum = 0;
		for ( unsigned i = j + 1; i < nsing; i++ )
		{
			sum += r[j * m_m + i] * wa4[i];
		}
		wa4[j] = ( wa4[j] - sum ) / sdiag[j];
	}

	/// permute the components of z back to components of x.
	for ( unsigned j = 0; j < m_n; j++ )
	{
		x[m_ipvt[j]] = wa4[j];
	}
}

template<typename T, typename ErrorFn, template<typename> class Traits>
T LevenbergMarquardt<T, ErrorFn, Traits>::euclideanNorm( typename std::vector<T>::const_iterator begin, typename std::vector<T>::const_iterator end ) const
{
	unsigned int n = std::distance( begin, end );

	T s1 = 0;
	T s2 = 0;
	T s3 = 0;
	T x1max = 0;
	T x3max = 0;
	T agiant = Traits<T>::sqrtMax() / (( T ) n );

	/// sum squares
	for ( typename std::vector<T>::const_iterator it = begin; it != end; ++it )
	{
		T xabs = Imath::Math<T>::fabs( *it );
		if ( xabs > Traits<T>::sqrtMin() && xabs < agiant )
		{
			///  sum for intermediate components.
			s2 += xabs * xabs;
		}
		else
		{
			if ( xabs > Traits<T>::sqrtMin() )
			{
				///  sum for large components.
				if ( xabs > x1max )
				{
					s1 = 1 + s1 * sqr( x1max / xabs );
					x1max = xabs;
				}
				else
				{
					s1 += sqr( xabs / x1max );
				}
			}
			else
			{
				///  sum for small components.
				if ( xabs > x3max )
				{
					s3 = 1 + s3 * sqr( x3max / xabs );
					x3max = xabs;
				}
				else
				{
					if ( xabs != 0. )
					{
						s3 += sqr( xabs / x3max );
					}
				}
			}
		}
	}

	/// calculation of norm.
	if ( s1 != 0 )
	{
		return x1max * Imath::Math<T>::sqrt( s1 + ( s2 / x1max ) / x1max );
	}

	if ( s2 != 0 )
	{
		if ( s2 >= x3max )
		{
			return Imath::Math<T>::sqrt( s2 * ( 1 + ( x3max / s2 ) * ( x3max * s3 ) ) );
		}
		else
		{
			return Imath::Math<T>::sqrt( x3max * (( s2 / x3max ) + ( x3max * s3 ) ) );
		}
	}

	return x3max * Imath::Math<T>::sqrt( s3 );
}

} // namespace IECore

#endif // IE_CORE_LEVENBERGMARQUARDT_INL
