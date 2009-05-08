//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_CORE_RANDOMROTATIONOP_H
#define IE_CORE_RANDOMROTATIONOP_H

#include "IECore/Op.h"
#include "IECore/NumericParameter.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( ObjectParameter )

/// The RandomRotationOp calculates a set of random vectors which rotate
/// coherently over time.
class RandomRotationOp : public Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( RandomRotationOp, Op );

		RandomRotationOp();
		virtual ~RandomRotationOp();

		ObjectParameterPtr seedParameter();
		ConstObjectParameterPtr seedParameter() const;

		FloatParameterPtr timeParameter();
		ConstFloatParameterPtr timeParameter() const;

		FloatParameterPtr speedMinParameter();
		ConstFloatParameterPtr speedMinParameter() const;

		FloatParameterPtr speedMaxParameter();
		ConstFloatParameterPtr speedMaxParameter() const;

		/// The core algorithm, for those who want direct access from C++ without
		/// all the Op wrapping.
		template<typename SeedIt, typename OutputIt>
		static void generate(
			SeedIt seedBegin,
			SeedIt seedEnd,
			typename std::iterator_traits<OutputIt>::value_type::BaseType time,
			typename std::iterator_traits<OutputIt>::value_type::BaseType speedMin,
			typename std::iterator_traits<OutputIt>::value_type::BaseType speedMax,
			OutputIt result
		);

	protected :

		virtual ObjectPtr doOperation( ConstCompoundObjectPtr operands );

	private :

		ObjectParameterPtr m_seedParameter;
		FloatParameterPtr m_timeParameter;
		FloatParameterPtr m_speedMinParameter;
		FloatParameterPtr m_speedMaxParameter;

};

IE_CORE_DECLAREPTR( RandomRotationOp );

} // namespace IECore

#include "IECore/RandomRotationOp.inl"

#endif // IE_CORE_RANDOMROTATIONOP_H
