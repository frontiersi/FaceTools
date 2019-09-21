/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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
#include <ObjModelTools.h>   // RFeatures
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

/************** FaceTools type and using declarations **************/
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
};  // end enum

FaceTools_EXPORT QString toSexString( int8_t);// any of "U", "F M", "F", "M"
FaceTools_EXPORT int8_t fromSexString( const QString&);

FaceTools_EXPORT QString toLongSexString( int8_t);// any of "Unknown", "Female | Male", "Female", "Male"
FaceTools_EXPORT int8_t fromLongSexString( const QString&);

static const IntSet EMPTY_INT_SET;      // Useful empty set
static const IntSet COMPLETE_INT_SET;   // Useful "complete" set

static const char IBAR = '|';
static const char SC = ';';

enum FaceLateral : uint8_t
{
    FACE_LATERAL_MEDIAL = 1,
    FACE_LATERAL_LEFT = 2,
    FACE_LATERAL_RIGHT = 4
};  // end enum


namespace FileIO {

class FaceModelFileHandler;
class FaceModelFileHandlerMap;
class FaceModelManager;
}   // end namespace (FileIO)

namespace Vis {

class FaceView;
using FV = FaceView;
class BaseVisualisation;
class MetricVisualiser;
class SurfaceVisualisation;
class SurfaceMetricsMapper;
using VisualisationLayers = std::unordered_set<BaseVisualisation*>;
using MetricVisSet = std::unordered_set<MetricVisualiser*>;
}   // end namespace (Vis)

namespace Action {
FaceTools_EXPORT Q_NAMESPACE
// Macro Q_NAMESPACE allows use of Q_ENUM_NS for Event


class UndoState;
class UndoStates;
class FaceAction;
class FaceModelState;
class FaceActionWorker;
class FaceActionManager;
using FAM = FaceActionManager;

enum struct Event : int
{
    NONE = 0x0,
    USER = 0x1,                     // Events triggered explicitly by the user (only used internally!)
    LOADED_MODEL = 0x2,             // Can be used to specify that an action should process on load (via FaceAction::addProcessOn).
    SAVED_MODEL = 0x4,              // Emitted immediately after saving a model or models.
    CLOSED_MODEL = 0x8,             // Emitted immediately after closing a model or models.
    FACE_DETECTED = 0x10,           // After face has been successfully detected and orientation and landmarks set.
    GEOMETRY_CHANGE = 0x20,         // Non-affine change to the underlying geometry of the model without changing graph connectivity.
    CONNECTIVITY_CHANGE = 0x40,     // Emit together with GEOMETRY_CHANGE to notify that graph connectivity has been altered.
    AFFINE_CHANGE = 0x80,           // Affine transform change to the model data (shear, scale, rotate, reflect, translate).
    ORIENTATION_CHANGE = 0x100,     // When the model's orientation (transform) becomes fixed.
    SURFACE_DATA_CHANGE = 0x200,    // Change to some surface mapped data.
    LANDMARKS_CHANGE = 0x400,       // Change to landmark(s) (implies orientation change).
    METRICS_CHANGE = 0x800,         // Change to measurements - either measurements taken, or measurement parameters.
    STATISTICS_CHANGE = 0x1000,     // Change to statistics.
    PATHS_CHANGE = 0x2000,          // Change to drawn paths.
    VIEW_CHANGE = 0x4000,           // Changes to views of the data (visualisations etc) - NOT CAMERA.
    VIEWER_CHANGE = 0x8000,         // Changed viewer (or the viewer's state) in which view is shown.
    CAMERA_CHANGE = 0x10000,        // Changes to camera parameters within a viewer.
    ACTOR_MOVE = 0x20000,           // Change to the position of a visualisation actor through interaction.
    METADATA_CHANGE = 0x40000,      // Any of a model's metadata changed (including age/ethnicity).
    ASSESSMENT_CHANGE = 0x80000,    // Assessment change either in name, notes, landmarks, or path data.
    U3D_MODEL_CHANGE = 0x100000,    // The U3D model associated with a FaceModel has been updated.
    ALL_VIEWS = 0x200000,           // Specify that the event relates to all models in the selected viewer.
    ALL_VIEWERS = 0x400000,         // Specify that all viewers should be considered as "selected" and partaking in the event.
    ACT_CANCELLED = 0x800000,       // An action was cancelled from its doBeforeAction function.
    ACT_COMPLETE = 0x1000000,       // An action completed after running doAfterAction and refreshing its state.
    MODEL_SELECT = 0x2000000        // When a model has just been selected.
};  // end enum

// Make Event available to Qt's meta type system.
Q_ENUM_NS(Event)


// Inherit from EventGroup to test for the presence of individual events
struct FaceTools_EXPORT EventGroup
{
    EventGroup();
    EventGroup( Event);
    EventGroup( std::initializer_list<Event>);

    EventGroup( const EventGroup&) = default;
    EventGroup& operator=( const EventGroup&) = default;
    virtual ~EventGroup();

    Event event() const { return _E;}

    // Return human readable name of the event(s).
    std::string name() const;

    // Combine event(s) with this one and return the union.
    Event add( EventGroup);

    // Returns true if there exists an intersection of some subset of
    // events between this event group and the parameter event group.
    bool has( EventGroup e) const;

    // Returns true iff e == _E (_E is exactly e)
    bool is( EventGroup e) const;

private:
    Event _E;
};  // end struct

}   // end namespace (Action)

namespace Metric {

class MetricCalculatorType;
class MetricCalculator;
using MC = MetricCalculator;

}   // end namespace (Metric)

namespace Interactor {
class ContextMenu;
}   // end namespace (Interactor)

}   // end namespace (FaceTools)

Q_DECLARE_METATYPE( FaceTools::FaceLateral)
Q_DECLARE_METATYPE( cv::Vec3f);

/**********************************************************************/



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

template <>
struct hash<FaceTools::Action::Event>
{
    //size_t operator()( const FaceTools::Action::Event& x) const
    size_t operator()( FaceTools::Action::Event x) const
    {
        return hash<int>()( int(x));
    }   // end operator()
};  // end struct

}   // end namespace (std)

/*************************************************************/



/************** Other miscellaneous typedefs *****************/

using QStringSet = std::unordered_set<QString>;

/*************************************************************/

#endif
