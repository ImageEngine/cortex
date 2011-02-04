##########################################################################
#
#  Copyright (c) 2007-2011, Image Engine Design Inc. All rights reserved.
#
#  Copyright 2010 Dr D Studios Pty Limited (ACN 127 184 954) (Dr. D Studios), 
#  its affiliates and/or its licensors.
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

EnsureSConsVersion( 0, 97 )
SConsignFile()

ieCoreMajorVersion=6
ieCoreMinorVersion=0
ieCorePatchVersion=0

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
	"g++",
)

o.Add(
	"CXXFLAGS",
	"The extra flags to pass to the C++ compiler during compilation.",
	[ "-pipe", "-Wall", "-O2", "-DNDEBUG", "-DBOOST_DISABLE_ASSERTS" ]
)

o.Add(
	"TESTCXXFLAGS",
	"The extra flags to pass to the C++ compiler during compilation of unit tests.",
	[ "-pipe", "-Wall", "-O0" ]
)

o.Add(
	"PYTHONCXXFLAGS",
	"The extra flags to pass to the C++ compiler during compilation of Python bindings.",
	[ "-pipe", "-Wall", "-O2", "-DNDEBUG", "-DBOOST_DISABLE_ASSERTS" ]
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
	"/usr/local/include/tbb",
)

o.Add(
	"TBB_LIB_PATH",
	"The path to the tbb library directory.",
	"/usr/local/lib",
)

