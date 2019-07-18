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

EnsureSConsVersion( 0, 97 )
SConsignFile()

ieCoreMajorVersion=10
ieCoreMinorVersion=0
ieCorePatchVersion=0
ieCoreVersionSuffix="a58" # used for alpha/beta releases. Example: "a1", "b2", etc.

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
	[ "-pipe", "-Wall" ]
)

o.Add(
	"CXXSTD",
	"The C++ standard to build against.",
	"c++11",
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
	"to link against libraries installed with non-default names.",
	"",
)

# Boost options

o.Add(
	"BOOST_INCLUDE_PATH",
	"The path to the boost include directory.",
	"/usr/include",
)

o.Add(
	"BOOST_LIB_PATH",
	"The path to the boost library directory.",
	"/usr/lib",
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
	"/usr/include/OpenEXR",
)

o.Add(
	"OPENEXR_LIB_PATH",
	"The path to the OpenEXR lib directory.",
	"/usr/lib",
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
	"/usr/include/freetype2",
)

o.Add(
	"FREETYPE_LIB_PATH",
	"The path to the FreeType lib directory.",
	"/usr/local/lib",
)

# OSL options

o.Add(
	"OSL_INCLUDE_PATH",
	"The path to the OpenShadingLanguage include directory.",
	"/usr/include",
)

o.Add(
	"OSL_LIB_PATH",
	"The path to the OpenShadingLanguage library directory.",
	"/usr/lib",
)

# OIIO options

o.Add(
	"OIIO_INCLUDE_PATH",
	"The path to the OpenImageIO include directory.",
	"/usr/include",
)

o.Add(
	"OIIO_LIB_PATH",
	"The path to the OpenImageIO library directory.",
	"/usr/lib",
)

o.Add(
	"OIIO_LIB_SUFFIX",
	"The suffix appended to the names of the OpenImageIO libraries. You can modify this "
	"to link against libraries installed with non-defalt names.",
	"",
)

# Blosc options

o.Add(
	"BLOSC_INCLUDE_PATH",
	"The path to the Blosc include directory.",
	"/usr/include",
)
o.Add(
	"BLOSC_LIB_PATH",
	"The path to the Blosc library directory.",
	"/usr/lib",
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
	"A colon separated list of paths to search for headers on.",
	"",
)

o.Add(
	"LIBPATH",
	"A colon separated list of paths to search for libraries on.",
	"/usr/lib",
)

o.Add(
	"PYTHONPATH",
	"A colon separated list of paths to search for python modules on.",
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
	"/usr/local/include",
)

o.Add(
	"GLEW_LIB_PATH",
	"The path to the directory with libGLEW in it.",
	"/usr/local/lib",
)

o.Add(
	"GLEW_LIB_SUFFIX",
	"The suffix appended to the names of the GLEW library. You can modify this "
	"to link against libraries installed with non-defalt names.",
	"",
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

# Arnold options

o.Add(
	"ARNOLD_ROOT",
	"The directory in which Arnold is installed.",
	"/usr/local",
)

o.Add(
	"USD_INCLUDE_PATH",
	"The path to the USD include directory.",
	"/usr/local/include",
)

o.Add(
	"USD_LIB_PATH",
	"The path to the USD lib directory.",
	"/usr/local/lib",
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
	"/usr/local/include",
)

o.Add(
	"ALEMBIC_LIB_PATH",
	"The path to the Alembic lib directory.",
	"/usr/local/lib",
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
	"/usr/local/include",
)

o.Add(
	"HDF5_LIB_PATH",
	"The path to the hdf5 lib directory.",
	"/usr/local/lib",
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
	"/usr/local/include",
)

o.Add(
	"VDB_LIB_PATH",
	"The path to the OpenVDB lib directory.",
	"/usr/local/lib",
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
	"/usr/local/lib",
)

# appleseed options

o.Add(
	"APPLESEED_INCLUDE_PATH",
	"The path to the appleseed include directory.",
	"/usr/local/appleseed/include",
)

o.Add(
	"APPLESEED_LIB_PATH",
	"The path to the appleseed lib directory.",
	"/usr/local/appleseed/lib",
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
	"INSTALL_ARNOLDLIB_NAME",
	"The name under which to install the Arnold libraries. This "
	"can be used to build and install the library for multiple "
	"Arnold versions.",
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
	"INSTALL_APPLESEEDLIB_NAME",
	"The name under which to install the appleseed libraries. This "
	"can be used to build and install the library for multiple "
	"appleseed versions.",
	"$INSTALL_PREFIX/lib/$IECORE_NAME",
)

o.Add(
	"INSTALL_PYTHON_DIR",
	"The directory in which to install python modules.",
	"$INSTALL_PREFIX/lib/python$PYTHON_VERSION/site-packages",
)

o.Add(
	"INSTALL_ARNOLDPYTHON_DIR",
	"The directory in which to install the Arnold python module.",
	"$INSTALL_PYTHON_DIR",
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
	"INSTALL_RMANDISPLAY_NAME",
	"The name under which to install the renderman displays.",
	"$INSTALL_PREFIX/rmanDisplays/$IECORE_NAME",
)

o.Add(
	"INSTALL_MEL_DIR",
	"The directory in which to install mel scripts.",
	"$INSTALL_PREFIX/maya/mel/$IECORE_NAME",
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
	"INSTALL_HOUDINIMENU_DIR",
	"The directory under which to install houdini menu files.",
	"$INSTALL_PREFIX/houdini",
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
	"INSTALL_ARNOLDOUTPUTDRIVER_NAME",
	"The name under which to install the arnold procedurals.",
	"$INSTALL_PREFIX/arnoldOutputDrivers/$IECORE_NAME",
)

o.Add(
	"INSTALL_APPLESEEDOUTPUTDRIVER_NAME",
	"The name under which to install the appleseed output driver.",
	"$INSTALL_PREFIX/appleseedDisplays/$IECORE_NAME",
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
		( "IECore.SequenceLsOp", "common/fileSystem/seqLs" ),
		( "IECore.SequenceCpOp", "common/fileSystem/seqCp" ),
		( "IECore.SequenceMvOp", "common/fileSystem/seqMv" ),
		( "IECore.SequenceRmOp", "common/fileSystem/Rm" ),
		( "IECore.SequenceRenumberOp", "common/fileSystem/seqRenumber" ),
		( "IECore.SequenceConvertOp", "common/fileSystem/seqConvert" ),
		( "IECore.SequenceCatOp", "common/fileSystem/seqCat" ),
		( "IECoreScene.RemovePrimitiveVariables", "common/primitive/removeVariables" ),
		( "IECoreScene.RenamePrimitiveVariables", "common/primitive/renameVariables" ),
		( "IECore.ClassLsOp", "common/classes/classLs" ),
		( "IECore.LsHeaderOp", "common/fileSystem/lsHeader" ),
		( "IECore.SearchReplaceOp", "common/fileSystem/searchReplace" ),
		( "IECoreScene.TriangulateOp", "common/primitive/mesh/triangulate" ),
		( "IECoreScene.MeshNormalsOp", "common/primitive/mesh/addNormals" ),
		( "IECoreScene.MeshMergeOp", "common/primitive/mesh/merge" ),
		( "IECoreScene.MeshVertexReorderOp", "common/primitive/mesh/vertexReorder" ),
		( "IECoreScene.MeshPrimitiveShrinkWrapOp", "common/primitive/mesh/shrinkWrap" ),
		( "IECoreScene.AddSmoothSkinningInfluencesOp", "rigging/smoothSkinning/addInfluences" ),
		( "IECoreScene.RemoveSmoothSkinningInfluencesOp", "rigging/smoothSkinning/removeInfluences" ),
		( "IECoreScene.CompressSmoothSkinningDataOp", "rigging/smoothSkinning/compress" ),
		( "IECoreScene.DecompressSmoothSkinningDataOp", "rigging/smoothSkinning/decompress" ),
		( "IECoreScene.NormalizeSmoothSkinningWeightsOp", "rigging/smoothSkinning/normalizeWeights" ),
		( "IECoreScene.ReorderSmoothSkinningInfluencesOp", "rigging/smoothSkinning/reorderInfluences" ),
		( "IECoreScene.SmoothSmoothSkinningWeightsOp", "rigging/smoothSkinning/smoothWeights" ),
		( "IECoreScene.ContrastSmoothSkinningWeightsOp", "rigging/smoothSkinning/contrastWeights" ),
		( "IECoreScene.LimitSmoothSkinningInfluencesOp", "rigging/smoothSkinning/limitInfluences" ),
		( "IECoreScene.TransferSmoothSkinningWeightsOp", "rigging/smoothSkinning/transferWeights" ),
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
	"INSTALL_COREHOUDINI_POST_COMMAND",
	"A command which is run following a successful installation of "
	"the CoreHoudini library. This could be used to customise installation "
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
	"test/IECoreHoudini/All.py"
)

o.Add(
	"TEST_NUKE_SCRIPT",
	"The python script to run for the nuke tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"test/IECoreNuke/All.py"
)

o.Add(
	"TEST_ARNOLD_SCRIPT",
	"The python script to run for the arnold tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"contrib/IECoreArnold/test/IECoreArnold/All.py"
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
	"TEST_APPLESEED_SCRIPT",
	"The python script to run for the appleseed tests. The default will run all the tests, "
	"but it can be useful to override this to run just the test for the functionality "
	"you're working on.",
	"contrib/IECoreAppleseed/test/IECoreAppleseed/All.py"
)

o.Add(
	"TEST_LIBPATH",
	"Additional colon separated paths to be prepended to the library path"
	"used when running tests.",
	""
)

if Environment()["PLATFORM"]=="darwin" :
	libraryPathEnvVar = "DYLD_LIBRARY_PATH"
else :
	libraryPathEnvVar = "LD_LIBRARY_PATH"

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
	"doxygen"
)

###########################################################################################
# An environment for building libraries
###########################################################################################

