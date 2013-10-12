//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
//  its affiliates and/or its licensors.
//
//  Copyright (c) 2010-2013, Image Engine Design Inc. All rights reserved.
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

#include "CH/CH_Manager.h" 

#include "IECore/MessageHandler.h"

#include "IECorePython/ScopedGILLock.h"

#include "IECoreHoudini/CoreHoudini.h"

using namespace IECoreHoudini;
using namespace boost::python;

boost::python::object CoreHoudini::g_globalContext;
bool CoreHoudini::g_initialized = false;

void CoreHoudini::initPython()
{
	if (!g_initialized)
	{
		{
			// the global dictionary
			IECorePython::ScopedGILLock lock;
			object main_module((
				handle<>(borrowed(PyImport_AddModule("__main__")))));
			g_globalContext = main_module.attr("__dict__");
		}

		// import our main modules
		import( "hou" );
		import( "IECore" );
		import( "IECoreHoudini" );
		import( "IECoreGL" );

		g_initialized = true;
	}
}

void CoreHoudini::import( const std::string &module )
{
	IECorePython::ScopedGILLock lock;
	try
	{
		boost::python::object pymodule( ( boost::python::handle<>(PyImport_ImportModule(module.c_str())) ) );
		g_globalContext[module] = pymodule;
	}
	catch( ... )
	{
		PyErr_Print();
	}
}

object CoreHoudini::evalPython( const std::string &cmd )
{
	IECorePython::ScopedGILLock lock;
	object result;
	try
	{
		handle<> resultHandle( PyRun_String( cmd.c_str(), Py_eval_input,
				CoreHoudini::globalContext().ptr(), CoreHoudini::globalContext().ptr() ) );
		result = object( resultHandle );
	}
	catch( ... )
	{
		PyErr_Print();
	}
	return result;
}
boost::python::object & CoreHoudini::globalContext()
{
	return g_globalContext;
}
