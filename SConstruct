##########################################################################
#
#  Copyright (c) 2007-2008, Image Engine Design Inc. All rights reserved.
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
ieCoreMinorVersion=30
ieCorePatchVersion=0

###########################################################################################
# Command line options
###########################################################################################

optionsFile = None

if "CORTEX_OPTIONS_FILE" in os.environ :
	optionsFile = os.environ["CORTEX_OPTIONS_FILE"]
	
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
	BoolOption( "WITH_SQLITE", "Set this to build support for SQLiteIndexedIO.", False ),
)

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

# Nuke options

o.Add(
	"NUKE_ROOT",
	"The directory in which Nuke is installed.",
	"/usr/local/foundry/nuke"
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

# Maya options

o.Add(
	BoolOption( "WITH_MAYA", "Set this to build the coreMaya library in the contrib directory.", False ),
)

o.Add(
	"MAYA_ROOT",
	"The path to the root of the maya installation.",
	"/usr/aw/maya",
)

# Debug options

o.Add(
	BoolOption( "DEBUG", "Set this to build without optimisation and with debug symbols.", False ),
)

# Build options

o.Add(
	"BUILD_CACHEDIR",
	"Specify a directory for SCons to cache build results in. This allows the sharing of build results"
	"among multiple developers and can significantly reduce build times, particularly when switching"
	"between multiple compilers and build options.",
	""
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
	"INSTALL_GLSL_HEADER_DIR",
	"The directory in which to install GLSL headers.",
	"$INSTALL_PREFIX/glsl",
)

o.Add(
	"INSTALL_DOC_DIR",
	"The directory in which to install the documentation.",
	"$INSTALL_PREFIX/share/cortex",
)

# Test options

o.Add(
	"TEST_CORE_SCRIPT",
	"The python script to run for the core tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"test/IECore/All.py"
)

o.Add(
	"TEST_LIBPATH",
	"Additional colon separated paths to be prepended to the library path"
	"used when running tests.",
	""
)

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

env["LIBPATH"] = env["LIBPATH"].split( ":" )

for e in env["ENV_VARS_TO_IMPORT"].split() :
	if e in os.environ :
		env["ENV"][e] = os.environ[e]

if "SAVE_OPTIONS" in ARGUMENTS :
	o.Save( ARGUMENTS["SAVE_OPTIONS"], env )

if env["BUILD_CACHEDIR"] != "" :
	CacheDir( env["BUILD_CACHEDIR"] )

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
		"-pipe",
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
	pythonEnv["PYTHON_INCLUDE_FLAGS"] = getPythonConfig( pythonEnv, "--includes" ).split()
else :
	pythonEnv["PYTHON_INCLUDE_FLAGS"] = "-I$PYTHON_INCLUDE_PATH"
pythonEnv.Append( CPPFLAGS="$PYTHON_INCLUDE_FLAGS" )

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
pythonEnv.Prepend( LIBPATH = [ "./lib" ] )
pythonEnv["SHLIBPREFIX"] = ""
pythonEnv["SHLIBSUFFIX"] = ".so"

if pythonEnv["PLATFORM"]=="darwin" :
	pythonEnv.Append( SHLINKFLAGS = "-single_module" )
	
###########################################################################################
# An environment for running tests
###########################################################################################

testEnv = env.Copy()

testEnv.Prepend( LIBPATH = [ "./lib" ] )

testEnvLibPath = ":".join( testEnv["LIBPATH"] )
if testEnv["TEST_LIBPATH"] != "" :
	testEnvLibPath += ":" + testEnv["TEST_LIBPATH"]

if testEnv["TEST_LIBRARY_PATH_ENV_VAR"]!="" :
	testEnv["ENV"][testEnv["TEST_LIBRARY_PATH_ENV_VAR"]] = testEnvLibPath

if env["PLATFORM"]=="darwin" :
	testEnv["ENV"]["DYLD_LIBRARY_PATH"] = testEnvLibPath
	
	# Special workaround for suspected gcc issue - see BoostUnitTestTest for more information
	if not testEnv["DEBUG"] :
		testEnv.Append( CXXFLAGS = "-O0" )
else :
	testEnv["ENV"]["LD_LIBRARY_PATH"] = testEnvLibPath

testEnv.Append( LIBS=["boost_unit_test_framework"] )

testEnv["ENV"]["PYTHONPATH"] = "./python"

###########################################################################################
# Helper functions
###########################################################################################
	
# Makes versioned symlinks for use during installation
def makeSymLinks( env, target ) :

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
				
				makeSymLink( env.subst( linkName ), env.subst( target ) )
				
				target = linkName
				done = False	

# Makes versioned symlinks for the library an environment makes.
# This function is necessary as there's some name munging to get
# the right prefix and suffix on the library names.
def makeLibSymLinks( env ) :

	p = coreEnv["INSTALL_LIB_NAME"]
	d = os.path.dirname( p )
	n = os.path.basename( p )
	n = "$SHLIBPREFIX" + n + "$SHLIBSUFFIX"
	p = os.path.join( d, n )
	makeSymLinks( env, p )

# Make a symlink pointing from target to source
def makeSymLink( target, source ) :

	commonPrefix = os.path.commonprefix( [ os.path.dirname( target ), os.path.dirname( source ) ] )
	relativeSource = source[len(commonPrefix)+1:]

	if os.path.exists( target ) :
		if os.path.islink( target ) :
			os.remove( target )
		else :
			# there's not a good reason for a non-link file to be here
			# so we won't delete it on the grounds that we don't know
			# what's happening
			sys.stderr.write( "Wanted to make a link at \"%s\", but a file exists there - aborting.\n" % target )
			Exit( 1 )

	os.symlink( relativeSource, target )
	
###########################################################################################
# Build, install and test the core library and bindings
###########################################################################################

coreEnv = env.Copy( IECORE_NAME="IECore" )
corePythonEnv = pythonEnv.Copy( IECORE_NAME="IECore" )
coreTestEnv = testEnv.Copy()

# lists of sources
coreSources = sorted( glob.glob( "src/IECore/*.cpp" ) )
coreHeaders = glob.glob( "include/IECore/*.h" ) + glob.glob( "include/IECore/*.inl" )
coreBindingHeaders = glob.glob( "include/IECore/bindings/*.h" ) + glob.glob( "include/IECore/bindings/*.inl" )
corePythonSources = sorted( glob.glob( "src/IECore/bindings/*.cpp" ) )
corePythonScripts = glob.glob( "python/IECore/*.py" )

# configure checks
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
			
	if coreEnv["WITH_SQLITE"] and c.CheckLibWithHeader( "sqlite3", "sqlite/sqlite3.h", "CXX" ) :
		c.env.Append( CPPFLAGS = "-DIECORE_WITH_SQLITE" )
		corePythonEnv.Append( CPPFLAGS = '-DIECORE_WITH_SQLITE' )
		coreTestEnv.Append( CPPFLAGS = '-DIECORE_WITH_SQLITE' )
	else :
		coreSources.remove( "src/IECore/SQLiteIndexedIO.cpp" )
		if coreEnv["WITH_SQLITE"] :
			sys.stderr.write( "WARNING: no SQLITE library found, no SQLITE support, check SQLITE_INCLUDE_PATH and SQLITE_LIB_PATH\n" )
		
	c.Finish()

# This is a simple mechanism to ensure that all of the installs get performed only after all of the builds
# have been done. We make this coreInstallSync object depend on every build action, and every install action
# depends on the coreInstallSync object. The reason we want to do this is so that, for example, the time between a 
# library being installed and its corresponding python binding library being installed is minimised. If we did
# not do this then there could be a potentially long period of time when the two co-dependent libraries won't
# link together at runtime due to mismatched versions.
#
# NB ":" is the "null" command
coreInstallSync = coreEnv.Command( "coreInstallSync.out", "", ":" )
NoCache( coreInstallSync )

# library
coreLibrary = coreEnv.SharedLibrary( "lib/" + os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ), coreSources )
coreEnv.Depends( coreInstallSync, coreLibrary )
coreLibraryInstall = coreEnv.Install( os.path.dirname( coreEnv.subst( "$INSTALL_LIB_NAME" ) ), coreLibrary )
coreEnv.Depends( coreLibraryInstall, coreInstallSync )
coreEnv.AddPostAction( coreLibraryInstall, lambda target, source, env : makeLibSymLinks( coreEnv ) )
coreEnv.Alias( "install", [ coreLibraryInstall ] )
coreEnv.Alias( "installCore", [ coreLibraryInstall ] )

