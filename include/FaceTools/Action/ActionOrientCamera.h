/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_ACTION_ORIENT_CAMERA_H
#define FACE_TOOLS_ACTION_ORIENT_CAMERA_H

#include "FaceAction.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionOrientCamera : public FaceAction
{ Q_OBJECT
public:
    /**
     * Parameters:
     * p     : The distance proportion of the camera from the model bounding box.
     * raxis : The local axis of rotation 0=X, 1=Y, 2=Z (defaults to the Y axis)
     * rads  : The rotation amount in radians.
     */
    ActionOrientCamera( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence(),
                              float p=1.0f,
                              int raxis=1,
                              float rads=0.0f);

    // Set/get the distance proportion for the placement of the camera from the model focus given
    // the size of the model bounding box and the camera's field of view. The default proportion
    // causes the camera to be placed such that the model's bounding box will fill the viewer.
    // Values greater than 1.0 place the camera further away.
    void setDistanceProportion( float p) { _dprop = p;}
    float distanceProportion() const { return _dprop;}

    // If dprop is <= 0, the camera's existing distance proportion is used.
    static void orient( Vis::FV*, float dprop=0, int raxis=1, float rads=0.0f);

    // Return a front oriented camera for given model and fov with distance scaled by dscale.
    static r3d::CameraParams makeFrontCamera( const FM&, float fov, float dscale=1.0f);

protected:
    bool isAllowed( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private:
    float _dprop;
    int _raxis;
    float _urads;
    Event _e;
};  // end class

}}   // end namespace

#endif
