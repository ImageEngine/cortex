//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_VECTORTYPEDDATA_H
#define IECORE_VECTORTYPEDDATA_H

#include "IECore/Export.h"
#include "IECore/GeometricTypedData.h"
#include "IECore/TypedData.h"

IECORE_PUSH_DEFAULT_VISIBILITY
#include "Imath/ImathBox.h"
#include "Imath/ImathColor.h"
#include "Imath/ImathQuat.h"
#include "Imath/ImathVec.h"
#include "Imath/half.h"
IECORE_POP_DEFAULT_VISIBILITY

#include <string>
#include <vector>

namespace IECore
{

// vectors of basic types

IECORE_DECLARE_TYPEDDATA( BoolVectorData, std::vector<bool>, void, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( HalfVectorData, std::vector<half>, half, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( FloatVectorData, std::vector<float>, float, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( DoubleVectorData, std::vector<double>, double, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( IntVectorData, std::vector<int>, int, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( UIntVectorData, std::vector<unsigned int>, unsigned int, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( CharVectorData, std::vector<char>, char, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( UCharVectorData, std::vector<unsigned char>, unsigned char, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( ShortVectorData, std::vector<short>, short, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( UShortVectorData, std::vector<unsigned short>, unsigned short, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( Int64VectorData, std::vector<int64_t>, int64_t, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( UInt64VectorData, std::vector<uint64_t>, uint64_t, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( StringVectorData, std::vector<std::string>, std::string, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( InternedStringVectorData, std::vector<InternedString>, InternedString, SharedDataHolder )

// vectors of Imath types

IECORE_DECLARE_GEOMETRICTYPEDDATA( V2fVectorData, std::vector<Imath::V2f>, float, SharedDataHolder )
IECORE_DECLARE_GEOMETRICTYPEDDATA( V2dVectorData, std::vector<Imath::V2d>, double, SharedDataHolder )
IECORE_DECLARE_GEOMETRICTYPEDDATA( V2iVectorData, std::vector<Imath::V2i>, int, SharedDataHolder )
IECORE_DECLARE_GEOMETRICTYPEDDATA( V3fVectorData, std::vector<Imath::V3f>, float, SharedDataHolder )
IECORE_DECLARE_GEOMETRICTYPEDDATA( V3dVectorData, std::vector<Imath::V3d>, double, SharedDataHolder )
IECORE_DECLARE_GEOMETRICTYPEDDATA( V3iVectorData, std::vector<Imath::V3i>, int, SharedDataHolder )

IECORE_DECLARE_TYPEDDATA( Box2iVectorData, std::vector<Imath::Box2i>, int, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( Box2fVectorData, std::vector<Imath::Box2f>, float, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( Box2dVectorData, std::vector<Imath::Box2d>, double, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( Box3iVectorData, std::vector<Imath::Box3i>, int, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( Box3fVectorData, std::vector<Imath::Box3f>, float, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( Box3dVectorData, std::vector<Imath::Box3d>, double, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( M33fVectorData, std::vector<Imath::M33f>, float, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( M33dVectorData, std::vector<Imath::M33d>, double, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( M44fVectorData, std::vector<Imath::M44f>, float, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( M44dVectorData, std::vector<Imath::M44d>, double, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( QuatfVectorData, std::vector<Imath::Quatf>, float, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( QuatdVectorData, std::vector<Imath::Quatd>, double, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( Color3fVectorData, std::vector<Imath::Color3f>, float, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( Color4fVectorData, std::vector<Imath::Color4f>, float, SharedDataHolder )

#if !defined(IECore_EXPORTS) && !defined(_MSC_VER)

extern template class TypedData<std::vector<bool>>;
extern template class TypedData<std::vector<half>>;
extern template class TypedData<std::vector<float>>;
extern template class TypedData<std::vector<double>>;
extern template class TypedData<std::vector<int>>;
extern template class TypedData<std::vector<unsigned int>>;
extern template class TypedData<std::vector<char>>;
extern template class TypedData<std::vector<unsigned char>>;
extern template class TypedData<std::vector<short>>;
extern template class TypedData<std::vector<unsigned short>>;
extern template class TypedData<std::vector<int64_t>>;
extern template class TypedData<std::vector<uint64_t>>;
extern template class TypedData<std::vector<std::string>>;
extern template class TypedData<std::vector<InternedString>>;
extern template class TypedData<std::vector<Imath::V2f>>;
extern template class TypedData<std::vector<Imath::V2d>>;
extern template class TypedData<std::vector<Imath::V2i>>;
extern template class TypedData<std::vector<Imath::V3f>>;
extern template class TypedData<std::vector<Imath::V3d>>;
extern template class TypedData<std::vector<Imath::V3i>>;
extern template class TypedData<std::vector<Imath::Box2i>>;
extern template class TypedData<std::vector<Imath::Box2f>>;
extern template class TypedData<std::vector<Imath::Box2d>>;
extern template class TypedData<std::vector<Imath::Box3i>>;
extern template class TypedData<std::vector<Imath::Box3f>>;
extern template class TypedData<std::vector<Imath::Box3d>>;
extern template class TypedData<std::vector<Imath::M33f>>;
extern template class TypedData<std::vector<Imath::M33d>>;
extern template class TypedData<std::vector<Imath::M44f>>;
extern template class TypedData<std::vector<Imath::M44d>>;
extern template class TypedData<std::vector<Imath::Quatf>>;
extern template class TypedData<std::vector<Imath::Quatd>>;
extern template class TypedData<std::vector<Imath::Color3f>>;
extern template class TypedData<std::vector<Imath::Color4f>>;

#endif

} // namespace IECore

#endif // IECORE_VECTORTYPEDDATA_H
