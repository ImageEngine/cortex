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

#ifndef IE_COREMAYA_FROMMAYAARRAYDATACONVERTER_H
#define IE_COREMAYA_FROMMAYAARRAYDATACONVERTER_H

#include "maya/MIntArray.h"
#include "maya/MDoubleArray.h"
#include "maya/MStringArray.h"
#include "maya/MVectorArray.h"

#include "IECore/VectorTypedData.h"

#include "IECoreMaya/FromMayaObjectConverter.h"

namespace IECoreMaya
{

/// This template class can convert from various maya array data types
/// into various IECore TypedVectorData types.
template<typename F, typename T>
class FromMayaArrayDataConverter : public FromMayaObjectConverter
{

	public :

		FromMayaArrayDataConverter( const MObject &object );

		IECORE_RUNTIMETYPED_DECLARETEMPLATE( FromMayaArrayDataConverter, FromMayaObjectConverter )

	protected :

		virtual IECore::ObjectPtr doConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const;

		static FromMayaObjectConverterDescription<FromMayaArrayDataConverter> m_description;

};

typedef FromMayaArrayDataConverter<MIntArray, IECore::IntVectorData> FromMayaArrayDataConverterii;
typedef FromMayaArrayDataConverter<MDoubleArray, IECore::DoubleVectorData> FromMayaArrayDataConverterdd;
typedef FromMayaArrayDataConverter<MDoubleArray, IECore::FloatVectorData> FromMayaArrayDataConverterdf;
typedef FromMayaArrayDataConverter<MStringArray, IECore::StringVectorData> FromMayaArrayDataConverterss;
typedef FromMayaArrayDataConverter<MVectorArray, IECore::V3fVectorData> FromMayaArrayDataConverterVV3f;
typedef FromMayaArrayDataConverter<MVectorArray, IECore::V3dVectorData> FromMayaArrayDataConverterVV3d;
typedef FromMayaArrayDataConverter<MVectorArray, IECore::Color3fVectorData> FromMayaArrayDataConverterVC3f;

} // namespace IECoreMaya

#endif // IE_COREMAYA_FROMMAYAARRAYDATACONVERTER_H