env = Environment(
	options = o
)

if isinstance( env["LIBPATH"], basestring ) :
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
env["IECORE_VERSION_SUFFIX"] = ieCoreVersionSuffix
env["IECORE_MAJORMINOR_VERSION"] = "${IECORE_MAJOR_VERSION}.${IECORE_MINOR_VERSION}"
env["IECORE_MAJORMINORPATCH_VERSION"] = "${IECORE_MAJOR_VERSION}.${IECORE_MINOR_VERSION}.${IECORE_PATCH_VERSION}"
if ieCoreVersionSuffix :
	env["IECORE_MAJORMINORPATCH_VERSION"] += "-${IECORE_VERSION_SUFFIX}"

env.Append(
	CPPFLAGS = [
		"-DIE_CORE_MAJORVERSION=$IECORE_MAJOR_VERSION",
		"-DIE_CORE_MINORVERSION=$IECORE_MINOR_VERSION",
		"-DIE_CORE_PATCHVERSION=$IECORE_PATCH_VERSION",
		"-DBOOST_FILESYSTEM_VERSION=3",
	]
)

# update the include and lib paths
dependencyIncludes = [
	"-isystem", "$BOOST_INCLUDE_PATH",
	"-isystem", "$OPENEXR_INCLUDE_PATH",
	"-isystem", "$ILMBASE_INCLUDE_PATH",
	"-isystem", "$TBB_INCLUDE_PATH",
	"-isystem", "$BLOSC_INCLUDE_PATH",
	# we use "OpenEXR/x.h" and they use "x.h"
	"-isystem", os.path.join( "$OPENEXR_INCLUDE_PATH","OpenEXR" ),
	"-isystem", os.path.join( "$ILMBASE_INCLUDE_PATH","OpenEXR" ),
	"-isystem", "$FREETYPE_INCLUDE_PATH",
]

env.Prepend(
	CXXFLAGS = dependencyIncludes,
	CPPPATH = [
		"include",
	],
	LIBPATH = [
		"./lib",
		"$TBB_LIB_PATH",
		"$BOOST_LIB_PATH",
		"$OPENEXR_LIB_PATH",
		"$ILMBASE_LIB_PATH",
		"$FREETYPE_LIB_PATH",
		"$BLOSC_LIB_PATH"
	],
	LIBS = [
		"pthread",
	]
)

if env["PLATFORM"]=="darwin" :
	# necessary to fix errors from boost/numeric/interval.hpp
	env.Append( CXXFLAGS = [ "-D__USE_ISOC99" ] )
	# os x versions before snow leopard require the no-long-double flag
	compilerVersion = map( int, env["CXXVERSION"].split( "." ) )
	if compilerVersion[0] < 4 or compilerVersion[0]==4 and compilerVersion[1] < 2 :
		env.Append( CXXFLAGS = "-Wno-long-double" )
	osxVersion = [ int( v ) for v in platform.mac_ver()[0].split( "." ) ]
	# Work around problem with unused local typedefs in boost and
	# deprecation of gluBuild2DMipmaps() in OSX 10.9.
	if osxVersion[0] == 10 and osxVersion[1] > 7 :
		env.Append( CXXFLAGS = [ "-Wno-unused-local-typedef", "-Wno-deprecated-declarations" ] )

env.Append( CXXFLAGS = [ "-std=$CXXSTD", "-fvisibility=hidden" ] )

if "clang++" in os.path.basename( env["CXX"] ) :
	env.Append( CXXFLAGS = ["-Wno-unused-local-typedef"] )

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
	boostVersionHeader = env.FindFile( "boost/version.hpp", env["CXXFLAGS"] )
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
			"boost_timer" + env["BOOST_LIB_SUFFIX"],
			"boost_chrono" + env["BOOST_LIB_SUFFIX"]
		]
	)
	if int( env["BOOST_MINOR_VERSION"] ) >=35 :
		env.Append( LIBS = [ "boost_system" + env["BOOST_LIB_SUFFIX"] ] )

	if int( env["BOOST_MINOR_VERSION"] ) >=55 :
		env.Append( CXXFLAGS = [ "-DBOOST_SIGNALS_NO_DEPRECATION_WARNING" ] )

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

	f = subprocess.Popen( env["PYTHON_CONFIG"] + " " + flags, env=env["ENV"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True )
	stdOut, stdErr = f.communicate()
	r = stdOut.strip()
	if f.returncode :
		sys.stderr.write( "ERROR : Error running \"%s\".\n%s" % ( env["PYTHON_CONFIG"], stdErr ) )
		Exit( 1 )
	return r

pythonEnv = env.Clone()

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
	pythonEnv["PYTHON_INCLUDE_FLAGS"] = [ "-isystem", "$PYTHON_INCLUDE_PATH" ]
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

pythonEnv.Append( CPPFLAGS = "-DBOOST_PYTHON_MAX_ARITY=20" )
pythonEnv.Append( LIBS = [
		"boost_python" + pythonEnv["BOOST_LIB_SUFFIX"],
	]
)

pythonModuleEnv = pythonEnv.Clone()

pythonModuleEnv["SHLIBPREFIX"] = ""
pythonModuleEnv["SHLIBSUFFIX"] = ".so"

if pythonModuleEnv["PLATFORM"]=="darwin" :
	pythonModuleEnv.Append( SHLINKFLAGS = "-single_module" )

###########################################################################################
# An environment for running tests
###########################################################################################

testEnv = env.Clone()

testEnvLibPath = ":".join( testEnv["LIBPATH"] )
if testEnv["TEST_LIBPATH"] != "" :
	testEnvLibPath += ":" + testEnv["TEST_LIBPATH"]
testEnvLibPath = testEnv.subst( testEnvLibPath )

testEnv["ENV"][testEnv["TEST_LIBRARY_PATH_ENV_VAR"]] = testEnvLibPath
testEnv["ENV"][libraryPathEnvVar] = testEnvLibPath
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

testEnv["ENV"]["PYTHONPATH"] = "./python:" + testEnv.subst( "$PYTHONPATH" )

###########################################################################################
# Helper functions
###########################################################################################

# Makes versioned symlinks for use during installation
def makeSymLinks( env, target ) :

	links = {
		"$IECORE_MAJORMINORPATCH_VERSION" : "$IECORE_MAJORMINOR_VERSION",
		"$IECORE_MAJORMINOR_VERSION" : "$IECORE_MAJOR_VERSION",
		"${IECORE_MAJORMINORPATCH_VERSION}" : "${IECORE_MAJORMINOR_VERSION}",
		"${IECORE_MAJORMINOR_VERSION}" : "${IECORE_MAJOR_VERSION}",
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

# installs the core/python build configurations to a pkg-config configuration file
def writePkgConfig( env, python_env ):
	global ieCoreMajorVersion, ieCoreMinorVersion, ieCorePatchVersion
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
	fd.write( "Version: %d.%d.%d\n" % ( ieCoreMajorVersion, ieCoreMinorVersion, ieCorePatchVersion ) )
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

# shared configure checks
if doConfigure :

	freetypeEnv = coreEnv.Clone()
	c = Configure( freetypeEnv )

	if c.CheckLibWithHeader( "freetype", ["ft2build.h"], "CXX" ) :
		for e in allCoreEnvs :
			e.Append( CPPFLAGS = "-DIECORE_WITH_FREETYPE" )
	else :
		sys.stderr.write( "WARNING: no FreeType library found, no font support, check FREETYPE_INCLUDE_PATH and FREETYPE_LIB_PATH.\n" )

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
if env["INSTALL_PKG_CONFIG_FILE"]:
        coreEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECore", lambda target, source, env : writePkgConfig( coreEnv, corePythonEnv ) )
coreEnv.Alias( "install", headerInstall )
coreEnv.Alias( "installCore", headerInstall )

# python library
corePythonEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )
corePythonLibrary = corePythonEnv.SharedLibrary( "lib/" + os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ), corePythonSources )
corePythonLibraryInstall = corePythonEnv.Install( os.path.dirname( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ), corePythonLibrary )
corePythonEnv.NoCache( corePythonLibraryInstall )
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
corePythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECore", lambda target, source, env : makeSymLinks( corePythonEnv, corePythonEnv["INSTALL_PYTHON_DIR"] ) )
corePythonModuleEnv.Alias( "install", corePythonModuleInstall )
corePythonModuleEnv.Alias( "installCore", corePythonModuleInstall )

# stubs
for cls in env['INSTALL_IECORE_OPS'] :
	stubName = os.path.basename( cls[1] )
	stubEnv = corePythonModuleEnv.Clone( IECORE_NAME=os.path.join( cls[1], stubName ) )
	stubInstall = stubEnv.Command( "$INSTALL_IECORE_OP_PATH", None, 'echo "from %s import %s as %s" > $TARGET' % ( cls[0].rpartition( "." )[0], cls[0].rpartition( "." )[-1], stubName ) )
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

coreTestEnv.Append(
	LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
	CPPPATH = [ "test/IECore" ],
)

coreTestSources = glob.glob( "test/IECore/*.cpp" )
coreTestProgram = coreTestEnv.Program( "test/IECore/IECoreTest", coreTestSources )

coreTest = coreTestEnv.Command( "test/IECore/results.txt", coreTestProgram, "test/IECore/IECoreTest > test/IECore/results.txt 2>&1" )
NoCache( coreTest )
coreTestEnv.Alias( "testCore", coreTest )

corePythonTest = coreTestEnv.Command( "test/IECore/resultsPython.txt", corePythonModule, pythonExecutable + " $TEST_CORE_SCRIPT" )
coreTestEnv.Depends( corePythonTest, glob.glob( "test/IECore/*.py" ) )
NoCache( corePythonTest )
coreTestEnv.Alias( "testCorePython", corePythonTest )

###########################################################################################
# Build, install and test the IECoreImage library and bindings
###########################################################################################

