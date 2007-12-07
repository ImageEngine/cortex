##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
#     * Neither the name of Image Engine Design nor the names of any
#       other contributors to this software may be used to endorse or
#       promote products derived from this software without specific prior
#       written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##########################################################################

import SCons
import glob
import sys
import os
import re

EnsureSConsVersion( 0, 97 )
SConsignFile()

ieCoreMajorVersion=2
ieCoreMinorVersion=22
ieCorePatchVersion=0

###########################################################################################
# Command line options
###########################################################################################

optionsFile = None
if "OPTIONS" in ARGUMENTS :
	optionsFile = ARGUMENTS["OPTIONS"]

o = Options( optionsFile, ARGUMENTS )

o.Add(
	"CXX",
	"The C++ compiler.",
	"g++",
)

# Boost options

o.Add(
	"BOOST_INCLUDE_PATH",
	"The path to the boost include directory.",
	"/usr/local/include/boost-1_34_1",
)

o.Add(
	"BOOST_LIB_PATH",
	"The path to the boost library directory.",
	"/usr/local/lib",
)

o.Add(
	"BOOST_LIB_SUFFIX",
	"The suffix appended to the names of the boost libraries. You can modify this "
	"to link against the debug versions of the libraries, or to link against "
	"unversioned libraries.",
	"-${BOOST_MAJOR_VERSION}_${BOOST_MINOR_VERSION}_${BOOST_PATCH_VERSION}",
)

# OpenEXR options

o.Add(
	"OPENEXR_INCLUDE_PATH",
	"The path to the OpenEXR include directory.",
	"/usr/local/include/OpenEXR",
)

o.Add(
	"OPENEXR_LIB_PATH",
	"The path to the OpenEXR lib directory.",
	"/usr/local/lib",
)

# JPEG options

o.Add(
	"JPEG_INCLUDE_PATH",
	"The path to the JPEG include directory.",
	"/usr/local/include/",
)

o.Add(
	"JPEG_LIB_PATH",
	"The path to the JPEG lib directory.",
	"/usr/local/lib",
)

# TIFF options

o.Add(
	"TIFF_INCLUDE_PATH",
	"The path to the TIFF include directory.",
	"/usr/local/include/",
)

o.Add(
	"TIFF_LIB_PATH",
	"The path to the TIFF lib directory.",
	"/usr/local/lib",
)

# SQLite options

o.Add(
	"SQLITE_INCLUDE_PATH",
	"The path to the SQLITE include directory.",
	"/usr/local/include/",
)

o.Add(
	"SQLITE_LIB_PATH",
	"The path to the SQLITE lib directory.",
	"/usr/local/lib",
)


# General path options

o.Add(
	"CPPPATH",
	"A colon separated list of paths to search for headers on.",
	"",
)

o.Add(
	"LIBPATH",
	"A colon separated list of paths to search for libraries on.",
	"/usr/lib",
)


# Python options

o.Add( 
	"PYTHON_CONFIG", 
	"The path to the python-config program for the Python installation you wish to "
	"build against. This is used to automatically obtain the appropriate compilation and linking "
	"flags, unless you override them with the other PYTHON_ flags. "
	"The default is to look for python-config on the path.",
	"python-config",""
)

o.Add( 
	"PYTHON",
	"The path to the python executable. If unspecified this will be discovered automatically using PYTHON_CONFIG.",
	""
)

o.Add( 
	"PYTHON_INCLUDE_PATH",
	"The path to the python headers. If unspecified this will be discovered automatically using PYTHON_CONFIG.",
	"",
)

o.Add(
	"PYTHON_LINK_FLAGS",
	"Flags to use when linking python modules. If unspecified this will be discovered automatically using PYTHON_CONFIG.",
	""
)

# Renderman options

o.Add(
	"RMAN_ROOT",
	"The directory in which your RenderMan renderer is installed.",
	"/usr/local/bin",
)

# OpenGL options

o.Add(
	BoolOption( "WITH_GL", "Set this to build the coreGL library in the contrib directory.", False ),
)

o.Add(
	"GLEW_INCLUDE_PATH",
	"The path to the directory with glew.h in it.",
	"/usr/local/include/GL",
)

o.Add(
	"GLEW_LIB_PATH",
	"The path to the directory with libGLEW in it.",
	"/usr/local/lib",
)

# Debug options

o.Add(
	BoolOption( "DEBUG", "Set this to build without optimisation and with debug symbols.", False ),
)

