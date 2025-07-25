##########################################################################
#
#  Copyright (c) 2007-2015, Image Engine Design Inc. All rights reserved.
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios),
#  its affiliates and/or its licensors.
#
#  Copyright (c) 2012, John Haddon. All rights reserved.
#
#  Copyright 2012, Electric Theatre Collective Limited. All rights reserved.
#
#  Copyright 2019, Hypothetical Inc. All rights reserved.
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
import shutil
import glob
import sys
import os
import re
import subprocess
import platform
import distutils

EnsureSConsVersion( 3, 0, 2 )  # Substfile is a default builder as of 3.0.2
SConsignFile()

ieCoreMilestoneVersion = 10 # for announcing major milestones - may contain all of the below
ieCoreMajorVersion = 6 # backwards-incompatible changes
ieCoreMinorVersion = 0 # new backwards-compatible features
ieCorePatchVersion = 0 # bug fixes
ieCoreVersionSuffix = "a1" # used for alpha/beta releases. Example: "a1", "b2", etc.

###########################################################################################
# Command line options
###########################################################################################

optionsFile = None

if "CORTEX_OPTIONS_FILE" in os.environ :
	optionsFile = os.environ["CORTEX_OPTIONS_FILE"]

if "OPTIONS" in ARGUMENTS :
	optionsFile = ARGUMENTS["OPTIONS"]

try :
	o = Variables( optionsFile, ARGUMENTS )
except NameError :
	# fallback for old scons versions
	o = Options( optionsFile, ARGUMENTS )

o.Add(
	"CXX",
	"The C++ compiler.",
	{"darwin" : "clang++", "win32" : "cl"}.get(Environment()["PLATFORM"], "g++")

)

# Windows : Disable permissive mode, making MSVC more standards compliant.
# /D_USE_MATH_DEFINES is needed with permissive mode off for
# common definitions like M_P.
# /Zc:externC- fixes a compilation error with Boost::interprocess
# described here:
# https://developercommunity.visualstudio.com/content/problem/756694/including-windowsh-and-boostinterprocess-headers-l.html
# /DBOOST_ALL_NO_LIB is needed to find Boost when it is built without
# verbose system information added to file and directory names.
o.Add(
	"CXXFLAGS",
	"The extra flags to pass to the C++ compiler during compilation.",
	[ "-pipe", "-Wall", "-Wextra", "-Wsuggest-override" ] if Environment()["PLATFORM"] != "win32" else [ "/permissive-", "/D_USE_MATH_DEFINES", "/Zc:externC-", "/DBOOST_ALL_NO_LIB" ],
)

o.Add(
	"CXXSTD",
	"The C++ standard to build against.",
	"c++17"
)

o.Add(
	BoolVariable( "ASAN", "Enable ASan when compiling with clang++", False)
)

o.Add(
	BoolVariable( "WARNINGS_AS_ERRORS", "Treats compiler warnings as errors.", True )
)

o.Add(
	EnumVariable(
		"BUILD_TYPE",
		"Optimisation and debug symbol configuration",
		"RELEASE",
		allowed_values = ('RELEASE', 'DEBUG', 'RELWITHDEBINFO')
	)
)

o.Add(
	"LINKFLAGS",
	"The extra flags to pass to the linker.",
	[]
)

# TBB options

o.Add(
	"TBB_INCLUDE_PATH",
	"The path to the tbb include directory.",
	"",
)

o.Add(
	"TBB_LIB_PATH",
	"The path to the tbb library directory.",
	"",
)

o.Add(
	"TBB_LIB_SUFFIX",
	"The suffix appended to the names of the tbb libraries. You can modify this "
	"to link against libraries installed with non-default names.",
	"",
)

# Boost options

o.Add(
	"BOOST_INCLUDE_PATH",
	"The path to the boost include directory.",
	"",
)

o.Add(
	"BOOST_LIB_PATH",
	"The path to the boost library directory.",
	"",
)

o.Add(
	"BOOST_LIB_SUFFIX",
	"The suffix appended to the names of the boost libraries. You can modify this "
	"to link against the debug versions of the libraries, or to link against "
	"unversioned libraries.",
	"-${BOOST_MAJOR_VERSION}_${BOOST_MINOR_VERSION}_${BOOST_PATCH_VERSION}",
)

o.Add(
	"BOOST_PYTHON_LIB_SUFFIX",
	"The suffix appended to the names of the python boost libraries. "
	"You can modify this so that the correct python library name is used, "
	"likely related to the specific python version.",
)

# OpenEXR options

o.Add(
	"OPENEXR_INCLUDE_PATH",
	"The path to the OpenEXR include directory.",
	"",
)

o.Add(
	"OPENEXR_LIB_PATH",
	"The path to the OpenEXR lib directory.",
	"",
)

o.Add(
	"ILMBASE_INCLUDE_PATH",
	"The path to the IlmBase include directory.",
	"$OPENEXR_INCLUDE_PATH",
)

o.Add(
	"ILMBASE_LIB_PATH",
	"The path to the IlmBase lib directory.",
	"$OPENEXR_LIB_PATH",
)

o.Add(
	"OPENEXR_LIB_SUFFIX",
	"The suffix appended to the names of the OpenEXR libraries. You can modify this "
	"to link against libraries installed with non-default names",
	"",
)

# Freetype options

o.Add(
	"FREETYPE_INCLUDE_PATH",
	"The path to the FreeType include directory.",
	"",
)

o.Add(
	"FREETYPE_LIB_PATH",
	"The path to the FreeType lib directory.",
	"",
)

# OSL options

o.Add(
	"OSL_INCLUDE_PATH",
	"The path to the OpenShadingLanguage include directory.",
	"",
)

o.Add(
	"OSL_LIB_PATH",
	"The path to the OpenShadingLanguage library directory.",
	"",
)

# OIIO options

o.Add(
	"OIIO_INCLUDE_PATH",
	"The path to the OpenImageIO include directory.",
	"",
)

o.Add(
	"OIIO_LIB_PATH",
	"The path to the OpenImageIO library directory.",
	"",
)

o.Add(
	"OIIO_LIB_SUFFIX",
	"The suffix appended to the names of the OpenImageIO libraries. You can modify this "
	"to link against libraries installed with non-defalt names.",
	"",
)

o.Add(
	BoolVariable( "WITH_OIIO_UTIL", "Build with OpenImageIO_Util", True ),
)

# Blosc options

o.Add(
	"BLOSC_INCLUDE_PATH",
	"The path to the Blosc include directory.",
	"",
)
o.Add(
	"BLOSC_LIB_PATH",
	"The path to the Blosc library directory.",
	"",
)

o.Add(
	"BLOSC_LIB_SUFFIX",
	"The suffix appended to the names of the Blosc libraries. You can modify this "
	"to link against libraries installed with non-default names.",
	"",
)

# General path options

o.Add(
	"CPPPATH",
	"A colon (semi-colon on Windows) separated list of paths to search for headers on.",
	"",
)

o.Add(
	"LIBPATH",
	"A colon (semi-colon on Windows) separated list of paths to search for libraries on.",
	"",
)

