set( CMAKE_COLOR_MAKEFILE TRUE)
set( CMAKE_VERBOSE_MAKEFILE FALSE)

if(UNIX)
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-deprecated -Wno-deprecated-declarations -Wno-error=unknown-pragmas")
endif()
set(CMAKE_CXX_STANDARD 14)

set( LIB_PRE_REQS "$ENV{INSTALL_PARENT_DIR}" CACHE PATH
    "Where library prerequisites are installed (if not in the standard system library locations).")
set( CMAKE_LIBRARY_PATH "${LIB_PRE_REQS}")
set( CMAKE_PREFIX_PATH "${LIB_PRE_REQS}")

set( BUILD_SHARED_LIBS TRUE CACHE BOOL "Dynamic Linking?" FORCE)
set( CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set( BUILD_USING_SHARED_LIBS TRUE)

# Set IS_DEBUG and _dsuffix
set( IS_DEBUG FALSE)
set( _dsuffix "")
string( TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
if( CMAKE_BUILD_TYPE_LOWER STREQUAL "debug")
    set( IS_DEBUG TRUE)
    set(_dsuffix "d")
endif()
set( CMAKE_DEBUG_POSTFIX ${_dsuffix})

# For Valloric YouCompleteMe (VIM plugin) for clangd-completer
set( CMAKE_EXPORT_COMPILE_COMMANDS ON)


macro( get_msvc_version _ver)
    set( ${_ver} "")
    if(MSVC)
        if( MSVC70 OR MSVC71 )
            set(_msvcv "7")
        elseif( MSVC80 )
            set(_msvcv "8")
        elseif( MSVC90 )
            set(_msvcv "9")
        elseif( MSVC10 )
            set(_msvcv "10")
        elseif( MSVC11 )
            set(_msvcv "11")
        elseif( MSVC12 )
            set(_msvcv "12")
        elseif( MSVC14 )
            set(_msvcv "14")
        else()
            set(_msvcv "15")
        endif()
        set( ${_ver} "${_msvcv}")
    endif(MSVC)
endmacro( get_msvc_version)


# Add definitions for Windows 7 build target (SDK 8.1 support Win 7 and up)
if(WIN32)
    add_definitions( -DWINVER=0x0601)
    add_definitions( -D_WIN32_WINNT=0x0601)
    set( CMAKE_SYSTEM_VERSION 8.1)
endif()
message( STATUS "OS Name build target:    ${CMAKE_SYSTEM_NAME}")
message( STATUS "OS Version build target: ${CMAKE_SYSTEM_VERSION}")

set( _cmake_dir "lib/cmake")


if(WITH_TESTUTILS)
    set( TestUtils_DIR "${LIB_PRE_REQS}/TestUtils/${_cmake_dir}" CACHE PATH "Location of TestUtilsConfig.cmake")
    find_package( TestUtils REQUIRED)
    include_directories( ${TestUtils_INCLUDE_DIRS})
    link_directories( ${TestUtils_LIBRARY_DIR})
    set(WITH_FACETOOLS TRUE)
    message( STATUS "TestUtils:  ${TestUtils_LIBRARIES}")
    set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${TestUtils_LIBRARY_DIR})
endif()

if(WITH_FACETOOLS)
    set( FaceTools_DIR "${LIB_PRE_REQS}/FaceTools/${_cmake_dir}" CACHE PATH "Location of FaceToolsConfig.cmake")
    find_package( FaceTools REQUIRED)
    include_directories( ${FaceTools_INCLUDE_DIRS})
    link_directories( ${FaceTools_LIBRARY_DIR})
    set(WITH_LUA TRUE)
    set(WITH_QUAZIP TRUE)
    #set(WITH_CPD TRUE)
    #set(WITH_CGAL TRUE)
    set(WITH_DLIB TRUE)
    set(WITH_QTOOLS TRUE)
    set(WITH_RMODELIO TRUE)
    message( STATUS "FaceTools:  ${FaceTools_LIBRARIES}")
    set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${FaceTools_LIBRARY_DIR})
endif()