# Environment options

o.Add(
	"ENV_VARS_TO_IMPORT",
	"By default SCons ignores the environment it is run in, to avoid it contaminating the "
	"build process. This can be problematic if some of the environment is critical for "
	"running the applications used during the build. This space separated list of environment "
	"variables is imported to help overcome these problems.",
	"",
)

# Installation options

o.Add(
	"INSTALL_PREFIX",
	"The prefix under which to install things.",
	"/usr/local",
)

o.Add(
	"INSTALL_HEADER_DIR",
	"The directory in which to install headers.",
	"$INSTALL_PREFIX/include",
)

o.Add(
	"INSTALL_LIB_NAME",
	"The name under which to install the libraries.",
	"$INSTALL_PREFIX/lib/$IECORE_NAME",
)

o.Add(
	"INSTALL_PYTHON_DIR",
	"The directory in which to install python modules.",
	"$INSTALL_PREFIX/lib/python$PYTHON_VERSION/site-packages",
)

o.Add(
	"INSTALL_DOC_DIR",
	"The directory in which to install the documentation.",
	"$INSTALL_PREFIX/share/cortex",
)

# Test options

o.Add(
	"TEST_LIBRARY_PATH_ENV_VAR",
	"This is a curious one, probably only ever necessary at image engine. It "
	"specifies the name of an environment variable used to specify the library "
	"search paths correctly when running the tests. This should probably be left "
	"unspecified everywhere except image engine.",
	""
)

###########################################################################################
# An environment for building libraries
###########################################################################################

env = Environment(
	options = o
)

for e in env["ENV_VARS_TO_IMPORT"].split() :
	if e in os.environ :
		env["ENV"][e] = os.environ[e]

if "SAVE_OPTIONS" in ARGUMENTS :
	o.Save( ARGUMENTS["SAVE_OPTIONS"], env )

env["IECORE_MAJOR_VERSION"] = ieCoreMajorVersion
env["IECORE_MINOR_VERSION"] = ieCoreMinorVersion
env["IECORE_PATCH_VERSION"] = ieCorePatchVersion
env["IECORE_MAJORMINOR_VERSION"] = "${IECORE_MAJOR_VERSION}.${IECORE_MINOR_VERSION}"
env["IECORE_MAJORMINORPATCH_VERSION"] = "${IECORE_MAJOR_VERSION}.${IECORE_MINOR_VERSION}.${IECORE_PATCH_VERSION}"

env.Append(
	CPPFLAGS = [
    	"-DIE_CORE_MAJORVERSION=$IECORE_MAJOR_VERSION",
    	"-DIE_CORE_MINORVERSION=$IECORE_MINOR_VERSION",
    	"-DIE_CORE_PATCHVERSION=$IECORE_PATCH_VERSION",
	]
)

# update the include and lib paths
env.Prepend(
	CPPPATH = [
		"include",
		"$OPENEXR_INCLUDE_PATH",
		# we use "OpenEXR/x.h" and they use "x.h"
		os.path.join( "$OPENEXR_INCLUDE_PATH","OpenEXR" ),
		"$BOOST_INCLUDE_PATH",
		"$JPEG_INCLUDE_PATH",
		"$TIFF_INCLUDE_PATH",
		"$SQLITE_INCLUDE_PATH",
	],
	LIBPATH = [
		"$BOOST_LIB_PATH",
		"$OPENEXR_LIB_PATH",
		"$JPEG_LIB_PATH",
		"$TIFF_LIB_PATH",
		"$SQLITE_LIB_PATH",
	],
	LIBS = [
		"pthread",
	],
	CXXFLAGS = [
		"-Wall",
# removing the flag below till we can fix the warnings coming from boost
#		"-Werror",
	],
)

if env["PLATFORM"]=="darwin" :
	env.Append( CXXFLAGS = "-Wno-long-double" )

if env["DEBUG"] :
	env.Append( CXXFLAGS = "-g" )
else :
	env.Append( CXXFLAGS = [ "-O2", "-DNDEBUG" ] )
			
# autoconf-like checks for stuff.
# this part of scons doesn't seem so well thought out.

# unlike the rest of the scons build process, the configure stuff
# runs immediately. we don't want to run it in a few cases, as failures
# would mean that people don't get to do what they're actually trying to do
doConfigure = not "--help" in sys.argv and not "-h" in sys.argv and not env.GetOption( "clean" )