# headers
headerInstall = coreEnv.Install( "$INSTALL_HEADER_DIR/IECore", coreHeaders )
headerInstall += coreEnv.Install( "$INSTALL_HEADER_DIR/IECore/bindings", coreBindingHeaders )
coreEnv.Depends( headerInstall, coreInstallSync )
coreEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECore", lambda target, source, env : makeSymLinks( coreEnv, coreEnv["INSTALL_HEADER_DIR"] ) )
coreEnv.Alias( "install", headerInstall )
coreEnv.Alias( "installCore", headerInstall )

# python module
corePythonEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )
corePythonModule = corePythonEnv.SharedLibrary( "python/IECore/_IECore", corePythonSources )
corePythonEnv.Depends( corePythonModule, coreLibrary )
corePythonEnv.Depends( coreInstallSync, corePythonModule )

corePythonModuleInstall = corePythonEnv.Install( "$INSTALL_PYTHON_DIR/IECore", corePythonScripts + corePythonModule )
coreEnv.Depends( corePythonModuleInstall, coreInstallSync )
corePythonEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECore", lambda target, source, env : makeSymLinks( corePythonEnv, corePythonEnv["INSTALL_PYTHON_DIR"] ) )
corePythonEnv.Alias( "install", corePythonModuleInstall )
corePythonEnv.Alias( "installCore", corePythonModuleInstall )
corePythonEnv.Depends( corePythonModuleInstall, coreInstallSync )

