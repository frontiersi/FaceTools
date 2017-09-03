cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project( FaceTools)

set(WITH_CGAL TRUE)
set(WITH_DLIB TRUE)
set(WITH_RMODELIO TRUE)
set(WITH_QTOOLS TRUE)
include( "cmake/FindLibs.cmake")

set( INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set( SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set( FORMS_DIR "${PROJECT_SOURCE_DIR}/forms")
include_directories( ${INCLUDE_DIR})
include_directories( ${PROJECT_BINARY_DIR})

#set( FORMS
#    )

set( QOBJECTS
    "${INCLUDE_DIR}/BoundaryView.h"
    "${INCLUDE_DIR}/FaceActionInterface.h"
    "${INCLUDE_DIR}/FaceActionWorker.h"
    "${INCLUDE_DIR}/FaceModel.h"
    "${INCLUDE_DIR}/FaceModelManager.h"
    "${INCLUDE_DIR}/FaceModelWidget.h"
    "${INCLUDE_DIR}/FaceModelTabWidget.h"
    "${INCLUDE_DIR}/FaceView.h"
    "${INCLUDE_DIR}/FaceViewComboBox.h"
    "${INCLUDE_DIR}/FileIOInterface.h"
    "${INCLUDE_DIR}/InteractionInterface.h"
    "${INCLUDE_DIR}/InteractiveModelViewer.h"
    "${INCLUDE_DIR}/LandmarkGroupView.h"
    "${INCLUDE_DIR}/ModelInteractor.h"
    "${INCLUDE_DIR}/ModelViewerAnnotator.h"
    "${INCLUDE_DIR}/ModelViewerQtInterface.h"
    "${INCLUDE_DIR}/SurfacePathDrawer.h"
    "${INCLUDE_DIR}/VisualisationAction.h"
    )

set( INCLUDE_FILES
    ${QOBJECTS}
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
    "${INCLUDE_DIR}/LandmarkView.h"
    "${INCLUDE_DIR}/MiscFunctions.h"
    "${INCLUDE_DIR}/ModelViewer.h"
    "${INCLUDE_DIR}/ModelViewerVTKInterface.h"
    "${INCLUDE_DIR}/NoseFinder.h"
    "${INCLUDE_DIR}/ObjMetaData.h"
    "${INCLUDE_DIR}/RegistrationFace.h"
    "${INCLUDE_DIR}/SurfaceMesher.h"
    "${INCLUDE_DIR}/VisualisationOptions.h"
    )

set( SRC_FILES
    ${SRC_DIR}/${PROJECT_NAME}
    ${SRC_DIR}/BoundaryView
    ${SRC_DIR}/CorrespondenceFinder
    ${SRC_DIR}/CurvatureSpeedFunctor
    ${SRC_DIR}/CurvatureVariableSpeedFunctor
    ${SRC_DIR}/FacesCohort
    ${SRC_DIR}/FaceActionInterface
    ${SRC_DIR}/FaceActionWorker
    ${SRC_DIR}/FaceDetector
    ${SRC_DIR}/FaceFinder2D
    ${SRC_DIR}/FaceModel
    ${SRC_DIR}/FaceModelManager
    ${SRC_DIR}/FaceModelWidget
    ${SRC_DIR}/FaceModelTabWidget
    ${SRC_DIR}/FaceOrienter
    ${SRC_DIR}/FaceRegistrationMapper
    ${SRC_DIR}/FaceView
    ${SRC_DIR}/FaceViewComboBox
    ${SRC_DIR}/FeaturesDetector
    ${SRC_DIR}/FileIOInterface
    ${SRC_DIR}/InteractionInterface
    ${SRC_DIR}/InteractiveModelViewer
    ${SRC_DIR}/LandmarkGroupView
    ${SRC_DIR}/LandmarkView
    ${SRC_DIR}/MiscFunctions
    ${SRC_DIR}/ModelInteractor
    ${SRC_DIR}/ModelViewer
    ${SRC_DIR}/ModelViewerAnnotator
    ${SRC_DIR}/ModelViewerQtInterface
    ${SRC_DIR}/ModelViewerVTKInterface
    ${SRC_DIR}/NoseFinder 
    ${SRC_DIR}/ObjMetaData
    ${SRC_DIR}/RegistrationFace
    ${SRC_DIR}/SurfaceMesher
    ${SRC_DIR}/SurfacePathDrawer
    ${SRC_DIR}/VisualisationAction
    )

set( RESOURCE_FILE "res.qrc")

#qt5_wrap_ui( FORM_HEADERS ${FORMS})
qt5_wrap_cpp( QOBJECT_MOCS ${QOBJECTS})
qt5_add_resources( RESOURCE_FILE "${RESOURCE_FILE}")
#qt5_add_resources( RESOURCE_FILE "${RESOURCE_FILE}" OPTIONS "-no-compress")

add_library( ${PROJECT_NAME} ${SRC_FILES} ${QOBJECT_MOCS} ${INCLUDE_FILES} ${RESOURCE_FILE})
include( "cmake/LinkLibs.cmake")

install( DIRECTORY "${PROJECT_SOURCE_DIR}/haarcascades" DESTINATION "${LIB_PRE_REQS}/${PROJECT_NAME}")
install( FILES "${PROJECT_SOURCE_DIR}/shape_predictor_68_face_landmarks.dat" DESTINATION "${LIB_PRE_REQS}/${PROJECT_NAME}")