if doConfigure :

	c = Configure( env ) 
	
	if not c.CheckHeader( "boost/version.hpp", "\"\"", "C++" ) :
		sys.stderr.write( "ERROR : unable to find the boost headers, check BOOST_INCLUDE_PATH.\n" )
		Exit( 1 )
	
	# figure out the boost version in use so we can append it to the
	# library names	if necessary
	boostVersion = None
	boostVersionHeader = env.FindFile( "boost/version.hpp", env["CPPPATH"] )
	if (boostVersionHeader ==None):
		sys.stderr.write( "ERROR : unable to find the boost headers, check BOOST_INCLUDE_PATH.\n" )
		Exit( 1 )

	for line in open( str( boostVersionHeader ) ) :
		m = re.compile( "^#define BOOST_LIB_VERSION \"(.*)\"\s*$" ).match( line )
		if m  :
			boostVersion = m.group( 1 )
		if boostVersion :
			m = re.compile( "^([0-9]+)_([0-9]+)_([0-9]+)$" ).match( boostVersion )
			boostMajorVersion, boostMinorVersion, boostPatchVersion = m.group( 1, 2, 3 )
			env["BOOST_MAJOR_VERSION"] = boostMajorVersion
			env["BOOST_MINOR_VERSION"] = boostMinorVersion
			env["BOOST_PATCH_VERSION"] = boostPatchVersion
			break
			
	if not boostVersion :
		sys.stderr.write( "ERROR : unable to determine boost version from \"%s\".\n" % boostVersionHeader )
		Exit( 1 )

	if not c.CheckLibWithHeader( env.subst( "boost_filesystem" + env["BOOST_LIB_SUFFIX"] ), "boost/filesystem/path.hpp", "CXX" ) :
		sys.stderr.write( "ERROR : unable to find the boost libraries - check BOOST_LIB_PATH.\n" )
		Exit( 1 )

	if not c.CheckLibWithHeader( "Iex", "OpenEXR/ImfInputFile.h", "C++" ) :
		sys.stderr.write( "ERROR : unable to find the OpenEXR libraries - check OPENEXR_INCLUDE_PATH and OPENEXR_LIB_PATH.\n" )
		Exit( 1 )
		
	c.Finish()
		
env.Append( LIBS = [
		"boost_filesystem" + env["BOOST_LIB_SUFFIX"],
		"boost_regex" + env["BOOST_LIB_SUFFIX"],
		"boost_iostreams" + env["BOOST_LIB_SUFFIX"],
		"boost_date_time" + env["BOOST_LIB_SUFFIX"],
		"Half",
		"Iex",
		"Imath",
		"IlmImf",
		"IlmThread",
		"z",
	]
)

Help( o.GenerateHelpText( env ) )
	
###########################################################################################
# An environment for building python modules
###########################################################################################

def getPythonConfig( env, flags ) :

	f = os.popen( env["PYTHON_CONFIG"] + " " + flags )
	r = f.read().strip()
	if f.close() :
		sys.stderr.write( "ERROR : Error running \"%s\".\n" % env["PYTHON_CONFIG"] )
		Exit( 1 )
	return r

pythonEnv = env.Copy()

# decide where python is
if pythonEnv["PYTHON"]=="" :
	pythonEnv["PYTHON"] = getPythonConfig( pythonEnv, "--exec-prefix" ) + "/bin/python"

# try to run it to determine version
pythonExecutable = pythonEnv["PYTHON"]
try :
	pythonEnv["PYTHON_VERSION"] = os.popen( pythonExecutable + " -c 'import sys; print \"%s.%s\" % sys.version_info[:2]'" ).read().strip()
except :
	sys.stderr.write( "ERROR : Unable to determine python version from \"%s\".\n" % pythonExecutable )
	Exit( 1 )

# get the include path for python if we haven't been told it explicitly
if pythonEnv["PYTHON_INCLUDE_PATH"]=="" :
	pythonEnv.Append( CPPFLAGS=getPythonConfig( pythonEnv, "--includes" ).split() )
else :
	pythonEnv.Append( CPPPATH="$PYTHON_INCLUDE_PATH" )

# get the python link flags
if pythonEnv["PYTHON_LINK_FLAGS"]=="" :
	pythonEnv.Append( SHLINKFLAGS = getPythonConfig( pythonEnv, "--ldflags" ).split() )
