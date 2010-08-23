//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_VECTORTYPEDDATA_H
#define IE_CORE_VECTORTYPEDDATA_H

#include <string>
#include <vector>

#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathQuat.h"
#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathColor.h"
#include "OpenEXR/half.h"

#include "IECore/TypedData.h"

namespace IECore
{

// vectors for basic types
typedef TypedData< std::vector< bool > > BoolVectorData;
typedef TypedData< std::vector< half > > HalfVectorData;
typedef TypedData< std::vector< float > > FloatVectorData;
typedef TypedData< std::vector< double > > DoubleVectorData;
typedef TypedData< std::vector< int > > IntVectorData;
typedef TypedData< std::vector< unsigned int > > UIntVectorData;
typedef TypedData< std::vector< char > > CharVectorData;
typedef TypedData< std::vector< unsigned char > > UCharVectorData;
typedef TypedData< std::vector< short > > ShortVectorData;
typedef TypedData< std::vector< unsigned short > > UShortVectorData;
typedef TypedData< std::vector< int64_t > > Int64VectorData;
typedef TypedData< std::vector< uint64_t > > UInt64VectorData;
typedef TypedData< std::vector< std::string > > StringVectorData;

// vectors for Imath types
typedef TypedData< std::vector< Imath::V2f > > V2fVectorData;
typedef TypedData< std::vector< Imath::V2d > > V2dVectorData;
typedef TypedData< std::vector< Imath::V2i > > V2iVectorData;
typedef TypedData< std::vector< Imath::V3f > > V3fVectorData;
typedef TypedData< std::vector< Imath::V3d > > V3dVectorData;
typedef TypedData< std::vector< Imath::V3i > > V3iVectorData;
typedef TypedData< std::vector< Imath::Box2i > > Box2iVectorData;
typedef TypedData< std::vector< Imath::Box2f > > Box2fVectorData;
typedef TypedData< std::vector< Imath::Box2d > > Box2dVectorData;
typedef TypedData< std::vector< Imath::Box3i > > Box3iVectorData;
typedef TypedData< std::vector< Imath::Box3f > > Box3fVectorData;
typedef TypedData< std::vector< Imath::Box3d > > Box3dVectorData;
typedef TypedData< std::vector< Imath::M33f > > M33fVectorData;
typedef TypedData< std::vector< Imath::M33d > > M33dVectorData;
typedef TypedData< std::vector< Imath::M44f > > M44fVectorData;
typedef TypedData< std::vector< Imath::M44d > > M44dVectorData;
typedef TypedData< std::vector< Imath::Quatf > > QuatfVectorData;
typedef TypedData< std::vector< Imath::Quatd > > QuatdVectorData;
typedef TypedData< std::vector< Imath::Color3f > > Color3fVectorData;
typedef TypedData< std::vector< Imath::Color4f > > Color4fVectorData;
typedef TypedData< std::vector< Imath::Color3<double> > > Color3dVectorData;
typedef TypedData< std::vector< Imath::Color4<double> > > Color4dVectorData;

// pointers to vectors of basic types
IE_CORE_DECLAREPTR( BoolVectorData );
IE_CORE_DECLAREPTR( HalfVectorData );
IE_CORE_DECLAREPTR( FloatVectorData );
IE_CORE_DECLAREPTR( DoubleVectorData );
IE_CORE_DECLAREPTR( IntVectorData );
IE_CORE_DECLAREPTR( UIntVectorData );
IE_CORE_DECLAREPTR( CharVectorData );
IE_CORE_DECLAREPTR( UCharVectorData );
IE_CORE_DECLAREPTR( ShortVectorData );
IE_CORE_DECLAREPTR( UShortVectorData );
IE_CORE_DECLAREPTR( StringVectorData );
IE_CORE_DECLAREPTR( Int64VectorData );
IE_CORE_DECLAREPTR( UInt64VectorData );
// pointers to vectors of Imath types
IE_CORE_DECLAREPTR( V2fVectorData );
IE_CORE_DECLAREPTR( V2dVectorData );
IE_CORE_DECLAREPTR( V2iVectorData );
IE_CORE_DECLAREPTR( V3fVectorData );
IE_CORE_DECLAREPTR( V3dVectorData );
IE_CORE_DECLAREPTR( V3iVectorData );
IE_CORE_DECLAREPTR( Box2iVectorData );
IE_CORE_DECLAREPTR( Box2fVectorData );
IE_CORE_DECLAREPTR( Box2dVectorData );
IE_CORE_DECLAREPTR( Box3iVectorData );
IE_CORE_DECLAREPTR( Box3fVectorData );
IE_CORE_DECLAREPTR( Box3dVectorData );
IE_CORE_DECLAREPTR( M33fVectorData );
IE_CORE_DECLAREPTR( M33dVectorData );
IE_CORE_DECLAREPTR( M44fVectorData );
IE_CORE_DECLAREPTR( M44dVectorData );
IE_CORE_DECLAREPTR( QuatfVectorData );
IE_CORE_DECLAREPTR( QuatdVectorData );
IE_CORE_DECLAREPTR( Color3fVectorData );
IE_CORE_DECLAREPTR( Color4fVectorData );
IE_CORE_DECLAREPTR( Color3dVectorData );
IE_CORE_DECLAREPTR( Color4dVectorData );

#include "IECore/VectorTypedDataTraits.inl"

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

#endif // IE_CORE_VECTORTYPEDDATA_H
