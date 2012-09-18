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

#include "IECoreMaya/NumericTraits.h"

using namespace IECoreMaya;

template<>
MFnNumericData::Type NumericTraits<bool>::dataType() { return MFnNumericData::kBoolean; };

template<>
MFnNumericData::Type NumericTraits<int>::dataType() { return MFnNumericData::kInt; };

template<>
MFnNumericData::Type NumericTraits<float>::dataType() { return MFnNumericData::kFloat; };

template<>
MFnNumericData::Type NumericTraits<double>::dataType() { return MFnNumericData::kDouble; };

template<>
MFnNumericData::Type NumericTraits<Imath::V2i>::dataType() { return MFnNumericData::k2Int; };
template<>
MFnNumericData::Type NumericTraits<Imath::V2i>::baseDataType() { return MFnNumericData::kInt; };

template<>
MFnNumericData::Type NumericTraits<Imath::V3i>::dataType() { return MFnNumericData::k3Int; };
template<>
MFnNumericData::Type NumericTraits<Imath::V3i>::baseDataType() { return MFnNumericData::kInt; };

template<>
MFnNumericData::Type NumericTraits<Imath::V2f>::dataType() { return MFnNumericData::k2Float; };
template<>
MFnNumericData::Type NumericTraits<Imath::V2f>::baseDataType() { return MFnNumericData::kFloat; };

template<>
MFnNumericData::Type NumericTraits<Imath::V3f>::dataType() { return MFnNumericData::k3Float; };
template<>
MFnNumericData::Type NumericTraits<Imath::V3f>::baseDataType() { return MFnNumericData::kFloat; };

template<>
MFnNumericData::Type NumericTraits<Imath::V2d>::dataType() { return MFnNumericData::k2Double; };
template<>
MFnNumericData::Type NumericTraits<Imath::V2d>::baseDataType() { return MFnNumericData::kDouble; };

template<>
MFnNumericData::Type NumericTraits<Imath::V3d>::dataType() { return MFnNumericData::k3Double; };
template<>
MFnNumericData::Type NumericTraits<Imath::V3d>::baseDataType() { return MFnNumericData::kDouble; };

template<>
MFnNumericData::Type NumericTraits<Imath::Color3f>::dataType() { return MFnNumericData::k3Float; };
template<>
MFnNumericData::Type NumericTraits<Imath::Color3f>::baseDataType() { return MFnNumericData::kFloat; };
template<>
bool NumericTraits<Imath::Color3f>::isColor() { return true; };