o.Add(
	"TBB_LIB_SUFFIX",
	"The suffix appended to the names of the tbb libraries. You can modify this "
	"to link against libraries installed with non-defalt names.",
	"",
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

o.Add(
	"NUKE_LICENSE_FILE",
	"The path to the FlexLM license file to use for Nuke. This is necessary to run the tests.",
	"/usr/local/foundry/FLEXlm",
)

# Truelight options

o.Add(
	"TRUELIGHT_ROOT",
	"The directory in which Truelight is installed.",
	"/usr/fl/truelight"
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
	"/usr/local/include/GL",
)

o.Add(
	"GLEW_LIB_PATH",
	"The path to the directory with libGLEW in it.",
	"/usr/local/lib",
)

o.Add(
	"GLUT_INCLUDE_PATH",
	"The path to the directory with glut.h in it.",
	"$GLEW_INCLUDE_PATH",
)

o.Add(
	"GLUT_LIB_PATH",
	"The path to the directory with libGLUT in it.",
	"$GLEW_LIB_PATH",
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
	"MAYA_ADLM_ENV_FILE",
	"The path to ADLM env xml file to use as of Maya 2010.",
	"/usr/adlm/AdlmThinClientCustomEnv.xml",
)

try :
	o.Add(
		BoolVariable( 
			"WITH_MAYA_PLUGIN_LOADER", 
			"Set this to install the Maya plugin with a stub loader.",
			 False
		),
	)
except NameError :
	# fallback for old scons versions
	o.Add(
		BoolOption( 
			"WITH_MAYA_PLUGIN_LOADER", 
			"Set this to install the Maya plugin with a stub loader.",
			 False
		),
	)
	
# Houdini options

o.Add(
	"HOUDINI_ROOT",
	"The path to the Houdini install.",
	os.environ.get( "HFS", "" ),
)

o.Add(
	"HOUDINI_INCLUDE_PATH",
	"The path to the Houdini include directory.",
	"$HOUDINI_ROOT/toolkit/include",
)

o.Add(
	"HOUDINI_LIB_PATH",
	"The path to the houdini lib directory.",
	"$HOUDINI_ROOT/dsolib",
)

o.Add(
	"HOUDINI_BIN_PATH",
	"The path to the houdini lib directory.",
	"$HOUDINI_ROOT/bin",
)

o.Add(
	"HOUDINI_CXX_FLAGS",
	"C++ Flags to pass to the Houdini compilation.",
	"",
)

o.Add(
	"HOUDINI_LINK_FLAGS",
	"Flags to pass to the Houdini linker.",
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
	"INSTALL_PYTHONLIB_NAME",
	"The name under which to install the python library. This can "
	"be used to build and install the library for multiple python versions.",
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
	"INSTALL_HOUDINILIB_NAME",
	"The name under which to install the houdini libraries. This "
	"can be used to build and install the library for multiple "
	"Houdini versions.",
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
	"INSTALL_GLSL_SHADER_DIR",
	"The directory in which to install GLSL shaders.",
	"$INSTALL_PREFIX/glsl",
)

o.Add(
	"INSTALL_RMANPROCEDURAL_NAME",
	"The name under which to install the renderman procedurals.",
	"$INSTALL_PREFIX/rmanProcedurals/$IECORE_NAME",
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
	"INSTALL_HOUDINIOTL_DIR",
	"The directory in which to install houdini otls.",
	"$INSTALL_PREFIX/houdini/otls/",
)

o.Add(
	"INSTALL_HOUDINIICON_DIR",
	"The directory under which to install houdini icons.",
	"$INSTALL_PREFIX/houdini/icons",
)

o.Add(
	"INSTALL_HOUDINITOOLBAR_DIR",
	"The directory under which to install houdini shelf files.",
	"$INSTALL_PREFIX/houdini/toolbar",
)

o.Add(
	"INSTALL_MAYAICON_DIR",
	"The directory under which to install maya icons.",
	"$INSTALL_PREFIX/maya/icons",
)

o.Add(
	"INSTALL_MAYAPLUGIN_NAME",
	"The name under which to install maya plugins.",
	"$INSTALL_PREFIX/maya/plugins/$IECORE_NAME",
)

o.Add(
	"INSTALL_HOUDINIPLUGIN_NAME",
	"The name under which to install houdini plugins.",
	"$INSTALL_PREFIX/houdini/dso/$IECORE_NAME",
)

o.Add(
	"INSTALL_NUKEPLUGIN_NAME",
	"The name under which to install nuke plugins.",
	"$INSTALL_PREFIX/nuke/plugins/$IECORE_NAME",
)

o.Add(
	"INSTALL_DOC_DIR",
	"The directory in which to install the documentation.",
	"$INSTALL_PREFIX/share/cortex",
)

o.Add(
	"INSTALL_IECORE_OP_PATH",
	"The directory in which to install the IECore op stubs.",
	"$INSTALL_PREFIX/ops",
)

o.Add(
	"INSTALL_IECORE_OPS",
	"The IECore C++ ops to install via python stubs.",
	[
		( "IECore.SequenceLsOp", "common/fileSystem/seqLs" ), 
		( "IECore.SequenceCpOp", "common/fileSystem/seqCp" ), 
		( "IECore.SequenceMvOp", "common/fileSystem/seqMv" ), 
		( "IECore.SequenceRmOp", "common/fileSystem/Rm" ), 
		( "IECore.SequenceRenumberOp", "common/fileSystem/seqRenumber" ), 
		( "IECore.SequenceConvertOp", "common/fileSystem/seqConvert" ),
		( "IECore.SequenceCatOp", "common/fileSystem/seqCat" ),
		( "IECore.RemovePrimitiveVariables", "common/primitive/removeVariables" ), 
		( "IECore.RenamePrimitiveVariables", "common/primitive/renameVariables" ), 
		( "IECore.PointsExpressionOp", "common/primitive/pointsExpression" ),
		( "IECore.ClassLsOp", "common/classes/classLs" ), 
		( "IECore.FileDependenciesOp", "common/fileSystem/depLs" ), 
		( "IECore.CheckFileDependenciesOp", "common/fileSystem/depCheck" ), 
		( "IECore.LsHeaderOp", "common/fileSystem/lsHeader" ),
		( "IECore.SearchReplaceOp", "common/fileSystem/searchReplace" ),
		( "IECore.CheckImagesOp", "common/fileSystem/checkImages" ),
		( "IECore.FileSequenceGraphOp", "common/fileSystem/fileSequenceGraph" ),
		( "IECore.MeshPrimitiveImplicitSurfaceOp", "common/primitive/mesh/implicitSurface" ), 	
		( "IECore.MeshVertexReorderOp", "common/primitive/mesh/vertexReorder" ),
		( "IECore.MeshPrimitiveShrinkWrapOp", "common/primitive/mesh/shrinkWrap" ),	 		
		( "IECore.Grade", "common/colorSpace/grade" ),
		( "IECore.CubeColorTransformOp", "common/colorSpace/cubeColorTransform" ),
		( "IECore.CineonToLinearOp", "common/colorSpace/cineonToLinear" ),
		( "IECore.LinearToCineonOp", "common/colorSpace/linearToCineon" ),
		( "IECore.SRGBToLinearOp", "common/colorSpace/SRGBToLinear" ),
		( "IECore.LinearToSRGBOp", "common/colorSpace/linearToSRGB" ),
		( "IECore.Rec709ToLinearOp", "common/colorSpace/Rec709ToLinear" ),
		( "IECore.LinearToRec709Op", "common/colorSpace/linearToRec709" ),
		( "IECore.PanalogToLinearOp", "common/colorSpace/PanalogToLinear" ),
		( "IECore.LinearToPanalogOp", "common/colorSpace/linearToPanalog" ),
		( "IECore.UVDistortOp", "common/2d/image/uvDistort" ),
		( "IECore.ImageCompositeOp", "common/2d/image/imageComposite" ),
		( "IECore.ImageConvolveOp", "common/2d/image/imageConvolve" ),
		( "IECore.AddSmoothSkinningInfluencesOp", "rigging/smoothSkinning/addInfluences" ),
		( "IECore.RemoveSmoothSkinningInfluencesOp", "rigging/smoothSkinning/removeInfluences" ),
		( "IECore.CompressSmoothSkinningDataOp", "rigging/smoothSkinning/compress" ),
		( "IECore.DecompressSmoothSkinningDataOp", "rigging/smoothSkinning/decompress" ),
		( "IECore.NormalizeSmoothSkinningWeightsOp", "rigging/smoothSkinning/normalizeWeights" ),
		( "IECore.ReorderSmoothSkinningInfluencesOp", "rigging/smoothSkinning/reorderInfluences" ),
		( "IECore.SmoothSmoothSkinningWeightsOp", "rigging/smoothSkinning/smoothWeights" ),
		( "IECore.LimitSmoothSkinningInfluencesOp", "rigging/smoothSkinning/limitInfluences" ),
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
	"test/IECoreGL/All.py"
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
	"TEST_HOUDINI_SCRIPT",
	"The python script to run for the houdini tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"contrib/IECoreHoudini/test/All.py"
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
	
libraryPathEnvVar = "DYLD_LIBRARY_PATH" if Environment()["PLATFORM"]=="darwin" else "LD_LIBRARY_PATH"
o.Add(
	"TEST_LIBRARY_PATH_ENV_VAR",
	"This is a curious one, probably only ever necessary at image engine. It "
	"specifies the name of an environment variable used to specify the library "
	"search paths correctly when running the tests. Defaults to LD_LIBRARY_PATH on "
	"Linux and DYLD_LIBRARY_PATH on OSX.",
	libraryPathEnvVar
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

if env["PLATFORM"]=="darwin" :
	env["ENV"]["MACOSX_DEPLOYMENT_TARGET"] = "10.4"

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
		"$TBB_INCLUDE_PATH",
		"$OPENEXR_INCLUDE_PATH",
		"$ILMBASE_INCLUDE_PATH",
		# we use "OpenEXR/x.h" and they use "x.h"
		os.path.join( "$OPENEXR_INCLUDE_PATH","OpenEXR" ),
		os.path.join( "$ILMBASE_INCLUDE_PATH","OpenEXR" ),
		"$BOOST_INCLUDE_PATH",
		"$JPEG_INCLUDE_PATH",
		"$TIFF_INCLUDE_PATH",
		"$FREETYPE_INCLUDE_PATH",
	],
	LIBPATH = [
		"$TBB_LIB_PATH",
		"$BOOST_LIB_PATH",
		"$OPENEXR_LIB_PATH",
		"$ILMBASE_LIB_PATH",
		"$JPEG_LIB_PATH",
		"$TIFF_LIB_PATH",
		"$FREETYPE_LIB_PATH",
	],
	LIBS = [
		"pthread",
	]
)

if env["PLATFORM"]=="darwin" :
	# os x versions before snow leopard require the no-long-double flag
	compilerVersion = map( int, env["CXXVERSION"].split( "." ) )
	if compilerVersion[0] < 4 or compilerVersion[0]==4 and compilerVersion[1] < 2 :	
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

	if not c.CheckLibWithHeader( "Iex" + env["OPENEXR_LIB_SUFFIX"], "OpenEXR/ImfInputFile.h", "C++" ) :
		sys.stderr.write( "ERROR : unable to find the OpenEXR libraries - check OPENEXR_INCLUDE_PATH and OPENEXR_LIB_PATH.\n" )
		Exit( 1 )
		
	if not c.CheckLibWithHeader( "tbb" + env["TBB_LIB_SUFFIX"], "tbb/tbb.h", "C++" ) :
		sys.stderr.write( "ERROR : unable to find the TBB libraries - check TBB_INCLUDE_PATH and TBB_LIB_PATH.\n" )
		Exit( 1 )	
		
	c.Finish()
		
env.Append( LIBS = [
		"Half" + env["OPENEXR_LIB_SUFFIX"],
		"Iex" + env["OPENEXR_LIB_SUFFIX"],
		"Imath" + env["OPENEXR_LIB_SUFFIX"],
		"IlmImf" + env["OPENEXR_LIB_SUFFIX"],
		"IlmThread" + env["OPENEXR_LIB_SUFFIX"],
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

pythonEnv = env.Clone()
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

pythonModuleEnv = pythonEnv.Clone()

pythonModuleEnv["SHLIBPREFIX"] = ""
pythonModuleEnv["SHLIBSUFFIX"] = ".so"

if pythonModuleEnv["PLATFORM"]=="darwin" :
	pythonModuleEnv.Append( SHLINKFLAGS = "-single_module" )
	
###########################################################################################
# An environment for running tests
###########################################################################################

testEnv = env.Clone()
testEnv.Replace( CXXFLAGS = env.subst("$TESTCXXFLAGS") )

testEnv.Prepend( LIBPATH = [ "./lib" ] )

testEnvLibPath = ":".join( testEnv["LIBPATH"] )
if testEnv["TEST_LIBPATH"] != "" :
	testEnvLibPath += ":" + testEnv["TEST_LIBPATH"]

testEnv["ENV"][testEnv["TEST_LIBRARY_PATH_ENV_VAR"]] = testEnvLibPath
testEnv["ENV"][libraryPathEnvVar] = testEnvLibPath
testEnv["ENV"]["IECORE_PROCEDURAL_PATHS"] = "test/IECore/procedurals"
testEnv["ENV"]["IECORE_OP_PATHS"] = "test/IECore/ops"

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
	
# Create an op stub so that a C++ op can be loaded by IECore.ClassLoader
def createOpStub( op_install_path, full_class_name, stub_path ):
	module_name = full_class_name.split('.')[0]
	class_name = '.'.join( full_class_name.split('.')[1:])
	stub_name = stub_path.split('/')[-1]
	
	# make sure our directory exists
	install_path = "%s/%s" % ( op_install_path, stub_path )
	if not os.path.exists( install_path ):
		os.makedirs( install_path )
		
	# write our stub file
	stub_filename = "%s/%s-1.py" % ( install_path, stub_name )
	stub = open( stub_filename, "w" )
	stub.write( "from %s import %s as %s\n" % ( module_name, class_name, stub_name ) )
	stub.close()
	
# builder action that creates the C++ op stubs
def createOpStubs( target, source, env ):
	for op in env['INSTALL_IECORE_OPS']:
		createOpStub( env.subst( "$INSTALL_IECORE_OP_PATH" ), op[0], op[1] ) 

def readLinesMinusLicense( f ) :

	if isinstance( f, basestring ) :
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
	
###########################################################################################
# Build, install and test the core library and bindings
###########################################################################################

coreEnv = env.Clone( IECORE_NAME="IECore" )
corePythonEnv = pythonEnv.Clone( IECORE_NAME="IECorePython" )
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

# configure checks
if doConfigure :

	c = Configure( coreEnv )
		
	if c.CheckCXXHeader( "boost/asio.hpp" ) :
		for e in allCoreEnvs :
			e.Append( CPPFLAGS = '-DIECORE_WITH_ASIO' )
	else :
		sys.stderr.write( "WARNING: boost/asio.hpp not found, some functionality will be disabled.\n" )
		coreSources.remove( "src/IECore/ClientDisplayDriver.cpp" )
		coreSources.remove( "src/IECore/DisplayDriver.cpp" )
		coreSources.remove( "src/IECore/ImageDisplayDriver.cpp" )
		coreSources.remove( "src/IECore/DisplayDriverServer.cpp" )
		corePythonSources.remove( "src/IECorePython/ClientDisplayDriverBinding.cpp" )
		corePythonSources.remove( "src/IECorePython/DisplayDriverServerBinding.cpp" )
		corePythonSources.remove( "src/IECorePython/DisplayDriverBinding.cpp" )
		corePythonSources.remove( "src/IECorePython/ImageDisplayDriverBinding.cpp" )
		## \todo: OBJReader needs a version of boost::bind that doesn't give warnings when some
		## placeholders aren't bound (which is true of any boost version that includes asio.hpp)
		coreSources.remove( "src/IECore/OBJReader.cpp" )
		corePythonSources.remove( "src/IECorePython/OBJReaderBinding.cpp" )
	
	if c.CheckCXXHeader( "boost/math/special_functions/factorials.hpp" ) :
		for e in allCoreEnvs :
			e.Append( CPPFLAGS = '-DIECORE_WITH_BOOSTFACTORIAL' )
	else :
		sys.stderr.write( "WARNING: boost/math/special_functions/factorials.hpp not found, some functionality will be disabled.\n" )
		coreSources.remove( "src/IECore/AssociatedLegendre.cpp" )
		coreSources.remove( "src/IECore/SphericalHarmonics.cpp" )
		coreSources.remove( "src/IECore/SphericalHarmonicsTensor.cpp" )
		coreSources.remove( "src/IECore/SphericalHarmonicsRotationMatrix.cpp" )
		coreSources.remove( "src/IECore/SphericalHarmonicsTransferMatrix.cpp" )
		coreSources.remove( "src/IECore/SphericalHarmonicsProjector.cpp" )
		coreSources.remove( "src/IECore/EnvMapSHProjector.cpp" )
		coreSources.remove( "src/IECore/ImageConvolveOp.cpp" )
		corePythonSources.remove( "src/IECorePython/EnvMapSHProjectorBinding.cpp" )
		corePythonSources.remove( "src/IECorePython/ImageConvolveOpBinding.cpp" )
	
	if c.CheckLibWithHeader( "tiff", "tiff.h", "CXX" ) :
		for e in allCoreEnvs :
			e.Append( CPPFLAGS = '-DIECORE_WITH_TIFF' )
	else :
		sys.stderr.write( "WARNING: no TIFF library found, no TIFF support, check TIFF_INCLUDE_PATH and TIFF_LIB_PATH.\n" )
		coreSources.remove( "src/IECore/TIFFImageWriter.cpp" )
		coreSources.remove( "src/IECore/TIFFImageReader.cpp" )
		coreSources.remove( "src/IECore/ScopedTIFFErrorHandler.cpp" )		
		corePythonSources.remove( "src/IECorePython/TIFFImageReaderBinding.cpp" )
		corePythonSources.remove( "src/IECorePython/TIFFImageWriterBinding.cpp" )
		
	if c.CheckLibWithHeader( "jpeg", ["stdio.h", "jpeglib.h"], "CXX" ) :
		for e in allCoreEnvs :
			e.Append( CPPFLAGS = '-DIECORE_WITH_JPEG' )
	else :
		sys.stderr.write( "WARNING: no JPEG library found, no JPEG support, check JPEG_INCLUDE_PATH and JPEG_LIB_PATH.\n" )
		coreSources.remove( "src/IECore/JPEGImageWriter.cpp" )
		coreSources.remove( "src/IECore/JPEGImageReader.cpp" )
		corePythonSources.remove( "src/IECorePython/JPEGImageReaderBinding.cpp" )
		corePythonSources.remove( "src/IECorePython/JPEGImageWriterBinding.cpp" )
	
	if c.CheckLibWithHeader( "freetype", ["ft2build.h"], "CXX" ) :
		for e in allCoreEnvs :
			e.Append( CPPFLAGS = "-DIECORE_WITH_FREETYPE" )
	else :
		sys.stderr.write( "WARNING: no FreeType library found, no font support, check FREETYPE_INCLUDE_PATH and FREETYPE_LIB_PATH.\n" )
		coreSources.remove( "src/IECore/Font.cpp" )
		corePythonSources.remove( "src/IECorePython/FontBinding.cpp" )
				
	c.Finish()

# library
coreLibrary = coreEnv.SharedLibrary( "lib/" + os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ), coreSources )
coreLibraryInstall = coreEnv.Install( os.path.dirname( coreEnv.subst( "$INSTALL_LIB_NAME" ) ), coreLibrary )
coreEnv.NoCache( coreLibraryInstall )
coreEnv.AddPostAction( coreLibraryInstall, lambda target, source, env : makeLibSymLinks( coreEnv ) )
coreEnv.Alias( "install", [ coreLibraryInstall ] )
coreEnv.Alias( "installCore", [ coreLibraryInstall ] )
coreEnv.Alias( "installLib", [ coreLibraryInstall ] )

# headers
headerInstall = coreEnv.Install( "$INSTALL_HEADER_DIR/IECore", coreHeaders )
coreEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECore", lambda target, source, env : makeSymLinks( coreEnv, coreEnv["INSTALL_HEADER_DIR"] ) )
coreEnv.Alias( "install", headerInstall )
coreEnv.Alias( "installCore", headerInstall )

# python library
corePythonEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )
corePythonLibrary = corePythonEnv.SharedLibrary( "lib/" + os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ), corePythonSources )
corePythonLibraryInstall = corePythonEnv.Install( os.path.dirname( corePythonEnv.subst( "$INSTALL_LIB_NAME" ) ), corePythonLibrary )
corePythonEnv.NoCache( corePythonLibraryInstall )
corePythonEnv.AddPostAction( corePythonLibraryInstall, lambda target, source, env : makeLibSymLinks( corePythonEnv, libNameVar="INSTALL_PYTHONLIB_NAME" ) )
corePythonEnv.AddPostAction( corePythonLibraryInstall, createOpStubs )
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

corePythonModuleInstall = corePythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECore", corePythonScripts + corePythonModule )
corePythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECore", lambda target, source, env : makeSymLinks( corePythonEnv, corePythonEnv["INSTALL_PYTHON_DIR"] ) )
corePythonModuleEnv.Alias( "install", corePythonModuleInstall )
corePythonModuleEnv.Alias( "installCore", corePythonModuleInstall )

Default( coreLibrary, corePythonLibrary, corePythonModule )

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

coreTestSources = glob.glob( "test/IECore/*.cpp" )
if '-DIECORE_WITH_BOOSTFACTORIAL' not in coreTestEnv['CPPFLAGS'] :
	coreTestSources.remove( "test/IECore/AssociatedLegendreTest.cpp" )
	coreTestSources.remove( "test/IECore/SphericalHarmonicsTest.cpp" )
	coreTestSources.remove( "test/IECore/LevenbergMarquardtTest.cpp" )

coreTestProgram = coreTestEnv.Program( "test/IECore/IECoreTest", coreTestSources )

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

riEnv = coreEnv.Clone( IECORE_NAME = "IECoreRI" )
riEnv.Append( CPPPATH = [ "$RMAN_ROOT/include" ] )
riEnv.Append( LIBPATH = [ "$RMAN_ROOT/lib" ] )

riPythonModuleEnv = pythonModuleEnv.Clone( IECORE_NAME = "IECoreRI" )
riPythonModuleEnv.Append( CPPPATH = [ "$RMAN_ROOT/include" ] )
riPythonModuleEnv.Append( LIBPATH = [ "$RMAN_ROOT/lib" ] )

riPythonProceduralEnv = riPythonModuleEnv.Clone( IECORE_NAME = "iePython" )

haveRI = False
riLibs = []

if doConfigure :

	c = Configure( riEnv )

	haveDelight = c.CheckLibWithHeader( "3delight", "ri.h", "CXX" )
	havePRMan = c.CheckLibWithHeader( "prman", "ri.h", "CXX" )

	if not haveDelight and not havePRMan :
	
		sys.stderr.write( "WARNING : no 3delight or prman library found, not building IECoreRI - check RMAN_ROOT.\n" )
		c.Finish()

	else :
	
		haveRI = True
		if haveDelight :
			riLibs = [ "3delight" ]
		else :
			riLibs = [ "prman" ]
		riEnv.Append( LIBS = riLibs )
		riPythonModuleEnv.Append( LIBS = riLibs )
		
		riSources = glob.glob( "src/IECoreRI/*.cpp" )
		riHeaders = glob.glob( "include/IECoreRI/*.h" ) + glob.glob( "include/IECoreRI/*.inl" )
		riPythonSources = glob.glob( "src/IECoreRI/bindings/*.cpp" )
		riPythonScripts = glob.glob( "python/IECoreRI/*.py" )
	
		if c.CheckHeader( "pointcloud.h" ) :
			
			riEnv.Append( CPPFLAGS = "-DIECORERI_WITH_PTC" )
			riPythonModuleEnv.Append( CPPFLAGS = "-DIECORERI_WITH_PTC" )
			
		else :
		
			riSources.remove( "src/IECoreRI/PTCParticleReader.cpp" )
			riSources.remove( "src/IECoreRI/PTCParticleWriter.cpp" )
			riPythonSources.remove( "src/IECoreRI/bindings/PTCParticleReaderBinding.cpp" )
			riPythonSources.remove( "src/IECoreRI/bindings/PTCParticleWriterBinding.cpp" )
			
		if c.CheckFunc( "RiObjectBeginV" ) :
			
			riEnv.Append( CPPFLAGS = [ "-DIECORERI_WITH_OBJECTBEGINV" ] )
		
		if c.CheckCXXHeader( "sx.h" ) and c.CheckFunc( "SxGetParameter" ) :
		
			riEnv.Append( CPPFLAGS = "-DIECORERI_WITH_SX" )
			riPythonModuleEnv.Append( CPPFLAGS = "-DIECORERI_WITH_SX" )
			
		else :
		
			riSources.remove( "src/IECoreRI/SXRenderer.cpp" )
			riSources.remove( "src/IECoreRI/SXRendererImplementation.cpp" )
			riSources.remove( "src/IECoreRI/SXExecutor.cpp" )
			riPythonSources.remove( "src/IECoreRI/bindings/SXRendererBinding.cpp" )

			sys.stderr.write( "WARNING : Supported Sx API version not found - not building SXRenderer. Use 3delight 9.0.36 or later.\n" )
		
		if c.CheckCXXHeader( "gx.h" ) and c.CheckFunc( "GxGetGeometry" ) :
		
			riEnv.Append( CPPFLAGS = "-DIECORERI_WITH_GX" )
			riPythonModuleEnv.Append( CPPFLAGS = "-DIECORERI_WITH_GX" )
			
		else :
		
			riSources.remove( "src/IECoreRI/GXEvaluator.cpp" )
			riPythonSources.remove( "src/IECoreRI/bindings/GXEvaluatorBinding.cpp" )

			sys.stderr.write( "WARNING : Gx API not found - not building GXEvaluator. Use 3delight 9.0.39 or later.\n" )
			
		c.Finish()	

		# we can't append this before configuring, as then it gets built as
		# part of the configure process
		riEnv.Prepend( LIBPATH = [ "./lib" ] )
		riEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )
		riPythonModuleEnv.Append( LIBS = os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ) )
	
		# library
		riLibrary = riEnv.SharedLibrary( "lib/" + os.path.basename( riEnv.subst( "$INSTALL_LIB_NAME" ) ), riSources )
		riLibraryInstall = riEnv.Install( os.path.dirname( riEnv.subst( "$INSTALL_LIB_NAME" ) ), riLibrary )
		riEnv.NoCache( riLibraryInstall )
		riEnv.AddPostAction( riLibraryInstall, lambda target, source, env : makeLibSymLinks( riEnv ) )
		riEnv.Alias( "install", riLibraryInstall )
		riEnv.Alias( "installRI", riLibraryInstall )
		riEnv.Alias( "installLib", [ riLibraryInstall ] )

		# headers
		riHeaderInstall = riEnv.Install( "$INSTALL_HEADER_DIR/IECoreRI", riHeaders )
		riEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreRI", lambda target, source, env : makeSymLinks( riEnv, riEnv["INSTALL_HEADER_DIR"] ) )
		riEnv.Alias( "install", riHeaderInstall )
		riEnv.Alias( "installRI", riHeaderInstall )

		# python procedural
		riPythonProcedural = riPythonProceduralEnv.SharedLibrary( "src/rmanProcedurals/python/" + os.path.basename( riPythonProceduralEnv.subst( "$INSTALL_RMANPROCEDURAL_NAME" ) ), "src/rmanProcedurals/python/Procedural.cpp" )
		riPythonProceduralInstall = riEnv.Install( os.path.dirname( riPythonProceduralEnv.subst( "$INSTALL_RMANPROCEDURAL_NAME" ) ), riPythonProcedural )
		riPythonProceduralEnv.NoCache( riPythonProceduralInstall )
		riPythonProceduralEnv.AddPostAction( riPythonProceduralInstall, lambda target, source, env : makeLibSymLinks( riPythonProceduralEnv, libNameVar="INSTALL_RMANPROCEDURAL_NAME" ) )
		riPythonProceduralEnv.Alias( "install", riPythonProceduralInstall )
		riPythonProceduralEnv.Alias( "installRI", riPythonProceduralInstall )
		riPythonProceduralForTest = riPythonProceduralEnv.Command( "src/rmanProcedurals/python/python.so", riPythonProcedural, Copy( "$TARGET", "$SOURCE" ) )

		# rsl headers
		rslHeaders = glob.glob( "rsl/IECoreRI/*.h" ) + glob.glob( "rsl/IECoreRI/*.inl" )
		rslHeaderInstall = riEnv.Install( "$INSTALL_RSL_HEADER_DIR/IECoreRI", rslHeaders )
		riEnv.AddPostAction( "$INSTALL_RSL_HEADER_DIR/IECoreRI", lambda target, source, env : makeSymLinks( riEnv, riEnv["INSTALL_RSL_HEADER_DIR"] ) )
		riEnv.Alias( "install", rslHeaderInstall )
		riEnv.Alias( "installRI", rslHeaderInstall )
		
		# python module
		riPythonModuleEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( riEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)
		riPythonModule = riPythonModuleEnv.SharedLibrary( "python/IECoreRI/_IECoreRI", riPythonSources )
		riPythonModuleEnv.Depends( riPythonModule, riLibrary )

		riPythonModuleInstall = riPythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreRI", riPythonScripts + riPythonModule )
		riPythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreRI", lambda target, source, env : makeSymLinks( riPythonModuleEnv, riPythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
		riPythonModuleEnv.Alias( "install", riPythonModuleInstall )
		riPythonModuleEnv.Alias( "installRI", riPythonModuleInstall )

		if coreEnv["INSTALL_CORERI_POST_COMMAND"]!="" :
			# this is the only way we could find to get a post action to run for an alias
			riPythonModuleEnv.Alias( "install", riPythonModuleInstall, "$INSTALL_CORERI_POST_COMMAND" ) 
			riPythonModuleEnv.Alias( "installRI", riPythonModuleInstall, "$INSTALL_CORERI_POST_COMMAND" ) 

		Default( [ riLibrary, riPythonModule, riPythonProcedural ] )
		
		# tests
		riTestEnv = testEnv.Clone()

		riTestEnv["ENV"][testEnv["TEST_LIBRARY_PATH_ENV_VAR"]] += ":" + riEnv.subst( ":".join( [ "./lib" ] + riPythonModuleEnv["LIBPATH"] ) )
		riTestEnv["ENV"]["SHADER_PATH"] = riEnv.subst( "$RMAN_ROOT/shaders" )
		riTestEnv["ENV"]["DELIGHT"] = riEnv.subst( "$RMAN_ROOT" )
		riTestEnv["ENV"]["DL_SHADERS_PATH"] = riEnv.subst( "$RMAN_ROOT/shaders" ) + ":./"
		riTestEnv["ENV"]["DL_DISPLAYS_PATH"] = riEnv.subst( "$RMAN_ROOT/displays" )
		riTestEnv["ENV"]["PATH"] = riEnv.subst( "$RMAN_ROOT/bin" ) + ":" + riTestEnv["ENV"]["PATH"] 
		
		riTest = riTestEnv.Command( "test/IECoreRI/results.txt", riPythonModule, pythonExecutable + " $TEST_RI_SCRIPT" )
		NoCache( riTest )
		riTestEnv.Depends( riTest, [ corePythonModule + riPythonProceduralForTest ] )
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
		],
		"LIBPATH" : [
			"./lib",
		]
	}
	glEnvAppends = {
		
		"CPPPATH" : [
			"$GLEW_INCLUDE_PATH",
			"$GLUT_INCLUDE_PATH",
		],
		"LIBPATH" : [
			"$GLEW_LIB_PATH",
			"$GLUT_LIB_PATH",
		],
	}
	
	glEnv = coreEnv.Clone( **glEnvSets )

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

		glSources = glob.glob( "src/IECoreGL/*.cpp" )
		if not "-DIECORE_WITH_FREETYPE" in glEnv["CPPFLAGS"] :
			glSources.remove( "src/IECoreGL/Font.cpp" )
			glSources.remove( "src/IECoreGL/TextPrimitive.cpp" )
		
		glLibrary = glEnv.SharedLibrary( "lib/" + os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ), glSources )
		glLibraryInstall = glEnv.Install( os.path.dirname( glEnv.subst( "$INSTALL_LIB_NAME" ) ), glLibrary )
		glEnv.NoCache( glLibraryInstall )
		glEnv.AddPostAction( glLibraryInstall, lambda target, source, env : makeLibSymLinks( glEnv ) )
		glEnv.Alias( "install", glLibraryInstall )
		glEnv.Alias( "installGL", glLibraryInstall )
		glEnv.Alias( "installLib", [ glLibraryInstall ] )

		glHeaders = glob.glob( "include/IECoreGL/*.h" ) + glob.glob( "include/IECoreGL/*.inl" )
		glHeaderInstall = glEnv.Install( "$INSTALL_HEADER_DIR/IECoreGL", glHeaders )
		glEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreGL", lambda target, source, env : makeSymLinks( glEnv, glEnv["INSTALL_HEADER_DIR"] ) )
		glEnv.Alias( "install", glHeaderInstall )
		glEnv.Alias( "installGL", glHeaderInstall )
		
		glslHeaders = glob.glob( "glsl/IECoreGL/*.h" )
		glslHeaderInstall = glEnv.Install( "$INSTALL_GLSL_HEADER_DIR/IECoreGL", glslHeaders )
		glEnv.AddPostAction( "$INSTALL_GLSL_HEADER_DIR/IECoreGL", lambda target, source, env : makeSymLinks( glEnv, glEnv["INSTALL_GLSL_HEADER_DIR"] ) )
		glEnv.Alias( "install", glslHeaderInstall )
		glEnv.Alias( "installGL", glslHeaderInstall )
				
		glslShaderFiles = glob.glob( "glsl/*.frag" ) + glob.glob( "glsl/*.vert" )		
		glslShaderInstall = glEnv.Install( "$INSTALL_GLSL_SHADER_DIR", glslShaderFiles )
		glEnv.AddPostAction( "$INSTALL_GLSL_SHADER_DIR", lambda target, source, env : makeSymLinks( glEnv, glEnv["INSTALL_GLSL_SHADER_DIR"] ) )
		glEnv.Alias( "install", glslShaderInstall )
		glEnv.Alias( "installGL", glslShaderInstall )		

		glPythonSources = glob.glob( "src/IECoreGL/bindings/*.cpp" )
		glPythonModuleEnv = pythonModuleEnv.Clone( **glEnvSets )
		glPythonModuleEnv.Append( **glEnvAppends )
		glPythonModuleEnv.Prepend( **glEnvPrepends )
		glPythonModuleEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ),
			]
		)
		glPythonModule = glPythonModuleEnv.SharedLibrary( "python/IECoreGL/_IECoreGL", glPythonSources )
		glPythonModuleEnv.Depends( glPythonModule, glLibrary )

		glPythonScripts = glob.glob( "python/IECoreGL/*.py" )
		glPythonModuleInstall = glPythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreGL", glPythonScripts + glPythonModule )		
		glPythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreGL", lambda target, source, env : makeSymLinks( glPythonModuleEnv, glPythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
		glPythonModuleEnv.Alias( "install", glPythonModuleInstall )
		glPythonModuleEnv.Alias( "installGL", glPythonModuleInstall )

		if coreEnv["INSTALL_COREGL_POST_COMMAND"]!="" :
			# this is the only way we could find to get a post action to run for an alias
			glPythonModuleEnv.Alias( "install", glPythonModuleInstall, "$INSTALL_COREGL_POST_COMMAND" ) 
			glPythonModuleEnv.Alias( "installGL", glPythonModuleInstall, "$INSTALL_COREGL_POST_COMMAND" ) 

		Default( [ glLibrary, glPythonModule ] )

		glTestEnv = testEnv.Clone()
		glTestEnv["ENV"]["PYTHONPATH"] = glTestEnv["ENV"]["PYTHONPATH"] + ":python"
		for e in ["DISPLAY", "XAUTHORITY"] :
			if e in os.environ :
				glTestEnv["ENV"][e] = os.environ[e]
		
		glTest = glTestEnv.Command( "test/IECoreGL/results.txt", glPythonModule, pythonExecutable + " $TEST_GL_SCRIPT --verbose" )
		NoCache( glTest )
		glTestEnv.Depends( glTest, corePythonModule )
		glTestEnv.Depends( glTest, glob.glob( "test/IECoreGL/*.py" ) )
		glTestEnv.Alias( "testGL", glTest )
		
