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

//! \file SphericalHarmonicsAlgo.h
/// Defines additional spherical harmonics algorithms.
/// \ingroup shGroup

#ifndef IECORE_SPHERICALHARMONICSALGO_H
#define IECORE_SPHERICALHARMONICSALGO_H

#include "IECore/SphericalHarmonics.h"

namespace IECore
{

template <class S, class T>
SphericalHarmonics<S> operator * ( const SphericalHarmonics<S> &sh1, const SphericalHarmonics<T> &sh2 );

template <class S, class T>
const SphericalHarmonics<S> operator *= ( SphericalHarmonics<S> &sh1, const SphericalHarmonics<T> &sh2 );

/// Creates a SphericalHarmonics kernel that represents the lambert cosine rule
/// From "On the Relationship between Radiance and Irradiance: Determining the illumination from images of a convex Lambertian object" by
/// by Ramamoorthi, Ravi and Hanrahan, Pat - 2001.
template < class T >
SphericalHarmonics<T> lambertianKernel( unsigned int bands, bool normalized = false );

/// Creates a SphericalHarmonics object by pointing a given SH kernel to a given direction.
/// The rotation is a lot faster than SHRotation because it takes in consideration the kernel symmetries on the Z axis.
/// Based on "Real-time Soft Shadows in Dynamic Scenes using Spherical Harmonic Exponentiation" by
/// Zhong Ren and Rui Wang and John Snyder and Kun Zhou and Xinguo Liu and Bo Sun and Peter-pike Sloan and Hujun Bao and Qunsheng Peng and Baining Guo - 2006.
template < class T >
SphericalHarmonics<T> rotatedKernel( const SphericalHarmonics<T> &kernel, const Imath::V3f &direction );

/// Applies windowing filter to attenuate "ringing" artifacts.
/// Based on "Real-time Soft Shadows in Dynamic Scenes using Spherical Harmonic Exponentiation" by
/// Zhong Ren and Rui Wang and John Snyder and Kun Zhou and Xinguo Liu and Bo Sun and Peter-pike Sloan and Hujun Bao and Qunsheng Peng and Baining Guo - 2006.
/// The authors suggest using windowSize = 2*bands
template < class T >
void windowingFilter( SphericalHarmonics<T> &sh, float windowSize );

} // namespace IECore

#include "SphericalHarmonicsAlgo.inl"

#endif // IECORE_SPHERICALHARMONICSALGO_H
