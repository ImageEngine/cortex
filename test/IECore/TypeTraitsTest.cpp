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

#include "IECore/TypeTraits.h"

using namespace IECore;
using namespace IECore::TypeTraits;

// ValueType
BOOST_STATIC_ASSERT( (sizeof( ValueType< M33fData >::type ) == sizeof( M33fData::ValueType ) ) );
BOOST_STATIC_ASSERT( (sizeof( ValueType< FloatVectorData >::type ) == sizeof( FloatVectorData::ValueType ) ) );

/// IsMatrix
BOOST_STATIC_ASSERT( (IsMatrix< const Imath::M33f >::value) );
BOOST_STATIC_ASSERT( (IsMatrix< Imath::M44d >::value) );
BOOST_STATIC_ASSERT( (IsMatrix< ValueType< M33fData >::type >::value ) );
BOOST_STATIC_ASSERT( (boost::mpl::not_< IsMatrix< Imath::V2i > >::value) );

/// IsVec3
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVec3<Imath::V2d> >::value) );

/// IsVec2
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVec2<Imath::V3i> >::value) );

/// IsVec
BOOST_STATIC_ASSERT( (IsVec<Imath::V3i>::value) );
BOOST_STATIC_ASSERT( (IsVec<Imath::V2d>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVec<int> >::value) );

/// IsColor3
BOOST_STATIC_ASSERT( ( IsColor3<Imath::Color3f>::value ) );
BOOST_STATIC_ASSERT( ( IsColor3<const Imath::Color3f>::value ) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsColor3<Imath::V3f> >::value) );

/// IsColor4
BOOST_STATIC_ASSERT( ( IsColor4<Imath::Color4f>::value ) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsColor4<Imath::V3f> >::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsColor4<Imath::Color3f> >::value) );

/// IsColor
BOOST_STATIC_ASSERT( (IsColor<Imath::Color3f>::value) );
BOOST_STATIC_ASSERT( (IsColor<Imath::Color4f>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsColor<int> >::value) );

/// IsQuat
BOOST_STATIC_ASSERT( (IsQuat<const Imath::Quatf>::value) );
BOOST_STATIC_ASSERT( (IsQuat<Imath::Quatd>::value) );

/// IsBox
BOOST_STATIC_ASSERT( (IsBox<const Imath::Box3d>::value) );

/// IsMatrixTypedData
BOOST_STATIC_ASSERT( (IsMatrixTypedData<M33fData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsMatrixTypedData<V3fData> >::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsMatrixTypedData<char> >::value) );

/// IsVec2TypedData
BOOST_STATIC_ASSERT( (IsVec2TypedData<V2fData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVec2TypedData<V3iData> >::value) );

/// IsVec2VectorTypedData
BOOST_STATIC_ASSERT( (IsVec2VectorTypedData<V2iVectorData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVec2VectorTypedData<V2fData> >::value) );

/// IsVec3TypedData
BOOST_STATIC_ASSERT( (IsVec3TypedData<V3fData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVec3TypedData<V2iData> >::value) );

/// IsVec3VectorTypedData
BOOST_STATIC_ASSERT( (IsVec3VectorTypedData<V3iVectorData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVec3VectorTypedData<V3fData> >::value) );

/// IsVecTypedData
BOOST_STATIC_ASSERT( (IsVecTypedData<V2iData>::value) );
BOOST_STATIC_ASSERT( (IsVecTypedData<V3fData>::value) );
BOOST_STATIC_ASSERT( (IsVecTypedData<const V3dData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVecTypedData<M33fData> >::value) );

/// IsVecVectorTypedData
BOOST_STATIC_ASSERT( (IsVecVectorTypedData<V2iVectorData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsVecVectorTypedData<M33fData> >::value) );

/// IsNumericVectorTypedData
BOOST_STATIC_ASSERT( (IsNumericVectorTypedData<FloatVectorData>::value) );
BOOST_STATIC_ASSERT( (IsNumericVectorTypedData<UCharVectorData>::value) );
BOOST_STATIC_ASSERT( (IsNumericVectorTypedData<HalfVectorData>::value) );
BOOST_STATIC_ASSERT( (IsNumericVectorTypedData<const Int64VectorData>::value) );
BOOST_STATIC_ASSERT( (IsNumericVectorTypedData<UInt64VectorData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsNumericVectorTypedData<StringVectorData> >::value) );

/// IsFloatVectorTypedData
BOOST_STATIC_ASSERT( (IsFloatVectorTypedData<FloatVectorData>::value) );
BOOST_STATIC_ASSERT( (IsFloatVectorTypedData<HalfVectorData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsFloatVectorTypedData<IntVectorData> >::value) );

/// IsNumericSimpleTypedData
BOOST_STATIC_ASSERT( (IsNumericSimpleTypedData<FloatData>::value) );
BOOST_STATIC_ASSERT( (IsNumericSimpleTypedData<ShortData>::value) );
BOOST_STATIC_ASSERT( (IsNumericSimpleTypedData<HalfData>::value) );
BOOST_STATIC_ASSERT( (IsNumericSimpleTypedData<const Int64Data>::value) );
BOOST_STATIC_ASSERT( (IsNumericSimpleTypedData<UInt64Data>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsNumericSimpleTypedData<char> >::value) );

/// IsInterpolable
BOOST_STATIC_ASSERT( (IsInterpolable<int>::value) );	
BOOST_STATIC_ASSERT( (IsInterpolable<Imath::Color3f>::value) );	
BOOST_STATIC_ASSERT( (IsInterpolable<Imath::V2d>::value) );	
BOOST_STATIC_ASSERT( (IsInterpolable<Imath::Box3f>::value) );	
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsInterpolable<std::string> >::value) );

/// IsInterpolableVectorTypedData
BOOST_STATIC_ASSERT( (IsInterpolableVectorTypedData<HalfVectorData>::value) );	
BOOST_STATIC_ASSERT( (IsInterpolableVectorTypedData<Color4fVectorData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsInterpolableVectorTypedData<FloatData> >::value) );	
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsInterpolableVectorTypedData<StringData> >::value) );

/// IsInterpolableSimpleTypedData
BOOST_STATIC_ASSERT( (IsInterpolableSimpleTypedData<IntData>::value) );	
BOOST_STATIC_ASSERT( (IsInterpolableSimpleTypedData<V2iData>::value) );
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsInterpolableSimpleTypedData<DateTimeData> >::value) );	
BOOST_STATIC_ASSERT( ( boost::mpl::not_< IsInterpolableSimpleTypedData<IntVectorData> >::value) );	

/// IsSpline
BOOST_STATIC_ASSERT( (IsSpline< Spline<float, float> >::value) );
BOOST_STATIC_ASSERT( (boost::mpl::not_< IsSpline< Imath::V2f > >::value) );

/// IsSplineTypedData
BOOST_STATIC_ASSERT( (IsSplineTypedData<SplineffData>::value) );
BOOST_STATIC_ASSERT( (IsSplineTypedData<SplineddData>::value) );
BOOST_STATIC_ASSERT( (IsSplineTypedData<SplinefColor3fData>::value) );
BOOST_STATIC_ASSERT( (boost::mpl::not_< IsSplineTypedData< Imath::V2f > >::value) );
BOOST_STATIC_ASSERT( (boost::mpl::not_< IsSplineTypedData< FloatData > >::value) );
