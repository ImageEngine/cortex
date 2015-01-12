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

namespace IECore
{

template< class S >
SphericalHarmonicsTransferMatrix<S>::SphericalHarmonicsTransferMatrix( const SphericalHarmonics<S> &sh )
{
	int coeffs = sh.coefficients().size();
	m_columns = coeffs;

	m_matrix.resize( m_columns * m_columns, S(0) );
	TransferMatrixFunctor func( *this, sh );
	SphericalHarmonicsTensor::tensor().evaluate( sh.bands(), func );
}

template< class S >
template < class T >
void SphericalHarmonicsTransferMatrix<S>::transform( SphericalHarmonics<T> &sh ) const
{
	typename SphericalHarmonics<T>::CoefficientVector::iterator itRes = sh.coefficients().begin();

	std::vector<T> tmp( sh.coefficients() );

	for ( int i = 0; i < m_columns && itRes != sh.coefficients().end(); itRes++, i++ )
	{
		typename std::vector<T>::const_iterator it = tmp.begin();
		S sum(0);
		for ( int j = 0; j < m_columns && it != tmp.end(); j++, it++ )
		{
			sum += (*it) * m_matrix[ j * m_columns + i ];
		}
		*itRes = sum;
	}
}

} // namespace IECore