imageEnvSets = {
	"IECORE_NAME" : "IECoreImage",
}

imageEnvAppends = {

	"CXXFLAGS" : [
		"-isystem", "$OIIO_INCLUDE_PATH",
	],
	"LIBPATH" : [
		"$OIIO_LIB_PATH",
	],
	"LIBS" : [
		"boost_signals" + env["BOOST_LIB_SUFFIX"],
	],
}

imageEnv = coreEnv.Clone( **imageEnvSets )
imageEnv.Append( **imageEnvAppends )
imageEnv.Append( CXXFLAGS="-DIECoreImage_EXPORTS" )

if doConfigure :

	c = Configure( imageEnv )

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
		imageEnv.AddPostAction( imageLibraryInstall, lambda target, source, env : makeLibSymLinks( imageEnv ) )
		imageEnv.Alias( "install", imageLibraryInstall )
		imageEnv.Alias( "installImage", imageLibraryInstall )
		imageEnv.Alias( "installLib", [ imageLibraryInstall ] )

		# headers
		imageHeaderInstall = imageEnv.Install( "$INSTALL_HEADER_DIR/IECoreImage", imageHeaders )
		imageEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreImage", lambda target, source, env : makeSymLinks( imageEnv, imageEnv["INSTALL_HEADER_DIR"] ) )
		imageEnv.Alias( "install", imageHeaderInstall )
		imageEnv.Alias( "installImage", imageHeaderInstall )

		# python headers
		imagePythonHeaderInstall = imageEnv.Install( "$INSTALL_HEADER_DIR/IECoreImageBindings", imagePythonHeaders )
		imageEnv.Alias( "install", imagePythonHeaderInstall )
		imageEnv.Alias( "installImage", imagePythonHeaderInstall )

		# python module
		imagePythonModuleEnv = corePythonModuleEnv.Clone( **imageEnvSets )
		imagePythonModuleEnv.Append( **imageEnvAppends )
		imagePythonModuleEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( imageEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ),
			]
		)
		imagePythonModule = imagePythonModuleEnv.SharedLibrary( "python/IECoreImage/_IECoreImage", imagePythonSources + imagePythonModuleSources )
		imagePythonModuleEnv.Depends( imagePythonModule, imageLibrary )

		# python module install
		imagePythonModuleInstall = imagePythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreImage", imagePythonScripts + imagePythonModule )
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
		imageTestEnv["ENV"]["PYTHONPATH"] = imageTestEnv["ENV"]["PYTHONPATH"] + ":python"

		imageTest = imageTestEnv.Command( "test/IECoreImage/results.txt", imagePythonModule, pythonExecutable + " $TEST_IMAGE_SCRIPT --verbose" )
		NoCache( imageTest )
		imageTestEnv.Depends( imageTest, [ corePythonModule + imagePythonModule ]  )
		imageTestEnv.Depends( imageTest, glob.glob( "test/IECoreImage/*.py" ) )
		imageTestEnv.Alias( "testImage", imageTest )

###########################################################################################
# Build, install and test the scene library and bindings
###########################################################################################

sceneEnv = coreEnv.Clone( IECORE_NAME="IECoreScene" )
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
	sceneEnv.AddPostAction( sceneLibraryInstall, lambda target, source, env : makeLibSymLinks( sceneEnv ) )
	sceneEnv.Alias( "install", [ sceneLibraryInstall ] )
	sceneEnv.Alias( "installScene", [ sceneLibraryInstall ] )
	sceneEnv.Alias( "installSceneLib", [ sceneLibraryInstall ] )

	# headers
	sceneHeaderInstall = sceneEnv.Install( "$INSTALL_HEADER_DIR/IECoreScene", sceneHeaders )
	sceneEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreScene", lambda target, source, env : makeSymLinks( sceneEnv, sceneEnv["INSTALL_HEADER_DIR"] ) )
	sceneEnv.Alias( "install", sceneHeaderInstall )
	sceneEnv.Alias( "installScene", sceneHeaderInstall )

	# python module
	scenePythonModuleEnv.Append( LIBS = os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ) )
	scenePythonModule = scenePythonModuleEnv.SharedLibrary( "python/IECoreScene/_IECoreScene", scenePythonModuleSources )
	scenePythonModuleEnv.Depends( scenePythonModule, coreLibrary )
	scenePythonModuleEnv.Depends( scenePythonModule, corePythonLibrary )

	scenePythonModuleInstall = scenePythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreScene", scenePythonScripts + scenePythonModule )
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
	sceneTest = coreTestEnv.Command( "test/IECoreScene/results.txt", scenePythonModule, pythonExecutable + " $TEST_SCENE_SCRIPT" )
	NoCache( sceneTest )
	sceneTestEnv.Alias( "testScene", sceneTest )


###########################################################################################
# Build, install and test the VDB library and bindings
###########################################################################################

vdbEnv = coreEnv.Clone( IECORE_NAME="IECoreVDB" )

vdbEnvAppends = {
	"CXXFLAGS" : [
		"-isystem", "$VDB_INCLUDE_PATH",
	],
	"LIBPATH" : [
		"$VDB_LIB_PATH",
	],
	"LIBS" : ["openvdb$VDB_LIB_SUFFIX"]
}

vdbEnv.Append( **vdbEnvAppends )

vdbPythonModuleEnv = corePythonModuleEnv.Clone( IECORE_NAME="IECoreVDB" )

vdbPythonModuleEnv.Append( **vdbEnvAppends )

vdbSources = sorted( glob.glob( "src/IECoreVDB/*.cpp" ) )
vdbHeaders = glob.glob( "include/IECoreVDB/*.h" ) + glob.glob( "include/IECoreVDB/*.inl" )
vdbPythonModuleSources = sorted( glob.glob( "src/IECoreVDB/bindings/*.cpp" ) )
vdbPythonScripts = glob.glob( "python/IECoreVDB/*.py" )

