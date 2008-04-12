//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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

#ifndef IE_COREMAYA_PYTHONCMD_H
#define IE_COREMAYA_PYTHONCMD_H

#include <boost/python.hpp>

#include <string>
#include <map>

#include "maya/MPxCommand.h"

namespace IECoreMaya
{

/// A Maya plugin command to allow execution of Python statements/files under
/// specific named contexts. Provides methods for creating and deleting contexts.
/// Flags:<br>
/// <ul>
/// <li>-cmd / -command <i>&lt;string&gt;</i> : Execute an explicit python command string</li>
/// <li>-f / -file <i>&lt;string&gt;</i> : Execute python commands from the given file</li>
/// <li>-e / -eval <i>&lt;string&gt;</i> : Evaluate a line of python and return the result in string form</li>
/// <li>-ctx / -context <i>&lt;string&gt;</i> : Execute python under the named context</li>
/// <li>-cctx / -createContext <i>&lt;string&gt;</i> : Create a new python context</li>
/// <li>-dctx / -deleteContext <i>&lt;string&gt;</i> : Remove a python context</li>
/// </ul>
/// If a context for execution is not specified then a default global one is used.<br>
/// Examples:
/// <br><br>
/// <code>python -command "print(str(dir()))"</code><br>
/// <code>python -createContext "myContext" -command "i = 3";</code><br>
/// <code>python -context "myContext" -command "print(i)";</code><br>
/// <code>python -createContext "anotherContext" -command "print(i)";</code><br>
/// <code>python -createContext "moreContext";</code>;<br>
/// <code>python -file "/tmp/somePython.py"</code>;<br>
/// <code>python -eval "10 * 10"</code>;<br>
/// \todo Support the conversion of -eval results into the most appropriate mel form based on type
/// \todo Split the management of python contexts and all the evaluation code into a Python object
/// defined in an IECorePython library. Then we can use that object in a Nuke python plugin and
/// wherever else we want. And this command becomes a lot smaller and easier to maintain. Nice.
class PythonCmd : public MPxCommand
{
	public:
		PythonCmd();
		virtual ~PythonCmd();
		
		static void* creator();
		static MSyntax newSyntax();
		
		MStatus doIt( const MArgList & args );
		
		static void import( const std::string &moduleName );
		
		static void initialize();
		static void uninitialize();
		
		/// Returns the python context used for commands not specifying a context.
		/// This may be useful for executing python code from c++ elsewhere.
		static boost::python::object &globalContext();
		
	private:
		static bool g_initialized;
		static boost::python::object g_globalContext;
		
		typedef std::map< std::string, boost::python::dict> ContextMap;
		
		static ContextMap g_contextMap;
};

}

#endif // IE_COREMAYA_PYTHONCMD_H
