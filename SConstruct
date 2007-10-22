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
ieCoreMinorVersion=14
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

o.Add(
	"CPPPATH",
	"A colon separated list of paths to search for headers on.",
	"",
)

o.Add(
	"LIBPATH",
	"A colon separated list of paths to search for libraries on.",
	"",
)

o.Add(
	"BOOST_INCLUDE_PATH",
	"The path to the boost include directory.",
	"/usr/local/include/boost-1_34_1",
)

o.Add(
	"BOOST_LIB_SUFFIX",
	"The suffix appended to the names of the boost libraries. You can modify this "
	"to link against the debug versions of the libraries, or to link against "
	"unversioned libraries.",
	"-${BOOST_MAJOR_VERSION}_${BOOST_MINOR_VERSION}_${BOOST_PATCH_VERSION}",
)

o.Add(
	"OPENEXR_INCLUDE_PATH",
	"The path to the OpenEXR include directory.",
	"/usr/local/include/OpenEXR",
)

o.Add( 
	"PYTHON_CONFIG", 
	"The path to the python-config program for the Python installation you wish to "
	"build against. This is used to obtain the appropriate compilation and linking "
	"flags. The default is to look for python-config on the path.",
	"python-config",
)

o.Add(
	"RMAN_ROOT",
	"The directory in which your RenderMan renderer is installed.",
	"",
)

o.Add(
	BoolOption( "DEBUG", "Set this to build without optimisation and with debug symbols.", False ),
)

o.Add(
	"IECORE_LIB_SUFFIX",
	"The suffix for the IECore libraries that are build. This can be used to produce "
	"versioned or unversioned libraries.",
	"-${IECORE_MAJOR_VERSION}.${IECORE_MINOR_VERSION}.${IECORE_PATCH_VERSION}",
)

o.Add(
	"INSTALL_PREFIX",
	"The prefix under which to install the libraries.",
	"/usr/local",
)

o.Add(
	"INSTALL_HEADER_DIR",
	"The directory in which to install the headers.",
	"$INSTALL_PREFIX/include",
)

o.Add(
	"INSTALL_LIB_DIR",
	"The directory in which to install the libraries.",
	"$INSTALL_PREFIX/lib",
)

o.Add(
	"INSTALL_PYTHON_DIR",
	"The directory in which to install the python modules.",
	"$INSTALL_PREFIX/lib/python$PYTHON_VERSION/site-packages",
)

###########################################################################################
# An environment for building libraries
###########################################################################################

env = Environment(
	options = o
)

if "SAVE_OPTIONS" in ARGUMENTS :
	o.Save( ARGUMENTS["SAVE_OPTIONS"], env )

env["IECORE_MAJOR_VERSION"] = ieCoreMajorVersion
env["IECORE_MINOR_VERSION"] = ieCoreMinorVersion
env["IECORE_PATCH_VERSION"] = ieCorePatchVersion

env.Append(
	CPPFLAGS = [
    	"-DIE_CORE_MAJORVERSION=$IECORE_MAJOR_VERSION",
    	"-DIE_CORE_MINORVERSION=$IECORE_MINOR_VERSION",
    	"-DIE_CORE_PATCHVERSION=$IECORE_PATCH_VERSION",
	]
)
	
# we use "OpenEXR/x.h" and they use "x.h"
env.Prepend( CPPPATH = [ os.path.dirname( env["OPENEXR_INCLUDE_PATH"] ) ] )
env.Prepend( CPPPATH = [ "include", "$BOOST_INCLUDE_PATH", "$OPENEXR_INCLUDE_PATH" ] )

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
		sys.stderr.write( "ERROR : unable to find the boost headers.\n" )
		Exit( 1 )
	
	# figure out the boost version in use so we can append it to the
	# library names	if necessary
	boostVersion = None
	boostVersionHeader = env.FindFile( "boost/version.hpp", env["CPPPATH"] )
	for line in open( str( boostVersionHeader ) ) :
		m = re.compile( "^#define BOOST_LIB_VERSION \"(.*)\"$" ).match( line )
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
		sys.stderr.write( "ERROR : unable to determine boost version from \"%s\"." % boostVersionHeader )
		Exit( 1 )

	if not c.CheckLibWithHeader( env.subst( "boost_filesystem" + env["BOOST_LIB_SUFFIX"] ), "boost/filesystem.hpp", "CXX" ) :
		sys.stderr.write( "ERROR : unable to find the boost libraries.\n" )
		Exit( 1 )
		
	if not c.CheckLibWithHeader( "IlmImf", "OpenEXR/ImfInputFile.h", "CXX" ) :
		sys.stderr.write( "ERROR : unable to find the OpenEXR libraries.\n" )
		Exit( 1 )
		
	c.Finish()
		