###########################################################################################
# Build, install and test the coreMaya library and bindings
###########################################################################################

mayaEnvSets = {
	"IECORE_NAME" : "IECoreMaya",
}

mayaEnvAppends = {
	"CPPPATH" : [
		"$GLEW_INCLUDE_PATH",
	],
	"LIBS" : [
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
	mayaEnvAppends["CPPFLAGS"] += ["-DLINUX"]
	mayaEnvAppends["LIBPATH"] = ["$MAYA_ROOT/lib"]
	mayaEnvAppends["CPPPATH"] += ["$MAYA_ROOT/include"]
	mayaEnvAppends["LIBS"]  += ["OpenMayalib"]

elif env["PLATFORM"]=="darwin" :
	mayaEnvAppends["CPPFLAGS"]  += ["-DOSMac_","-DOSMac_MachO_"]
	mayaEnvAppends["LIBPATH"] = ["$MAYA_ROOT/MacOS"]
	mayaEnvAppends["CPPPATH"] += ["$MAYA_ROOT/../../devkit/include"]
	mayaEnvAppends["LIBS"] += ["Foundation", "OpenMayaRender"]
	mayaEnvAppends["FRAMEWORKS"] = ["AGL", "OpenGL"]

mayaEnv = env.Clone( **mayaEnvSets )
mayaEnv.Append( **mayaEnvAppends )

mayaEnv.Append( SHLINKFLAGS = pythonEnv["PYTHON_LINK_FLAGS"].split() )

mayaPythonModuleEnv = pythonModuleEnv.Clone( **mayaEnvSets )
mayaPythonModuleEnv.Append( **mayaEnvAppends )

mayaPluginEnv = mayaEnv.Clone( IECORE_NAME="ieCore" )

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

		# deal with adding or removing renderman bits as necessary
		if haveRI :
			mayaEnv.Append( LIBS = riLibs )
			mayaEnv.Append( LIBPATH = [ "$RMAN_ROOT/lib" ] )
			mayaEnv.Append( CPPFLAGS = "-DIECOREMAYA_WITH_RI" )
			mayaEnv.Append( CPPPATH = [ "$RMAN_ROOT/include" ] )
			mayaEnv.Append( CPPFLAGS = [ "-DIECORERI_RMANPROCEDURAL_NAME=" + os.path.basename( riPythonProceduralEnv.subst( "$INSTALL_RMANPROCEDURAL_NAME" ) ) ] )
			mayaEnv.Append( LIBS = os.path.basename( riEnv.subst( "$INSTALL_LIB_NAME" ) ) )
		else :
			mayaSources.remove( "src/IECoreMaya/DelightProceduralCacheCommand.cpp" )

		# we can't append this before configuring, as then it gets built as
		# part of the configure process
		mayaEnv.Prepend( LIBPATH = [ "./lib" ] )
		mayaEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )
		mayaEnv.Append( LIBS = os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ) )
		mayaEnv.Append( LIBS = os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ) )

		# maya library
		mayaLibrary = mayaEnv.SharedLibrary( "lib/" + os.path.basename( mayaEnv.subst( "$INSTALL_MAYALIB_NAME" ) ), mayaSources )
		mayaLibraryInstall = mayaEnv.Install( os.path.dirname( mayaEnv.subst( "$INSTALL_MAYALIB_NAME" ) ), mayaLibrary )
		mayaEnv.NoCache( mayaLibraryInstall )
		mayaEnv.AddPostAction( mayaLibraryInstall, lambda target, source, env : makeLibSymLinks( mayaEnv, "INSTALL_MAYALIB_NAME" ) )
		mayaEnv.Alias( "install", mayaLibraryInstall )
		mayaEnv.Alias( "installMaya", mayaLibraryInstall )
		mayaEnv.Alias( "installLib", [ mayaLibraryInstall ] )

 		# maya headers
		mayaHeaderInstall = mayaEnv.Install( "$INSTALL_HEADER_DIR/IECoreMaya", mayaHeaders )
		mayaHeaderInstall += mayaEnv.Install( "$INSTALL_HEADER_DIR/IECoreMaya/bindings", mayaBindingHeaders )		
		mayaEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreMaya", lambda target, source, env : makeSymLinks( mayaEnv, mayaEnv["INSTALL_HEADER_DIR"] ) )
		mayaEnv.Alias( "install", mayaHeaderInstall )
		mayaEnv.Alias( "installMaya", mayaHeaderInstall )

		# maya mel
		mayaMelInstall = mayaEnv.Install( "$INSTALL_MEL_DIR", mayaMel )
		mayaEnv.AddPostAction( "$INSTALL_MEL_DIR", lambda target, source, env : makeSymLinks( mayaEnv, mayaEnv["INSTALL_MEL_DIR"] ) )
		mayaEnv.Alias( "install", mayaMelInstall )
		mayaEnv.Alias( "installMaya", mayaMelInstall )
		
		# maya icons
		mayaIcons = glob.glob( "icons/IECoreMaya/*.xpm" )
		mayaIconInstall = mayaEnv.Install( "$INSTALL_MAYAICON_DIR", source=mayaIcons )
		mayaEnv.Alias( "install", mayaIconInstall )
		mayaEnv.Alias( "installMaya", mayaIconInstall )
		
		# maya plugin
		mayaPluginEnv.Append(
			LIBPATH = [ "./lib" ],
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_MAYALIB_NAME" ) ),
				os.path.basename( mayaEnv.subst( "$INSTALL_MAYALIB_NAME" ) ),
			]
		)
		if env["PLATFORM"]=="darwin" :
			mayaPluginEnv['SHLINKFLAGS'] = '$LINKFLAGS -bundle'
			mayaPluginEnv['SHLIBSUFFIX'] =  '.bundle'
			
		mayaPluginTarget = "plugins/maya/" + os.path.basename( mayaPluginEnv.subst( "$INSTALL_MAYAPLUGIN_NAME" ) )
		
		if env["WITH_MAYA_PLUGIN_LOADER"] :
		
			mayaPluginLoaderSources = [ 'src/IECoreMaya/plugin/Loader.cpp' ]
		
			mayaPluginLoaderEnv = mayaPluginEnv.Clone()
			mayaPluginLoaderEnv.Append(			
				LIBS = [
					"dl"
				]
			)
			
			mayaPluginLoader = mayaPluginLoaderEnv.SharedLibrary( mayaPluginTarget, mayaPluginLoaderSources, SHLIBPREFIX="" )
			mayaPluginLoaderInstall = mayaPluginLoaderEnv.InstallAs( mayaPluginLoaderEnv.subst( "$INSTALL_MAYAPLUGIN_NAME$SHLIBSUFFIX" ), mayaPluginLoader )
			mayaPluginLoaderEnv.AddPostAction( mayaPluginLoaderInstall, lambda target, source, env : makeSymLinks( mayaPluginLoaderEnv, mayaPluginLoaderEnv["INSTALL_MAYAPLUGIN_NAME"] ) )
			mayaPluginLoaderEnv.Alias( "install", mayaPluginLoaderInstall )
			mayaPluginLoaderEnv.Alias( "installMaya", mayaPluginLoaderInstall )
			
			Default( mayaPluginLoader )
			
			mayaPluginEnv["INSTALL_MAYAPLUGIN_NAME"] = os.path.join( os.path.dirname( mayaPluginEnv["INSTALL_MAYAPLUGIN_NAME"] ), 'impl', os.path.basename( mayaPluginEnv["INSTALL_MAYAPLUGIN_NAME"] ) )
			mayaPluginTarget = "plugins/maya/impl/" + os.path.basename( mayaPluginEnv.subst( "$INSTALL_MAYAPLUGIN_NAME" ) )
		
		mayaPlugin = mayaPluginEnv.SharedLibrary( mayaPluginTarget, mayaPluginSources, SHLIBPREFIX="" )
		mayaPluginInstall = mayaPluginEnv.Install( os.path.dirname( mayaPluginEnv.subst( "$INSTALL_MAYAPLUGIN_NAME" ) ), mayaPlugin )
		mayaPluginEnv.Depends( mayaPlugin, corePythonModule )
		
		mayaPluginEnv.AddPostAction( mayaPluginInstall, lambda target, source, env : makeSymLinks( mayaPluginEnv, mayaPluginEnv["INSTALL_MAYAPLUGIN_NAME"] ) )
		mayaPluginEnv.Alias( "install", mayaPluginInstall )
		mayaPluginEnv.Alias( "installMaya", mayaPluginInstall )
				
		# maya python
		mayaPythonModuleEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( mayaEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ),
			]
		)
		mayaPythonModule = mayaPythonModuleEnv.SharedLibrary( "python/IECoreMaya/_IECoreMaya", mayaPythonSources )
		mayaPythonModuleEnv.Depends( mayaPythonModule, mayaLibrary )

		mayaPythonModuleInstall = mayaPythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreMaya", mayaPythonScripts + mayaPythonModule )
		mayaPythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreMaya", lambda target, source, env : makeSymLinks( mayaPythonModuleEnv, mayaPythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
		mayaPythonModuleEnv.Alias( "install", mayaPythonModuleInstall )
		mayaPythonModuleEnv.Alias( "installMaya", mayaPythonModuleInstall )

		if coreEnv["INSTALL_COREMAYA_POST_COMMAND"]!="" :
			# this is the only way we could find to get a post action to run for an alias
			mayaPythonModuleEnv.Alias( "install", mayaPythonModuleInstall, "$INSTALL_COREMAYA_POST_COMMAND" ) 
			mayaPythonModuleEnv.Alias( "installMaya", mayaPythonModuleInstall, "$INSTALL_COREMAYA_POST_COMMAND" ) 

		Default( [ mayaLibrary, mayaPlugin, mayaPythonModule ] )
		
		mayaTestEnv = testEnv.Clone()
		
		mayaTestLibPaths = mayaEnv.subst( ":".join( [ "./lib" ] + mayaPythonModuleEnv["LIBPATH"] ) )
		if haveRI :
			mayaTestLibPaths += ":" + mayaEnv.subst( "$RMAN_ROOT/lib" )
		mayaTestEnv["ENV"][mayaTestEnv["TEST_LIBRARY_PATH_ENV_VAR"]] += ":" + mayaTestLibPaths
		mayaTestEnv["ENV"][libraryPathEnvVar] += ":" + mayaTestLibPaths
		
		mayaTestEnv["ENV"]["PATH"] = mayaEnv.subst( "$MAYA_ROOT/bin:" ) + mayaEnv["ENV"]["PATH"]
		mayaTestEnv["ENV"]["MAYA_PLUG_IN_PATH"] = "./plugins/maya:./test/IECoreMaya/plugins"
		mayaTestEnv["ENV"]["MAYA_SCRIPT_PATH"] = "./mel"
		
		mayaPythonTestEnv = mayaTestEnv.Clone()
		
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
		mayaTestEnv["ENV"]["AUTODESK_ADLM_THINCLIENT_ENV"] = env["MAYA_ADLM_ENV_FILE"]
		
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
		mayaPythonTestEnv.Depends( mayaPythonTest, glob.glob( "python/IECoreMaya/*.py" ) )
		if env["WITH_MAYA_PLUGIN_LOADER"] :
			mayaPythonTestEnv.Depends( mayaPythonTest, mayaPluginLoader )
		if env["WITH_GL"] :
			mayaPythonTestEnv.Depends( mayaPythonTest, [ glLibrary, glPythonModule ] )
		mayaPythonTestEnv.Alias( "testMaya", mayaPythonTest )			
		mayaPythonTestEnv.Alias( "testMayaPython", mayaPythonTest )			

