//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_EUCLIDEANTOSPHERICALTRANSFORM_H
#define IE_CORE_EUCLIDEANTOSPHERICALTRANSFORM_H

#include "IECore/SpaceTransform.h"
#include "IECore/TypeTraits.h"

#include "boost/static_assert.hpp"

namespace IECore
{

/// Forward declaration
template< typename, typename > class SphericalToEuclideanTransform;

/// A templated SpaceTransform class to perform Euclidean to Spherical coordinates.
/// The Spherical coordinate structure can optionally have a third component specifying the radius. So type T can be either Imath::Vec2<> or Imath::Vec3<>.
/// Check documentation about SphericalToEuclideanTransform for more details on spherical coordinates.
/// \ingroup mathGroup
template<typename F, typename T>
class EuclideanToSphericalTransform : public SpaceTransform< F, T >
{
	public:
		BOOST_STATIC_ASSERT( ( TypeTraits::IsVec3<F>::value ) );
		BOOST_STATIC_ASSERT( ( boost::mpl::or_< TypeTraits::IsVec3<T>, TypeTraits::IsVec2<T> >::value == true ) );

		typedef EuclideanToSphericalTransform< T, F > InverseType;

		EuclideanToSphericalTransform();

		/// Perform the conversion.
		virtual T transform( const F &f );

		/// Returns an instance of a class able to perform the inverse conversion
		InverseType inverse() const;
};

typedef EuclideanToSphericalTransform<Imath::V3f, Imath::V2f> EuclideanToSphericalTransform3f2f;
typedef EuclideanToSphericalTransform<Imath::V3f, Imath::V3f> EuclideanToSphericalTransform3f3f;
typedef EuclideanToSphericalTransform<Imath::V3d, Imath::V2d> EuclideanToSphericalTransform3d2d;
typedef EuclideanToSphericalTransform<Imath::V3d, Imath::V3d> EuclideanToSphericalTransform3d3d;

} // namespace IECore

#include "IECore/EuclideanToSphericalTransform.inl"

#endif // IE_CORE_EUCLIDEANTOSPHERICALTRANSFORM_H
