cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

project( FaceTools)

set(WITH_QUAZIP TRUE)
set(WITH_DLIB TRUE)
set(WITH_RMODELIO TRUE)
set(WITH_QTOOLS TRUE)
set(WITH_LUA TRUE)
include( "cmake/FindLibs.cmake")

set( INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set( FORMS_DIR "${PROJECT_SOURCE_DIR}/forms")
set( SRC_DIR "${PROJECT_SOURCE_DIR}/src")

set( INCLUDE_ACTION_DIR "${INCLUDE_DIR}/Action")
set( INCLUDE_DETECT_DIR "${INCLUDE_DIR}/Detect")
set( INCLUDE_FILEIO_DIR "${INCLUDE_DIR}/FileIO")
set( INCLUDE_LNDMRK_DIR "${INCLUDE_DIR}/LndMrk")
set( INCLUDE_METRIC_DIR "${INCLUDE_DIR}/Metric")
set( INCLUDE_REPORT_DIR "${INCLUDE_DIR}/Report")
set( INCLUDE_WIDGET_DIR "${INCLUDE_DIR}/Widget")
set( INCLUDE_INT_DIR "${INCLUDE_DIR}/Interactor")
set( INCLUDE_VIS_DIR "${INCLUDE_DIR}/Vis")

set( SRC_ACTION_DIR "${SRC_DIR}/Action")
set( SRC_DETECT_DIR "${SRC_DIR}/Detect")
set( SRC_FILEIO_DIR "${SRC_DIR}/FileIO")
set( SRC_LNDMRK_DIR "${SRC_DIR}/LndMrk")
set( SRC_METRIC_DIR "${SRC_DIR}/Metric")
set( SRC_REPORT_DIR "${SRC_DIR}/Report")
set( SRC_WIDGET_DIR "${SRC_DIR}/Widget")
set( SRC_INT_DIR "${SRC_DIR}/Interactor")
set( SRC_VIS_DIR "${SRC_DIR}/Vis")

include_directories( ${INCLUDE_DIR})
include_directories( ${PROJECT_BINARY_DIR}) # Req. for generated Qt form headers

set( FORMS
    "${FORMS_DIR}/ChartDialog.ui"
    "${FORMS_DIR}/DetectionCheckDialog.ui"
    "${FORMS_DIR}/MetricsDialog.ui"
    "${FORMS_DIR}/ModelPropertiesDialog.ui"
    "${FORMS_DIR}/PhenotypesDialog.ui"
    "${FORMS_DIR}/ReportChooserDialog.ui"
    "${FORMS_DIR}/ResizeDialog.ui"
    "${FORMS_DIR}/ScanInfoDialog.ui"
    )

set( QOBJECTS
    "${INCLUDE_ACTION_DIR}/ActionAddPath.h"
    "${INCLUDE_ACTION_DIR}/ActionAlignICP.h"
    "${INCLUDE_ACTION_DIR}/ActionAlignLandmarks.h"
    "${INCLUDE_ACTION_DIR}/ActionAlignVertices.h"
    "${INCLUDE_ACTION_DIR}/ActionBackfaceCulling.h"
    "${INCLUDE_ACTION_DIR}/ActionChangeSurfaceMappingRange.h"
    "${INCLUDE_ACTION_DIR}/ActionCloseAllFaceModels.h"
    "${INCLUDE_ACTION_DIR}/ActionCloseFaceModel.h"
    "${INCLUDE_ACTION_DIR}/ActionCopyView.h"
    "${INCLUDE_ACTION_DIR}/ActionCrop.h"
    "${INCLUDE_ACTION_DIR}/ActionDeletePath.h"
    "${INCLUDE_ACTION_DIR}/ActionDetectFace.h"
    "${INCLUDE_ACTION_DIR}/ActionDiscardManifold.h"
    "${INCLUDE_ACTION_DIR}/ActionEditLandmarks.h"
    "${INCLUDE_ACTION_DIR}/ActionEditPaths.h"
    "${INCLUDE_ACTION_DIR}/ActionExportMetaData.h"
    "${INCLUDE_ACTION_DIR}/ActionExportPDF.h"
    "${INCLUDE_ACTION_DIR}/ActionFillHoles.h"
    "${INCLUDE_ACTION_DIR}/ActionGetFaceManifold.h"
    "${INCLUDE_ACTION_DIR}/ActionImportMetaData.h"
    "${INCLUDE_ACTION_DIR}/ActionInvertNormals.h"
    "${INCLUDE_ACTION_DIR}/ActionLoadDirFaceModels.h"
    "${INCLUDE_ACTION_DIR}/ActionLoadFaceModels.h"
    "${INCLUDE_ACTION_DIR}/ActionMakeHalfFace.h"
    "${INCLUDE_ACTION_DIR}/ActionMapCurvature.h"
    "${INCLUDE_ACTION_DIR}/ActionMarquee.h"
    "${INCLUDE_ACTION_DIR}/ActionMoveView.h"
    "${INCLUDE_ACTION_DIR}/ActionNonRigidRegistration.h"
    "${INCLUDE_ACTION_DIR}/ActionOrientCameraToFace.h"
    "${INCLUDE_ACTION_DIR}/ActionRadialSelect.h"
    "${INCLUDE_ACTION_DIR}/ActionRedo.h"
    "${INCLUDE_ACTION_DIR}/ActionReflect.h"
    #"${INCLUDE_ACTION_DIR}/ActionRemesh.h"
    "${INCLUDE_ACTION_DIR}/ActionRemoveManifolds.h"
    "${INCLUDE_ACTION_DIR}/ActionRenamePath.h"
    "${INCLUDE_ACTION_DIR}/ActionResetCamera.h"
    "${INCLUDE_ACTION_DIR}/ActionResetDetection.h"
    "${INCLUDE_ACTION_DIR}/ActionRotateModel.h"
    "${INCLUDE_ACTION_DIR}/ActionSaveAsFaceModel.h"
    "${INCLUDE_ACTION_DIR}/ActionSaveFaceModel.h"
    "${INCLUDE_ACTION_DIR}/ActionSaveScreenshot.h"
    "${INCLUDE_ACTION_DIR}/ActionScaleModel.h"
    "${INCLUDE_ACTION_DIR}/ActionSetFocus.h"
    "${INCLUDE_ACTION_DIR}/ActionSetMaxScalarColour.h"
    "${INCLUDE_ACTION_DIR}/ActionSetMinScalarColour.h"
    "${INCLUDE_ACTION_DIR}/ActionSetNumScalarColours.h"
    "${INCLUDE_ACTION_DIR}/ActionSetOpacity.h"
    "${INCLUDE_ACTION_DIR}/ActionSetParallelProjection.h"
    "${INCLUDE_ACTION_DIR}/ActionSetSurfaceColour.h"
    "${INCLUDE_ACTION_DIR}/ActionShowMetrics.h"
    "${INCLUDE_ACTION_DIR}/ActionShowModelProperties.h"
    "${INCLUDE_ACTION_DIR}/ActionShowScanInfo.h"
    "${INCLUDE_ACTION_DIR}/ActionSlice.h"
    "${INCLUDE_ACTION_DIR}/ActionSmooth.h"
    "${INCLUDE_ACTION_DIR}/ActionSynchroniseCameraMovement.h"
    "${INCLUDE_ACTION_DIR}/ActionToggleAxes.h"
    "${INCLUDE_ACTION_DIR}/ActionToggleCameraActorInteraction.h"
    "${INCLUDE_ACTION_DIR}/ActionToggleScalarLegend.h"
    "${INCLUDE_ACTION_DIR}/ActionToggleStereoRendering.h"
    "${INCLUDE_ACTION_DIR}/ActionTransformToCentre.h"
    "${INCLUDE_ACTION_DIR}/ActionTransformToStandardPosition.h"
    "${INCLUDE_ACTION_DIR}/ActionUndo.h"
    "${INCLUDE_ACTION_DIR}/ActionUpdateMeasurements.h"
    "${INCLUDE_ACTION_DIR}/ActionUpdateThumbnail.h"
    "${INCLUDE_ACTION_DIR}/ActionUpdateU3D.h"
    "${INCLUDE_ACTION_DIR}/ActionVisualise.h"
    "${INCLUDE_ACTION_DIR}/CameraWorker.h"
    "${INCLUDE_ACTION_DIR}/FaceAction.h"
    "${INCLUDE_ACTION_DIR}/FaceActionManager.h"
    "${INCLUDE_ACTION_DIR}/FaceActionWorker.h"
    "${INCLUDE_ACTION_DIR}/UndoStates.h"

    "${INCLUDE_FILEIO_DIR}/BackgroundModelLoader.h"

    "${INCLUDE_METRIC_DIR}/Chart.h"

    "${INCLUDE_REPORT_DIR}/Report.h"

    "${INCLUDE_INT_DIR}/ActorMoveNotifier.h"
    "${INCLUDE_INT_DIR}/LandmarksHandler.h"
    "${INCLUDE_INT_DIR}/MovementNotifier.h"
    "${INCLUDE_INT_DIR}/PathsHandler.h"
    "${INCLUDE_INT_DIR}/PropHandler.h"
    "${INCLUDE_INT_DIR}/PropNotifier.h"
    "${INCLUDE_INT_DIR}/RadialSelectHandler.h"
    "${INCLUDE_INT_DIR}/SelectNotifier.h"

    "${INCLUDE_WIDGET_DIR}/ChartDialog.h"
    "${INCLUDE_WIDGET_DIR}/CheckAllTableHeader.h"
    "${INCLUDE_WIDGET_DIR}/DetectionCheckDialog.h"
    "${INCLUDE_WIDGET_DIR}/MetricsDialog.h"
    "${INCLUDE_WIDGET_DIR}/ModelPropertiesDialog.h"
    "${INCLUDE_WIDGET_DIR}/PhenotypesDialog.h"
    "${INCLUDE_WIDGET_DIR}/ReportChooserDialog.h"
    "${INCLUDE_WIDGET_DIR}/ResizeDialog.h"
    "${INCLUDE_WIDGET_DIR}/ScanInfoDialog.h"

    "${INCLUDE_DIR}/FaceModelViewer.h"
    "${INCLUDE_DIR}/FaceTypes.h"
    "${INCLUDE_DIR}/ModelViewer.h"
    "${INCLUDE_DIR}/ModelViewerAnnotator.h"
    "${INCLUDE_DIR}/MultiFaceModelViewer.h"
    )

set( INCLUDE_FILES
    ${QOBJECTS}
    "${INCLUDE_DIR}/${PROJECT_NAME}.h"

    "${INCLUDE_ACTION_DIR}/FaceModelState.h"
    "${INCLUDE_ACTION_DIR}/ModelSelector.h"
    "${INCLUDE_ACTION_DIR}/UndoState.h"

    "${INCLUDE_DETECT_DIR}/FaceFinder2D.h"
    "${INCLUDE_DETECT_DIR}/FaceOrientationDetector.h"
    "${INCLUDE_DETECT_DIR}/FaceShapeLandmarks2DDetector.h"
    "${INCLUDE_DETECT_DIR}/FeaturesDetector.h"

    "${INCLUDE_FILEIO_DIR}/FaceModelAssImpFileHandlerFactory.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelFileHandlerException.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelFileHandlerMap.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelManager.h"
    "${INCLUDE_FILEIO_DIR}/LoadFaceModelsHelper.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelFileHandler.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelAssImpFileHandler.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelOBJFileHandler.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelPLYFileHandler.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelU3DFileHandler.h"
    "${INCLUDE_FILEIO_DIR}/FaceModelXMLFileHandler.h"

    "${INCLUDE_INT_DIR}/ContextMenu.h"
    "${INCLUDE_INT_DIR}/MouseHandler.h"
    "${INCLUDE_INT_DIR}/ViewerNotifier.h"

    "${INCLUDE_LNDMRK_DIR}/Landmark.h"
    "${INCLUDE_LNDMRK_DIR}/LandmarkSet.h"
    "${INCLUDE_LNDMRK_DIR}/LandmarksManager.h"

    "${INCLUDE_METRIC_DIR}/Gene.h"
    "${INCLUDE_METRIC_DIR}/GeneManager.h"
    "${INCLUDE_METRIC_DIR}/GrowthData.h"
    "${INCLUDE_METRIC_DIR}/MetricCalculatorTypeRegistry.h"
    "${INCLUDE_METRIC_DIR}/MetricCalculatorManager.h"
    "${INCLUDE_METRIC_DIR}/MetricSet.h"
    "${INCLUDE_METRIC_DIR}/MetricValue.h"
    "${INCLUDE_METRIC_DIR}/Phenotype.h"
    "${INCLUDE_METRIC_DIR}/PhenotypeManager.h"
    "${INCLUDE_METRIC_DIR}/Syndrome.h"
    "${INCLUDE_METRIC_DIR}/SyndromeManager.h"
    "${INCLUDE_METRIC_DIR}/CircularityMetricCalculatorType.h"
    "${INCLUDE_METRIC_DIR}/CurvatureMetricCalculatorType.h"
    "${INCLUDE_METRIC_DIR}/DistanceMetricCalculatorType.h"
    "${INCLUDE_METRIC_DIR}/LineAngleMetricCalculatorType.h"
    "${INCLUDE_METRIC_DIR}/MetricCalculatorType.h"
    "${INCLUDE_METRIC_DIR}/MetricCalculator.h"

    "${INCLUDE_REPORT_DIR}/ReportManager.h"

    "${INCLUDE_VIS_DIR}/AngleVisualiser.h"
    "${INCLUDE_VIS_DIR}/BaseVisualisation.h"
    "${INCLUDE_VIS_DIR}/BoundingView.h"
    "${INCLUDE_VIS_DIR}/BoundingVisualisation.h"
    "${INCLUDE_VIS_DIR}/EuclideanDistanceVisualiser.h"
    "${INCLUDE_VIS_DIR}/FaceView.h"
    "${INCLUDE_VIS_DIR}/LabelsView.h"
    "${INCLUDE_VIS_DIR}/LabelsVisualisation.h"
    "${INCLUDE_VIS_DIR}/LandmarkSetView.h"
    "${INCLUDE_VIS_DIR}/LandmarksVisualisation.h"
    "${INCLUDE_VIS_DIR}/LoopView.h"
    "${INCLUDE_VIS_DIR}/MetricVisualiser.h"
    "${INCLUDE_VIS_DIR}/OutlinesVisualisation.h"
    "${INCLUDE_VIS_DIR}/PathSetVisualisation.h"
    "${INCLUDE_VIS_DIR}/PathView.h"
    "${INCLUDE_VIS_DIR}/PathSetView.h"
    "${INCLUDE_VIS_DIR}/PointsView.h"
    "${INCLUDE_VIS_DIR}/PolyLabelsView.h"
    "${INCLUDE_VIS_DIR}/RadialSelectVisualisation.h"
    "${INCLUDE_VIS_DIR}/RegionVisualiser.h"
    "${INCLUDE_VIS_DIR}/SimpleView.h"
    "${INCLUDE_VIS_DIR}/SphereView.h"
    "${INCLUDE_VIS_DIR}/SurfaceMetricsMapper.h"
    "${INCLUDE_VIS_DIR}/SurfaceVisualisation.h"
    "${INCLUDE_VIS_DIR}/ViewInterface.h"
    "${INCLUDE_VIS_DIR}/VertexLabelsView.h"

    "${INCLUDE_WIDGET_DIR}/IntTableWidgetItem.h"

    #"${INCLUDE_DIR}/CurvatureSpeedFunctor.h"
    #"${INCLUDE_DIR}/CurvatureVariableSpeedFunctor.h"
    "${INCLUDE_DIR}/Ethnicities.h"
    "${INCLUDE_DIR}/FaceAssessment.h"
    "${INCLUDE_DIR}/FaceModel.h"
    "${INCLUDE_DIR}/FaceModelCurvature.h"
    "${INCLUDE_DIR}/FaceViewSet.h"
    "${INCLUDE_DIR}/MiscFunctions.h"
    #"${INCLUDE_DIR}/SurfaceMesher.h"
    "${INCLUDE_DIR}/Path.h"
    "${INCLUDE_DIR}/PathSet.h"
    "${INCLUDE_DIR}/U3DCache.h"
    )

set( SRC_FILES
    ${SRC_DIR}/${PROJECT_NAME}

    ${SRC_ACTION_DIR}/ActionAddPath
    ${SRC_ACTION_DIR}/ActionAlignICP
    ${SRC_ACTION_DIR}/ActionAlignLandmarks
    ${SRC_ACTION_DIR}/ActionAlignVertices
    ${SRC_ACTION_DIR}/ActionBackfaceCulling
    ${SRC_ACTION_DIR}/ActionChangeSurfaceMappingRange
    ${SRC_ACTION_DIR}/ActionCloseAllFaceModels
    ${SRC_ACTION_DIR}/ActionCloseFaceModel
    ${SRC_ACTION_DIR}/ActionCopyView
    ${SRC_ACTION_DIR}/ActionCrop
    ${SRC_ACTION_DIR}/ActionDeletePath
    ${SRC_ACTION_DIR}/ActionDetectFace
    ${SRC_ACTION_DIR}/ActionDiscardManifold
    ${SRC_ACTION_DIR}/ActionEditLandmarks
    ${SRC_ACTION_DIR}/ActionEditPaths
    ${SRC_ACTION_DIR}/ActionExportMetaData
    ${SRC_ACTION_DIR}/ActionExportPDF
    ${SRC_ACTION_DIR}/ActionFillHoles
    ${SRC_ACTION_DIR}/ActionGetFaceManifold
    ${SRC_ACTION_DIR}/ActionImportMetaData
    ${SRC_ACTION_DIR}/ActionInvertNormals
    ${SRC_ACTION_DIR}/ActionLoadDirFaceModels
    ${SRC_ACTION_DIR}/ActionLoadFaceModels
    ${SRC_ACTION_DIR}/ActionMakeHalfFace
    ${SRC_ACTION_DIR}/ActionMapCurvature
    ${SRC_ACTION_DIR}/ActionMarquee
    ${SRC_ACTION_DIR}/ActionMoveView
    ${SRC_ACTION_DIR}/ActionNonRigidRegistration
    ${SRC_ACTION_DIR}/ActionOrientCameraToFace
    ${SRC_ACTION_DIR}/ActionRadialSelect
    ${SRC_ACTION_DIR}/ActionRedo
    ${SRC_ACTION_DIR}/ActionReflect
    #${SRC_ACTION_DIR}/ActionRemesh
    ${SRC_ACTION_DIR}/ActionRemoveManifolds
    ${SRC_ACTION_DIR}/ActionRenamePath
    ${SRC_ACTION_DIR}/ActionResetCamera
    ${SRC_ACTION_DIR}/ActionResetDetection
    ${SRC_ACTION_DIR}/ActionRotateModel
    ${SRC_ACTION_DIR}/ActionSaveAsFaceModel
    ${SRC_ACTION_DIR}/ActionSaveFaceModel
    ${SRC_ACTION_DIR}/ActionSaveScreenshot
    ${SRC_ACTION_DIR}/ActionScaleModel
    ${SRC_ACTION_DIR}/ActionSetFocus
    ${SRC_ACTION_DIR}/ActionSetMaxScalarColour
    ${SRC_ACTION_DIR}/ActionSetMinScalarColour
    ${SRC_ACTION_DIR}/ActionSetNumScalarColours
    ${SRC_ACTION_DIR}/ActionSetOpacity
    ${SRC_ACTION_DIR}/ActionSetParallelProjection
    ${SRC_ACTION_DIR}/ActionSetSurfaceColour
    ${SRC_ACTION_DIR}/ActionShowMetrics
    ${SRC_ACTION_DIR}/ActionShowModelProperties
    ${SRC_ACTION_DIR}/ActionShowScanInfo
    ${SRC_ACTION_DIR}/ActionSlice
    ${SRC_ACTION_DIR}/ActionSmooth
    ${SRC_ACTION_DIR}/ActionSynchroniseCameraMovement
    ${SRC_ACTION_DIR}/ActionToggleAxes
    ${SRC_ACTION_DIR}/ActionToggleCameraActorInteraction
    ${SRC_ACTION_DIR}/ActionToggleScalarLegend
    ${SRC_ACTION_DIR}/ActionToggleStereoRendering
    ${SRC_ACTION_DIR}/ActionTransformToCentre
    ${SRC_ACTION_DIR}/ActionTransformToStandardPosition
    ${SRC_ACTION_DIR}/ActionUndo
    ${SRC_ACTION_DIR}/ActionUpdateMeasurements
    ${SRC_ACTION_DIR}/ActionUpdateThumbnail
    ${SRC_ACTION_DIR}/ActionUpdateU3D
    ${SRC_ACTION_DIR}/ActionVisualise
    ${SRC_ACTION_DIR}/CameraWorker
    ${SRC_ACTION_DIR}/FaceAction
    ${SRC_ACTION_DIR}/FaceActionManager
    ${SRC_ACTION_DIR}/FaceActionWorker
    ${SRC_ACTION_DIR}/FaceModelState
    ${SRC_ACTION_DIR}/ModelSelector
    ${SRC_ACTION_DIR}/UndoState
    ${SRC_ACTION_DIR}/UndoStates

    ${SRC_DETECT_DIR}/FaceFinder2D
    ${SRC_DETECT_DIR}/FaceOrientationDetector
    ${SRC_DETECT_DIR}/FaceShapeLandmarks2DDetector
    ${SRC_DETECT_DIR}/FeaturesDetector

    ${SRC_FILEIO_DIR}/BackgroundModelLoader
    ${SRC_FILEIO_DIR}/FaceModelAssImpFileHandler
    ${SRC_FILEIO_DIR}/FaceModelAssImpFileHandlerFactory
    ${SRC_FILEIO_DIR}/FaceModelFileHandler
    ${SRC_FILEIO_DIR}/FaceModelFileHandlerException
    ${SRC_FILEIO_DIR}/FaceModelFileHandlerMap
    ${SRC_FILEIO_DIR}/FaceModelManager
    ${SRC_FILEIO_DIR}/FaceModelOBJFileHandler
    ${SRC_FILEIO_DIR}/FaceModelPLYFileHandler
    ${SRC_FILEIO_DIR}/FaceModelXMLFileHandler
    ${SRC_FILEIO_DIR}/FaceModelU3DFileHandler
    ${SRC_FILEIO_DIR}/LoadFaceModelsHelper

    ${SRC_INT_DIR}/ActorMoveNotifier
    ${SRC_INT_DIR}/ContextMenu
    ${SRC_INT_DIR}/LandmarksHandler
    ${SRC_INT_DIR}/MouseHandler
    ${SRC_INT_DIR}/MovementNotifier
    ${SRC_INT_DIR}/PropHandler
    ${SRC_INT_DIR}/PropNotifier
    ${SRC_INT_DIR}/PathsHandler
    ${SRC_INT_DIR}/RadialSelectHandler
    ${SRC_INT_DIR}/SelectNotifier
    ${SRC_INT_DIR}/ViewerNotifier

    ${SRC_LNDMRK_DIR}/Landmark
    ${SRC_LNDMRK_DIR}/LandmarkSet
    ${SRC_LNDMRK_DIR}/LandmarksManager

    ${SRC_METRIC_DIR}/CircularityMetricCalculatorType
    ${SRC_METRIC_DIR}/CurvatureMetricCalculatorType
    ${SRC_METRIC_DIR}/Chart
    ${SRC_METRIC_DIR}/DistanceMetricCalculatorType
    ${SRC_METRIC_DIR}/Gene
    ${SRC_METRIC_DIR}/GeneManager
    ${SRC_METRIC_DIR}/GrowthData
    ${SRC_METRIC_DIR}/LineAngleMetricCalculatorType
    ${SRC_METRIC_DIR}/MetricCalculatorType
    ${SRC_METRIC_DIR}/MetricCalculatorTypeRegistry
    ${SRC_METRIC_DIR}/MetricCalculatorManager
    ${SRC_METRIC_DIR}/MetricCalculator
    ${SRC_METRIC_DIR}/MetricSet
    ${SRC_METRIC_DIR}/MetricValue
    ${SRC_METRIC_DIR}/Phenotype
    ${SRC_METRIC_DIR}/PhenotypeManager
    ${SRC_METRIC_DIR}/Syndrome
    ${SRC_METRIC_DIR}/SyndromeManager

    ${SRC_REPORT_DIR}/Report
    ${SRC_REPORT_DIR}/ReportManager

    ${SRC_VIS_DIR}/AngleVisualiser
    ${SRC_VIS_DIR}/BaseVisualisation
    ${SRC_VIS_DIR}/BoundingView
    ${SRC_VIS_DIR}/BoundingVisualisation
    ${SRC_VIS_DIR}/EuclideanDistanceVisualiser
    ${SRC_VIS_DIR}/FaceView
    ${SRC_VIS_DIR}/LabelsView
    ${SRC_VIS_DIR}/LandmarkSetView
    ${SRC_VIS_DIR}/LandmarksVisualisation
    ${SRC_VIS_DIR}/LoopView
    ${SRC_VIS_DIR}/MetricVisualiser
    ${SRC_VIS_DIR}/OutlinesVisualisation
    ${SRC_VIS_DIR}/PathView
    ${SRC_VIS_DIR}/PathSetView
    ${SRC_VIS_DIR}/PathSetVisualisation
    ${SRC_VIS_DIR}/PointsView
    ${SRC_VIS_DIR}/PolyLabelsView
    ${SRC_VIS_DIR}/RadialSelectVisualisation
    ${SRC_VIS_DIR}/RegionVisualiser
    ${SRC_VIS_DIR}/SimpleView
    ${SRC_VIS_DIR}/SphereView
    ${SRC_VIS_DIR}/SurfaceMetricsMapper
    ${SRC_VIS_DIR}/SurfaceVisualisation
    ${SRC_VIS_DIR}/VertexLabelsView

    ${SRC_WIDGET_DIR}/ChartDialog
    ${SRC_WIDGET_DIR}/CheckAllTableHeader
    ${SRC_WIDGET_DIR}/DetectionCheckDialog
    ${SRC_WIDGET_DIR}/IntTableWidgetItem
    ${SRC_WIDGET_DIR}/MetricsDialog
    ${SRC_WIDGET_DIR}/ModelPropertiesDialog
    ${SRC_WIDGET_DIR}/PhenotypesDialog
    ${SRC_WIDGET_DIR}/ReportChooserDialog
    ${SRC_WIDGET_DIR}/ResizeDialog
    ${SRC_WIDGET_DIR}/ScanInfoDialog

    #${SRC_DIR}/CurvatureSpeedFunctor
    #${SRC_DIR}/CurvatureVariableSpeedFunctor
    ${SRC_DIR}/Ethnicities
    ${SRC_DIR}/FaceAssessment
    ${SRC_DIR}/FaceModel
    ${SRC_DIR}/FaceModelCurvature
    ${SRC_DIR}/FaceModelViewer
    ${SRC_DIR}/FaceTypes
    ${SRC_DIR}/FaceViewSet
    ${SRC_DIR}/MiscFunctions
    ${SRC_DIR}/ModelViewer
    ${SRC_DIR}/ModelViewerAnnotator
    ${SRC_DIR}/MultiFaceModelViewer
    #${SRC_DIR}/SurfaceMesher
    ${SRC_DIR}/Path
    ${SRC_DIR}/PathSet
    ${SRC_DIR}/U3DCache
    )

set( RCC_FILE "FaceTools_res.qrc")

qt5_wrap_ui( FORM_HEADERS ${FORMS})
qt5_wrap_cpp( QOBJECT_MOCS ${QOBJECTS})
qt5_add_resources( RESOURCE_FILE "${RCC_FILE}")

add_library( ${PROJECT_NAME} ${SRC_FILES} ${QOBJECT_MOCS} ${INCLUDE_FILES} ${FORM_HEADERS} ${FORMS} ${RESOURCE_FILE} ${RCC_FILE})
include( "cmake/LinkLibs.cmake")

install( DIRECTORY "${PROJECT_SOURCE_DIR}/haarcascades" DESTINATION "${LIB_PRE_REQS}/${PROJECT_NAME}")
install( FILES "${PROJECT_SOURCE_DIR}/shape_predictor_68_face_landmarks.dat" DESTINATION "${LIB_PRE_REQS}/${PROJECT_NAME}")
