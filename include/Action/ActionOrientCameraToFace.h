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

#include "FaceAction.h"

namespace FaceTools {
namespace Action {
class ActionSynchroniseCameraMovement;

class FaceTools_EXPORT ActionOrientCameraToFace : public FaceAction
{ Q_OBJECT
public:
    ActionOrientCameraToFace( const QString& dname="Orient Camera to Face", const QIcon& icon=QIcon());

    // Set/get the distance in world units to the face centre point
    // for the new position of the camera when transformed.
    void setDistance( float d) { _distance = d;}
    float distance() const { return _distance;}

    // Set/get the angle about the face that will be viewed.
    // Angles are given about the face up vector looking toward
    // the negative side, with the face normal vector as the pseudo
    // up vector from that point of view. An angle of 0 radians is
    // straight up along the normal vector (default). An angle of
    // +pi/2 radians will cause the camera to look at the RIGHT
    // profile of the face and an angle of -pi/2 radians will cause
    // the camera to look at the LEFT profile of the face.
    void setAngleAboutUpVector( float rads) { _urads = rads;}
    float angleAboutUpVector() const { return _urads;}

    void setCamSynch( const ActionSynchroniseCameraMovement* cs) { _camSynch = cs;}
   
private slots:
    bool testReady( const FaceControl*);
    bool testEnabled() const override { return readyCount() == 1;}
    bool doAction( FaceControlSet&) override;
    void doAfterAction( ChangeEventSet& cs, const FaceControlSet&, bool) override { cs.insert(CAMERA_CHANGE);}

private:
    float _distance;
    float _urads;
    const ActionSynchroniseCameraMovement *_camSynch;
    bool displayDebugStatusProgression() const override { return false;}
};  // end class

}   // end namespace
}   // end namespace

#endif
