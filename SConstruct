##########################################################################
#
#  Copyright (c) 2007-2009, Image Engine Design Inc. All rights reserved.
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

ieCoreMajorVersion=3
ieCoreMinorVersion=34
ieCorePatchVersion=1

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

o.Add(
	"CXXFLAGS",
	"The extra flags to pass to the C++ compiler during compilation.",
	[ "-pipe", "-Wall", "-O2" ]
)

o.Add(
	"TESTCXXFLAGS",
	"The extra flags to pass to the C++ compiler during compilation of unit tests.",
	[ "-pipe", "-Wall", "-O0" ]
)

o.Add(
	"PYTHONCXXFLAGS",
	"The extra flags to pass to the C++ compiler during compilation of Python bindings.",
	[ "-pipe", "-Wall", "-O2" ]
)

o.Add(
	"LINKFLAGS",
	"The extra flags to pass to the linker.",
	[]
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

# Freetype options

o.Add(
	"FREETYPE_INCLUDE_PATH",
	"The path to the FreeType include directory.",
	"/usr/local/include/freetype2",
)

o.Add(
	"FREETYPE_LIB_PATH",
	"The path to the FreeType lib directory.",
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

# Truelight options

o.Add(
	"TRUELIGHT_ROOT",
	"The directory in which Truelight is installed.",
	"/usr/fl/truelight"
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
	"MAYA_ROOT",
	"The path to the root of the maya installation.",
	"/usr/aw/maya",
)

o.Add(
	"MAYA_LICENSE_FILE",
	"The path to FlexLM license file to use for Maya.",
	"/usr/flexlm/license.dat",
)


o.Add(
	BoolOption( 
		"WITH_MAYA_PLUGIN_LOADER", 
		"Set this to install the Maya plugin with a stub loader.",
		 False
	),
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
	"INSTALL_MAYALIB_NAME",
	"The name under which to install the maya libraries. This "
	"can be used to build and install the library for multiple "
	"Maya versions.",
	"$INSTALL_PREFIX/lib/$IECORE_NAME",
)

o.Add(
	"INSTALL_NUKELIB_NAME",
	"The name under which to install the nuke libraries. This "
	"can be used to build and install the library for multiple "
	"Nuke versions.",
	"$INSTALL_PREFIX/lib/$IECORE_NAME",
)

o.Add(
	"INSTALL_PYTHON_DIR",
	"The directory in which to install python modules.",
	"$INSTALL_PREFIX/lib/python$PYTHON_VERSION/site-packages",
)

o.Add(
	"INSTALL_NUKEPYTHON_DIR",
	"The directory in which to install the nuke python module.",
	"$INSTALL_PREFIX/lib/python$PYTHON_VERSION/site-packages",
)

o.Add(
	"INSTALL_GLSL_HEADER_DIR",
	"The directory in which to install GLSL headers.",
	"$INSTALL_PREFIX/glsl",
)

o.Add(
	"INSTALL_RSL_HEADER_DIR",
	"The directory in which to install RSL headers.",
	"$INSTALL_PREFIX/rsl",
)

o.Add(
	"INSTALL_MEL_DIR",
	"The directory in which to install mel scripts.",
	"$INSTALL_PREFIX/maya/mel/$IECORE_NAME",
)

o.Add(
	"INSTALL_MAYAPLUGIN_NAME",
	"The name under which to install maya plugins.",
	"$INSTALL_PREFIX/maya/plugins/$IECORE_NAME",
)

o.Add(
	"INSTALL_DOC_DIR",
	"The directory in which to install the documentation.",
	"$INSTALL_PREFIX/share/cortex",
)

o.Add(
	"INSTALL_CORE_POST_COMMAND",
	"A command which is run following a successful installation of "
	"the core library. This could be used to customise installation "
	"further for a particular site.",
	""
)

o.Add(
	"INSTALL_CORERI_POST_COMMAND",
	"A command which is run following a successful installation of "
	"the CoreRI library. This could be used to customise installation "
	"further for a particular site.",
	""
)

o.Add(
	"INSTALL_COREGL_POST_COMMAND",
	"A command which is run following a successful installation of "
	"the CoreGL library. This could be used to customise installation "
	"further for a particular site.",
	""
)

o.Add(
	"INSTALL_COREMAYA_POST_COMMAND",
	"A command which is run following a successful installation of "
	"the CoreMaya library. This could be used to customise installation "
	"further for a particular site.",
	""
)

o.Add(
	"INSTALL_CORENUKE_POST_COMMAND",
	"A command which is run following a successful installation of "
	"the CoreNuke library. This could be used to customise installation "
	"further for a particular site.",
	""
)

o.Add(
	"INSTALL_CORETRUELIGHT_POST_COMMAND",
	"A command which is run following a successful installation of "
	"the CoreTruelight library. This could be used to customise installation "
	"further for a particular site.",
	""
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
	"TEST_RI_SCRIPT",
	"The python script to run for the renderman tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"test/IECoreRI/All.py"
)

o.Add(
	"TEST_GL_SCRIPT",
	"The python script to run for the OpenGL tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"contrib/IECoreGL/test/All.py"
)

o.Add(
	"TEST_TRUELIGHT_SCRIPT",
	"The python script to run for the Truelight tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"test/IECoreTruelight/All.py"
)

o.Add(
	"TEST_MAYA_SCRIPT",
	"The python script to run for the maya tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"test/IECoreMaya/All.py"
)

o.Add(
	"TEST_NUKE_SCRIPT",
	"The python script to run for the nuke tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"test/IECoreNuke/All.py"
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
	"search paths correctly when running the tests. Defaults to LD_LIBRARY_PATH on "
	"Linux and DYLD_LIBRARY_PATH on OSX.",
	"DYLD_LIBRARY_PATH" if Environment()["PLATFORM"]=="darwin" else "LD_LIBRARY_PATH"
)

# Documentation options

o.Add(
	"DOXYGEN",
	"The path to the doxygen binary.",
	"/usr/local/bin/doxygen"
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
		"$FREETYPE_INCLUDE_PATH",
	],
	LIBPATH = [
		"$BOOST_LIB_PATH",
		"$OPENEXR_LIB_PATH",
		"$JPEG_LIB_PATH",
		"$TIFF_LIB_PATH",
		"$FREETYPE_LIB_PATH",
	],
	LIBS = [
		"pthread",
	]
)

if env["PLATFORM"]=="darwin" :
	env.Append( CXXFLAGS = "-Wno-long-double" )
			
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
			m = re.compile( "^([0-9]+)_([0-9]+)(?:_([0-9]+)|)$" ).match( boostVersion )
			boostMajorVersion, boostMinorVersion, boostPatchVersion = m.group( 1, 2, 3 )
			env["BOOST_MAJOR_VERSION"] = boostMajorVersion
			env["BOOST_MINOR_VERSION"] = boostMinorVersion
			env["BOOST_PATCH_VERSION"] = boostPatchVersion or ""
			break
			
	if not boostVersion :
		sys.stderr.write( "ERROR : unable to determine boost version from \"%s\".\n" % boostVersionHeader )
		Exit( 1 )

	env.Append( LIBS = [
			"boost_filesystem" + env["BOOST_LIB_SUFFIX"],
			"boost_regex" + env["BOOST_LIB_SUFFIX"],
			"boost_iostreams" + env["BOOST_LIB_SUFFIX"],
			"boost_date_time" + env["BOOST_LIB_SUFFIX"],
			"boost_thread" + env["BOOST_LIB_SUFFIX"],
			"boost_wave" + env["BOOST_LIB_SUFFIX"],
		]
	)
	if int( env["BOOST_MINOR_VERSION"] ) >=35 :
		env.Append( LIBS = [ "boost_system" + env["BOOST_LIB_SUFFIX"] ] )
		
	if not c.CheckLibWithHeader( env.subst( "boost_iostreams" + env["BOOST_LIB_SUFFIX"] ), "boost/filesystem/path.hpp", "CXX" ) :
		sys.stderr.write( "ERROR : unable to find the boost libraries - check BOOST_LIB_PATH.\n" )
		Exit( 1 )

	if not c.CheckLibWithHeader( "Iex", "OpenEXR/ImfInputFile.h", "C++" ) :
		sys.stderr.write( "ERROR : unable to find the OpenEXR libraries - check OPENEXR_INCLUDE_PATH and OPENEXR_LIB_PATH.\n" )
		Exit( 1 )
		
	c.Finish()
		
env.Append( LIBS = [
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
pythonEnv.Replace( CXXFLAGS = env.subst("$PYTHONCXXFLAGS") )

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
	pythonEnv["PYTHON_LINK_FLAGS"] = getPythonConfig( pythonEnv, "--ldflags" )

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
testEnv.Replace( CXXFLAGS = env.subst("$TESTCXXFLAGS") )

testEnv.Prepend( LIBPATH = [ "./lib" ] )

testEnvLibPath = ":".join( testEnv["LIBPATH"] )
if testEnv["TEST_LIBPATH"] != "" :
	testEnvLibPath += ":" + testEnv["TEST_LIBPATH"]

testEnv["ENV"][testEnv["TEST_LIBRARY_PATH_ENV_VAR"]] = testEnvLibPath
testEnv["ENV"]["DYLD_LIBRARY_PATH" if Environment()["PLATFORM"]=="darwin" else "LD_LIBRARY_PATH"] = testEnvLibPath

if env["PLATFORM"]=="darwin" :	
	# Special workaround for suspected gcc issue - see BoostUnitTestTest for more information
	
	# Link to the boost unit test library	
	if env.has_key("BOOST_MINOR_VERSION") and env["BOOST_MINOR_VERSION"] >= 35 :
		testEnv.Append( LIBS=["boost_test_exec_monitor" + env["BOOST_LIB_SUFFIX"] ] )
	else:
		testEnv.Append( LIBS=["boost_unit_test_framework" + env["BOOST_LIB_SUFFIX"] ] )	
	
else:
	# Link to the boost unit test library
	testEnv.Append( LIBS=["boost_unit_test_framework" + env["BOOST_LIB_SUFFIX"] ] )	
	
	# Unit test library requirement of boost > 1.35.0
	if env.has_key("BOOST_MINOR_VERSION") and env["BOOST_MINOR_VERSION"] >= 35 :
		testEnv.Append( LIBS=["boost_test_exec_monitor" + env["BOOST_LIB_SUFFIX"] ] )

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
		
			keyIndex = target.find( key )
			if keyIndex != -1 :
			
				nextSlashIndex = target.find( "/", keyIndex + len(key) )
				if nextSlashIndex!=-1 :
					# the place where we need to make the links is
					# in the middle of the path somewhere, not in the last
					# component. truncate the path.
					target = target[:nextSlashIndex]

				linkName = target.replace( key, links[key] )
								
				makeSymLink( env.subst( linkName ), env.subst( target ) )
				
				target = linkName
				done = False	

# Makes versioned symlinks for the library an environment makes.
# This function is necessary as there's some name munging to get
# the right prefix and suffix on the library names.
def makeLibSymLinks( env, libNameVar="INSTALL_LIB_NAME" ) :

	p = env[libNameVar]
	
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
		coreSources.remove( "src/IECore/ScopedTIFFExceptionTranslator.cpp" )		
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
	
	if c.CheckLibWithHeader( "freetype", ["ft2build.h"], "CXX" ) :
		c.env.Append( CPPFLAGS = "-DIECORE_WITH_FREETYPE" )
		corePythonEnv.Append( CPPFLAGS = "-DIECORE_WITH_FREETYPE" )
	else :
		sys.stderr.write( "WARNING: no FreeType library found, no font support, check FREETYPE_INCLUDE_PATH and FREETYPE_LIB_PATH.\n" )
		coreSources.remove( "src/IECore/Font.cpp" )
		corePythonSources.remove( "src/IECore/bindings/FontBinding.cpp" )
				
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
coreEnv.NoCache( coreLibraryInstall )
coreEnv.Depends( coreLibraryInstall, coreInstallSync )
coreEnv.AddPostAction( coreLibraryInstall, lambda target, source, env : makeLibSymLinks( coreEnv ) )
coreEnv.Alias( "install", [ coreLibraryInstall ] )
coreEnv.Alias( "installCore", [ coreLibraryInstall ] )
coreEnv.Alias( "installLib", [ coreLibraryInstall ] )

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

# post installation script

if coreEnv["INSTALL_CORE_POST_COMMAND"]!="" :
	# this is the only way we could find to get a post action to run for an alias
	corePythonEnv.Alias( "install", corePythonModuleInstall, "$INSTALL_CORE_POST_COMMAND" ) 
	corePythonEnv.Alias( "installCore", corePythonModuleInstall, "$INSTALL_CORE_POST_COMMAND" ) 

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

riEnv = coreEnv.Copy( IECORE_NAME = "IECoreRI" )
riEnv.Append( CPPPATH = [ "$RMAN_ROOT/include" ] )
riEnv.Append( LIBPATH = [ "$RMAN_ROOT/lib" ] )
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
		riEnv.NoCache( riLibraryInstall )
		riEnv.Depends( riLibraryInstall, coreInstallSync )		
		riEnv.AddPostAction( riLibraryInstall, lambda target, source, env : makeLibSymLinks( riEnv ) )
		riEnv.Alias( "install", riLibraryInstall )
		riEnv.Alias( "installRI", riLibraryInstall )
		riEnv.Alias( "installLib", [ riLibraryInstall ] )

		riHeaderInstall = riEnv.Install( "$INSTALL_HEADER_DIR/IECoreRI", riHeaders )
		riEnv.Depends( riHeaderInstall, coreInstallSync )
		riEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreRI", lambda target, source, env : makeSymLinks( riEnv, riEnv["INSTALL_HEADER_DIR"] ) )
		riEnv.Alias( "install", riHeaderInstall )
		riEnv.Alias( "installRI", riHeaderInstall )

		rslHeaders = glob.glob( "rsl/IECoreRI/*.h" )
		rslHeaderInstall = riEnv.Install( "$INSTALL_RSL_HEADER_DIR/IECoreRI", rslHeaders )
		riEnv.AddPostAction( "$INSTALL_RSL_HEADER_DIR/IECoreRI", lambda target, source, env : makeSymLinks( riEnv, riEnv["INSTALL_RSL_HEADER_DIR"] ) )
		riEnv.Alias( "install", rslHeaderInstall )
		riEnv.Alias( "installRI", rslHeaderInstall )
		
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

		if coreEnv["INSTALL_CORERI_POST_COMMAND"]!="" :
			# this is the only way we could find to get a post action to run for an alias
			riPythonEnv.Alias( "install", riPythonModuleInstall, "$INSTALL_CORERI_POST_COMMAND" ) 
			riPythonEnv.Alias( "installRI", riPythonModuleInstall, "$INSTALL_CORERI_POST_COMMAND" ) 

		Default( [ riLibrary, riPythonModule ] )
		
		riTestEnv = testEnv.Copy()

		riTestEnv["ENV"][testEnv["TEST_LIBRARY_PATH_ENV_VAR"]] += ":" + riEnv.subst( ":".join( [ "./lib" ] + riPythonEnv["LIBPATH"] ) )
		riTestEnv["ENV"]["SHADER_PATH"] = riEnv.subst( "$RMAN_ROOT/shaders" )
		riTestEnv["ENV"]["DELIGHT"] = riEnv.subst( "$RMAN_ROOT" )
		riTestEnv["ENV"]["DL_SHADERS_PATH"] = riEnv.subst( "$RMAN_ROOT/shaders" )
		riTestEnv["ENV"]["DL_DISPLAYS_PATH"] = riEnv.subst( "$RMAN_ROOT/displays" )
		
		riTest = riTestEnv.Command( "test/IECoreRI/results.txt", riPythonModule, pythonExecutable + " $TEST_RI_SCRIPT" )
		NoCache( riTest )
		riTestEnv.Depends( riTest, corePythonModule )
		riTestEnv.Depends( riTest, glob.glob( "test/IECoreRI/*.py" ) )
		riTestEnv.Alias( "testRI", riTest )

###########################################################################################
# Build, install and test the optional CoreGL library and bindings
###########################################################################################

if env["WITH_GL"] and doConfigure :

	glEnvSets = {
		"IECORE_NAME" : "IECoreGL",
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
		"LIBPATH" : [
			"$GLEW_LIB_PATH",
		],
	}
	
	glEnv = coreEnv.Copy( **glEnvSets )

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
		if not "-DIECORE_WITH_FREETYPE" in glEnv["CPPFLAGS"] :
			glSources.remove( "contrib/IECoreGL/src/Font.cpp" )
			glSources.remove( "contrib/IECoreGL/src/TextPrimitive.cpp" )
		
		glLibrary = glEnv.SharedLibrary( "lib/" + os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ), glSources )
		glEnv.Depends( coreInstallSync, glLibrary )
		glLibraryInstall = glEnv.Install( os.path.dirname( glEnv.subst( "$INSTALL_LIB_NAME" ) ), glLibrary )
		glEnv.NoCache( glLibraryInstall )
		glEnv.Depends( glLibraryInstall, coreInstallSync )
		glEnv.AddPostAction( glLibraryInstall, lambda target, source, env : makeLibSymLinks( glEnv ) )
		glEnv.Alias( "install", glLibraryInstall )
		glEnv.Alias( "installGL", glLibraryInstall )
		glEnv.Alias( "installLib", [ glLibraryInstall ] )

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
		
		glPythonSources = glob.glob( "contrib/IECoreGL/src/bindings/*.cpp" )
		glPythonEnv = pythonEnv.Copy( **glEnvSets )
		glPythonEnv.Append( **glEnvAppends )
		glPythonEnv.Prepend( **glEnvPrepends )
		glPythonEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)
		glPythonModule = glPythonEnv.SharedLibrary( "contrib/IECoreGL/python/IECoreGL/_IECoreGL", glPythonSources )
		glPythonEnv.Depends( coreInstallSync, glPythonModule )
		glPythonEnv.Depends( glPythonModule, glLibrary )

		glPythonScripts = glob.glob( "contrib/IECoreGL/python/IECoreGL/*.py" )
		glPythonModuleInstall = glPythonEnv.Install( "$INSTALL_PYTHON_DIR/IECoreGL", glPythonScripts + glPythonModule )		
		glPythonEnv.Depends( glPythonModuleInstall, coreInstallSync )
		glPythonEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreGL", lambda target, source, env : makeSymLinks( glPythonEnv, glPythonEnv["INSTALL_PYTHON_DIR"] ) )
		glPythonEnv.Alias( "install", glPythonModuleInstall )
		glPythonEnv.Alias( "installGL", glPythonModuleInstall )

		if coreEnv["INSTALL_COREGL_POST_COMMAND"]!="" :
			# this is the only way we could find to get a post action to run for an alias
			glPythonEnv.Alias( "install", glPythonModuleInstall, "$INSTALL_COREGL_POST_COMMAND" ) 
			glPythonEnv.Alias( "installGL", glPythonModuleInstall, "$INSTALL_COREGL_POST_COMMAND" ) 

		Default( [ glLibrary, glPythonModule ] )

		glTestEnv = testEnv.Copy()
		glTestEnv["ENV"]["PYTHONPATH"] = glTestEnv["ENV"]["PYTHONPATH"] + ":contrib/IECoreGL/python"
		for e in ["DISPLAY", "XAUTHORITY"] :
			if e in os.environ :
				glTestEnv["ENV"][e] = os.environ[e]
		
		glTest = glTestEnv.Command( "contrib/IECoreGL/test/results.txt", glPythonModule, pythonExecutable + " $TEST_GL_SCRIPT" )
		NoCache( glTest )
		glTestEnv.Depends( glTest, corePythonModule )
		glTestEnv.Depends( glTest, glob.glob( "contrib/IECoreGL/test/*.py" ) )
		glTestEnv.Alias( "testGL", glTest )
		
###########################################################################################
# Build, install and test the coreMaya library and bindings
###########################################################################################

mayaEnvSets = {
	"IECORE_NAME" : "IECoreMaya",
}

mayaEnvAppends = {
	"CPPPATH" : [
		"$MAYA_ROOT/include",
		"contrib/IECoreGL/include",
		"$GLEW_INCLUDE_PATH",
	],
	"LIBPATH" : [ "$MAYA_ROOT/lib" ],
	"LIBS" : [
		"OpenMayalib",
		"OpenMaya",
		"OpenMayaUI",
		"OpenMayaAnim",
		"OpenMayaFX",
		"boost_python" + pythonEnv["BOOST_LIB_SUFFIX"],
	],
	"CPPFLAGS" : [
		"-D_BOOL",
		"-DREQUIRE_IOSTREAM",
		pythonEnv["PYTHON_INCLUDE_FLAGS"],
	],
}

if env["PLATFORM"]=="posix" :
	mayaEnvAppends["CPPFLAGS"].append( "-DLINUX" )

mayaEnv = env.Copy( **mayaEnvSets )
mayaEnv.Append( **mayaEnvAppends )

mayaEnv.Append( SHLINKFLAGS = pythonEnv["PYTHON_LINK_FLAGS"].split() )

mayaPythonEnv = pythonEnv.Copy( **mayaEnvSets )
mayaPythonEnv.Append( **mayaEnvAppends )

mayaPluginEnv = mayaEnv.Copy( IECORE_NAME="ieCore" )

if doConfigure :

	c = Configure( mayaEnv )

	if not c.CheckHeader( "maya/MVectorArray.h" ) :

		sys.stderr.write( "WARNING : no maya devkit found, not building IECoreMaya - check MAYA_ROOT.\n" )
		c.Finish()

	else :

		c.Finish()

		mayaSources = glob.glob( "src/IECoreMaya/*.cpp" )
		mayaHeaders = glob.glob( "include/IECoreMaya/bindings/*.h" ) + glob.glob( "include/IECoreMaya/*.h" ) + glob.glob( "include/IECoreMaya/*.inl" )
		mayaBindingHeaders = glob.glob( "include/IECoreMaya/bindings/*.h" ) + glob.glob( "include/IECoreMaya/bindings/*.inl" )
		mayaPythonSources = glob.glob( "src/IECoreMaya/bindings/*.cpp" )
		mayaPythonScripts = glob.glob( "python/IECoreMaya/*.py" )
		mayaMel = glob.glob( "mel/IECoreMaya/*.mel" )
		mayaPluginSources = [ "src/IECoreMaya/plugin/Plugin.cpp" ]

		# we can't append this before configuring, as then it gets built as
		# part of the configure process
		mayaEnv.Prepend( LIBPATH = [ "./lib" ] )
		mayaEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )
		mayaEnv.Append( LIBS = os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ) )

		# maya library
		mayaLibrary = mayaEnv.SharedLibrary( "lib/" + os.path.basename( mayaEnv.subst( "$INSTALL_MAYALIB_NAME" ) ), mayaSources )
		mayaEnv.Depends( coreInstallSync, mayaLibrary )
		mayaLibraryInstall = mayaEnv.Install( os.path.dirname( mayaEnv.subst( "$INSTALL_MAYALIB_NAME" ) ), mayaLibrary )
		mayaEnv.NoCache( mayaLibraryInstall )
		mayaEnv.Depends( mayaLibraryInstall, coreInstallSync )
		mayaEnv.AddPostAction( mayaLibraryInstall, lambda target, source, env : makeLibSymLinks( mayaEnv, "INSTALL_MAYALIB_NAME" ) )
		mayaEnv.Alias( "install", mayaLibraryInstall )
		mayaEnv.Alias( "installMaya", mayaLibraryInstall )
		mayaEnv.Alias( "installLib", [ mayaLibraryInstall ] )

 		# maya headers
		mayaHeaderInstall = mayaEnv.Install( "$INSTALL_HEADER_DIR/IECoreMaya", mayaHeaders )
		mayaHeaderInstall += mayaEnv.Install( "$INSTALL_HEADER_DIR/IECoreMaya/bindings", mayaBindingHeaders )		
		mayaEnv.Depends( mayaHeaderInstall, coreInstallSync )			
		mayaEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreMaya", lambda target, source, env : makeSymLinks( mayaEnv, mayaEnv["INSTALL_HEADER_DIR"] ) )
		mayaEnv.Alias( "install", mayaHeaderInstall )
		mayaEnv.Alias( "installMaya", mayaHeaderInstall )

		# maya mel
		mayaMelInstall = mayaEnv.Install( "$INSTALL_MEL_DIR", mayaMel )
		mayaEnv.Depends( mayaMelInstall, coreInstallSync )
		mayaEnv.AddPostAction( "$INSTALL_MEL_DIR", lambda target, source, env : makeSymLinks( mayaEnv, mayaEnv["INSTALL_MEL_DIR"] ) )
		mayaEnv.Alias( "install", mayaMelInstall )
		mayaEnv.Alias( "installMaya", mayaMelInstall )
		
		# maya plugin
		mayaPluginEnv.Append(
			LIBPATH = [ "./lib" ],
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_MAYALIB_NAME" ) ),
				os.path.basename( mayaEnv.subst( "$INSTALL_MAYALIB_NAME" ) ),
			]
		)
		
		mayaPluginTarget = "plugins/maya/" + os.path.basename( mayaPluginEnv.subst( "$INSTALL_MAYAPLUGIN_NAME" ) )
		
		if env["WITH_MAYA_PLUGIN_LOADER"] :
		
			mayaPluginLoaderSources = [ 'src/IECoreMaya/plugin/Loader.cpp' ]
		
			mayaPluginLoaderEnv = mayaPluginEnv.Copy()
			mayaPluginLoaderEnv.Append(			
				LIBS = [
					"dl"
				]
			)
			
			mayaPluginLoader = mayaPluginLoaderEnv.SharedLibrary( mayaPluginTarget, mayaPluginLoaderSources, SHLIBPREFIX="" )
			mayaPluginLoaderInstall = mayaPluginLoaderEnv.InstallAs( mayaPluginLoaderEnv.subst( "$INSTALL_MAYAPLUGIN_NAME$SHLIBSUFFIX" ), mayaPluginLoader )
			mayaPluginLoaderEnv.Depends( mayaPluginLoaderInstall, coreInstallSync )
			mayaPluginLoaderEnv.AddPostAction( mayaPluginLoaderInstall, lambda target, source, env : makeSymLinks( mayaPluginLoaderEnv, mayaPluginLoaderEnv["INSTALL_MAYAPLUGIN_NAME"] ) )
			mayaPluginLoaderEnv.Alias( "install", mayaPluginLoaderInstall )
			mayaPluginLoaderEnv.Alias( "installMaya", mayaPluginLoaderInstall )
			
			Default( mayaPluginLoader )
			
			mayaPluginEnv["INSTALL_MAYAPLUGIN_NAME"] = os.path.join( os.path.dirname( mayaPluginEnv["INSTALL_MAYAPLUGIN_NAME"] ), 'impl', os.path.basename( mayaPluginEnv["INSTALL_MAYAPLUGIN_NAME"] ) )
			mayaPluginTarget = "plugins/maya/impl/" + os.path.basename( mayaPluginEnv.subst( "$INSTALL_MAYAPLUGIN_NAME" ) )
		
		mayaPlugin = mayaPluginEnv.SharedLibrary( mayaPluginTarget, mayaPluginSources, SHLIBPREFIX="" )
		mayaPluginInstall = mayaPluginEnv.Install( os.path.dirname( mayaPluginEnv.subst( "$INSTALL_MAYAPLUGIN_NAME" ) ), mayaPlugin )
		mayaPluginEnv.Depends( mayaPlugin, corePythonModule )
		mayaPluginEnv.Depends( mayaPluginInstall, coreInstallSync )
		
		mayaPluginEnv.AddPostAction( mayaPluginInstall, lambda target, source, env : makeSymLinks( mayaPluginEnv, mayaPluginEnv["INSTALL_MAYAPLUGIN_NAME"] ) )
		mayaPluginEnv.Alias( "install", mayaPluginInstall )
		mayaPluginEnv.Alias( "installMaya", mayaPluginInstall )
				
		# maya python
		mayaPythonEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( mayaEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)
		mayaPythonModule = mayaPythonEnv.SharedLibrary( "python/IECoreMaya/_IECoreMaya", mayaPythonSources )
		mayaPythonEnv.Depends( mayaPythonModule, mayaLibrary )

		mayaPythonModuleInstall = mayaPythonEnv.Install( "$INSTALL_PYTHON_DIR/IECoreMaya", mayaPythonScripts + mayaPythonModule )
		mayaPythonEnv.Depends( mayaPythonModuleInstall, coreInstallSync )
		mayaPythonEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreMaya", lambda target, source, env : makeSymLinks( mayaPythonEnv, mayaPythonEnv["INSTALL_PYTHON_DIR"] ) )
		mayaPythonEnv.Alias( "install", mayaPythonModuleInstall )
		mayaPythonEnv.Alias( "installMaya", mayaPythonModuleInstall )

		if coreEnv["INSTALL_COREMAYA_POST_COMMAND"]!="" :
			# this is the only way we could find to get a post action to run for an alias
			mayaPythonEnv.Alias( "install", mayaPythonModuleInstall, "$INSTALL_COREMAYA_POST_COMMAND" ) 
			mayaPythonEnv.Alias( "installMaya", mayaPythonModuleInstall, "$INSTALL_COREMAYA_POST_COMMAND" ) 

		Default( [ mayaLibrary, mayaPlugin, mayaPythonModule ] )
		
		mayaTestEnv = testEnv.Copy()
		mayaTestEnv["ENV"]["LD_LIBRARY_PATH"] += ":" + mayaEnv.subst( ":".join( [ "./lib" ] + mayaPythonEnv["LIBPATH"] ) )
		mayaTestEnv["ENV"]["PATH"] = mayaEnv.subst( "$MAYA_ROOT/bin:" ) + mayaEnv["ENV"]["PATH"]
		mayaTestEnv["ENV"]["MAYA_PLUG_IN_PATH"] = "./plugins/maya:./test/IECoreMaya/plugins"
		mayaTestEnv["ENV"]["MAYA_SCRIPT_PATH"] = "./mel"
		
		mayaPythonTestEnv = mayaTestEnv.Copy()
		
		mayaTestEnv.Append( **mayaEnvAppends )
		mayaTestEnv.Append( 
			LIBS = [ 
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ), 
				os.path.basename( mayaEnv.subst( "$INSTALL_LIB_NAME" ) ), 				
				"OpenMayalib" 
			] 
		)
		mayaTestEnv["ENV"]["PYTHONHOME"] = mayaTestEnv.subst( "$MAYA_ROOT" )
		mayaTestEnv["ENV"]["MAYA_LOCATION"] = mayaTestEnv.subst( "$MAYA_ROOT" )
		mayaTestEnv["ENV"]["LM_LICENSE_FILE"] = env["MAYA_LICENSE_FILE"]
				
		mayaTestProgram = mayaTestEnv.Program( "test/IECoreMaya/IECoreMayaTest", glob.glob( "test/IECoreMaya/*.cpp" ) )
		mayaTest = mayaTestEnv.Command( "test/IECoreMaya/results.txt", mayaTestProgram, "test/IECoreMaya/IECoreMayaTest >& test/IECoreMaya/results.txt" )
		NoCache( mayaTest )
		mayaTestEnv.Depends( mayaTest, [ mayaPlugin, mayaPythonModule ] )
		mayaTestEnv.Alias( "testMaya", mayaTest )
		
		mayaPythonExecutable = "mayapy"
		
		mayaPythonTest = mayaPythonTestEnv.Command( "test/IECoreMaya/resultsPython.txt", mayaPythonModule, mayaPythonExecutable + " $TEST_MAYA_SCRIPT" )
		NoCache( mayaPythonTest )
		mayaPythonTestEnv.Depends( mayaPythonTest, [ mayaPlugin, mayaPythonModule ] )
		mayaPythonTestEnv.Depends( mayaPythonTest, glob.glob( "test/IECoreMaya/*.py" ) )
		if env["WITH_MAYA_PLUGIN_LOADER"] :
			mayaPythonTestEnv.Depends( mayaPythonTest, mayaPluginLoader )
		mayaPythonTestEnv.Alias( "testMaya", mayaPythonTest )			

