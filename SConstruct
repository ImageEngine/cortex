import SCons
import glob
import sys
import os

EnsureSConsVersion( 0, 97 )
SConsignFile()

###########################################################################################
# Command line options
###########################################################################################

o = Options( ARGUMENTS )
o.Add( 
	"pythonConfig", 
	"The path to the python-config program for the Python installation you wish to "
	"build against. This is used to obtain the appropriate compilation and linking "
	"flags. The default is to look for python-config on the path.",
	"python-config",
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
	"OPENEXR_INCLUDE_PATH",
	"The path to the OpenEXR include directory.",
	"/usr/local/include/OpenEXR",
)

ieCoreMajorVersion=2
ieCoreMinorVersion=7
ieCorePatchVersion=0

cppFlags = [
    "-DIE_CORE_MAJORVERSION=%d" % ieCoreMajorVersion,
    "-DIE_CORE_MINORVERSION=%d" % ieCoreMinorVersion,
    "-DIE_CORE_PATCHVERSION=%d" % ieCorePatchVersion,
]

coreSources = glob.glob( "src/*.cpp" )
pythonSources = glob.glob( "src/bindings/*.cpp" )

###########################################################################################
# An environment for building libraries
###########################################################################################

e = Environment(
    CPPFLAGS=cppFlags,
	options = o
)

e.Append( CPPPATH = [ "include", "$BOOST_INCLUDE_PATH", "$OPENEXR_INCLUDE_PATH" ] )
# we use "OpenEXR/x.h" and they use "x.h"
e.Append( CPPPATH = [ os.path.dirname( e["OPENEXR_INCLUDE_PATH"] ) ] )

if e["PLATFORM"]=="darwin" :
	e.Append( CXXFLAGS = "-Wno-long-double" )

## \todo We need to determine this automatically somehow
boostVersion = "1_34_1"

# autoconf-like checks for stuff.
# this part of scons doesn't seem so well thought out.

# unlike the rest of the scons build process, the configure stuff
# runs immediately. we don't want to run it in a few cases, as failures
# would mean that people don't get to do what they're actually trying to do
if not "--help" in sys.argv and not "-h" in sys.argv and not e.GetOption( "clean" ) :

	c = Configure( e )
	
	if not c.CheckLibWithHeader( "boost_filesystem-" + boostVersion, "boost/filesystem.hpp", "CXX" ) :
		sys.stderr.write( "ERROR : unable to find the boost libraries.\n" )
		Exit( 1 )
		
	if not c.CheckLibWithHeader( "IlmImf", "OpenEXR/ImfInputFile.h", "CXX" ) :
		sys.stderr.write( "ERROR : unable to find the OpenEXR libraries.\n" )
		Exit( 1 )
		
	if c.CheckLib( "tiff" ) :
		c.env.Append( CPPFLAGS = '-DIECORE_WITH_TIFF' )
	else :
		coreSources.remove( "src/TIFFImageWriter.cpp" )
		coreSources.remove( "src/TIFFImageReader.cpp" )
		pythonSources.remove( "src/bindings/TIFFImageReaderBinding.cpp" )
		pythonSources.remove( "src/bindings/TIFFImageWriterBinding.cpp" )
		
	if c.CheckLib( "jpeg" ) :
		c.env.Append( CPPFLAGS = '-DIECORE_WITH_JPEG' )
	else :
		coreSources.remove( "src/JPEGImageWriter.cpp" )
		coreSources.remove( "src/JPEGImageReader.cpp" )
		pythonSources.remove( "src/bindings/JPEGImageReaderBinding.cpp" )
		pythonSources.remove( "src/bindings/JPEGImageWriterBinding.cpp" )
			
	if c.CheckLibWithHeader( "sqlite3", "sqlite/sqlite3.h", "CXX" ) :
		c.env.Append( CPPFLAGS = "-DIECORE_WITH_SQLITE" )
	else :
		coreSources.remove( "src/SQLiteIndexedIO.cpp" )
	
	c.Finish()

e.Append( LIBS= [
		"boost_filesystem-" + boostVersion,
		"boost_regex-" + boostVersion,
		"boost_iostreams-" + boostVersion,
		"half",
		"IEx",
		"Imath",
		"IlmImf",
	]
)

Help( o.GenerateHelpText( e ) )

library = e.SharedLibrary( "lib/IECore", coreSources )

###########################################################################################
# An environment for building python modules
###########################################################################################

pythonEnv = e.Copy()

try :
	pythonEnv.Append( CPPFLAGS = os.popen( pythonEnv["pythonConfig"] + " --includes" ).read().split() )
	pythonEnv.Append( SHLINKFLAGS = os.popen( pythonEnv["pythonConfig"] + " --ldflags" ).read().split() )
	pythonExecutable = os.popen( pythonEnv["pythonConfig"] + " --exec-prefix" ).read().strip() + "/bin/python"
except :
	sys.stderr.write( "ERROR : Unable to run %s.\n" % pythonEnv["pythonConfig"] )
	Exit( 1 )

pythonEnv.Append( CPPFLAGS = "-DBOOST_PYTHON_MAX_ARITY=20" )
pythonEnv.Append( LIBS = [
		"IECore",
		"boost_python-" + boostVersion,
	]
)
pythonEnv.Append( LIBPATH = [ "./lib" ] )
pythonEnv["SHLIBPREFIX"] = ""
pythonEnv["SHLIBSUFFIX"] = ".so"

if pythonEnv["PLATFORM"]=="darwin" :
	pythonEnv.Append( SHLINKFLAGS = "-single_module" )

pythonModule = pythonEnv.SharedLibrary( "python/IECore/_IECore", pythonSources )
pythonEnv.Depends( pythonModule, library )

###########################################################################################
# Test runner
###########################################################################################

testEnv = Environment()
testEnv["ENV"]["PYTHONPATH"] = "./python"
if testEnv["PLATFORM"]=="darwin" :
	testEnv["ENV"]["DYLD_LIBRARY_PATH"] = ":".join( [ "./lib", e["LIBPATH"] ] )
else :
	testEnv["ENV"]["LD_LIBRARY_PATH"] = ":".join( [ "./lib", e["LIBPATH"] ] )

testEnv.Command( "test/results.txt", pythonModule, pythonExecutable + " test/All.py" )

###########################################################################################
# Documentation
###########################################################################################

docEnv = Environment()
docEnv["ENV"]["PATH"] = os.environ["PATH"]
docs = docEnv.Command( "doc/html/index.html", "", "doxygen doc/config/Doxyfile" )
docEnv.Depends( docs, glob.glob( "src/*.cpp" ) )
docEnv.Depends( docs, glob.glob( "python/IECore/*.py" ) )

Default( [ library, pythonModule ] )
