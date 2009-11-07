//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

#include <iostream>

#include "ri.h"

#if defined(_WIN32)
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

using namespace std;
using namespace boost::python;

static object g_mainModule;
static object g_mainModuleNamespace;

static void initialise()
{
	static bool initialised = false;
	if( !initialised )
	{
		// start python
		Py_Initialize();
		g_mainModule = object( handle<>( borrowed( PyImport_AddModule( "__main__" ) ) ) );
		g_mainModuleNamespace = g_mainModule.attr( "__dict__" );
		
		// load the IECoreRI and IECore modules so people don't have to do that in the string
		// they pass to be executed. this also means people don't have to worry about which
		// version to load.
		string toExecute =  "import IECore\nimport IECoreRI\n";
		
		handle<> ignored( PyRun_String( 
			toExecute.c_str(),
			Py_file_input, g_mainModuleNamespace.ptr(),
			g_mainModuleNamespace.ptr() ) );
		
		initialised = true;
	}
}

extern "C"
{

RtPointer DLLEXPORT ConvertParameters( RtString paramstr )
{
	return new string( paramstr );
}

RtVoid DLLEXPORT Subdivide( RtPointer data, float detail )
{
	try
	{
		initialise();
		
		string *i = (string *)data;
		
		handle<> ignored( PyRun_String( i->c_str(), Py_file_input, g_mainModuleNamespace.ptr(),
			g_mainModuleNamespace.ptr() ) );
		
	}
	catch( const error_already_set &e )
	{
		PyErr_Print();
	}
	catch( const std::exception &e )
	{
		cerr << "ERROR : Python procedural : " << e.what() << endl;
	}
	catch( ... )
	{
		cerr << "ERROR : Python procedural : caught unknown exception" << endl;
	}
}

RtVoid DLLEXPORT Free( RtPointer data )
{
	string * i = (string *)data;
	delete i;
}

}
