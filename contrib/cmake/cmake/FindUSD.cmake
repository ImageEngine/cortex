# Simple module to find USD.

set(USD_LIB_NAME "libusd.so")
if( "${CMAKE_SYSTEM_NAME}" MATCHES "Windows" )
    set(USD_LIB_NAME "usd.lib")
elseif( "${CMAKE_SYSTEM_NAME}" MATCHES "Darwin" )
    set(USD_LIB_NAME "libusd.dynlib")
endif()

if (EXISTS "$ENV{USD_ROOT}")
    set(USD_ROOT $ENV{USD_ROOT})
endif ()

find_path(USD_INCLUDE_DIR pxr/pxr.h
          PATHS ${USD_ROOT}/include
          DOC "USD Include directory")

find_path(USD_LIBRARY_DIR ${USD_LIB_NAME}
          PATHS ${USD_ROOT}/lib
          DOC "USD Libraries directory")

#find_file(USD_GENSCHEMA
#          names usdGenSchema
#          PATHS ${USD_ROOT}/bin
#          DOC "USD Gen schema application")

if(USD_INCLUDE_DIR AND EXISTS "${USD_INCLUDE_DIR}/pxr/pxr.h")
    foreach(_usd_comp MAJOR MINOR PATCH)
        file(STRINGS
             "${USD_INCLUDE_DIR}/pxr/pxr.h"
             _usd_tmp
             REGEX "#define PXR_${_usd_comp}_VERSION .*$")
        string(REGEX MATCHALL "[0-9]+" USD_${_usd_comp}_VERSION ${_usd_tmp})
    endforeach()
    set(USD_VERSION ${USD_MAJOR_VERSION}.${USD_MINOR_VERSION}.${USD_PATCH_VERSION})
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    USD
    REQUIRED_VARS
    USD_INCLUDE_DIR
    USD_LIBRARY_DIR
#    USD_GENSCHEMA
    VERSION_VAR
    USD_VERSION)

# USD libs
set( USD_LIBRARIES
        ar
        arch
        gf
        js
        kind
        pcp
        plug
        sdf
        tf
        tracelite
        usd
        vt
)

# USD schema libs
set( USD_SCHEMA_LIBRARIES
        usdGeom
        usdHydra
        usdLux
        usdRi
        usdShade
        usdSkel
        usdUI
        usdUtils
)