//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2010, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_DATAPROMOTEOP_H
#define IE_CORE_DATAPROMOTEOP_H

#include "boost/static_assert.hpp"

#include "IECore/Export.h"
#include "IECore/Op.h"
#include "IECore/ObjectParameter.h"
#include "IECore/NumericParameter.h"

namespace IECore
{

/// The DataPromoteOp promotes data of simple scalar types
/// to data of compound types - for instance constructing
/// V3f from float or int. It differs from the DataCastOp
/// in that the former keeps the rawSize() of the data constant,
/// while this keeps the size() constant (e.g. one V3f is made from
/// each float).
/// \ingroup coreGroup
/// \see DataCastOp, DataConvertOp
class IECORE_API DataPromoteOp : public Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( DataPromoteOp, Op );

		DataPromoteOp();
		virtual ~DataPromoteOp();

	protected :

		virtual ObjectPtr doOperation( const CompoundObject * operands );

	private :

		struct Promote1Fn;

		template<typename T, typename E=void >
		struct Promote2Fn
		{
			BOOST_STATIC_ASSERT( sizeof(T) == 0 );
		};

		ObjectParameterPtr m_objectParameter;
		IntParameterPtr m_targetTypeParameter;
};

IE_CORE_DECLAREPTR( DataPromoteOp );

} // namespace IECore

#endif // IE_CORE_DATAPROMOTEOP_H
