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

#ifndef IE_COREMAYA_NUMERICTRAITS_H
#define IE_COREMAYA_NUMERICTRAITS_H

#include "boost/static_assert.hpp"

#include "maya/MFnNumericData.h"
#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathColor.h"

namespace IECoreMaya
{

/// This traits class is useful for determining the appropriate maya
/// types to use for the basic numeric types in different contexts.
template<typename T>
struct NumericTraits
{
	/// Returns the maya data type suitable for representing this type of number.
	static MFnNumericData::Type dataType()
	{
		BOOST_STATIC_ASSERT( sizeof(T) == 0 );
		return MFnNumericData::kInvalid;
	}
	
	/// For compound numeric types, returns the maya data type suitable for representing
	/// a single element of the compound.
	static MFnNumericData::Type baseDataType()
	{
		BOOST_STATIC_ASSERT( sizeof(T) == 0 );
		return MFnNumericData::kInvalid;
	}
	
	/// Returns whether or not the maya useAsColor() flag should be set on attributes
	/// representing this kind of data.
	static bool isColor()
	{
		return false;
	}
};

template<>
MFnNumericData::Type NumericTraits<bool>::dataType();

template<>
MFnNumericData::Type NumericTraits<int>::dataType();

template<>
MFnNumericData::Type NumericTraits<float>::dataType();

template<>
MFnNumericData::Type NumericTraits<double>::dataType();

template<>
MFnNumericData::Type NumericTraits<Imath::V2i>::dataType();
template<>
MFnNumericData::Type NumericTraits<Imath::V2i>::baseDataType();

template<>
MFnNumericData::Type NumericTraits<Imath::V3i>::dataType();
template<>
MFnNumericData::Type NumericTraits<Imath::V3i>::baseDataType();

template<>
MFnNumericData::Type NumericTraits<Imath::V2f>::dataType();
template<>
MFnNumericData::Type NumericTraits<Imath::V2f>::baseDataType();

template<>
MFnNumericData::Type NumericTraits<Imath::V3f>::dataType();
template<>
MFnNumericData::Type NumericTraits<Imath::V3f>::baseDataType();

template<>
MFnNumericData::Type NumericTraits<Imath::V2d>::dataType();
template<>
MFnNumericData::Type NumericTraits<Imath::V2d>::baseDataType();

template<>
MFnNumericData::Type NumericTraits<Imath::V3d>::dataType();
template<>
MFnNumericData::Type NumericTraits<Imath::V3d>::baseDataType();

template<>
MFnNumericData::Type NumericTraits<Imath::Color3f>::dataType();
template<>
MFnNumericData::Type NumericTraits<Imath::Color3f>::baseDataType();
template<>
bool NumericTraits<Imath::Color3f>::isColor();

} // namespace IECoreMaya

#endif // IE_COREMAYA_NUMERICTRAITS_H