o.Add(
	"PYTHONPATH",
	"A colon (semi-colon on Windows) separated list of paths to search for python modules on.",
	"",
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

o.Add(
	"PYTHON_LIB_PATH",
	"The path to the Python library directory. If unspecified this will be discovered automatically using PYTHON_CONFIG.",
	"",
)

# OpenGL options

try :
	o.Add(
		BoolVariable( "WITH_GL", "Set this to build the IECoreGL library.", False ),
	)
except NameError :
	# fallback for old scons versions
	o.Add(
		BoolOption( "WITH_GL", "Set this to build the IECoreGL library.", False ),
	)

o.Add(
	"GLEW_INCLUDE_PATH",
	"The path to the directory with glew.h in it.",
	"",
)

o.Add(
	"GLEW_LIB_PATH",
	"The path to the directory with libGLEW in it.",
	"",
)

o.Add(
	"GLEW_LIB_SUFFIX",
	"The suffix appended to the names of the GLEW library. You can modify this "
	"to link against libraries installed with non-defalt names.",
	"",
)

# USD options

o.Add(
	"USD_INCLUDE_PATH",
	"The path to the USD include directory.",
	"",
)

o.Add(
	"USD_LIB_PATH",
	"The path to the USD lib directory.",
	"",
)

o.Add(
	"USD_LIB_PREFIX",
	"The prefix to prepend to the names of the USD libraries. You can modify this "
	"to link against libraries installed with non-default names. "
	"Should match the USD build option PXR_LIB_PREFIX",
	""
)

o.Add(
	BoolVariable(
		"WITH_USD_MONOLITHIC",
		"Determines if we link to the individual usd libs or a monolithic lib"
		"Should match the USD linker option PXR_BUILD_MONOLITHIC",
		 False
	)
)

# Alembic options

o.Add(
	"ALEMBIC_INCLUDE_PATH",
	"The path to the Alembic include directory.",
	"",
)

o.Add(
	"ALEMBIC_LIB_PATH",
	"The path to the Alembic lib directory.",
	"",
)

o.Add(
	"ALEMBIC_LIB_SUFFIX",
	"The suffix appended to the names of the Alembic libraries. You can modify this "
	"to link against libraries installed with non-default names",
	"",
)

o.Add(
	"HDF5_INCLUDE_PATH",
	"The path to the hdf5 include directory.",
	"",
)

o.Add(
	"HDF5_LIB_PATH",
	"The path to the hdf5 lib directory.",
	"",
)

o.Add(
	"HDF5_LIB_SUFFIX",
	"The suffix appended to the names of the HDF5 libraries. You can modify this "
	"to link against libraries installed with non-default names",
	"",
)

#
o.Add(
	"VDB_INCLUDE_PATH",
	"The path to the OpenVDB include directory.",
	"",
)

o.Add(
	"VDB_LIB_PATH",
	"The path to the OpenVDB lib directory.",
	"",
)

o.Add(
	"VDB_LIB_SUFFIX",
	"The suffix appended to the names of the OpenVDB libraries. You can modify this "
	"to link against libraries installed with non-default names",
	"",
)

o.Add(
	"VDB_PYTHON_PATH",
	"The path to the OpenVDB lib directory for the python bindings.",
	"",
)

o.Add(
	"PYBIND11_INCLUDE_PATH",
	"The path to the pybind11 include directory.",
	"",
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
	"CI",
)

# Installation options

o.Add(
	"INSTALL_PREFIX",
	"The prefix under which to install things.",
	"",
)

o.Add(
	BoolVariable( "INSTALL_PKG_CONFIG_FILE",
				"Install a pkg-config configuration in $INSTALL_PREFIX/lib/pkgconfig.",
				True ),
)

o.Add(
	"INSTALL_HEADER_DIR",
	"The directory in which to install headers.",
	"$INSTALL_PREFIX/include",
)

o.Add(
	"INSTALL_USD_RESOURCE_DIR",
	"The directory in which to install USD resource files.",
	"$INSTALL_PREFIX/resources",
)

o.Add(
	"INSTALL_LIB_NAME",
	"The name under which to install the libraries.",
	"$INSTALL_PREFIX/lib/$IECORE_NAME",
)

o.Add(
	"INSTALL_PYTHONLIB_NAME",
	"The name under which to install the python library. This can "
	"be used to build and install the library for multiple python versions.",
	"$INSTALL_PREFIX/lib/$IECORE_NAME",
)

o.Add(
	"INSTALL_ALEMBICLIB_NAME",
	"The name under which to install the Alembic libraries. This "
	"can be used to build and install the library for multiple "
	"Alembic versions.",
	"$INSTALL_PREFIX/lib/$IECORE_NAME",
)

o.Add(
	"INSTALL_USDLIB_NAME",
	"The name under which to install the USD libraries. This "
	"can be used to build and install the library for multiple "
	"USD versions.",
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
	"INSTALL_GLSL_SHADER_DIR",
	"The directory in which to install GLSL shaders.",
	"$INSTALL_PREFIX/glsl",
)

o.Add(
	"INSTALL_DOC_DIR",
	"The directory in which to install the documentation.",
	"$INSTALL_PREFIX/share/cortex",
)

o.Add(
	"INSTALL_IECORE_OP_PATH",
	"The directory in which to install the IECore op stubs.",
	"$INSTALL_PREFIX/ops/$IECORE_NAME-1.py",
)

o.Add(
	"INSTALL_IECORE_OPS",
	"The IECore C++ ops to install via python stubs.",
	[
		( "IECore.SequenceLsOp", os.path.join( "common", "fileSystem", "seqLs" ) ),
		( "IECore.SequenceCpOp", os.path.join( "common", "fileSystem", "seqCp" ) ),
		( "IECore.SequenceMvOp", os.path.join( "common", "fileSystem", "seqMv" ) ),
		( "IECore.SequenceRmOp", os.path.join( "common", "fileSystem", "Rm" ) ),
		( "IECore.SequenceRenumberOp", os.path.join( "common", "fileSystem", "seqRenumber" ) ),
		( "IECore.SequenceConvertOp", os.path.join( "common", "fileSystem", "seqConvert" ) ),
		( "IECore.SequenceCatOp", os.path.join( "common", "fileSystem", "seqCat" ) ),
		( "IECoreScene.RemovePrimitiveVariables", os.path.join( "common", "primitive", "removeVariables" ) ),
		( "IECoreScene.RenamePrimitiveVariables", os.path.join ("common", "primitive", "renameVariables" ) ),
		( "IECore.ClassLsOp", os.path.join( "common", "classes", "classLs" ) ),
		( "IECore.LsHeaderOp", os.path.join( "common", "fileSystem", "lsHeader" ) ),
		( "IECore.SearchReplaceOp", os.path.join( "common", "fileSystem", "searchReplace" ) ),
		( "IECoreScene.TriangulateOp", os.path.join( "common", "primitive", "mesh", "triangulate" ) ),
		( "IECoreScene.MeshNormalsOp", os.path.join( "common", "primitive", "mesh", "addNormals" ) ),
		( "IECoreScene.MeshMergeOp", os.path.join( "common", "primitive", "mesh", "merge"  ) ),
		( "IECoreScene.MeshVertexReorderOp", os.path.join( "common", "primitive", "mesh", "vertexReorder" ) ),
		( "IECoreScene.MeshPrimitiveShrinkWrapOp", os.path.join( "common", "primitive", "mesh", "shrinkWrap" ) ),
	]
)

o.Add(
	"INSTALL_CORE_POST_COMMAND",
	"A command which is run following a successful installation of "
	"the core library. This could be used to customise installation "
	"further for a particular site.",
	""
)

o.Add(
	"INSTALL_COREIMAGE_POST_COMMAND",
	"A command which is run following a successful installation of "
	"the CoreImage library. This could be used to customise installation "
	"further for a particular site.",
	""
)

o.Add(
	"INSTALL_CORESCENE_POST_COMMAND",
	"A command which is run following a successful installation of "
	"the CoreScene library. This could be used to customise installation "
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
	BoolVariable( "INSTALL_CREATE_SYMLINKS", "Whether to create symlinks post install", True )
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
	"TEST_IMAGE_SCRIPT",
	"The python script to run for the image tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"test/IECoreImage/All.py"
)

o.Add(
	"TEST_SCENE_SCRIPT",
	"The python script to run for the scene tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"test/IECoreScene/All.py"
)

o.Add(
	"TEST_GL_SCRIPT",
	"The python script to run for the OpenGL tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"test/IECoreGL/All.py"
)

o.Add(
	"TEST_ALEMBIC_SCRIPT",
	"The python script to run for the alembic tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"contrib/IECoreAlembic/test/IECoreAlembic/All.py"
)

o.Add(
	"TEST_VDB_SCRIPT",
	"The python script to run for the VDB tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"test/IECoreVDB/All.py"
)

o.Add(
	"TEST_USD_SCRIPT",
	"The python script to run for the USD tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"contrib/IECoreUSD/test/IECoreUSD/All.py"
)

o.Add(
	"TEST_LIBPATH",
	"Additional colon separated (semi-colon on Windows) paths to be prepended to the "
	"library path used when running tests.",
	""
)

if Environment()["PLATFORM"]=="darwin" :
	libraryPathEnvVar = "DYLD_LIBRARY_PATH"
elif Environment()["PLATFORM"] != "win32":
	libraryPathEnvVar = "LD_LIBRARY_PATH"
else:
	libraryPathEnvVar = "PATH"

o.Add(
	"TEST_LIBRARY_PATH_ENV_VAR",
	"This is a curious one, probably only ever necessary at image engine. It "
	"specifies the name of an environment variable used to specify the library "
	"search paths correctly when running the tests. Defaults to LD_LIBRARY_PATH on "
	"Linux, DYLD_LIBRARY_PATH on OSX and PATH on Windows.",
	libraryPathEnvVar
)

# Documentation options

o.Add(
	"DOXYGEN",
	"The path to the doxygen binary.",
	"doxygen"
)

###########################################################################################
# An environment for building libraries
###########################################################################################

###########################################################################################
# Common configuration
###########################################################################################

env = Environment(
	MSVC_VERSION = "14.3",
	options = o
)

if isinstance( env["LIBPATH"], str ) :
	env["LIBPATH"] = env["LIBPATH"].split( os.pathsep )

for e in env["ENV_VARS_TO_IMPORT"].split() :
	if e in os.environ :
		env["ENV"][e] = os.environ[e]

if "SAVE_OPTIONS" in ARGUMENTS :
	o.Save( ARGUMENTS["SAVE_OPTIONS"], env )

if env["BUILD_CACHEDIR"] != "" :
	CacheDir( env["BUILD_CACHEDIR"] )

env["IECORE_MILESTONE_VERSION"] = ieCoreMilestoneVersion
env["IECORE_MAJOR_VERSION"] = ieCoreMajorVersion
env["IECORE_MINOR_VERSION"] = ieCoreMinorVersion
env["IECORE_PATCH_VERSION"] = ieCorePatchVersion
env["IECORE_VERSION_SUFFIX"] = ieCoreVersionSuffix
env["IECORE_COMPATIBILITY_VERSION"] = "${IECORE_MILESTONE_VERSION}.${IECORE_MAJOR_VERSION}"
env["IECORE_VERSION"] = "${IECORE_MILESTONE_VERSION}.${IECORE_MAJOR_VERSION}.${IECORE_MINOR_VERSION}.${IECORE_PATCH_VERSION}"
if ieCoreVersionSuffix :
	env["IECORE_VERSION"] += "-${IECORE_VERSION_SUFFIX}"

env.Append(
	CPPFLAGS = [
		"-DIE_CORE_MILESTONEVERSION=$IECORE_MILESTONE_VERSION",
		"-DIE_CORE_MAJORVERSION=$IECORE_MAJOR_VERSION",
		"-DIE_CORE_MINORVERSION=$IECORE_MINOR_VERSION",
		"-DIE_CORE_PATCHVERSION=$IECORE_PATCH_VERSION",
		"-DBOOST_FILESYSTEM_VERSION=3",
		# Boost has deprecated `boost/bind.hpp` in favour of
		# `boost/bind/bind.hpp`, and we have updated our code accordingly. But
		# `boost::python` and others are still using the deprecated header,
		# so we define BOOST_BIND_GLOBAL_PLACEHOLDERS to silence the reams of
		# warnings triggered by that.
		"-DBOOST_BIND_GLOBAL_PLACEHOLDERS",
	]
)
systemIncludeArgument = "/external:I" if env[ "PLATFORM" ] == "win32" else "-isystem"

# update the include and lib paths
dependencyIncludes = [
	systemIncludeArgument, "$BOOST_INCLUDE_PATH",
	systemIncludeArgument, "$OPENEXR_INCLUDE_PATH",
	systemIncludeArgument, "$ILMBASE_INCLUDE_PATH",
	systemIncludeArgument, "$TBB_INCLUDE_PATH",
	systemIncludeArgument, "$BLOSC_INCLUDE_PATH",
	# we use "OpenEXR/x.h" and they use "x.h"
	systemIncludeArgument, os.path.join( "$OPENEXR_INCLUDE_PATH","OpenEXR" ),
	systemIncludeArgument, os.path.join( "$ILMBASE_INCLUDE_PATH","Imath" ),
	systemIncludeArgument, "$FREETYPE_INCLUDE_PATH",
]

env.Prepend(
	LIBPATH = [
		"./lib",
		"$PYTHON_LIB_PATH",
		"$TBB_LIB_PATH",
		"$BOOST_LIB_PATH",
		"$OPENEXR_LIB_PATH",
		"$ILMBASE_LIB_PATH",
		"$FREETYPE_LIB_PATH",
		"$BLOSC_LIB_PATH"
	],
)

env.Append( CXXFLAGS = dependencyIncludes )

env.Prepend(
	CPPPATH = [
		"include",
	],
)

if env["PLATFORM"] == "win32" :
	env["MAXLINELENGTH"] = 8191

###########################################################################################
# POSIX configuration
###########################################################################################

if env["PLATFORM"] != "win32" :
	env.Prepend(
		LIBS = [
			"pthread",
		]
	)

	if env["PLATFORM"]=="darwin" :
		# necessary to fix errors from boost/numeric/interval.hpp
		env.Append( CXXFLAGS = [ "-D__USE_ISOC99" ] )
		# os x versions before snow leopard require the no-long-double flag
		compilerVersion = [ int( v ) for v in env["CXXVERSION"].split( "." ) ]
		if compilerVersion[0] < 4 or compilerVersion[0]==4 and compilerVersion[1] < 2 :
			env.Append( CXXFLAGS = "-Wno-long-double" )
		osxVersion = [ int( v ) for v in platform.mac_ver()[0].split( "." ) ]
		# Work around problem with unused local typedefs in boost and
		# deprecation of gluBuild2DMipmaps() in OSX 10.9.
		if osxVersion[0] == 10 and osxVersion[1] > 7 :
			env.Append( CXXFLAGS = [ "-Wno-unused-local-typedef", "-Wno-deprecated-declarations" ] )
		clangVersion = subprocess.check_output( [ env["CXX"], "-dumpversion" ], env=env["ENV"], universal_newlines=True ).strip()
		clangVersion = [ int( v ) for v in clangVersion.split( "." ) ]
		# Work around Boost issues with Xcode 15 where `std::unary_function` has been removed.
		if clangVersion >= [ 15, 0, 0 ] :
			env.Append( CXXFLAGS = [ "-DBOOST_NO_CXX98_FUNCTION_BASE", "-D_HAS_AUTO_PTR_ETC=0" ] )
		# Disable FMA on arm64 builds to limit floating point discrepancies with x86_64 builds.
		if platform.machine() == "arm64" :
			env.Append( CXXFLAGS = [ "-ffp-contract=off" ] )

	elif env["PLATFORM"]=="posix" :
		if "g++" in os.path.basename( env["CXX"] ) and not "clang++" in os.path.basename( env["CXX"] ) :
			gccVersion = subprocess.check_output( [ env["CXX"], "-dumpversion" ], env=env["ENV"], universal_newlines=True ).strip()
			if "." not in gccVersion :
				# GCC 7 onwards requires `-dumpfullversion` to get minor/patch, but this
				# flag does not exist on earlier GCCs, where minor/patch was provided by `-dumpversion`.
				gccVersion = subprocess.check_output( [ env["CXX"], "-dumpfullversion" ], env=env["ENV"], universal_newlines=True ).strip()
			gccVersion = [ int( v ) for v in gccVersion.split( "." ) ]
			if gccVersion >= [ 5, 1 ] and gccVersion < [ 11, 2 ] :
				env.Append( CXXFLAGS = [ "-D_GLIBCXX_USE_CXX11_ABI=0" ] )

	env.Append( CXXFLAGS = [ "-std=$CXXSTD", "-fvisibility=hidden" ] )

	if "clang++" in os.path.basename( env["CXX"] ) :
		# Turn off the parts of `-Wall` and `-Wextra` that we don't like.
		env.Append( CXXFLAGS = ["-Wno-unused-local-typedef", "-Wno-unused-parameter"] )

	elif "g++" in os.path.basename( env["CXX"] ) :
		# Turn off the parts of `-Wextra` that we don't like.
		env.Append( CXXFLAGS = [ "-Wno-cast-function-type", "-Wno-unused-parameter" ] )

	if env["ASAN"] :
		env.Append(
			CXXFLAGS = [ "-fsanitize=address" ],
			LINKFLAGS = [ "-fsanitize=address" ]
		)
		if "clang++" in os.path.basename( env["CXX"] ) :
			env.Append(
				CXXFLAGS = [ "-shared-libasan" ],
				LINKFLAGS = [ "-shared-libasan" ],
			)

	if env["WARNINGS_AS_ERRORS"] :
		env.Append(
			CXXFLAGS = [ "-Werror" ],
			SHLINKFLAGS = [ "-Wl,-fatal_warnings" ],
		)

	if env["BUILD_TYPE"] == "DEBUG" :
		env.Append( CXXFLAGS = ["-g", "-O0"] )
	elif env["BUILD_TYPE"] == "RELEASE" :
		env.Append( CXXFLAGS = ["-DNDEBUG", "-DBOOST_DISABLE_ASSERTS", "-O3"] )
	elif env["BUILD_TYPE"] == "RELWITHDEBINFO" :
		env.Append( CXXFLAGS = ["-DNDEBUG", "-DBOOST_DISABLE_ASSERTS", "-O3", "-g", "-fno-omit-frame-pointer"] )

###########################################################################################
# Windows configuration
###########################################################################################

else:
	env.Append(
		CXXFLAGS = [
			"/nologo",
			"/diagnostics:classic",
			"/DWIN32",
			"/D_WINDOWS",
			"/DOPENEXR_DLL",
			"/DIMATH_DLL",
			"/DNOMINMAX",
			"/D__PRETTY_FUNCTION__=__FUNCSIG__",
			"/DBOOST_ALL_DYN_LINK",
			"/DBOOST_FILESYSTEM_NO_DEPRICATED",
			"/DBOOST_PYTHON_MAX_ARITY=20",
			"/D_WINDLL",
			"/D_MBCS",
			"/W4",
			"/external:W0",
			"/Zc:inline", # Remove unreferenced function or data if it is COMDAT or has internal linkage only
			"/GR", # enable RTTI
			"/TP", # treat all files as c++ (vs C)
			"/FC", # display full paths in diagnostics
			"/EHsc", # catch c++ exceptions only
			"/MP",  # enable multiprocessing of builds
			"/std:$CXXSTD",
		]
	)

	if env["WARNINGS_AS_ERRORS"] :
		env.Append(
			CXXFLAGS=[
				"/WX",
				# We are building all client code in the exact same environment, so we can safely
				# disable warnings about not exporting private classes
				"/wd4251",
				"/wd4100",  # suppress warning about unused parameters
				"/wd4706",	# suppress warning about using assignment in conditionals
				"/wd4267",  # suppress warning about conversion from int to size_t
				"/wd4244",  # suppress warning about possible loss of data in type conversion
				"/wd4305",  # suppress warning about conversion from double to float
				"/wd4506",  # suppress warning about no definition for inline function. Needed for USD::Glf
				# NOTE : the following warnings are generated by external dependencies, even when using /external
				# They may be able to be re-enabled after updating to MSVC 2022.
				"/wd4127",  # suppress warning "conditional expression is constant", Needed for USD
				"/wd4456",  # suppress warning "declaration of 'x' hides previous local declaration"
				"/wd4459",  # suppress warning "declaration of 'x' hides global declaration"
				"/wd4201",  # suppress warning "nonstandard extension used : nameless struct/union"
				"/wd4245",  # suppress warning "'initializing': conversion from 'int' to 'size_t', signed / unsigned mismatch
				# suppress warning about exported class deriving from non-exported class.
				# Microsoft states (in https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-2-c4275?view=msvc-170)
				# that "C4275 can be ignored if you are deriving from a type in the
				# C++ Standard Library", which is the case
				"/wd4275",
				"/wd4003",  # suppress warning "not enough arguments for function-like macro invocation 'BOOST_PP_SEQ_DETAIL_IS_NOT_EMPTY'"
				"/D_CRT_SECURE_NO_WARNINGS",  # suppress warnings about getenv and similar
				"/DHAVE_SNPRINTF",  # Fixes error "multiple definitions of snprintf"
			]
		)

	if env["BUILD_TYPE"] == "DEBUG" :
		env.Append(
			CXXFLAGS =
			[
				"/O0",
				"/Zi",
				"/MDd",
				"/DBOOST_DISABLE_ASSERTS",
				"/bigobj",
			],
			CCPDBFLAGS=
			[
				"/Zi",
				"/Fd${TARGET}.pdb",
			],
		)
	elif env["BUILD_TYPE"] == "RELEASE" :
		env.Append(
			CXXFLAGS =
			[
				"/DNDEBUG",
				"/MD",	# create multithreaded DLL
				"/DBOOST_DISABLE_ASSERTS",
				"/O2",
				# -Og optimization (included via -Ox) generates lots of unreachable
				# code warnings from boost::intrusive_ptr. Disabled in release build only.
				"/wd4702"
			]
		)
	elif env["BUILD_TYPE"] == "RELWITHDEBINFO" :
		env.Append(
			CXXFLAGS =
			[
				"/DNDEBUG",
				"/MD",
				"/bigobj",
				"/DBOOST_DISABLE_ASSERTS",
				"/Zi",
			],
			LINKFLAGS =
			[
				"/DEBUG",
			],
			CCPDBFLAGS=
			[
				"/Zi",
				"/Fd${TARGET}.pdb",
			],
		)

	# Reorder build commands so that `/external:I` includes come after `/I` includes.
	# Otherwise we'll pick up the Gaffer includes from the build directory, and not
	# the ones in the source tree.

	for command, cxxFlags in [
		( "CXXCOM", "$CXXFLAGS" ),
		( "SHCXXCOM", "$SHCXXFLAGS" )
	] :
		if env[command].index( cxxFlags ) < env[command].index( "$_CCCOMCOM" ) :
			# `$_CCCOMCOM` contains the preprocessor flags, including `/I`. Swap
			# it with `cxxFlags`, which contains `/external:I`.
			env[command] = env[command].replace( cxxFlags, "<>" ).replace( "$_CCCOMCOM", cxxFlags ).replace( "<>",  "$_CCCOMCOM" )



# autoconf-like checks for stuff.
# this part of scons doesn't seem so well thought out.

# unlike the rest of the scons build process, the configure stuff
# runs immediately. we don't want to run it in a few cases, as failures
# would mean that people don't get to do what they're actually trying to do
doConfigure = not "--help" in sys.argv and not "-h" in sys.argv and not env.GetOption( "clean" )

def configureSharedLibrary( env ) :

	# Since we only build shared libraries and not executables,
	# we only need to check that shared libs will link correctly.
	# This is necessary when building against a library that links
	# to extra dependencies not required by Cortex. This approach
	# succeeds because building a shared library doesn't require
	# resolving the unresolved symbols of the libraries that it links to.
	checkEnv = env.Clone()
	checkEnv.Append( CXXFLAGS = [ "-fPIC" ] )
	checkEnv.Append( LINKFLAGS = [ "-shared" ] )

	return Configure( checkEnv )

if doConfigure :

	c = Configure( env )

	if not c.CheckHeader( "boost/version.hpp", "\"\"", "C++" ) :
		sys.stderr.write( "ERROR : unable to find the boost headers, check BOOST_INCLUDE_PATH.\n" )
		Exit( 1 )

	c.Finish()

	# figure out the boost version in use so we can append it to the
	# library names	if necessary
	boostVersion = None
	boostVersionHeader = env.FindFile( "boost/version.hpp", dependencyIncludes )
	if boostVersionHeader is None :
		sys.stderr.write( "ERROR : unable to find the boost headers, check BOOST_INCLUDE_PATH.\n" )
		Exit( 1 )

	for line in open( str( boostVersionHeader ) ) :
		m = re.compile( r"^#define BOOST_LIB_VERSION \"(.*)\"\s*$" ).match( line )
		if m  :
			boostVersion = m.group( 1 )
		if boostVersion :
			m = re.compile( r"^([0-9]+)_([0-9]+)(?:_([0-9]+)|)$" ).match( boostVersion )
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
			"boost_timer" + env["BOOST_LIB_SUFFIX"],
			"boost_chrono" + env["BOOST_LIB_SUFFIX"]
		]
	)

	if int( env["BOOST_MINOR_VERSION"] ) >=35 :
		env.Append( LIBS = [ "boost_system" + env["BOOST_LIB_SUFFIX"] ] )

	c = configureSharedLibrary( env )

	if not c.CheckLibWithHeader( env.subst( "boost_iostreams" + env["BOOST_LIB_SUFFIX"] ), "boost/iostreams/chain.hpp", "CXX" ) :
		sys.stderr.write( "ERROR : unable to find the boost libraries - check BOOST_LIB_PATH.\n" )
		Exit( 1 )

	if not c.CheckLibWithHeader( "Iex" + env["OPENEXR_LIB_SUFFIX"], "OpenEXR/ImfInputFile.h", "C++" ) :
		sys.stderr.write( "ERROR : unable to find the OpenEXR libraries - check OPENEXR_INCLUDE_PATH and OPENEXR_LIB_PATH.\n" )
		Exit( 1 )

	if not c.CheckLibWithHeader( "tbb" + env["TBB_LIB_SUFFIX"], "tbb/tbb.h", "C++" ) :
		sys.stderr.write( "ERROR : unable to find the TBB libraries - check TBB_INCLUDE_PATH and TBB_LIB_PATH.\n" )
		Exit( 1 )

	if not c.CheckLibWithHeader( "blosc" + env["BLOSC_LIB_SUFFIX"], "blosc.h", "C++") :
		sys.stderr.write( "ERROR : unable to find the Blosc libraries - check BLOSC_INCLUDE_PATH and BLOSC_LIB_PATH.\n" )
		Exit( 1 )

	if c.CheckLibWithHeader( "freetype", ["ft2build.h"], "CXX" ) :
		env.Append( CPPFLAGS = '-DIECORE_WITH_FREETYPE' )
	else :
		sys.stderr.write( "WARNING: no Freetype library found, no font support, check FREETYPE_INCLUDE_PATH and FREETYPE_LIB_PATH.\n" )

	c.Finish()

