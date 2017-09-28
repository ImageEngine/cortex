//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
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
#include "boost/format.hpp"
#include <iostream>

#include "maya/MArgList.h"
#include "maya/MGlobal.h"
#include "maya/MSyntax.h"
#include "maya/MArgDatabase.h"

#include "IECorePython/ScopedGILLock.h"

#include "IECoreMaya/PythonCmd.h"

static const char *kCommandFlag     = "-cmd";
static const char *kCommandFlagLong = "-command";

static const char *kFileFlag	 = "-f";
static const char *kFileFlagLong = "-file";

static const char *kEvalFlag	       = "-e";
static const char *kEvalFlagLong       = "-eval";

static const char *kContextFlag     = "-ctx";
static const char *kContextFlagLong = "-context";

static const char *kCreateContextFlag	  = "-cc";
static const char *kCreateContextFlagLong = "-createContext";

static const char *kDeleteContextFlag	  = "-dc";
static const char *kDeleteContextFlagLong = "-deleteContext";

using namespace IECoreMaya;
using namespace boost::python;
using namespace boost;
using namespace std;

object PythonCmd::g_globalContext;
bool PythonCmd::g_initialized;
PythonCmd::ContextMap PythonCmd::g_contextMap;

namespace
{
	PyMethodDef initial_methods[] = { { 0, 0, 0, 0 } };
}

void PythonCmd::import( const std::string &moduleName )
{
	IECorePython::ScopedGILLock lock;

	try
	{
		string toExecute = boost::str( format(
				"import %1%\n"
			) % moduleName
		);

		handle<> ignored( PyRun_String(
			toExecute.c_str(),
			Py_file_input, g_globalContext.ptr(),
			g_globalContext.ptr() )
		);
	}
	catch ( error_already_set & )
	{
		PyErr_Print();
	}

}

void PythonCmd::initialize()
{
	if (!g_initialized)
	{
		/// Maya (8.5 onwards) may have already initialized Maya for us
		if (!Py_IsInitialized())
		{
			Py_Initialize();
		}

		assert( Py_IsInitialized() );

		IECorePython::ScopedGILLock lock;

		/// Initialize the __main__ module if not already present
		PyObject* sysModules = PyImport_GetModuleDict();
		assert(sysModules);

   		object mainModule(borrowed(PyDict_GetItemString(sysModules, "__main__")));
		if (!mainModule)
		{
			mainModule = object(borrowed(Py_InitModule("__main__", initial_methods)));
		}
		assert( mainModule );

		/// Retrieve the global context from the __main__ module
		g_globalContext = mainModule.attr("__dict__");
		assert( g_globalContext );

		import( "IECore" );
		import( "IECoreMaya" );
		import( "IECoreGL" );

		g_initialized = true;

	}
}

void PythonCmd::uninitialize()
{
	if (g_initialized)
	{
#if MAYA_API_VERSION < 850
		Py_Finalize();
#endif
		g_contextMap.clear();
	}
	g_initialized = false;
}

boost::python::object &PythonCmd::globalContext()
{
	return g_globalContext;
}

PythonCmd::PythonCmd()
{
	assert( g_initialized );
}

PythonCmd::~PythonCmd()
{
}

void *PythonCmd::creator()
{
	return new PythonCmd;
}

MSyntax PythonCmd::newSyntax()
{
	MSyntax syn;
	MStatus s;

	s = syn.addFlag( kCommandFlag, kCommandFlagLong, MSyntax::kString );
	assert(s);

	s = syn.addFlag( kFileFlag, kFileFlagLong, MSyntax::kString );
	assert(s);

	s = syn.addFlag( kEvalFlag, kEvalFlagLong, MSyntax::kString );
	assert(s);

	s = syn.addFlag( kContextFlag, kContextFlagLong, MSyntax::kString );
	assert(s);

	s = syn.addFlag( kCreateContextFlag, kCreateContextFlagLong, MSyntax::kString );
	assert(s);

	s = syn.addFlag( kDeleteContextFlag, kDeleteContextFlagLong, MSyntax::kString );
	assert(s);

	return syn;
}