if(WITH_QTOOLS)
    set( QTools_DIR "${LIB_PRE_REQS}/QTools/${_cmake_dir}" CACHE PATH "Location of QToolsConfig.cmake")
    find_package( QTools REQUIRED)
    include_directories( ${QTools_INCLUDE_DIRS})
    link_directories( ${QTools_LIBRARY_DIR})
    set(WITH_QT TRUE)
    set(WITH_RVTK TRUE)
    message( STATUS "QTools:     ${QTools_LIBRARIES}")
    set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${QTools_LIBRARY_DIR})
endif()

if(WITH_RVTK)
    set( rVTK_DIR "${LIB_PRE_REQS}/rVTK/${_cmake_dir}" CACHE PATH "Location of rVTKConfig.cmake")
    find_package( rVTK REQUIRED)
    include_directories( ${rVTK_INCLUDE_DIRS})
    link_directories( ${rVTK_LIBRARY_DIR})
    set(WITH_RFEATURES TRUE)
    set(WITH_VTK TRUE)
    message( STATUS "rVTK:       ${rVTK_LIBRARIES}")
    set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${rVTK_LIBRARY_DIR})
endif()

if(WITH_RPASCALVOC)
    set( rPascalVOC_DIR "${LIB_PRE_REQS}/rPascalVOC/${_cmake_dir}" CACHE PATH "Location of rPascalVOCConfig.cmake")
    find_package( rPascalVOC REQUIRED)
    include_directories( ${rPascalVOC_INCLUDE_DIRS})
    link_directories( ${rPascalVOC_LIBRARY_DIR})
    set(WITH_TINYXML TRUE)
    set(WITH_RLEARNING TRUE)
    message( STATUS "rPascalVOC: ${rPascalVOC_LIBRARIES}")
    set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${rPascalVOC_LIBRARY_DIR})
endif()

if(WITH_RLEARNING)
    set( rLearning_DIR "${LIB_PRE_REQS}/rLearning/${_cmake_dir}" CACHE PATH "Location of rLearningConfig.cmake")
    find_package( rLearning REQUIRED)
    include_directories( ${rLearning_INCLUDE_DIRS})
    link_directories( ${rLearning_LIBRARY_DIR})
    set(WITH_RFEATURES TRUE)
    message( STATUS "rLearning:  ${rLearning_LIBRARIES}")
    set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${rLearning_LIBRARY_DIR})
endif()

if(WITH_RMODELIO)
    set( rModelIO_DIR "${LIB_PRE_REQS}/rModelIO/${_cmake_dir}" CACHE PATH "Location of rModelIOConfig.cmake")
    find_package( rModelIO REQUIRED)
    include_directories( ${rModelIO_INCLUDE_DIRS})
    link_directories( ${rModelIO_LIBRARY_DIR})
    #set(WITH_VCG TRUE)  
    set(WITH_RFEATURES TRUE)
    set(WITH_ASSIMP TRUE)
    set(WITH_LIBLAS FALSE)
    message( STATUS "rModelIO:   ${rModelIO_LIBRARIES}")
    set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${rModelIO_LIBRARY_DIR})
endif()

# RModelIO::U3DExporter requires IDTFConverter
if(WITH_IDTF_CONVERTER)
    set( idtfconv_exe "$ENV{IDTF_CONVERTER_EXE}")   # Set location of IDTFConverter from env var if set
    if( "${idtfconv_exe}" STREQUAL "")              # If not set, try to guess based on platform
        set( idtfconv_exe "${LIB_PRE_REQS}/u3dIntel/bin/IDTFConverter")
    endif()

    if ( NOT EXISTS ${idtfconv_exe})
        set( idtfconv_exe "rModelIO_IDTF_CONVERTER-NOTFOUND")
    endif()

    set( rModelIO_IDTF_CONVERTER ${idtfconv_exe} CACHE FILEPATH "Location of the IDTFConverter executable.")
    if ( EXISTS ${rModelIO_IDTF_CONVERTER})
        message( STATUS "[rModelIO] RModelIO::U3DExporter using IDTFConverter at ${rModelIO_IDTF_CONVERTER}")
        add_definitions( -DIDTF_CONVERTER=\"${rModelIO_IDTF_CONVERTER}\")
    else()
        message( STATUS "[rModelIO] IDTFConverter not found; RModelIO::U3DExporter disabled.")
    endif()
endif()

if(WITH_RFEATURES)
    set( rFeatures_DIR "${LIB_PRE_REQS}/rFeatures/${_cmake_dir}" CACHE PATH "Location of rFeaturesConfig.cmake")
    find_package( rFeatures REQUIRED)
    include_directories( ${rFeatures_INCLUDE_DIRS})
    link_directories( ${rFeatures_LIBRARY_DIR})
    set(WITH_RLIB TRUE)
    set(WITH_OPENCV TRUE)
    message( STATUS "rFeatures:  ${rFeatures_LIBRARIES}")
    set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${rFeatures_LIBRARY_DIR})
