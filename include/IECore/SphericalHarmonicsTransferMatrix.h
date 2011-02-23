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

#ifndef SPHERICALHARMONICSTRANSFERMATRIX_H
#define SPHERICALHARMONICSTRANSFERMATRIX_H

#include "IECore/SphericalHarmonics.h"
#include "IECore/SphericalHarmonicsTensor.h"

namespace IECore
{

/// This class represents the transfer matrix for any given SH object.
/// It can be applied to any given SH object by the multiplication operator.
/// \todo: should consider matrix sparsity.
/// \ingroup shGroup
template< class S >
class SphericalHarmonicsTransferMatrix
{
	public:

		SphericalHarmonicsTransferMatrix( const SphericalHarmonics<S> &sh );

		// Applies the transformation over the given SH object.
		template < class T >
		void transform( SphericalHarmonics<T> &sh ) const;

	private:

		struct TransferMatrixFunctor
		{
			TransferMatrixFunctor( SphericalHarmonicsTransferMatrix &m, const SphericalHarmonics<S> &sh ) : m_sh( sh ), m_transfer( m )
			{
			}

			void operator() ( unsigned int i, unsigned int j, unsigned int k, double tensor )
			{
				m_transfer.m_matrix[ i * m_transfer.m_columns + j ] += m_sh.coefficients()[ k ] * tensor;
			}

			private :
	
				const SphericalHarmonics<S> &m_sh;
				SphericalHarmonicsTransferMatrix &m_transfer;			
		};

	protected:

		int m_columns;
		std::vector<S> m_matrix;
};

// Define in-place multiplication of a SphericalHarmonics by the transfer matrix.
template <class S, class T>
const SphericalHarmonics<S> & operator *= (SphericalHarmonics<S> &sh, const SphericalHarmonicsTransferMatrix<T> &m)
{
	m.transform( sh );
	return sh;
}

typedef SphericalHarmonicsTransferMatrix< double > SHTransferMatrixd;
typedef SphericalHarmonicsTransferMatrix< float > SHTransferMatrixf;

} // namespace IECore

#include "IECore/SphericalHarmonicsTransferMatrix.inl"

#endif // SPHERICALHARMONICSTRANSFERMATRIX_H
