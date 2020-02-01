//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2019, Image Engine Design Inc. All rights reserved.
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

#include "IECoreMaya/FromMayaEnumPlugConverter.h"

#include "IECore/MessageHandler.h"

#include "maya/MString.h"
#include "maya/MFnEnumAttribute.h"



using namespace IECore;

namespace IECoreMaya
{

template<typename T>
FromMayaPlugConverter::Description< FromMayaEnumPlugConverter<T> > FromMayaEnumPlugConverter<T>::m_description{};

template<typename T>
const MString FromMayaEnumPlugConverter<T>::convertToStringCategory = "ieConvertToStringData";

template<typename T>
FromMayaEnumPlugConverter<T>::FromMayaEnumPlugConverter( const MPlug &plug )
	: FromMayaPlugConverter( plug )
{
}

template<>
IECore::ObjectPtr FromMayaEnumPlugConverter<IECore::ShortData>::doConversion( IECore::ConstCompoundObjectPtr operands ) const
{
	return new IECore::ShortData( plug().asShort() );
}

template<>
IECore::ObjectPtr FromMayaEnumPlugConverter<IECore::StringData>::doConversion( IECore::ConstCompoundObjectPtr operands ) const
{
	MFnEnumAttribute fne( plug().attribute() );
	return new IECore::StringData( fne.fieldName( plug().asShort() ).asChar() );
}

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaEnumPlugConverterst, FromMayaEnumPlugConverterstTypeId )
IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( FromMayaEnumPlugConvertersh, FromMayaEnumPlugConvertershTypeId )

template class FromMayaEnumPlugConverter<IECore::StringData>;
template class FromMayaEnumPlugConverter<IECore::ShortData>;
}