###########################################################################################
# Build and install the coreNuke library, plugin, python module and headers
###########################################################################################

nukeEnvAppends = {

	"CPPPATH" : [
		"$NUKE_ROOT/include",
		"$GLEW_INCLUDE_PATH",
	],
	
	"CPPFLAGS" : [
		pythonEnv["PYTHON_INCLUDE_FLAGS"],
	],
	
	"LIBPATH" : [
		"$NUKE_ROOT",
		"./lib"
	],

	"LIBS" : [
		"GLEW",
	]

}

nukeEnv = env.Clone( IECORE_NAME = "IECoreNuke" )
nukeEnv.Append( **nukeEnvAppends )
		
nukePythonModuleEnv = pythonModuleEnv.Clone( IECORE_NAME = "IECoreNuke" )
nukePythonModuleEnv.Append( **nukeEnvAppends )

nukePluginEnv = nukeEnv.Clone( IECORE_NAME="ieCore" )

nukeTestEnv = testEnv.Clone()
nukeTestEnv["ENV"]["LM_LICENSE_FILE"] = nukeTestEnv["NUKE_LICENSE_FILE"]
nukeTestEnv["ENV"]["NUKE_PATH"] = "plugins/nuke"

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
				if len( w ) > 1 :
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
				nukeEnv.Append( LIBS = [
					os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
					os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ),
					os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ),
				]	)
				
				nukeEnv.Append( LIBS = [ nukeLibName, "boost_python$BOOST_LIB_SUFFIX" ] )

				nukeEnv.Append(
					CPPFLAGS = [
    					"-DIECORENUKE_NUKE_MAJOR_VERSION=$NUKE_MAJOR_VERSION",
    					"-DIECORENUKE_NUKE_MINOR_VERSION=$NUKE_MINOR_VERSION",
					]
				)
				
				nukePythonModuleEnv.Append( LIBS = os.path.basename( nukeEnv.subst( "$INSTALL_LIB_NAME" ) ) )
				
				nukeHeaders = glob.glob( "include/IECoreNuke/*.h" ) + glob.glob( "include/IECoreNuke/*.inl" )
				nukeSources = glob.glob( "src/IECoreNuke/*.cpp" )
				nukePythonSources = glob.glob( "src/IECoreNuke/bindings/*.cpp" )
				nukePythonScripts = glob.glob( "python/IECoreNuke/*.py" )
				nukePluginSources = glob.glob( "src/IECoreNuke/plugin/*.cpp" )

				# nuke library

				nukeLibrary = nukeEnv.SharedLibrary( "lib/" + os.path.basename( nukeEnv.subst( "$INSTALL_NUKELIB_NAME" ) ), nukeSources )
				nukeLibraryInstall = nukeEnv.Install( os.path.dirname( nukeEnv.subst( "$INSTALL_NUKELIB_NAME" ) ), nukeLibrary )
				nukeEnv.AddPostAction( nukeLibraryInstall, lambda target, source, env : makeLibSymLinks( nukeEnv, "INSTALL_NUKELIB_NAME" ) )
				nukeEnv.Alias( "install", nukeLibraryInstall )
				nukeEnv.Alias( "installNuke", nukeLibraryInstall )
				nukeEnv.Alias( "installLib", [ nukeLibraryInstall ] )

				# nuke headers

				nukeHeaderInstall = nukeEnv.Install( "$INSTALL_HEADER_DIR/IECoreNuke", nukeHeaders )
				nukeEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreNuke", lambda target, source, env : makeSymLinks( nukeEnv, nukeEnv["INSTALL_HEADER_DIR"] ) )
				nukeEnv.Alias( "installNuke", nukeHeaderInstall )
				nukeEnv.Alias( "install", nukeHeaderInstall )

				# nuke python module

				nukePythonModule = nukePythonModuleEnv.SharedLibrary( "python/IECoreNuke/_IECoreNuke", nukePythonSources )
				nukePythonModuleInstall = nukePythonModuleEnv.Install( "$INSTALL_NUKEPYTHON_DIR/IECoreNuke", nukePythonScripts + nukePythonModule )
				nukePythonModuleEnv.AddPostAction( "$INSTALL_NUKEPYTHON_DIR/IECoreNuke", lambda target, source, env : makeSymLinks( nukePythonModuleEnv, nukePythonModuleEnv["INSTALL_NUKEPYTHON_DIR"] ) )
				nukePythonModuleEnv.Alias( "install", nukePythonModuleInstall )
				nukePythonModuleEnv.Alias( "installNuke", nukePythonModuleInstall )

				if coreEnv["INSTALL_CORENUKE_POST_COMMAND"]!="" :
					# this is the only way we could find to get a post action to run for an alias
					nukeEnv.Alias( "install", nukeLibraryInstall, "$INSTALL_CORENUKE_POST_COMMAND" ) 
					nukeEnv.Alias( "installNuke", nukeLibraryInstall, "$INSTALL_CORENUKE_POST_COMMAND" ) 

				# nuke plugin

				nukePluginEnv.Append(
					LIBPATH = [ "./lib" ],
					LIBS = [
						os.path.basename( coreEnv.subst( "$INSTALL_NUKELIB_NAME" ) ),
						os.path.basename( nukeEnv.subst( "$INSTALL_NUKELIB_NAME" ) ),
					]
				)
				nukePluginTarget = "plugins/nuke/" + os.path.basename( nukePluginEnv.subst( "$INSTALL_NUKEPLUGIN_NAME" ) )
				nukePlugin = nukePluginEnv.SharedLibrary( nukePluginTarget, nukePluginSources, SHLIBPREFIX="" )
				nukePluginInstall = nukePluginEnv.Install( os.path.dirname( nukePluginEnv.subst( "$INSTALL_NUKEPLUGIN_NAME" ) ), nukePlugin )
		
				nukePluginEnv.AddPostAction( nukePluginInstall, lambda target, source, env : makeSymLinks( nukePluginEnv, nukePluginEnv["INSTALL_NUKEPLUGIN_NAME"] ) )
				nukePluginEnv.Alias( "install", nukePluginInstall )
				nukePluginEnv.Alias( "installNuke", nukePluginInstall )
		
				Default( [ nukeLibrary, nukePlugin ] )
				
				# stubs for each of nodes within the plugin
				
				nukeStubs = []
				for nodeName in [ "ieProcedural", "ieObject", "ieOp", "ieDrawable" ] :
				
					nukeStubEnv = nukePluginEnv.Clone( IECORE_NAME=nodeName )
					nukeStubName = "plugins/nuke/" + os.path.basename( nukeStubEnv.subst( "$INSTALL_NUKEPLUGIN_NAME" ) ) + ".tcl"
					nukeStub = nukePluginEnv.Command( nukeStubName, nukePlugin, "echo 'load ieCore' > $TARGET" )
					nukeStubInstall = nukeStubEnv.Install( os.path.dirname( nukeStubEnv.subst( "$INSTALL_NUKEPLUGIN_NAME" ) ), nukeStub )
					nukeStubEnv.Alias( "install", nukeStubInstall )
					nukeStubEnv.Alias( "installNuke", nukeStubInstall )
					nukeStubs.append( nukeStub )
					Default( [ nukeStub ] )

				# nuke tests

				nukeTest = nukeTestEnv.Command( "test/IECoreNuke/resultsPython.txt", nukeLibrary, "echo \"execfile( '$TEST_NUKE_SCRIPT' )\" | $NUKE_ROOT/Nuke${NUKE_MAJOR_VERSION}.${NUKE_MINOR_VERSION} -t" )
				NoCache( nukeTest )
				nukeTestEnv.Depends( nukeTest, glob.glob( "test/IECoreNuke/*.py" ) )
				nukeTestEnv.Depends( nukeTest, nukePythonModule )
				nukeTestEnv.Depends( nukeTest, nukePlugin )
				nukeTestEnv.Depends( nukeTest, nukeStubs )
				nukeTestEnv.Alias( "testNuke", nukeTest )