Default( coreLibrary, corePythonModule )

# testing

coreTestEnv.Append(
	LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
	CPPPATH = [ "test/IECore" ],
)

coreTestProgram = coreTestEnv.Program( "test/IECore/IECoreTest", glob.glob( "test/IECore/*.cpp" ) )

coreTest = coreTestEnv.Command( "test/IECore/results.txt", coreTestProgram, "test/IECore/IECoreTest >& test/IECore/results.txt" )
NoCache( coreTest )
coreTestEnv.Alias( "testCore", coreTest )

corePythonTest = coreTestEnv.Command( "test/IECore/resultsPython.txt", corePythonModule, pythonExecutable + " $TEST_CORE_SCRIPT" )
coreTestEnv.Depends( corePythonTest, glob.glob( "test/IECore/*.py" ) )
NoCache( corePythonTest )
coreTestEnv.Alias( "testCorePython", corePythonTest )

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
		riEnv.Depends( coreInstallSync, riLibrary )
		riLibraryInstall = riEnv.Install( os.path.dirname( riEnv.subst( "$INSTALL_LIB_NAME" ) ), riLibrary )
		riEnv.Depends( riLibraryInstall, coreInstallSync )		
		riEnv.AddPostAction( riLibraryInstall, lambda target, source, env : makeLibSymLinks( riEnv ) )
		riEnv.Alias( "install", riLibraryInstall )
		riEnv.Alias( "installRI", riLibraryInstall )

		riHeaderInstall = riEnv.Install( "$INSTALL_HEADER_DIR/IECoreRI", riHeaders )
		riEnv.Depends( riHeaderInstall, coreInstallSync )
		riEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreRI", lambda target, source, env : makeSymLinks( riEnv, riEnv["INSTALL_HEADER_DIR"] ) )
		riEnv.Alias( "install", riHeaderInstall )
		riEnv.Alias( "installRI", riHeaderInstall )

		riPythonEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( riEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)
		riPythonModule = riPythonEnv.SharedLibrary( "python/IECoreRI/_IECoreRI", riPythonSources )
		riPythonEnv.Depends( coreInstallSync, riPythonModule )		
		riPythonEnv.Depends( riPythonModule, riLibrary )

		riPythonModuleInstall = riPythonEnv.Install( "$INSTALL_PYTHON_DIR/IECoreRI", riPythonScripts + riPythonModule )
		riPythonEnv.Depends( riPythonModuleInstall, coreInstallSync )
		riPythonEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreRI", lambda target, source, env : makeSymLinks( riPythonEnv, riPythonEnv["INSTALL_PYTHON_DIR"] ) )
		riPythonEnv.Alias( "install", riPythonModuleInstall )
		riPythonEnv.Alias( "installRI", riPythonModuleInstall )

		Default( [ riLibrary, riPythonModule ] )
		
		riTestEnv = testEnv.Copy()
		riTestEnv["ENV"][testEnv["TEST_LIBRARY_PATH_ENV_VAR"]] = riEnv.subst( ":".join( [ "./lib" ] + riPythonEnv["LIBPATH"] ) )
		riTestEnv["ENV"]["SHADER_PATH"] = riEnv.subst( "$RMAN_ROOT/shaders" )
		riTest = riTestEnv.Command( "test/IECoreRI/results.txt", riPythonModule, pythonExecutable + " test/IECoreRI/All.py" )
		NoCache( riTest )
		riTestEnv.Depends( riTest, corePythonModule )
		riTestEnv.Depends( riTest, glob.glob( "test/IECoreRI/*.py" ) )
		riTestEnv.Alias( "testRI", riTest )

