//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_SIMPLETYPEDDATA_H
#define IE_CORE_SIMPLETYPEDDATA_H

#include "IECore/TypedData.h"

#include "OpenEXR/ImathVec.h"
#include "OpenEXR/ImathBox.h"
#include "OpenEXR/ImathMatrix.h"
#include "OpenEXR/ImathQuat.h"
#include "OpenEXR/ImathColor.h"
#include "OpenEXR/half.h"

#include <string>

namespace IECore 
{

// simple typeddata types
typedef TypedData < bool > BoolData;
typedef TypedData < float > FloatData;
typedef TypedData < double > DoubleData;
typedef TypedData < int > IntData;
typedef TypedData < unsigned int > UIntData;
typedef TypedData < char > CharData;
typedef TypedData < unsigned char > UCharData;
typedef TypedData < short > ShortData;
typedef TypedData < unsigned short > UShortData;
typedef TypedData < std::string > StringData;
typedef TypedData<half> HalfData;
typedef TypedData<Imath::V2i> V2iData;
typedef TypedData<Imath::V3i> V3iData;
typedef TypedData<Imath::V2f> V2fData;
typedef TypedData<Imath::V3f> V3fData;
typedef TypedData<Imath::V2d> V2dData;
typedef TypedData<Imath::V3d> V3dData;
typedef TypedData<Imath::Color3f> Color3fData;
typedef TypedData<Imath::Color4f> Color4fData;
typedef TypedData<Imath::Color3<double> > Color3dData;
typedef TypedData<Imath::Color4<double> > Color4dData;
typedef TypedData<Imath::Box2i> Box2iData;
typedef TypedData<Imath::Box3i> Box3iData;
typedef TypedData<Imath::Box2f> Box2fData;
typedef TypedData<Imath::Box3f> Box3fData;
typedef TypedData<Imath::Box2d> Box2dData;
typedef TypedData<Imath::Box3d> Box3dData;
typedef TypedData<Imath::M33f> M33fData;
typedef TypedData<Imath::M33d> M33dData;
typedef TypedData<Imath::M44f> M44fData;
typedef TypedData<Imath::M44d> M44dData;
typedef TypedData<Imath::Quatf> QuatfData;
typedef TypedData<Imath::Quatd> QuatdData;

// pointer declarations
IE_CORE_DECLAREPTR( BoolData );
IE_CORE_DECLAREPTR( FloatData );
IE_CORE_DECLAREPTR( DoubleData );
IE_CORE_DECLAREPTR( IntData );
IE_CORE_DECLAREPTR( UIntData );
IE_CORE_DECLAREPTR( CharData );
IE_CORE_DECLAREPTR( UCharData );
IE_CORE_DECLAREPTR( StringData );
IE_CORE_DECLAREPTR( HalfData );
IE_CORE_DECLAREPTR( ShortData );
IE_CORE_DECLAREPTR( UShortData );
IE_CORE_DECLAREPTR( V2iData );
IE_CORE_DECLAREPTR( V3iData );
IE_CORE_DECLAREPTR( V2fData );
IE_CORE_DECLAREPTR( V3fData );
IE_CORE_DECLAREPTR( V2dData );
IE_CORE_DECLAREPTR( V3dData );
IE_CORE_DECLAREPTR( Color3fData );
IE_CORE_DECLAREPTR( Color3dData );
IE_CORE_DECLAREPTR( Color4fData );
IE_CORE_DECLAREPTR( Color4dData );
IE_CORE_DECLAREPTR( Box2iData );
IE_CORE_DECLAREPTR( Box3iData );
IE_CORE_DECLAREPTR( Box2fData );
IE_CORE_DECLAREPTR( Box3fData );
IE_CORE_DECLAREPTR( Box2dData );
IE_CORE_DECLAREPTR( Box3dData );
IE_CORE_DECLAREPTR( M33fData );
IE_CORE_DECLAREPTR( M33dData );
IE_CORE_DECLAREPTR( M44fData );
IE_CORE_DECLAREPTR( M44dData );
IE_CORE_DECLAREPTR( QuatfData );
IE_CORE_DECLAREPTR( QuatdData );

#include "IECore/SimpleTypedDataTraits.inl"

/// \deprecated This class allows data of the obsolete typeId LongDataTypeId or typename "LongData" to register
/// itself with an IntData constructor to the Object factory. This allows temporary backwards compatibility (since
/// long and int were both 32-bits wide on 32-bit platforms)
class LongDataAlias : private IntData
{
	private:	
		static TypeDescription<IntData> m_typeDescription;	
		~LongDataAlias() {};	
};

} // namespace IECore

#endif // IE_CORE_SIMPLETYPEDDATA_H
