cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project( FaceTools)

set(WITH_RMODELIO TRUE)
set(WITH_CGAL TRUE)
set(WITH_DLIB TRUE)
include( "cmake/LibCommon.cmake")

set( INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set( SRC_DIR "${PROJECT_SOURCE_DIR}/src")
include_directories( ${INCLUDE_DIR})

set( INCLUDE_FILES
    "${INCLUDE_DIR}/${PROJECT_NAME}.h"
    "${INCLUDE_DIR}/CorrespondenceFinder.h"
    "${INCLUDE_DIR}/CurvatureSpeedFunctor.h"
    "${INCLUDE_DIR}/CurvatureVariableSpeedFunctor.h"
    "${INCLUDE_DIR}/FacesCohort.h"
    "${INCLUDE_DIR}/FaceDetector.h"
    "${INCLUDE_DIR}/FaceFinder2D.h"
    "${INCLUDE_DIR}/FaceOrienter.h"
    "${INCLUDE_DIR}/FaceRegistrationMapper.h"
    "${INCLUDE_DIR}/FeaturesDetector.h"
    "${INCLUDE_DIR}/Landmarks.h"
    "${INCLUDE_DIR}/MiscFunctions.h"
    "${INCLUDE_DIR}/ModelViewer.h"
    "${INCLUDE_DIR}/NoseFinder.h"
    "${INCLUDE_DIR}/ObjMetaData.h"
    "${INCLUDE_DIR}/RegistrationFace.h"
    "${INCLUDE_DIR}/SurfaceMesher.h"
    )
set( SRC_FILES
    ${SRC_DIR}/${PROJECT_NAME}
    ${SRC_DIR}/CorrespondenceFinder
    ${SRC_DIR}/CurvatureSpeedFunctor
    ${SRC_DIR}/CurvatureVariableSpeedFunctor
    ${SRC_DIR}/FacesCohort
    ${SRC_DIR}/FaceDetector
    ${SRC_DIR}/FaceFinder2D
    ${SRC_DIR}/FaceOrienter
    ${SRC_DIR}/FaceRegistrationMapper
    ${SRC_DIR}/FeaturesDetector
    ${SRC_DIR}/MiscFunctions
    ${SRC_DIR}/ModelViewer
    ${SRC_DIR}/NoseFinder 
    ${SRC_DIR}/ObjMetaData
    ${SRC_DIR}/RegistrationFace
    ${SRC_DIR}/SurfaceMesher
    )

add_library( ${PROJECT_NAME} ${SRC_FILES} ${INCLUDE_FILES})
include( "cmake/LibInstall.cmake")
install( DIRECTORY "${PROJECT_SOURCE_DIR}/haarcascades" DESTINATION "${LIB_PRE_REQS}/${PROJECT_NAME}")
install( FILES "${PROJECT_SOURCE_DIR}/shape_predictor_68_face_landmarks.dat" DESTINATION "${LIB_PRE_REQS}/${PROJECT_NAME}")

