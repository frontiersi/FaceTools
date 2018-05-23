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
#include <unordered_set>

namespace FaceTools {
namespace Action {


enum ChangeEvent
{
    DATA_CHANGE,    // Changes to the underlying data that require a save to ensure peristence.
    CALC_CHANGE,    // Changes to the results of cached calculations on the data (often after response to data change).
    VIEW_CHANGE,    // Changes to views of the data (camera movements / visualisations etc).
};  // end enum

typedef std::unordered_set<ChangeEvent> ChangeEventSet;

}   // end namespace
}   // end namespace

// Custom hash function for ChangeEvent (to allow use of ChangeEventSet)
// just invokes std::hash<int>() on the internal change ID.
namespace std {
template<> struct hash<FaceTools::Action::ChangeEvent>
{
    size_t operator()( const FaceTools::Action::ChangeEvent& c) const { return hash<int>()((int)c);}
};  // end struct
}   // end namespace (std)


#endif
