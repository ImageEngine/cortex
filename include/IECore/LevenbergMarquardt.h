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


#ifndef IE_CORE_LEVENBERGMARQUARDT_H
#define IE_CORE_LEVENBERGMARQUARDT_H

#include "boost/static_assert.hpp"

#include "IECore/TypeTraits.h"
#include "IECore/VectorTypedData.h"

namespace IECore
{

template<typename T>
struct DefaultLevenbergMarquardtTraits;

/// Performs Levenberg-Marquardt minimisation of the given parameters and user-supplied objective function. Based on public domain routines.
///
/// ErrorFn should be a model of:
///
/// template<typename T>
/// class DefaultErrorFn
/// {
///	BOOST_STATIC_ASSERT( boost::is_floating_point<T>::value );
///
///	public :
///
///             /// "errors" is already sized to the correct length, it just needs filling in. Changing the length of either parameter within this
///             /// function is prohibited, and leads to undefined behaviour.
///		void operator()(
///			typename TypedData< std::vector<T> >::ConstPtr parameters,
///			typename TypedData< std::vector<T> >::Ptr errors
///		);
///
///		unsigned numErrors() const;
///
///};
///
/// Traits should be a model of
/// template<typename T>
/// class Traits
/// {
///    static T machinePrecision(); // machine epsilon
///    static T sqrtMin(); /// sqrt of the smallest representable number
///    static T sqrtMax(); /// sqrt of the largest representable number
///    static T tolerance(); /// user defined tolerance
/// };
///
template<typename T, typename ErrorFn, template<typename> class Traits = DefaultLevenbergMarquardtTraits >
class LevenbergMarquardt : public boost::noncopyable
{
		BOOST_STATIC_ASSERT( boost::is_floating_point<T>::value );

	public:
	
		typedef T ValueType;
		typedef ErrorFn ErrorFunctionType;
		typedef Traits<T> TraitsType;

		enum Status
		{
			Success,
			Degenerate,
			CallLimit,  // Maximum number of function calls has been reached.
			FailedFTol, // FTol is too small. No further improvement in solution available.
			FailedXTol, // XTol is too small. No further improvement in solution available.
			FailedGTol, // GTol is too small. No further improvement in solution available.
		};

		LevenbergMarquardt();

		void setParameters(
		        T ftol = Traits<T>::tolerance(),
		        T xtol = Traits<T>::tolerance(),
		        T gtol = Traits<T>::tolerance(),
		        T epsilon = Traits<T>::machinePrecision(),
		        T stepBound = T(100)
		);
		
		void getParameters(
		        T &ftol,
		        T &xtol,
		        T &gtol,
		        T &epsilon,
		        T &stepBound
		) const;				
		
		void setMaxCalls( unsigned maxCalls );
		unsigned getMaxCalls() const;

		/// Updates parameters in place. Returns true on success.
		bool solve( typename TypedData< std::vector<T> >::Ptr parameters, ErrorFn &fn, Status *status = 0 );

	protected :

		T m_ftol;
		T m_xtol;
		T m_gtol;
		T m_epsilon;
		T m_stepBound;

		unsigned int m_maxCalls;
		unsigned int m_numCalls;

		unsigned int m_m;
		unsigned int m_n;

	private :

		inline T sqr( const T &x ) const
		{
			return x * x;
		}

		void qrFactorize();
		T computeLMParameter( std::vector<T> &x, std::vector<T> &sdiag, T delta );

		void qrSolve( std::vector<T> &r, std::vector<T> &diag,
		              std::vector<T> &qtb, std::vector<T> &x, std::vector<T> &sdiag );

		/// Robust way of computing the magnitude (euclidean norm) of a vector
		T euclideanNorm( typename std::vector<T>::const_iterator begin, typename std::vector<T>::const_iterator end ) const;

		typename TypedData< std::vector<T> >::Ptr m_fvec;
		std::vector<T> m_diag;
		std::vector<T> m_qtf;
		std::vector<T> m_fjac;
		std::vector<T> m_wa1;
		typename TypedData< std::vector<T> >::Ptr m_wa2;
		std::vector<T> m_wa3;
		typename TypedData< std::vector<T> >::Ptr m_wa4;
		std::vector<int> m_ipvt;
};

} // namespace IECore

#include "IECore/LevenbergMarquardt.inl"

#endif // IE_CORE_LEVENBERGMARQUARDT_H
