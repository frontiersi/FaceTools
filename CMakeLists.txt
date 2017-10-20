cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project( FaceTools)

set(WITH_CPD TRUE)
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
    "${INCLUDE_DIR}/ActionAddLandmark.h"
    "${INCLUDE_DIR}/ActionAlignFace.h"
    "${INCLUDE_DIR}/ActionCloseModel.h"
    "${INCLUDE_DIR}/ActionCropBoundary.h"
    "${INCLUDE_DIR}/ActionDeleteLandmark.h"
    "${INCLUDE_DIR}/ActionDetectFace.h"
    "${INCLUDE_DIR}/ActionExportPDF.h"
    "${INCLUDE_DIR}/ActionLoadModel.h"
    "${INCLUDE_DIR}/ActionOrientCameraToFace.h"
    "${INCLUDE_DIR}/ActionPrintSurfaceInfo.h"
    "${INCLUDE_DIR}/ActionResetCamera.h"
    "${INCLUDE_DIR}/ActionSaveModel.h"
    "${INCLUDE_DIR}/ActionSaveModelAs.h"
    "${INCLUDE_DIR}/ActionSaveSnapshot.h"
    "${INCLUDE_DIR}/ActionSetFocus.h"
    "${INCLUDE_DIR}/ActionToggleShowBoundary.h"
    "${INCLUDE_DIR}/ActionToggleShowLandmarks.h"
    "${INCLUDE_DIR}/AssImpIO.h"
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
    "${INCLUDE_DIR}/PointsVisualisation.h"
    "${INCLUDE_DIR}/SurfacePathDrawer.h"
    "${INCLUDE_DIR}/SurfaceVisualisation.h"
    "${INCLUDE_DIR}/TextureVisualisation.h"
    "${INCLUDE_DIR}/U3DFileIO.h"
    "${INCLUDE_DIR}/VisualisationAction.h"
    "${INCLUDE_DIR}/WireframeVisualisation.h"
    )

set( INCLUDE_FILES
    ${QOBJECTS}
    "${INCLUDE_DIR}/${PROJECT_NAME}.h"
    #"${INCLUDE_DIR}/CorrespondenceFinder.h"
    "${INCLUDE_DIR}/CurvatureSpeedFunctor.h"
    "${INCLUDE_DIR}/CurvatureVariableSpeedFunctor.h"
    "${INCLUDE_DIR}/FacesCohort.h"
    "${INCLUDE_DIR}/FaceAverager.h"
    "${INCLUDE_DIR}/FaceDetector.h"
    "${INCLUDE_DIR}/FaceFinder2D.h"
    "${INCLUDE_DIR}/FaceOrienter.h"
    "${INCLUDE_DIR}/FacePointSampler.h"
    "${INCLUDE_DIR}/FacePreferences.h"
    "${INCLUDE_DIR}/FacePreProcessor.h"
    "${INCLUDE_DIR}/FeaturesDetector.h"
    "${INCLUDE_DIR}/Landmarks.h"
    "${INCLUDE_DIR}/LandmarkView.h"
    "${INCLUDE_DIR}/MiscFunctions.h"
    "${INCLUDE_DIR}/ModelViewer.h"
    "${INCLUDE_DIR}/ModelViewerVTKInterface.h"
    "${INCLUDE_DIR}/NoseFinder.h"
    "${INCLUDE_DIR}/ObjMetaData.h"
    "${INCLUDE_DIR}/SurfaceMesher.h"
    "${INCLUDE_DIR}/VisualisationOptions.h"
    )

set( SRC_FILES
    ${SRC_DIR}/${PROJECT_NAME}
    ${SRC_DIR}/ActionAddLandmark
    ${SRC_DIR}/ActionAlignFace
    ${SRC_DIR}/ActionCloseModel
    ${SRC_DIR}/ActionCropBoundary
    ${SRC_DIR}/ActionDeleteLandmark
    ${SRC_DIR}/ActionDetectFace
    ${SRC_DIR}/ActionExportPDF
    ${SRC_DIR}/ActionLoadModel
    ${SRC_DIR}/ActionOrientCameraToFace
    ${SRC_DIR}/ActionPrintSurfaceInfo
    ${SRC_DIR}/ActionResetCamera
    ${SRC_DIR}/ActionSaveModel
    ${SRC_DIR}/ActionSaveModelAs
    ${SRC_DIR}/ActionSaveSnapshot
    ${SRC_DIR}/ActionSetFocus
    ${SRC_DIR}/ActionToggleShowBoundary
    ${SRC_DIR}/ActionToggleShowLandmarks
    ${SRC_DIR}/AssImpIO
    ${SRC_DIR}/BoundaryView
    #${SRC_DIR}/CorrespondenceFinder
    ${SRC_DIR}/CurvatureSpeedFunctor
    ${SRC_DIR}/CurvatureVariableSpeedFunctor
    ${SRC_DIR}/FacesCohort
    ${SRC_DIR}/FaceActionInterface
    ${SRC_DIR}/FaceActionWorker
    ${SRC_DIR}/FaceAverager
    ${SRC_DIR}/FaceDetector
    ${SRC_DIR}/FaceFinder2D
    ${SRC_DIR}/FaceModel
    ${SRC_DIR}/FaceModelManager
    ${SRC_DIR}/FaceModelWidget
    ${SRC_DIR}/FaceModelTabWidget
    ${SRC_DIR}/FaceOrienter
    ${SRC_DIR}/FacePointSampler
    ${SRC_DIR}/FacePreProcessor
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
    ${SRC_DIR}/PointsVisualisation
    ${SRC_DIR}/SurfaceMesher
    ${SRC_DIR}/SurfacePathDrawer
    ${SRC_DIR}/SurfaceVisualisation
    ${SRC_DIR}/TextureVisualisation
    ${SRC_DIR}/U3DFileIO
    ${SRC_DIR}/VisualisationAction
    ${SRC_DIR}/WireframeVisualisation
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