###########################################################################################
# Build, install and test the coreHoudini library and bindings
###########################################################################################

houdiniEnvSets = {
	"IECORE_NAME" : "IECoreHoudini",
	"CXXFLAGS" : [
		"$HOUDINI_CXX_FLAGS", "-DMAKING_DSO", "-DNEED_SPECIALIZATION_STORAGE"
	],
}

houdiniEnvAppends = {
	"CPPPATH" : [
		"contrib/IECoreHoudini/include",
		"contrib/IECoreHoudini/include/bindings",
		"$GLEW_INCLUDE_PATH",
		"$HOUDINI_INCLUDE_PATH",
	],
	"CPPFLAGS" : [
		pythonEnv["PYTHON_INCLUDE_FLAGS"],
	],
	"LIBPATH" : [
		"$HOUDINI_LIB_PATH",
		"$GLEW_LIB_PATH",
	],
	"LIBS" : [
		"HoudiniUI",
  		"HoudiniOPZ",
  		"HoudiniOP3",
  		"HoudiniOP2",
  		"HoudiniOP1",
  		"HoudiniSIM",
  		"HoudiniGEO",
  		"HoudiniPRM",
  		"HoudiniUT",
		"boost_python" + env["BOOST_LIB_SUFFIX"],
		"GLEW"
	]
}

