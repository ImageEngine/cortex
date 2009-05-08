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

#ifndef IE_COREMAYA_FROMMAYACOMPOUNDNUMERICPLUGCONVERTER_H
#define IE_COREMAYA_FROMMAYACOMPOUNDNUMERICPLUGCONVERTER_H

#include "IECore/SimpleTypedData.h"

#include "IECoreMaya/FromMayaPlugConverter.h"

namespace IECoreMaya
{

template<typename F, typename T>
class FromMayaCompoundNumericPlugConverter : public FromMayaPlugConverter
{

	public :

		FromMayaCompoundNumericPlugConverter( const MPlug &plug );

		IECORE_RUNTIMETYPED_DECLARETEMPLATE( FromMayaCompoundNumericPlugConverter, FromMayaPlugConverter )

	protected :

		virtual IECore::ObjectPtr doConversion( IECore::ConstCompoundObjectPtr operands ) const;

	private :

		static Description<FromMayaCompoundNumericPlugConverter> m_description;

};

typedef FromMayaCompoundNumericPlugConverter<Imath::V2f, IECore::V2iData> FromMayaCompoundNumericPlugConverterV2fV2i;
typedef FromMayaCompoundNumericPlugConverter<Imath::V2f, IECore::V2fData> FromMayaCompoundNumericPlugConverterV2fV2f;
typedef FromMayaCompoundNumericPlugConverter<Imath::V2f, IECore::V2dData> FromMayaCompoundNumericPlugConverterV2fV2d;

typedef FromMayaCompoundNumericPlugConverter<Imath::V2d, IECore::V2iData> FromMayaCompoundNumericPlugConverterV2dV2i;
typedef FromMayaCompoundNumericPlugConverter<Imath::V2d, IECore::V2fData> FromMayaCompoundNumericPlugConverterV2dV2f;
typedef FromMayaCompoundNumericPlugConverter<Imath::V2d, IECore::V2dData> FromMayaCompoundNumericPlugConverterV2dV2d;

typedef FromMayaCompoundNumericPlugConverter<Imath::V3f, IECore::V3iData> FromMayaCompoundNumericPlugConverterV3fV3i;
typedef FromMayaCompoundNumericPlugConverter<Imath::V3f, IECore::V3fData> FromMayaCompoundNumericPlugConverterV3fV3f;
typedef FromMayaCompoundNumericPlugConverter<Imath::V3f, IECore::V3dData> FromMayaCompoundNumericPlugConverterV3fV3d;
typedef FromMayaCompoundNumericPlugConverter<Imath::V3f, IECore::Color3fData> FromMayaCompoundNumericPlugConverterV3fC3f;

typedef FromMayaCompoundNumericPlugConverter<Imath::V3d, IECore::V3iData> FromMayaCompoundNumericPlugConverterV3dV3i;
typedef FromMayaCompoundNumericPlugConverter<Imath::V3d, IECore::V3fData> FromMayaCompoundNumericPlugConverterV3dV3f;
typedef FromMayaCompoundNumericPlugConverter<Imath::V3d, IECore::V3dData> FromMayaCompoundNumericPlugConverterV3dV3d;
typedef FromMayaCompoundNumericPlugConverter<Imath::V3d, IECore::Color3fData> FromMayaCompoundNumericPlugConverterV3dC3f;

} // namespace IECoreMaya

#endif // IE_COREMAYA_FROMMAYACOMPOUNDNUMERICPLUGCONVERTER_H