else :
	pythonEnv.Append( SHLINKFLAGS = pythonEnv["PYTHON_LINK_FLAGS"].split() )

pythonEnv.Append( CPPFLAGS = "-DBOOST_PYTHON_MAX_ARITY=20" )
pythonEnv.Append( LIBS = [
		"boost_python" + pythonEnv["BOOST_LIB_SUFFIX"],
	]
)
pythonEnv.Append( LIBPATH = [ "./lib" ] )
pythonEnv["SHLIBPREFIX"] = ""
pythonEnv["SHLIBSUFFIX"] = ".so"

if pythonEnv["PLATFORM"]=="darwin" :
	pythonEnv.Append( SHLINKFLAGS = "-single_module" )
	
###########################################################################################
# An environment for running tests
###########################################################################################

if env["TEST_LIBRARY_PATH_ENV_VAR"]=="" :
	if env["PLATFORM"]=="darwin" :
		env["TEST_LIBRARY_PATH_ENV_VAR"] = "DYLD_LIBRARY_PATH"
	else :
		env["TEST_LIBRARY_PATH_ENV_VAR"] = "LD_LIBRARY_PATH"

testEnv = env.Copy()
testEnv["ENV"]["PYTHONPATH"] = "./python"

testEnv["ENV"][testEnv["TEST_LIBRARY_PATH_ENV_VAR"]] = ":".join( [ "./lib" ] + env["LIBPATH"] )

###########################################################################################
# Helper functions
###########################################################################################

# Makes versioned symlinks for use during installation
def makeSymlinks( env, target ) :

	def Link( target, source, env ) :
	
		target = str( target[0] )
		source = str( source[0] )
		
		commonPrefix = os.path.commonprefix( [ os.path.dirname( target ), os.path.dirname( source ) ] )
		relativeSource = source[len(commonPrefix)+1:]
	
		os.symlink( relativeSource, target )

	result = []
	links = {
		"$IECORE_MAJORMINORPATCH_VERSION" : "$IECORE_MAJORMINOR_VERSION",
		"$IECORE_MAJORMINOR_VERSION" : "$IECORE_MAJOR_VERSION",
	}
		
	done = False
	while not done :
	
		done = True
		for key in links.keys() :
		
			if target.find( key ) != -1 :
			
				linkName = target.replace( key, links[key] )
				link = env.Command( linkName, target, Link )
				
				result.append( link )
				target = linkName
				done = False	
	
	return result

# Makes versioned symlinks for the library an environment makes.
# This function is necessary as there's some name munging to get
# the right prefix and suffix on the library names.
def makeLibSymlinks( env ) :

	p = coreEnv["INSTALL_LIB_NAME"]
	d = os.path.dirname( p )
	n = os.path.basename( p )
	n = "$SHLIBPREFIX" + n + "$SHLIBSUFFIX"
	p = os.path.join( d, n )
	return makeSymlinks( env, p )
	

###########################################################################################
# Build, install and test the core library and bindings
###########################################################################################

coreEnv = env.Copy( IECORE_NAME="IECore" )
corePythonEnv = pythonEnv.Copy( IECORE_NAME="IECore" )

coreSources = glob.glob( "src/IECore/*.cpp" )
coreHeaders = glob.glob( "include/IECore/*.h" ) + glob.glob( "include/IECore/*.inl" )
coreBindingHeaders = glob.glob( "include/IECore/bindings/*.h" ) + glob.glob( "include/IECore/bindings/*.inl" )
corePythonSources = glob.glob( "src/IECore/bindings/*.cpp" )
corePythonScripts = glob.glob( "python/IECore/*.py" )

