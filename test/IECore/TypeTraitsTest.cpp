//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009-2013, Image Engine Design Inc. All rights reserved.
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

#include "IECore/TypeTraits.h"

using namespace IECore;
using namespace IECore::TypeTraits;

// ValueType
static_assert( (sizeof( ValueType< M33fData >::type ) == sizeof( M33fData::ValueType ) ) );
static_assert( (sizeof( ValueType< FloatVectorData >::type ) == sizeof( FloatVectorData::ValueType ) ) );

/// IsMatrix
static_assert( (IsMatrix< const Imath::M33f >::value) );
static_assert( (IsMatrix< Imath::M44d >::value) );
static_assert( (IsMatrix< ValueType< M33fData >::type >::value ) );
static_assert( (boost::mpl::not_< IsMatrix< Imath::V2i > >::value) );

/// IsVec3
static_assert( ( boost::mpl::not_< IsVec3<Imath::V2d> >::value) );

/// IsVec2
static_assert( ( boost::mpl::not_< IsVec2<Imath::V3i> >::value) );

/// IsVec
static_assert( (IsVec<Imath::V3i>::value) );
static_assert( (IsVec<Imath::V2d>::value) );
static_assert( ( boost::mpl::not_< IsVec<int> >::value) );

/// IsColor3
static_assert( ( IsColor3<Imath::Color3f>::value ) );
static_assert( ( IsColor3<const Imath::Color3f>::value ) );
static_assert( ( boost::mpl::not_< IsColor3<Imath::V3f> >::value) );

/// IsColor4
static_assert( ( IsColor4<Imath::Color4f>::value ) );
static_assert( ( boost::mpl::not_< IsColor4<Imath::V3f> >::value) );
static_assert( ( boost::mpl::not_< IsColor4<Imath::Color3f> >::value) );

/// IsColor
static_assert( (IsColor<Imath::Color3f>::value) );
static_assert( (IsColor<Imath::Color4f>::value) );
static_assert( ( boost::mpl::not_< IsColor<int> >::value) );

/// IsQuat
static_assert( (IsQuat<const Imath::Quatf>::value) );
static_assert( (IsQuat<Imath::Quatd>::value) );

/// IsBox
static_assert( (IsBox<const Imath::Box3d>::value) );

/// IsMatrixTypedData
static_assert( (IsMatrixTypedData<M33fData>::value) );
static_assert( ( boost::mpl::not_< IsMatrixTypedData<V3fData> >::value) );
static_assert( ( boost::mpl::not_< IsMatrixTypedData<char> >::value) );

/// IsVec2TypedData
static_assert( (IsVec2TypedData<V2fData>::value) );
static_assert( ( boost::mpl::not_< IsVec2TypedData<V3iData> >::value) );

/// IsVec2VectorTypedData
static_assert( (IsVec2VectorTypedData<V2iVectorData>::value) );
static_assert( ( boost::mpl::not_< IsVec2VectorTypedData<V2fData> >::value) );

/// IsVec3TypedData
static_assert( (IsVec3TypedData<V3fData>::value) );
static_assert( ( boost::mpl::not_< IsVec3TypedData<V2iData> >::value) );

/// IsVec3VectorTypedData
static_assert( (IsVec3VectorTypedData<V3iVectorData>::value) );
static_assert( ( boost::mpl::not_< IsVec3VectorTypedData<V3fData> >::value) );

/// IsVecTypedData
static_assert( (IsVecTypedData<V2iData>::value) );
static_assert( (IsVecTypedData<V3fData>::value) );
static_assert( (IsVecTypedData<const V3dData>::value) );
static_assert( ( boost::mpl::not_< IsVecTypedData<M33fData> >::value) );

/// IsVecVectorTypedData
static_assert( (IsVecVectorTypedData<V2iVectorData>::value) );
static_assert( ( boost::mpl::not_< IsVecVectorTypedData<M33fData> >::value) );

/// IsNumericVectorTypedData
static_assert( (IsNumericVectorTypedData<FloatVectorData>::value) );
static_assert( (IsNumericVectorTypedData<UCharVectorData>::value) );
static_assert( (IsNumericVectorTypedData<HalfVectorData>::value) );
static_assert( (IsNumericVectorTypedData<const Int64VectorData>::value) );
static_assert( (IsNumericVectorTypedData<UInt64VectorData>::value) );
static_assert( ( boost::mpl::not_< IsNumericVectorTypedData<StringVectorData> >::value) );