env.Append( LIBS = [
		"boost_filesystem" + env["BOOST_LIB_SUFFIX"],
		"boost_regex" + env["BOOST_LIB_SUFFIX"],
		"boost_iostreams" + env["BOOST_LIB_SUFFIX"],
		"boost_date_time" + env["BOOST_LIB_SUFFIX"],
		"half",
		"IEx",
		"Imath",
		"IlmImf",
	]
)

Help( o.GenerateHelpText( env ) )
	
###########################################################################################
# An environment for building python modules
###########################################################################################

pythonEnv = env.Copy()

try :
	pythonEnv.Append( CPPFLAGS = os.popen( pythonEnv["PYTHON_CONFIG"] + " --includes" ).read().split() )
	pythonEnv.Append( SHLINKFLAGS = os.popen( pythonEnv["PYTHON_CONFIG"] + " --ldflags" ).read().split() )
	pythonExecutable = os.popen( pythonEnv["PYTHON_CONFIG"] + " --exec-prefix" ).read().strip() + "/bin/python"
	pythonEnv["PYTHON_VERSION"] = os.popen( pythonExecutable + " -c 'import sys; print \"%s.%s\" % sys.version_info[:2]'" ).read().strip()
except :
	sys.stderr.write( "ERROR : Unable to run \"%s\".\n" % pythonEnv["PYTHON_CONFIG"] )
	Exit( 1 )

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

if env["PLATFORM"]=="darwin" :
	libraryPathEnvVar = "DYLD_LIBRARY_PATH"
else :
	libraryPathEnvVar = "LD_LIBRARY_PATH"

testEnv = Environment()
testEnv["ENV"]["PYTHONPATH"] = "./python"
testEnv["ENV"][libraryPathEnvVar] = ":".join( [ "./lib", env["LIBPATH"] ] )

###########################################################################################
# Build, install and test the core library and bindings
###########################################################################################

coreEnv = env.Copy()
corePythonEnv = pythonEnv.Copy()

coreSources = glob.glob( "src/IECore/*.cpp" )
coreHeaders = glob.glob( "include/IECore/*.h" ) + glob.glob( "include/IECore/*.inl" )
corePythonSources = glob.glob( "src/IECore/bindings/*.cpp" )
corePythonScripts = glob.glob( "python/IECore/*.py" )

if doConfigure :

	c = Configure( coreEnv )

	if c.CheckLibWithHeader( "tiff", "tiff.h", "CXX" ) :
		c.env.Append( CPPFLAGS = '-DIECORE_WITH_TIFF' )
		corePythonEnv.Append( CPPFLAGS = '-DIECORE_WITH_TIFF' )
	else :
		coreSources.remove( "src/IECore/TIFFImageWriter.cpp" )
		coreSources.remove( "src/IECore/TIFFImageReader.cpp" )
		corePythonSources.remove( "src/IECore/bindings/TIFFImageReaderBinding.cpp" )
		corePythonSources.remove( "src/IECore/bindings/TIFFImageWriterBinding.cpp" )
		
	if c.CheckLibWithHeader( "jpeg", "jpeg.h", "CXX" ) :
		c.env.Append( CPPFLAGS = '-DIECORE_WITH_JPEG' )
		corePythonEnv.Append( CPPFLAGS = '-DIECORE_WITH_JPEG' )
	else :
		coreSources.remove( "src/IECore/JPEGImageWriter.cpp" )
		coreSources.remove( "src/IECore/JPEGImageReader.cpp" )
		corePythonSources.remove( "src/IECore/bindings/JPEGImageReaderBinding.cpp" )
		corePythonSources.remove( "src/IECore/bindings/JPEGImageWriterBinding.cpp" )
			
	if c.CheckLibWithHeader( "sqlite3", "sqlite/sqlite3.h", "CXX" ) :
		c.env.Append( CPPFLAGS = "-DIECORE_WITH_SQLITE" )
	else :
		coreSources.remove( "src/IECore/SQLiteIndexedIO.cpp" )

	c.Finish()

coreLibrary = coreEnv.SharedLibrary( "lib/IECore" + coreEnv.subst( "$IECORE_LIB_SUFFIX" ), coreSources )

coreLibraryInstall = coreEnv.Install( "$INSTALL_LIB_DIR", coreLibrary )
coreEnv.Alias( "install", coreLibraryInstall )

