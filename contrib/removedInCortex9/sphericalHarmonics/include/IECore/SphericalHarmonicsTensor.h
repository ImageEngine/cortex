//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_SPHERICALHARMONICSTENSOR_H
#define IECORE_SPHERICALHARMONICSTENSOR_H

#include <vector>
#include <complex>
#include "boost/noncopyable.hpp"
#include "boost/function.hpp"
#include "boost/tuple/tuple.hpp"
#include "tbb/queuing_rw_mutex.h"
#include "IECore/Export.h"


namespace IECore
{

/// This class computes Spherical Harmonics triple product integrals, also called tensors.
/// It is used on the SphericalHarmonicsTransferMatrix and the product of two SH objects.
/// Work based on: 
/// "Some properties of the coupling coefﬁcients of real spherical harmonics and their relation to Gaunt coefﬁcients"
/// By Herbert H.H. Homeier, E. Otto Steinborn, 1996.
/// I've based on equation 26 as opposed to the optimized special case equations. 
/// In order to get the triple product integrals I had to ignore the conjugate of U on equation 26. 
/// Gaunt and Wigner 3J symbols computation based on:
/// Wolfram Mathworld : http://mathworld.wolfram.com/Wigner3j-Symbol.html and
/// Sage Documentation : http://www.sagemath.org/doc/reference/sage/functions/wigner.html
/// Lucio Moser - March 2010
/// \ingroup shGroup
class IECORE_API SphericalHarmonicsTensor : private boost::noncopyable
{
	public :

		// Returns the single instance supposed to be used to compute SH triple product integrals.
		static SphericalHarmonicsTensor &tensor();

		typedef boost::function< void ( unsigned int, unsigned int, unsigned int, double ) > tensorFunc;

		// Main evaluation method: calls the given functor passing the computed non-zero SH tensor values along with
		// corresponding SH coefficient indices i,j,k, up to the given number of bands.
		// It considers the indices permutations and calls the functor as many times as necessary.
		// Thread-safe function.
		void evaluate( size_t bands, tensorFunc functor );
		
	private :

		volatile size_t m_bands;
		tbb::queuing_rw_mutex m_mutex;

		enum IndexPermutation 
		{
			IJK = 0,
			IIK,
			IJJ,
			III
		};

		typedef std::vector< boost::tuple< unsigned int, unsigned int, unsigned int, IndexPermutation, double > > TensorVector;

		// all unique non-zero tensors are stored on this array containing ijk indices, how they repeat themselves and the tensor value.
		TensorVector m_tensors;

		void compute( unsigned int bands );

		static double wigner3j( int Ji, int Mi, int Jj, int Mj, int Jk, int Mk );
		static double wigner3j0( int Ji, int Jj, int Jk );
		static double gaunt( int Ji, int Mi, int Jj, int Mj, int Jk, int Mk );
		static std::complex<double> U( int l, int m, int u );
		static double realGaunt( int Ji, int Mi, int Jj, int Mj, int Jk, int Mk );

	private :

		// prevent external instantiation of this class
		SphericalHarmonicsTensor();

};

} // namespace IECore

#endif // IECORE_SPHERICALHARMONICSTENSOR_H
