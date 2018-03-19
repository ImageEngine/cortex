# CMake version required
cmake_minimum_required( VERSION 2.8 )

# Declare a Cortex Project
project( Cortex )

set (CMAKE_CXX_STANDARD 11)

# Declare the version

set( CORTEX_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../.. )

file( READ  ${CORTEX_SOURCE_DIR}/SConstruct SCONSTRUCT )

string( REGEX MATCH "ieCoreMajorVersion=([0-9]+)" ${SCONSTRUCT} )
set( IECORE_MAJOR_VERSION ${CMAKE_MATCH_1} )

string( REGEX MATCH "ieCoreMinorVersion=([0-9]+)" ${SCONSTRUCT} )
set( IECORE_MINOR_VERSION ${CMAKE_MATCH_1} )

string( REGEX MATCH "ieCorePatchVersion=([0-9]+)" ${SCONSTRUCT} )
set( IECORE_PATCH_VERSION ${CMAKE_MATCH_1} )

string( REGEX MATCH "ieCoreVersionSuffix=\"([a-zA-Z0-9]+)\"" ${SCONSTRUCT} )
set( IECORE_VERSION_SUFFIX ${CMAKE_MATCH_1} )

message( STATUS "Configuring Cortex : " ${IECORE_MAJOR_VERSION}.${IECORE_MINOR_VERSION}.${IECORE_PATCH_VERSION}-${IECORE_VERSION_SUFFIX} )

add_definitions( -DIE_CORE_MAJORVERSION=${IECORE_MAJOR_VERSION}
                 -DIE_CORE_MINORVERSION=${IECORE_MINOR_VERSION}
                 -DIE_CORE_PATCHVERSION=${IECORE_PATCH_VERSION} )

option( WITH_TESTS "Compile tests" OFF )
option( WITH_IECORE_PYTHON "Compile IECorePython" ON )
option( WITH_IECORE_GL "Compile IECoreGL" OFF )
option( WITH_IECORE_IMAGE "Compile IECoreImage" OFF )
option( WITH_IECORE_SCENE "Compile IECoreScene" OFF )
option( WITH_IECORE_ALEMBIC "Compile IECoreAlembic" OFF )
option( WITH_IECORE_VDB "Compile IECoreVDB" OFF)
option( WITH_IECORE_ARNOLD "Compile IECoreArnold" OFF )
option( WITH_IECORE_APPLESEED "Compile IECoreAppleseed" OFF )
option( WITH_IECORE_USD "Compile IECoreUSD" OFF )

option( WITH_IECORE_HOUDINI "Compile IECoreHoudini" OFF )
option( WITH_IECORE_MAYA "Compile IECoreMaya" OFF )
option( WITH_IECORE_NUKE "Compile IECoreNuke" OFF )

#-******************************************************************************
#-******************************************************************************
# PLATFORM SPECIFIC
#-******************************************************************************
#-******************************************************************************

# Windows flag
set( WINDOWS FALSE )
if( "${CMAKE_SYSTEM_NAME}" MATCHES "Windows" )
  set( WINDOWS TRUE )
endif()

set( DARWIN FALSE )
if( "${CMAKE_SYSTEM_NAME}" MATCHES "Darwin" )
    set( DARWIN TRUE )
    if( ${CMAKE_SYSTEM_VERSION} MATCHES "13." )
        add_definitions( "-std=c++11" )
    endif()
endif()

set( LINUX FALSE )
if( "${CMAKE_SYSTEM_NAME}" MATCHES "Linux" )
    set( LINUX TRUE )
endif()

# Set some debug vs opt flags
if( "${CMAKE_BUILD_TYPE}" MATCHES "Debug" )
    add_definitions( -DDEBUG=1 -UNDEBUG )
    if( NOT WINDOWS )
        add_definitions( -pipe -Wall -O0 -Wno-unused-local-typedefs )
    endif()
elseif ( "${CMAKE_BUILD_TYPE}" MATCHES "Release" )
    add_definitions( -DNDEBUG=1 -UDEBUG )
    if ( NOT WINDOWS )
        add_definitions( -pipe -Wall -Werror -O3 -DNDEBUG -DBOOST_DISABLE_ASSERTS -Wno-unused-local-typedefs -Wno-strict-aliasing -Wno-maybe-uninitialized ) #-Wno-return-type )
    endif()