endif()

if(WITH_RLIB)
    set( rlib_DIR "${LIB_PRE_REQS}/rlib/${_cmake_dir}" CACHE PATH "Location of rlibConfig.cmake")
    find_package( rlib REQUIRED)
    include_directories( ${rlib_INCLUDE_DIRS})
    link_directories( ${rlib_LIBRARY_DIR})
    set(WITH_BOOST TRUE)
    set(WITH_EIGEN TRUE)
    message( STATUS "rlib:       ${rlib_LIBRARIES}")
    set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${rlib_LIBRARY_DIR})
endif()


if(WITH_LIBICP) # The ICP source library (Andreas Geiger) (https://github.com/symao/libicp)
    set( LIBICP_DIR "${LIB_PRE_REQS}/libICP" CACHE PATH "Location of libICP source files.")
    set( LIBICP_SRC "${LIBICP_DIR}/src")
    set( LIBICP_SRC_FILES
        "${LIBICP_SRC}/icp.cpp"
        "${LIBICP_SRC}/icpPointToPlane.cpp"
        "${LIBICP_SRC}/icpPointToPoint.cpp"
        "${LIBICP_SRC}/kdtree.cpp"
        "${LIBICP_SRC}/matrix.cpp"
        )
    if (NOT IS_DIRECTORY ${LIBICP_SRC})
        message( FATAL_ERROR "Can't find libICP source files!")
    endif()
    message( STATUS "LibICP:     ${LIBICP_SRC}/*")
    include_directories( "${LIBICP_SRC}")
endif()


if(WITH_QUAZIP)     # QuaZIP
    set(QuaZip_DIR "${LIB_PRE_REQS}/quazip/cmake" CACHE PATH "Location of QuaZip (Qt/C++ wrapper for Minizip)")
    find_package( QuaZip REQUIRED)
    include_directories( ${QuaZip_INCLUDE_DIRS})
    link_directories( ${QuaZip_LIBRARY_DIR})
    set(WITH_QT TRUE)
    if (WIN32)
        set(WITH_ZLIB TRUE)
    endif()
    message( STATUS "QuaZip:     ${QuaZip_LIBRARIES}")
    set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${QuaZip_LIBRARY_DIR})
endif()


if(WITH_ASSIMP)     # AssImp
    set( ASSIMP_DIR "${LIB_PRE_REQS}/AssImp/lib/cmake/assimp-4.1" CACHE PATH "Location of assimp-config.cmake")
    find_package( ASSIMP REQUIRED)
    include_directories( ${ASSIMP_INCLUDE_DIRS})
    link_directories( ${ASSIMP_LIBRARY_DIRS})
    if (WIN32)
        set(WITH_ZLIB TRUE)
    endif()
    message( STATUS "AssImp      ${ASSIMP_LIBRARY_DIRS}/${ASSIMP_LIBRARIES}")
    set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${ASSIMP_LIBRARY_DIRS})
endif()


