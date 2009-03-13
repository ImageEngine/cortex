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

#ifndef IE_CORE_TYPETRAITS_H
#define IE_CORE_TYPETRAITS_H

#include "boost/static_assert.hpp"

#include "boost/mpl/or.hpp"
#include "boost/mpl/and.hpp"
#include "boost/mpl/if.hpp"
#include "boost/mpl/not.hpp"

#include "boost/type_traits.hpp"

#include "IECore/HalfTypeTraits.h"
#include "IECore/SimpleTypedData.h"
#include "IECore/VectorTypedData.h"
#include "IECore/Spline.h"
#include "IECore/SplineData.h"
#include "IECore/DateTimeData.h"
#include "IECore/TransformationMatrixData.h"

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathQuat.h"
#include "OpenEXR/ImathBox.h"

namespace IECore
{

namespace TypeTraits
{

/// HasValueType
template<typename T> struct HasValueType : public boost::false_type {};
template<typename T> struct HasValueType< TypedData<T> > : public boost::true_type {};
template<typename T> struct HasValueType< const TypedData<T> > : public boost::true_type {};

/// HasVectorValueType
template<typename T> struct HasVectorValueType : public boost::false_type {};
template<typename T> struct HasVectorValueType< TypedData< std::vector< T > > > : public boost::true_type {};
template<typename T> struct HasVectorValueType< const TypedData< std::vector< T > > > : public boost::true_type {};

namespace Detail
{

template <class T>
struct GetValueType
{
	typedef typename T::ValueType type;
};

template <class T>
struct GetVectorValueType
{
	typedef typename T::ValueType::value_type type;
};

} // namespace Detail

template <class T>
struct ValueType
{
	typedef typename boost::mpl::eval_if< HasValueType<T>, Detail::GetValueType<T>, boost::mpl::identity<T> >::type type;
};

template <class T>
struct VectorValueType
{
	typedef typename boost::mpl::eval_if< HasVectorValueType<T>, Detail::GetVectorValueType<T>, boost::mpl::identity<T> >::type type;
};

/// IsTypedData
template<typename T> struct IsTypedData : public boost::false_type {};
template<typename T> struct IsTypedData< TypedData<T> > : public boost::true_type {};
template<typename T> struct IsTypedData< const TypedData<T> > : public boost::true_type {};

/// IsVectorTypedData
template<typename T> struct IsVectorTypedData : public boost::false_type {};
template<typename T> struct IsVectorTypedData< TypedData<std::vector<T> > > : public boost::true_type {};
template<typename T> struct IsVectorTypedData< const TypedData<std::vector<T> > > : public boost::true_type {};

/// IsSimpleTypedData
template<typename T> struct IsSimpleTypedData : public boost::false_type {};
template<typename T> struct IsSimpleTypedData< TypedData<T > > : public boost::true_type {};
template<typename T> struct IsSimpleTypedData< const TypedData<T > > : public boost::true_type {};
template<typename T> struct IsSimpleTypedData< TypedData<std::vector<T> > > : public boost::false_type {};
template<typename T> struct IsSimpleTypedData< const TypedData<std::vector<T> > > : public boost::false_type {};

/// IsMatrix33
template<typename T> struct IsMatrix33 : public boost::false_type {};
template<typename T> struct IsMatrix33< Imath::Matrix33<T> > : public boost::true_type {};
template<typename T> struct IsMatrix33< const Imath::Matrix33<T> > : public boost::true_type {};

/// IsMatrix44
template<typename T> struct IsMatrix44 : public boost::false_type {};
template<typename T> struct IsMatrix44< Imath::Matrix44<T> > : public boost::true_type {};
template<typename T> struct IsMatrix44< const Imath::Matrix44<T> > : public boost::true_type {};

/// IsMatrix
template<typename T> struct IsMatrix : boost::mpl::or_< IsMatrix33<T>, IsMatrix44<T> > {};

/// IsVec3
template<typename T> struct IsVec3 : public boost::false_type {};
template<typename T> struct IsVec3< Imath::Vec3<T> > : public boost::true_type {};
template<typename T> struct IsVec3< const Imath::Vec3<T> > : public boost::true_type {};

/// IsVec2
template<typename T> struct IsVec2 : public boost::false_type {};
template<typename T> struct IsVec2< Imath::Vec2<T> > : public boost::true_type {};
template<typename T> struct IsVec2< const Imath::Vec2<T> > : public boost::true_type {};

/// IsVec
template<typename T> struct IsVec : boost::mpl::or_< IsVec3<T>, IsVec2<T> > {};

/// IsColor3
template<typename T> struct IsColor3 : public boost::false_type {};
template<typename T> struct IsColor3< Imath::Color3<T> > : public boost::true_type {};
template<typename T> struct IsColor3< const Imath::Color3<T> > : public boost::true_type {};

/// IsColor4
template<typename T> struct IsColor4 : public boost::false_type {};
template<typename T> struct IsColor4< Imath::Color4<T> > : public boost::true_type {};
template<typename T> struct IsColor4< const Imath::Color4<T> > : public boost::true_type {};

/// IsColor
template<typename T> struct IsColor : boost::mpl::or_< IsColor3<T>, IsColor4<T> > {};

/// IsQuat
template<typename T> struct IsQuat : public boost::false_type {};
template<typename T> struct IsQuat< Imath::Quat<T> > : public boost::true_type {};
template<typename T> struct IsQuat< const Imath::Quat<T> > : public boost::true_type {};

/// IsBox
template<typename T> struct IsBox : public boost::false_type {};
template<typename T> struct IsBox< Imath::Box<T> > : public boost::true_type {};
template<typename T> struct IsBox< const Imath::Box<T> > : public boost::true_type {};

/// IsTransformationMatrix
template<typename T> struct IsTransformationMatrix : public boost::false_type {};
template<typename T> struct IsTransformationMatrix< TransformationMatrix<T> > : public boost::true_type {};
template<typename T> struct IsTransformationMatrix< const TransformationMatrix<T> > : public boost::true_type {};

/// IsMatrixTypedData
template< typename T > struct IsMatrixTypedData : boost::mpl::and_< IsTypedData<T>, IsMatrix< typename ValueType<T>::type > > {};

/// IsVec2TypedData
template< typename T > struct IsVec2TypedData : boost::mpl::and_< IsTypedData<T>, IsVec2< typename ValueType<T>::type > > {};

/// IsVec2VectorTypedData
template< typename T > struct IsVec2VectorTypedData : boost::mpl::and_< IsVectorTypedData<T>, IsVec2< typename VectorValueType<T>::type > > {};

/// IsVec3TypedData
template< typename T > struct IsVec3TypedData : boost::mpl::and_< IsTypedData<T>, IsVec3< typename ValueType<T>::type > > {};

/// IsVec3VectorTypedData
template< typename T > struct IsVec3VectorTypedData : boost::mpl::and_< IsVectorTypedData<T>, IsVec3< typename VectorValueType<T>::type > > {};

/// IsVecTypedData
template< typename T > struct IsVecTypedData : boost::mpl::or_< IsVec2TypedData<T>, IsVec3TypedData<T> > {};

/// IsVecVectorTypedData
template< typename T > struct IsVecVectorTypedData : boost::mpl::or_< IsVec2VectorTypedData<T>, IsVec3VectorTypedData<T> > {};

/// IsNumericVectorTypedData
template< typename T > struct IsNumericVectorTypedData : boost::mpl::and_< IsVectorTypedData<T>, boost::is_arithmetic< typename VectorValueType<T>::type > > {};

/// IsFloatVectorTypedData
template< typename T > struct IsFloatVectorTypedData : boost::mpl::and_< IsVectorTypedData<T>, boost::is_floating_point< typename VectorValueType<T>::type > > {};

/// IsNumericSimpleTypedData
template< typename T > struct IsNumericSimpleTypedData : boost::mpl::and_< IsSimpleTypedData<T>, boost::is_arithmetic< typename ValueType<T>::type > > {};

/// IsNumericTypedData
template< typename T > struct IsNumericTypedData : boost::mpl::or_< IsNumericSimpleTypedData<T>, IsNumericVectorTypedData<T> > {};

/// IsInterpolable
/// This represents all types for which interpolators in Interpolator.h can be instantiated, not necessarily those for which we *want* to perform interpolation 
/// on. For example integral types, while technically interpolable, probably don't want to be interpolated in specific situations.
template< typename T > struct IsInterpolable 
	: boost::mpl::or_< 
		boost::is_arithmetic<T>,
		boost::mpl::or_<
			IsColor<T>,
			boost::mpl::or_<
				IsQuat<T>,
				boost::mpl::or_<
					IsVec<T>,
					boost::mpl::or_<
						IsTransformationMatrix<T>,
						IsBox<T>
					>
				>
			>
		>
	> {} ;

/// IsInterpolableTypedData
template< typename T > struct IsInterpolableTypedData : boost::mpl::and_< IsTypedData<T>, IsInterpolable< typename ValueType<T>::type > > {};

/// IsInterpolableVectorTypedData
template< typename T > struct IsInterpolableVectorTypedData : boost::mpl::and_< IsVectorTypedData<T>, IsInterpolable< typename VectorValueType<T>::type > > {};

/// IsInterpolableSimpleTypedData
template< typename T > struct IsInterpolableSimpleTypedData : boost::mpl::and_< IsSimpleTypedData<T>, IsInterpolable< typename ValueType<T>::type > > {};

/// IsSpline
template<typename T, typename U = void > struct IsSpline : public boost::false_type {};
template<typename T, typename U> struct IsSpline< Spline<T, U> > : public boost::true_type {};
template<typename T, typename U> struct IsSpline< const Spline<T, U> > : public boost::true_type {};

/// IsSplineTypedData
template< typename T > struct IsSplineTypedData : boost::mpl::and_< IsTypedData<T>, IsSpline< typename ValueType<T>::type > > {};


} // namespace TypeTraits

} // namespace IECore

#endif // IE_CORE_TYPETRAITS_H