endif()

if( ${WINDOWS} )
    add_definitions( -DOPENEXR_DLL -DNOMINMAX -D__PRETTY_FUNCTION__=__FUNCSIG__ -DBOOST_ALL_DYN_LINK )
    set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} /MP )
    set( PLATFORM_LIBS winmm.lib )
endif()

set( CORTEX_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../.. )
#-******************************************************************************
#-******************************************************************************
# GLOBAL PACKAGE & INCLUDE/LINK DEPENDENCY
#-******************************************************************************
#-******************************************************************************

# check in the source directory
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/ )

# Boost definitions
add_definitions( -DBOOST_FILESYSTEM_VERSION=3 )
add_definitions( -DBOOST_FILESYSTEM_NO_DEPRICATED )
add_definitions( -DBOOST_SIGNALS_NO_DEPRECATION_WARNING )
add_definitions( -DBOOST_PYTHON_MAX_ARITY=20 )

# Find packages
find_package( Threads REQUIRED )
find_package( ZLIB REQUIRED )
find_package( TBB REQUIRED )
find_package( PythonLibs 2.7 REQUIRED )
find_package( Boost REQUIRED COMPONENTS signals iostreams filesystem date_time wave regex system chrono python )
find_package( IlmBase REQUIRED )
find_package( OpenEXR REQUIRED )
find_package( Freetype REQUIRED )
find_package( Blosc REQUIRED )
find_package( PNG REQUIRED ) # For freetype linking

# Include directories
include_directories( ${CORTEX_SOURCE_DIR}/include )
include_directories( ${TBB_INCLUDE_DIRS} )
include_directories( ${PYTHON_INCLUDE_DIRS} )
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( ${ILMBASE_INCLUDE_DIR} )
include_directories( ${OPENEXR_INCLUDE_DIRS} )
include_directories( ${FREETYPE_INCLUDE_DIRS} )
include_directories( ${BLOSC_INCLUDE_DIR} )

# Link directories
link_directories( ${TBB_LIBRARY_DIRS} )
link_directories( ${Boost_LIBRARY_DIRS} )

# Base libs to link to
set( BASE_LIBS
     ${Boost_SIGNALS_LIBRARY}
     ${Boost_IOSTREAMS_LIBRARY}
     ${Boost_FILESYSTEM_LIBRARY}
     ${Boost_DATE_TIME_LIBRARY}
     ${Boost_THREAD_LIBRARY}
     ${Boost_WAVE_LIBRARY}
     ${Boost_REGEX_LIBRARY}
     ${Boost_SYSTEM_LIBRARY}
     ${Boost_CHRONO_LIBRARY}
     ${TBB_LIBRARIES}
     ${Ilmbase_HALF_LIBRARY}
     ${Ilmbase_IEX_LIBRARY}
     ${Ilmbase_IEXMATH_LIBRARY}
     ${Ilmbase_ILMTHREAD_LIBRARY}
     ${Ilmbase_IMATH_LIBRARY}
     ${Openexr_ILMIMF_LIBRARY}
     ${ZLIB_LIBRARIES}
     ${BLOSC_blosc_LIBRARY} )

# Python base libs to link to
set( BASE_PYTHON_LIBS
     ${PYTHON_LIBRARIES}
     ${Boost_PYTHON_LIBRARY}
     IECorePython )

