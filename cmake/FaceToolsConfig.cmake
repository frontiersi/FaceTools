# =============================================================================
# The FaceTools CMake configuration file.
#
#           ** File generated automatically, DO NOT MODIFY! ***

# To use from an external project, in your project's CMakeLists.txt add:
#   FIND_PACKAGE( FaceTools REQUIRED)
#   INCLUDE_DIRECTORIES( FaceTools ${FaceTools_INCLUDE_DIRS})
#   LINK_DIRECTORIES( ${FaceTools_LIBRARY_DIR})
#   TARGET_LINK_LIBRARIES( MY_TARGET_NAME ${FaceTools_LIBRARIES})
#
# This module defines the following variables:
#   - FaceTools_FOUND         : True if FaceTools is found.
#   - FaceTools_ROOT_DIR      : The root directory where FaceTools is installed.
#   - FaceTools_BIN_DIR       : The FaceTools bin directory.
#   - FaceTools_INCLUDE_DIRS  : The FaceTools include directories.
#   - FaceTools_LIBRARY_DIR   : The FaceTools library directory.
#   - FaceTools_LIBRARIES     : The FaceTools imported libraries to link to.
#
# =============================================================================

get_filename_component( FaceTools_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component( FaceTools_ROOT_DIR  "${FaceTools_CMAKE_DIR}"           PATH)

set( FaceTools_INCLUDE_DIRS "${FaceTools_ROOT_DIR}/../include" CACHE PATH "The FaceTools include directories.")
set( FaceTools_BIN_DIR      "${FaceTools_ROOT_DIR}/../bin"     CACHE PATH "The FaceTools bin directory.")
set( FaceTools_LIBRARY_DIR  "${FaceTools_ROOT_DIR}"            CACHE PATH "The FaceTools library directory.")

set( _prefix "")
set( _suffix "-vc${MSVC_TOOLSET_VERSION}.lib")
if(UNIX)
    set( _prefix "lib")
    if(BUILD_USING_SHARED_LIBS)
        set( _suffix ".so")
    else()
        set( _suffix ".a")
    endif()
endif()

set( _hint ${_prefix}FaceTools${_dsuffix}${_suffix})
find_library( FaceTools_LIBRARIES NAMES ${_hint} PATHS "${FaceTools_LIBRARY_DIR}/static" "${FaceTools_LIBRARY_DIR}")
set( FaceTools_LIBRARIES     ${FaceTools_LIBRARIES}         CACHE FILEPATH "The FaceTools imported libraries to link to.")

# handle QUIETLY and REQUIRED args and set FaceTools_FOUND to TRUE if all listed variables are TRUE
include( "${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake")
find_package_handle_standard_args( FaceTools FaceTools_FOUND FaceTools_LIBRARIES FaceTools_INCLUDE_DIRS)