env.Append( LIBS = [
		"tbb" + env["TBB_LIB_SUFFIX"],
		"blosc" + env["BLOSC_LIB_SUFFIX"],
		"Iex" + env["OPENEXR_LIB_SUFFIX"],
		"Imath" + env["OPENEXR_LIB_SUFFIX"],
		"IlmThread" + env["OPENEXR_LIB_SUFFIX"],
		"OpenEXR" + env["OPENEXR_LIB_SUFFIX"],
		# Link Windows zlib against static library to avoid potential conflicts
		# with system provided version.
		"z" if env["PLATFORM"] != "win32" else "zlibstatic"
	]
)

Help( o.GenerateHelpText( env ) )

###########################################################################################
# An environment for building python modules
###########################################################################################

def getPythonConfig( env, flags ) :

	f = subprocess.Popen( env["PYTHON_CONFIG"] + " " + flags, env=env["ENV"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True, universal_newlines=True )
	stdOut, stdErr = f.communicate()
	r = stdOut.strip()
	if f.returncode :
		sys.stderr.write( "ERROR : Error running \"%s\".\n%s" % ( env["PYTHON_CONFIG"], stdErr ) )
		Exit( 1 )
	return r

def pythonVersion( pythonEnv ) :

	pythonExecutable = pythonEnv.subst( "$PYTHON" )
	env = pythonEnv["ENV"].copy()
	env[libraryPathEnvVar] = pythonEnv.subst( os.pathsep.join( pythonEnv["LIBPATH"] ) )
	return subprocess.check_output(
		[ pythonExecutable, "-c", 'import sys; print( \"%s.%s\" % sys.version_info[:2] )' ],
		env = env,
		universal_newlines = True
	).strip()

pythonEnv = env.Clone()

# decide where python is
if pythonEnv["PYTHON"]=="" :
	if env["PLATFORM"] == "win32" :
		sys.stderr.write( "ERROR : Python executable path must be set with PYTHON option.\n" )
		Exit( 1 )
	else:
		pythonEnv["PYTHON"] = getPythonConfig( pythonEnv, "--exec-prefix" ) + "/bin/python"

# run it to determine version
pythonEnv["PYTHON_VERSION"] = pythonVersion( pythonEnv )

# get the include path for python if we haven't been told it explicitly
# Windows does not have python-config so rely the user setting the appropriate options
if env["PLATFORM"] != "win32" :
	if pythonEnv["PYTHON_INCLUDE_PATH"]=="" :
		pythonEnv["PYTHON_INCLUDE_FLAGS"] = getPythonConfig( pythonEnv, "--includes" ).split()
	else :
		pythonEnv["PYTHON_INCLUDE_FLAGS"] = [ systemIncludeArgument, "$PYTHON_INCLUDE_PATH" ]

	pythonEnv.Append( CXXFLAGS = "$PYTHON_INCLUDE_FLAGS" )

	if env["PLATFORM"] == "posix" :
		## We really want to not have the -Wno-strict-aliasing flag, but it's necessary to stop boost
		# python warnings that don't seem to be prevented by including boost via -isystem even. Better to
		# be able to have -Werror but be missing one warning than to have no -Werror.
		## \todo This is probably only necessary for specific gcc versions where -isystem doesn't
		# fully work. Reenable when we encounter versions that work correctly.
		pythonEnv.Append( CXXFLAGS = [ "-Wno-strict-aliasing" ] )

	# get the python link flags
	if pythonEnv["PYTHON_LINK_FLAGS"]=="" :
		pythonEnv["PYTHON_LINK_FLAGS"] = getPythonConfig( pythonEnv, "--ldflags" )
		pythonEnv["PYTHON_LINK_FLAGS"] = pythonEnv["PYTHON_LINK_FLAGS"].replace( "Python.framework/Versions/" + pythonEnv["PYTHON_VERSION"] + "/Python", "" )

	pythonEnv.Append( SHLINKFLAGS = pythonEnv["PYTHON_LINK_FLAGS"].split() )
else :
	pythonEnv["PYTHON_INCLUDE_FLAGS"] = ""

pythonEnv.Append( CPPFLAGS = "-DBOOST_PYTHON_MAX_ARITY=20" )

# if BOOST_PYTHON_LIB_SUFFIX is provided, use it
boostPythonLibSuffix = pythonEnv.get( "BOOST_PYTHON_LIB_SUFFIX", None )
if boostPythonLibSuffix is None :
	boostPythonLibSuffix = pythonEnv["BOOST_LIB_SUFFIX"]
	if ( int( env["BOOST_MAJOR_VERSION"] ), int( env["BOOST_MINOR_VERSION"] ) ) >= ( 1, 67 ) :
		boostPythonLibSuffix = pythonEnv["PYTHON_VERSION"].replace( ".", "" ) + boostPythonLibSuffix

pythonEnv.Append( LIBS = [
		"boost_python" + boostPythonLibSuffix,
	]
)

if doConfigure :

	c = configureSharedLibrary( pythonEnv )

	if not c.CheckHeader( "boost/python.hpp", language = "C++" ) :
		sys.stderr.write( "ERROR : unable to find the Python headers, check PYTHON_INCLUDE_PATH.\n" )
		Exit( 1 )

	c.Finish()

pythonModuleEnv = pythonEnv.Clone()

pythonModuleEnv["SHLIBPREFIX"] = ""
pythonModuleEnv["SHLIBSUFFIX"] = ".so" if env["PLATFORM"] != "win32" else ".pyd"

###########################################################################################
# An environment for running tests
###########################################################################################

testEnv = env.Clone()

testEnvLibPath = os.pathsep.join( testEnv["LIBPATH"] )
if testEnv["TEST_LIBPATH"] != "" :
	testEnvLibPath = testEnv["TEST_LIBPATH"] + os.pathsep + testEnvLibPath
testEnvLibPath = testEnv.subst( testEnvLibPath )

if testEnv["PLATFORM"] == "darwin" :
	testEnvLibPath = os.pathsep.join( [ "/System/Library/Frameworks/ApplicationServices.framework/Versions/A/Frameworks/ImageIO.framework/Resources/", testEnvLibPath ] )

testEnv["ENV"][testEnv["TEST_LIBRARY_PATH_ENV_VAR"]] = os.pathsep.join( [ testEnv["ENV"].get(testEnv["TEST_LIBRARY_PATH_ENV_VAR"], ""), testEnvLibPath ] )
if testEnv["TEST_LIBRARY_PATH_ENV_VAR"] != libraryPathEnvVar :
	testEnv["ENV"][libraryPathEnvVar] = os.pathsep.join( [ testEnv["ENV"].get(libraryPathEnvVar, ""), testEnvLibPath ] )
testEnv["ENV"]["IECORE_DLL_DIRECTORIES"] = testEnv["ENV"][libraryPathEnvVar]
testEnv["ENV"]["IECORE_OP_PATHS"] = os.path.join( "test", "IECore", "ops" )

c = configureSharedLibrary( testEnv )

withBoostUnitTest = c.CheckLibWithHeader( env.subst( "boost_unit_test_framework" + env["BOOST_LIB_SUFFIX"] ), "boost/test/unit_test.hpp", "CXX" )
# Boost on Windows does includes the "lib" prefix for the boost_test_exec_monitor
boostTestExecMonitorLibName = "libboost_test_exec_monitor" if testEnv["PLATFORM"] == "win32" else "boost_test_exec_monitor"
withBoostTestExecMonitor = c.CheckLibWithHeader( env.subst( boostTestExecMonitorLibName + env["BOOST_LIB_SUFFIX"] ), "boost/test/test_exec_monitor.hpp", "CXX" )
withBoostTest = withBoostUnitTest and withBoostTestExecMonitor

c.Finish()

testEnv["ENV"]["PYTHONPATH"] = os.pathsep.join(
	[
		os.path.join( ".", "python" ),
		testEnv.subst( "$PYTHONPATH" )
	]
)

if testEnv["PLATFORM"] == "darwin" :

	# On macOS, SIP prevents any of the system shells from inheriting the
	# DYLD_LIBRARY_PATH. We can work around that by providing our own
	# shell-less `SPAWN` setting for the environment.

	def spawnWithoutShell( sh, escape, cmd, args, env ) :

		return subprocess.Popen( args, env = env, close_fds = True ).wait()

	testEnv["SPAWN"] = spawnWithoutShell
	# The ESCAPE setting causes quoting etc to be added to commands, which
	# would break the shell-less spawning we're doing.
	del testEnv["ESCAPE"]

###########################################################################################
# Helper functions
###########################################################################################

# Makes versioned symlinks for use during installation
def makeSymLinks( env, target ) :

	links = {
		"${IECORE_VERSION}" : "${IECORE_MILESTONE_VERSION}.${IECORE_MAJOR_VERSION}.${IECORE_MINOR_VERSION}",
		"${IECORE_MILESTONE_VERSION}.${IECORE_MAJOR_VERSION}.${IECORE_MINOR_VERSION}" : "${IECORE_COMPATIBILITY_VERSION}",
		"${IECORE_COMPATIBILITY_VERSION}" : "${IECORE_MILESTONE_VERSION}",
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
			# return if the existing link is exactly what we want already
			if os.path.realpath( target ) == os.path.realpath( source ) :
				return

			os.remove( target )
		else :
			# there's not a good reason for a non-link file to be here
			# so we won't delete it on the grounds that we don't know
			# what's happening
			sys.stderr.write( "Wanted to make a link at \"%s\", but a file exists there - aborting.\n" % target )
			Exit( 1 )

	os.symlink( relativeSource, target )

def readLinesMinusLicense( f ) :

	if isinstance( f, str ) :
		f = open( f, "r" )

	result = []
	skippedLicense = False
	for line in f.readlines() :

		if not line.startswith( "#" ) :
			skippedLicense = True
		if skippedLicense :
			result.append( line )

	return result

# Builder action that munges a nicely organised python module into a much less nicely organised one
# that doxygen will understand. Otherwise it puts every class implemented in its own file
# into its own namespace and the docs get mighty confusing.
def createDoxygenPython( target, source, env ) :

	target = str( target[0] )
	source = str( source[0] )

	if not os.path.isdir( target ) :
		os.makedirs( target )

	outFile = open( target + "/__init__.py", "w" )

	for line in readLinesMinusLicense( source ) :

		outFile.write( line )

		if line.startswith( "import" ) :

			# copy source file over to target directory
			words = line.split()
			fileName = os.path.dirname( source ) + "/" + words[1] + ".py"
			if os.path.isfile( fileName ) :
				destFile = open( target + "/" + words[1] + ".py", "w" )
				for l in readLinesMinusLicense( fileName ) :
					destFile.write( l )

		elif line.startswith( "from" ) :

			# cat source file directly into init file
			words = line.split()
			fileName = os.path.dirname( source ) + "/" + words[1] + ".py"
			if os.path.isfile( fileName ) :

				outFile.write( "\n" )

				for line in readLinesMinusLicense( fileName ) :
					outFile.write( line )

				outFile.write( "\n" )

# installs the core/python build configurations to a pkg-config configuration file
def writePkgConfig( env, python_env ):
	global ieCoreMilestoneVersion, ieCoreMajorVersion, ieCoreMinorVersion, ieCorePatchVersion
	prefix = env.subst( env['INSTALL_PREFIX'] )
	filedir = os.path.join( prefix, 'lib/pkgconfig' )
	if not os.path.exists(filedir):
		os.makedirs(filedir)
	fd = open( os.path.join( filedir, 'cortex.pc' ), 'w' )
	fd.write( "prefix=%s\n" % prefix )
	fd.write( "exec_prefix=${prefix}\n" )
	fd.write( "libdir=${prefix}/lib\n" )
	fd.write( "includedir=%s\n" % env.subst(env['INSTALL_HEADER_DIR']) )
	fd.write( "\n" )
	fd.write( "Name: Cortex\n" )
	fd.write( "Description: Open-source libraries for VFX development.\n" )
	fd.write( "Version: %d.%d.%d.%d\n" % ( ieCoreMilestoneVersion, ieCoreMajorVersion, ieCoreMinorVersion, ieCorePatchVersion ) )
	corelib = os.path.basename( env.subst("$INSTALL_LIB_NAME") )
	pythonlib = os.path.basename( python_env.subst("$INSTALL_PYTHONLIB_NAME") )
	fd.write( "Libs: -L${libdir} -l%s -l%s -L%s -lboost_python%s\n" % ( corelib,
																	pythonlib,
																	env['BOOST_LIB_PATH'],
																	python_env["BOOST_LIB_SUFFIX"] ) )
	python_includes = python_env.subst("$PYTHON_INCLUDE_FLAGS")
	openexr_includes = "-I%s -I%s/OpenEXR" % (env['OPENEXR_INCLUDE_PATH'],
											env['OPENEXR_INCLUDE_PATH'])
	if env['ILMBASE_INCLUDE_PATH']!=env['OPENEXR_INCLUDE_PATH']:
		openexr_includes = "-I%s -I%s/OpenEXR -I%s -I%s/OpenEXR" % (env['ILMBASE_INCLUDE_PATH'],
																	env['ILMBASE_INCLUDE_PATH'],
																	env['OPENEXR_INCLUDE_PATH'],
																	env['OPENEXR_INCLUDE_PATH'])
	fd.write( "Cflags: -I${includedir} %s -I%s %s\n" % (openexr_includes,
														env['BOOST_INCLUDE_PATH'],
														python_includes ) )
	fd.close()


###########################################################################################
# Build, install and test the core library and bindings
###########################################################################################

coreEnv = env.Clone( IECORE_NAME="IECore" )
coreEnv.Append( CXXFLAGS="-DIECore_EXPORTS" )
if coreEnv["PLATFORM"] == "win32" :
	coreEnv.Append( LIBS="version.lib" )
corePythonEnv = pythonEnv.Clone( IECORE_NAME="IECorePython" )
corePythonEnv.Append( CXXFLAGS="-DIECorePython_EXPORTS" )
corePythonModuleEnv = pythonModuleEnv.Clone( IECORE_NAME="IECore" )
coreTestEnv = testEnv.Clone()

allCoreEnvs = ( coreEnv, corePythonEnv, corePythonModuleEnv, coreTestEnv )

# lists of sources
coreSources = sorted( glob.glob( "src/IECore/*.cpp" ) )
coreHeaders = glob.glob( "include/IECore/*.h" ) + glob.glob( "include/IECore/*.inl" )
corePythonHeaders = glob.glob( "include/IECorePython/*.h" ) + glob.glob( "include/IECorePython/*.inl" )
corePythonSources = sorted( glob.glob( "src/IECorePython/*.cpp" ) )
corePythonModuleSources = sorted( glob.glob( "src/IECorePythonModule/*.cpp" ) )
corePythonScripts = glob.glob( "python/IECore/*.py" )

# library
coreLibrary = coreEnv.SharedLibrary( "lib/" + os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ), coreSources )
coreLibraryInstall = coreEnv.Install( os.path.dirname( coreEnv.subst( "$INSTALL_LIB_NAME" ) ), coreLibrary )
coreEnv.NoCache( coreLibraryInstall )
if env["INSTALL_CREATE_SYMLINKS"] :
	coreEnv.AddPostAction( coreLibraryInstall, lambda target, source, env : makeLibSymLinks( coreEnv ) )
coreEnv.Alias( "install", [ coreLibraryInstall ] )
coreEnv.Alias( "installCore", [ coreLibraryInstall ] )
coreEnv.Alias( "installLib", [ coreLibraryInstall ] )

# headers

# take special care for the Version header
# windows seems to return the glob matches with a delightful mix of path seperators (eg "include/IECore\\Version.h")
versionHeader = os.path.join( "include/IECore", "Version.h" )
coreHeaders.remove( versionHeader )
versionHeaderInstall = env.Substfile(
	"$INSTALL_HEADER_DIR/IECore/Version.h",
	versionHeader,
	SUBST_DICT = {
		"IE_CORE_MILESTONEVERSION": "$IECORE_MILESTONE_VERSION",
		"IE_CORE_MAJORVERSION": "$IECORE_MAJOR_VERSION",
		"IE_CORE_MINORVERSION": "$IECORE_MINOR_VERSION",
		"IE_CORE_PATCHVERSION": "$IECORE_PATCH_VERSION",
	}
)
# handle the remaining core headers
headerInstall = coreEnv.Install( "$INSTALL_HEADER_DIR/IECore", coreHeaders )
if env["INSTALL_CREATE_SYMLINKS"]:
	coreEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECore", lambda target, source, env : makeSymLinks( coreEnv, coreEnv["INSTALL_HEADER_DIR"] ) )
if env["INSTALL_PKG_CONFIG_FILE"]:
		coreEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECore", lambda target, source, env : writePkgConfig( coreEnv, corePythonEnv ) )
coreEnv.Alias( "install", [ headerInstall, versionHeaderInstall ] )
coreEnv.Alias( "installCore", [ headerInstall, versionHeaderInstall ] )

# python library
corePythonEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )
corePythonLibrary = corePythonEnv.SharedLibrary( "lib/" + os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ), corePythonSources )
corePythonLibraryInstall = corePythonEnv.Install( os.path.dirname( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ), corePythonLibrary )
corePythonEnv.NoCache( corePythonLibraryInstall )
if env["INSTALL_CREATE_SYMLINKS"] :
	corePythonEnv.AddPostAction( corePythonLibraryInstall, lambda target, source, env : makeLibSymLinks( corePythonEnv, libNameVar="INSTALL_PYTHONLIB_NAME" ) )
