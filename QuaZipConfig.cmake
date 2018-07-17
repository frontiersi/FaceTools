# =============================================================================
# The QuaZip CMake configuration file.
#
#           ** File generated automatically, DO NOT MODIFY! ***

# To use from an external project, in your project's CMakeLists.txt add:
#   FIND_PACKAGE( QuaZip REQUIRED)
#   INCLUDE_DIRECTORIES( QuaZip ${QuaZip_INCLUDE_DIRS})
#   LINK_DIRECTORIES( ${QuaZip_LIBRARY_DIR})
#   TARGET_LINK_LIBRARIES( MY_TARGET_NAME ${QuaZip_LIBRARIES})
#
# This module defines the following variables:
#   - QuaZip_FOUND         : True if QuaZip is found.
#   - QuaZip_ROOT_DIR      : The root directory where QuaZip is installed.
#   - QuaZip_INCLUDE_DIRS  : The QuaZip include directories.
#   - QuaZip_LIBRARY_DIR   : The QuaZip library directory.
#   - QuaZip_LIBRARIES     : The QuaZip imported libraries to link to.
#
# =============================================================================

get_filename_component( QuaZip_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component( QuaZip_ROOT_DIR  "${QuaZip_CMAKE_DIR}"        PATH)

set( QuaZip_INCLUDE_DIRS "${QuaZip_ROOT_DIR}/include" CACHE PATH "The QuaZip include directories.")
set( QuaZip_LIBRARY_DIR  "${QuaZip_ROOT_DIR}/lib"     CACHE PATH "The QuaZip library directory.")

set( _hints quazip5 libquazip5)
find_library( QuaZip_LIBRARIES NAMES ${_hints} PATHS "${QuaZip_LIBRARY_DIR}")
set( QuaZip_LIBRARIES ${QuaZip_LIBRARIES}  CACHE FILE "The QuaZip imported libraries to link to.")

# handle QUIETLY and REQUIRED args and set QuaZip_FOUND to TRUE if all listed variables are TRUE
include( "${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake")
find_package_handle_standard_args( QuaZip "Found:\t${QuaZip_LIBRARIES}" QuaZip_LIBRARIES QuaZip_INCLUDE_DIRS)