###########################################################################################
# Build, install and test the optional CoreGL library and bindings
###########################################################################################

# because coreGL isn't really stable in terms of api yet it has its own version
# number. when it moves out of /contrib it'll use the same version number as the
# main libraries.
ieCoreGLMajorVersion = 0
ieCoreGLMinorVersion = 6
ieCoreGLPatchVersion = 0

if env["WITH_GL"] and doConfigure :

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
		"LIBPATH" : [
			"./lib",
		]
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
			"boost_wave",
		],
	}
	
	glEnv = env.Copy( **glEnvSets )

	glEnv.Append( **glEnvAppends )
	glEnv.Prepend( **glEnvPrepends )
	
	c = Configure( glEnv )
	
	## \todo We need to check for GLUT here too
	if not c.CheckLibWithHeader( "GLEW", "glew.h", "CXX" ) :
	
		sys.stderr.write( "WARNING : GLEW library not found, not building IECoreGL - check GLEW_INCLUDE_PATH and GLEW_LIB_PATH.\n" )
		c.Finish()
			
	else :
	
		c.Finish()

		# we can't add this earlier as then it's built during the configure stage, and that's no good
		glEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )
	
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
		glEnv.Depends( coreInstallSync, glLibrary )
		glLibraryInstall = glEnv.Install( os.path.dirname( glEnv.subst( "$INSTALL_LIB_NAME" ) ), glLibrary )
		glEnv.Depends( glLibraryInstall, coreInstallSync )
		glEnv.AddPostAction( glLibraryInstall, lambda target, source, env : makeLibSymLinks( glEnv ) )
		glEnv.Alias( "install", glLibraryInstall )
		glEnv.Alias( "installGL", glLibraryInstall )

		glHeaders = glob.glob( "contrib/IECoreGL/include/IECoreGL/*.h" ) + glob.glob( "contrib/IECoreGL/include/IECoreGL/*.inl" )
		glHeaderInstall = glEnv.Install( "$INSTALL_HEADER_DIR/IECoreGL", glHeaders )
		glEnv.Depends( glHeaderInstall, coreInstallSync )
		glEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreGL", lambda target, source, env : makeSymLinks( glEnv, glEnv["INSTALL_HEADER_DIR"] ) )
		glEnv.Alias( "install", glHeaderInstall )
		glEnv.Alias( "installGL", glHeaderInstall )
		
		glslHeaders = glob.glob( "contrib/IECoreGL/glsl/IECoreGL/*.h" )
		glslHeaderInstall = glEnv.Install( "$INSTALL_GLSL_HEADER_DIR/IECoreGL", glslHeaders )
		glEnv.AddPostAction( "$INSTALL_GLSL_HEADER_DIR/IECoreGL", lambda target, source, env : makeSymLinks( glEnv, glEnv["INSTALL_GLSL_HEADER_DIR"] ) )
		glEnv.Alias( "install", glslHeaderInstall )
		glEnv.Alias( "installGL", glslHeaderInstall )
		
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
		glPythonEnv.Depends( coreInstallSync, glPythonModule )
		glPythonEnv.Depends( glPythonModule, glLibrary )

		glPythonScripts = glob.glob( "contrib/IECoreGL/python/IECoreGL/*.py" )
		glPythonModuleInstall = glPythonEnv.Install( "$INSTALL_PYTHON_DIR/IECoreGL", glPythonScripts + glPythonModule )		
		glPythonEnv.Depends( glPythonModuleInstall, coreInstallSync )
		glPythonEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreGL", lambda target, source, env : makeSymLinks( glPythonEnv, glPythonEnv["INSTALL_PYTHON_DIR"] ) )
		glPythonEnv.Alias( "install", glPythonModuleInstall )
		glPythonEnv.Alias( "installGL", glPythonModuleInstall )

		Default( [ glLibrary, glPythonModule ] )

###########################################################################################
# Build, install and test the optional coreMaya library and bindings
###########################################################################################

# because coreMaya isn't really stable in terms of api yet it has its own version
# number. when it moves out of /contrib it'll use the same version number as the
# main libraries.
ieCoreMayaMajorVersion = 2
ieCoreMayaMinorVersion = 13
ieCoreMayaPatchVersion = 0