corePythonEnv.Alias( "install", [ corePythonLibraryInstall ] )
corePythonEnv.Alias( "installCore", [ corePythonLibraryInstall ] )
corePythonEnv.Alias( "installLib", [ corePythonLibraryInstall ] )

# python headers
pythonHeaderInstall = coreEnv.Install( "$INSTALL_HEADER_DIR/IECorePython", corePythonHeaders )
coreEnv.Alias( "install", pythonHeaderInstall )
coreEnv.Alias( "installCore", pythonHeaderInstall )

# python module
corePythonModuleEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )
corePythonModuleEnv.Append( LIBS = os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ) )
corePythonModule = corePythonModuleEnv.SharedLibrary( "python/IECore/_IECore", corePythonModuleSources )
corePythonModuleEnv.Depends( corePythonModule, coreLibrary )
corePythonModuleEnv.Depends( corePythonModule, corePythonLibrary )

corePythonModuleInstall = corePythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECore", corePythonScripts + corePythonModule )
if env["INSTALL_CREATE_SYMLINKS"]:
	corePythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECore", lambda target, source, env : makeSymLinks( corePythonEnv, corePythonEnv["INSTALL_PYTHON_DIR"] ) )
corePythonModuleEnv.Alias( "install", corePythonModuleInstall )
corePythonModuleEnv.Alias( "installCore", corePythonModuleInstall )

