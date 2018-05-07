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
 *
 * Since the ChangeEvent tags are simple ints (given here as enums for
 * readability), it is possible to define new event types and FaceAction
 * types that change and respond to these events.
 */

#include "FaceTools_Export.h"
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace FaceTools {
namespace Action {
class ChangeEvent;
}   // end namespace
}   // end namespace


namespace FaceTools {
namespace Action {


enum ChangeID
{
    // Camera changes
    CAMERA_FOCUS_CHANGED,
    CAMERA_POSITION_CHANGED,
    CAMERA_ORIENTATION_CHANGED,
    // Data (FaceModel) changes
    MODEL_GEOMETRY_CHANGED,         // For changes to the shape only
    MODEL_TRANSFORMED,              // Affine transformation of the model
    MODEL_TEXTURE_CHANGED,          // Changes to the surface texture mapping (ObjModel)
    LANDMARK_ADDED,
    LANDMARK_DELETED,
    LANDMARK_CHANGED,               // E.g. name, position, metadata
    FACE_NOTE_ADDED,
    FACE_NOTE_DELETED,
    FACE_NOTE_CHANGED,
    MODEL_ORIENTATION_CHANGED,      // Updated face orientation vectors
    MODEL_DESCRIPTION_CHANGED,
    MODEL_SOURCE_CHANGED,
    // Visualisation
    VISUALISATION_CHANGED,          // Visualisation changed (any)
};  // end enum


struct ChangeEvent
{
    ChangeEvent( ChangeID id);

    // Define new ChangeEvents - cannot be an extant ChangeID.
    ChangeEvent( int id, const std::string& d);

    int id() const;
    const std::string& description() const;
    bool operator==( const ChangeEvent& c) const;
private:
    int _id;
    static std::unordered_map<int, std::string> s_descs;
    static void s_init();
};  // end struct

typedef std::unordered_set<ChangeEvent> ChangeEventSet;

}   // end namespace
}   // end namespace


// Custom hash function for ChangeEvent (to allow use of ChangeEventSet)
// just invokes std::hash<int>() on the internal change ID.
namespace std {
template<> struct hash<FaceTools::Action::ChangeEvent>
{
    size_t operator()( const FaceTools::Action::ChangeEvent& c) const { return hash<int>()(c.id());}
};  // end struct
}   // end namespace (std)


#endif