if doConfigure :

	# Since we only build shared libraries and not exectuables,
	# we only need to check that shared libs will link correctly.
	# This is necessary when building against a VDB that links to
	# extra optional dependencies not required by Cortex (eg a VDB
	# lib that ships with a DCC). This approach succeeds because
	# building a shared library doesn't require resolving the
	# unresolved symbols of the libraries that it links to.
	vdbCheckEnv = vdbEnv.Clone()
	vdbCheckEnv.Append( CXXFLAGS = [ "-fPIC" ] )
	vdbCheckEnv.Append( LINKFLAGS = [ "-shared" ] )
	c = Configure( vdbCheckEnv )

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
		vdbEnv.AddPostAction( vdbLibraryInstall, lambda target, source, env : makeLibSymLinks( vdbEnv ) )
		vdbEnv.Alias( "install", [ vdbLibraryInstall ] )
		vdbEnv.Alias( "installVDB", [ vdbLibraryInstall ] )
		vdbEnv.Alias( "installVDBLib", [ vdbLibraryInstall ] )

		# headers
		vdbHeaderInstall = sceneEnv.Install( "$INSTALL_HEADER_DIR/IECoreVDB", vdbHeaders )
		sceneEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreVDB", lambda target, source, env : makeSymLinks( vdbEnv, vdbEnv["INSTALL_HEADER_DIR"] ) )
		sceneEnv.Alias( "install", vdbHeaderInstall )
		sceneEnv.Alias( "installScene", vdbHeaderInstall )

		# python module
		vdbPythonModuleEnv.Append(
			LIBS = [
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( vdbEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)
		vdbPythonModule = vdbPythonModuleEnv.SharedLibrary( "python/IECoreVDB/_IECoreVDB", vdbPythonModuleSources )
		vdbPythonModuleEnv.Depends( vdbPythonModule, coreLibrary )
		vdbPythonModuleEnv.Depends( vdbPythonModule, corePythonLibrary )

		vdbPythonModuleInstall = vdbPythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreVDB", vdbPythonScripts + vdbPythonModule )
		vdbPythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreVDB", lambda target, source, env : makeSymLinks( vdbPythonModuleEnv, vdbPythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
		vdbPythonModuleEnv.Alias( "install", vdbPythonModuleInstall )
		vdbPythonModuleEnv.Alias( "installScene", vdbPythonModuleInstall )

		Default( vdbLibrary, vdbPythonModule )

		# testing
		vdbTestEnv = testEnv.Clone()

		vdbTestEnv["ENV"]["PYTHONPATH"] = vdbTestEnv["ENV"]["PYTHONPATH"] + ":" + vdbTestEnv["VDB_PYTHON_PATH"]

		vdbTest = vdbTestEnv.Command( "test/IECoreVDB/results.txt", vdbPythonModule, pythonExecutable + " $TEST_VDB_SCRIPT" )
		NoCache( vdbTest )
		vdbTestEnv.Alias( "testVDB", vdbTest )

###########################################################################################
# Build and install the renderman display driver
###########################################################################################

if doConfigure :

	riDisplayDriverEnv = coreEnv.Clone( IECORE_NAME = "ieDisplay", SHLIBPREFIX="" )
	riDisplayDriverEnv.Append( CXXFLAGS = [ "-isystem", "$RMAN_ROOT/include" ] )

	c = Configure( riDisplayDriverEnv )
	if not c.CheckCXXHeader( "ndspy.h" ) :

		sys.stderr.write( "WARNING : ndspy.h not found - check RMAN_ROOT.\n" )
		c.Finish()

	else :

		c.Finish()

		# we can't append this before configuring, as then it gets built as
		# part of the configure process
		riDisplayDriverEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( imageEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)

		riDisplayDriver = riDisplayDriverEnv.SharedLibrary( "src/rmanDisplays/ieDisplay/" + os.path.basename( riDisplayDriverEnv.subst( "$INSTALL_RMANDISPLAY_NAME" ) ), "src/rmanDisplays/ieDisplay/IEDisplay.cpp" )
		riDisplayDriverInstall = riDisplayDriverEnv.Install( os.path.dirname( riDisplayDriverEnv.subst( "$INSTALL_RMANDISPLAY_NAME" ) ), riDisplayDriver )
		riDisplayDriverEnv.NoCache( riDisplayDriverInstall )
		riDisplayDriverEnv.AddPostAction( riDisplayDriverInstall, lambda target, source, env : makeLibSymLinks( riDisplayDriverEnv, libNameVar="INSTALL_RMANDISPLAY_NAME" ) )
		riDisplayDriverEnv.Alias( "install", riDisplayDriverInstall )
		riDisplayDriverEnv.Alias( "installRI", riDisplayDriverInstall )

		Default( [ riDisplayDriver ] )

###########################################################################################
# Build, install and test the optional CoreGL library and bindings
###########################################################################################

if env["WITH_GL"] and doConfigure :

	glEnvSets = {
		"IECORE_NAME" : "IECoreGL",
	}

	glEnvAppends = {

		"CXXFLAGS" : [
			"-isystem", "$GLEW_INCLUDE_PATH",
			# These are to work around warnings in boost::wave
			# while still using -Werror.
			"-Wno-format",
			"-Wno-strict-aliasing",
		],
		"LIBPATH" : [
			"$GLEW_LIB_PATH",
		],
	}

	glEnv = coreEnv.Clone( **glEnvSets )
	glEnv.Append( **glEnvAppends )
	glEnv.Append( CXXFLAGS = "-DIECoreGL_EXPORTS")

	c = Configure( glEnv )

	if not c.CheckLibWithHeader( env.subst( "GLEW$GLEW_LIB_SUFFIX" ), "GL/glew.h", "CXX" ) :

		sys.stderr.write( "WARNING : GLEW library not found, not building IECoreGL - check GLEW_INCLUDE_PATH and GLEW_LIB_PATH.\n" )
		c.Finish()

	else :

		c.Finish()

		# we can't add this earlier as then it's built during the configure stage, and that's no good
		glEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( imageEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)

		if env["PLATFORM"]=="darwin" :
			glEnv.Append(
				FRAMEWORKS = [
					"OpenGL",
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

		glPythonModuleEnv = corePythonModuleEnv.Clone( **glEnvSets )
		glPythonModuleEnv.Append( **glEnvAppends )
		glPythonModuleEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ),
				os.path.basename( imageEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
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
		glTestEnv["ENV"]["IECOREGL_SHADER_INCLUDE_PATHS"] = "./glsl"
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
	"CXXFLAGS" : [
		"-isystem", "$GLEW_INCLUDE_PATH",
		"-DIECoreMaya_EXPORTS",
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
	mayaEnvAppends["CXXFLAGS"] += [ "-isystem", "$MAYA_ROOT/include" ]
	mayaEnvAppends["LIBS"]  += ["OpenMayalib"]

elif env["PLATFORM"]=="darwin" :
	mayaEnvAppends["CPPFLAGS"]  += ["-DOSMac_","-DOSMac_MachO_"]
	mayaEnvAppends["LIBPATH"] = ["$MAYA_ROOT/MacOS"]
	mayaEnvAppends["CPPPATH"] = ["$MAYA_ROOT/../../devkit/include"]
	mayaEnvAppends["LIBS"] += ["Foundation", "OpenMayaRender"]
	mayaEnvAppends["FRAMEWORKS"] = ["AGL", "OpenGL"]

mayaEnv = env.Clone( **mayaEnvSets )
mayaEnv.Append( **mayaEnvAppends )

mayaEnv.Append( SHLINKFLAGS = pythonEnv["PYTHON_LINK_FLAGS"].split() )

mayaPythonModuleEnv = pythonModuleEnv.Clone( **mayaEnvSets )
mayaPythonModuleEnv.Append( **mayaEnvAppends )

mayaPluginEnv = mayaEnv.Clone( IECORE_NAME="ieCore" )

haveMaya = False

if doConfigure :

	c = Configure( mayaEnv )

	if not c.CheckCXXHeader( "maya/MVectorArray.h" ) :

		sys.stderr.write( "WARNING : no maya devkit found, not building IECoreMaya - check MAYA_ROOT.\n" )
		c.Finish()

	else :

		c.Finish()

		haveMaya = True

		mayaSources = sorted( glob.glob( "src/IECoreMaya/*.cpp" ) )
		mayaHeaders = glob.glob( "include/IECoreMaya/bindings/*.h" ) + glob.glob( "include/IECoreMaya/*.h" ) + glob.glob( "include/IECoreMaya/*.inl" )
		mayaBindingHeaders = glob.glob( "include/IECoreMaya/bindings/*.h" ) + glob.glob( "include/IECoreMaya/bindings/*.inl" )
		mayaPythonSources = sorted( glob.glob( "src/IECoreMaya/bindings/*.cpp" ) )
		mayaPythonScripts = glob.glob( "python/IECoreMaya/*.py" )
		mayaMel = glob.glob( "mel/IECoreMaya/*.mel" )
		mayaPluginSources = [ "src/IECoreMaya/plugin/Plugin.cpp" ]

		# we can't append this before configuring, as then it gets built as
		# part of the configure process
		mayaEnv.Append( LIBS = os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ) )
		mayaEnv.Append( LIBS = os.path.basename( imageEnv.subst( "$INSTALL_LIB_NAME" ) ) )
		mayaEnv.Append( LIBS = os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ) )
		mayaEnv.Append( LIBS = os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ) )
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
		mayaIcons = glob.glob( "icons/IECoreMaya/*.xpm" ) + glob.glob( "icons/IECoreMaya/*.png" )
		mayaIconInstall = mayaEnv.Install( "$INSTALL_MAYAICON_DIR", source=mayaIcons )
		mayaEnv.Alias( "install", mayaIconInstall )
		mayaEnv.Alias( "installMaya", mayaIconInstall )

		# maya plugin
		mayaPluginEnv.Append(
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

		mayaTestLibPaths = mayaEnv.subst( ":".join( mayaPythonModuleEnv["LIBPATH"] ) )
		mayaTestEnv["ENV"][mayaTestEnv["TEST_LIBRARY_PATH_ENV_VAR"]] += ":" + mayaTestLibPaths
		mayaTestEnv["ENV"][libraryPathEnvVar] += ":" + mayaTestLibPaths

		mayaTestEnv["ENV"]["PATH"] = mayaEnv.subst( "$MAYA_ROOT/bin:" ) + mayaEnv["ENV"]["PATH"]
		mayaTestEnv["ENV"]["MAYA_PLUG_IN_PATH"] = "./plugins/maya:./test/IECoreMaya/plugins"
		mayaTestEnv["ENV"]["MAYA_SCRIPT_PATH"] = "./mel"
		mayaTestEnv["ENV"]["PYTHONHOME"] = mayaTestEnv.subst( "$MAYA_ROOT" )
		mayaTestEnv["ENV"]["MAYA_LOCATION"] = mayaTestEnv.subst( "$MAYA_ROOT" )
		mayaTestEnv["ENV"]["LM_LICENSE_FILE"] = env["MAYA_LICENSE_FILE"]
		mayaTestEnv["ENV"]["AUTODESK_ADLM_THINCLIENT_ENV"] = env["MAYA_ADLM_ENV_FILE"]

		mayaPythonTestEnv = mayaTestEnv.Clone()

		mayaTestEnv.Append( **mayaEnvAppends )
		mayaTestEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( mayaEnv.subst( "$INSTALL_LIB_NAME" ) ),
				"OpenMayalib"
			]
		)

		mayaPythonTest = mayaPythonTestEnv.Command( "test/IECoreMaya/resultsPython.txt", mayaPythonModule, "mayapy $TEST_MAYA_SCRIPT" )
		NoCache( mayaPythonTest )
		mayaPythonTestEnv.Depends( mayaPythonTest, [ mayaPlugin, mayaPythonModule, mayaLibrary ] )
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

	"CXXFLAGS" : [
		"-isystem", "$NUKE_ROOT/include",
		"-isystem", "$GLEW_INCLUDE_PATH",
	],

	"CPPFLAGS" : [
		pythonEnv["PYTHON_INCLUDE_FLAGS"],
	],

	"LINKFLAGS" : [
		"-Wl,-rpath-link=$NUKE_ROOT",
	],

	"LIBPATH" : [
		"$NUKE_ROOT",
	],

	"LIBS" : [
		"GLEW$GLEW_LIB_SUFFIX",
	]

}

if env["PLATFORM"] == "darwin" :
	# FN_OS_MAC is required to work around isnan errors in DDImage/Matrix4.h
	nukeEnvAppends["CPPFLAGS"].append( "-DFN_OS_MAC" )

nukeEnv = env.Clone( IECORE_NAME = "IECoreNuke" )
nukeEnv.Append( **nukeEnvAppends )
nukeEnv.Append( SHLINKFLAGS = pythonEnv["PYTHON_LINK_FLAGS"].split() )
if env["PLATFORM"] == "darwin" :
	nukeEnv.Append( FRAMEWORKS = [ "OpenGL" ] )

nukePythonModuleEnv = pythonModuleEnv.Clone( IECORE_NAME = "IECoreNuke" )
nukePythonModuleEnv.Append( **nukeEnvAppends )

nukePluginEnv = nukeEnv.Clone( IECORE_NAME="ieCore" )

nukeTestEnv = testEnv.Clone()
nukeTestEnv["ENV"]["LM_LICENSE_FILE"] = nukeTestEnv["NUKE_LICENSE_FILE"]
nukeTestEnv["ENV"]["foundry_LICENSE"] = nukeTestEnv["NUKE_LICENSE_FILE"]
nukeTestEnv["ENV"]["NUKE_PATH"] = "plugins/nuke"
nukeTestEnv["ENV"]["IECORE_OP_PATHS"] = "test/IECoreNuke/ops:test/IECore/ops"

