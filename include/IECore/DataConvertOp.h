//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#ifndef IECORE_DATACONVERTOP_H
#define IECORE_DATACONVERTOP_H

#include "IECore/Export.h"
#include "IECore/NumericParameter.h"
#include "IECore/Op.h"
#include "IECore/TypedObjectParameter.h"

namespace IECore
{

/// The DataConvertOp converts between different vector data types
/// using the ScaledDataConversion class. This distinguishes it from
/// the DataCastOp which simply casts the elements from one type to
/// another without scaling.
/// \ingroup coreGroup
/// \see DataCastOp, DataPromoteOp
class IECORE_API DataConvertOp : public Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( DataConvertOp, Op );

		DataConvertOp();
		~DataConvertOp() override;

		/// The data to be interleaved. This is specified
		/// as an ObjectVector containing Data objects of
		/// identical type and length.
		ObjectParameter *dataParameter();
		const ObjectParameter *dataParameter() const;

		/// The typeId for the type of Data to be returned
		/// as the result;
		IntParameter *targetTypeParameter();
		const IntParameter *targetTypeParameter() const;

	protected :

		ObjectPtr doOperation( const CompoundObject *operands ) override;

	private :

		struct ConvertFnStage1;
		template<class FromBaseType>
		struct ConvertFnStage2;

		static InternedString g_dataName;
		static InternedString g_targetTypeName;

};

IE_CORE_DECLAREPTR( DataConvertOp );

} // namespace IECore

#endif // IECORE_DATACONVERTOP_H
