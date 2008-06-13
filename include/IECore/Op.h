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

#ifndef IE_CORE_OP_H
#define IE_CORE_OP_H

#include "IECore/Parameterised.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( Object );
IE_CORE_FORWARDDECLARE( Parameter );
IE_CORE_FORWARDDECLARE( CompoundObject );

/// The Op class defines a base class for objects which perform an operation
/// based on some input parameters and returns a result derived from Object.
/// Parameter objects are used to define both the format of the inputs and
/// the result.
class Op : public Parameterised
{
	public :
		
		IE_CORE_DECLARERUNTIMETYPED( Op, Parameterised );
		
		/// Derived classes must pass a suitable resultParameter in their
		/// initialiser. Also, every non-abstract derived class from Op should 
		/// provide a default constructor so that it's possible to extract the 
		/// parameters from an unknown Op (method used in ClassLoader). 
		Op( const std::string name, const std::string description, ParameterPtr resultParameter );

		/// Alternative constructor for replacing the internal CompoundParameter object by another
		/// derived class from CompoundParameter.
		Op( const std::string name, const std::string description, CompoundParameterPtr compoundParameter, ParameterPtr resultParameter );
		
		virtual ~Op();
		
		/// Performs the operation using the current values of parameters().
		/// Throws an Exception if the parameter values are not valid.
		ObjectPtr operate();
		
		/// Returns a parameter describing the result of the operation - the
		/// value of this parameter is always the value last returned by operate.
		ConstParameterPtr resultParameter() const;
		
	protected :

		/// Called by operate() to actually perform the operation. operands
		/// contains the result of parameters()->getValidatedValue - this
		/// function will never be called when the contents of the parameters
		/// are in a bad state.
		/// \todo This should be const.
		virtual ObjectPtr doOperation( ConstCompoundObjectPtr operands ) = 0;
		
	private :
	
		ParameterPtr m_resultParameter;

};

IE_CORE_DECLAREPTR( Op );

} // namespace IECore

#endif // IE_CORE_OP_H