if doConfigure :

	c = Configure( nukeEnv )

	if not c.CheckHeader( "DDImage/Vector3.h", "\"\"", "CXX" ) :

		sys.stderr.write( "WARNING : no nuke devkit found, not building IECoreNuke - check NUKE_ROOT.\n" )
		c.Finish()

	else :

		# figure out the nuke version from the headers
		nukeMajorVersion = None
		nukeMinorVersion = None
		nukeVersionHeader = env.FindFile( "DDImage/ddImageVersionNumbers.h", nukeEnv["CXXFLAGS"] )
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
					os.path.basename( imageEnv.subst( "$INSTALL_LIB_NAME" ) ),
					os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ),
					os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ),
				]	)

				nukeEnv.Append( LIBS = [ nukeLibName, "boost_python$BOOST_LIB_SUFFIX" ] )

				nukeEnv.Append(
					CPPFLAGS = [
						"-DIECORENUKE_NUKE_MAJOR_VERSION=$NUKE_MAJOR_VERSION",
						"-DIECORENUKE_NUKE_MINOR_VERSION=$NUKE_MINOR_VERSION",
						"-DIECoreNuke_EXPORTS",
					]
				)

				nukePythonModuleEnv.Append( LIBS = [
					os.path.basename( nukeEnv.subst( "$INSTALL_LIB_NAME" ) ),
					os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
					os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ),
				] )

				nukeHeaders = glob.glob( "include/IECoreNuke/*.h" ) + glob.glob( "include/IECoreNuke/*.inl" )
				nukeSources = sorted( glob.glob( "src/IECoreNuke/*.cpp" ) )
				nukePythonSources = sorted( glob.glob( "src/IECoreNuke/bindings/*.cpp" ) )
				nukePythonScripts = glob.glob( "python/IECoreNuke/*.py" )
				nukePluginSources = sorted( glob.glob( "src/IECoreNuke/plugin/*.cpp" ) )
				nukeNodeNames = [ "ieObject", "ieOp", "ieDrawable", "ieDisplay" ]

				# nuke library
				nukeEnv.Append( LIBS = [ "boost_signals" + env["BOOST_LIB_SUFFIX"] ] )
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
				nukePythonModuleEnv.Depends( nukePythonModule, corePythonModule )

				if coreEnv["INSTALL_CORENUKE_POST_COMMAND"]!="" :
					# this is the only way we could find to get a post action to run for an alias
					nukeEnv.Alias( "install", nukeLibraryInstall, "$INSTALL_CORENUKE_POST_COMMAND" )
					nukeEnv.Alias( "installNuke", nukeLibraryInstall, "$INSTALL_CORENUKE_POST_COMMAND" )

				# nuke plugin

				nukePluginEnv.Append(
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

				Default( [ nukeLibrary, nukePythonModule, nukePlugin ] )

				# nuke menu

				nukeMenuInstall = nukePluginEnv.Install( os.path.dirname( nukePluginEnv.subst( "$INSTALL_NUKEPLUGIN_NAME" ) ), "src/IECoreNuke/plugin/menu.py" )
				nukePluginEnv.Alias( "install", nukeMenuInstall )

				# stubs for each of the nodes within the plugin

				nukeStubs = []
				for nodeName in nukeNodeNames :

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
}

houdiniEnvAppends = {
	"CXXFLAGS" : [
		"-isystem", "$GLEW_INCLUDE_PATH",
		"-isystem", "$HOUDINI_INCLUDE_PATH",
		"$HOUDINI_CXX_FLAGS",
		"-DMAKING_DSO",
		"-DIECoreHoudini_EXPORTS",
	],
	"CPPFLAGS" : [
		## \todo: libIECoreHoudini should not use python.
		## Remove it from the src and then remove these flags.
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
		"HoudiniRAY",
		"HoudiniAPPS3",
		## \todo: libIECoreHoudini should not use python.
		## Remove it from the src and then remove this lib.
		"boost_python" + env["BOOST_LIB_SUFFIX"],
		"GLEW$GLEW_LIB_SUFFIX"
	]
}

if env["WITH_GL"] :
	houdiniEnvAppends["CXXFLAGS"].extend( [ "-isystem", "$GLEW_INCLUDE_PATH" ] )
	houdiniEnvAppends["LIBPATH"].append( "$GLEW_LIB_PATH" )
	houdiniEnvAppends["LIBS"].append( "GLEW$GLEW_LIB_SUFFIX" )

if env["PLATFORM"]=="posix" :
	houdiniEnvAppends["CPPFLAGS"] += ["-DLINUX"]
elif env["PLATFORM"]=="darwin" :
	houdiniEnvAppends["CPPFLAGS"] += ["-D__APPLE__"]
	houdiniEnvAppends["FRAMEWORKS"] = ["OpenGL"]
	houdiniEnvAppends["LIBS"] += [ "GR"]

houdiniEnv = env.Clone( **houdiniEnvSets )
houdiniEnv.Append( **houdiniEnvAppends )

houdiniEnv.Append( SHLINKFLAGS = pythonEnv["PYTHON_LINK_FLAGS"].split() )
houdiniEnv.Prepend( SHLINKFLAGS = "$HOUDINI_LINK_FLAGS" )

houdiniPythonModuleEnv = pythonModuleEnv.Clone( **houdiniEnvSets )
houdiniPythonModuleEnv.Append( **houdiniEnvAppends )
if env["PLATFORM"] == "posix" :
	## We really want to not have the -Wno-strict-aliasing flag, but it's necessary to stop boost
	# python warnings that don't seem to be prevented by including boost via -isystem even. Better to
	# be able to have -Werror but be missing one warning than to have no -Werror.
	## \todo This is probably only necessary for specific gcc versions where -isystem doesn't
	# fully work. Reenable when we encounter versions that work correctly.
	houdiniPythonModuleEnv.Append( CXXFLAGS = [ "-Wno-strict-aliasing" ] )

houdiniPluginEnv = houdiniEnv.Clone( IECORE_NAME="ieCoreHoudini" )

if doConfigure :

	# Since we only build shared libraries and not exectuables,
	# we only need to check that shared libs will link correctly.
	# This approach succeeds because building a shared library
	# doesn't require resolving the unresolved symbols of the
	# libraries that it links to.
	houdiniCheckEnv = houdiniEnv.Clone()
	houdiniCheckEnv.Append( CXXFLAGS = [ "-fPIC" ] )
	houdiniCheckEnv.Append( LINKFLAGS = [ "-shared" ] )
	c = Configure( houdiniCheckEnv )

	if not c.CheckLibWithHeader( "HoudiniGEO", "SOP/SOP_API.h", "CXX" ) :

		sys.stderr.write( "WARNING : no houdini devkit found, not building IECoreHoudini - check HOUDINI_ROOT.\n" )
		c.Finish()

	else :

		# Houdini 16.0 and beyond can optionally ship using Qt5.
		# Since IECoreHoudini makes some UI related calls, we add
		# a custom define so we can change the logic as needed.
		if os.path.exists( os.path.join( houdiniCheckEnv.subst( "$HOUDINI_LIB_PATH" ), "libQt5Core.so" ) ) :
			houdiniPythonModuleEnv.Append( CXXFLAGS = "-DIECOREHOUDINI_WITH_QT5" )

		c.Finish()

		#=====
		# glob the files
		#=====
		houdiniSources = sorted( glob.glob( "src/IECoreHoudini/*.cpp" ) )
		houdiniHeaders = glob.glob( "include/IECoreHoudini/*.h" ) + glob.glob( "include/IECoreHoudini/*.inl" )
		houdiniBindingHeaders = glob.glob( "include/IECoreHoudini/bindings/*.h" ) + glob.glob( "include/IECoreHoudini/bindings/*.inl" )
		houdiniPythonSources = sorted( glob.glob( "src/IECoreHoudini/bindings/*.cpp" ) )
		houdiniPythonScripts = glob.glob( "python/IECoreHoudini/*.py" )
		houdiniPluginSources = [ "src/IECoreHoudini/plugin/Plugin.cpp" ]
		if not env['WITH_GL'] :
			houdiniSources.remove( "src/IECoreHoudini/GR_CortexPrimitive.cpp" )
			houdiniSources.remove( "src/IECoreHoudini/GUI_CortexPrimitiveHook.cpp" )
		else:
		    houdiniEnv.Append( CPPFLAGS = '-DIECOREHOUDINI_WITH_GL' )

		# we can't append this before configuring, as then it gets built as
		# part of the configure process
		houdiniEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ),
			]
		)
		if env['WITH_GL'] :
			houdiniEnv.Append( LIBS = os.path.basename( glEnv.subst( "$INSTALL_LIB_NAME" ) ) )

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
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( houdiniEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ),
			]
		)
		houdiniPythonModule = houdiniPythonModuleEnv.SharedLibrary( "python/IECoreHoudini/_IECoreHoudini", houdiniPythonSources )
		houdiniPythonModuleEnv.Depends( houdiniPythonModule, houdiniLib )
		houdiniPythonModuleInstall = houdiniPythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreHoudini", houdiniPythonScripts + houdiniPythonModule )
		houdiniPythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreHoudini", lambda target, source, env : makeSymLinks( houdiniPythonModuleEnv, houdiniPythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
		houdiniPythonModuleEnv.Alias( "install", houdiniPythonModuleInstall )
		houdiniPythonModuleEnv.Alias( "installHoudini", houdiniPythonModuleInstall )

		#=====
		# install icons
		#=====
		houdiniIcons = glob.glob( "icons/IECoreHoudini/*.svg" ) + glob.glob( "graphics/CortexLogo*.svg" )
		houdiniIconInstall = houdiniPluginEnv.Install( "$INSTALL_HOUDINIICON_DIR", source=houdiniIcons )
		houdiniPluginEnv.Alias( "install", houdiniIconInstall )
		houdiniPluginEnv.Alias( "installHoudini", houdiniIconInstall )

		#=====
		# install toolbar
		#=====
		houdiniToolbars = glob.glob( "menus/IECoreHoudini/*.shelf" )
		houdiniToolbarInstall = houdiniPluginEnv.Install( "$INSTALL_HOUDINITOOLBAR_DIR", source=houdiniToolbars )
		houdiniPluginEnv.Alias( "install", houdiniToolbarInstall )
		houdiniPluginEnv.Alias( "installHoudini", houdiniToolbarInstall )

		if coreEnv["INSTALL_COREHOUDINI_POST_COMMAND"] != "" :
			# this is the only way we could find to get a post action to run for an alias
			houdiniPythonModuleEnv.Alias( "install", houdiniPythonModuleInstall, "$INSTALL_COREHOUDINI_POST_COMMAND" )
			houdiniPythonModuleEnv.Alias( "installHoudini", houdiniPythonModuleInstall, "$INSTALL_COREHOUDINI_POST_COMMAND" )

		Default( [ houdiniLib, houdiniPlugin, houdiniPythonModule ] )

		#=====
		# Houdini tests
		#=====
		houdiniTestEnv = testEnv.Clone()

		houdiniTestLibPaths = houdiniEnv.subst( ":".join( houdiniPythonModuleEnv["LIBPATH"] ) )
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

		houdiniTestEnv["ENV"]["PYTHONPATH"] += ":./python"
		houdiniTestEnv["ENV"]["HOUDINI_DSO_PATH"] = "./plugins/houdini:&"
		houdiniTestEnv["ENV"]["HOUDINI_OTLSCAN_PATH"] = "./plugins/houdini:&"

		houdiniTestEnv["ENV"]["IECORE_OP_PATHS"] = "./test/IECoreHoudini/ops"

		houdiniPythonExecutable = "hython"

		houdiniPythonTest = houdiniTestEnv.Command( "test/IECoreHoudini/resultsPython.txt", houdiniPythonModule, houdiniPythonExecutable + " $TEST_HOUDINI_SCRIPT" )
		NoCache( houdiniPythonTest )
		houdiniTestEnv.Depends( houdiniPythonTest, [ houdiniLib, houdiniPlugin, houdiniPythonModule ] )
		houdiniTestEnv.Depends( houdiniPythonTest, glob.glob( "test/IECoreHoudini/*.py" ) )
		houdiniTestEnv.Depends( houdiniPythonTest, glob.glob( "python/IECoreHoudini/*.py" ) )
		if env["WITH_GL"] :
			houdiniTestEnv.Depends( houdiniPythonTest, [ glLibrary, glPythonModule ] )
		houdiniTestEnv.Alias( "testHoudini", houdiniPythonTest )
		houdiniTestEnv.Alias( "testHoudiniPython", houdiniPythonTest )