headerInstall = coreEnv.Install( "$INSTALL_HEADER_DIR/IECore", coreHeaders )
coreEnv.Alias( "install", headerInstall )

corePythonEnv.Append( LIBS = coreLibrary )
corePythonModule = corePythonEnv.SharedLibrary( "python/IECore/_IECore", corePythonSources )
corePythonEnv.Depends( corePythonModule, coreLibrary )

corePythonModuleInstall = corePythonEnv.Install( "$INSTALL_PYTHON_DIR/IECore", corePythonScripts + corePythonModule )
pythonEnv.Alias( "install", corePythonModuleInstall )

Default( coreLibrary, corePythonModule )

coreTest = testEnv.Command( "test/IECore/results.txt", corePythonModule, pythonExecutable + " test/IECore/All.py" )
testEnv.Alias( "coreTest", coreTest )

###########################################################################################
# Build, install and test the coreRI library and bindings
###########################################################################################

riEnv = env.Copy()
riEnv.Append( CPPPATH = [ "$RMAN_ROOT/include" ] )
riEnv.Append( LIBPATH = [ "$RMAN_ROOT/lib" ] )
riEnv.Append(
	CPPFLAGS = [
    	"-DIE_CORE_MAJOR_VERSION=%d" % ieCoreMajorVersion,
	]
)
riEnv.Append(
	LIBS = [
		"3delight",
	]
)

riPythonEnv = pythonEnv.Copy()
riPythonEnv.Append( LIBPATH = [ "$RMAN_ROOT/lib" ] )
riPythonEnv.Append(
	LIBS = [
		"3delight",
	]
)

if doConfigure :

	c = Configure( riEnv )

	if not c.CheckLibWithHeader( "3delight", "ri.h", "C" ) :
	
		sys.stderr.write( "WARNING : not building IECoreRI.\n" )
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
			
		if c.CheckFunc(  "RiObjectBeginV", "ri.h" ) :
			
			riEnv.Append( CPPFLAGS = [ "-DIECORERI_WITH_OBJECTBEGINV" ] )
			
		c.Finish()	

		# we can't append this before configuring, as then it gets built as
		# part of the configure process
		riEnv.Prepend( LIBPATH = [ "./lib" ] )
		riEnv.Append( LIBS = coreLibrary )
	
		riLibrary = riEnv.SharedLibrary( "lib/IECoreRI" + riEnv.subst( "$IECORE_LIB_SUFFIX" ), riSources )
		riLibraryInstall = riEnv.Install( "$INSTALL_LIB_DIR", riLibrary )
		riEnv.Alias( "install", riLibraryInstall )

		riHeaderInstall = riEnv.Install( "$INSTALL_HEADER_DIR/IECoreRI", riHeaders )
		riEnv.Alias( "install", riHeaderInstall )

		riPythonEnv.Append( LIBS = [ coreLibrary, riLibrary ] )
		riPythonModule = riPythonEnv.SharedLibrary( "python/IECoreRI/_IECoreRI", riPythonSources )
		riPythonEnv.Depends( riPythonModule, riLibrary )

		riPythonModuleInstall = riPythonEnv.Install( "$INSTALL_PYTHON_DIR/IECoreRI", riPythonScripts + riPythonModule )
		riPythonEnv.Alias( "install", riPythonModuleInstall )

		Default( [ riLibrary, riPythonModule ] )
		
		riTestEnv = testEnv.Copy()
		riTestEnv["ENV"][libraryPathEnvVar] = riEnv.subst( ":".join( [ "./lib" ] + riPythonEnv["LIBPATH"] ) )
		riTestEnv["ENV"]["SHADER_PATH"] = riEnv.subst( "$RMAN_ROOT/shaders" )
		riTest = riTestEnv.Command( "test/IECoreRI/results.txt", riPythonModule, pythonExecutable + " test/IECoreRI/All.py" )
		riTestEnv.Depends( riTest, corePythonModule )
		riTestEnv.Alias( "riTest", riTest )
		
###########################################################################################
# Documentation
###########################################################################################

docEnv = Environment()
docEnv["ENV"]["PATH"] = os.environ["PATH"]
docs = docEnv.Command( "doc/html/index.html", "", "doxygen doc/config/Doxyfile" )
docEnv.Depends( docs, glob.glob( "src/IECore/*.cpp" ) )
docEnv.Depends( docs, glob.glob( "src/IECoreRI/*.cpp" ) )
docEnv.Depends( docs, glob.glob( "python/IECore/*.py" ) )
docEnv.Depends( docs, glob.glob( "python/IECoreRI/*.py" ) )	
