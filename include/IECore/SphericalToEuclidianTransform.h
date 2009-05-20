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

#ifndef IE_CORE_SPHERICALTOEUCLIDIANTRANSFORM_H
#define IE_CORE_SPHERICALTOEUCLIDIANTRANSFORM_H

#include "IECore/SpaceTransform.h"
#include "boost/static_assert.hpp"

#include "IECore/TypeTraits.h"

namespace IECore
{

/// Forward declaration
template< typename, typename > class EuclidianToSphericalTransform;

/// A templated SpaceTransform class to perform Spherical coordinates to Euclidian coordinates.
/// Spherical coordinates are defined by two angles: phi and theta stored in x and y components of a Imath::Vec2 structure respectively. They can optionally have a third component specifying the radius. So type F can be either Imath::Vec2<> or Imath::Vec3<>.
/// The theta ranges from 0 to PI and it represents the angle from Z axis. The phi component ranges from 0 to 2*PI and represents the angle of rotation on the XY plane.
template<typename F, typename T>
class SphericalToEuclidianTransform : public SpaceTransform< F, T >
{
	public:
		BOOST_STATIC_ASSERT( (boost::mpl::or_< TypeTraits::IsVec3<F>, TypeTraits::IsVec2<F> >::value == true) );
		BOOST_STATIC_ASSERT( (TypeTraits::IsVec3<T>::value) );

		typedef EuclidianToSphericalTransform< T, F > InverseType;

		SphericalToEuclidianTransform();

		/// Perform the conversion. The x component should be in the range [0,2*M_PI] and the second [0,M_PI]
		virtual T transform( const F &f );

		/// Returns an instance of a class able to perform the inverse conversion
		InverseType inverse() const;
};

} // namespace IECore

#include "IECore/SphericalToEuclidianTransform.inl"

#endif // IE_CORE_SPHERICALTOEUCLIDIANTRANSFORM_H