###########################################################################################
# Build, install and test the IECoreArnold library and bindings
###########################################################################################

arnoldEnv = coreEnv.Clone( IECORE_NAME = "IECoreArnold" )
arnoldEnv.Append(
	CXXFLAGS = [
		"-isystem", "$ARNOLD_ROOT/include",
		"-DIECoreArnold_EXPORTS",
	],
	CPPPATH = [
		"contrib/IECoreArnold/include",
	]
)
arnoldEnv.Append( LIBPATH = [ "$ARNOLD_ROOT/bin" ] )

arnoldPythonModuleEnv = pythonModuleEnv.Clone( IECORE_NAME = "IECoreArnold" )
arnoldPythonModuleEnv.Append(
	CXXFLAGS = [
		"-isystem", "$ARNOLD_ROOT/include",
	],
	CPPPATH = [
		"contrib/IECoreArnold/include",
		"contrib/IECoreArnold/include/bindings",
	]
)
arnoldPythonModuleEnv.Append( LIBPATH = [ "$ARNOLD_ROOT/bin" ] )

arnoldDriverEnv = arnoldEnv.Clone( IECORE_NAME = "ieOutputDriver" )
arnoldDriverEnv["SHLIBPREFIX"] = ""
arnoldDriverEnv["SHLIBSUFFIX"] = ".so"

haveArnold = False

