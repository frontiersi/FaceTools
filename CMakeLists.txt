cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project( FaceTools)

set(WITH_QUAZIP TRUE)
set(WITH_CPD TRUE)
set(WITH_CGAL TRUE)
set(WITH_DLIB TRUE)
set(WITH_RMODELIO TRUE)
set(WITH_QTOOLS TRUE)
include( "cmake/FindLibs.cmake")

set( INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set( INCLUDE_ACTION_DIR "${INCLUDE_DIR}/Action")
set( INCLUDE_DETECT_DIR "${INCLUDE_DIR}/Detect")
set( INCLUDE_FILEIO_DIR "${INCLUDE_DIR}/FileIO")
set( INCLUDE_REPORT_DIR "${INCLUDE_DIR}/Report")
set( INCLUDE_VIS_DIR "${INCLUDE_DIR}/Vis")
set( INCLUDE_INT_DIR "${INCLUDE_DIR}/Interactor")
include_directories( ${INCLUDE_DIR})
include_directories( ${INCLUDE_ACTION_DIR})
include_directories( ${INCLUDE_DETECT_DIR})
include_directories( ${INCLUDE_FILEIO_DIR})
include_directories( ${INCLUDE_REPORT_DIR})
include_directories( ${INCLUDE_VIS_DIR})
include_directories( ${INCLUDE_INT_DIR})
include_directories( ${PROJECT_BINARY_DIR})
set( SRC_DIR "${PROJECT_SOURCE_DIR}/src")

set( QOBJECTS
    "${INCLUDE_ACTION_DIR}/ActionAddLandmark.h"
    "${INCLUDE_ACTION_DIR}/ActionAddPath.h"
    "${INCLUDE_ACTION_DIR}/ActionAlignICP.h"
    "${INCLUDE_ACTION_DIR}/ActionAlignLandmarks.h"
    "${INCLUDE_ACTION_DIR}/ActionBackfaceCulling.h"
    "${INCLUDE_ACTION_DIR}/ActionCloseAllFaceModels.h"
    "${INCLUDE_ACTION_DIR}/ActionCloseFaceModels.h"
    "${INCLUDE_ACTION_DIR}/ActionCopyViewer.h"
    "${INCLUDE_ACTION_DIR}/ActionCrop.h"
    "${INCLUDE_ACTION_DIR}/ActionDeleteLandmark.h"
    "${INCLUDE_ACTION_DIR}/ActionDeletePath.h"
    "${INCLUDE_ACTION_DIR}/ActionDetectFace.h"
    "${INCLUDE_ACTION_DIR}/ActionEditLandmarks.h"
    "${INCLUDE_ACTION_DIR}/ActionEditPaths.h"
    "${INCLUDE_ACTION_DIR}/ActionExportPDF.h"
    "${INCLUDE_ACTION_DIR}/ActionFillHoles.h"
    "${INCLUDE_ACTION_DIR}/ActionGetComponent.h"
    "${INCLUDE_ACTION_DIR}/ActionLoadDirFaceModels.h"
    "${INCLUDE_ACTION_DIR}/ActionLoadFaceModels.h"
    "${INCLUDE_ACTION_DIR}/ActionMapSurfaceData.h"
    "${INCLUDE_ACTION_DIR}/ActionMarquee.h"
    "${INCLUDE_ACTION_DIR}/ActionMoveViewer.h"
    "${INCLUDE_ACTION_DIR}/ActionOrientCameraToFace.h"
    "${INCLUDE_ACTION_DIR}/ActionRadialSelect.h"
    "${INCLUDE_ACTION_DIR}/ActionRemesh.h"
    "${INCLUDE_ACTION_DIR}/ActionRenameLandmark.h"
    "${INCLUDE_ACTION_DIR}/ActionRenamePath.h"
    "${INCLUDE_ACTION_DIR}/ActionResetCamera.h"
    "${INCLUDE_ACTION_DIR}/ActionSaveScreenshot.h"
    "${INCLUDE_ACTION_DIR}/ActionSaveFaceModels.h"
    "${INCLUDE_ACTION_DIR}/ActionSaveAsFaceModel.h"
    "${INCLUDE_ACTION_DIR}/ActionSetFocus.h"
    "${INCLUDE_ACTION_DIR}/ActionSetOpacity.h"
    "${INCLUDE_ACTION_DIR}/ActionSetParallelProjection.h"
    "${INCLUDE_ACTION_DIR}/ActionSmooth.h"
    "${INCLUDE_ACTION_DIR}/ActionSynchroniseCameraMovement.h"
    "${INCLUDE_ACTION_DIR}/ActionToggleAxes.h"
    "${INCLUDE_ACTION_DIR}/ActionToggleCameraActorInteraction.h"
    "${INCLUDE_ACTION_DIR}/ActionTransformToStandardPosition.h"
    "${INCLUDE_ACTION_DIR}/ActionVisualise.h"
    "${INCLUDE_ACTION_DIR}/CameraWorker.h"
    "${INCLUDE_ACTION_DIR}/FaceAction.h"
    "${INCLUDE_ACTION_DIR}/FaceActionGroup.h"
    "${INCLUDE_ACTION_DIR}/FaceActionManager.h"
    "${INCLUDE_ACTION_DIR}/FaceActionWorker.h"
    "${INCLUDE_ACTION_DIR}/ModelSelector.h"
    "${INCLUDE_ACTION_DIR}/VisualisationsManager.h"

    "${INCLUDE_FILEIO_DIR}/FaceModelAssImpFileHandler.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelFileHandler.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelFileHandlerInterface.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelOBJFileHandler.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelU3DFileHandler.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelXMLFileHandler.h"

    "${INCLUDE_REPORT_DIR}/BaseReportTemplate.h"
    "${INCLUDE_REPORT_DIR}/ReportInterface.h"
    "${INCLUDE_REPORT_DIR}/SampleReport.h"

    "${INCLUDE_VIS_DIR}/BaseVisualisation.h"
    "${INCLUDE_VIS_DIR}/BoundingVisualisation.h"
    "${INCLUDE_VIS_DIR}/LandmarksVisualisation.h"
    "${INCLUDE_VIS_DIR}/OutlinesVisualisation.h"
    "${INCLUDE_VIS_DIR}/PathSetVisualisation.h"
    "${INCLUDE_VIS_DIR}/RadialSelectVisualisation.h"
    "${INCLUDE_VIS_DIR}/ScalarVisualisation.h"
    "${INCLUDE_VIS_DIR}/VisualisationInterface.h"

    "${INCLUDE_INT_DIR}/ContextMenuInteractor.h"
    "${INCLUDE_INT_DIR}/FaceEntryExitInteractor.h"
    "${INCLUDE_INT_DIR}/FaceHoveringInteractor.h"
    "${INCLUDE_INT_DIR}/LandmarksInteractor.h"
    "${INCLUDE_INT_DIR}/ModelViewerEntryExitInteractor.h"
    "${INCLUDE_INT_DIR}/ModelMoveInteractor.h"
    "${INCLUDE_INT_DIR}/ModelMoveResponder.h"
    "${INCLUDE_INT_DIR}/ModelSelectInteractor.h"
    "${INCLUDE_INT_DIR}/ModelViewerInteractor.h"
    "${INCLUDE_INT_DIR}/RadialSelectInteractor.h"
    "${INCLUDE_INT_DIR}/PathSetInteractor.h"
    "${INCLUDE_INT_DIR}/ViewerInteractionManager.h"

    "${INCLUDE_DIR}/FaceModelSurfaceData.h"
    "${INCLUDE_DIR}/FaceModelViewer.h"
    "${INCLUDE_DIR}/ModelViewer.h"
    "${INCLUDE_DIR}/ModelViewerAnnotator.h"
    "${INCLUDE_DIR}/MultiFaceModelViewer.h"
    "${INCLUDE_DIR}/SurfaceData.h"
    )

set( INCLUDE_FILES
    ${QOBJECTS}
    "${INCLUDE_DIR}/${PROJECT_NAME}.h"

    "${INCLUDE_ACTION_DIR}/ActionExecutionQueue.h"
    "${INCLUDE_ACTION_DIR}/ChangeEvents.h"

    "${INCLUDE_DETECT_DIR}/FaceDetector.h"
    "${INCLUDE_DETECT_DIR}/FaceFinder2D.h"
    "${INCLUDE_DETECT_DIR}/FacialOrientation.h"
    "${INCLUDE_DETECT_DIR}/FaceShapeLandmarks2DDetector.h"
    "${INCLUDE_DETECT_DIR}/FeaturesDetector.h"
    "${INCLUDE_DETECT_DIR}/NoseFinder.h"

    "${INCLUDE_FILEIO_DIR}/FaceModelAssImpFileHandlerFactory.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelFileHandlerException.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelFileHandlerMap.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelManager.h"
    "${INCLUDE_FILEIO_DIR}/LoadFaceModelsHelper.h"

    "${INCLUDE_VIS_DIR}/BoundingView.h"
    "${INCLUDE_VIS_DIR}/FaceView.h"
    "${INCLUDE_VIS_DIR}/LandmarkSetView.h"
    "${INCLUDE_VIS_DIR}/LegendScalarColourRangeMapper.h"
    "${INCLUDE_VIS_DIR}/LoopsView.h"
    "${INCLUDE_VIS_DIR}/PathView.h"
    "${INCLUDE_VIS_DIR}/PathSetView.h"
    "${INCLUDE_VIS_DIR}/SphereView.h"

    #"${INCLUDE_DIR}/CorrespondenceFinder.h"
    "${INCLUDE_DIR}/CurvatureSpeedFunctor.h"
    "${INCLUDE_DIR}/CurvatureVariableSpeedFunctor.h"
    #"${INCLUDE_DIR}/FaceAverager.h"
    "${INCLUDE_DIR}/FaceControl.h"
    "${INCLUDE_DIR}/FaceControlSet.h"

    "${INCLUDE_DIR}/FaceModel.h"
    #"${INCLUDE_DIR}/FacePointSampler.h"
    "${INCLUDE_DIR}/Hashing.h"
    "${INCLUDE_DIR}/Landmark.h"
    "${INCLUDE_DIR}/LandmarkSet.h"
    "${INCLUDE_DIR}/MiscFunctions.h"
    "${INCLUDE_DIR}/SurfaceMesher.h"
    "${INCLUDE_DIR}/PathSet.h"
    "${INCLUDE_DIR}/Path.h"
    )

set( SRC_FILES
    ${SRC_DIR}/${PROJECT_NAME}
    ${SRC_DIR}/Action/ActionAddLandmark
    ${SRC_DIR}/Action/ActionAddPath
    ${SRC_DIR}/Action/ActionAlignICP
    ${SRC_DIR}/Action/ActionAlignLandmarks
    ${SRC_DIR}/Action/ActionBackfaceCulling
    ${SRC_DIR}/Action/ActionCloseAllFaceModels
    ${SRC_DIR}/Action/ActionCloseFaceModels
    ${SRC_DIR}/Action/ActionCopyViewer
    ${SRC_DIR}/Action/ActionCrop
    ${SRC_DIR}/Action/ActionDeleteLandmark
    ${SRC_DIR}/Action/ActionDeletePath
    ${SRC_DIR}/Action/ActionDetectFace
    ${SRC_DIR}/Action/ActionEditLandmarks
    ${SRC_DIR}/Action/ActionEditPaths
    ${SRC_DIR}/Action/ActionExportPDF
    ${SRC_DIR}/Action/ActionExecutionQueue
    ${SRC_DIR}/Action/ActionFillHoles
    ${SRC_DIR}/Action/ActionGetComponent
    ${SRC_DIR}/Action/ActionLoadDirFaceModels
    ${SRC_DIR}/Action/ActionLoadFaceModels
    ${SRC_DIR}/Action/ActionMapSurfaceData
    ${SRC_DIR}/Action/ActionMarquee
    ${SRC_DIR}/Action/ActionMoveViewer
    ${SRC_DIR}/Action/ActionSaveFaceModels
    ${SRC_DIR}/Action/ActionSaveAsFaceModel
    ${SRC_DIR}/Action/ActionOrientCameraToFace
    ${SRC_DIR}/Action/ActionRadialSelect
    ${SRC_DIR}/Action/ActionRemesh
    ${SRC_DIR}/Action/ActionRenameLandmark
    ${SRC_DIR}/Action/ActionRenamePath
    ${SRC_DIR}/Action/ActionResetCamera
    ${SRC_DIR}/Action/ActionSaveScreenshot
    ${SRC_DIR}/Action/ActionSetFocus
    ${SRC_DIR}/Action/ActionSetOpacity
    ${SRC_DIR}/Action/ActionSetParallelProjection
    ${SRC_DIR}/Action/ActionSmooth
    ${SRC_DIR}/Action/ActionSynchroniseCameraMovement
    ${SRC_DIR}/Action/ActionToggleAxes
    ${SRC_DIR}/Action/ActionToggleCameraActorInteraction
    ${SRC_DIR}/Action/ActionTransformToStandardPosition
    ${SRC_DIR}/Action/ActionVisualise
    ${SRC_DIR}/Action/CameraWorker
    ${SRC_DIR}/Action/FaceAction
    ${SRC_DIR}/Action/FaceActionGroup
    ${SRC_DIR}/Action/FaceActionManager
    ${SRC_DIR}/Action/FaceActionWorker
    ${SRC_DIR}/Action/ModelSelector
    ${SRC_DIR}/Action/VisualisationsManager

    ${SRC_DIR}/Detect/FaceDetector
    ${SRC_DIR}/Detect/FaceFinder2D
    ${SRC_DIR}/Detect/FacialOrientation
    ${SRC_DIR}/Detect/FaceShapeLandmarks2DDetector
    ${SRC_DIR}/Detect/FeaturesDetector
    ${SRC_DIR}/Detect/NoseFinder 

    ${SRC_DIR}/FileIO/FaceModelAssImpFileHandler
    ${SRC_DIR}/FileIO/FaceModelAssImpFileHandlerFactory
    ${SRC_DIR}/FileIO/FaceModelFileHandler
    ${SRC_DIR}/FileIO/FaceModelFileHandlerException
    ${SRC_DIR}/FileIO/FaceModelFileHandlerMap
    ${SRC_DIR}/FileIO/FaceModelManager
    ${SRC_DIR}/FileIO/FaceModelOBJFileHandler
    ${SRC_DIR}/FileIO/FaceModelXMLFileHandler
    ${SRC_DIR}/FileIO/FaceModelU3DFileHandler
    ${SRC_DIR}/FileIO/LoadFaceModelsHelper

    ${SRC_DIR}/Report/SampleReport

    ${SRC_DIR}/Vis/BaseVisualisation
    ${SRC_DIR}/Vis/BoundingView
    ${SRC_DIR}/Vis/BoundingVisualisation
    ${SRC_DIR}/Vis/FaceView
    ${SRC_DIR}/Vis/LandmarkSetView
    ${SRC_DIR}/Vis/LandmarksVisualisation
    ${SRC_DIR}/Vis/LegendScalarColourRangeMapper
    ${SRC_DIR}/Vis/LoopsView
    ${SRC_DIR}/Vis/OutlinesVisualisation
    ${SRC_DIR}/Vis/PathView
    ${SRC_DIR}/Vis/PathSetView
    ${SRC_DIR}/Vis/PathSetVisualisation
    ${SRC_DIR}/Vis/RadialSelectVisualisation
    ${SRC_DIR}/Vis/ScalarVisualisation
    ${SRC_DIR}/Vis/SphereView

    ${SRC_DIR}/Interactor/ContextMenuInteractor
    ${SRC_DIR}/Interactor/FaceEntryExitInteractor
    ${SRC_DIR}/Interactor/FaceHoveringInteractor
    ${SRC_DIR}/Interactor/LandmarksInteractor
    ${SRC_DIR}/Interactor/ModelViewerInteractor
    ${SRC_DIR}/Interactor/ModelMoveInteractor
    ${SRC_DIR}/Interactor/ModelMoveResponder
    ${SRC_DIR}/Interactor/ModelSelectInteractor
    ${SRC_DIR}/Interactor/RadialSelectInteractor
    ${SRC_DIR}/Interactor/PathSetInteractor
    ${SRC_DIR}/Interactor/ViewerInteractionManager

    #${SRC_DIR}/CorrespondenceFinder
    ${SRC_DIR}/CurvatureSpeedFunctor
    ${SRC_DIR}/CurvatureVariableSpeedFunctor
    #${SRC_DIR}/FaceAverager
    ${SRC_DIR}/FaceControl
    ${SRC_DIR}/FaceControlSet

    ${SRC_DIR}/FaceModel
    ${SRC_DIR}/FaceModelSurfaceData
    ${SRC_DIR}/FaceModelViewer
    #${SRC_DIR}/FacePointSampler
    ${SRC_DIR}/Landmark
    ${SRC_DIR}/LandmarkSet
    ${SRC_DIR}/MiscFunctions
    ${SRC_DIR}/ModelViewer
    ${SRC_DIR}/ModelViewerAnnotator
    ${SRC_DIR}/MultiFaceModelViewer
    ${SRC_DIR}/SurfaceData
    ${SRC_DIR}/SurfaceMesher
    ${SRC_DIR}/Path
    ${SRC_DIR}/PathSet
    )

set( RESOURCE_FILE "FaceTools_res.qrc")

#qt5_wrap_ui( FORM_HEADERS ${FORMS})
qt5_wrap_cpp( QOBJECT_MOCS ${QOBJECTS})
qt5_add_resources( RESOURCE_FILE "${RESOURCE_FILE}")
#qt5_add_resources( RESOURCE_FILE "${RESOURCE_FILE}" OPTIONS "-no-compress")

add_library( ${PROJECT_NAME} ${SRC_FILES} ${QOBJECT_MOCS} ${INCLUDE_FILES} ${RESOURCE_FILE})
include( "cmake/LinkLibs.cmake")

install( DIRECTORY "${PROJECT_SOURCE_DIR}/haarcascades" DESTINATION "${LIB_PRE_REQS}/${PROJECT_NAME}")
install( FILES "${PROJECT_SOURCE_DIR}/shape_predictor_68_face_landmarks.dat" DESTINATION "${LIB_PRE_REQS}/${PROJECT_NAME}")

