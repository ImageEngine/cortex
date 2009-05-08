//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_PERLINNOISE_H
#define IE_CORE_PERLINNOISE_H

#include "IECore/VectorTraits.h"

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathColor.h"

#include <vector>

namespace IECore
{

/// A smoothstep shaped falloff for use with the PerlinNoise class.
/// This is the falloff shape from Ken Perlin's original noise implementation. It's
/// quick but does display discontinuities along grid boundaries, which become
/// pretty severe if the noise is used for bump mapping or displacement.
/// If better continuity is required then the SmootherStepFalloff functor
/// may be a good alternative.
template<typename T>
struct SmoothStepFalloff
{
	inline T operator()( T t ) const;
};

/// A falloff for use with the PerlinNoise class. It's a little
/// slower than the SmoothStepFalloff equivalent but provides a higher
/// order of continuity and is therefore less prone to displaying grid
/// artifacts.
template<typename T>
struct SmootherStepFalloff
{
	inline T operator()( T t ) const;
};

/// The PerlinNoise class template provides perlin noise functions
/// across arbitrary dimensions of input and output. It uses the
/// VectorTraits.h and VectorOps.h functionality to operate
/// with different input and output types. P is the type of the Point
/// class over which the noise is defined and V is the type of the value
/// computed as the noise result. Both P and V must be types for which
/// VectorTraits have been properly defined. F is the type of a functor
/// providing a falloff function - see SmoothStepFalloff for an example
/// of one of these.
/// \todo 4d ones
template<typename P, typename V, typename F>
class PerlinNoise
{

	public :
		typedef PerlinNoise<P,V,F> *Ptr;
		typedef P Point;
		typedef VectorTraits<P> PointTraits;
		typedef typename VectorTraits<P>::BaseType PointBaseType;
		typedef V Value;
		typedef VectorTraits<V> ValueTraits;
		typedef typename VectorTraits<V>::BaseType ValueBaseType;

		/// Constructs a new PerlinNoise object, passing the seed used
		/// by the random number generator to construct the
		/// gradient and permutations tables.
		PerlinNoise( unsigned long int seed = 0 );
		/// Copy constructor
		PerlinNoise( const PerlinNoise &other );

		/// Reinitialises the random gradient table using a
		/// potentially different seed.
		void initGradients( unsigned long int seed );

		/// Computes the noise value at the given point. The components
		/// of the returned value will range from -0.5 to 0.5.
		inline Value noise( const Point &p ) const;

		/// Computes the noise value at the given point. The components
		/// of the returned value will range from -0.5 to 0.5.
		inline Value operator()( const Point &p ) const;

	private :

		static inline ValueBaseType weight( PointBaseType t );

		inline Value noiseWalk( int *pi, const Point &pf, int d ) const;

		static const unsigned int m_maxPointDimensions = 4;
		static const unsigned int m_permSize = 256;
		std::vector<unsigned int> m_perm;
		std::vector<Value> m_grad;

		F m_falloff;

};

/// Typedefs for common uses
typedef PerlinNoise<Imath::V3f, float, SmootherStepFalloff<float> > PerlinNoiseV3ff;
typedef PerlinNoise<Imath::V2f, float, SmootherStepFalloff<float> > PerlinNoiseV2ff;
typedef PerlinNoise<float, float, SmootherStepFalloff<float> > PerlinNoiseff;

typedef PerlinNoise<Imath::V3f, Imath::V2f, SmootherStepFalloff<float> > PerlinNoiseV3fV2f;
typedef PerlinNoise<Imath::V2f, Imath::V2f, SmootherStepFalloff<float> > PerlinNoiseV2fV2f;
typedef PerlinNoise<float, Imath::V2f, SmootherStepFalloff<float> > PerlinNoisefV2f;

typedef PerlinNoise<Imath::V3f, Imath::V3f, SmootherStepFalloff<float> > PerlinNoiseV3fV3f;
typedef PerlinNoise<Imath::V2f, Imath::V3f, SmootherStepFalloff<float> > PerlinNoiseV2fV3f;
typedef PerlinNoise<float, Imath::V3f, SmootherStepFalloff<float> > PerlinNoisefV3f;

typedef PerlinNoise<Imath::V3f, Imath::Color3f, SmootherStepFalloff<float> > PerlinNoiseV3fColor3f;
typedef PerlinNoise<Imath::V2f, Imath::Color3f, SmootherStepFalloff<float> > PerlinNoiseV2fColor3f;
typedef PerlinNoise<float, Imath::Color3f, SmootherStepFalloff<float> > PerlinNoisefColor3f;

} // namespace IECore

#include "IECore/PerlinNoise.inl"

#endif // IE_CORE_PERLINNOISE_H