# Link to separately compiled zlib on Windows.
if(WITH_ZLIB)
    set( ZLIB_DIR "${LIB_PRE_REQS}/zlib" CACHE PATH "Location of zlib")
    set( ZLIB_INCLUDE_DIR "${ZLIB_DIR}/include")
    set( ZLIB_LIBRARY_DIR "${ZLIB_DIR}/lib")
    set( ZLIB_LIBRARIES "${ZLIB_LIBRARY_DIR}/zlibstatic${_dsuffix}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    set( ZLIB_LIBRARY "${ZLIB_DIR}/bin/zlib${_dsuffix}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    if ( NOT EXISTS ${ZLIB_LIBRARY})
        message( FATAL_ERROR "Can't find zlib library at ${ZLIB_LIBRARY}!")
    endif()
    include_directories( "${ZLIB_INCLUDE_DIR}")
    #link_directories( "${ZLIB_LIBRARY_DIR}")
    message( STATUS "Zlib:       ${ZLIB_LIBRARIES}")
    #set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${ZLIB_LIBRARY_DIR})
endif()


if(WITH_LUA)
    set( LUA_DIR "${LIB_PRE_REQS}/lua5" CACHE PATH "Location of Lua")
    set( LUA_INCLUDE_DIRS "${LUA_DIR}/include")
    set( LUA_LIBRARY_DIR  "${LUA_DIR}/lib")
    set( LUA_LIBRARY "${LUA_DIR}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}lua53${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set( LUA_LIBRARIES "${LUA_LIBRARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}lua53${CMAKE_STATIC_LIBRARY_SUFFIX}")
    if(UNIX)
        set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${LUA_LIBRARY_DIR})
    endif()
    set( SOL_HPP "${LUA_INCLUDE_DIRS}/sol.hpp")

    if(NOT EXISTS ${LUA_LIBRARY})
        message( FATAL_ERROR "Can't find Lua library at ${LUA_LIBRARY}!")
    endif()

    if(NOT EXISTS ${LUA_LIBRARIES})
        message( FATAL_ERROR "Can't find Lua library at ${LUA_LIBRARIES}!")
    endif()

    if(NOT EXISTS "${SOL_HPP}")
        message( FATAL_ERROR "Can't find ${SOL_HPP}!")
    endif()

    include_directories( "${LUA_INCLUDE_DIRS}")
    message( STATUS "Lua:        ${LUA_LIBRARIES}")
    message( STATUS "sol.hpp:    ${SOL_HPP}")
endif()


if(WITH_DLIB)   # dlib
    set( dlib_ROOT "${LIB_PRE_REQS}/dlib" CACHE PATH "Location of dlib")
    set( dlib_DIR "${dlib_ROOT}/${CMAKE_BUILD_TYPE_LOWER}/lib/cmake/dlib" CACHE PATH "Location of dlibConfig.cmake")
    find_package( dlib REQUIRED)
    get_filename_component( dlib_LIBRARY_DIR "${dlib_DIR}/../.." REALPATH)
    set( dlib_LIBRARY "${dlib_LIBRARY_DIR}/dlib.lib")
    if(UNIX)
        set( dlib_LIBRARY "${dlib_LIBRARY_DIR}/libdlib.so.${dlib_VERSION}")
        set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${dlib_LIBRARY_DIR})
    endif()
    if(NOT EXISTS ${dlib_LIBRARY})
        message( FATAL_ERROR "Can't find dlib library at ${dlib_LIBRARY}!")
    endif()
    include_directories( ${dlib_INCLUDE_DIRS})
    message( STATUS "dlib:       ${dlib_LIBRARY}")
endif()


#if(WITH_LIBLAS)     # libLAS
#    set( LibLAS_DIR "${LIB_PRE_REQS}/libLAS-1.7.0" CACHE PATH "Location of LibLASConfig.cmake")
#    find_package( LibLAS REQUIRED)
#    include_directories( ${LibLAS_INCLUDE_DIR})
#endif()

if(WITH_TINYXML)    # tinyxml
    set( tinyxml_DIR "${LIB_PRE_REQS}/tinyxml/cmake" CACHE PATH "Location of tinyxmlConfig.cmake")
    find_package( tinyxml REQUIRED)
    include_directories( ${tinyxml_INCLUDE_DIR})
    link_directories( ${tinyxml_LIBRARY_DIR})
