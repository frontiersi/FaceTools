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

#ifndef FACE_TOOLS_ACTION_ORIENT_CAMERA_TO_FACE_H
#define FACE_TOOLS_ACTION_ORIENT_CAMERA_TO_FACE_H

#include "ActionProcessModel.h"

namespace FaceTools {

class FaceTools_EXPORT ActionOrientCameraToFace : public ActionProcessModel
{ Q_OBJECT
public:
    ActionOrientCameraToFace( const std::string& dname="Orient Camera to Face", const std::string& iconfilename="");
    virtual ~ActionOrientCameraToFace(){}

    virtual bool operator()( FaceControl*);
    virtual bool isActionable( FaceControl*) const;
};  // end class

}   // end namespace

#endif


