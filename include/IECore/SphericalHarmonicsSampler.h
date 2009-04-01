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

#ifndef IECORE_SPHERICALHARMONICSSAMPLER_H
#define IECORE_SPHERICALHARMONICSSAMPLER_H

#include "boost/static_assert.hpp"
#include "boost/type_traits.hpp"

#include "IECore/VectorTypedData.h"
#include "IECore/SphericalHarmonics.h"

namespace IECore
{

// Class that discretizes the spherical harmonics functions on a set of points on the sphere.
// The object defines a distribution of points over the sphere surface and uses that distribution to project a given function
// onto a spherical harmonics object or from a given spherical harmonics reconstruct the function by returning the values at the
// sampling points.
// Based mainly on "Spherical Harmonic Lighting: The Gritty Details" by Robin Green.
template < typename V >
class SphericalHarmonicsSampler : public RefCounted
{
	public:
		BOOST_STATIC_ASSERT( boost::is_floating_point<V>::value );

		typedef std::vector< V > EvaluationVector;
		typedef std::vector< EvaluationVector > EvaluationSamples;

		IE_CORE_DECLAREMEMBERPTR( SphericalHarmonicsSampler );

		// uses unbiased uniform distribution
		// the actual number of samples is rounded to sqrt(samples)*sqrt(samples).
		SphericalHarmonicsSampler( unsigned int bands, unsigned int samples, unsigned long int seed = 0 );

		// uses given uniform point distribution.
		// V2f defines (theta and phi) angles.
		SphericalHarmonicsSampler( unsigned int bands, ConstV2fVectorDataPtr &sphericalCoordinates );

		// uses given non-uniform point distribution and weights.
		// The weight should be proportional to each sample's spherical area. 
		// Uniform distribution results in weight constant = 4*PI
		SphericalHarmonicsSampler( unsigned int bands, ConstV2fVectorDataPtr &sphericalCoordinates, ConstFloatVectorDataPtr &weights );

		// returns all the samples coordinates used on this projector in polar coordinates (theta and phi).
		ConstV2fVectorDataPtr sphericalCoordinates() const
		{
			return m_sphericalCoordinates;
		}

		// returns all the sample coordinates in euclidian space ( unit vector in 3D space ).
		ConstV3fVectorDataPtr euclidianCoordinates() const;

		// returns all the spherical harmonics used on this projector representing each sample point on the sphere.
		const EvaluationSamples &sphericalHarmonicsSamples() const
		{
			return m_shEvaluations;
		}

		// returns the weights for each sample. In case it's uniform distribution it returns 0.
		ConstFloatVectorDataPtr weights() const
		{
			return m_weights;
		}

		// Sets the given SphericalHarmonics object with the projection of the given functor for every polar coordinate.
		// the value stored in the spherical harmonics is the same as the functor return type.
		template< typename T, typename U > 
		void polarProjection( T functor, boost::intrusive_ptr< SphericalHarmonics< U > > result ) const;

		// Sets the given SphericalHarmonics object with the projection of the given functor for every euclidean direction.
		// the value stored in the spherical harmonics is the same as the functor return type.
		template< typename T, typename U > 
		void euclideanProjection( T functor, boost::intrusive_ptr< SphericalHarmonics< U > > result ) const;

		// returns a vector of "points" reconstructing a function defined by the given spherical harmonics.
		// the returned type depends on the given spherical harmonics object.
		template< typename T > 
		void reconstruction( const boost::intrusive_ptr< SphericalHarmonics< T > > sh, boost::intrusive_ptr< TypedData< std::vector< T > > > result ) const;

	protected:

		static Imath::V3f sphericalCoordsToUnitVector( const Imath::V2f &sphericalCoord );

		// computes without temporary storage: harmonics coefficients += evaluationVector * scale
		template <typename T>
		static void addProjection( typename SphericalHarmonics<T>::CoefficientVector &c, const EvaluationVector &v, const T &scale );

		void evaluateSphericalHarmonicsSamples();

		unsigned int m_bands;
		V2fVectorDataPtr m_sphericalCoordinates;
		mutable V3fVectorDataPtr m_euclidianCoordinates;
		EvaluationSamples m_shEvaluations;
		FloatVectorDataPtr m_weights;
};

typedef SphericalHarmonicsSampler<float> FloatShSampler;
typedef SphericalHarmonicsSampler<double> DoubleShSampler;

} // namespace IECore

#include "SphericalHarmonicsSampler.inl"

#endif // IECORE_SPHERICALHARMONICSSAMPLER_H