if env["PLATFORM"]=="posix" :
	houdiniEnvAppends["CPPFLAGS"] += ["-DLINUX"]
elif env["PLATFORM"]=="darwin" :
	houdiniEnvAppends["CPPFLAGS"] += ["-D__APPLE__"]
	houdiniEnvAppends["FRAMEWORKS"] = ["OpenGL"]
	houdiniEnvAppends["LIBS"] += [ "GR"]

houdiniEnv = env.Clone( **houdiniEnvSets )
houdiniEnv.Append( **houdiniEnvAppends )

houdiniEnv.Append( SHLINKFLAGS = pythonEnv["PYTHON_LINK_FLAGS"].split() )
houdiniEnv.Append( SHLINKFLAGS = "$HOUDINI_LINK_FLAGS" )

houdiniPythonModuleEnv = pythonModuleEnv.Clone( **houdiniEnvSets )
houdiniPythonModuleEnv.Append( **houdiniEnvAppends )

houdiniPluginEnv = houdiniEnv.Clone( IECORE_NAME="ieCoreHoudini" )

if doConfigure :
	
	c = Configure( houdiniEnv )
	
	if not c.CheckHeader( "SOP/SOP_API.h" ) :
		
		sys.stderr.write( "WARNING : no houdini devkit found, not building IECoreHoudini - check HOUDINI_ROOT.\n" )
		c.Finish()
	
	else :
		
		c.Finish()
		
		#=====
		# glob the files
		#=====
		houdiniSources = glob.glob( "contrib/IECoreHoudini/src/*.cpp" )
		houdiniHeaders = glob.glob( "contrib/IECoreHoudini/include/*.h" ) + glob.glob( "contrib/IECoreHoudini/include/*.inl" )
		houdiniBindingHeaders = glob.glob( "contrib/IECoreHoudini/include/bindings/*.h" ) + glob.glob( "contrib/IECoreHoudini/include/bindings/*.inl" )
		houdiniPythonSources = glob.glob( "contrib/IECoreHoudini/src/bindings/*.cpp" )
		houdiniPythonScripts = glob.glob( "contrib/IECoreHoudini/python/IECoreHoudini/*.py" )
		houdiniPluginSources = [ "contrib/IECoreHoudini/src/plugin/Plugin.cpp" ]
		
		# we can't append this before configuring, as then it gets built as
		# part of the configure process
		houdiniEnv.Prepend( LIBPATH = [ "./lib" ] )
		houdiniEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )
		houdiniEnv.Append( LIBS = os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ) )
		houdiniEnv.Append( LIBS = os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ) )
		
		#=====
		# build library
		#=====
		houdiniLib = houdiniEnv.SharedLibrary( "lib/" + os.path.basename( houdiniEnv.subst( "$INSTALL_HOUDINILIB_NAME" ) ), houdiniSources )
		houdiniLibInstall = houdiniEnv.Install( os.path.dirname( houdiniEnv.subst( "$INSTALL_HOUDINILIB_NAME" ) ), houdiniLib )
		houdiniEnv.NoCache( houdiniLibInstall )
		houdiniEnv.AddPostAction( houdiniLibInstall, lambda target, source, env : makeLibSymLinks( houdiniEnv, "INSTALL_HOUDINILIB_NAME" ) )
		houdiniEnv.Alias( "install", houdiniLibInstall )
		houdiniEnv.Alias( "installHoudini", houdiniLibInstall )
		houdiniEnv.Alias( "installLib", [ houdiniLibInstall ] )
		
 		#=====
		# install headers
		#=====
		houdiniHeaderInstall = houdiniEnv.Install( "$INSTALL_HEADER_DIR/IECoreHoudini", houdiniHeaders )
		houdiniHeaderInstall += houdiniEnv.Install( "$INSTALL_HEADER_DIR/IECoreHoudini/bindings", houdiniBindingHeaders )		
		houdiniEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreHoudini", lambda target, source, env : makeSymLinks( houdiniEnv, houdiniEnv["INSTALL_HEADER_DIR"] ) )
		houdiniEnv.Alias( "install", houdiniHeaderInstall )
		houdiniEnv.Alias( "installHoudini", houdiniHeaderInstall )
		
		#=====
		# build houdini plugin
		#=====
		houdiniPluginEnv.Append(
			LIBPATH = [ "./lib" ],
			LIBS=[
				os.path.basename( houdiniEnv.subst( "$INSTALL_HOUDINILIB_NAME" ) ),
			],
		)
		houdiniPluginTarget = "plugins/houdini/" + os.path.basename( houdiniPluginEnv.subst( "$INSTALL_HOUDINIPLUGIN_NAME" ) )
		houdiniPlugin = houdiniPluginEnv.SharedLibrary( houdiniPluginTarget, houdiniPluginSources, SHLIBPREFIX="" )
		houdiniPluginInstall = houdiniPluginEnv.Install( os.path.dirname( houdiniPluginEnv.subst( "$INSTALL_HOUDINIPLUGIN_NAME" ) ), houdiniPlugin )
		houdiniPluginEnv.Depends( houdiniPlugin, corePythonModule )
		houdiniPluginEnv.AddPostAction( houdiniPluginInstall, lambda target, source, env : makeSymLinks( houdiniPluginEnv, houdiniPluginEnv["INSTALL_HOUDINIPLUGIN_NAME"] ) )
		houdiniPluginEnv.Alias( "install", houdiniPluginInstall )
		houdiniPluginEnv.Alias( "installHoudini", houdiniPluginInstall )
		
		#=====
		# build python module
		#=====
		houdiniPythonModuleEnv.Append(
			LIBPATH = [ "./lib" ],
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( houdiniEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ),
			]
		)
		houdiniPythonModule = houdiniPythonModuleEnv.SharedLibrary( "contrib/IECoreHoudini/python/IECoreHoudini/_IECoreHoudini", houdiniPythonSources )
		houdiniPythonModuleEnv.Depends( houdiniPythonModule, houdiniLib )
		houdiniPythonModuleInstall = houdiniPythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreHoudini", houdiniPythonScripts + houdiniPythonModule )
		houdiniPythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreHoudini", lambda target, source, env : makeSymLinks( houdiniPythonModuleEnv, houdiniPythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
		houdiniPythonModuleEnv.Alias( "install", houdiniPythonModuleInstall )
		houdiniPythonModuleEnv.Alias( "installHoudini", houdiniPythonModuleInstall )
		
		#=====
		# build otls
		#=====
		otlPath = "contrib/IECoreHoudini/otls/ieCoreHoudini"
		buildPath = "contrib/IECoreHoudini/otls/build"
		otlTarget = "plugins/houdini/" + os.path.basename( houdiniPluginEnv.subst( "$IECORE_NAME" ) ) + ".otl"
		otlCommand = houdiniPluginEnv.Command( otlTarget, otlPath, "cp -r %s %s; $HOUDINI_BIN_PATH/hotl -C %s $TARGET; rm -rf %s" % ( otlPath, buildPath, buildPath, buildPath ) )
		houdiniPluginEnv.Depends( otlTarget, glob.glob( otlPath + "/*" ) + glob.glob( otlPath + "/*/*" ) + glob.glob( otlPath + "/*/*/*" ) + glob.glob( otlPath + "/*/*/*/*" ) )
		otlInstall = houdiniPluginEnv.Install( "$INSTALL_HOUDINIOTL_DIR", source=[ otlTarget ] )
		houdiniPluginEnv.AddPostAction( "$INSTALL_HOUDINIOTL_DIR", lambda target, source, env : makeSymLinks( houdiniPluginEnv, houdiniPluginEnv["INSTALL_HOUDINIOTL_DIR"] ) )
		houdiniPluginEnv.Alias( "install", otlInstall )
		houdiniPluginEnv.Alias( "installHoudini", otlInstall )
		
		#=====
		# install icons
		#=====
		houdiniIcons = glob.glob( "contrib/IECoreHoudini/icons/*.svg" )
		houdiniIconInstall = houdiniPluginEnv.Install( "$INSTALL_HOUDINIICON_DIR", source=houdiniIcons )
		houdiniPluginEnv.Alias( "install", houdiniIconInstall )
		houdiniPluginEnv.Alias( "installHoudini", houdiniIconInstall )
		
		#=====
		# install toolbar
		#=====
		houdiniToolbars = glob.glob( "contrib/IECoreHoudini/toolbar/*.shelf" )
		houdiniToolbarInstall = houdiniPluginEnv.Install( "$INSTALL_HOUDINITOOLBAR_DIR", source=houdiniToolbars )
		houdiniPluginEnv.Alias( "install", houdiniToolbarInstall )
		houdiniPluginEnv.Alias( "installHoudini", houdiniToolbarInstall )
		
		Default( [ houdiniLib, houdiniPlugin, houdiniPythonModule, otlCommand ] )

		#=====
		# Houdini tests
		#=====
		houdiniTestEnv = testEnv.Copy()
		
		houdiniTestLibPaths = houdiniEnv.subst( ":".join( [ "./lib" ] + houdiniPythonModuleEnv["LIBPATH"] ) )
		if haveRI :
			houdiniTestLibPaths += ":" + houdiniEnv.subst( "$RMAN_ROOT/lib" )
		houdiniTestEnv["ENV"][houdiniTestEnv["TEST_LIBRARY_PATH_ENV_VAR"]] += ":" + houdiniTestLibPaths
		houdiniTestEnv["ENV"][libraryPathEnvVar] += ":" + houdiniTestLibPaths
		
		houdiniTestEnv["ENV"]["PATH"] = houdiniEnv.subst( "$HOUDINI_ROOT/bin:" ) + houdiniEnv["ENV"]["PATH"]
		
		houdiniTestEnv.Append( **houdiniEnvAppends )
		houdiniTestEnv.Append( 
			LIBS = [ 
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ), 
				os.path.basename( houdiniEnv.subst( "$INSTALL_LIB_NAME" ) ), 				
			]
		)
		
		houdiniTestEnv["ENV"]["PYTHONPATH"] += ":./contrib/IECoreHoudini/python"
		houdiniTestEnv["ENV"]["HOUDINI_DSO_PATH"] = "./plugins/houdini:&"
		houdiniTestEnv["ENV"]["HOUDINI_OTLSCAN_PATH"] = "./plugins/houdini:&"
		
		houdiniTestEnv["ENV"]["IECORE_OP_PATHS"] = "./contrib/IECoreHoudini/test/ops"
		houdiniTestEnv["ENV"]["IECORE_PROCEDURAL_PATHS"] = "./contrib/IECoreHoudini/test/procedurals"
		
		houdiniPythonExecutable = "hython"
		
		houdiniPythonTest = houdiniTestEnv.Command( "contrib/IECoreHoudini/test/resultsPython.txt", houdiniPythonModule, houdiniPythonExecutable + " $TEST_HOUDINI_SCRIPT" )
		NoCache( houdiniPythonTest )
		houdiniTestEnv.Depends( houdiniPythonTest, [ houdiniPlugin, houdiniPythonModule ] )
		houdiniTestEnv.Depends( houdiniPythonTest, glob.glob( "contrib/IECoreHoudini/test/*.py" ) )
		houdiniTestEnv.Depends( houdiniPythonTest, glob.glob( "contrib/IECoreHoudini/python/IECoreHoudini/*.py" ) )
		if env["WITH_GL"] :
			houdiniTestEnv.Depends( houdiniPythonTest, [ glLibrary, glPythonModule ] )
		houdiniTestEnv.Alias( "testHoudini", houdiniPythonTest )
		houdiniTestEnv.Alias( "testHoudiniPython", houdiniPythonTest )