endif()


if(WITH_EIGEN) # Eigen3
    if(WIN32)
        set( EIGEN3_INCLUDE_DIR "${LIB_PRE_REQS}/eigen3/include/eigen3" CACHE PATH "Location of Eigen3 headers (Eigen/*.h) directory")
    elseif(UNIX)
        find_package( Eigen3 REQUIRED)
    endif()
    if( NOT IS_DIRECTORY ${EIGEN3_INCLUDE_DIR})
        message( FATAL_ERROR "Can't find Eigen3 headers!")
    endif()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif()


if(WITH_VTK)    # VTK
    set( VTK_VER 8.1)

    if( ${VTK_VER} EQUAL "8.1")
        set( VTK_BASE "${LIB_PRE_REQS}/VTK-8.1.1")
    elseif( ${VTK_VER} EQUAL "8.2")
        set( VTK_BASE "${LIB_PRE_REQS}/VTK-8.2.0")
    endif()

    set( VTK_DIR "${VTK_BASE}/${CMAKE_BUILD_TYPE_LOWER}/lib/cmake/vtk-${VTK_VER}" CACHE PATH "Location of VTKConfig.cmake")

    if(NOT IS_DIRECTORY ${VTK_DIR})
        message( FATAL_ERROR "Can't find VTK! Set VTK_VER correctly for the version of VTK you want to build against!")
    endif()

    if(UNIX)
        set( VTK_LIBRARY_DIR "${VTK_DIR}/../.." CACHE PATH "Location of VTK shared libraries")
    elseif(WIN32)
        set( VTK_BIN "${VTK_DIR}/../../../bin" CACHE PATH "Location of VTK shared libraries")
    endif()

    find_package( VTK REQUIRED)
    include( ${VTK_USE_FILE})
    set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${VTK_LIBRARY_DIR})
    message( STATUS "VTK:        ${VTK_DIR}")
endif()


if(WITH_QT)     # Qt5
    set( Qt5_DIR "$ENV{QT5}/lib/cmake/Qt5" CACHE PATH "Location of Qt5Config.cmake")
    if(NOT IS_DIRECTORY ${Qt5_DIR})
        message( FATAL_ERROR "Can't find Qt5! Set environment variable QT5 to the location of the library!")
    endif()
    get_filename_component( QT_INSTALLER_FRAMEWORK "$ENV{QT5}/../../Tools/QtInstallerFramework/3.1/bin" REALPATH)
    set( QT_INF_BINARY_CREATOR "${QT_INSTALLER_FRAMEWORK}/binarycreator${CMAKE_EXECUTABLE_SUFFIX}")
    set( QT_INF_REPO_GEN "${QT_INSTALLER_FRAMEWORK}/repogen${CMAKE_EXECUTABLE_SUFFIX}")

    #find_package( Qt5 CONFIG REQUIRED Core Widgets WebEngine Charts Sql Svg)
    find_package( Qt5 CONFIG REQUIRED Core Widgets Charts Sql Svg)
    include_directories( ${Qt5Core_INCLUDE_DIRS})
    include_directories( ${Qt5Widgets_INCLUDE_DIRS})
    #include_directories( ${Qt5WebEngine_INCLUDE_DIRS})
    include_directories( ${Qt5Charts_INCLUDE_DIRS})
    include_directories( ${Qt5Sql_INCLUDE_DIRS})
    include_directories( ${Qt5Svg_INCLUDE_DIRS})

    #set( QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::WebEngine Qt5::Charts Qt5::Sql Qt5::Svg)
    set( QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Charts Qt5::Sql Qt5::Svg)

    set( QT_LIB_DIR "${Qt5_DIR}/../..")
    set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${QT_LIB_DIR})

    message( STATUS "Qt5:        ${Qt5_DIR}")
endif()


