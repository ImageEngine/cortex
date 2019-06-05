//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include <cassert>
#include <dlfcn.h>

#include "maya/MFnPlugin.h"

#include "IECore/Export.h"

static void *g_libraryHandle = 0;

IECORE_EXPORT MStatus initializePlugin( MObject obj )
{
	assert( !g_libraryHandle );
	MFnPlugin plugin(obj, "Image Engine", "1.0");

	std::string pluginName = plugin.name().asChar();
	std::string pluginPath = plugin.loadPath().asChar();

	std::string implName = pluginPath + "/impl/" + pluginName + ".so";

	const char *forceGlobals = std::getenv( "IECORE_FORCE_GLOBAL_SYMBOLS" );
	if( forceGlobals && !strcmp( forceGlobals, "1" ) )
	{
		g_libraryHandle = dlopen( implName.c_str(), RTLD_NOW | RTLD_GLOBAL );
	}
	else
	{
		g_libraryHandle = dlopen( implName.c_str(), RTLD_NOW );
	}

	if (! g_libraryHandle )
	{
		printf("Failed to load '%s':\n%s\n", implName.c_str(), dlerror());
		return MS::kFailure;
	}

#if MAYA_API_VERSION >= 201800
	const char *symbolName( "_Z16initializePluginN8Autodesk4Maya16OpenMaya201800007MObjectE" );
#else
	const char *symbolName( "_Z16initializePlugin7MObject" );
#endif

	void *initializeSymbol = dlsym( g_libraryHandle, symbolName );
	if ( ! initializeSymbol )
	{
		printf( "Unable to find symbol: %s\n%s\n", symbolName, dlerror() );
		dlclose( g_libraryHandle );
		g_libraryHandle = 0;
		return MS::kFailure;
	}

	typedef MStatus (*InitializePluginFn)( MObject );

	InitializePluginFn initializePluginImpl = (InitializePluginFn)( initializeSymbol );

	return initializePluginImpl( obj );
}

IECORE_EXPORT MStatus uninitializePlugin( MObject obj )
{
	MFnPlugin plugin(obj);
	assert( g_libraryHandle );

#if MAYA_API_VERSION >= 201800
	const char *symbolName( "_Z18uninitializePluginN8Autodesk4Maya16OpenMaya201800007MObjectE" );
#else
	const char *symbolName( "_Z18uninitializePlugin7MObject" );
#endif
	void *uninitializeSymbol = dlsym( g_libraryHandle, symbolName );

	if ( !uninitializeSymbol )
	{
		printf( "Unable to find symbol: %s\n%s\n", symbolName, dlerror() );
		dlclose( g_libraryHandle );
		g_libraryHandle = 0;
		return MS::kFailure;
	}

	typedef MStatus (*UninitializePluginFn)( MObject );
	UninitializePluginFn uninitializePluginImpl = (UninitializePluginFn)( uninitializeSymbol );

	MStatus s = uninitializePluginImpl( obj );
	dlclose( g_libraryHandle );
	g_libraryHandle = 0;
	return s;
}
