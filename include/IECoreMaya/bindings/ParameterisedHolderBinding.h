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

#ifndef IE_COREMAYAPYTHON_PARAMETERISEDHOLDERBINDING_H
#define IE_COREMAYAPYTHON_PARAMETERISEDHOLDERBINDING_H

#include "boost/python.hpp"

#include "IECoreMaya/ParameterisedHolder.h"
#include "IECoreMaya/bindings/NodeBinding.h"
#include "IECoreMaya/bindings/PlugBinding.h"

namespace IECoreMaya
{

class ParameterisedHolderWrapper : public Node
{
	public :
	
		/// Throws if object is not a ParameterisedHolder instance.
		ParameterisedHolderWrapper( const MObject &object );
		/// Throws if named node is not a ParameterisedHolder instance.
		ParameterisedHolderWrapper( const char *name );
	
		void setParameterised( IECore::ParameterisedPtr parameterised );
		void setParameterised( const std::string &className, int classVersion, const std::string &envVarName );
		boost::python::tuple getParameterised();
		void setNodeValues();
		void setNodeValue( IECore::ParameterPtr pa );
		void setParameterisedValues();
		void setParameterisedValue( IECore::ParameterPtr pa );
		Plug parameterPlug( IECore::ParameterPtr pa );
		IECore::ParameterPtr plugParameter( const char *plugName );
 
	private :
	
		void initInterface();
		ParameterisedHolderInterface *m_interface;

};

void bindParameterisedHolder();

}

#endif // IE_COREMAYAPYTHON_PARAMETERISEDHOLDERBINDING_H