MStatus PythonCmd::doIt( const MArgList &argList )
{
	MStatus s;
	MArgDatabase args( syntax(), argList );

	if (args.isFlagSet( kCommandFlag ) && args.isFlagSet( kFileFlag ) )
	{
		displayError("Must specify only one of " + MString(kCommandFlagLong) + "/" + MString(kFileFlagLong));
		return MS::kFailure;
	}

	IECorePython::ScopedGILLock gilLock;

	list argv;

	PySys_SetObject( (char *)"argv", argv.ptr() );

	object *context = &g_globalContext;
	assert(context);

	if (args.isFlagSet( kContextFlagLong ) )
	{
		if (args.isFlagSet( kCreateContextFlagLong ) || args.isFlagSet( kDeleteContextFlagLong ) )
		{
			displayError("Syntax error");
			return MS::kFailure;
		}

		if (!args.isFlagSet( kCommandFlagLong ) && !args.isFlagSet( kFileFlagLong ) )
		{
			displayError("Must specify one of " + MString(kCommandFlagLong) + "/" + MString(kFileFlagLong) );
			return MS::kFailure;
		}

		MString contextName;
		s = args.getFlagArgument( kContextFlagLong, 0, contextName );
		assert(s);

		ContextMap::iterator it = g_contextMap.find( contextName.asChar() );
		if (it == g_contextMap.end())
		{
			displayError("Context does not exist");
			return MS::kFailure;
		}

		context = &(it->second);
		assert( context != &g_globalContext );
	}

	if (args.isFlagSet( kCreateContextFlag ) || args.isFlagSet( kCreateContextFlagLong ))
	{
		if (args.isFlagSet( kContextFlagLong ) || args.isFlagSet( kDeleteContextFlagLong ) )
		{
			displayError("Syntax error");
			return MS::kFailure;
		}

		MString contextName;
		s = args.getFlagArgument( kCreateContextFlagLong, 0, contextName );
		assert(s);

		ContextMap::iterator it = g_contextMap.find( contextName.asChar() );
		if (it != g_contextMap.end())
		{
			displayWarning("Context already exists");
			context = &(it->second);
		}
		else
		{
			g_contextMap[ contextName.asChar() ] = dict();
			context = &(g_contextMap[ contextName.asChar() ]);
		}
		assert( context != &g_globalContext );
		return MS::kSuccess;
	}

	if (args.isFlagSet( kDeleteContextFlagLong ) )
	{
		if (args.isFlagSet( kContextFlagLong ) || args.isFlagSet( kCreateContextFlagLong ) )
		{
			displayError("Syntax error");
			return MS::kFailure;
		}

		if (args.isFlagSet( kCommandFlagLong ) || args.isFlagSet( kFileFlagLong ) )
		{
			displayError("Syntax error");
			return MS::kFailure;
		}

		MString contextName;
		s = args.getFlagArgument( kDeleteContextFlagLong, 0, contextName );
		assert(s);

		ContextMap::iterator it = g_contextMap.find( contextName.asChar() );
		if (it == g_contextMap.end())
		{
			displayWarning("Context does not exist");
		}
		else
		{
			g_contextMap.erase( it );
		}
		return MS::kSuccess;
	}

	assert(context);

	if (args.isFlagSet( kCommandFlagLong ) )
	{
		if (args.isFlagSet( kFileFlagLong ) || args.isFlagSet( kEvalFlagLong ) )
		{
			displayError("Must specify only one of " + MString(kCommandFlagLong) + "/" + MString(kFileFlagLong) + "/" + MString(kEvalFlagLong));
			return MS::kFailure;
		}
		MString cmd;
		s = args.getFlagArgument( kCommandFlagLong, 0, cmd );

		argv.append( "<string>" );

		try
		{
			handle<> ignored((
				PyRun_String(
					cmd.asChar(),
					Py_file_input,
					context->ptr(),
					context->ptr()
					)
				));
			return MS::kSuccess;
		}
		catch(error_already_set)
		{
			PyErr_Print();
			return MS::kFailure;
		}
		catch(...)
		{
			displayError("Caught unexpected exception");
			return MS::kFailure;
		}

	}
	else if (args.isFlagSet( kFileFlagLong ) )
	{
		if (args.isFlagSet( kCommandFlagLong ) || args.isFlagSet( kEvalFlagLong ) )
		{
			displayError("Must specify only one of " + MString(kCommandFlagLong) + "/" + MString(kFileFlagLong) + "/" + MString(kEvalFlagLong) );
			return MS::kFailure;
		}

		MString filename;
		s = args.getFlagArgument( kFileFlagLong, 0, filename );
		assert(s);

		argv.append( filename.asChar() ); // causes python to print out the filename appropriately in a stack trace

		FILE *fp = fopen( filename.asChar(), "r" );
		if (!fp)
		{
			displayError( MString("Cannot open file ") + filename );
			return MS::kFailure;
		}

		try
		{
			handle<> ignored((
				PyRun_FileEx(
					fp,
					filename.asChar(),
					Py_file_input,
					context->ptr(),
					context->ptr(),
					1
					)
				));
			return MS::kSuccess;
		}
		catch(error_already_set)
		{
			PyErr_Print();
			return MS::kFailure;
		}
		catch(...)
		{
			displayError("Caught unexpected exception");
			return MS::kFailure;
		}
	}
	else if (args.isFlagSet( kEvalFlagLong ) )
	{
		if (args.isFlagSet( kCommandFlagLong ) || args.isFlagSet( kFileFlagLong ) )
		{
			displayError("Must specify only one of " + MString(kCommandFlagLong) + "/" + MString(kFileFlagLong) + "/" + MString(kEvalFlagLong) );
			return MS::kFailure;
		}

		MString cmd;
		s = args.getFlagArgument( kEvalFlagLong, 0, cmd );
		assert(s);

		argv.append( cmd.asChar() );

		try
		{
			handle<> resultHandle( (
				PyRun_String(
					cmd.asChar(),
					Py_eval_input,
					context->ptr(),
					context->ptr()
				)
			) );

			object result( resultHandle );
			string strResult = extract<string>( boost::python::str( result ) )();
			setResult( strResult.c_str() );
			return MS::kSuccess;
		}
		catch(error_already_set)
		{
			PyErr_Print();
			return MS::kFailure;
		}
		catch(...)
		{
			displayError( "Caught unexpected exception" );
			return MS::kFailure;
		}
	}

	return MS::kFailure;
}
