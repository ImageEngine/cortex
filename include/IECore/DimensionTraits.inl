//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_DIMENSIONTRAITS_INL
#define IE_CORE_DIMENSIONTRAITS_INL

#include "boost/static_assert.hpp"

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathPlane.h"

#include "IECore/TypeTraits.h"
#include "IECore/LineSegment.h"

namespace IECore
{

/// 1-D partial specialization
template<typename T>
struct DimensionTraits<1, T>
{
	BOOST_STATIC_ASSERT( boost::is_arithmetic<T>::value );
	typedef T ValueType;
	typedef T VectorType;
	typedef void BoxType;
	typedef void PlaneType;
	typedef void LineSegmentType;
};

/// 2-D partial specialization
template<typename T>
struct DimensionTraits<2, T>
{
	BOOST_STATIC_ASSERT( boost::is_arithmetic<T>::value );
	typedef T ValueType;
	typedef Imath::Vec2<T> VectorType;
	typedef Imath::Box< VectorType > BoxType;
	typedef void PlaneType;
	typedef LineSegment< VectorType > LineSegmentType;
};

/// 3-D partial specialization
template<typename T>
struct DimensionTraits<3, T>
{
	BOOST_STATIC_ASSERT( boost::is_arithmetic<T>::value );
	typedef T ValueType;
	typedef Imath::Vec3<T> VectorType;
	typedef Imath::Box< VectorType > BoxType;
	typedef Imath::Plane3<T> PlaneType;
	typedef LineSegment< VectorType > LineSegmentType;
};

}

#endif // IE_CORE_DIMENSIONTRAITS_INL
