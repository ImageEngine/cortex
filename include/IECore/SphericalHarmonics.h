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

#ifndef IECORE_SPHERICALHARMONICS_H
#define IECORE_SPHERICALHARMONICS_H

#include "IECore/VectorTraits.h"
#include "OpenEXR/ImathVec.h"
#include <vector>
#include <algorithm>

namespace IECore
{

// Class for representing a set of real spherical harmonics basis functions scaled by coefficients.
// Based mainly on "Spherical Harmonic Lighting: The Gritty Details" by Robin Green.
// \todo see if we can get the result type of multiplying two objects using data traits and apply it to dot() then create ^ operator.
template < typename V >
class SphericalHarmonics 
{
	public :
		typedef typename VectorTraits<V>::BaseType BaseType;
		typedef V ValueType;
		typedef std::vector<V> CoefficientVector;

		SphericalHarmonics( unsigned int bands = 0 );

		SphericalHarmonics( const SphericalHarmonics &sh );

		~SphericalHarmonics();

		const SphericalHarmonics & operator =( const SphericalHarmonics &sh );

		// return number of spherical harmonics bands represented by this object
		unsigned int bands() const
		{
			return m_bands;
		}

		// change the number of bands without changing the shape of the spherical harmonics.
		void setBands( unsigned int bands );

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

		// evaluates the spherical harmonics at spherical coordinates phi and theta.
		inline V operator() ( const Imath::Vec2< BaseType > &phiTheta ) const;

		// evaluates the spherical harmonics at spherical coordinates phi and theta up to the given number of bands.
		inline V operator() ( const Imath::Vec2< BaseType > &phiTheta, unsigned int bands ) const;

		// evaluates the spherical harmonics at euclidian coordinates ( normalized vector ).
		inline V operator() ( const Imath::Vec3< BaseType > &xyz ) const;

		// evaluates the spherical harmonics at euclidian coordinates up to the given number of bands.
		inline V operator() ( const Imath::Vec3< BaseType > &xyz, unsigned int bands ) const;

		// dot product on the coefficient vectors.
		// The return value is dependent on the result of the multiplication between the two harmonics coefficients.
		template <typename T, typename R>
		R dot( const SphericalHarmonics<T> &s ) const;

		// dot product operator
		// This operators only works if the left SH object is templated on the most complex data type such that V ^ T results in V type.
		template < typename T >
		V operator ^ ( const SphericalHarmonics<T> &s ) const
		{
			return dot< T, V >( s );
		}

		// convolves a given SH kernel on the current SH object.
		// The kernel should be a function dependent only on theta.
		template < typename T >
		void convolve( const SphericalHarmonics<T> &s );

	protected :

		class SHEvaluator
		{
			public:
				SHEvaluator( const SphericalHarmonics *sh ) : m_result(0), m_coeffs( sh->coefficients().begin() )
				{
				}

				void operator() ( unsigned int l, int m, BaseType v )
				{
					m_result += m_coeffs[ l*(l+1)+m ] * V(v);
				}

				const V & result() const
				{
					return m_result;
				}

			private:

				V m_result;
				typename std::vector< V >::const_iterator m_coeffs;
		};

		unsigned int m_bands;
		std::vector< V > m_coefficients;
};


typedef SphericalHarmonics<float> SHf;
typedef SphericalHarmonics<double> SHd;
typedef SphericalHarmonics<Imath::V3f> SHV3f;
typedef SphericalHarmonics<Imath::V3d> SHV3d;


// Define addition of two SphericalHarmonics object.
// The resulting object will be resized to accomodate the maximum number of bands between the two SH.
template <class S>
SphericalHarmonics<S> operator + ( const SphericalHarmonics<S> &lsh, const SphericalHarmonics<S> &rsh );

// Define inplace addition of two SphericalHarmonics object.
// The resulting object will be resized to accomodate the maximum number of bands between the two SH.
template <class S>
const SphericalHarmonics<S> & operator += ( SphericalHarmonics<S> &lsh, const SphericalHarmonics<S> &rsh );

// Define subtraction of two SphericalHarmonics object.
// The resulting object will be resized to accomodate the maximum number of bands between the two SH.
template <class S>
SphericalHarmonics<S> operator - ( const SphericalHarmonics<S> &lsh, const SphericalHarmonics<S> &rsh );

// Define inplace subtraction of two SphericalHarmonics object.
// The resulting object will be resized to accomodate the maximum number of bands between the two SH.
template <class S>
const SphericalHarmonics<S> & operator -= ( SphericalHarmonics<S> &lsh, const SphericalHarmonics<S> &rsh );

// Define scaling of an SphericalHarmonics object.
// Each coefficient will be scaled independently.
template <class S, class T>
SphericalHarmonics<S> operator * ( const SphericalHarmonics<S> &lsh, const T &scale );

// Define inplace scaling of an SphericalHarmonics object.
// Each coefficient will be scaled independently.
template <class S, class T>
const SphericalHarmonics<S> & operator *= ( SphericalHarmonics<S> &lsh, const T &scale );


} // namespace IECore

#include "SphericalHarmonics.inl"

#endif // IECORE_SPHERICALHARMONICS_H
