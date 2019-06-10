//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2012, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_TOHOUDININUMERICATTRIBCONVERTER_H
#define IECOREHOUDINI_TOHOUDININUMERICATTRIBCONVERTER_H

#include "IECoreHoudini/Export.h"
#include "IECoreHoudini/ToHoudiniAttribConverter.h"
#include "IECoreHoudini/TypeIds.h"

#include "IECore/VectorTypedData.h"

namespace IECoreHoudini
{

/// This template class can convert from various numeric IECore VectorTypeData types
/// to a Houdini GA_Attribute on the provided GU_Detail.
template<typename T>
class IECOREHOUDINI_API ToHoudiniNumericVectorAttribConverter : public ToHoudiniAttribConverter
{

	public :

		IECORE_RUNTIMETYPED_DECLARETEMPLATE( ToHoudiniNumericVectorAttribConverter, ToHoudiniAttribConverter );

		ToHoudiniNumericVectorAttribConverter( const IECore::Data *data );

		virtual ~ToHoudiniNumericVectorAttribConverter();

	protected :

		virtual GA_RWAttributeRef doConversion( const IECore::Data *data, std::string name, GU_Detail *geo ) const;
		virtual GA_RWAttributeRef doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, const GA_Range &range ) const;

	private :

		static ToHoudiniAttribConverter::Description<ToHoudiniNumericVectorAttribConverter> m_description;

};

/// This template class can convert from various numeric IECore SimpleTypedData types
/// to a Houdini GA_Attribute on the provided GU_Detail.
template<typename T>
class IECOREHOUDINI_API ToHoudiniNumericDetailAttribConverter : public ToHoudiniAttribConverter
{

	public :

		IECORE_RUNTIMETYPED_DECLARETEMPLATE( ToHoudiniNumericDetailAttribConverter, ToHoudiniAttribConverter );

		ToHoudiniNumericDetailAttribConverter( const IECore::Data *data );

		virtual ~ToHoudiniNumericDetailAttribConverter();

	protected :

		virtual GA_RWAttributeRef doConversion( const IECore::Data *data, std::string name, GU_Detail *geo ) const;
		virtual GA_RWAttributeRef doConversion( const IECore::Data *data, std::string name, GU_Detail *geo, const GA_Range &range ) const;

	private :

		static ToHoudiniAttribConverter::Description<ToHoudiniNumericDetailAttribConverter> m_description;

};

typedef ToHoudiniNumericVectorAttribConverter<IECore::FloatVectorData> ToHoudiniFloatVectorAttribConverter;
typedef ToHoudiniNumericVectorAttribConverter<IECore::V2fVectorData> ToHoudiniV2fVectorAttribConverter;
typedef ToHoudiniNumericVectorAttribConverter<IECore::V3fVectorData> ToHoudiniV3fVectorAttribConverter;
typedef ToHoudiniNumericVectorAttribConverter<IECore::Color3fVectorData> ToHoudiniColor3fVectorAttribConverter;
typedef ToHoudiniNumericVectorAttribConverter<IECore::IntVectorData> ToHoudiniIntVectorAttribConverter;
typedef ToHoudiniNumericVectorAttribConverter<IECore::V2iVectorData> ToHoudiniV2iVectorAttribConverter;
typedef ToHoudiniNumericVectorAttribConverter<IECore::V3iVectorData> ToHoudiniV3iVectorAttribConverter;
typedef ToHoudiniNumericVectorAttribConverter<IECore::M33fVectorData> ToHoudiniM33fVectorAttribConverter;
typedef ToHoudiniNumericVectorAttribConverter<IECore::M44fVectorData> ToHoudiniM44fVectorAttribConverter;

typedef ToHoudiniNumericDetailAttribConverter<IECore::FloatData> ToHoudiniFloatDetailAttribConverter;
typedef ToHoudiniNumericDetailAttribConverter<IECore::V2fData> ToHoudiniV2fDetailAttribConverter;
typedef ToHoudiniNumericDetailAttribConverter<IECore::V3fData> ToHoudiniV3fDetailAttribConverter;
typedef ToHoudiniNumericDetailAttribConverter<IECore::Color3fData> ToHoudiniColor3fDetailAttribConverter;
typedef ToHoudiniNumericDetailAttribConverter<IECore::IntData> ToHoudiniIntDetailAttribConverter;
typedef ToHoudiniNumericDetailAttribConverter<IECore::V2iData> ToHoudiniV2iDetailAttribConverter;
typedef ToHoudiniNumericDetailAttribConverter<IECore::V3iData> ToHoudiniV3iDetailAttribConverter;
typedef ToHoudiniNumericDetailAttribConverter<IECore::M33fData> ToHoudiniM33fDetailAttribConverter;
typedef ToHoudiniNumericDetailAttribConverter<IECore::M44fData> ToHoudiniM44fDetailAttribConverter;

} // namespace IECoreHoudini

#include "ToHoudiniNumericAttribConverter.inl"

#endif // IECOREHOUDINI_TOHOUDININUMERICATTRIBCONVERTER_H