###########################################################################################
# Build and install the coreNuke library and headers
###########################################################################################

nukeEnv = env.Copy( IECORE_NAME = "IECoreNuke" )
nukeEnv.Append( CPPPATH = [ "$NUKE_ROOT/include" ] )
nukeEnv.Prepend( LIBPATH = [ "$NUKE_ROOT", "./lib" ] )
nukeEnv.Prepend( LIBS = [ "GLEW" ] )
		
nukePythonEnv = pythonEnv.Copy( IECORE_NAME = "IECoreNuke" )

nukeTestEnv = testEnv.Copy()

if doConfigure :

	c = Configure( nukeEnv )

	if not c.CheckHeader( "DDImage/Vector3.h", "\"\"", "CXX" ) :
		
		sys.stderr.write( "WARNING : no nuke devkit found, not building IECoreNuke - check NUKE_ROOT.\n" )
		c.Finish()
		
	else :

		# figure out the nuke version from the headers
		nukeMajorVersion = None
		nukeMinorVersion = None
		nukeVersionHeader = env.FindFile( "DDImage/ddImageVersionNumbers.h", nukeEnv["CPPPATH"] )
		if nukeVersionHeader :

			for line in open( str( nukeVersionHeader ) ) :
				w = line.split()
				if w[0]=="#define" and w[1]=="kDDImageVersionMajorNum" :
					nukeMajorVersion = int( w[2] )
				elif w[0]=="#define" and w[1]=="kDDImageVersionMinorNum" :
					nukeMinorVersion = int( w[2] )
					
		else :
		
			nukeMajorVersion = 4
			nukeMinorVersion = 8
					
		if nukeMajorVersion is None or nukeMinorVersion is None :
		
			sys.stderr.write( "ERROR : unable to determine nuke version - not building IECoreNuke.\n" )
		
		else :
		
			nukeEnv["NUKE_MAJOR_VERSION"] = nukeMajorVersion
			nukeEnv["NUKE_MINOR_VERSION"] = nukeMinorVersion
			
			nukeTestEnv["NUKE_MAJOR_VERSION"] = nukeMajorVersion
			nukeTestEnv["NUKE_MINOR_VERSION"] = nukeMinorVersion						

			if nukeMajorVersion >=5 and nukeMinorVersion >=0 :
				nukeLibName = "DDImage"
			else :
				nukeLibName = "DDImage%d.%d" % ( nukeMajorVersion, nukeMinorVersion )
				
			if not c.CheckLibWithHeader( nukeLibName, "DDImage/Vector3.h", "CXX" ) :
		
				sys.stderr.write( "WARNING : no nuke libraries found, not building IECoreNuke - check NUKE_ROOT.\n" )
				c.Finish()
		
			else :
	
				c.Finish()
				
				# we can't add this earlier as then it's built during the configure stage, and that's no good
				nukeEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )
				
				nukeEnv.Append( LIBS = [ nukeLibName ] )

				nukeEnv.Append(
					CPPFLAGS = [
    					"-DIECORENUKE_NUKE_MAJOR_VERSION=$NUKE_MAJOR_VERSION",
    					"-DIECORENUKE_NUKE_MINOR_VERSION=$NUKE_MINOR_VERSION",
					]
				)

				nukeHeaders = glob.glob( "include/IECoreNuke/*.h" ) + glob.glob( "include/IECoreNuke/*.inl" )
				nukeSources = glob.glob( "src/IECoreNuke/*.cpp" )
				nukePythonScripts = glob.glob( "python/IECoreNuke/*.py" )

				nukeLibrary = nukeEnv.SharedLibrary( "lib/" + os.path.basename( nukeEnv.subst( "$INSTALL_NUKELIB_NAME" ) ), nukeSources )
				nukeEnv.Depends( coreInstallSync, nukeLibrary )
				nukeLibraryInstall = nukeEnv.Install( os.path.dirname( nukeEnv.subst( "$INSTALL_NUKELIB_NAME" ) ), nukeLibrary )
				nukeEnv.Depends( nukeLibraryInstall, coreInstallSync )
				nukeEnv.AddPostAction( nukeLibraryInstall, lambda target, source, env : makeLibSymLinks( nukeEnv, "INSTALL_NUKELIB_NAME" ) )
				nukeEnv.Alias( "install", nukeLibraryInstall )
				nukeEnv.Alias( "installNuke", nukeLibraryInstall )
				nukeEnv.Alias( "installLib", [ nukeLibraryInstall ] )

				nukeHeaderInstall = nukeEnv.Install( "$INSTALL_HEADER_DIR/IECoreNuke", nukeHeaders )
				nukeEnv.Depends( nukeHeaderInstall, coreInstallSync )
				nukeEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreNuke", lambda target, source, env : makeSymLinks( nukeEnv, nukeEnv["INSTALL_HEADER_DIR"] ) )
				nukeEnv.Alias( "installNuke", nukeHeaderInstall )
				nukeEnv.Alias( "install", nukeHeaderInstall )

				nukePythonModuleInstall = nukePythonEnv.Install( "$INSTALL_NUKEPYTHON_DIR/IECoreNuke", nukePythonScripts )
				nukePythonEnv.Depends( nukePythonModuleInstall, coreInstallSync )
				nukePythonEnv.AddPostAction( "$INSTALL_NUKEPYTHON_DIR/IECoreNuke", lambda target, source, env : makeSymLinks( nukePythonEnv, nukePythonEnv["INSTALL_NUKEPYTHON_DIR"] ) )
				nukePythonEnv.Alias( "install", nukePythonModuleInstall )
				nukePythonEnv.Alias( "installNuke", nukePythonModuleInstall )

				if coreEnv["INSTALL_CORENUKE_POST_COMMAND"]!="" :
					# this is the only way we could find to get a post action to run for an alias
					nukeEnv.Alias( "install", nukeLibraryInstall, "$INSTALL_CORENUKE_POST_COMMAND" ) 
					nukeEnv.Alias( "installNuke", nukeLibraryInstall, "$INSTALL_CORENUKE_POST_COMMAND" ) 

				Default( [ nukeLibrary ] )

				nukeTest = nukeTestEnv.Command( "test/IECoreNuke/resultsPython.txt", nukeLibrary, "echo \"execfile( '$TEST_NUKE_SCRIPT' )\" | $NUKE_ROOT/Nuke${NUKE_MAJOR_VERSION}.${NUKE_MINOR_VERSION} -t" )
				NoCache( nukeTest )
				nukeTestEnv.Depends( nukeTest, glob.glob( "test/IECoreNuke/*.py" ) )
				nukeTestEnv.Alias( "testNuke", nukeTest )			