if doConfigure :

	c = Configure( coreEnv )

	if c.CheckLibWithHeader( "tiff", "tiff.h", "CXX" ) :
		c.env.Append( CPPFLAGS = '-DIECORE_WITH_TIFF' )
		corePythonEnv.Append( CPPFLAGS = '-DIECORE_WITH_TIFF' )
	else :
		sys.stderr.write( "WARNING: no TIFF library found, no TIFF support, check TIFF_INCLUDE_PATH and TIFF_LIB_PATH.\n" )
		coreSources.remove( "src/IECore/TIFFImageWriter.cpp" )
		coreSources.remove( "src/IECore/TIFFImageReader.cpp" )
		corePythonSources.remove( "src/IECore/bindings/TIFFImageReaderBinding.cpp" )
		corePythonSources.remove( "src/IECore/bindings/TIFFImageWriterBinding.cpp" )
		
	if c.CheckLibWithHeader( "jpeg", ["stdio.h", "jpeglib.h"], "CXX" ) :
		c.env.Append( CPPFLAGS = '-DIECORE_WITH_JPEG' )
		corePythonEnv.Append( CPPFLAGS = '-DIECORE_WITH_JPEG' )
	else :
		sys.stderr.write( "WARNING: no JPEG library found, no JPEG support, check JPEG_INCLUDE_PATH and JPEG_LIB_PATH.\n" )
		coreSources.remove( "src/IECore/JPEGImageWriter.cpp" )
		coreSources.remove( "src/IECore/JPEGImageReader.cpp" )
		corePythonSources.remove( "src/IECore/bindings/JPEGImageReaderBinding.cpp" )
		corePythonSources.remove( "src/IECore/bindings/JPEGImageWriterBinding.cpp" )
			
	if c.CheckLibWithHeader( "sqlite3", "sqlite/sqlite3.h", "CXX" ) :
		c.env.Append( CPPFLAGS = "-DIECORE_WITH_SQLITE" )
		corePythonEnv.Append( CPPFLAGS = '-DIECORE_WITH_SQLITE' )
	else :
		sys.stderr.write( "WARNING: no SQLITE library found, no SQLITE support, check SQLITE_INCLUDE_PATH and SQLITE_LIB_PATH\n" )
		coreSources.remove( "src/IECore/SQLiteIndexedIO.cpp" )

	c.Finish()

coreLibrary = coreEnv.SharedLibrary( "lib/" + os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ), coreSources )
coreLibraryInstall = coreEnv.Install( os.path.dirname( coreEnv.subst( "$INSTALL_LIB_NAME" ) ), coreLibrary )
coreLibrarySymlinks = makeLibSymlinks( coreEnv )
coreEnv.Alias( "install", [ coreLibraryInstall ] + coreLibrarySymlinks )

headerInstall = coreEnv.Install( "$INSTALL_HEADER_DIR/IECore", coreHeaders )
headerInstall += coreEnv.Install( "$INSTALL_HEADER_DIR/IECore/bindings", coreBindingHeaders )
headerSymlinks = makeSymlinks( coreEnv, coreEnv["INSTALL_HEADER_DIR"] )
coreEnv.Alias( "install", headerInstall + headerSymlinks )

corePythonEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )
corePythonModule = corePythonEnv.SharedLibrary( "python/IECore/_IECore", corePythonSources )
corePythonEnv.Depends( corePythonModule, coreLibrary )

corePythonModuleInstall = corePythonEnv.Install( "$INSTALL_PYTHON_DIR/IECore", corePythonScripts + corePythonModule )
corePythonModuleSymlinks = makeSymlinks( corePythonEnv, corePythonEnv["INSTALL_PYTHON_DIR"] )
pythonEnv.Alias( "install", corePythonModuleInstall + corePythonModuleSymlinks )

Default( coreLibrary, corePythonModule )

coreTest = testEnv.Command( "test/IECore/results.txt", corePythonModule, pythonExecutable + " test/IECore/All.py" )
testEnv.Alias( "coreTest", coreTest )

###########################################################################################
# Build, install and test the coreRI library and bindings
###########################################################################################

riEnv = env.Copy( IECORE_NAME = "IECoreRI" )
riEnv.Append( CPPPATH = [ "$RMAN_ROOT/include" ] )
riEnv.Append( LIBPATH = [ "$RMAN_ROOT/lib" ] )
riEnv.Append(
	CPPFLAGS = [
		## \todo This makes no sense now the versions for the two libraries are locked together
    	"-DIE_CORE_MAJOR_VERSION=%d" % ieCoreMajorVersion,
	]
)
riEnv.Append(
	LIBS = [
		"3delight",
	]
)

riPythonEnv = pythonEnv.Copy( IECORE_NAME = "IECoreRI" )
riPythonEnv.Append( LIBPATH = [ "$RMAN_ROOT/lib" ] )
riPythonEnv.Append(
	LIBS = [
		"3delight",
	]
)

