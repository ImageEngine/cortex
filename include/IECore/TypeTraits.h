//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "OpenEXR/ImathMatrix.h"

namespace IECore
{

namespace TypeTraits
{

/// HasValueType
template<typename T> struct HasValueType : public boost::false_type {};
template<typename T> struct HasValueType< TypedData<T> > : public boost::true_type {};

/// HasVectorValueType
template<typename T> struct HasVectorValueType : public boost::false_type {};
template<typename T> struct HasVectorValueType< TypedData< std::vector< T > > > : public boost::true_type {};

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

BOOST_STATIC_ASSERT( (sizeof( ValueType< M33fData >::type ) == sizeof( M33fData::ValueType ) ) );
BOOST_STATIC_ASSERT( (sizeof( ValueType< FloatVectorData >::type ) == sizeof( FloatVectorData::ValueType ) ) );

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
template<typename T> struct IsSimpleTypedData< TypedData<std::vector<T> > > : public boost::false_type {};
template<typename T> struct IsSimpleTypedData< const TypedData<std::vector<T> > > : public boost::false_type {};

/// IsMatrix33
template<typename T> struct IsMatrix33 : public boost::false_type {};
template<typename T> struct IsMatrix33< Imath::Matrix33<T> > : public boost::true_type {};

/// IsMatrix44
template<typename T> struct IsMatrix44 : public boost::false_type {};
template<typename T> struct IsMatrix44< Imath::Matrix44<T> > : public boost::true_type {};

/// IsMatrix
template<typename T> struct IsMatrix : boost::mpl::or_< IsMatrix33<T>, IsMatrix44<T> > {};
BOOST_STATIC_ASSERT( (IsMatrix< Imath::M33f >::value) );
BOOST_STATIC_ASSERT( (IsMatrix< Imath::M44d >::value) );
BOOST_STATIC_ASSERT( (IsMatrix< Detail::GetValueType< M33fData >::type >::value ) );
BOOST_STATIC_ASSERT( (IsMatrix< ValueType< M33fData >::type >::value ) );
BOOST_STATIC_ASSERT( (boost::mpl::not_< IsMatrix< Imath::V2i > >::value) );

/// IsVec3
template<typename T> struct IsVec3 : public boost::false_type {};
template<typename T> struct IsVec3< Imath::Vec3<T> > : public boost::true_type {};
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVec3<Imath::V2d> >::value) );

/// IsVec2
template<typename T> struct IsVec2 : public boost::false_type {};
template<typename T> struct IsVec2< Imath::Vec2<T> > : public boost::true_type {};
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVec2<Imath::V3i> >::value) );

/// IsVec
template<typename T> struct IsVec : boost::mpl::or_< IsVec3<T>, IsVec2<T> > {};
BOOST_STATIC_ASSERT( (IsVec<Imath::V3i>::value) );
BOOST_STATIC_ASSERT( (IsVec<Imath::V2d>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVec<int> >::value) );

/// IsColor3
template<typename T> struct IsColor3 : public boost::false_type {};
template<typename T> struct IsColor3< Imath::Color3<T> > : public boost::true_type {};
BOOST_STATIC_ASSERT( ( IsColor3<Imath::Color3f>::value ) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsColor3<Imath::V3f> >::value) );

/// IsColor4
template<typename T> struct IsColor4 : public boost::false_type {};
template<typename T> struct IsColor4< Imath::Color4<T> > : public boost::true_type {};
BOOST_STATIC_ASSERT( ( IsColor4<Imath::Color4f>::value ) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsColor4<Imath::V3f> >::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsColor4<Imath::Color3f> >::value) );

/// IsColor
template<typename T> struct IsColor : boost::mpl::or_< IsColor3<T>, IsColor4<T> > {};
BOOST_STATIC_ASSERT( (IsColor<Imath::Color3f>::value) );
BOOST_STATIC_ASSERT( (IsColor<Imath::Color4f>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsColor<int> >::value) );

/// IsMatrixTypedData
template< typename T > struct IsMatrixTypedData : boost::mpl::and_< IsTypedData<T>, IsMatrix< typename ValueType<T>::type > > {};
BOOST_STATIC_ASSERT( (IsMatrixTypedData<M33fData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsMatrixTypedData<V3fData> >::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsMatrixTypedData<char> >::value) );

