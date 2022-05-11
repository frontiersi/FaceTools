/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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
#include <Eigen/Dense>
#include <vtkSmartPointer.h>
#include <QtGlobal>
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

using byte = unsigned char;
using IntSet = std::unordered_set<int>;

using Vec2d = Eigen::Vector2d;
using Vec3d = Eigen::Vector3d;
using Vec4d = Eigen::Vector4d;

using Vec2f = Eigen::Vector2f;
using Vec3f = Eigen::Vector3f;
using Vec4f = Eigen::Vector4f;
using Mat3f = Eigen::Matrix3f;
using Mat4f = Eigen::Matrix4f;
using VecXf = Eigen::VectorXf;
using MatX3f = Eigen::Matrix<float, Eigen::Dynamic, 3>;

class FaceModel;
using FM = FaceModel;
using FaceModelSet = std::unordered_set<FM*>;
using FMS = FaceModelSet;

class FaceViewSet;
using FVS = FaceViewSet;

class ModelViewer;
class FaceModelViewer;
using FMV = FaceModelViewer;
using FaceModelViewerSet = std::unordered_set<FMV*>;
using FMVS = FaceModelViewerSet;

enum Sex : int8_t
{
    UNKNOWN_SEX = 0,    // Always Female | Male
    FEMALE_SEX = 1,
    MALE_SEX = 2,
};  // end enum

FaceTools_EXPORT QString toSexString( int8_t);  // "F M", "F", "M"
FaceTools_EXPORT QString toLongSexString( int8_t);  // "Female | Male", "Female", "Male"
FaceTools_EXPORT int8_t fromSexString( const QString&);

static const IntSet EMPTY_INT_SET;      // Useful empty set
static const IntSet COMPLETE_INT_SET;   // Useful "complete" set

static const char IBAR = '|';
static const char SC = ';';
static const QString ANY_ITEM = "-- Any --";

enum FaceSide : uint8_t
{
    MID = 1,
    LEFT = 2,
    RIGHT = 4
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

enum struct Event : uint32_t
{
    NONE =                     0x0, // A non-event event.
    USER =                     0x1, // Events triggered explicitly by the user (only used internally!)
    LOADED_MODEL =             0x2, // A model was loaded.
    SAVED_MODEL =              0x4, // A model was saved.
    CLOSED_MODEL =             0x8, // A model was closed.
    MESH_CHANGE =             0x10, // Change to the internal mesh of a FaceModel or fixing of its transform.
    AFFINE_CHANGE =           0x20, // Affine transform change to the model data.
    MASK_CHANGE =             0x40, // When the model is registered against a different mask.
    SURFACE_DATA_CHANGE =     0x80, // Change to some surface mapped data.
    LANDMARKS_CHANGE =       0x100, // Change to landmark(s).
    METRICS_CHANGE =         0x200, // Change to measurements.
    STATS_CHANGE =           0x400, // Change of subject demographic or measurement statistics.
    PATHS_CHANGE =           0x800, // Change to drawn paths.
    VIEW_CHANGE =           0x1000, // Changes to views of the data (visualisations etc) - NOT CAMERA.
    VIEWER_CHANGE =         0x2000, // Changed viewer (or the viewer's state) in which view is shown.
    CAMERA_CHANGE =         0x4000, // Changes to camera parameters within a viewer.
    ACTOR_MOVE =            0x8000, // Change to the position of a visualisation actor through interaction.
    METADATA_CHANGE =      0x10000, // Metadata change (also raise STATS_CHANGE for demographic changes).
    RESTORE_CHANGE =       0x20000, // Emitted after restoring any undo.
    ALL_VIEWS =            0x40000, // Specify that the event relates to all models in the selected viewer.
    ALL_VIEWERS =          0x80000, // Specify that all viewers partake in the event.
    MODEL_SELECT =        0x100000, // When a model has just been selected.
    CACHE =               0x200000, // Some caching operation
    CANCEL =              0x400000, // An action was cancelled from its doBeforeAction function.
    ERR =                 0x800000  // Some error.
};  // end enum


// Create a new Event as the union of two others.
FaceTools_EXPORT Event operator|( Event, Event);

// Add event e1 to e.
FaceTools_EXPORT void operator|=( Event &e, Event e1);

// Synonym for |= but also returning it for convenience.
FaceTools_EXPORT Event& add( Event &e, Event e1);

// Return the intersection of the two events.
FaceTools_EXPORT Event operator&( Event, Event);

// Returns true iff event e0 contains event e1 (and e1 isn't Event::NONE).
// E.g. if e0 == (ACTOR_MOVE | ALL_VIEWS | CAMERA_CHANGE) and e1 == (ACTOR_MOVE | CAMERA_CHANGE),
// this function will return true. But if ACTOR_MOVE or CAMERA_CHANGE (or both) are missing
// from e0, this function will return false.
FaceTools_EXPORT bool has( Event e0, Event e1);

// Returns true iff event e0 has one of the events of e1 (except Event::NONE).
// E.g. if e0 == (ACTOR_MOVE | ALL_VIEWS) and e1 == (ACTOR_MOVE | CAMERA_CHANGE),
// this function will return true because there's an intersection of ACTOR_MOVE.
FaceTools_EXPORT bool any( Event e0, Event e1);

// Print the name of the event.
FaceTools_EXPORT std::ostream &operator<<( std::ostream &os, Event);


// Make Event available to Qt's meta type system.
Q_ENUM_NS(Event)

}   // end namespace (Action)

namespace Metric {

class MetricType;
class Metric;

}   // end namespace (Metric)

}   // end namespace (FaceTools)


Q_DECLARE_METATYPE( FaceTools::Vec3f)
Q_DECLARE_METATYPE( FaceTools::FaceSide)

/**********************************************************************/



/************* Hashing functions for std types ***************/

namespace std {

// This hash function in the std namespace is defined in QtCore/qhashfunctions.h (204)
// in version 5.15.0 of Qt - it might be defined in earlier versions, but nothing lower
// than 5.12.3 (not present in this version on Linux).
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
template <>
struct hash<QString>
{
    size_t operator()( const QString& x) const
    {
        return hash<std::string>()( x.toStdString());
    }   // end operator()
};  // end struct
#endif

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
