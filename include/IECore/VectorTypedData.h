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

#include <string>
#include <vector>

#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathQuat.h"
#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathColor.h"
#include "OpenEXR/half.h"

#include "IECore/TypedData.h"
#include "IECore/GeometricTypedData.h"

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
IECORE_DECLARE_TYPEDDATA( Color3dVectorData, std::vector<Imath::Color3<double> >, double, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( Color4fVectorData, std::vector<Imath::Color4f>, float, SharedDataHolder )
IECORE_DECLARE_TYPEDDATA( Color4dVectorData, std::vector<Imath::Color4<double> >, double, SharedDataHolder )

/// \deprecated This class allows data of the obsolete typeId LongVectorDataTypeId or typename "LongVectorData" to register
/// itself with an IntVectorData constructor to the Object factory. This allows temporary backwards compatibility (since
/// long and int were both 32-bits wide on 32-bit platforms)
class LongVectorDataAlias : private IntVectorData
{
	protected:
		static TypeDescription<IntVectorData> m_typeDescription;
		~LongVectorDataAlias() {};
};

} // namespace IECore

#endif // IECORE_VECTORTYPEDDATA_H