# stubs
for cls in env['INSTALL_IECORE_OPS'] :
	stubName = os.path.basename( cls[1] )
	stubEnv = corePythonModuleEnv.Clone( IECORE_NAME=os.path.join( cls[1], stubName ) )
	stubInstall = stubEnv.Command( "$INSTALL_IECORE_OP_PATH", None, 'echo "from %s import %s as %s" > $TARGET' % ( cls[0].rpartition( "." )[0], cls[0].rpartition( "." )[-1], stubName ) )
	if env[ "INSTALL_CREATE_SYMLINKS" ] :
		stubEnv.AddPostAction( stubInstall, lambda target, source, env : makeSymLinks( env, env["INSTALL_IECORE_OP_PATH"] ) )
	stubEnv.Alias( "install", stubInstall )
	stubEnv.Alias( "installCore", stubInstall )
	stubEnv.Alias( "installStubs", stubInstall )

Default( coreLibrary, corePythonLibrary, corePythonModule )

# post installation script

if coreEnv["INSTALL_CORE_POST_COMMAND"]!="" :
	# this is the only way we could find to get a post action to run for an alias
	corePythonEnv.Alias( "install", corePythonModuleInstall, "$INSTALL_CORE_POST_COMMAND" )
	corePythonEnv.Alias( "installCore", corePythonModuleInstall, "$INSTALL_CORE_POST_COMMAND" )

# testing

if withBoostTest:
	coreTestEnv.Append(
		LIBS = [
			os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
			"boost_unit_test_framework$BOOST_LIB_SUFFIX",
			boostTestExecMonitorLibName + "$BOOST_LIB_SUFFIX",
		],
		CPPPATH = [ "test/IECore" ],
	)

	coreTestSources = glob.glob( "test/IECore/*.cpp" )
	coreTestProgram = coreTestEnv.Program( "test/IECore/IECoreTest", coreTestSources )

	coreTest = coreTestEnv.Command(
		os.path.join( "test", "IECore", "results.txt" ),
		coreTestProgram,
		"{} --report_sink={}".format(
			os.path.join( "test", "IECore", "IECoreTest" ),
			os.path.join( "test", "IECore", "results.txt" )
		)
	)
	NoCache( coreTest )
	coreTestEnv.Alias( "testCore", coreTest )

corePythonTest = coreTestEnv.Command( "test/IECore/resultsPython.txt", corePythonModule, "$PYTHON $TEST_CORE_SCRIPT --verbose" )
coreTestEnv.Depends( corePythonTest, glob.glob( "test/IECore/*.py" ) )
NoCache( corePythonTest )
coreTestEnv.Alias( "testCorePython", corePythonTest )

###########################################################################################
# Build, install and test the IECoreImage library and bindings
###########################################################################################

imageEnvSets = {
	"IECORE_NAME" : "IECoreImage",
}

imageEnv = env.Clone( **imageEnvSets )

imageEnvPrepends = {
	"LIBPATH" : [
		"$OIIO_LIB_PATH",
	],
	"LIBS" : [
		"OpenImageIO$OIIO_LIB_SUFFIX",
	],
	"CXXFLAGS" : [
		"-DIECoreImage_EXPORTS",
		systemIncludeArgument, "$OIIO_INCLUDE_PATH"
	]
}
if imageEnv.get( "WITH_OIIO_UTIL", True ):
	imageEnvPrepends["LIBS"].append( "OpenImageIO_Util$OIIO_LIB_SUFFIX" )

imageEnv.Prepend( **imageEnvPrepends )
# Windows uses PATH for to find libraries, we must append to it to make sure we don't overwrite existing PATH entries.
# On Linux and MacOS this will append to an empty library path.
imageEnv["ENV"][libraryPathEnvVar] = os.pathsep.join( [ imageEnv["ENV"].get(libraryPathEnvVar, "") ] + imageEnv["LIBPATH"] )