if env["WITH_MAYA"] :


	mayaEnvSets = {
		"IECORE_NAME" : "IECoreMaya",
		"IECORE_MAJOR_VERSION" : ieCoreMayaMajorVersion,
		"IECORE_MINOR_VERSION" : ieCoreMayaMinorVersion,
		"IECORE_PATCH_VERSION" : ieCoreMayaPatchVersion,
	}

	mayaEnvAppends = {
		"CPPPATH" : [
			"$MAYA_ROOT/include",
			"contrib/IECoreMaya/include",
			"contrib/IECoreGL/include",
			"$GLEW_INCLUDE_PATH",
		],
		"LIBPATH" : [ "$MAYA_ROOT/lib" ],
    	"CPPFLAGS" : [
			## \todo The following will make no sense as soon as CoreMaya is out of contrib
			# and synced in terms of version number
			"-DIECORE_MAJOR_VERSION=%d" % ieCoreMajorVersion,
			"-DIE_MAJOR_VERSION=%d" % ieCoreMayaMajorVersion,
			"-DIE_MINOR_VERSION=%d" % ieCoreMayaMinorVersion,
			"-DIE_PATCH_VERSION=%d" % ieCoreMayaPatchVersion,
			## End of defines we need to remove in future
			"-D_BOOL",
			"-DREQUIRE_IOSTREAM",
			pythonEnv["PYTHON_INCLUDE_FLAGS"],
		],
	}

	if env["PLATFORM"]=="posix" :
		mayaEnvAppends["CPPFLAGS"].append( "-DLINUX" )

	mayaEnv = env.Copy( **mayaEnvSets )
	mayaEnv.Append( **mayaEnvAppends )
	
	mayaPythonEnv = pythonEnv.Copy( **mayaEnvSets )
	riPythonEnv.Append( **mayaEnvAppends )

	if doConfigure :

		c = Configure( mayaEnv )

		if not c.CheckHeader( "maya/MVectorArray.h" ) :
		
			sys.stderr.write( "WARNING : no maya devkit found, not building IECoreMaya - check MAYA_ROOT.\n" )
			c.Finish()

		else :

			c.Finish()
			
			mayaSources = glob.glob( "contrib/IECoreMaya/src/*.cpp" )
			mayaHeaders = glob.glob( "contrib/IECoreMaya/include/IECoreMaya/*.h" ) + glob.glob( "contrib/IECoreMaya/include/IECoreMaya/*.inl" )
			mayaPythonSources = glob.glob( "contrib/IECoreMaya/src/IECoreMaya/bindings/*.cpp" )
			mayaPythonScripts = glob.glob( "contrib/IECoreMaya/python/IECoreMaya/*.py" )

			# we can't append this before configuring, as then it gets built as
			# part of the configure process
			mayaEnv.Prepend( LIBPATH = [ "./lib" ] )
			mayaEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )

			mayaLibrary = mayaEnv.SharedLibrary( "lib/" + os.path.basename( mayaEnv.subst( "$INSTALL_LIB_NAME" ) ), mayaSources )
			mayaEnv.Depends( coreInstallSync, mayaLibrary )
			mayaLibraryInstall = mayaEnv.Install( os.path.dirname( mayaEnv.subst( "$INSTALL_LIB_NAME" ) ), mayaLibrary )
			mayaEnv.Depends( mayaLibraryInstall, coreInstallSync )
			mayaEnv.AddPostAction( mayaLibraryInstall, lambda target, source, env : makeLibSymLinks( mayaEnv ) )
			mayaEnv.Alias( "install", mayaLibraryInstall )
			mayaEnv.Alias( "installMaya", mayaLibraryInstall )

			mayaHeaderInstall = mayaEnv.Install( "$INSTALL_HEADER_DIR/IECoreMaya", mayaHeaders )
			mayaEnv.Depends( mayaHeaderInstall, coreInstallSync )			
			mayaEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreMaya", lambda target, source, env : makeSymLinks( mayaEnv, mayaEnv["INSTALL_HEADER_DIR"] ) )
			mayaEnv.Alias( "install", mayaHeaderInstall )
			mayaEnv.Alias( "installMaya", mayaHeaderInstall )

			mayaPythonEnv.Append(
				LIBS = [
					os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
					os.path.basename( mayaEnv.subst( "$INSTALL_LIB_NAME" ) ),
				]
			)
			mayaPythonModule = mayaPythonEnv.SharedLibrary( "contrib/IECoreMaya/python/IECoreMaya/_IECoreMaya", mayaPythonSources )
			mayaPythonEnv.Depends( mayaPythonModule, mayaLibrary )

			mayaPythonModuleInstall = mayaPythonEnv.Install( "$INSTALL_PYTHON_DIR/IECoreMaya", mayaPythonScripts + mayaPythonModule )
			mayaPythonEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreMaya", lambda target, source, env : makeSymLinks( mayaPythonEnv, mayaPythonEnv["INSTALL_PYTHON_DIR"] ) )
			mayaPythonEnv.Alias( "install", mayaPythonModuleInstall )
			mayaPythonEnv.Alias( "installMaya", mayaPythonModuleInstall )

			Default( [ mayaLibrary, mayaPythonModule ] )

