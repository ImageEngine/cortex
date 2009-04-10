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

#ifndef IECORE_SPHERICALHARMONICSPROJECTOR_H
#define IECORE_SPHERICALHARMONICSPROJECTOR_H

#include "boost/static_assert.hpp"
#include "boost/type_traits.hpp"

#include <vector>
#include "OpenEXR/ImathVec.h"
#include "IECore/SphericalHarmonics.h"

namespace IECore
{

// Class that discretizes the spherical harmonics functions on a set of points on the sphere.
// The object defines a distribution of points over the sphere surface and uses that distribution to project a given function
// onto a spherical harmonics object or from a given spherical harmonics reconstruct the function by returning the values at the
// sampling points.
// Based mainly on "Spherical Harmonic Lighting: The Gritty Details" by Robin Green.
template < typename V >
class SphericalHarmonicsProjector
{
	public:
		BOOST_STATIC_ASSERT( boost::is_floating_point<V>::value );

		typedef std::vector< V > EvaluationVector;
		typedef std::vector< EvaluationVector > EvaluationSamples;

		// uses unbiased uniform distribution
		SphericalHarmonicsProjector( unsigned int samples, unsigned long int seed = 0 );

		// uses given uniform point distribution.
		// V2f defines (theta and phi) angles.
		SphericalHarmonicsProjector( const std::vector< Imath::Vec2< V > > &sphericalCoordinates );

		// uses given non-uniform point distribution and weights.
		// The weight should be proportional to each sample's spherical area. 
		// Uniform distribution results in weight constant = 4*PI
		SphericalHarmonicsProjector( const std::vector< Imath::Vec2< V > > &sphericalCoordinates, const std::vector< V > &weights );

		// returns all the samples coordinates used on this projector in polar coordinates (theta and phi).
		const std::vector< Imath::Vec2< V > > &sphericalCoordinates() const
		{
			return m_sphericalCoordinates;
		}

		// returns all the sample coordinates in euclidian space ( unit vector in 3D space ).
		const std::vector< Imath::Vec3< V > > &euclidianCoordinates() const;

		// returns the weights for each sample. In case it's uniform distribution it returns empty array.
		const std::vector< V > &weights() const
		{
			return m_weights;
		}

		// Projects just one sample point at the given coordinate index.
		// The function does the appropriate initialization and finalization of the spherical harmonics object on the first and last samples.
		template< typename U > 
		void operator()( unsigned int coordinateIndex, const U &value, SphericalHarmonics< U > &result ) const;

		// Sets the given SphericalHarmonics object with the projection of the given functor for every polar coordinate.
		// the value stored in the spherical harmonics is the same as the functor return type.
		template< typename T, typename U > 
		void polarProjection( T functor,  SphericalHarmonics< U > &result ) const;

		// Sets the given SphericalHarmonics object with the projection of the given functor for every euclidean direction.
		// the value stored in the spherical harmonics is the same as the functor return type.
		template< typename T, typename U > 
		void euclideanProjection( T functor, SphericalHarmonics< U > & result ) const;

		// make sure the evaluations go up to the given number of bands.
		// this function is called by the projection functions, but you may want to use it if the projector is used by several threads.
		void computeSamples( unsigned int bands ) const;

	protected:

		static Imath::Vec3<V> sphericalCoordsToUnitVector( const Imath::Vec2<V> &sphericalCoord );

		// computes without temporary storage: harmonics coefficients += evaluationVector * scale
		template <typename T>
		static void addProjection( typename SphericalHarmonics<T>::CoefficientVector &c, const EvaluationVector &v, const T &scale );


		mutable unsigned int m_bands;
		std::vector< Imath::Vec2< V > > m_sphericalCoordinates;
		mutable std::vector< Imath::Vec3< V > > m_euclidianCoordinates;
		mutable EvaluationSamples m_shEvaluations;
		std::vector< V > m_weights;
};

typedef SphericalHarmonicsProjector<float> SHProjectorf;
typedef SphericalHarmonicsProjector<double> SHProjectord;

} // namespace IECore

#include "SphericalHarmonicsProjector.inl"

#endif // IECORE_SPHERICALHARMONICSPROJECTOR_H