if doConfigure :

	# Since we only build shared libraries and not exectuables,
	# we only need to check that shared libs will link correctly.
	# This is necessary for arnold, which uses
	# a run-time compatible, but link-time incompatbile libstdc++
	# in some obscure studio setups. This approach succeeds because
	# building a shared library doesn't require resolving the
	# unresolved symbols of the libraries that it links to.
	arnoldCheckEnv = arnoldEnv.Clone()
	arnoldCheckEnv.Append( CXXFLAGS = [ "-fPIC" ] )
	arnoldCheckEnv.Append( LINKFLAGS = [ "-shared" ] )
	c = Configure( arnoldCheckEnv )

	if not c.CheckLibWithHeader( "ai", "ai.h", "CXX" ) :

		sys.stderr.write( "WARNING : no ai library found, not building IECoreArnold - check ARNOLD_ROOT.\n" )
		c.Finish()

	else :

		haveArnold = True

		arnoldSources = sorted( glob.glob( "contrib/IECoreArnold/src/IECoreArnold/*.cpp" ) )
		arnoldHeaders = glob.glob( "contrib/IECoreArnold/include/IECoreArnold/*.h" ) + glob.glob( "contrib/IECoreArnold/include/IECoreArnold/*.inl" )
		arnoldPythonSources = sorted( glob.glob( "contrib/IECoreArnold/src/IECoreArnold/bindings/*.cpp" ) )
		arnoldPythonScripts = glob.glob( "contrib/IECoreArnold/python/IECoreArnold/*.py" )

		c.Finish()

		# we can't append this before configuring, as then it gets built as
		# part of the configure process
		arnoldEnv.Append(
			LIBS = [
				"ai",
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		 )
		arnoldPythonModuleEnv.Append( LIBS = os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ) )

		arnoldDriverEnv.Append(
			LIBS = [
				"ai",
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( imageEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( arnoldEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)

		# library
		arnoldLibrary = arnoldEnv.SharedLibrary( "lib/" + os.path.basename( arnoldEnv.subst( "$INSTALL_ARNOLDLIB_NAME" ) ), arnoldSources )
		arnoldLibraryInstall = arnoldEnv.Install( os.path.dirname( arnoldEnv.subst( "$INSTALL_ARNOLDLIB_NAME" ) ), arnoldLibrary )
		arnoldEnv.NoCache( arnoldLibraryInstall )
		arnoldEnv.AddPostAction( arnoldLibraryInstall, lambda target, source, env : makeLibSymLinks( arnoldEnv ) )
		arnoldEnv.Alias( "install", arnoldLibraryInstall )
		arnoldEnv.Alias( "installArnold", arnoldLibraryInstall )
		arnoldEnv.Alias( "installLib", [ arnoldLibraryInstall ] )

		# headers
		arnoldHeaderInstall = arnoldEnv.Install( "$INSTALL_HEADER_DIR/IECoreArnold", arnoldHeaders )
		arnoldEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreArnold", lambda target, source, env : makeSymLinks( arnoldEnv, arnoldEnv["INSTALL_HEADER_DIR"] ) )
		arnoldEnv.Alias( "install", arnoldHeaderInstall )
		arnoldEnv.Alias( "installArnold", arnoldHeaderInstall )

		# python module
		arnoldPythonModuleEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( arnoldEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)
		arnoldPythonModule = arnoldPythonModuleEnv.SharedLibrary( "contrib/IECoreArnold/python/IECoreArnold/_IECoreArnold", arnoldPythonSources )
		arnoldPythonModuleEnv.Depends( arnoldPythonModule, arnoldLibrary )

		arnoldPythonModuleInstall = arnoldPythonModuleEnv.Install( "$INSTALL_ARNOLDPYTHON_DIR/IECoreArnold", arnoldPythonScripts + arnoldPythonModule )
		arnoldPythonModuleEnv.AddPostAction( "$INSTALL_ARNOLDPYTHON_DIR/IECoreArnold", lambda target, source, env : makeSymLinks( arnoldPythonModuleEnv, arnoldPythonModuleEnv["INSTALL_ARNOLDPYTHON_DIR"] ) )
		arnoldPythonModuleEnv.Alias( "install", arnoldPythonModuleInstall )
		arnoldPythonModuleEnv.Alias( "installArnold", arnoldPythonModuleInstall )

		# output driver
		arnoldDriver = arnoldDriverEnv.SharedLibrary( "contrib/IECoreArnold/src/IECoreArnold/outputDriver/" + os.path.basename( arnoldDriverEnv.subst( "$INSTALL_ARNOLDOUTPUTDRIVER_NAME" ) ), "contrib/IECoreArnold/src/IECoreArnold/outputDriver/OutputDriver.cpp" )
		arnoldDriverInstall = arnoldDriverEnv.Install( os.path.dirname( arnoldDriverEnv.subst( "$INSTALL_ARNOLDOUTPUTDRIVER_NAME" ) ), arnoldDriver )
		arnoldDriverEnv.NoCache( arnoldDriverInstall )
		arnoldDriverEnv.AddPostAction( arnoldDriverInstall, lambda target, source, env : makeLibSymLinks( arnoldDriverEnv, libNameVar="INSTALL_ARNOLDOUTPUTDRIVER_NAME" ) )
		arnoldDriverEnv.Alias( "install", arnoldDriverInstall )
		arnoldDriverEnv.Alias( "installArnold", arnoldDriverInstall )
		arnoldDriverForTest = arnoldDriverEnv.Command( "contrib/IECoreArnold/test/IECoreArnold/plugins/ieOutputDriver.so", arnoldDriver, Copy( "$TARGET", "$SOURCE" ) )

		Default( [ arnoldLibrary, arnoldPythonModule, arnoldDriver, arnoldDriverForTest ] )

		# tests
		arnoldTestEnv = testEnv.Clone()
		arnoldTestEnv["ENV"]["PYTHONPATH"] += ":./contrib/IECoreArnold/python:" + arnoldEnv.subst( "$ARNOLD_ROOT/python" )
		arnoldTestEnv["ENV"][testEnv["TEST_LIBRARY_PATH_ENV_VAR"]] += ":" + arnoldEnv.subst( ":".join( arnoldPythonModuleEnv["LIBPATH"] ) )
		arnoldTestEnv["ENV"]["PATH"] = arnoldEnv.subst( "$ARNOLD_ROOT/bin" ) + ":" + arnoldTestEnv["ENV"]["PATH"]
		arnoldTestEnv["ENV"]["ARNOLD_PLUGIN_PATH"] = "contrib/IECoreArnold/test/IECoreArnold/plugins"
		arnoldTest = arnoldTestEnv.Command( "contrib/IECoreArnold/test/IECoreArnold/results.txt", arnoldPythonModule, pythonExecutable + " $TEST_ARNOLD_SCRIPT" )
		NoCache( arnoldTest )
		arnoldTestEnv.Depends( arnoldTest, [ arnoldPythonModule + arnoldDriverForTest + arnoldLibrary ] )
		arnoldTestEnv.Depends( arnoldTest, glob.glob( "contrib/IECoreArnold/test/IECoreArnold/*.py" ) )
		arnoldTestEnv.Alias( "testArnold", arnoldTest )

###########################################################################################
# Build, install and test the IECoreUSD library and bindings
###########################################################################################

usdEnv = pythonEnv.Clone( IECORE_NAME = "IECoreUSD" )

if usdEnv["WITH_USD_MONOLITHIC"] :
	usdLibs = [ "usd_ms" ]
else :
	usdLibs = [
		"usd",
		"usdGeom",
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
		"-isystem", "$USD_INCLUDE_PATH",
		"-DBUILD_COMPONENT_SRC_PREFIX=",
		"-DBUILD_OPTLEVEL_DEV",
		"-Wno-deprecated"
	],
	"CPPPATH" : [
		"contrib/IECoreUSD/include"
	],
	"LIBPATH" : [
		"$USD_LIB_PATH"
	],
	"LIBS" : usdLibs,
}

usdEnv.Append( **usdEnvAppends )

usdPythonModuleEnv = pythonModuleEnv.Clone( IECORE_NAME = "IECoreUSD" )
usdPythonModuleEnv.Append( **usdEnvAppends )

if doConfigure :

	# Since we only build shared libraries and not exectuables,
	# we only need to check that shared libs will link correctly.
	# This is necessary when building against a USD that links to
	# extra optional dependencies not required by Cortex (eg a USD
	# lib that ships with a DCC). This approach succeeds because
	# building a shared library doesn't require resolving the
	# unresolved symbols of the libraries that it links to.
	usdCheckEnv = usdEnv.Clone()
	usdCheckEnv.Append( CXXFLAGS = [ "-fPIC" ] )
	usdCheckEnv.Append( LINKFLAGS = [ "-shared" ] )
	c = Configure( usdCheckEnv )

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

		# we can't append this before configuring, as then it gets built as
		# part of the configure process
		usdEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)

		# library
		usdLibrary = usdEnv.SharedLibrary( "lib/" + os.path.basename( usdEnv.subst( "$INSTALL_ALEMBICLIB_NAME" ) ), usdSources )
		usdLibraryInstall = usdEnv.Install( os.path.dirname( usdEnv.subst( "$INSTALL_ALEMBICLIB_NAME" ) ), usdLibrary )
		usdEnv.NoCache( usdLibraryInstall )
		usdEnv.AddPostAction( usdLibraryInstall, lambda target, source, env : makeLibSymLinks( usdEnv ) )
		usdEnv.Alias( "install", usdLibraryInstall )
		usdEnv.Alias( "installUSD", usdLibraryInstall )
		usdEnv.Alias( "installLib", [ usdLibraryInstall ] )

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
		usdPythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreUSD", lambda target, source, env : makeSymLinks( usdPythonModuleEnv, usdPythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
		usdPythonModuleEnv.Alias( "install", usdPythonModuleInstall )
		usdPythonModuleEnv.Alias( "installUSD", usdPythonModuleInstall )

		Default( [ usdLibrary, usdPythonModule ] )

		# tests
		usdTestEnv = testEnv.Clone()
		usdTestEnv["ENV"]["PYTHONPATH"] += ":./contrib/IECoreUSD/python"

		usdLibPath = coreEnv.subst("$USD_LIB_PATH")
		usdPythonPath = os.path.join(usdLibPath, "python")

		usdTestEnv["ENV"]["PYTHONPATH"] += ":" + usdPythonPath
		usdTestEnv["ENV"][testEnv["TEST_LIBRARY_PATH_ENV_VAR"]] += ":" + usdLibPath

		usdTest = usdTestEnv.Command( "contrib/IECoreUSD/test/IECoreUSD/results.txt", usdPythonModule, pythonExecutable + " $TEST_USD_SCRIPT" )
		NoCache( usdTest )
		usdTestEnv.Alias( "testUSD", usdTest )


###########################################################################################
# Build, install and test the IECoreAlembic library and bindings
###########################################################################################

alembicEnv = coreEnv.Clone( IECORE_NAME = "IECoreAlembic" )
alembicEnvPrepends = {
	"CXXFLAGS" : [
		"-isystem", "$ALEMBIC_INCLUDE_PATH",
		"-isystem", "$HDF5_INCLUDE_PATH",
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

alembicPythonModuleEnv = pythonModuleEnv.Clone( IECORE_NAME = "IECoreAlembic" )
alembicPythonModuleEnv.Prepend( **alembicEnvPrepends )

if doConfigure :

	c = Configure( alembicEnv )

	haveAlembic = False
	if c.CheckLibWithHeader( alembicEnv.subst( "Alembic" + env["ALEMBIC_LIB_SUFFIX"] ), "Alembic/AbcGeom/Foundation.h", "CXX" ) :

		# Alembic 1.6 and later is provided as a single library
		haveAlembic = True
		alembicEnv.Prepend(
			CPPFLAGS = "-DIECOREALEMBIC_WITH_OGAWA"
		)

	elif c.CheckLibWithHeader( alembicEnv.subst( "AlembicAbcGeom" + env["ALEMBIC_LIB_SUFFIX"] ), "Alembic/AbcGeom/Foundation.h", "CXX" ) :

		# Prior to 1.6, Alembic was provided as a bunch of individual libraries.
		haveAlembic = True
		alembicEnv.Append(
			LIBS = [
				"AlembicAbc$ALEMBIC_LIB_SUFFIX",
				"AlembicAbcCoreHDF5$ALEMBIC_LIB_SUFFIX",
				"AlembicAbcCoreAbstract$ALEMBIC_LIB_SUFFIX",
				"AlembicUtil$ALEMBIC_LIB_SUFFIX",
			],
		)

		if c.CheckLibWithHeader( alembicEnv.subst( "AlembicOgawa" + env["ALEMBIC_LIB_SUFFIX"] ), "Alembic/AbcCoreOgawa/ReadWrite.h", "CXX" ) :
			alembicEnv.Prepend(
				CPPFLAGS = "-DIECOREALEMBIC_WITH_OGAWA",
				LIBS = [
					"AlembicAbcCoreFactory$ALEMBIC_LIB_SUFFIX",
					"AlembicAbcCoreOgawa$ALEMBIC_LIB_SUFFIX",
					"AlembicOgawa$ALEMBIC_LIB_SUFFIX",
				]
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
		alembicEnv.AddPostAction( alembicLibraryInstall, lambda target, source, env : makeLibSymLinks( alembicEnv ) )
		alembicEnv.Alias( "install", alembicLibraryInstall )
		alembicEnv.Alias( "installAlembic", alembicLibraryInstall )
		alembicEnv.Alias( "installLib", [ alembicLibraryInstall ] )

		# headers
		alembicHeaderInstall = alembicEnv.Install( "$INSTALL_HEADER_DIR/IECoreAlembic", alembicHeaders )
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
		alembicPythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreAlembic", lambda target, source, env : makeSymLinks( alembicPythonModuleEnv, alembicPythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
		alembicPythonModuleEnv.Alias( "install", alembicPythonModuleInstall )
		alembicPythonModuleEnv.Alias( "installAlembic", alembicPythonModuleInstall )

		Default( [ alembicLibrary, alembicPythonModule ] )

		# tests
		alembicTestEnv = testEnv.Clone()
		alembicTestLibPaths = alembicEnv.subst( ":".join( alembicPythonModuleEnv["LIBPATH"] ) )
		alembicTestEnv["ENV"][alembicTestEnv["TEST_LIBRARY_PATH_ENV_VAR"]] += ":" + alembicTestLibPaths
		alembicTestEnv["ENV"]["PYTHONPATH"] += ":./contrib/IECoreAlembic/python"
		alembicTest = alembicTestEnv.Command( "contrib/IECoreAlembic/test/IECoreAlembic/results.txt", alembicPythonModule, pythonExecutable + " $TEST_ALEMBIC_SCRIPT" )
		NoCache( alembicTest )
		alembicTestEnv.Alias( "testAlembic", alembicTest )

###########################################################################################
# Build, install and test the IECoreAppleseed library and bindings
###########################################################################################

appleseedEnv = coreEnv.Clone( IECORE_NAME = "IECoreAppleseed" )
appleseedEnv.Append(
	CXXFLAGS = [
		"-isystem", "$APPLESEED_INCLUDE_PATH",
		"-isystem", "$OSL_INCLUDE_PATH",
		"-isystem", "$OIIO_INCLUDE_PATH",
		"-DIECoreAppleseed_EXPORTS",
	],
	CPPPATH = [
		"contrib/IECoreAppleseed/include",
	],
	CPPFLAGS = [
		"-DAPPLESEED_ENABLE_IMATH_INTEROP",
		"-DAPPLESEED_USE_SSE",
	],
)
appleseedEnv.Append( LIBPATH = [ "$APPLESEED_LIB_PATH", "$OSL_LIB_PATH", "$OIIO_LIB_PATH" ] )

appleseedPythonModuleEnv = pythonModuleEnv.Clone( IECORE_NAME = "IECoreAppleseed" )
appleseedPythonModuleEnv.Append(
	CXXFLAGS = [
		"-isystem", "$APPLESEED_INCLUDE_PATH",
		"-isystem", "$OSL_INCLUDE_PATH",
		"-isystem", "$OIIO_INCLUDE_PATH",
	],
	CPPPATH = [
		"contrib/IECoreAppleseed/include",
		"contrib/IECoreAppleseed/include/bindings",
	],
	CPPFLAGS = [
		"-DAPPLESEED_ENABLE_IMATH_INTEROP",
		"-DAPPLESEED_USE_SSE",
	],
	LIBPATH = [
		"$APPLESEED_LIB_PATH",
		"$OSL_LIB_PATH",
		"$OIIO_LIB_PATH"
	],
)


appleseedDriverEnv = appleseedEnv.Clone( IECORE_NAME = "ieDisplay" )
appleseedDriverEnv["SHLIBPREFIX"] = ""
appleseedDriverEnv["SHLIBSUFFIX"] = ".so"

haveAppleseed = False

if doConfigure :

	# Since we only build shared libraries and not exectuables,
	# we only need to check that shared libs will link correctly.
	# This is necessary for appleseed, which uses
	# a run-time compatible, but link-time incompatbile libstdc++
	# in some obscure studio setups. This approach succeeds because
	# building a shared library doesn't require resolving the
	# unresolved symbols of the libraries that it links to.
	appleseedCheckEnv = appleseedEnv.Clone()
	appleseedCheckEnv.Append( CXXFLAGS = [ "-fPIC" ] )
	appleseedCheckEnv.Append( LINKFLAGS = [ "-shared" ] )
	c = Configure( appleseedCheckEnv )

	if not c.CheckLibWithHeader( "appleseed", "renderer/api/rendering.h", "CXX" ) :

		sys.stderr.write( "WARNING : no appleseed library found, not building IECoreAppleseed - check APPLESEED_INCLUDE_PATH and APPLESEED_LIB_PATH.\n" )
		c.Finish()

	else :

		haveAppleseed = True

		appleseedSources = sorted( glob.glob( "contrib/IECoreAppleseed/src/IECoreAppleseed/*.cpp" ) )
		appleseedHeaders = glob.glob( "contrib/IECoreAppleseed/include/IECoreAppleseed/*.h" ) + glob.glob( "contrib/IECoreAppleseed/include/IECoreAppleseed/*.inl" )
		appleseedPythonSources = sorted( glob.glob( "contrib/IECoreAppleseed/src/IECoreAppleseed/bindings/*.cpp" ) )
		appleseedPythonScripts = glob.glob( "contrib/IECoreAppleseed/python/IECoreAppleseed/*.py" )

		c.Finish()

		appleseedEnv.Append(
			LIBS = [
				"appleseed",
				# We can't append this before configuring, as then it gets built as
				# part of the configure process
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)

		appleseedPythonModuleEnv.Append(
			LIBS = [
				os.path.basename( corePythonEnv.subst( "$INSTALL_PYTHONLIB_NAME" ) ),
				os.path.basename( sceneEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)

		appleseedDriverEnv.Append(
			LIBS = [
				"appleseed",
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( imageEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( appleseedEnv.subst( "$INSTALL_LIB_NAME" ) ),
			]
		)

		# library
		appleseedLibrary = appleseedEnv.SharedLibrary( "lib/" + os.path.basename( appleseedEnv.subst( "$INSTALL_APPLESEEDLIB_NAME" ) ), appleseedSources )
		appleseedLibraryInstall = appleseedEnv.Install( os.path.dirname( appleseedEnv.subst( "$INSTALL_APPLESEEDLIB_NAME" ) ), appleseedLibrary )
		appleseedEnv.NoCache( appleseedLibraryInstall )
		appleseedEnv.AddPostAction( appleseedLibraryInstall, lambda target, source, env : makeLibSymLinks( appleseedEnv ) )
		appleseedEnv.Alias( "install", appleseedLibraryInstall )
		appleseedEnv.Alias( "installAppleseed", appleseedLibraryInstall )
		appleseedEnv.Alias( "installLib", [ appleseedLibraryInstall ] )

		# headers
		appleseedHeaderInstall = appleseedEnv.Install( "$INSTALL_HEADER_DIR/IECoreAppleseed", appleseedHeaders )
		appleseedEnv.AddPostAction( "$INSTALL_HEADER_DIR/IECoreAppleseed", lambda target, source, env : makeSymLinks( appleseedEnv, appleseedEnv["INSTALL_HEADER_DIR"] ) )
		appleseedEnv.Alias( "install", appleseedHeaderInstall )
		appleseedEnv.Alias( "installAppleseed", appleseedHeaderInstall )

		# python module
		appleseedPythonModuleEnv.Append(
			LIBS = [
				os.path.basename( coreEnv.subst( "$INSTALL_LIB_NAME" ) ),
				os.path.basename( appleseedEnv.subst( "$INSTALL_LIB_NAME" ) ),
				"appleseed",
			]
		)
		appleseedPythonModule = appleseedPythonModuleEnv.SharedLibrary( "contrib/IECoreAppleseed/python/IECoreAppleseed/_IECoreAppleseed", appleseedPythonSources )
		appleseedPythonModuleEnv.Depends( appleseedPythonModule, appleseedLibrary )

		appleseedPythonModuleInstall = appleseedPythonModuleEnv.Install( "$INSTALL_PYTHON_DIR/IECoreAppleseed", appleseedPythonScripts + appleseedPythonModule )
		appleseedPythonModuleEnv.AddPostAction( "$INSTALL_PYTHON_DIR/IECoreAppleseed", lambda target, source, env : makeSymLinks( appleseedPythonModuleEnv, appleseedPythonModuleEnv["INSTALL_PYTHON_DIR"] ) )
		appleseedPythonModuleEnv.Alias( "install", appleseedPythonModuleInstall )
		appleseedPythonModuleEnv.Alias( "installAppleseed", appleseedPythonModuleInstall )


		# output driver
		appleseedDriver = appleseedDriverEnv.SharedLibrary( "contrib/IECoreAppleseed/src/IECoreAppleseed/outputDriver/" + os.path.basename( appleseedDriverEnv.subst( "$INSTALL_APPLESEEDOUTPUTDRIVER_NAME" ) ), "contrib/IECoreAppleseed/src/IECoreAppleseed/outputDriver/DisplayTileCallback.cpp" )
		appleseedDriverInstall = appleseedDriverEnv.Install( os.path.dirname( appleseedDriverEnv.subst( "$INSTALL_APPLESEEDOUTPUTDRIVER_NAME" ) ), appleseedDriver )
		appleseedDriverEnv.NoCache( appleseedDriverInstall )
		appleseedDriverEnv.AddPostAction( appleseedDriverInstall, lambda target, source, env : makeLibSymLinks( appleseedDriverEnv, libNameVar="INSTALL_APPLESEEDOUTPUTDRIVER_NAME" ) )
		appleseedDriverEnv.Alias( "install", appleseedDriverInstall )
		appleseedDriverEnv.Alias( "installAppleseed", appleseedDriverInstall )
		appleseedDriverForTest = appleseedDriverEnv.Install( "contrib/IECoreAppleseed/test/IECoreAppleseed/plugins", appleseedDriver )

		Default( [ appleseedLibrary, appleseedPythonModule, appleseedDriver, appleseedDriverForTest ] )

		# tests
		appleseedTestEnv = testEnv.Clone()
		appleseedTestEnv["ENV"]["PYTHONPATH"] += ":./contrib/IECoreAppleseed/python" + ":" + appleseedEnv.subst( "$APPLESEED_LIB_PATH/python2.7" )
		appleseedTestEnv["ENV"][testEnv["TEST_LIBRARY_PATH_ENV_VAR"]] += ":" + appleseedEnv.subst( ":".join( appleseedPythonModuleEnv["LIBPATH"] ) )
		appleseedTestEnv["ENV"]["PATH"] = appleseedEnv.subst( "$APPLESEED_ROOT/bin" ) + ":" + appleseedTestEnv["ENV"]["PATH"]
		appleseedTestEnv["ENV"]["APPLESEED_SEARCHPATH"] = os.getcwd() + "/contrib/IECoreAppleseed/test/IECoreAppleseed/plugins"
		appleseedTest = appleseedTestEnv.Command( "contrib/IECoreAppleseed/test/IECoreAppleseed/results.txt", appleseedPythonModule, pythonExecutable + " $TEST_APPLESEED_SCRIPT" )
		NoCache( appleseedTest )
		appleseedTestEnv.Depends( appleseedTest, [ corePythonModule, scenePythonModule, appleseedPythonModule, appleseedLibrary, appleseedDriverForTest ] )
		appleseedTestEnv.Depends( appleseedTest, glob.glob( "contrib/IECoreAppleseed/test/IECoreAppleseed/*.py" ) )
		appleseedTestEnv.Alias( "testAppleseed", appleseedTest )

###########################################################################################
# Documentation
###########################################################################################

docEnv = env.Clone()
docEnv["ENV"]["PATH"] = os.environ["PATH"]

if doConfigure :

	sys.stdout.write( "Checking for doxygen... " )

	if docEnv.WhereIs( docEnv["DOXYGEN"] ) :

		sys.stdout.write( "yes\n" )

		docs = docEnv.Command( "doc/html/index.html", "doc/config/Doxyfile", "sed s/!CORTEX_VERSION!/$IECORE_MAJORMINORPATCH_VERSION/g $SOURCE | $DOXYGEN -" )
		docEnv.NoCache( docs )

		for modulePath in ( "python/IECore", "python/IECoreGL", "python/IECoreNuke", "python/IECoreMaya", "python/IECoreHoudini" ) :

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
