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

#include <ChangeEvents.h>
using FaceTools::Action::ChangeEvent;
using FaceTools::Action::ChangeID;


std::unordered_map<int, std::string> ChangeEvent::s_descs;

void ChangeEvent::s_init()
{
    if ( !s_descs.empty())
        return;

    s_descs[CAMERA_FOCUS_CHANGED] = "Camera Focus Changed";
    s_descs[CAMERA_POSITION_CHANGED] = "Camera Position Changed";
    s_descs[CAMERA_ORIENTATION_CHANGED] = "Camera Orientation Changed";

    s_descs[LANDMARK_ADDED] = "Landmark Added";
    s_descs[LANDMARK_DELETED] = "Landmark Deleted";
    s_descs[LANDMARK_CHANGED] = "Landmark Changed";

    s_descs[FACE_NOTE_ADDED] = "Face-note Added";
    s_descs[FACE_NOTE_DELETED] = "Face-note Deleted";
    s_descs[FACE_NOTE_CHANGED] = "Face-note Changed";

    s_descs[MODEL_TEXTURE_CHANGED] = "Model Texture Changed";
    s_descs[MODEL_GEOMETRY_CHANGED] = "Model Geometry Changed";
    s_descs[MODEL_TRANSFORMED] = "Model Transformed";
    s_descs[MODEL_ORIENTATION_CHANGED] = "Model Orientation Changed";
    s_descs[MODEL_DESCRIPTION_CHANGED] = "Model Description Changed";
    s_descs[MODEL_SOURCE_CHANGED] = "Model Source Changed";

    s_descs[VISUALISATION_CHANGED] = "Visualisation Changed";
    s_descs[SURFACE_METRICS_CALCULATED] = "Surface Metrics Calculated";
}   // end s_init


ChangeEvent::ChangeEvent( ChangeID id) : _id(id)
{
    s_init();   // Ensure static initialiser run
}   // end ctor


ChangeEvent::ChangeEvent( int id, const std::string& d) : _id(-1)
{
    s_init();   // Ensure static initialiser run
    if ( s_descs.count(id) == 0)
    {
        s_descs[id] = d;
        _id = id;
    }   // end if
}   // end ctor

int ChangeEvent::id() const { return _id;}

const std::string& ChangeEvent::description() const { return s_descs.at(id());}

bool ChangeEvent::operator==( const ChangeEvent& c) const { return c.id() == id();}
