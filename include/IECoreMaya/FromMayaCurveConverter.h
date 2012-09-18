//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREMAYA_FROMMAYACURVECONVERTER_H
#define IECOREMAYA_FROMMAYACURVECONVERTER_H

#include "IECoreMaya/FromMayaShapeConverter.h"

#include "IECore/TypedParameter.h"

class MFnNurbsCurve;

namespace IECoreMaya
{

/// Converts maya curveShape objects into IECore::CurvesPrimitive objects.
/// \ingroup conversionGroup
class FromMayaCurveConverter : public FromMayaShapeConverter
{

	public :

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( FromMayaCurveConverter, FromMayaCurveConverterTypeId, FromMayaShapeConverter );

		FromMayaCurveConverter( const MObject &object );
		FromMayaCurveConverter( const MDagPath &dagPath );

		IECore::BoolParameterPtr linearParameter();
		IECore::ConstBoolParameterPtr linearParameter() const;

	protected :

		virtual IECore::PrimitivePtr doPrimitiveConversion( const MObject &object, IECore::ConstCompoundObjectPtr operands ) const;
		virtual IECore::PrimitivePtr doPrimitiveConversion( const MDagPath &dagPath, IECore::ConstCompoundObjectPtr operands ) const;

	private :

		void constructCommon();

		IECore::PrimitivePtr doPrimitiveConversion( MFnNurbsCurve &fnCurve ) const;

		static Description<FromMayaCurveConverter> m_description;
		static Description<FromMayaCurveConverter> m_dataDescription;

		IECore::BoolParameterPtr m_linearParameter;

};

IE_CORE_DECLAREPTR( FromMayaCurveConverter );

} // namespace IECoreMaya

#endif // IECOREMAYA_FROMMAYACURVECONVERTER_H
