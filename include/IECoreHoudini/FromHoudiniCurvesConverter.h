//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2010-2014, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_FROMHOUDINICURVESCONVERTER_H
#define IECOREHOUDINI_FROMHOUDINICURVESCONVERTER_H

#include "IECoreHoudini/FromHoudiniGeometryConverter.h"
#include "IECoreHoudini/TypeIds.h"

#include "IECoreScene/CurvesPrimitive.h"

namespace IECoreHoudini
{

/// Converter which converts from a Houdini GU_Detail to an IECore::CurvesPrimitive.
class IECOREHOUDINI_API FromHoudiniCurvesConverter : public IECoreHoudini::FromHoudiniGeometryConverter
{
	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromHoudiniCurvesConverter, FromHoudiniCurvesConverterTypeId, IECoreHoudini::FromHoudiniGeometryConverter );

		FromHoudiniCurvesConverter( const GU_DetailHandle &handle );
		FromHoudiniCurvesConverter( const SOP_Node *sop );

		virtual ~FromHoudiniCurvesConverter();

		/// Determines if the given GU_Detail can be converted
		static FromHoudiniGeometryConverter::Convertability canConvert( const GU_Detail *geo );

	protected :

		/// performs conversion to an IECore::CurvesPrimitive
		virtual IECore::ObjectPtr doDetailConversion( const GU_Detail *geo, const IECore::CompoundObject *operands ) const;

	private :

		static FromHoudiniGeometryConverter::Description<FromHoudiniCurvesConverter> m_description;

		/// Utility struct for duplicating curve end points using DespatchedTypedData
		struct DuplicateEnds
		{
			typedef void ReturnType;

			DuplicateEnds( const std::vector<int> &vertsPerCurve );

			template<typename T>
			ReturnType operator()( T *data ) const;

			const std::vector<int> &m_vertsPerCurve;
		};

		static bool compatiblePrimitive( GA_PrimitiveTypeId type )
		{
			if ( ( type == GEO_PRIMNURBCURVE ) || ( type == GEO_PRIMBEZCURVE ) || (type == GEO_PRIMPOLY ))
			{
				return true;
			}

			return false;
		};
};

// register our converter
IE_CORE_DECLAREPTR( FromHoudiniCurvesConverter );

}

#endif // IECOREHOUDINI_FROMHOUDINICURVESCONVERTER_H
