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

#ifndef IE_CORE_PARAMETERISED_H
#define IE_CORE_PARAMETERISED_H

#include "IECore/RunTimeTyped.h"
#include "IECore/ParameterisedInterface.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( CompoundObject )

/// The Parameterised class provides a useful base class for all
/// classes wishing to implement both RunTimeTyped and ParameterisedInterface.
class Parameterised : public RunTimeTyped, public ParameterisedInterface
{

	public :
		
		IE_CORE_DECLARERUNTIMETYPED( Parameterised, RunTimeTyped );
		
		/// The base class constructor receives the name and a short description about the Parameterised object created.
		/// They are usually defined within the derived class constructors.
		Parameterised( const std::string name, const std::string description );

		/// Another constructor added later on that gives defines the CompoundParameter object to be hold in this Parameterised instance.
		Parameterised( const std::string name, const std::string description, CompoundParameterPtr compoundParameter );

		virtual ~Parameterised();

		/// Returns the name of this parameterised object.
		const std::string &name();
		/// Returns a description for this parameterised object.
		const std::string &description();
		
		/// Returns the parameters for editing. Subclasses should
		/// typically add parameters to this from their constructors.
		virtual CompoundParameterPtr parameters();
		/// Returns the parameters for query.
		virtual ConstCompoundParameterPtr parameters() const;

		/// Returns an object that anyone can use for storing anything.
		CompoundObjectPtr userData();
		/// Read only version of the above.
		ConstCompoundObjectPtr userData() const;
		
	private :

		std::string m_name;
		std::string m_description;
		CompoundParameterPtr m_parameters;
		CompoundObjectPtr m_userData;
		
};

IE_CORE_DECLAREPTR( Parameterised );

} // namespace IECore

#endif // IE_CORE_PARAMETERISED_H