###########################################################################################
# Build and install the coreTruelight library and headers
###########################################################################################

truelightEnv = env.Copy( IECORE_NAME = "IECoreTruelight" )
truelightEnv.Append( LIBS = [ "truelight" ] )

# Remove all the boost libs for the configure state- if we don't do this then the configure test can fail for some compilers. 
# \todo Need to establish exactly what is going on here.
oldTruelightLibs = list( truelightEnv["LIBS"] )
truelightEnv["LIBS"] = [ x for x in truelightEnv["LIBS"] if x.find( "boost_" ) == -1 ] 

truelightEnv.Append( CPPPATH = [ "$TRUELIGHT_ROOT/include" ] )
truelightEnv.Prepend( LIBPATH = [
		"./lib",
		"$TRUELIGHT_ROOT/lib"
	]
)

truelightPythonEnv = pythonEnv.Copy( IECORE_NAME="IECoreTruelight" )

if doConfigure :

	c = Configure( truelightEnv )

	if not c.CheckLibWithHeader( "truelight", "truelight.h", "CXX" ) :
		
		sys.stderr.write( "WARNING : no truelight devkit found, not building IECoreTruelight - check TRUELIGHT_ROOT.\n" )
		c.Finish()
		
	else :
	
		c.Finish()
		
		truelightEnv["LIBS"] = oldTruelightLibs

		# we can't add this earlier as then it's built during the configure stage, and that's no good
		truelightEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )

		truelightHeaders = glob.glob( "include/IECoreTruelight/*.h" ) + glob.glob( "include/IECoreTruelight/*.inl" )
		truelightSources = glob.glob( "src/IECoreTruelight/*.cpp" )
		truelightPythonSources = glob.glob( "src/IECoreTruelight/bindings/*.cpp" )
		truelightPythonScripts = glob.glob( "python/IECoreTruelight/*.py" )

		# library
		truelightLibrary = truelightEnv.SharedLibrary( "lib/" + os.path.basename( truelightEnv.subst( "$INSTALL_LIB_NAME" ) ), truelightSources )
		truelightEnv.Depends( truelightLibrary, coreLibrary )
		truelightEnv.Depends( coreInstallSync, truelightLibrary )
		truelightLibraryInstall = truelightEnv.Install( os.path.dirname( truelightEnv.subst( "$INSTALL_LIB_NAME" ) ), truelightLibrary )
		truelightEnv.NoCache( truelightLibraryInstall )
		truelightEnv.Depends( truelightLibraryInstall, coreInstallSync )
		truelightEnv.AddPostAction( truelightLibraryInstall, lambda target, source, env : makeLibSymLinks( truelightEnv ) )
		truelightEnv.Alias( "install", truelightLibraryInstall )
		truelightEnv.Alias( "installTruelight", truelightLibraryInstall )
		truelightEnv.Alias( "installLib", [ truelightLibraryInstall ] )

		# headers
		truelightHeaderInstall = truelightEnv.Install( "$INSTALL_HEADER_DIR/IECoreTruelight", truelightHeaders )
		truelightEnv.Depends( truelightHeaderInstall, coreInstallSync )
		truelightEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreTruelight", lambda target, source, env : makeSymLinks( truelightEnv, truelightEnv["INSTALL_HEADER_DIR"] ) )
		truelightEnv.Alias( "installTruelight", truelightHeaderInstall )
		truelightEnv.Alias( "install", truelightHeaderInstall )
		
		# python
		truelightPythonEnv.Append( LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( truelightEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)

		truelightPythonModule = truelightPythonEnv.SharedLibrary( "python/IECoreTruelight/_IECoreTruelight", truelightPythonSources )
		truelightPythonEnv.Depends( truelightPythonModule, corePythonModule )
		truelightPythonEnv.Depends( coreInstallSync, truelightPythonModule )		
		truelightPythonEnv.Depends( truelightPythonModule, truelightLibrary )
		
		truelightPythonModuleInstall = truelightPythonEnv.Install( "$INSTALL_PYTHON_DIR/IECoreTruelight", truelightPythonScripts + truelightPythonModule )
		truelightPythonEnv.Depends( truelightPythonModuleInstall, coreInstallSync )
		truelightPythonEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreTruelight", lambda target, source, env : makeSymLinks( truelightPythonEnv, truelightPythonEnv["INSTALL_PYTHON_DIR"] ) )
		truelightPythonEnv.Alias( "install", truelightPythonModuleInstall )
		truelightPythonEnv.Alias( "installTruelight", truelightPythonModuleInstall )
		
		if coreEnv["INSTALL_CORETRUELIGHT_POST_COMMAND"]!="" :
			# this is the only way we could find to get a post action to run for an alias
			truelightEnv.Alias( "install", truelightLibraryInstall, "$INSTALL_CORETRUELIGHT_POST_COMMAND" ) 
			truelightEnv.Alias( "installTruelight", truelightLibraryInstall, "$INSTALL_CORETRUELIGHT_POST_COMMAND" ) 

		Default( [ truelightLibrary, truelightPythonModule ] )
		
		# tests
		truelightTestEnv = testEnv.Copy()
		#riTestEnv["ENV"][testEnv["TEST_LIBRARY_PATH_ENV_VAR"]] = riEnv.subst( ":".join( [ "./lib" ] + riPythonEnv["LIBPATH"] ) )
		truelightTestEnv["ENV"]["TRUELIGHT_ROOT"] = truelightEnv.subst( "$TRUELIGHT_ROOT" )
		truelightTest = truelightTestEnv.Command( "test/IECoreTruelight/results.txt", truelightPythonModule, pythonExecutable + " $TEST_TRUELIGHT_SCRIPT" )
		NoCache( truelightTest )
		truelightTestEnv.Depends( truelightTest, truelightPythonModule )
		truelightTestEnv.Depends( truelightTest, glob.glob( "test/IECoreTruelight/*.py" ) )
		truelightTestEnv.Alias( "testTruelight", truelightTest )
		
###########################################################################################
# Documentation
###########################################################################################

docEnv = env.Copy()
docEnv["ENV"]["PATH"] = os.environ["PATH"]

if doConfigure :

	sys.stdout.write( "Checking for doxygen... " )

	if os.path.exists( docEnv["DOXYGEN"] ) :
	
		sys.stdout.write( "yes\n" )
		
		f = open( "doc/config/Doxyfile", "r" )
		
		doxyfile = {}
		
		for line in f.readlines() :
		
			m = re.compile( "^([ \t])*([A-Z_]+)([ \t])*=([ \t])*(.*)" ).match( line )
			if m  :
			
				pair = m.group( 2,5 )
				doxyfile[ pair[0] ] = pair[1]
				
		f.close()
		
		docs = docEnv.Command( "doc/html/index.html", "doc/config/Doxyfile", "$DOXYGEN $SOURCE" )
		
		for inputDirectory in doxyfile["INPUT"].split( ' ' ) :
		
			for filePattern in doxyfile["FILE_PATTERNS"].split( ' ' ) :
			
				docEnv.Depends( docs, glob.glob( inputDirectory + "/" + filePattern ) )
				
		docEnv.Depends( docs, doxyfile["HTML_HEADER"] )
		docEnv.Depends( docs, doxyfile["HTML_FOOTER"] )
		docEnv.Depends( docs, doxyfile["HTML_STYLESHEET"] )						
				
		docEnv.Alias( "doc", "doc/html/index.html" )

		# \todo This won't reinstall the documentation if the directory already exists
		installDoc = docEnv.Install( "$INSTALL_DOC_DIR", "doc/html" )
		docEnv.Alias( "install", installDoc )
		
	else: 
	
		sys.stdout.write( "no\n" )
		sys.stderr.write( "WARNING : no doxygen binary found, not building documentation - check DOXYGEN\n" )

	
