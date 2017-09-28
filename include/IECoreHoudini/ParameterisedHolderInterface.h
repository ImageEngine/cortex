//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#ifndef IECOREHOUDINI_PARAMETERISEDHOLDERINTERFACE_H
#define IECOREHOUDINI_PARAMETERISEDHOLDERINTERFACE_H

#include "IECore/Parameterised.h"
#include "IECore/Parameter.h"

namespace IECoreHoudini
{

/// A base class from which nodes to hold IECore::ParameterisedInterface objects
/// should multiply inherit (for example, ParameterisedHolder).
class ParameterisedHolderInterface
{

	public :

		ParameterisedHolderInterface();
		virtual ~ParameterisedHolderInterface();

		/// Sets the Parameterised object this node is holding. An IECore.ClassLoader object will be
		/// used with searchpaths obtained from the specified environment variable to actually load
		/// the Parameterised object. This mechanism is used rather than passing a ParameterisedPtr
		/// as it allows the Parameterised object to be loaded again when a houdini scene is opened.
		virtual void setParameterised( const std::string &className, int classVersion, const std::string &searchPathEnvVar ) = 0;
		/// Sets the Parameterised object this node is holding, directly.
		virtual void setParameterised( IECore::RunTimeTypedPtr p ) = 0;

		/// Returns whether or not this node is holding a valid parameterised object
		virtual bool hasParameterised() = 0;

		/// Returns the parameterised object held by this node
		virtual IECore::RunTimeTypedPtr getParameterised() = 0;

		/// Convenience method to return dynamic_cast<IECore::ParameterisedInterface *>( getParameterised().get() )
		IECore::ParameterisedInterface *getParameterisedInterface();

		/// Sets the attributes of the node to reflect the current values of the parameters in the held
		/// Parameterised object. Performs validation of the parameter values and will return false if
		//// any one is not valid.
		virtual bool setNodeValues() = 0;

		/// Sets the values of the parameters of the held Parameterised object to reflect the values
		// of the attributes of the node.
		virtual void setParameterisedValues( double time ) = 0;

};

}

#endif // IECOREHOUDINI_PARAMETERISEDHOLDERINTERFACE_H
