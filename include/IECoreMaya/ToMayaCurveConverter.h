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

#ifndef IE_COREMAYA_TOMAYACURVECONVERTER_H
#define IE_COREMAYA_TOMAYACURVECONVERTER_H

#include "IECore/CurvesPrimitive.h"
#include "IECore/NumericParameter.h"

#include "IECoreMaya/ToMayaObjectConverter.h"

namespace IECoreMaya
{

class ToMayaCurveConverter;
IE_CORE_DECLAREPTR( ToMayaCurveConverter );

/// This class converts IECore::CurvesPrimitives to maya curve objects.
/// \ingroup conversionGroup
class ToMayaCurveConverter : public ToMayaObjectConverter
{
	public:

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( ToMayaCurveConverter, ToMayaCurveConverterTypeId, ToMayaObjectConverter );

		ToMayaCurveConverter( IECore::ConstObjectPtr object );

		IECore::IntParameterPtr indexParameter();
		IECore::ConstIntParameterPtr indexParameter() const;

	protected:

		/// Converts one of the curves in srcParameter() to a maya curve. The curve it converts
		/// is specified by indexParameter() (named "index")
		virtual bool doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const;

		typedef ToMayaObjectConverterDescription<ToMayaCurveConverter> Description;
		static Description g_curvesDataDescription;
		static Description g_curvesDescription;

	private:

		IECore::IntParameterPtr m_indexParameter;
};

}

#endif // IE_COREMAYA_TOMAYACURVECONVERTER_H
