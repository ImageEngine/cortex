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

#ifndef IE_COREMAYA_TOMAYANUMERICDATACONVERTER_H
#define IE_COREMAYA_TOMAYANUMERICDATACONVERTER_H

#include "IECoreMaya/ToMayaObjectConverter.h"

#include "IECore/VectorTypedData.h"

#include "maya/MString.h"

namespace IECoreMaya
{

/// The ToMayaNumericDataConverter converts numeric IECore types into compatible MObjects
/// \ingroup conversionGroup
class ToMayaNumericDataConverter : public ToMayaObjectConverter
{

	public :

		ToMayaNumericDataConverter( IECore::ConstObjectPtr object );

	protected :

		virtual bool doConversion( IECore::ConstObjectPtr from, MObject &to, IECore::ConstCompoundObjectPtr operands ) const;

	private :

		static ToMayaObjectConverterDescription<ToMayaNumericDataConverter> g_3Double;
		static ToMayaObjectConverterDescription<ToMayaNumericDataConverter> g_3Float;
		static ToMayaObjectConverterDescription<ToMayaNumericDataConverter> g_3Int;
		static ToMayaObjectConverterDescription<ToMayaNumericDataConverter> g_3Short;
		static ToMayaObjectConverterDescription<ToMayaNumericDataConverter> g_2Double;
		static ToMayaObjectConverterDescription<ToMayaNumericDataConverter> g_2Float;
		static ToMayaObjectConverterDescription<ToMayaNumericDataConverter> g_2Int;
		static ToMayaObjectConverterDescription<ToMayaNumericDataConverter> g_2Short;
};

IE_CORE_DECLAREPTR( ToMayaNumericDataConverter );

} // namespace IECoreMaya

#endif // IE_COREMAYA_TOMAYANUMERICDATACONVERTER_H