/// IsFloatVectorTypedData
static_assert( (IsFloatVectorTypedData<FloatVectorData>::value) );
static_assert( (IsFloatVectorTypedData<HalfVectorData>::value) );
static_assert( ( boost::mpl::not_< IsFloatVectorTypedData<IntVectorData> >::value) );

/// IsNumericSimpleTypedData
static_assert( (IsNumericSimpleTypedData<FloatData>::value) );
static_assert( (IsNumericSimpleTypedData<ShortData>::value) );
static_assert( (IsNumericSimpleTypedData<HalfData>::value) );
static_assert( (IsNumericSimpleTypedData<const Int64Data>::value) );
static_assert( (IsNumericSimpleTypedData<UInt64Data>::value) );
static_assert( ( boost::mpl::not_< IsNumericSimpleTypedData<char> >::value) );

/// IsInterpolable
static_assert( (IsInterpolable<int>::value) );
static_assert( (IsInterpolable<Imath::Color3f>::value) );
static_assert( (IsInterpolable<Imath::V2d>::value) );
static_assert( (IsInterpolable<Imath::Box3f>::value) );
static_assert( (IsInterpolable<std::vector<Imath::Box3f> >::value) );
static_assert( (IsInterpolable<const TypedData< std::vector<Imath::V2i> > >::value) );
static_assert( ( boost::mpl::not_< IsInterpolable<std::string> >::value) );

/// IsStrictlyInterpolable
static_assert( (IsStrictlyInterpolable<float>::value) );
static_assert( (IsStrictlyInterpolable<Imath::Color3f>::value) );
static_assert( (IsStrictlyInterpolable<Imath::V2d>::value) );
static_assert( (IsStrictlyInterpolable<Imath::Box3f>::value) );
static_assert( (IsStrictlyInterpolable<std::vector<Imath::Box3f> >::value) );
static_assert( ( boost::mpl::not_< IsStrictlyInterpolable <const TypedData< std::vector<Imath::V2i> > > >::value) );
static_assert( ( boost::mpl::not_< IsStrictlyInterpolable<std::string> >::value) );


/// IsInterpolableVectorTypedData
static_assert( (IsInterpolableVectorTypedData<HalfVectorData>::value) );
static_assert( (IsInterpolableVectorTypedData<Color4fVectorData>::value) );
static_assert( ( boost::mpl::not_< IsInterpolableVectorTypedData<FloatData> >::value) );
static_assert( ( boost::mpl::not_< IsInterpolableVectorTypedData<StringData> >::value) );

/// IsInterpolableSimpleTypedData
static_assert( (IsInterpolableSimpleTypedData<IntData>::value) );
static_assert( (IsInterpolableSimpleTypedData<V2iData>::value) );
static_assert( ( boost::mpl::not_< IsInterpolableSimpleTypedData<DateTimeData> >::value) );
static_assert( ( boost::mpl::not_< IsInterpolableSimpleTypedData<IntVectorData> >::value) );

/// IsGeometricTypedData
static_assert( ( IsGeometricTypedData<V2iData>::value) );
static_assert( ( IsGeometricTypedData<V2fVectorData>::value) );
static_assert( ( boost::mpl::not_< IsGeometricTypedData<IntData> >::value) );
static_assert( ( boost::mpl::not_< IsGeometricTypedData<DateTimeData> >::value) );
static_assert( ( boost::mpl::not_< IsGeometricTypedData<FloatVectorData> >::value) );

/// IsSpline
static_assert( (IsSpline< Spline<float, float> >::value) );
static_assert( (boost::mpl::not_< IsSpline< Imath::V2f > >::value) );

/// IsSplineTypedData
static_assert( (IsSplineTypedData<SplineffData>::value) );
static_assert( (IsSplineTypedData<SplineddData>::value) );
static_assert( (IsSplineTypedData<SplinefColor3fData>::value) );
static_assert( (boost::mpl::not_< IsSplineTypedData< Imath::V2f > >::value) );
static_assert( (boost::mpl::not_< IsSplineTypedData< FloatData > >::value) );

/// IsStringVectorTypedData
static_assert( ( boost::mpl::not_< IsStringVectorTypedData<IntVectorData> >::value) );
static_assert( ( IsStringVectorTypedData<StringVectorData>::value ) );
static_assert( ( IsStringVectorTypedData<InternedStringVectorData>::value ) );
