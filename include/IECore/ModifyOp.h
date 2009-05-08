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

#ifndef IE_CORE_MODIFYOP_H
#define IE_CORE_MODIFYOP_H

#include "IECore/Op.h"
#include "IECore/SimpleTypedParameter.h"

namespace IECore
{

/// The ModifyOp class defines a base class for Ops whose
/// result is some modification of an input object. It defines
/// a parameter for the input object, a parameter to define
/// whether or not the object is modified in place or is copied
/// and a parameter to disable the operation completely. It's a little
/// bit naughty to modify it in place but it'll probably be quite handy
/// at times.
class ModifyOp : public Op
{
	public :

		IE_CORE_DECLARERUNTIMETYPED( ModifyOp, Op );

		ModifyOp( const std::string name, const std::string description, ParameterPtr resultParameter, ParameterPtr inputParameter );
		virtual ~ModifyOp();

		ParameterPtr inputParameter();
		ConstParameterPtr inputParameter() const;

		BoolParameterPtr copyParameter();
		BoolParameterPtr copyParameter() const;

		BoolParameterPtr enableParameter();
		BoolParameterPtr enableParameter() const;

	protected :

		/// Implemented to call modify() - implement modify rather than this.
		virtual ObjectPtr doOperation( ConstCompoundObjectPtr operands );

		/// Should be implemented by all subclasses to modify object.
		/// This won't be called if the Op is not enabled.
		virtual void modify( ObjectPtr object, ConstCompoundObjectPtr operands ) = 0;

	private :

		ParameterPtr m_inputParameter;
		BoolParameterPtr m_copyParameter;
		BoolParameterPtr m_enableParameter;

};

IE_CORE_DECLAREPTR( ModifyOp );

} // namespace IECore

#endif // IE_CORE_MODIFYOP_H
