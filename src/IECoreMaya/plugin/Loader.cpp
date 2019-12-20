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

/// \todo: Consider dropping this loader mechanism entirely:
/// https://mayastation.typepad.com/maya-station/2012/02/global-symbol-evaluation.html
#define STR2(a) #a
#define STR(a) STR2(a)
#if MAYA_API_VERSION >= 20190000
	#define IECOREMAYA_INITIALIZE_PLUGIN_SYMBOL "_Z16initializePluginN8Autodesk4Maya16OpenMaya" STR(MAYA_APP_VERSION) "00007MObjectE"
	#define IECOREMAYA_UNINITIALIZE_PLUGIN_SYMBOL "_Z18uninitializePluginN8Autodesk4Maya16OpenMaya" STR(MAYA_APP_VERSION) "00007MObjectE"
#elif MAYA_API_VERSION >= 20180000
	#define IECOREMAYA_INITIALIZE_PLUGIN_SYMBOL "_Z16initializePluginN8Autodesk4Maya16OpenMaya201800007MObjectE"
	#define IECOREMAYA_UNINITIALIZE_PLUGIN_SYMBOL "_Z18uninitializePluginN8Autodesk4Maya16OpenMaya201800007MObjectE"
#else
	#define IECOREMAYA_INITIALIZE_PLUGIN_SYMBOL "_Z16initializePlugin7MObject"
	#define IECOREMAYA_UNINITIALIZE_PLUGIN_SYMBOL "_Z18uninitializePlugin7MObject"
#endif

IECORE_EXPORT MStatus initializePlugin( MObject obj )
{
	assert( !g_libraryHandle );
	MFnPlugin plugin(obj, "Image Engine", "1.0");

	std::string pluginName = plugin.name().asChar();
	std::string pluginPath = plugin.loadPath().asChar();

	std::string implName = pluginPath + "/impl/" + pluginName + ".so";

 	g_libraryHandle = dlopen( implName.c_str(), RTLD_NOW | RTLD_GLOBAL );

	if (! g_libraryHandle )
	{
		printf("Failed to load '%s':\n%s\n", implName.c_str(), dlerror());
		return MS::kFailure;
	}

	void *initializeSymbol = dlsym( g_libraryHandle, IECOREMAYA_INITIALIZE_PLUGIN_SYMBOL );
	if ( ! initializeSymbol )
	{
		printf( "Unable to find symbol: %s\n%s\n", IECOREMAYA_INITIALIZE_PLUGIN_SYMBOL, dlerror() );
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

	void *uninitializeSymbol = dlsym( g_libraryHandle, IECOREMAYA_UNINITIALIZE_PLUGIN_SYMBOL );
	if ( !uninitializeSymbol )
	{
		printf( "Unable to find symbol: %s\n%s\n", IECOREMAYA_UNINITIALIZE_PLUGIN_SYMBOL, dlerror() );
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
