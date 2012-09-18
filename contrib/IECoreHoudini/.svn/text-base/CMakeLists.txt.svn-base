##########################################################################
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

cmake_minimum_required( VERSION 2.8 )
project( IECoreHoudini )

# where to find our cmake modules
set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/config/cmake )

# find our library dependencies
find_package( Houdini REQUIRED )
find_package( Cortex REQUIRED )
find_package( TBB REQUIRED )
find_package( Boost 1.33 REQUIRED COMPONENTS python )
find_package( PythonLibs 2.5 REQUIRED )

# this stops the HDK causing a backwards include warning
add_definitions( -D_BACKWARD_BACKWARD_WARNING_H )

# our include directories
include_directories(
  	${CMAKE_SOURCE_DIR}/include
  	${CMAKE_SOURCE_DIR}/include/bindings
  	${PYTHON_INCLUDE_PATH}
  	${Boost_INCLUDE_DIRS}
  	${Houdini_INCLUDE_DIR}
  	${Cortex_INCLUDE_DIR}
  	${TBB_INCLUDE_DIR}
  	$ENV{ILMBASE_ROOT}/include
  	$ENV{ILMBASE_ROOT}/include/OpenEXR
	$ENV{OPENEXR_ROOT}/include
  	$ENV{OPENEXR_ROOT}/include/OpenEXR
	$ENV{GLEW_ROOT}/include/GL
  	)

#=====
# create library
#=====
add_library( houdini_lib
	SHARED
	${CMAKE_SOURCE_DIR}/src/CoreHoudini.cpp
  	${CMAKE_SOURCE_DIR}/src/FnProceduralHolder.cpp
  	${CMAKE_SOURCE_DIR}/src/FnParameterisedHolder.cpp
  	${CMAKE_SOURCE_DIR}/src/FromHoudiniSopConverter.cpp
  	${CMAKE_SOURCE_DIR}/src/SOP_ParameterisedHolder.cpp
  	${CMAKE_SOURCE_DIR}/src/SOP_ProceduralHolder.cpp
  	)

set_target_properties( houdini_lib
  	PROPERTIES
  	OUTPUT_NAME "IECoreHoudini"
  	PREFIX "lib"
  	LIBRARY_OUTPUT_DIRECTORY "lib"
  	COMPILE_FLAGS "-O0 -ggdb ${Houdini_COMPILE_FLAGS}"
  	LINK_FLAGS "${Houdini_LINK_FLAGS}"
  	)

target_link_libraries( houdini_lib
  	${Houdini_LIBRARIES}
  	${Cortex_LIBRARY}
  	${Boost_LIBRARIES}
  	${TBB_LIBRARIES}
  	)

#=====
# create houdini plugin
#=====
add_library( houdini_plugin
 	SHARED
  	${CMAKE_SOURCE_DIR}/src/plugin/Plugin.cpp
  	${CMAKE_SOURCE_DIR}/src/GR_Procedural.cpp
 	)

set_target_properties( houdini_plugin
  	PROPERTIES
  	OUTPUT_NAME "ieCoreHoudini"
  	PREFIX ""
  	LIBRARY_OUTPUT_DIRECTORY "houdini/dso"
  	COMPILE_FLAGS "-O0 -ggdb ${Houdini_COMPILE_FLAGS}"
  	LINK_FLAGS "${Houdini_LINK_FLAGS}"
  	)

target_link_libraries( houdini_plugin
  	${Houdini_LIBRARIES}
  	${Cortex_LIBRARY}
  	IECoreGL
  	${Boost_LIBRARIES}
  	${PYTHON_LIBRARIES}
  	houdini_lib
  	)

#=====
# create python bindings
#=====
add_library( houdini_binding
	SHARED
    ${CMAKE_SOURCE_DIR}/src/bindings/IECoreHoudiniModule.cpp
	${CMAKE_SOURCE_DIR}/src/bindings/FnProceduralHolderBinding.cpp
	${CMAKE_SOURCE_DIR}/src/bindings/FromHoudiniSopConverterBinding.cpp
  	)

set_target_properties( houdini_binding
	PROPERTIES
	OUTPUT_NAME "_IECoreHoudini"
	PREFIX ""
  	LIBRARY_OUTPUT_DIRECTORY "lib/python2.5/site-packages/IECoreHoudini"
	SUFFIX ".so"
  	COMPILE_FLAGS "-O0 -ggdb ${Houdini_COMPILE_FLAGS}"
  	LINK_FLAGS "${Houdini_LINK_FLAGS}"
	)

target_link_libraries( houdini_binding
  	${Boost_LIBRARIES}
  	${PYTHON_LIBRARIES}
 	${Houdini_LIBRARIES}
  	${Cortex_LIBRARY}
  	IECorePython
  	houdini_lib
  	houdini_plugin
  	)
