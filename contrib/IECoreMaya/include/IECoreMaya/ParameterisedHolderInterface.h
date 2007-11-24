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

#ifndef IE_COREMAYA_PARAMETERISEDHOLDERINTERFACE_H
#define IE_COREMAYA_PARAMETERISEDHOLDERINTERFACE_H

#include "maya/MPxNode.h"

#include "IECore/Parameterised.h"
#include "IECore/Parameter.h"

namespace IECoreMaya
{

/// A base class from which nodes to hold IECore::Parametised objects
/// should multiply inherit (for example, Maya RI procedurals).
class ParameterisedHolderInterface 
{

	public:
	
		ParameterisedHolderInterface();
		virtual ~ParameterisedHolderInterface();
		
		/// Sets the Parameterised object this node is holding. An IECore.ClassLoader object will be
		/// used with searchpaths obtained from the specified environment variable to actually load
		/// the Parameterised object. This mechanism is used rather than passing a ParameterisedPtr
		/// as it allows the Parameterised object to be loaded again when a maya scene is opened.
		virtual MStatus setParameterised( const std::string &className, int classVersion, const std::string &searchPathEnvVar ) = 0;
		/// Returns the held Parameterised object, loading it if necessary. May return 0 if loading
		/// fails. Note that this doesn't update the values of the parameters - you can use the
		/// separate setParameterisedValues() call for that. If provided, the optional className,
		/// classVersion and searchPathEnvVar are updated to reflect the last values passed to
		/// setParameterised - in the case of a 0 return value these values are left unchanged.
		virtual IECore::ParameterisedPtr getParameterised( std::string *className = 0, int *classVersion = 0, std::string *searchPathEnvVar = 0 ) = 0;
		/// Sets the attributes of the node to reflect the current values
		/// of the parameters in the held Parameterised object. Performs validation
		/// of the parameter values and will return kFailure if any one is not valid.
		/// \todo We need a version of this that operates on just one Parameter.
		virtual MStatus setNodeValues() = 0;
		/// Sets the values of the parameters of the held Parameterised object
		/// to reflect the values of the attributes of the node. Performs validation
		/// of the parameter values and will return kFailure if any one in not valid.
		/// We need a version of this that operates on just one Parameter.
		virtual MStatus setParameterisedValues() = 0;
		/// Returns the plug used to represent the specified parameter, which should
		/// be a child of getParameterised()->parameters(). On failure returns a plug
		/// for which plug.isNull() returns true.
		virtual MPlug parameterPlug( IECore::ConstParameterPtr parameter ) = 0;
		/// Returns the parameter represented by the specified plug, returning 0
		/// if no such parameter exists.
		virtual IECore::ParameterPtr plugParameter( const MPlug &plug ) = 0;
	
};

}

#endif // IE_COREMAYA_PARAMETERISEDHOLDERINTERFACE_H
