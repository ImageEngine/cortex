//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "ai.h"

#include "IECore/MessageHandler.h"
#include "IECore/ParameterisedProcedural.h"

#include "IECorePython/ScopedGILLock.h"

#include "IECoreArnold/Renderer.h"

#include <iostream>

using namespace std;
using namespace boost::python;
using namespace IECore;
using namespace IECorePython;

static object g_mainModule;
static object g_mainModuleNamespace;

static void initialisePython()
{
	if( Py_IsInitialized() )
	{
		return;
	}

	Py_Initialize();

	PyEval_InitThreads();

		try
		{
			g_mainModule = object( handle<>( borrowed( PyImport_AddModule( "__main__" ) ) ) );
			g_mainModuleNamespace = g_mainModule.attr( "__dict__" );

			// Get rid of the python signal handler that
			// turns Ctrl-C into that annoying KeyboardInterrupt
			// exception, and import IECore ready for use in
			// procInit().
			string toExecute =
				"import signal\n"
				"signal.signal( signal.SIGINT, signal.SIG_DFL )\n"
				"import IECore";

			handle<> ignored( PyRun_String(
				toExecute.c_str(),
				Py_file_input, g_mainModuleNamespace.ptr(),
				g_mainModuleNamespace.ptr() ) );
		}
		catch( const error_already_set &e )
		{
			PyErr_Print();
		}
		catch( const std::exception &e )
		{
			msg( Msg::Error, "ieProcedural initialiser", e.what() );
		}
		catch( ... )
		{
			msg( Msg::Error, "ieProcedural initialiser", "Caught unknown exception" );
		}

	PyEval_ReleaseThread( PyThreadState_Get() );
}

static int procInit( AtNode *node, void **userPtr )
{
	// load the class

	initialisePython();

	const char *className = AiNodeGetStr( node, "className" );
	int classVersion = AiNodeGetInt( node, "classVersion" );
	AtArray *parameterValues = AiNodeGetArray( node, "parameterValues" );

	ParameterisedProceduralPtr parameterisedProcedural = 0;
	ScopedGILLock gilLock;
	try
	{
		object ieCore = g_mainModuleNamespace["IECore"];
		object classLoader = ieCore.attr( "ClassLoader" ).attr( "defaultProceduralLoader" )();
		object procedural = classLoader.attr( "load" )( className, classVersion )();

		if( parameterValues )
		{
			boost::python::list toParse;
			for( unsigned i=0; i<parameterValues->nelements; i++ )
			{
				// hack to workaround ass parsing errors
				/// \todo Remove when we get the Arnold version that fixes this
				std::string s = AiArrayGetStr( parameterValues, i );
				for( size_t c = 0; c<s.size(); c++ )
				{
					if( s[c] == '@' )
					{
						s[c] = '#';
					}
				}

				toParse.append( s );
			}

			object parameterParser = ieCore.attr( "ParameterParser" )();
			parameterParser.attr( "parse" )( toParse, procedural.attr( "parameters" )() );
		}

		parameterisedProcedural = extract<ParameterisedProceduralPtr>( procedural );
	}
	catch( const error_already_set &e )
	{
		PyErr_Print();
	}
	catch( const std::exception &e )
	{
		msg( Msg::Error, "ieProcedural", e.what() );
	}
	catch( ... )
	{
		msg( Msg::Error, "ieProcedural", "Caught unknown exception" );
	}

	// render with it

	if( parameterisedProcedural )
	{
		IECoreArnold::RendererPtr renderer = new IECoreArnold::Renderer( node );
		parameterisedProcedural->render( renderer.get() );

		renderer->addRef();
		*userPtr = renderer.get();
	}
	else
	{
		*userPtr = 0;
	}

	return 1;
}

static int procCleanup( void *userPtr )
{
	IECoreArnold::Renderer *renderer = (IECoreArnold::Renderer *)( userPtr );
	if( renderer )
	{
		renderer->removeRef();
	}
	return 1;
}

static int procNumNodes( void *userPtr )
{
	IECoreArnold::Renderer *renderer = (IECoreArnold::Renderer *)( userPtr );
	return renderer ? renderer->numProceduralNodes() : 0;
}

static AtNode* procGetNode( void *userPtr, int i )
{
	IECoreArnold::Renderer *renderer = (IECoreArnold::Renderer *)( userPtr );
	return renderer ? (AtNode *)renderer->proceduralNode( i ) : 0;
}


AI_EXPORT_LIB int ProcLoader( AtProcVtable *vTable )
{

	vTable->Init = procInit;
	vTable->Cleanup = procCleanup;
	vTable->NumNodes = procNumNodes;
	vTable->GetNode = procGetNode;
	strcpy( vTable->version, AI_VERSION );

	return 1;
}
