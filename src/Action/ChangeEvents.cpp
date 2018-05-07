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

std::unordered_map<int, std::string> ChangeEvent::s_descs;

void ChangeEvent::s_init()
{
    if ( !s_descs.empty())
        return;

    s_descs[CAMERA_FOCUS_CHANGED] = "Camera focus changed";
    s_descs[CAMERA_POSITION_CHANGED] = "Camera position changed";
    s_descs[CAMERA_ORIENTATION_CHANGED] = "Camera orientation changed";

    s_descs[LANDMARK_ADDED] = "Landmark added";
    s_descs[LANDMARK_DELETED] = "Landmark deleted";
    s_descs[LANDMARK_CHANGED] = "Landmark changed";

    s_descs[FACE_NOTE_ADDED] = "Face-note added";
    s_descs[FACE_NOTE_DELETED] = "Face-note deleted";
    s_descs[FACE_NOTE_CHANGED] = "Face-note changed";

    s_descs[MODEL_TEXTURE_CHANGED] = "Model texture changed";
    s_descs[MODEL_GEOMETRY_CHANGED] = "Model geometry changed";
    s_descs[MODEL_TRANSFORMED] = "Model transformed";
    s_descs[MODEL_ORIENTATION_CHANGED] = "Model orientation changed";
    s_descs[MODEL_DESCRIPTION_CHANGED] = "Model description changed";
    s_descs[MODEL_SOURCE_CHANGED] = "Model source changed";

    s_descs[VISUALISATION_CHANGED] = "Visualisation changed";
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