if(WITH_OPENCV) # OpenCV
    if(WIN32)
        set( OpenCV_DIR "${LIB_PRE_REQS}/opencv" CACHE PATH "Location of OpenCVConfig.cmake")
        get_msvc_version( _msvcv)
        set( OpenCV_BIN "${OpenCV_DIR}/x64/vc${_msvcv}/bin" CACHE PATH "Location of OpenCV binary (dll) files")
    endif()
    find_package( OpenCV 3.2 REQUIRED)
    include_directories( ${OpenCV_INCLUDE_DIRS})
    message( STATUS "OpenCV:     ${OpenCV_DIR}")
endif()


if(WITH_BOOST)  # Boost
    set( BOOST_ROOT "${LIB_PRE_REQS}/boost_1_68_0" CACHE PATH "Location of boost")
    set( BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")

    if(WIN32)
        get_msvc_version( _msvcv)
        set( BOOST_LIBRARYDIR "${BOOST_ROOT}/lib64-msvc-${_msvcv}.0")
        add_definitions( ${Boost_LIB_DIAGNOSTIC_DEFINITIONS})   # Info about Boost's auto linking
    endif()

    set( Boost_NO_SYSTEM_PATHS TRUE)  # Don't search for Boost in system directories
    set( Boost_USE_STATIC_LIBS OFF CACHE BOOL "use static Boost libraries")
    set( Boost_USE_MULTITHREADED ON)
    set( Boost_USE_STATIC_RUNTIME OFF)
    add_definitions( -DBOOST_ALL_NO_LIB)    # Disable autolinking
    add_definitions( -DBOOST_ALL_DYN_LINK)  # Force dynamic linking (probably don't need this)
    add_definitions( -DBOOST_SYSTEM_NO_DEPRECATED)

    find_package( Boost 1.68 REQUIRED COMPONENTS regex random thread filesystem system)
    include_directories( ${Boost_INCLUDE_DIRS})
    set( Boost_LIBRARIES Boost::regex Boost::random Boost::thread Boost::filesystem Boost::system)

    #message( STATUS "Boost_VERSION: ${Boost_VERSION}")
    #message( STATUS "Boost_LIB_VERSION: ${Boost_LIB_VERSION}")
    #message( STATUS "Boost_MAJOR_VERSION: ${Boost_MAJOR_VERSION}")
    #message( STATUS "Boost_MINOR_VERSION: ${Boost_MINOR_VERSION}")
    #message( STATUS "Boost_SUBMINOR_VERSION: ${Boost_SUBMINOR_VERSION}")

    set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} ${BOOST_LIBRARYDIR})
endif()


if(WITH_CGAL)   # CGAL
    set( CGAL_DIR "${LIB_PRE_REQS}/CGAL/${CMAKE_BUILD_TYPE_LOWER}/lib/CGAL" CACHE PATH "Location of CGALConfig.cmake")
    set( CGAL_BIN "${CGAL_DIR}/../../bin" CACHE PATH "Location of CGAL binary (dll) files")
    find_package( CGAL COMPONENTS Core)
    set( CGAL_DONT_OVERRIDE_CMAKE_FLAGS TRUE CACHE BOOL "Prevent CGAL from overwritting CMake flags.")
    include( ${CGAL_USE_FILE})
endif()


#if(WITH_VCG)    # VCGLib (Visual and Computer Graphics Library)
#    set( VCG_DIR "${LIB_PRE_REQS}/vcglib" CACHE PATH "Location of VCG header only library.")
#    include_directories( ${VCG_DIR})    # Header only library
#endif()


if(WITH_CPD) # Coherent Point Drift
    set( Cpd_ROOT "${LIB_PRE_REQS}/cpd" CACHE PATH "Location of CPD (Coherent Point Drift)")
    set( Cpd_DIR "${Cpd_ROOT}/${CMAKE_BUILD_TYPE_LOWER}/lib/cmake/cpd" CACHE PATH "Location of cpd-config.cmake")
    find_package( Cpd REQUIRED)
    set( Cpd_INCLUDE_DIRS "${Cpd_DIR}/../../../include")
    include_directories( ${Cpd_INCLUDE_DIRS})
    message( STATUS "Cpd:        ${Cpd_DIR}")
endif()

