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

#ifndef IE_COREMAYA_FROMMAYANUMERICPLUGCONVERTER_H
#define IE_COREMAYA_FROMMAYANUMERICPLUGCONVERTER_H

#include "IECore/SimpleTypedData.h"

#include "IECoreMaya/FromMayaPlugConverter.h"

namespace IECoreMaya
{

template<typename F, typename T> 
class FromMayaNumericPlugConverter : public FromMayaPlugConverter
{

	public :
	
		FromMayaNumericPlugConverter( const MPlug &plug );

		IECORE_RUNTIMETYPED_DECLARETEMPLATE( FromMayaNumericPlugConverter, FromMayaPlugConverter )
		
	protected :
		
		virtual IECore::ObjectPtr doConversion( IECore::ConstCompoundObjectPtr operands ) const;

	private :
	
		static Description<FromMayaNumericPlugConverter> m_description;
		
};

typedef FromMayaNumericPlugConverter<bool, IECore::BoolData> FromMayaNumericPlugConverterbb;
typedef FromMayaNumericPlugConverter<bool, IECore::IntData> FromMayaNumericPlugConverterbi;
typedef FromMayaNumericPlugConverter<int, IECore::IntData> FromMayaNumericPlugConverterii;
typedef FromMayaNumericPlugConverter<int, IECore::FloatData> FromMayaNumericPlugConverterif;
typedef FromMayaNumericPlugConverter<int, IECore::DoubleData> FromMayaNumericPlugConverterid;
typedef FromMayaNumericPlugConverter<float, IECore::IntData> FromMayaNumericPlugConverterfi;
typedef FromMayaNumericPlugConverter<float, IECore::FloatData> FromMayaNumericPlugConverterff;
typedef FromMayaNumericPlugConverter<float, IECore::DoubleData> FromMayaNumericPlugConverterfd;
typedef FromMayaNumericPlugConverter<double, IECore::IntData> FromMayaNumericPlugConverterdi;
typedef FromMayaNumericPlugConverter<double, IECore::FloatData> FromMayaNumericPlugConverterdf;
typedef FromMayaNumericPlugConverter<double, IECore::DoubleData> FromMayaNumericPlugConverterdd;

} // namespace IECoreMaya

#endif // IE_COREMAYA_FROMMAYANUMERICPLUGCONVERTER_H