if doConfigure :

	c = Configure( riEnv )

	if not c.CheckLibWithHeader( "3delight", "ri.h", "C" ) :
		sys.stderr.write( "WARNING : no 3delight library found, not building IECoreRI - check RMAN_ROOT.\n" )
		c.Finish()

	else :
	
		riSources = glob.glob( "src/IECoreRI/*.cpp" )
		riHeaders = glob.glob( "include/IECoreRI/*.h" ) + glob.glob( "include/IECoreRI/*.inl" )
		riPythonSources = glob.glob( "src/IECoreRI/bindings/*.cpp" )
		riPythonScripts = glob.glob( "python/IECoreRI/*.py" )
	
		if c.CheckHeader( "pointcloud.h" ) :
			
			riEnv.Append( CPPFLAGS = "-DIECORERI_WITH_PTC" )
			riPythonEnv.Append( CPPFLAGS = "-DIECORERI_WITH_PTC" )
			
		else :
		
			riSources.remove( "src/IECoreRI/PTCParticleReader.cpp" )
			riSources.remove( "src/IECoreRI/PTCParticleWriter.cpp" )
			riPythonSources.remove( "src/IECoreRI/bindings/PTCParticleReaderBinding.cpp" )
			riPythonSources.remove( "src/IECoreRI/bindings/PTCParticleWriterBinding.cpp" )
			
		if c.CheckFunc( "RiObjectBeginV" ) :
			
			riEnv.Append( CPPFLAGS = [ "-DIECORERI_WITH_OBJECTBEGINV" ] )
			
		c.Finish()	

		# we can't append this before configuring, as then it gets built as
		# part of the configure process
		riEnv.Prepend( LIBPATH = [ "./lib" ] )
		riEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )
	
		riLibrary = riEnv.SharedLibrary( "lib/" + os.path.basename( riEnv.subst( "$INSTALL_LIB_NAME" ) ), riSources )
		riLibraryInstall = riEnv.Install( os.path.dirname( riEnv.subst( "$INSTALL_LIB_NAME" ) ), riLibrary )
		riLibrarySymlinks = makeLibSymlinks( riEnv )
		riEnv.Alias( "install", riLibraryInstall + riLibrarySymlinks )

		riHeaderInstall = riEnv.Install( "$INSTALL_HEADER_DIR/IECoreRI", riHeaders )
		riHeaderSymlinks = makeSymlinks( riEnv, riEnv["INSTALL_HEADER_DIR"] )
		riEnv.Alias( "install", riHeaderInstall + riHeaderSymlinks )

		riPythonEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( riEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)
		riPythonModule = riPythonEnv.SharedLibrary( "python/IECoreRI/_IECoreRI", riPythonSources )
		riPythonEnv.Depends( riPythonModule, riLibrary )

		riPythonModuleInstall = riPythonEnv.Install( "$INSTALL_PYTHON_DIR/IECoreRI", riPythonScripts + riPythonModule )
		riPythonModuleSymlinks = makeSymlinks( riPythonEnv, riPythonEnv["INSTALL_PYTHON_DIR"] )
		riPythonEnv.Alias( "install", riPythonModuleInstall + riPythonModuleSymlinks )

		Default( [ riLibrary, riPythonModule ] )
		
		riTestEnv = testEnv.Copy()
		riTestEnv["ENV"][testEnv["TEST_LIBRARY_PATH_ENV_VAR"]] = riEnv.subst( ":".join( [ "./lib" ] + riPythonEnv["LIBPATH"] ) )
		riTestEnv["ENV"]["SHADER_PATH"] = riEnv.subst( "$RMAN_ROOT/shaders" )
		riTest = riTestEnv.Command( "test/IECoreRI/results.txt", riPythonModule, pythonExecutable + " test/IECoreRI/All.py" )
		riTestEnv.Depends( riTest, corePythonModule )
		riTestEnv.Alias( "riTest", riTest )

###########################################################################################
# Build, install and test the optional CoreGL library and bindings
###########################################################################################

# because coreGL isn't really stable in terms of api yet it has its own version
# number. when it moves out of /contrib it'll use the same version number as the
# main libraries.
ieCoreGLMajorVersion = 0
ieCoreGLMinorVersion = 3
ieCoreGLPatchVersion = 0