###########################################################################################
# Build and install the coreNuke library and headers
###########################################################################################

nukeEnv = env.Copy( IECORE_NAME = "IECoreNuke" )
nukeEnv.Append( CPPPATH = [ "$NUKE_ROOT/include" ] )
nukeEnv.Prepend( LIBPATH = [ "./lib" ] )

if doConfigure :

	c = Configure( nukeEnv )

	if not c.CheckHeader( "DDImage/Vector3.h", "\"\"", "CXX" ) :
		
		sys.stderr.write( "WARNING : no nuke devkit found, not building IECoreNuke - check NUKE_ROOT.\n" )
		c.Finish()
		
	else :
	
		c.Finish()

		# we can't add this earlier as then it's built during the configure stage, and that's no good
		nukeEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )

		nukeHeaders = glob.glob( "include/IECoreNuke/*.h" ) + glob.glob( "include/IECoreNuke/*.inl" )
		nukeSources = glob.glob( "src/IECoreNuke/*.cpp" )

		nukeLibrary = nukeEnv.SharedLibrary( "lib/" + os.path.basename( nukeEnv.subst( "$INSTALL_LIB_NAME" ) ), nukeSources )
		nukeEnv.Depends( coreInstallSync, nukeLibrary )
		nukeLibraryInstall = nukeEnv.Install( os.path.dirname( nukeEnv.subst( "$INSTALL_LIB_NAME" ) ), nukeLibrary )
		nukeEnv.Depends( nukeLibraryInstall, coreInstallSync )
		nukeEnv.AddPostAction( nukeLibraryInstall, lambda target, source, env : makeLibSymLinks( nukeEnv ) )
		nukeEnv.Alias( "install", nukeLibraryInstall )
		nukeEnv.Alias( "installNuke", nukeLibraryInstall )

		nukeHeaderInstall = nukeEnv.Install( "$INSTALL_HEADER_DIR/IECoreNuke", nukeHeaders )
		nukeEnv.Depends( nukeHeaderInstall, coreInstallSync )
		nukeEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreNuke", lambda target, source, env : makeSymLinks( nukeEnv, nukeEnv["INSTALL_HEADER_DIR"] ) )

		nukeEnv.Alias( "installNuke", nukeHeaderInstall )
		nukeEnv.Alias( "install", nukeHeaderInstall )

###########################################################################################
# Documentation
###########################################################################################

## \todo Have an option for people to point us to their doxygen install, and do
# Configure checks to be sure it's there
docEnv = env.Copy()
docEnv["ENV"]["PATH"] = os.environ["PATH"]
docs = docEnv.Command( "doc/html/index.html", "", "doxygen doc/config/Doxyfile" )
docEnv.Depends( docs, glob.glob( "include/IECore/*.h" ) )
docEnv.Depends( docs, glob.glob( "include/IECoreRI/*.h" ) )
docEnv.Depends( docs, glob.glob( "include/IECoreNuke/*.h" ) )
docEnv.Depends( docs, glob.glob( "src/IECore/*.cpp" ) )
docEnv.Depends( docs, glob.glob( "src/IECoreRI/*.cpp" ) )
docEnv.Depends( docs, glob.glob( "src/IECoreNuke/*.cpp" ) )
docEnv.Depends( docs, glob.glob( "python/IECore/*.py" ) )
docEnv.Depends( docs, glob.glob( "python/IECoreRI/*.py" ) )
docEnv.Depends( docs, glob.glob( "python/IECoreNuke/*.py" ) )

# \todo This won't reinstall the documentation if the directory already exists
installDoc = docEnv.Install( "$INSTALL_DOC_DIR", "doc/html" )
docEnv.Alias( "install", installDoc )
