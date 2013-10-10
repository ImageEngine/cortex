//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_FROMMAYAPLUGCONVERTER_INL
#define IE_COREMAYA_FROMMAYAPLUGCONVERTER_INL

#include "IECoreMaya/FromMayaPlugConverter.h"

namespace IECoreMaya
{

template<class T>
FromMayaPlugConverter::Description<T>::Description( MFnNumericData::Type fromType, IECore::TypeId resultType, bool isDefaultConverter )
{
	FromMayaPlugConverter::registerConverter( fromType, resultType, isDefaultConverter, creator );
	/// \todo Derive FromMayaPlugConverter::Description from RunTimeTyped::TypeDescription instead of calling this manually.
	IECore::RunTimeTyped::registerType( T::staticTypeId(), T::staticTypeName(), T::baseTypeId() );
}

template<class T>
FromMayaPlugConverter::Description<T>::Description( MFnData::Type fromType, IECore::TypeId resultType, bool isDefaultConverter )
{
	FromMayaPlugConverter::registerConverter( fromType, resultType, isDefaultConverter, creator );
	/// \todo Derive FromMayaPlugConverter::Description from RunTimeTyped::TypeDescription instead of calling this manually.
	IECore::RunTimeTyped::registerType( T::staticTypeId(), T::staticTypeName(), T::baseTypeId() );
}

template<class T>
FromMayaPlugConverter::Description<T>::Description( MFnUnitAttribute::Type fromType, IECore::TypeId resultType, bool isDefaultConverter )
{
	FromMayaPlugConverter::registerConverter( fromType, resultType, isDefaultConverter, creator );
	/// \todo Derive FromMayaPlugConverter::Description from RunTimeTyped::TypeDescription instead of calling this manually.
	IECore::RunTimeTyped::registerType( T::staticTypeId(), T::staticTypeName(), T::baseTypeId() );
}

template<class T>
FromMayaPlugConverterPtr FromMayaPlugConverter::Description<T>::creator( const MPlug &plug )
{
	return new T( plug );
}

} // namespace IECoreMaya

#endif // IE_COREMAYA_FROMMAYAPLUGCONVERTER_INL
