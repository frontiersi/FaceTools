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

#ifndef FACE_TOOLS_ACTION_ORIENT_CAMERA_TO_FACE_H
#define FACE_TOOLS_ACTION_ORIENT_CAMERA_TO_FACE_H

#include "FaceAction.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionOrientCameraToFace : public FaceAction
{ Q_OBJECT
public:
    ActionOrientCameraToFace( const QString&, const QIcon&, double d=500., double r=0., const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Orient the camera to look at the selected face.";}

    // Set/get the distance in world units to the face centre point
    // for the new position of the camera when transformed.
    void setDistance( double d) { _distance = d;}
    double distance() const { return _distance;}

    // Set/get the angle about the face that will be viewed.
    // Angles are given about the face up vector looking toward
    // the negative side, with the face normal vector as the pseudo
    // up vector from that point of view. An angle of 0 radians is
    // straight up along the normal vector (default). An angle of
    // +pi/2 radians will cause the camera to look at the RIGHT
    // profile of the face and an angle of -pi/2 radians will cause
    // the camera to look at the LEFT profile of the face.
    void setAngleAboutUpVector( double rads) { _urads = rads;}
    double angleAboutUpVector() const { return _urads;}

    static void orientToFace( const Vis::FV*, double distance, double rads=0.0);

protected:
    bool checkEnable( Event) override;
    void doAction( Event) override;

private:
    double _distance;
    double _urads;
};  // end class

}}   // end namespace

#endif