###########################################################################################
# Build and install the coreTruelight library and headers
###########################################################################################

truelightEnv = env.Clone( IECORE_NAME = "IECoreTruelight" )
truelightEnv.Append( LIBS = [ "truelight" ] )

# Remove all the boost and OpenEXR libs for the configure state- if we don't do this then the configure test can fail for some compilers. 
# \todo Need to establish exactly what is going on here.
oldTruelightLibs = list( truelightEnv["LIBS"] )
truelightEnv["LIBS"] = [ x for x in truelightEnv["LIBS"] if ( x.find( "boost_" ) == -1 and x.find( "Ilm" ) == -1 and x.find( "Iex" ) == -1 and x.find( "Half" )==-1 and x.find( "Imath" )==-1 ) ] 

truelightEnv.Append( CPPPATH = [ "$TRUELIGHT_ROOT/include" ] )
truelightEnv.Prepend( LIBPATH = [
		"./lib",
		"$TRUELIGHT_ROOT/lib"
	]
)

truelightPythonModuleEnv = pythonModuleEnv.Clone( IECORE_NAME="IECoreTruelight" )

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
		truelightLibraryInstall = truelightEnv.Install( os.path.dirname( truelightEnv.subst( "$INSTALL_LIB_NAME" ) ), truelightLibrary )
		truelightEnv.NoCache( truelightLibraryInstall )
		truelightEnv.AddPostAction( truelightLibraryInstall, lambda target, source, env : makeLibSymLinks( truelightEnv ) )
		truelightEnv.Alias( "install", truelightLibraryInstall )
		truelightEnv.Alias( "installTruelight", truelightLibraryInstall )
		truelightEnv.Alias( "installLib", [ truelightLibraryInstall ] )

		# headers
		truelightHeaderInstall = truelightEnv.Install( "$INSTALL_HEADER_DIR/IECoreTruelight", truelightHeaders )
		truelightEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreTruelight", lambda target, source, env : makeSymLinks( truelightEnv, truelightEnv["INSTALL_HEADER_DIR"] ) )
		truelightEnv.Alias( "installTruelight", truelightHeaderInstall )
		truelightEnv.Alias( "install", truelightHeaderInstall )
		
		# python
		truelightPythonModuleEnv.Append( LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( truelightEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ),
			]
		)

		truelightPythonModule = truelightPythonModuleEnv.SharedLibrary( "python/IECoreTruelight/_IECoreTruelight", truelightPythonSources )
		truelightPythonModuleEnv.Depends( truelightPythonModule, corePythonModule )
		truelightPythonModuleEnv.Depends( truelightPythonModule, truelightLibrary )
		
		truelightPythonModuleInstall = truelightPythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreTruelight", truelightPythonScripts + truelightPythonModule )
		truelightPythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreTruelight", lambda target, source, env : makeSymLinks( truelightPythonModuleEnv, truelightPythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
		truelightPythonModuleEnv.Alias( "install", truelightPythonModuleInstall )
		truelightPythonModuleEnv.Alias( "installTruelight", truelightPythonModuleInstall )
		
		if coreEnv["INSTALL_CORETRUELIGHT_POST_COMMAND"]!="" :
			# this is the only way we could find to get a post action to run for an alias
			truelightEnv.Alias( "install", truelightLibraryInstall, "$INSTALL_CORETRUELIGHT_POST_COMMAND" ) 
			truelightEnv.Alias( "installTruelight", truelightLibraryInstall, "$INSTALL_CORETRUELIGHT_POST_COMMAND" ) 

		Default( [ truelightLibrary, truelightPythonModule ] )
		
		# tests
		truelightTestEnv = testEnv.Clone()
		truelightTestEnv["ENV"]["TRUELIGHT_ROOT"] = truelightEnv.subst( "$TRUELIGHT_ROOT" )
		truelightTest = truelightTestEnv.Command( "test/IECoreTruelight/results.txt", truelightPythonModule, pythonExecutable + " $TEST_TRUELIGHT_SCRIPT" )
		NoCache( truelightTest )
		truelightTestEnv.Depends( truelightTest, truelightPythonModule )
		truelightTestEnv.Depends( truelightTest, glob.glob( "test/IECoreTruelight/*.py" ) )
		truelightTestEnv.Alias( "testTruelight", truelightTest )
		