if doConfigure :

	c = configureSharedLibrary( imageEnv )

	if not c.CheckLibWithHeader( imageEnv.subst( "OpenImageIO$OIIO_LIB_SUFFIX" ), "OpenImageIO/imageio.h", "CXX" ) :

		sys.stderr.write( "ERROR : unable to find the OpenImageIO libraries - check OIIO_INCLUDE_PATH and OIIO_LIB_PATH.\n" )
		c.Finish()

	else :

		c.Finish()

		# we can't add this earlier as then it's built during the configure stage, and that's no good
		imageEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )

		# source list
		imageSources = sorted( glob.glob( "src/IECoreImage/*.cpp" ) )
		imageHeaders = sorted( glob.glob( "include/IECoreImage/*.h" ) + glob.glob( "include/IECoreImage/*.inl" ) )
		imagePythonHeaders = sorted( glob.glob( "include/IECoreImageBindings/*.h" ) + glob.glob( "include/IECoreImageBindings/*.inl" ) )
		imagePythonSources = sorted( glob.glob( "src/IECoreImageBindings/*.cpp" ) )
		imagePythonModuleSources = sorted( glob.glob( "src/IECoreImageModule/*.cpp" ) )
		imagePythonScripts = glob.glob( "python/IECoreImage/*.py" )

		if "-DIECORE_WITH_FREETYPE" in imageEnv["CPPFLAGS"] :
			imageEnv.Append( LIBS = "freetype" )
		else :
			imageSources.remove( "src/IECoreImage/Font.cpp" )
			imagePythonSources.remove( "src/IECoreImageBindings/FontBinding.cpp" )

		# library
		imageLibrary = imageEnv.SharedLibrary( "lib/" + os.path.basename( imageEnv.subst( "$INSTALL_LIB_NAME" ) ), imageSources )
		imageLibraryInstall = imageEnv.Install( os.path.dirname( imageEnv.subst( "$INSTALL_LIB_NAME" ) ), imageLibrary )
		imageEnv.NoCache( imageLibraryInstall )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			imageEnv.AddPostAction( imageLibraryInstall, lambda target, source, env : makeLibSymLinks( imageEnv ) )
		imageEnv.Alias( "install", imageLibraryInstall )
		imageEnv.Alias( "installImage", imageLibraryInstall )
		imageEnv.Alias( "installLib", [ imageLibraryInstall ] )

		# headers
		imageHeaderInstall = imageEnv.Install( "$INSTALL_HEADER_DIR/IECoreImage", imageHeaders )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			imageEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreImage", lambda target, source, env : makeSymLinks( imageEnv, imageEnv["INSTALL_HEADER_DIR"] ) )
		imageEnv.Alias( "install", imageHeaderInstall )
		imageEnv.Alias( "installImage", imageHeaderInstall )

		# python headers
		imagePythonHeaderInstall = imageEnv.Install( "$INSTALL_HEADER_DIR/IECoreImageBindings", imagePythonHeaders )
		imageEnv.Alias( "install", imagePythonHeaderInstall )
		imageEnv.Alias( "installImage", imagePythonHeaderInstall )

		# python module
		imagePythonModuleEnv = corePythonModuleEnv.Clone( **imageEnvSets )
		imagePythonModuleEnv.Append( **imageEnvPrepends )
		imagePythonModuleEnv.Append(
			LIBS = [
				os.path.basename( imageEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)
		imagePythonModule = imagePythonModuleEnv.SharedLibrary( "python/IECoreImage/_IECoreImage", imagePythonSources + imagePythonModuleSources )
		imagePythonModuleEnv.Depends( imagePythonModule, imageLibrary )

		# python module install
		imagePythonModuleInstall = imagePythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreImage", imagePythonScripts + imagePythonModule )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			imagePythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreImage", lambda target, source, env : makeSymLinks( imagePythonModuleEnv, imagePythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
		imagePythonModuleEnv.Alias( "install", imagePythonModuleInstall )
		imagePythonModuleEnv.Alias( "installImage", imagePythonModuleInstall )

		Default( [ imageLibrary, imagePythonModule ] )

		# post installation script
		if imageEnv["INSTALL_COREIMAGE_POST_COMMAND"]!="" :
			# this is the only way we could find to get a post action to run for an alias
			imagePythonModuleEnv.Alias( "install", imagePythonModuleInstall, "$INSTALL_COREIMAGE_POST_COMMAND" )
			imagePythonModuleEnv.Alias( "installImage", imagePythonModuleInstall, "$INSTALL_COREIMAGE_POST_COMMAND" )

		# testing
		imageTestEnv = testEnv.Clone()
		imageTestEnv["ENV"]["PYTHONPATH"] = os.pathsep.join( [ imageTestEnv["ENV"]["PYTHONPATH"], "python" ] )

		imageEnvLibPath = os.pathsep.join( imageEnvPrepends["LIBPATH"] )
		imageLibs = imageTestEnv.subst( imageEnvLibPath )
		imageTestLibs = imageTestEnv["ENV"][imageTestEnv["TEST_LIBRARY_PATH_ENV_VAR"]]
		imageTestEnv["ENV"][imageTestEnv["TEST_LIBRARY_PATH_ENV_VAR"]] = os.pathsep.join( [imageLibs, imageTestLibs] )

		imageTest = imageTestEnv.Command( "test/IECoreImage/results.txt", imagePythonModule, "$PYTHON $TEST_IMAGE_SCRIPT --verbose" )
		NoCache( imageTest )
		imageTestEnv.Depends( imageTest, [ corePythonModule + imagePythonModule ]  )
		imageTestEnv.Depends( imageTest, glob.glob( "test/IECoreImage/*.py" ) )
		imageTestEnv.Alias( "testImage", imageTest )

###########################################################################################
# Build, install and test the scene library and bindings
###########################################################################################

sceneEnv = env.Clone( IECORE_NAME="IECoreScene" )
scenePythonModuleEnv = corePythonModuleEnv.Clone( IECORE_NAME="IECoreScene" )

sceneSources = sorted( glob.glob( "src/IECoreScene/*.cpp" ) )
sceneHeaders = glob.glob( "include/IECoreScene/*.h" ) + glob.glob( "include/IECoreScene/*.inl" )
scenePythonModuleSources = sorted( glob.glob( "src/IECoreScene/bindings/*.cpp" ) )
scenePythonScripts = glob.glob( "python/IECoreScene/*.py" )

if doConfigure :

	sceneEnv.Append(
		LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
		CXXFLAGS = "-DIECoreScene_EXPORTS"
	)

	if "-DIECORE_WITH_FREETYPE" in sceneEnv["CPPFLAGS"] :
		sceneEnv.Append( LIBS = "freetype" )
	else :
		sceneSources.remove( "src/IECoreScene/Font.cpp" )
		scenePythonModuleSources.remove( "src/IECoreScene/bindings/FontBinding.cpp" )

	# library
	sceneLibrary = sceneEnv.SharedLibrary( "lib/" + os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ), sceneSources )
	sceneLibraryInstall = sceneEnv.Install( os.path.dirname( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ), sceneLibrary )
	sceneEnv.NoCache( sceneLibraryInstall )
	if env[ "INSTALL_CREATE_SYMLINKS" ] :
		sceneEnv.AddPostAction( sceneLibraryInstall, lambda target, source, env : makeLibSymLinks( sceneEnv ) )
	sceneEnv.Alias( "install", [ sceneLibraryInstall ] )
	sceneEnv.Alias( "installScene", [ sceneLibraryInstall ] )
	sceneEnv.Alias( "installSceneLib", [ sceneLibraryInstall ] )

	# headers
	sceneHeaderInstall = sceneEnv.Install( "$INSTALL_HEADER_DIR/IECoreScene", sceneHeaders )
	if env[ "INSTALL_CREATE_SYMLINKS" ] :
		sceneEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreScene", lambda target, source, env : makeSymLinks( sceneEnv, sceneEnv["INSTALL_HEADER_DIR"] ) )
	sceneEnv.Alias( "install", sceneHeaderInstall )
	sceneEnv.Alias( "installScene", sceneHeaderInstall )

	# python module
	scenePythonModuleEnv.Append( LIBS = os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ) )
	scenePythonModule = scenePythonModuleEnv.SharedLibrary( "python/IECoreScene/_IECoreScene", scenePythonModuleSources )
	scenePythonModuleEnv.Depends( scenePythonModule, coreLibrary )
	scenePythonModuleEnv.Depends( scenePythonModule, corePythonLibrary )
	scenePythonModuleEnv.Depends( scenePythonModule, sceneLibrary )

	scenePythonModuleInstall = scenePythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreScene", scenePythonScripts + scenePythonModule )
	if env[ "INSTALL_CREATE_SYMLINKS" ] :
		scenePythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreScene", lambda target, source, env : makeSymLinks( scenePythonModuleEnv, scenePythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
	scenePythonModuleEnv.Alias( "install", scenePythonModuleInstall )
	scenePythonModuleEnv.Alias( "installScene", scenePythonModuleInstall )

	Default( sceneLibrary, scenePythonModule )

	# post installation script
	if sceneEnv["INSTALL_CORESCENE_POST_COMMAND"]!="" :
		# this is the only way we could find to get a post action to run for an alias
		scenePythonModuleEnv.Alias( "install", scenePythonModuleInstall, "$INSTALL_CORESCENE_POST_COMMAND" )
		scenePythonModuleEnv.Alias( "installScene", scenePythonModuleInstall, "$INSTALL_CORESCENE_POST_COMMAND" )

	# testing
	sceneTestEnv = testEnv.Clone()
	sceneTest = coreTestEnv.Command( "test/IECoreScene/results.txt", scenePythonModule, "$PYTHON $TEST_SCENE_SCRIPT --verbose" )
	sceneTestEnv.Depends( sceneTest, [ corePythonModule ] )
	NoCache( sceneTest )
	sceneTestEnv.Alias( "testScene", sceneTest )


###########################################################################################
# Build, install and test the VDB library and bindings
###########################################################################################

vdbEnvSets = {
	"IECORE_NAME" : "IECoreVDB"
}

vdbEnv = env.Clone( **vdbEnvSets )

vdbEnvPrepends = {
	"LIBPATH" : [
		"$VDB_LIB_PATH",
	],
	"LIBS" : ["openvdb$VDB_LIB_SUFFIX"],
	"CXXFLAGS" : [
		systemIncludeArgument, "$VDB_INCLUDE_PATH",
		systemIncludeArgument, "$PYBIND11_INCLUDE_PATH",
	]
}

vdbEnv.Prepend( **vdbEnvPrepends)

vdbPythonModuleEnv = corePythonModuleEnv.Clone( **vdbEnvSets )
vdbPythonModuleEnv.Prepend( **vdbEnvPrepends )

vdbSources = sorted( glob.glob( "src/IECoreVDB/*.cpp" ) )
vdbHeaders = glob.glob( "include/IECoreVDB/*.h" ) + glob.glob( "include/IECoreVDB/*.inl" )
vdbPythonModuleSources = sorted( glob.glob( "src/IECoreVDB/bindings/*.cpp" ) )
vdbPythonScripts = glob.glob( "python/IECoreVDB/*.py" )

if doConfigure :

	c = configureSharedLibrary( vdbEnv )

	haveVDB = False
	if c.CheckLibWithHeader( vdbEnv.subst( "openvdb" + env["VDB_LIB_SUFFIX"] ), "openvdb/openvdb.h", "CXX" ) :
		haveVDB = True
	else :
		sys.stderr.write( "WARNING : no OpenVDB library found, not building IECoreVDB - check VDB_INCLUDE_PATH, VDB_LIB_PATH and config.log.\n" )

	c.Finish()

	if haveVDB :

		vdbEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) )
			],
			CXXFLAGS = "-DIECoreVDB_EXPORTS"
		)

		# library
		vdbLibrary = vdbEnv.SharedLibrary( "lib/" + os.path.basename( vdbEnv.subst( "$INSTALL_LIB_NAME" ) ), vdbSources )
		vdbLibraryInstall = vdbEnv.Install( os.path.dirname( vdbEnv.subst( "$INSTALL_LIB_NAME" ) ), vdbLibrary )
		vdbEnv.NoCache( vdbLibraryInstall )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			vdbEnv.AddPostAction( vdbLibraryInstall, lambda target, source, env : makeLibSymLinks( vdbEnv ) )
		vdbEnv.Alias( "install", [ vdbLibraryInstall ] )
		vdbEnv.Alias( "installVDB", [ vdbLibraryInstall ] )
		vdbEnv.Alias( "installVDBLib", [ vdbLibraryInstall ] )

		# headers
		vdbHeaderInstall = sceneEnv.Install( "$INSTALL_HEADER_DIR/IECoreVDB", vdbHeaders )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			sceneEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreVDB", lambda target, source, env : makeSymLinks( vdbEnv, vdbEnv["INSTALL_HEADER_DIR"] ) )
		sceneEnv.Alias( "install", vdbHeaderInstall )
		sceneEnv.Alias( "installVDB", vdbHeaderInstall )

		# python module
		vdbPythonModuleEnv.Append(
			LIBS = [
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( vdbEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)
		vdbPythonModule = vdbPythonModuleEnv.SharedLibrary( "python/IECoreVDB/_IECoreVDB", vdbPythonModuleSources )
		vdbPythonModuleEnv.Depends( vdbPythonModule, vdbLibrary )

		vdbPythonModuleInstall = vdbPythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreVDB", vdbPythonScripts + vdbPythonModule )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			vdbPythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreVDB", lambda target, source, env : makeSymLinks( vdbPythonModuleEnv, vdbPythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
		vdbPythonModuleEnv.Alias( "install", vdbPythonModuleInstall )
		vdbPythonModuleEnv.Alias( "installVDB", vdbPythonModuleInstall )

		Default( vdbLibrary, vdbPythonModule )

		# testing
		vdbTestEnv = testEnv.Clone()

		vdbTestEnv["ENV"]["PYTHONPATH"] = vdbTestEnv["ENV"]["PYTHONPATH"] + os.pathsep + vdbTestEnv["VDB_PYTHON_PATH"]

		vdbTest = vdbTestEnv.Command( "test/IECoreVDB/results.txt", vdbPythonModule, "$PYTHON $TEST_VDB_SCRIPT --verbose" )
		vdbTestEnv.Depends( vdbTest, [ corePythonModule + scenePythonModule + vdbPythonModule ] )
		NoCache( vdbTest )
		vdbTestEnv.Alias( "testVDB", vdbTest )

###########################################################################################
# Build, install and test the optional CoreGL library and bindings
###########################################################################################

if env["WITH_GL"] and doConfigure :

	glEnvSets = {
		"IECORE_NAME" : "IECoreGL",
	}

	glEnv = env.Clone( **glEnvSets )

	glEnvAppends = {
		"CXXFLAGS" : [
			# These are to work around warnings in boost::wave
			# while still using -Werror.
			"-Wno-format" if env["PLATFORM"] != "win32" else "",
			"-Wno-strict-aliasing" if env["PLATFORM"] != "win32" else "",
			"/wd4701" if env["PLATFORM"] == "win32" else "",
			systemIncludeArgument, "$GLEW_INCLUDE_PATH",
			systemIncludeArgument, "$OIIO_INCLUDE_PATH",
		],
		"LIBPATH" : [
			"$GLEW_LIB_PATH",
			"$OIIO_LIB_PATH",
		],
	}

	glEnv.Append( **glEnvAppends )
	glEnv.Append( CXXFLAGS = "-DIECoreGL_EXPORTS")

	c = configureSharedLibrary( glEnv )

	if not c.CheckLibWithHeader( env.subst( "GLEW$GLEW_LIB_SUFFIX" ), "GL/glew.h", "CXX" ) :

		sys.stderr.write( "WARNING : GLEW library not found, not building IECoreGL - check GLEW_INCLUDE_PATH and GLEW_LIB_PATH.\n" )
		c.Finish()

	elif not c.CheckLibWithHeader( env.subst( "boost_wave" + env["BOOST_LIB_SUFFIX"] ), "boost/wave.hpp", "CXX" ) :

		sys.stderr.write( "WARNING : boost_wave library not found, not building IECoreGL - check BOOST_INCLUDE_PATH and BOOST_LIB_PATH.\n" )
		c.Finish()

	else :

		c.Finish()

		# we can't add this earlier as then it's built during the configure stage, and that's no good
		glEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( imageEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
				"OpenImageIO$OIIO_LIB_SUFFIX",
				"GLEW$GLEW_LIB_SUFFIX",
				"boost_wave$BOOST_LIB_SUFFIX",
			]
		)
		if glEnv.get( "WITH_OIIO_UTIL", True ):
			glEnv.Append( LIBS = [ "OpenImageIO_Util$OIIO_LIB_SUFFIX", ] )

		if env["PLATFORM"]=="darwin" :
			glEnv.Append(
				FRAMEWORKS = [
					"OpenGL",
				],
				CPPDEFINES = [ "GL_SILENCE_DEPRECATION" ],
			)
		elif env["PLATFORM"] == "win32" :
			glEnv.Append(
				LIBS = [
					"Gdi32",
					"User32",
					glEnv.subst( "opengl$GLEW_LIB_SUFFIX" ),
					glEnv.subst( "glu$GLEW_LIB_SUFFIX" ),
				]
			)
		else :
			glEnv.Append(
				LIBS = [
					"GL",
					"GLU",
				]
			)

		glSources = sorted( glob.glob( "src/IECoreGL/*.cpp" ) )
		glPythonSources = sorted( glob.glob( "src/IECoreGL/bindings/*.cpp" ) )
		if "-DIECORE_WITH_FREETYPE" in glEnv["CPPFLAGS"] :
			glEnv.Append( LIBS = "freetype" )
		else :
			glSources.remove( "src/IECoreGL/Font.cpp" )
			glSources.remove( "src/IECoreGL/FontLoader.cpp" )
			glSources.remove( "src/IECoreGL/TextPrimitive.cpp" )
			glPythonSources.remove( "src/IECoreGL/bindings/FontBinding.cpp" )
			glPythonSources.remove( "src/IECoreGL/bindings/FontLoaderBinding.cpp" )

		glLibrary = glEnv.SharedLibrary( "lib/" + os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ), glSources )
		glLibraryInstall = glEnv.Install( os.path.dirname( glEnv.subst( "$INSTALL_LIB_NAME" ) ), glLibrary )
		glEnv.NoCache( glLibraryInstall )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			glEnv.AddPostAction( glLibraryInstall, lambda target, source, env : makeLibSymLinks( glEnv ) )
		glEnv.Alias( "install", glLibraryInstall )
		glEnv.Alias( "installGL", glLibraryInstall )
		glEnv.Alias( "installLib", [ glLibraryInstall ] )

		glHeaders = glob.glob( "include/IECoreGL/*.h" ) + glob.glob( "include/IECoreGL/*.inl" )
		glHeaderInstall = glEnv.Install( "$INSTALL_HEADER_DIR/IECoreGL", glHeaders )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			glEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreGL", lambda target, source, env : makeSymLinks( glEnv, glEnv["INSTALL_HEADER_DIR"] ) )
		glEnv.Alias( "install", glHeaderInstall )
		glEnv.Alias( "installGL", glHeaderInstall )

		glslHeaders = glob.glob( "glsl/IECoreGL/*.h" )
		glslHeaderInstall = glEnv.Install( "$INSTALL_GLSL_HEADER_DIR/IECoreGL", glslHeaders )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			glEnv.AddPostAction( "$INSTALL_GLSL_HEADER_DIR/IECoreGL", lambda target, source, env : makeSymLinks( glEnv, glEnv["INSTALL_GLSL_HEADER_DIR"] ) )
		glEnv.Alias( "install", glslHeaderInstall )
		glEnv.Alias( "installGL", glslHeaderInstall )

		glslShaderFiles = glob.glob( "glsl/*.frag" ) + glob.glob( "glsl/*.vert" )
		glslShaderInstall = glEnv.Install( "$INSTALL_GLSL_SHADER_DIR", glslShaderFiles )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			glEnv.AddPostAction( "$INSTALL_GLSL_SHADER_DIR", lambda target, source, env : makeSymLinks( glEnv, glEnv["INSTALL_GLSL_SHADER_DIR"] ) )
		glEnv.Alias( "install", glslShaderInstall )
		glEnv.Alias( "installGL", glslShaderInstall )

		glPythonModuleEnv = corePythonModuleEnv.Clone( **glEnvSets )
		glPythonModuleEnv.Append( **glEnvAppends )
		glPythonModuleEnv.Append(
			LIBS = [
				os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( imageEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)
		glPythonModule = glPythonModuleEnv.SharedLibrary( "python/IECoreGL/_IECoreGL", glPythonSources )
		glPythonModuleEnv.Depends( glPythonModule, glLibrary )

		glPythonScripts = glob.glob( "python/IECoreGL/*.py" )
		glPythonModuleInstall = glPythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreGL", glPythonScripts + glPythonModule )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			glPythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreGL", lambda target, source, env : makeSymLinks( glPythonModuleEnv, glPythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
		glPythonModuleEnv.Alias( "install", glPythonModuleInstall )
		glPythonModuleEnv.Alias( "installGL", glPythonModuleInstall )

		if coreEnv["INSTALL_COREGL_POST_COMMAND"]!="" :
			# this is the only way we could find to get a post action to run for an alias
			glPythonModuleEnv.Alias( "install", glPythonModuleInstall, "$INSTALL_COREGL_POST_COMMAND" )
			glPythonModuleEnv.Alias( "installGL", glPythonModuleInstall, "$INSTALL_COREGL_POST_COMMAND" )

		Default( [ glLibrary, glPythonModule ] )

		glTestEnv = testEnv.Clone()
		glTestEnv["ENV"]["PYTHONPATH"] = glTestEnv["ENV"]["PYTHONPATH"] + os.pathsep + "python"
		glTestEnv["ENV"]["IECOREGL_SHADER_INCLUDE_PATHS"] = "./glsl"
		for e in ["DISPLAY", "XAUTHORITY"] :
			if e in os.environ :
				glTestEnv["ENV"][e] = os.environ[e]

		glTest = glTestEnv.Command( "test/IECoreGL/results.txt", glPythonModule, "$PYTHON $TEST_GL_SCRIPT --verbose" )
		NoCache( glTest )
		glTestEnv.Depends( glTest, glLibrary )
		glTestEnv.Depends( glTest, corePythonModule )
		glTestEnv.Depends( glTest, imagePythonModule )
		glTestEnv.Depends( glTest, glob.glob( "test/IECoreGL/*.py" ) )
		glTestEnv.Alias( "testGL", glTest )

###########################################################################################
# Build, install and test the IECoreUSD library and bindings
###########################################################################################

usdEnvSets = {
	"IECORE_NAME" : "IECoreUSD"
}

# We are deliberately cloning from `pythonEnv` rather than
# `env` because USD itself has dependencies on Python.
usdEnv = pythonEnv.Clone( **usdEnvSets )

if usdEnv["WITH_USD_MONOLITHIC"] :
	usdLibs = [ "usd_ms" ]
else :
	usdLibs = [
		"usd",
		"usdGeom",
		"usdLux",
		"usdSkel",
		"usdShade",
		"usdUtils",
		"sdf",
		"tf",
		"pcp",
		"arch",
		"gf",
		"js",
		"vt",
		"ar",
		"plug",
		"trace",
		"kind",
		"work"
	]

if usdEnv["USD_LIB_PREFIX"] :
	usdLibs = [ usdEnv["USD_LIB_PREFIX"] + x for x in usdLibs ]

usdEnvAppends = {
	"CXXFLAGS" : [
		"-Wno-deprecated" if env["PLATFORM"] != "win32" else "",
		"/Zc:inline-" if env["PLATFORM"] == "win32" else "",
		"-DIECoreUSD_EXPORTS",
		systemIncludeArgument, "$USD_INCLUDE_PATH",
		systemIncludeArgument, "$PYTHON_INCLUDE_PATH",
	],
	"CPPPATH" : [
		"contrib/IECoreUSD/include",
		"contrib/IECoreUSD/src",
	],
	"LIBPATH" : [
		"$USD_LIB_PATH"
	],
	"LIBS" : usdLibs,
}

usdEnv.Append( **usdEnvAppends )

usdPythonModuleEnv = pythonModuleEnv.Clone( **usdEnvSets )
usdPythonModuleEnv.Append( **usdEnvAppends )

# libIECoreUSD doesn't export any symbols we use, but is
# required to register the SceneInterface. Ensure the linker
# doesn't optimise it away.
if env["PLATFORM"] == "posix" :
	usdPythonModuleEnv.Append( LINKFLAGS = "-Wl,--no-as-needed" )

if doConfigure :

	c = configureSharedLibrary( usdEnv )

	haveUSD = False
	if c.CheckLibWithHeader( usdLibs[0], "pxr/usd/usd/api.h", "CXX" ) :
		haveUSD = True
	else :
		sys.stderr.write( "WARNING : no USD library found, not building IECoreUSD - check USD_INCLUDE_PATH, USD_LIB_PATH and config.log.\n" )

	c.Finish()

	if haveUSD :

		usdSources = sorted( glob.glob( "contrib/IECoreUSD/src/IECoreUSD/*.cpp" ) )
		usdPythonScripts = glob.glob( "contrib/IECoreUSD/python/IECoreUSD/*.py" )
		usdPythonSources = sorted( glob.glob( "contrib/IECoreUSD/src/IECoreUSD/bindings/*.cpp" ) )
		usdHeaders = glob.glob( "contrib/IECoreUSD/include/IECoreUSD/*.h" ) + glob.glob( "contrib/IECoreUSD/include/IECoreUSD/*.inl" )

		# we can't append this before configuring, as then it gets built as
		# part of the configure process
		usdEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)

		if haveVDB :
			usdEnv.Prepend( **vdbEnvPrepends )
			usdEnv.Append(
				LIBS = [
					os.path.basename( vdbEnv.subst( "$INSTALL_LIB_NAME" ) ),
					"${USD_LIB_PREFIX}usdVol",
				],
				CPPDEFINES = [ "IECOREUSD_WITH_OPENVDB" ]
			)
		else :
			usdSources = [ f for f in usdSources if os.path.basename( f ) != "VolumeAlgo.cpp" ]

		# library
		usdLibrary = usdEnv.SharedLibrary( "lib/" + os.path.basename( usdEnv.subst( "$INSTALL_USDLIB_NAME" ) ), usdSources )
		usdLibraryInstall = usdEnv.Install( os.path.dirname( usdEnv.subst( "$INSTALL_USDLIB_NAME" ) ), usdLibrary )
		usdEnv.NoCache( usdLibraryInstall )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			usdEnv.AddPostAction( usdLibraryInstall, lambda target, source, env : makeLibSymLinks( usdEnv ) )
		usdEnv.Alias( "install", usdLibraryInstall )
		usdEnv.Alias( "installUSD", usdLibraryInstall )
		usdEnv.Alias( "installLib", [ usdLibraryInstall ] )

		# headers
		usdHeaderInstall = usdEnv.Install( "$INSTALL_HEADER_DIR/IECoreUSD", usdHeaders )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			usdEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreUSD", lambda target, source, env : makeSymLinks( usdEnv, usdEnv["INSTALL_HEADER_DIR"] ) )
		usdEnv.Alias( "install", usdHeaderInstall )
		usdEnv.Alias( "installUSD", usdHeaderInstall )

		# resources
		usdResourceInstall = usdEnv.Substfile(
			"$INSTALL_USD_RESOURCE_DIR/IECoreUSD/plugInfo.json",
			"contrib/IECoreUSD/resources/plugInfo.json",
			SUBST_DICT = {
				"!IECOREUSD_RELATIVE_LIB_FOLDER!" : os.path.relpath(
					usdLibraryInstall[0].get_path(),
					os.path.dirname( usdEnv.subst( "$INSTALL_USD_RESOURCE_DIR/IECoreUSD/plugInfo.json" ) )
				).replace( "\\", "\\\\" ),
			}
		)
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			usdEnv.AddPostAction( "$INSTALL_USD_RESOURCE_DIR/IECoreUSD", lambda target, source, env : makeSymLinks( usdEnv, usdEnv["INSTALL_USD_RESOURCE_DIR"] ) )
		usdEnv.Alias( "install", usdResourceInstall )
		usdEnv.Alias( "installUSD", usdResourceInstall )

		# python module
		usdPythonModuleEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ),
				os.path.basename( usdEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)
		usdPythonModule = usdPythonModuleEnv.SharedLibrary( "contrib/IECoreUSD/python/IECoreUSD/_IECoreUSD", usdPythonSources )
		usdPythonModuleEnv.Depends( usdPythonModule, usdLibrary )

		usdPythonModuleInstall = usdPythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreUSD", usdPythonScripts + usdPythonModule )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			usdPythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreUSD", lambda target, source, env : makeSymLinks( usdPythonModuleEnv, usdPythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
		usdPythonModuleEnv.Alias( "install", usdPythonModuleInstall )
		usdPythonModuleEnv.Alias( "installUSD", usdPythonModuleInstall )

		Default( [ usdLibrary, usdPythonModule ] )

		# tests
		usdTestEnv = testEnv.Clone()
		usdTestEnv["ENV"]["PYTHONPATH"] += os.pathsep + "./contrib/IECoreUSD/python" + os.pathsep + usdTestEnv["VDB_PYTHON_PATH"]

		usdLibPath = coreEnv.subst("$USD_LIB_PATH")
		usdPythonPath = os.path.join(usdLibPath, "python")

		usdTestEnv["ENV"]["PYTHONPATH"] += os.pathsep + usdPythonPath
		usdTestEnv["ENV"][testEnv["TEST_LIBRARY_PATH_ENV_VAR"]] += os.pathsep + usdLibPath
		usdTestEnv["ENV"]["USD_SHADE_MATERIAL_BINDING_API_CHECK"] = "strict"

		# setup pluginInfo for custom file format registration
		testSdfPlugInfo = os.path.join( os.getcwd(), "plugins", "usd", "plugInfo.json" )
		usdTestResourceInstall = usdEnv.Substfile(
			testSdfPlugInfo,
			"contrib/IECoreUSD/resources/plugInfo.json",
			SUBST_DICT = {
				"!IECOREUSD_RELATIVE_LIB_FOLDER!" : os.path.join( os.getcwd(), "lib", os.path.basename( usdLibraryInstall[0].get_path() ) ).replace("\\", "\\\\"),
			}
		)
		usdTestEnv["ENV"]["PXR_PLUGINPATH_NAME"] = testSdfPlugInfo
		usdTest = usdTestEnv.Command( "contrib/IECoreUSD/test/IECoreUSD/results.txt", usdPythonModule, "$PYTHON $TEST_USD_SCRIPT --verbose" )
		usdTestEnv.Depends( usdTest, [ corePythonModule + scenePythonModule + usdPythonModule + usdTestResourceInstall ] )
		NoCache( usdTest )
		usdTestEnv.Alias( "testUSD", usdTest )


###########################################################################################
# Build, install and test the IECoreAlembic library and bindings
###########################################################################################

alembicEnvSets = {
	"IECORE_NAME" : "IECoreAlembic"
}

alembicEnv = env.Clone( **alembicEnvSets )

alembicEnvPrepends = {
	"CXXFLAGS" : [
		systemIncludeArgument, "$ALEMBIC_INCLUDE_PATH",
		systemIncludeArgument, "$HDF5_INCLUDE_PATH",
	],
	"CPPPATH" : [
		"contrib/IECoreAlembic/include",
	],
	"LIBPATH" : [
		"$ALEMBIC_LIB_PATH",
		"$HDF5_LIB_PATH",
	],
	"LIBS" : [
		"hdf5$HDF5_LIB_SUFFIX",
	],
}

alembicEnv.Prepend( **alembicEnvPrepends )
alembicEnv.Append( CXXFLAGS = "-DIECoreAlembic_EXPORTS" )

alembicPythonModuleEnv = pythonModuleEnv.Clone( **alembicEnvSets )
alembicPythonModuleEnv.Prepend( **alembicEnvPrepends )

if doConfigure :

	c = configureSharedLibrary( alembicEnv )

	haveAlembic = False
	if c.CheckLibWithHeader( alembicEnv.subst( "Alembic" + env["ALEMBIC_LIB_SUFFIX"] ), "Alembic/AbcGeom/Foundation.h", "CXX" ) :

		# Alembic 1.6 and later is provided as a single library
		haveAlembic = True
		alembicEnv.Prepend(
			CPPFLAGS = "-DIECOREALEMBIC_WITH_OGAWA"
		)
		alembicEnv.Append(
			LIBS = [
				"Alembic$ALEMBIC_LIB_SUFFIX",
			],
		)

	else :

		sys.stderr.write( "WARNING : no Alembic library found, not building IECoreAlembic - check ALEMBIC_INCLUDE_PATH, ALEMBIC_LIB_PATH and config.log.\n" )

	c.Finish()

	if haveAlembic :

		alembicSources = sorted( glob.glob( "contrib/IECoreAlembic/src/IECoreAlembic/*.cpp" ) )
		alembicHeaders = glob.glob( "contrib/IECoreAlembic/include/IECoreAlembic/*.h" ) + glob.glob( "contrib/IECoreAlembic/include/IECoreAlembic/*.inl" )
		alembicPythonScripts = glob.glob( "contrib/IECoreAlembic/python/IECoreAlembic/*.py" )
		alembicPythonSources = sorted( glob.glob( "contrib/IECoreAlembic/src/IECoreAlembic/bindings/*.cpp" ) )

		# we can't append this before configuring, as then it gets built as
		# part of the configure process
		alembicEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)

		# library
		alembicLibrary = alembicEnv.SharedLibrary( "lib/" + os.path.basename( alembicEnv.subst( "$INSTALL_ALEMBICLIB_NAME" ) ), alembicSources )
		alembicLibraryInstall = alembicEnv.Install( os.path.dirname( alembicEnv.subst( "$INSTALL_ALEMBICLIB_NAME" ) ), alembicLibrary )
		alembicEnv.NoCache( alembicLibraryInstall )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			alembicEnv.AddPostAction( alembicLibraryInstall, lambda target, source, env : makeLibSymLinks( alembicEnv ) )
		alembicEnv.Alias( "install", alembicLibraryInstall )
		alembicEnv.Alias( "installAlembic", alembicLibraryInstall )
		alembicEnv.Alias( "installLib", [ alembicLibraryInstall ] )

		# headers
		alembicHeaderInstall = alembicEnv.Install( "$INSTALL_HEADER_DIR/IECoreAlembic", alembicHeaders )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			alembicEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreAlembic", lambda target, source, env : makeSymLinks( alembicEnv, alembicEnv["INSTALL_HEADER_DIR"] ) )
		alembicEnv.Alias( "install", alembicHeaderInstall )
		alembicEnv.Alias( "installAlembic", alembicHeaderInstall )

		# python module
		alembicPythonModuleEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ),
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( alembicEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)
		alembicPythonModule = alembicPythonModuleEnv.SharedLibrary( "contrib/IECoreAlembic/python/IECoreAlembic/_IECoreAlembic", alembicPythonSources )
		alembicPythonModuleEnv.Depends( alembicPythonModule, alembicLibrary )
		alembicPythonModuleEnv.Depends( alembicPythonModule, scenePythonModule )

		alembicPythonModuleInstall = alembicPythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreAlembic", alembicPythonScripts + alembicPythonModule )
		if env[ "INSTALL_CREATE_SYMLINKS" ] :
			alembicPythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreAlembic", lambda target, source, env : makeSymLinks( alembicPythonModuleEnv, alembicPythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
		alembicPythonModuleEnv.Alias( "install", alembicPythonModuleInstall )
		alembicPythonModuleEnv.Alias( "installAlembic", alembicPythonModuleInstall )

		Default( [ alembicLibrary, alembicPythonModule ] )

		# tests
		alembicTestEnv = testEnv.Clone()
		alembicTestLibPaths = alembicEnv.subst( os.pathsep.join( alembicPythonModuleEnv["LIBPATH"] ) )
		alembicTestEnv["ENV"][alembicTestEnv["TEST_LIBRARY_PATH_ENV_VAR"]] += os.pathsep + alembicTestLibPaths
		alembicTestEnv["ENV"]["PYTHONPATH"] += os.pathsep + "./contrib/IECoreAlembic/python"
		alembicTest = alembicTestEnv.Command( "contrib/IECoreAlembic/test/IECoreAlembic/results.txt", alembicPythonModule, "$PYTHON $TEST_ALEMBIC_SCRIPT --verbose" )
		alembicTestEnv.Depends( alembicTest, [ corePythonModule ] )
		NoCache( alembicTest )
		alembicTestEnv.Alias( "testAlembic", alembicTest )

###########################################################################################
# Documentation
###########################################################################################

docEnv = env.Clone()
docEnv["ENV"]["PATH"] = os.environ["PATH"]

if doConfigure :

	sys.stdout.write( "Checking for doxygen... " )

	if docEnv.WhereIs( docEnv["DOXYGEN"] ) :

		sys.stdout.write( "yes\n" )

		substDocs = docEnv.Substfile(
			"doc/config/Doxyfile",
			SUBST_DICT = {
				"!CORTEX_VERSION!" : env.subst( "$IECORE_VERSION" ),
			}
		)
		docEnv.NoCache( substDocs )

		docs = docEnv.Command( "doc/html/index.html", "doc/config/Doxyfile", "$DOXYGEN $SOURCE")
		docEnv.NoCache( docs )

		for modulePath in ( "python/IECore", "python/IECoreGL" ) :

			module = os.path.basename( modulePath )
			mungedModule = docEnv.Command( "doc/python/" + module, modulePath + "/__init__.py", createDoxygenPython )
			docEnv.Depends( mungedModule, glob.glob( modulePath + "/*.py" ) )
			docEnv.Depends( docs, mungedModule )
			docEnv.NoCache( mungedModule )

		docEnv.Depends( docs, glob.glob( "include/*/*.h" ) )

		docEnv.Alias( "doc", "doc/html/index.html" )

		# \todo This won't reinstall the documentation if the directory already exists
		installDoc = docEnv.Install( "$INSTALL_DOC_DIR", "doc/html" )
		docEnv.Alias( "installDoc", installDoc )

	else:

		sys.stdout.write( "no\n" )
		sys.stderr.write( "WARNING : no doxygen binary found, not building documentation - check DOXYGEN\n" )
