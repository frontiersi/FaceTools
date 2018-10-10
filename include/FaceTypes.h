/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#ifndef FACE_TOOLS_FACE_TYPES_H
#define FACE_TOOLS_FACE_TYPES_H

#include "FaceTools_Export.h"
#include <ObjModelCurvatureMetrics.h>   // RFeatures
#include <vtkSmartPointer.h>
#include <QMetaType>
#include <QWidget>
#include <QString>
#include <QStringList>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <functional>
#include <unordered_set>
#include <unordered_map>

/************** FaceTools type declarations and typedefs **************/

namespace FaceTools {

class FaceModel;
using FM = FaceModel;
using FaceModelSet = std::unordered_set<FaceModel*>;
using FMS = FaceModelSet;

class FaceViewSet;
using FVS = FaceViewSet;

class ModelViewer;
class FaceModelViewer;
using FMV = FaceModelViewer;
using FaceModelViewerSet = std::unordered_set<FaceModelViewer*>;
using FMVS = FaceModelViewerSet;

enum Sex : int8_t
{
    UNKNOWN_SEX = 0,
    FEMALE_SEX = 1,
    MALE_SEX = 2,
    INTERSEX = 4
};  // end enum

std::string toSexString( Sex);  // any of "F M", "F", "M"
QString toLongSexString( Sex);  // any of "Female / Male", "Female", "Male"
Sex fromSexString( const std::string&);

static const char IBAR = '|';
static const char SC = ';';

enum FaceLateral : int8_t
{
    FACE_LATERAL_MEDIAL = 1,
    FACE_LATERAL_LEFT = 2,
    FACE_LATERAL_RIGHT = 4
};  // end enum


namespace FileIO {

class FaceModelAssImpFileHandlerInterface;
class FaceModelAssImpFileHandler;
class FaceModelAssImpFileHandlerFactory;
class FaceModelFileHandlerInterface;
class FaceModelFileHandlerMap;
class FaceModelManager;

}   // end namespace (FileIO)

namespace Vis {

class FaceView;
using FV = FaceView;
class VisualisationInterface;
class BaseVisualisation;
class MetricVisualiser;
class SurfaceVisualisation;
class SurfaceDataMapper;
using VisualisationLayers = std::unordered_set<BaseVisualisation*>;
using MetricVisSet = std::unordered_set<MetricVisualiser*>;

}   // end namespace (Vis)

namespace Interactor {

class ModelViewerInteractor;
using MVI = ModelViewerInteractor;
class ModelEntryExitInteractor;
using MEEI = ModelEntryExitInteractor;

}   // end namespace (Interactor)

namespace Detect {

class FaceDetector;

}   // end namespace (Detect)

namespace Action {

class EventProcessResponse;
using EPR = EventProcessResponse;
class FaceActionInterface;
class FaceAction;
class FaceActionWorker;
class FaceActionManager;

enum EventId : int16_t
{
    NULL_EVENT,
    LOADED_MODEL,       // Can be used to specify that an action should process on load (via FaceAction::addProcessOn).
    CLOSE_MODEL,        // INFORM that one or more models should be closed (actions should not close models themselves).
    GEOMETRY_CHANGE,    // Change to underlying geometry of the model.
    SURFACE_DATA_CHANGE,// Change to results of cached calculations on the data (often after response to data change).
    LANDMARKS_CHANGE,   // Change to existing landmark(s).
    LANDMARKS_ADD,      // Added landmark(s).
    LANDMARKS_DELETE,   // Deleted landmarks(s).
    ORIENTATION_CHANGE, // Change to model's orientation.
    METRICS_CHANGE,     // Change to model metrics.
    PATHS_CHANGE,       // Change to drawn paths.
    VIEW_CHANGE,        // Changes to views of the data (visualisations etc) - NOT CAMERA.
    VIEWER_CHANGE,      // Changed viewer (or the vieweer's state) in which view is shown.
    CAMERA_CHANGE,      // Changes to camera parameters within a viewer.
    AFFINE_CHANGE,      // Change to an actor's position (affine transform).
    REPORT_CREATED,     // A report was just created.
    METADATA_CHANGE     // Any of a model's metadata changed (including age/ethnicity).
};  // end enum

using EventSet = std::unordered_set<EventId>;
using ResponsePredicate = std::function<bool(FVS&)>;
using ProcessFlagPredicate = std::function<bool(const FVS&)>;

}   // end namespace (Action)

namespace Report {

class ReportInterface;

}   // end namespace (Report)

namespace Metric {

class MetricCalculatorTypeInterface;
class MetricCalculator;
using MC = MetricCalculator;

}   // end namespace (Metric)

}   // end namespace (FaceTools)

/**********************************************************************/



/************ Declare plugin interfaces *************/

#define FaceToolsPluginFaceActionInterface_iid "com.github.frontiersi.FaceTools.v3.Action.FaceActionInterface"
Q_DECLARE_INTERFACE( FaceTools::Action::FaceActionInterface, FaceToolsPluginFaceActionInterface_iid)

#define FaceToolsPluginVisVisualisationInterface_iid "com.github.frontiersi.FaceTools.v3.Vis.VisualisationInterface"
Q_DECLARE_INTERFACE( FaceTools::Vis::VisualisationInterface, FaceToolsPluginVisVisualisationInterface_iid)

#define FaceToolsPluginReportReportInterface_iid "com.github.frontiersi.FaceTools.v3.Report.ReportInterface"
Q_DECLARE_INTERFACE( FaceTools::Report::ReportInterface, FaceToolsPluginReportReportInterface_iid)

#define FaceToolsPluginMetricMetricCalculatorTypeInterface_iid "com.github.frontiersi.FaceTools.v3.Metric.MetricCalculatorTypeInterface"
Q_DECLARE_INTERFACE( FaceTools::Metric::MetricCalculatorTypeInterface, FaceToolsPluginMetricMetricCalculatorTypeInterface_iid)

#define FaceToolsFileIOPluginFaceModelFileHandlerInterface_iid "com.github.frontiersi.FaceTools.v3.FileIO.FaceModelFileHandlerInterface"
Q_DECLARE_INTERFACE( FaceTools::FileIO::FaceModelFileHandlerInterface, FaceToolsFileIOPluginFaceModelFileHandlerInterface_iid)

/****************************************************/



/************* Hashing functions for std types ***************/

namespace std {

template <>
struct hash<QString>
{
    size_t operator()( const QString& x) const
    {
        return hash<std::string>()( x.toStdString());
    }   // end operator()
};  // end struct

template <class T>
struct hash<vtkSmartPointer<T> >
{
    size_t operator()( const vtkSmartPointer<T>& x) const
    {
        return hash<void*>()( x.GetPointer());
    }   // end operator()
};  // end struct

// Custom hash function for EventId (to allow use of EventSet)
template<>
struct hash<FaceTools::Action::EventId>
{
    size_t operator()( const FaceTools::Action::EventId& e) const { return hash<int>()((int)e);}
};  // end struct

}   // end namespace (std)

/*************************************************************/



/************** Other miscellaneous typedefs *****************/

using QStringSet = std::unordered_set<QString>;

/*************************************************************/



/************ Register MetaTypes with Qt ************/

Q_DECLARE_METATYPE( FaceTools::Action::EventId)
Q_DECLARE_METATYPE( FaceTools::Action::EventSet)

namespace FaceTools {
FaceTools_EXPORT void registerTypes();
}   // end namespace (FaceTools)

/****************************************************/


#endif
