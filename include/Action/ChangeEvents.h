/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

#ifndef FACE_TOOLS_ACTION_CHANGE_EVENTS_H
#define FACE_TOOLS_ACTION_CHANGE_EVENTS_H

/**
 * Change events are notifications of state changes that have been carried
 * out by some FaceAction. FaceAction types specify which of these actions
 * they carry out and which they respond to.
 * FaceActionManager will connect FaceAction instances together to ensure
 * that FaceAction instances that respond to certain events are signalled by
 * the FaceAction instances that perform those events.
 */

#include "FaceTools_Export.h"
#include <QMetaType>
#include <unordered_set>

namespace FaceTools {
namespace Action {


enum ChangeId
{
    NULL_EVENT,
    LOADED_MODEL,       // Can be used to specify that an action should process on load (via FaceAction::addProcessOn).
    CLOSE_MODEL,        // INFORM that one or more models should be closed (actions should not close models themselves).
    GEOMETRY_CHANGE,    // Change to underlying geometry of the model.
    SURFACE_DATA_CHANGE,// Change to results of cached calculations on the data (often after response to data change).
    LANDMARKS_CHANGE,   // Change to landmark data.
    ORIENTATION_CHANGE, // Change to model's orientation.
    METRICS_CHANGE,     // Change to model metrics (path data etc)
    VIEW_CHANGE,        // Changes to views of the data (visualisations etc) - NOT CAMERA.
    CAMERA_CHANGE,      // Changes to camera parameters.
    AFFINE_CHANGE,      // Change to an actor's position (affine transform).
    REPORT_CREATED      // A report was just created.
};  // end enum

struct ChangeEvent
{
    ChangeEvent() : id(NULL_EVENT), processFlag(false) {}
    ChangeEvent( ChangeId i, bool ps=true) : id(i), processFlag(ps) {}
    ChangeEvent( const ChangeEvent& c) : id(c.id), processFlag(c.processFlag) {}

    ChangeId id;
    bool processFlag;
    bool operator==( const ChangeEvent& ce) const { return id == ce.id;}
};  // end struct

typedef std::unordered_set<ChangeEvent> ChangeEventSet;

}   // end namespace
}   // end namespace


// Custom hash function for ChangeEvent (to allow use of ChangeEventSet)
// just invokes std::hash<int>() on the internal change ID.
namespace std {
template<> struct hash<FaceTools::Action::ChangeEvent>
{
    size_t operator()( const FaceTools::Action::ChangeEvent& c) const { return hash<int>()((int)c.id);}
};  // end struct
}   // end namespace (std)

Q_DECLARE_METATYPE( FaceTools::Action::ChangeEvent)
Q_DECLARE_METATYPE( FaceTools::Action::ChangeEventSet)

#endif
