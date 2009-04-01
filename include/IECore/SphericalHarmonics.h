//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_SPHERICALHARMONICS_H
#define IECORE_SPHERICALHARMONICS_H

#include "IECore/RefCounted.h"
#include <vector>

namespace IECore
{

// Class for representing a set of real spherical harmonics basis functions scaled by coefficients.
// Based mainly on "Spherical Harmonic Lighting: The Gritty Details" by Robin Green.
// \todo test dot() function and add + operator
template < typename V >
class SphericalHarmonics : public RefCounted
{
	public :
		typedef V ValueType;
		typedef std::vector<V> CoefficientVector;
		typedef std::vector<V> EvaluationVector;

		IE_CORE_DECLAREMEMBERPTR( SphericalHarmonics );

		SphericalHarmonics( unsigned int bands ) : m_bands(bands)
		{
			m_coefficients.resize( bands * bands, V(0) );
		}

		virtual ~SphericalHarmonics()
		{
		}

		// return number of spherical harmonics bands represented by this object
		unsigned int bands() const
		{
			return m_bands;
		}

		// returns all the coefficients
		const std::vector< V > &coefficients() const
		{
			return m_coefficients;
		}

		// returns all the coefficients
		std::vector< V > &coefficients()
		{
			return m_coefficients;
		}

		// computes the spherical harmonics at polar coordinates theta and phi.
		void evaluate( V theta, V phi, EvaluationVector &result ) const;

		// computes the spherical harmonics at polar coordinates theta and phi up to the given band.
		void evaluate( unsigned int band, V theta, V phi, EvaluationVector &result ) const;

		// dot product on the coefficient vectors.
		// The return value is dependent on the result of the multiplication between the two harmonics coefficients.
		template <typename T, typename R>
		R dot( typename SphericalHarmonics<T>::ConstPtr &s ) const;

	protected :

		unsigned int m_bands;
		std::vector< V > m_coefficients;
};

typedef SphericalHarmonics<float> FloatSh;
typedef SphericalHarmonics<double> DoubleSh;

} // namespace IECore

#include "SphericalHarmonics.inl"

#endif // IECORE_SPHERICALHARMONICS_H
