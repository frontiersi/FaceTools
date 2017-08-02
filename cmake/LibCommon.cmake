set( CMAKE_COLOR_MAKEFILE TRUE)
set( CMAKE_VERBOSE_MAKEFILE FALSE)

#set( CMAKE_CXX_FLAGS "-Wno-deprecated -Wno-deprecated-declarations -Wno-error=unknown-pragmas")

set( LIB_PRE_REQS "$ENV{INSTALL_PARENT_DIR}" CACHE PATH
    "Where library prerequisites are installed (if not in the standard system library locations).")
set( CMAKE_LIBRARY_PATH "${LIB_PRE_REQS}")
set( CMAKE_PREFIX_PATH "${LIB_PRE_REQS}")

set( BUILD_SHARED_LIBS TRUE)
set( BUILD_USING_SHARED_LIBS TRUE)


# Set IS_DEBUG and _dsuffix
set( IS_DEBUG FALSE)
set( _dsuffix "")
string( TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
if( CMAKE_BUILD_TYPE_LOWER STREQUAL "debug")
    set( IS_DEBUG TRUE)
    set(_dsuffix "d")
endif()
unset( CMAKE_BUILD_TYPE_LOWER)
set( CMAKE_DEBUG_POSTFIX ${_dsuffix})


if(WITH_TESTUTILS)
    set( TestUtils_DIR "${LIB_PRE_REQS}/TestUtils/cmake")
    find_package( TestUtils REQUIRED)
    include_directories( ${TestUtils_INCLUDE_DIRS})
    link_directories( ${TestUtils_LIBRARY_DIR})
    link_libraries( ${TestUtils_LIBRARIES})
    set(WITH_FACETOOLS TRUE)
    set(WITH_RMODELIO TRUE)
endif()

if(WITH_FACETOOLS)
    set( FaceTools_DIR "${LIB_PRE_REQS}/FaceTools/cmake")
    find_package( FaceTools REQUIRED)
    include_directories( ${FaceTools_INCLUDE_DIRS})
    link_directories( ${FaceTools_LIBRARY_DIR})
    link_libraries( ${FaceTools_LIBRARIES})
    set(WITH_CGAL TRUE)
    set(WITH_DLIB TRUE)
    set(WITH_QTOOLS TRUE)
endif()

if(WITH_QTOOLS)
    set( QTools_DIR "${LIB_PRE_REQS}/QTools/cmake")
    find_package( QTools REQUIRED)
    include_directories( ${QTools_INCLUDE_DIRS})
    link_directories( ${QTools_LIBRARY_DIR})
    link_libraries( ${QTools_LIBRARIES})
    set(WITH_QT TRUE)
    set(WITH_RVTK TRUE)
endif()

if(WITH_RVTK)
    set( rVTK_DIR "${LIB_PRE_REQS}/rVTK/cmake")
    find_package( rVTK REQUIRED)
    include_directories( ${rVTK_INCLUDE_DIRS})
    link_directories( ${rVTK_LIBRARY_DIR})
    link_libraries( ${rVTK_LIBRARIES})
    set(WITH_VTK TRUE)
    set(WITH_RFEATURES TRUE)
endif()

if(WITH_RMODELIO)
    set( rModelIO_DIR "${LIB_PRE_REQS}/rModelIO/cmake")
    find_package( rModelIO REQUIRED)
    include_directories( ${rModelIO_INCLUDE_DIRS})
    link_directories( ${rModelIO_LIBRARY_DIR})
    link_libraries( ${rModelIO_LIBRARIES})
    set(WITH_ASSIMP TRUE)
    set(WITH_LIBLAS FALSE)
    set(WITH_RFEATURES TRUE)
endif()

if(WITH_RPASCALVOC)
    set( rPascalVOC_DIR "${LIB_PRE_REQS}/rPascalVOC/cmake")
    find_package( rPascalVOC REQUIRED)
    include_directories( ${rPascalVOC_INCLUDE_DIRS})
    link_directories( ${rPascalVOC_LIBRARY_DIR})
    link_libraries( ${rPascalVOC_LIBRARIES})
    set(WITH_TINYXML TRUE)
    set(WITH_RLEARNING TRUE)
endif()

if(WITH_RLEARNING)
    set( rLearning_DIR "${LIB_PRE_REQS}/rLearning/cmake")
    find_package( rLearning REQUIRED)
    include_directories( ${rLearning_INCLUDE_DIRS})
    link_directories( ${rLearning_LIBRARY_DIR})
    link_libraries( ${rLearning_LIBRARIES})
    set(WITH_RFEATURES TRUE)
endif()

if(WITH_RFEATURES)
    set( rFeatures_DIR "${LIB_PRE_REQS}/rFeatures/cmake")
    find_package( rFeatures REQUIRED)
    include_directories( ${rFeatures_INCLUDE_DIRS})
    link_directories( ${rFeatures_LIBRARY_DIR})
    link_libraries( ${rFeatures_LIBRARIES})
    set(WITH_OPENCV TRUE)
    set(WITH_RLIB TRUE)
endif()

if(WITH_RLIB)
    set( rlib_DIR "${LIB_PRE_REQS}/rlib/cmake")
    find_package( rlib REQUIRED)
    include_directories( ${rlib_INCLUDE_DIRS})
    link_directories( ${rlib_LIBRARY_DIR})
    link_libraries( ${rlib_LIBRARIES})
    set(WITH_BOOST TRUE)
    set(WITH_EIGEN TRUE)
endif()



if(WITH_ASSIMP)     # AssImp
    set( ASSIMP_DIR "${LIB_PRE_REQS}/AssImp/lib/cmake/assimp-3.3")
    find_package( ASSIMP REQUIRED)
    include_directories( ${ASSIMP_INCLUDE_DIRS})
    link_directories( ${ASSIMP_LIBRARY_DIRS})
    link_libraries( ${ASSIMP_LIBRARIES})
endif()

if(WITH_LIBLAS)     # libLAS
    set( LibLAS_DIR "${LIB_PRE_REQS}/libLAS-1.7.0")
    find_package( LibLAS REQUIRED)
    include_directories( ${LibLAS_INCLUDE_DIR})
endif()

if(WITH_TINYXML)    # tinyxml
    set( tinyxml_DIR "${LIB_PRE_REQS}/tinyxml/cmake")
    find_package( tinyxml REQUIRED)
    include_directories( ${tinyxml_INCLUDE_DIR})
    link_directories( ${tinyxml_LIBRARY_DIR})
    link_libraries( ${tinyxml_LIBRARY})
endif()


macro( get_boost_msvc_library libname)
    set( ${libname} "lib64")
    if(MSVC)
        if( MSVC12)
            set(MSVC_PREFIX "msvc-12.0")
        elseif( MSVC14)
            set(MSVC_PREFIX "msvc-14.0")
        elseif( MSVC15)
            set(MSVC_PREFIX "msvc-15.0")
        endif()
        set( ${libname} "lib64-${MSVC_PREFIX}")
    endif(MSVC)
endmacro( get_boost_msvc_library)


if(WITH_BOOST)  # Boost
    if(WIN32)
        set( BOOST_ROOT "${LIB_PRE_REQS}/boost_1_59_0")
        get_boost_msvc_library( _boostLibName)
        set( BOOST_LIBRARYDIR "${BOOST_ROOT}/${_boostLibName}")
        set( Boost_NO_SYSTEM_PATHS TRUE)  # Don't search for Boost anywhere other than the above hints
        set( Boost_USE_STATIC_LIBS OFF CACHE BOOL "use static Boost libraries")
        set( Boost_USE_MULTITHREADED ON)
        set( Boost_USE_STATIC_RUNTIME OFF)
        add_definitions( -DBOOST_ALL_NO_LIB)    # Disable autolinking
        add_definitions( -DBOOST_ALL_DYN_LINK)  # Force dynamic linking (probably don't need this)
    endif()
    find_package( Boost 1.53 REQUIRED COMPONENTS system filesystem thread random regex)
    include_directories( ${Boost_INCLUDE_DIRS})
    add_definitions( ${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
    link_directories( ${Boost_LIBRARY_DIR})
    link_libraries( ${Boost_LIBRARIES})
endif()

if(WITH_EIGEN) # Eigen3
    if(WIN32)
        set( EIGEN3_INCLUDE_DIR "${LIB_PRE_REQS}/eigen3/include/eigen3")
    else()
        find_package( Eigen3 REQUIRED)
    endif()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif()

if(WITH_OPENCV) # OpenCV
    set( OpenCV_DIR "${LIB_PRE_REQS}/opencv")
    find_package( OpenCV 2.4 REQUIRED)
    include_directories( ${OpenCV_INCLUDE_DIRS})
    link_libraries( ${OpenCV_LIBS})
endif()

if(WITH_VTK)    # VTK
    set( VTK_VER 7.1)
    if (IS_DEBUG)
        set( VTK_DIR "${LIB_PRE_REQS}/VTK/debug/lib/cmake/vtk-${VTK_VER}/")
    else()
        set( VTK_DIR "${LIB_PRE_REQS}/VTK/release/lib/cmake/vtk-${VTK_VER}/")
    endif()
    find_package( VTK REQUIRED)
    include( ${VTK_USE_FILE})
    link_libraries( ${VTK_LIBRARIES})
endif()

if(WITH_QT)     # Qt5
    find_package( Qt5 REQUIRED Widgets Sql)
    include_directories( ${Qt5Widgets_INCLUDE_DIRS})
    add_definitions( ${Qt5Widgets_DEFINITIONS})
    add_definitions( ${Qt5Sql_DEFINITIONS})
    set( QT_LIBRARIES Qt5::Widgets Qt5::Sql)
    link_libraries( ${QT_LIBRARIES})
endif()

if(WITH_CGAL)   # CGAL
    set( CGAL_DIR "${LIB_PRE_REQS}/CGAL/lib/CGAL")
    find_package( CGAL COMPONENTS Core)
    set( CGAL_DONT_OVERRIDE_CMAKE_FLAGS TRUE CACHE BOOL "Prevent CGAL from overwritting CMake flags.")
    include( ${CGAL_USE_FILE})
    link_libraries( ${CGAL_LIBRARIES})
endif()

if(WITH_DLIB)   # dlib
    if(IS_DEBUG)
        set( dlib_DIR "${LIB_PRE_REQS}/dlib/debug/lib/cmake/dlib")
    else()
        set( dlib_DIR "${LIB_PRE_REQS}/dlib/release/lib/cmake/dlib")
    endif()
    find_package( dlib REQUIRED)
    include_directories( ${dlib_INCLUDE_DIRS})
    link_libraries( ${dlib_LIBRARIES})
endif()