# IECore
file( GLOB IECORE_CXX_FILES  ${CORTEX_SOURCE_DIR}/src/IECore/*.cpp )
file( GLOB IECORE_H_FILES ${CORTEX_SOURCE_DIR}/include/IECore/*.h ${CORTEX_SOURCE_DIR}/include/IECore/*.inl )
file( GLOB IECORE_PY_FILES ${CORTEX_SOURCE_DIR}/python/IECore/*.py )
add_library( IECore SHARED ${IECORE_CXX_FILES} ${IECORE_H_FILES} )
target_link_libraries( IECore ${BASE_LIBS} )
# Install
install( TARGETS IECore DESTINATION lib/ )
install( FILES ${IECORE_H_FILES} DESTINATION include/IECore )

# IECorePython
if( ${WITH_IECORE_PYTHON} )
    file( GLOB IECOREPYTHON_CXX_FILES ${CORTEX_SOURCE_DIR}/src/IECorePython/*.cpp )
    file( GLOB IECOREPYTHON_H_FILES ${CORTEX_SOURCE_DIR}/include/IECorePython/*.h ${CORTEX_SOURCE_DIR}/include/IECorePython/*.inl )
    add_library( IECorePython SHARED ${IECOREPYTHON_CXX_FILES} ${IECOREPYTHON_H_FILES} )
    target_link_libraries( IECorePython IECore ${BASE_LIBS} ${PYTHON_LIBRARIES} ${Boost_PYTHON_LIBRARY} )
    add_dependencies( IECorePython IECore )
    install( TARGETS IECorePython DESTINATION lib/ )
    install( FILES ${IECOREPYTHON_H_FILES} DESTINATION include/IECorePython )
    # IECorePythonModule
    add_library( _IECore MODULE ${CORTEX_SOURCE_DIR}/src/IECorePythonModule/IECore.cpp )
    target_link_libraries( _IECore IECorePython )
    add_dependencies( _IECore IECore IECorePython )
    if ( WIN32 )
        set_target_properties( _IECore PROPERTIES SUFFIX ".pyd" )
    endif()
    # Install
    install( TARGETS _IECore DESTINATION python/IECore )
    install( FILES ${IECORE_PY_FILES} DESTINATION python/IECore )
endif()

# IECoreGL
if( ${WITH_IECORE_GL} )
    find_package( OpenGL REQUIRED )
    find_package( GLEW REQUIRED )
    # Library
    file( GLOB IECOREGL_CXX_FILES ${CORTEX_SOURCE_DIR}/src/IECoreGL/*.cpp )
    file( GLOB IECOREGL_H_FILES ${CORTEX_SOURCE_DIR}/include/IECoreGL/*.h ${CORTEX_SOURCE_DIR}/include/IECoreGL/*.inl )
    file( GLOB IECOREGL_PY_FILES ${CORTEX_SOURCE_DIR}/python/IECoreGL/*.py )
    add_library( IECoreGL SHARED ${IECOREGL_CXX_FILES} ${IECOREGL_H_FILES} )
    target_link_libraries( IECoreGL IECore IECoreScene IECoreImage ${BASE_LIBS} ${OPENGL_LIBRARIES} ${GLEW_LIBRARY} )
    add_dependencies( IECoreGL IECore )
    install( TARGETS IECoreGL DESTINATION lib/ )
    install( FILES ${IECOREGL_H_FILES} DESTINATION include/IECoreGL )
    install( DIRECTORY ${CORTEX_SOURCE_DIR}/glsl DESTINATION . )

    if( ${WITH_IECORE_PYTHON} )
        # Module/Bindings
        file( GLOB IECOREGL_CXX_MODULE_FILES ${CORTEX_SOURCE_DIR}/src/IECoreGL/bindings/*.cpp )
        file( GLOB IECOREGL_H_MODULE_FILES ${CORTEX_SOURCE_DIR}/include/IECoreGL/bindings/*.h ${CORTEX_SOURCE_DIR}/include/IECoreGL/bindings/*.inl )
        add_library( _IECoreGL MODULE ${IECOREGL_CXX_MODULE_FILES} ${IECOREGL_H_MODULE_FILES} )
        target_link_libraries( _IECoreGL IECore ${BASE_LIBS} ${BASE_PYTHON_LIBS} ${OPENGL_LIBRARIES} ${GLEW_LIBRARY} IECoreGL )
        add_dependencies( _IECoreGL IECore IECorePython IECoreGL )
        if ( WIN32 )
            set_target_properties( _IECoreGL PROPERTIES SUFFIX ".pyd" )
        endif()
        install( TARGETS _IECoreGL DESTINATION python/IECoreGL )
        install( FILES ${IECOREGL_PY_FILES} DESTINATION python/IECoreGL )
    endif()
endif()

# IECoreImage
if( ${WITH_IECORE_IMAGE} )
    find_package( OSL REQUIRED )
    find_package( OpenImageIO REQUIRED )
    # Library
    file( GLOB IECOREIMAGE_CXX_FILES ${CORTEX_SOURCE_DIR}/src/IECoreImage/*.cpp )
    file( GLOB IECOREIMAGE_H_FILES ${CORTEX_SOURCE_DIR}/include/IECoreImage/*.h ${CORTEX_SOURCE_DIR}/include/IECoreImage/*.inl )
    file( GLOB IECOREIMAGE_PY_FILES ${CORTEX_SOURCE_DIR}/python/IECoreImage/*.py )
    add_library( IECoreImage SHARED ${IECOREIMAGE_CXX_FILES} ${IECOREIMAGE_H_FILES} )
    target_link_libraries( IECoreImage IECore ${BASE_LIBS} ${OPENIMAGEIO_LIBRARIES} ${FREETYPE_LIBRARIES} ${PNG_LIBRARIES} )
    add_dependencies( IECoreImage IECore )
    install( TARGETS IECoreImage DESTINATION lib/ )
    install( FILES ${IECOREIMAGE_H_FILES} DESTINATION include/IECoreImage )

    if( ${WITH_IECORE_PYTHON} )
        # Bindings
        file( GLOB IECOREIMAGEBINDINGS_CXX_FILES ${CORTEX_SOURCE_DIR}/src/IECoreImageBindings/*.cpp )
        file( GLOB IECOREIMAGEBINDINGS_H_FILES ${CORTEX_SOURCE_DIR}/include/IECoreImageBindings/*.h )
        # Module
        add_library( _IECoreImage MODULE ${CORTEX_SOURCE_DIR}/src/IECoreImageModule/IECoreImageModule.cpp ${IECOREIMAGEBINDINGS_CXX_FILES} ${IECOREIMAGEBINDINGS_H_FILES} )
        target_link_libraries( _IECoreImage IECore ${BASE_LIBS} ${BASE_PYTHON_LIBS} ${OPENIMAGEIO_LIBRARIES} IECoreImage )
        add_dependencies( _IECoreImage IECore IECorePython IECoreImage )
        if ( WIN32 )
            set_target_properties( _IECoreImage PROPERTIES SUFFIX ".pyd" )
        endif()
        install( TARGETS _IECoreImage DESTINATION python/IECoreImage )
        install( FILES ${IECOREIMAGE_PY_FILES} DESTINATION python/IECoreImage )
        install( FILES ${IECOREIMAGEBINDINGS_H_FILES} DESTINATION include/IECoreImageBindings )
    endif()
endif()

# IECoreScene
if( ${WITH_IECORE_SCENE} )
    # Library
    file( GLOB IECORESCENE_CXX_FILES ${CORTEX_SOURCE_DIR}/src/IECoreScene/*.cpp )
    file( GLOB IECORESCENE_H_FILES ${CORTEX_SOURCE_DIR}/include/IECoreScene/*.h ${CORTEX_SOURCE_DIR}/include/IECoreScene/*.inl )
    file( GLOB IECORESCENE_PY_FILES ${CORTEX_SOURCE_DIR}/python/IECoreScene/*.py )
    add_library( IECoreScene SHARED ${IECORESCENE_CXX_FILES} ${IECORESCENE_H_FILES} )
    target_link_libraries( IECoreScene IECore ${BASE_LIBS} ${FREETYPE_LIBRARIES} ${PNG_LIBRARIES} )
    add_dependencies( IECoreScene IECore )
    install( TARGETS IECoreScene DESTINATION lib/ )
    install( FILES ${IECORESCENE_H_FILES} DESTINATION include/IECoreScene )

    if( ${WITH_IECORE_PYTHON} )
        # Module/Bindings
        file( GLOB IECORESCENE_CXX_MODULE_FILES  ${CORTEX_SOURCE_DIR}/src/IECoreScene/bindings/*.cpp )
        file( GLOB IECORESCENE_H_MODULE_FILES  ${CORTEX_SOURCE_DIR}/src/IECoreScene/bindings/*.h ${CORTEX_SOURCE_DIR}/src/IECoreScene/bindings/*.inl )
        add_library( _IECoreScene MODULE ${IECORESCENE_CXX_MODULE_FILES} ${IECORESCENE_H_MODULE_FILES} )
        target_link_libraries( _IECoreScene IECore ${BASE_LIBS} ${BASE_PYTHON_LIBS} IECoreScene )
        add_dependencies( _IECoreScene IECore IECorePython IECoreScene )
        if ( WIN32 )
            set_target_properties( _IECoreScene PROPERTIES SUFFIX ".pyd" )
        endif()
        install( TARGETS _IECoreScene DESTINATION python/IECoreScene )
        install( FILES ${IECORESCENE_PY_FILES} DESTINATION python/IECoreScene )
    endif()
endif()

# IECoreAlembic
if( ${WITH_IECORE_ALEMBIC} )
    #find_package( HDF5 REQUIRED )
    find_package( Alembic REQUIRED )
    include_directories( ${CORTEX_SOURCE_DIR}/contrib/IECoreAlembic/include )
    # Library
    file( GLOB IECOREALEMBIC_CXX_FILES ${CORTEX_SOURCE_DIR}/contrib/IECoreAlembic/src/IECoreAlembic/*.cpp )
    file( GLOB IECOREALEMBIC_H_FILES ${CORTEX_SOURCE_DIR}/contrib/IECoreAlembic/include/IECoreAlembic/*.h ${CORTEX_SOURCE_DIR}/contrib/IECoreAlembic/include/IECoreAlembic/*.inl )
    file( GLOB IECOREALEMBIC_PY_FILES  ${CORTEX_SOURCE_DIR}/contrib/IECoreAlembic/python/IECoreAlembic/*.py )
    add_library( IECoreAlembic SHARED ${IECOREALEMBIC_CXX_FILES} ${IECOREALEMBIC_H_FILES} )
    target_link_libraries( IECoreAlembic IECore IECoreScene ${BASE_LIBS} ${ALEMBIC_LIBRARY} )
    add_dependencies( IECoreAlembic IECore )
    install( TARGETS IECoreAlembic DESTINATION lib/ )
    install( FILES ${IECOREALEMBIC_H_FILES} DESTINATION include/IECoreAlembic )

    if( ${WITH_IECORE_PYTHON} )
        # Module/Bindings
        file( GLOB IECOREALEMBIC_CXX_MODULE_FILES  ${CORTEX_SOURCE_DIR}/contrib/IECoreAlembic/src/IECoreAlembic/bindings/*.cpp )
        file( GLOB IECOREALEMBIC_H_MODULE_FILES  ${CORTEX_SOURCE_DIR}/contrib/IECoreAlembic/include/IECoreAlembic/bindings/*.h )
        add_library( _IECoreAlembic MODULE ${IECOREALEMBIC_CXX_MODULE_FILES} ${IECOREALEMBIC_H_MODULE_FILES} )
        target_link_libraries( _IECoreAlembic IECore ${BASE_LIBS} ${BASE_PYTHON_LIBS} IECoreScene IECoreAlembic )
        add_dependencies( _IECoreAlembic IECore IECorePython IECoreAlembic )
        if ( WIN32 )
            set_target_properties( _IECoreAlembic PROPERTIES SUFFIX ".pyd" )
        endif()
        install( TARGETS _IECoreAlembic DESTINATION python/IECoreAlembic )
        install( FILES ${IECOREALEMBIC_PY_FILES} DESTINATION python/IECoreAlembic )
    endif()
endif()

# IECoreVDB
if( ${WITH_IECORE_VDB} )
    find_package( OpenVDB REQUIRED )
    include_directories( ${OpenVDB_INCLUDE_DIR} )
    # Library
    file( GLOB IECOREVDB_CXX_FILES ${CORTEX_SOURCE_DIR}/src/IECoreVDB/*.cpp )
    file( GLOB IECOREVDB_H_FILES ${CORTEX_SOURCE_DIR}/include/IECoreVDB/*.h ${CORTEX_SOURCE_DIR}/include/IECoreVDB/*.inl )
    file( GLOB IECOREVDB_PY_FILES ${CORTEX_SOURCE_DIR}/python/IECoreVDB/*.py )
    add_library( IECoreVDB SHARED ${IECOREVDB_CXX_FILES} ${IECOREVDB_H_FILES} )
    target_link_libraries( IECoreVDB IECore IECoreScene ${BASE_LIBS} ${OpenVDB_LIBRARIES} )
    target_compile_options( IECoreVDB PRIVATE )
    add_dependencies( IECoreVDB IECore )
    install( TARGETS IECoreVDB DESTINATION lib/ )
    install( FILES ${IECOREVDB_H_FILES} DESTINATION include/IECoreVDB )

    if( ${WITH_IECORE_PYTHON} )
        # Module/Bindings
        file( GLOB IECOREVDB_CXX_MODULE_FILES  ${CORTEX_SOURCE_DIR}/src/IECoreVDB/bindings/*.cpp )
        file( GLOB IECOREVDB_H_MODULE_FILES  ${CORTEX_SOURCE_DIR}/src/IECoreVDB/bindings/*.h )
        add_library( _IECoreVDB MODULE ${IECOREVDB_CXX_MODULE_FILES} ${IECOREVDB_H_MODULE_FILES} )
        target_link_libraries( _IECoreVDB IECore IECorePython IECoreVDB IECoreScene ${BASE_LIBS} ${BASE_PYTHON_LIBS} )
        target_compile_options( _IECoreVDB PRIVATE )
        add_dependencies( _IECoreVDB IECoreVDB IECoreScene IECore IECorePython  )
        if ( WIN32 )
            set_target_properties( _IECoreVDB PROPERTIES SUFFIX ".pyd" )
        endif()
        install( TARGETS _IECoreVDB DESTINATION python/IECoreVDB )
        install( FILES ${IECOREVDB_PY_FILES} DESTINATION python/IECoreVDB )
    endif()
endif()

# IECoreArnold
if( ${WITH_IECORE_ARNOLD} )
    find_package( Arnold REQUIRED )
    include_directories( ${CORTEX_ARNOLD_INCLUDE_PATH} )
    include_directories( ${CORTEX_SOURCE_DIR}/contrib/IECoreArnold/include )
    # Library
    file( GLOB IECOREARNOLD_CXX_FILES ${CORTEX_SOURCE_DIR}/contrib/IECoreArnold/src/IECoreArnold/*.cpp )
    file( GLOB IECOREARNOLD_H_FILES ${CORTEX_SOURCE_DIR}/contrib/IECoreArnold/include/IECoreArnold/*.h ${CORTEX_SOURCE_DIR}/contrib/IECoreArnold/include/IECoreArnold/*.inl )
    file( GLOB IECOREARNOLD_PY_FILES ${CORTEX_SOURCE_DIR}/contrib/IECoreArnold/python/IECoreArnold/*.py )
    add_library( IECoreArnold SHARED ${IECOREARNOLD_CXX_FILES} ${IECOREARNOLD_H_FILES} )
    target_link_libraries( IECoreArnold IECore IECoreScene ${BASE_LIBS} ${CORTEX_ARNOLD_LIBS} )
    add_dependencies( IECoreArnold IECore )
    install( TARGETS IECoreArnold DESTINATION lib/ )
    install( FILES ${IECOREARNOLD_H_FILES} DESTINATION include/IECoreArnold )

    if( ${WITH_IECORE_PYTHON} )
        # Module/Bindings
        file( GLOB IECOREARNOLD_CXX_MODULE_FILES  ${CORTEX_SOURCE_DIR}/contrib/IECoreArnold/src/IECoreArnold/bindings/*.cpp )
        file( GLOB IECOREARNOLD_H_MODULE_FILES  ${CORTEX_SOURCE_DIR}/contrib/IECoreArnold/include/IECoreArnold/bindings/*.h )
        add_library( _IECoreArnold MODULE ${IECOREARNOLD_CXX_MODULE_FILES} ${IECOREARNOLD_H_MODULE_FILES} )
        target_link_libraries( _IECoreArnold IECore ${BASE_LIBS} ${BASE_PYTHON_LIBS} IECoreScene IECoreArnold )
        add_dependencies( _IECoreArnold IECore IECorePython IECoreArnold )
        if ( WIN32 )
            set_target_properties( _IECoreArnold PROPERTIES SUFFIX ".pyd" )
        endif()
        install( TARGETS _IECoreArnold DESTINATION python/IECoreArnold )
        install( FILES ${IECOREARNOLD_PY_FILES} DESTINATION python/IECoreArnold )
    endif()

    # Display driver
    add_library( arnoldOutputDriver MODULE ${CORTEX_SOURCE_DIR}/contrib/IECoreArnold/src/IECoreArnold/outputDriver/OutputDriver.cpp )
    target_link_libraries( arnoldOutputDriver IECore IECoreImage IECoreArnold )
    add_dependencies( arnoldOutputDriver IECore IECoreImage IECoreArnold )
    set_target_properties( arnoldOutputDriver PROPERTIES OUTPUT_NAME ieOutputDriver )
    set_target_properties( arnoldOutputDriver PROPERTIES PREFIX "" )
    install( TARGETS arnoldOutputDriver DESTINATION arnold/plugins )
endif()

# IECoreAppleseed
if( ${WITH_IECORE_APPLESEED} )
    find_package( Appleseed REQUIRED )
    include_directories( ${APPLESEED_INCLUDE_DIRS} )
    include_directories( ${CORTEX_SOURCE_DIR}/contrib/IECoreAppleseed/include )
    # Library
    file( GLOB IECOREAPPLESEED_CXX_FILES ${CORTEX_SOURCE_DIR}/contrib/IECoreAppleseed/src/IECoreAppleseed/*.cpp )
    file( GLOB IECOREAPPLESEED_H_FILES ${CORTEX_SOURCE_DIR}/contrib/IECoreAppleseed/include/IECoreAppleseed/*.h ${CORTEX_SOURCE_DIR}/contrib/IECoreAppleseed/include/IECoreAppleseed/*.inl )
    file( GLOB IECOREAPPLESEED_PY_FILES ${CORTEX_SOURCE_DIR}/contrib/IECoreAppleseed/python/IECoreAppleseed/*.py )
    add_library( IECoreAppleseed SHARED ${IECOREAPPLESEED_CXX_FILES} ${IECOREAPPLESEED_H_FILES} )
    target_link_libraries( IECoreAppleseed IECore IECoreScene ${BASE_LIBS} ${APPLESEED_LIBRARY} )
    add_dependencies( IECoreAppleseed IECore )
    target_compile_options( IECoreAppleseed PRIVATE -DAPPLESEED_ENABLE_IMATH_INTEROP -DAPPLESEED_WITH_OIIO -DAPPLESEED_WITH_OSL -DAPPLESEED_USE_SSE )
    install( TARGETS IECoreAppleseed DESTINATION lib/ )
    install( FILES ${IECOREAPPLESEED_H_FILES} DESTINATION include/IECoreAppleseed )

    if( ${WITH_IECORE_PYTHON} )
        # Module/Bindings
        file( GLOB IECOREAPPLESEED_CXX_MODULE_FILES ${CORTEX_SOURCE_DIR}/contrib/IECoreAppleseed/src/IECoreAppleseed/bindings/*.cpp )
        file( GLOB IECOREAPPLESEED_H_MODULE_FILES ${CORTEX_SOURCE_DIR}/contrib/IECoreAppleseed/include/IECoreAppleseed/bindings/*.h )
        add_library( _IECoreAppleseed MODULE ${IECOREAPPLESEED_CXX_MODULE_FILES} ${IECOREAPPLESEED_H_MODULE_FILES} )
        target_link_libraries( _IECoreAppleseed IECore ${BASE_LIBS} ${BASE_PYTHON_LIBS} IECoreScene IECoreAppleseed )
        add_dependencies( _IECoreAppleseed IECore IECorePython IECoreAppleseed )
        target_compile_options( _IECoreAppleseed PRIVATE -DAPPLESEED_ENABLE_IMATH_INTEROP -DAPPLESEED_WITH_OIIO -DAPPLESEED_WITH_OSL -DAPPLESEED_USE_SSE )
        if ( WIN32 )
            set_target_properties( _IECoreAppleseed PROPERTIES SUFFIX ".pyd" )
        endif()
        install( TARGETS _IECoreAppleseed DESTINATION python/IECoreAppleseed )
        install( FILES ${IECOREAPPLESEED_PY_FILES} DESTINATION python/IECoreAppleseed )
    endif()

    # Display driver
    add_library( appleseedOutputDriver MODULE ${CORTEX_SOURCE_DIR}/contrib/IECoreAppleseed/src/IECoreAppleseed/outputDriver/DisplayTileCallback.cpp )
    target_link_libraries( appleseedOutputDriver IECore IECoreImage IECoreAppleseed )
    add_dependencies( appleseedOutputDriver IECore IECoreImage IECoreAppleseed )
    target_compile_options( appleseedOutputDriver PRIVATE -DAPPLESEED_ENABLE_IMATH_INTEROP -DAPPLESEED_WITH_OIIO -DAPPLESEED_WITH_OSL -DAPPLESEED_USE_SSE )
    set_target_properties( appleseedOutputDriver PROPERTIES OUTPUT_NAME ieDisplay )
    set_target_properties( appleseedOutputDriver PROPERTIES PREFIX "" )
    install( TARGETS appleseedOutputDriver DESTINATION appleseedDisplays )
endif()

# IECoreUSD
if( ${WITH_IECORE_USD} )
    find_package( USD REQUIRED )
    include_directories( ${CORTEX_SOURCE_DIR}/contrib/IECoreUSD/include )
    include_directories( ${USD_INCLUDE_DIR} )
    link_directories( ${USD_LIBRARY_DIR} )
    # Library
    file( GLOB IECOREUSD_CXX_FILES  ${CORTEX_SOURCE_DIR}/contrib/IECoreUSD/src/IECoreUSD/*.cpp )
    file( GLOB IECOREUSD_H_FILES  ${CORTEX_SOURCE_DIR}/contrib/IECoreUSD/include/IECoreUSD/*.h ${CORTEX_SOURCE_DIR}/contrib/IECoreUSD/include/IECoreUSD/*.inl )
    file( GLOB IECOREUSD_PY_FILES  ${CORTEX_SOURCE_DIR}/contrib/IECoreUSD/python/IECoreUSD/*.py )
    add_library( IECoreUSD SHARED ${IECOREUSD_CXX_FILES} ${IECOREUSD_H_FILES} )
    target_link_libraries( IECoreUSD IECore IECoreScene ${BASE_LIBS} ${USD_LIBRARIES} usdGeom )
    add_dependencies( IECoreUSD IECore )
    install( TARGETS IECoreUSD DESTINATION lib/ )
    install( FILES ${IECOREUSD_H_FILES} DESTINATION include/IECoreUSD )

    if( ${WITH_IECORE_PYTHON} )
        # Module/Bindings
        file( GLOB IECOREUSD_CXX_MODULE_FILES  ${CORTEX_SOURCE_DIR}/contrib/IECoreUSD/src/IECoreUSD/bindings/*.cpp )
        file( GLOB IECOREUSD_H_MODULE_FILES  ${CORTEX_SOURCE_DIR}/contrib/IECoreUSD/include/IECoreUSD/bindings/*.h )
        add_library( _IECoreUSD SHARED ${IECOREUSD_CXX_MODULE_FILES} ${IECOREUSD_H_MODULE_FILES} )
        target_link_libraries( _IECoreUSD IECore IECoreScene IECoreUSD ${BASE_LIBS} ${BASE_PYTHON_LIBS} )
        add_dependencies( _IECoreUSD IECore IECorePython IECoreUSD )
        if ( WIN32 )
            set_target_properties( _IECoreUSD PROPERTIES SUFFIX ".pyd" )
        endif()
        install( TARGETS _IECoreUSD DESTINATION python/IECoreUSD )
        install( FILES ${IECOREUSD_PY_FILES} DESTINATION python/IECoreUSD )
    endif()
endif()