/// IsVec2TypedData
template< typename T > struct IsVec2TypedData : boost::mpl::and_< IsTypedData<T>, IsVec2< typename ValueType<T>::type > > {};
BOOST_STATIC_ASSERT( (IsVec2TypedData<V2fData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVec2TypedData<V3iData> >::value) );

/// IsVec3TypedData
template< typename T > struct IsVec3TypedData : boost::mpl::and_< IsTypedData<T>, IsVec3< typename ValueType<T>::type > > {};
BOOST_STATIC_ASSERT( (IsVec3TypedData<V3fData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVec3TypedData<V2iData> >::value) );

/// IsVec3VectorTypedData
template< typename T > struct IsVec3VectorTypedData : boost::mpl::and_< IsVectorTypedData<T>, IsVec3< typename VectorValueType<T>::type > > {};
BOOST_STATIC_ASSERT( (IsVec3VectorTypedData<V3iVectorData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVec3VectorTypedData<V3fData> >::value) );

/// IsVecTypedData
template< typename T > struct IsVecTypedData : boost::mpl::or_< IsVec2TypedData<T>, IsVec3TypedData<T> > {};
BOOST_STATIC_ASSERT( (IsVecTypedData<V2iData>::value) );
BOOST_STATIC_ASSERT( (IsVecTypedData<V3fData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVecTypedData<M33fData> >::value) );

/// IsNumericVectorTypedData
template< typename T > struct IsNumericVectorTypedData : boost::mpl::and_< IsVectorTypedData<T>, boost::is_arithmetic< typename VectorValueType<T>::type > > {};
BOOST_STATIC_ASSERT( (IsNumericVectorTypedData<FloatVectorData>::value) );
BOOST_STATIC_ASSERT( (IsNumericVectorTypedData<UCharVectorData>::value) );
BOOST_STATIC_ASSERT( (IsNumericVectorTypedData<HalfVectorData>::value) );
BOOST_STATIC_ASSERT( (IsNumericVectorTypedData<Int64VectorData>::value) );
BOOST_STATIC_ASSERT( (IsNumericVectorTypedData<UInt64VectorData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsNumericVectorTypedData<StringVectorData> >::value) );

/// IsNumericSimpleTypedData
template< typename T > struct IsNumericSimpleTypedData : boost::mpl::and_< IsSimpleTypedData<T>, boost::is_arithmetic< typename ValueType<T>::type > > {};
BOOST_STATIC_ASSERT( (IsNumericSimpleTypedData<FloatData>::value) );
BOOST_STATIC_ASSERT( (IsNumericSimpleTypedData<ShortData>::value) );
BOOST_STATIC_ASSERT( (IsNumericSimpleTypedData<HalfData>::value) );
BOOST_STATIC_ASSERT( (IsNumericSimpleTypedData<Int64Data>::value) );
BOOST_STATIC_ASSERT( (IsNumericSimpleTypedData<UInt64Data>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsNumericSimpleTypedData<char> >::value) );

/// IsNumericTypedData
template< typename T > struct IsNumericTypedData : boost::mpl::or_< IsNumericSimpleTypedData<T>, IsNumericVectorTypedData<T> > {};

/// IsSpline
template<typename T, typename U = void > struct IsSpline : public boost::false_type {};
template<typename T, typename U> struct IsSpline< Spline<T, U> > : public boost::true_type {};
BOOST_STATIC_ASSERT( (IsSpline< Spline<float, float> >::value) );
BOOST_STATIC_ASSERT( (boost::mpl::not_< IsSpline< Imath::V2f > >::value) );

/// IsSplineTypedData
template< typename T > struct IsSplineTypedData : boost::mpl::and_< IsTypedData<T>, IsSpline< typename ValueType<T>::type > > {};
BOOST_STATIC_ASSERT( (IsSplineTypedData<SplineffData>::value) );
BOOST_STATIC_ASSERT( (IsSplineTypedData<SplineddData>::value) );
BOOST_STATIC_ASSERT( (IsSplineTypedData<SplinefColor3fData>::value) );
BOOST_STATIC_ASSERT( (boost::mpl::not_< IsSplineTypedData< Imath::V2f > >::value) );
BOOST_STATIC_ASSERT( (boost::mpl::not_< IsSplineTypedData< FloatData > >::value) );

} // namespace TypeTraits

} // namespace IECore

#endif // IE_CORE_TYPETRAITS_H
