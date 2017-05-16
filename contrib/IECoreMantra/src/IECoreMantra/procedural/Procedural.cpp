//////////////////////////////////////////////////////////////////////////
//
//  Copyright 2012 Electric Theatre Collective Limited. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//	 * Redistributions of source code must retain the above copyright
//	   notice, this list of conditions and the following disclaimer.
//
//	 * Redistributions in binary form must reproduce the above copyright
//	   notice, this list of conditions and the following disclaimer in the
//	   documentation and/or other materials provided with the distribution.
//
//	 * Neither the name of Image Engine Design nor the names of any
//	   other contributors to this software may be used to endorse or
//	   promote products derived from this software without specific prior
//	   written permission.
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

#include <UT/UT_WorkArgs.h>
#include <UT/UT_String.h>
#include <GU/GU_Detail.h>

#include "IECore/MessageHandler.h"
#include "IECore/ParameterisedProcedural.h"
#include "IECorePython/ScopedGILLock.h"

#include "IECoreHoudini/Convert.h"
#include "IECoreMantra/Renderer.h"
#include "IECoreMantra/ProceduralPrimitive.h"

#include <iostream>

using namespace std;
using namespace boost::python;
using namespace IECore;
using namespace IECorePython;
using namespace IECoreMantra;

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

class VRAY_ieProcedural : public ProceduralPrimitive {
public:
	VRAY_ieProcedural();
	virtual ~VRAY_ieProcedural();

#if UT_MAJOR_VERSION_INT >= 14

	virtual const char *className() const;

#else

	virtual const char  *getClassName();

#endif

	virtual int	  initialize(const UT_BoundingBox *);
	virtual void	 getBoundingBox(UT_BoundingBox &box);
	virtual void	 render();
#if UT_MAJOR_VERSION_INT >= 16
	UT_StringHolder m_className;
	int64 m_classVersion;
	UT_StringHolder m_parameterString;
#else 
	UT_String m_className;
	int m_classVersion;
	UT_String m_parameterString;
#endif

};

static VRAY_ProceduralArg theArgs[] = {
	VRAY_ProceduralArg("className", "string", "read"), 
	VRAY_ProceduralArg("classVersion", "int", "1"),
	VRAY_ProceduralArg("parameterString", "string", ""),
	VRAY_ProceduralArg()
};

VRAY_Procedural *
allocProcedural(const char *)
{
   return new VRAY_ieProcedural();
}

const VRAY_ProceduralArg *
getProceduralArgs(const char *)
{
	return theArgs;
}

VRAY_ieProcedural::VRAY_ieProcedural()
{
}

VRAY_ieProcedural::~VRAY_ieProcedural()
{
}

#if UT_MAJOR_VERSION_INT >= 14

const char *VRAY_ieProcedural::className() const
{
	return "VRAY_ieProcedural";
}

#else

const char *VRAY_ieProcedural::getClassName()
{
	return "VRAY_ieProcedural";
}

#endif

// The initialize method is called when the procedural is created. 
// Returning zero (failure) will abort the rendering of this procedural.
// The bounding box passed in is the user defined bounding box. 
// If the user didn't specify a bounding box, then the box will be NULL
int
VRAY_ieProcedural::initialize(const UT_BoundingBox *box)
{
	if ( box )
    {
		m_bound = convert<Imath::Box3f> ( *box );
	}
	import("className", m_className);
	import("classVersion", &m_classVersion, 1);
	import("parameterString", m_parameterString);
	
#if UT_MAJOR_VERSION_INT >= 16
	size_t size;
	const UT_StringHolder * classNameValue = getSParm("className", size);
	const int64 *classVersion = getIParm("classVersion", size);
	const UT_StringHolder * parameterString = getSParm("parameterString", size);
#else
	const char **classNameValue = getSParm("className");
	const int *classVersion = getIParm("classVersion");
	const char **parameterString = getSParm("parameterString");
#endif
	if (classNameValue) 
    {
		m_className = UT_StringHolder(*classNameValue);
	}
	if (classVersion) 
    {
		m_classVersion = *classVersion;
	}
	if (parameterString) 
    {
		m_parameterString = UT_StringHolder(*parameterString);
	}
	return 1;	
}

void
VRAY_ieProcedural::getBoundingBox(UT_BoundingBox &box)
{
	box = convert<UT_BoundingBox>( m_bound );
}

// When mantra determines that the bounding box needs to be rendered, the 
// render method is called. At this point, the procedural can either 
// generate geometry (VRAY_Procedural::openGeometryObject()) or it can 
// generate further procedurals (VRAY_Procedural::openProceduralObject()).
void
VRAY_ieProcedural::render()
{
	initialisePython();
	ParameterisedProceduralPtr parameterisedProcedural = 0;
	ScopedGILLock giLock;
	try
	{
		object ieCore = g_mainModuleNamespace["IECore"];
		object classLoader = ieCore.attr( "ClassLoader" ).attr( "defaultProceduralLoader" )();
		object procedural = classLoader.attr( "load" )( m_className.buffer(), m_classVersion )();
		boost::python::list params;
		UT_WorkArgs argv;
		UT_String parameterString = m_parameterString.c_str();
		if (parameterString.tokenize(argv, ","))
		{
            // The Cortex Mantra Inject otl parses the parameters of a 
            // SOP_ProceduralHolder and replaces empty values with a '!' 
            // character to make them easier to parse here.
            // 
            // This hack is upsetting, a pure python procedural in the style 
            // of IECoreRI iePython.dso could parse these parameters correctly
            // like python/IECoreRI/ExecuteProcedural.py
			for (int i = 0; i < argv.getArgc(); i++) 
			{
				std::string s(argv[i]);
				if (s == "!")
				{
					params.append(std::string(""));
				}
				else
				{
					params.append(s);
				}
			}
		}
		object parameterParser = ieCore.attr( "ParameterParser" )();
		parameterParser.attr( "parse" )( params, procedural.attr( "parameters" )() );
		parameterisedProcedural = extract<ParameterisedProceduralPtr>( procedural );
	}
	catch( const error_already_set &e )
	{
		PyErr_Print();
	}
	catch( const std::exception &e )
	{
		msg( Msg::Error, "VRAY_ieProcedural", e.what() );
	}
	catch( ... )
	{
		msg( Msg::Error, "VRAY_ieProcedural", "Caught unknown exception" );
	}
	if( parameterisedProcedural )
	{
		IECoreMantra::RendererPtr renderer = new IECoreMantra::Renderer( this );
		parameterisedProcedural->render( renderer.get(), false, false, true, true ); 
	}
}