if env["WITH_GL"] :
	
	glEnvSets = {
		"IECORE_NAME" : "IECoreGL",
		"IECORE_MAJOR_VERSION" : ieCoreGLMajorVersion,
		"IECORE_MINOR_VERSION" : ieCoreGLMinorVersion,
		"IECORE_PATCH_VERSION" : ieCoreGLPatchVersion,
	}

	glEnvPrepends = {
		"CPPPATH" : [
			"contrib/IECoreGL/include",
		],
	}
	glEnvAppends = {
		
		"CPPPATH" : [
			"contrib/IECoreGL/include",
			"$GLEW_INCLUDE_PATH",
		],
		"CPPFLAGS" : [
			"-DIE_CORE_MAJOR_VERSION=%d" % ieCoreMajorVersion,
		],
		"LIBPATH" : [
			"$GLEW_LIB_PATH",
		],
		"LIBS" : [
			"GLEW",
		],
	
	}
	
	glEnv = env.Copy( **glEnvSets )

	glEnv.Append( **glEnvAppends )
	glEnv.Prepend( **glEnvPrepends )
	
	c = Configure( glEnv )
	
	## \todo We need to check for GLUT here too
	if not c.CheckLibWithHeader( "GLEW", "glew.h", "C" ) :
	
		sys.stderr.write( "WARNING : GLEW library not found, not building IECoreGL - check GLEW_INCLUDE_PATH and GLEW_LIB_PATH.\n" )
		c.Finish()
			
	else :	
	
		c.Finish()

		glEnv.Append( LIBS = [ coreLibrary ] )
	
		if env["PLATFORM"]=="darwin" :
			glEnv.Append(
				FRAMEWORKS = [
					"OpenGL",
					"GLUT",
				]
			)
		else :
			glEnv.Append(
				LIBS = [
					"GL",
					"GLU",
					"glut",
				]
			)

		glSources = glob.glob( "contrib/IECoreGL/src/*.cpp" )
		glLibrary = glEnv.SharedLibrary( "lib/" + os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ), glSources )
		glLibraryInstall = glEnv.Install( os.path.dirname( glEnv.subst( "$INSTALL_LIB_NAME" ) ), glLibrary )
		glLibrarySymlinks = makeLibSymlinks( glEnv )
		glEnv.Alias( "install", glLibraryInstall + glLibrarySymlinks )

		glHeaders = glob.glob( "contrib/IECoreGL/include/IECoreGL/*.h" ) + glob.glob( "contrib/IECoreGL/include/IECoreGL/*.inl" )
		glHeaderInstall = glEnv.Install( "$INSTALL_HEADER_DIR/IECoreGL", glHeaders )
		glHeaderSymlinks = makeSymlinks( glEnv, glEnv["INSTALL_HEADER_DIR"] )
		glEnv.Alias( "install", glHeaderInstall )

		glPythonEnv = pythonEnv.Copy( **glEnvSets )
		glPythonEnv.Append( **glEnvAppends )
		glPythonEnv.Prepend( **glEnvPrepends )
		glPythonEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)
		glPythonSources = glob.glob( "contrib/IECoreGL/src/bindings/*.cpp" )
		glPythonModule = glPythonEnv.SharedLibrary( "contrib/IECoreGL/python/IECoreGL/_IECoreGL", glPythonSources )
		glPythonEnv.Depends( glPythonModule, glLibrary )

		glPythonScripts = glob.glob( "contrib/IECoreGL/python/IECoreGL/*.py" )
		glPythonModuleInstall = glPythonEnv.Install( "$INSTALL_PYTHON_DIR/IECoreGL", glPythonScripts + glPythonModule )
		glPythonModuleSymlinks = makeSymlinks( glPythonEnv, glPythonEnv["INSTALL_PYTHON_DIR"] )
		glPythonEnv.Alias( "install", glPythonModuleInstall + glPythonModuleSymlinks )

		Default( [ glLibrary, glPythonModule ] )
		
###########################################################################################
# Documentation
###########################################################################################

## \todo Have an option for people to point us to their doxygen install, and do
# Configure checks to be sure it's there
docEnv = env.Copy()
docEnv["ENV"]["PATH"] = os.environ["PATH"]
docs = docEnv.Command( "doc/html/index.html", "", "doxygen doc/config/Doxyfile" )
docEnv.Depends( docs, glob.glob( "src/IECore/*.cpp" ) )
docEnv.Depends( docs, glob.glob( "src/IECoreRI/*.cpp" ) )
docEnv.Depends( docs, glob.glob( "python/IECore/*.py" ) )
docEnv.Depends( docs, glob.glob( "python/IECoreRI/*.py" ) )

installDoc = docEnv.Install( "$INSTALL_DOC_DIR", "doc/html" )
docEnv.Alias( "install", installDoc )