###########################################################################################
# Documentation
###########################################################################################

docEnv = env.Clone()
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
		
		docs = docEnv.Command( "doc/html/index.html", "doc/config/Doxyfile", "sed s/!CORTEX_VERSION!/$IECORE_MAJORMINORPATCH_VERSION/g $SOURCE | $DOXYGEN -" )
		docEnv.NoCache( docs )
		
		for modulePath in ( "python/IECore", "python/IECoreRI", "python/IECoreGL", "python/IECoreNuke", "python/IECoreMaya", "contrib/IECoreHoudini/python/IECoreHoudini" ) :
			
			module = os.path.basename( modulePath )
			mungedModule = docEnv.Command( "doc/python/" + module, modulePath + "/__init__.py", createDoxygenPython )
			docEnv.Depends( mungedModule, glob.glob( modulePath + "/*.py" ) )
			docEnv.Depends( docs, mungedModule )
			docEnv.NoCache( mungedModule )
		
		for inputDirectory in doxyfile["INPUT"].split( ' ' ) :
		
			for filePattern in doxyfile["FILE_PATTERNS"].split( ' ' ) :
			
				docEnv.Depends( docs, glob.glob( inputDirectory + "/" + filePattern ) )
				
		docEnv.Depends( docs, doxyfile["HTML_HEADER"] )
		docEnv.Depends( docs, doxyfile["HTML_FOOTER"] )
		docEnv.Depends( docs, doxyfile["HTML_STYLESHEET"] )						
				
		docEnv.Alias( "doc", "doc/html/index.html" )
		
		# \todo This won't reinstall the documentation if the directory already exists
		installDoc = docEnv.Install( "$INSTALL_DOC_DIR", "doc/html" )
		docEnv.Alias( "installDoc", installDoc )
		
	else: 
	
		sys.stdout.write( "no\n" )
		sys.stderr.write( "WARNING : no doxygen binary found, not building documentation - check DOXYGEN\n" )

	
