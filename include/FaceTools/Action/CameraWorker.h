/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_CAMERA_WORKER_H
#define FACE_TOOLS_CAMERA_WORKER_H

#include <FaceTools_Export.h>
#include <QObject>
#include <QTimer>
#include <r3d/CameraParams.h>

namespace FaceTools {
class FaceModelViewer;

namespace Action {

class FaceTools_EXPORT CameraWorker : public QObject
{ Q_OBJECT
public:
    CameraWorker( FaceModelViewer* v,
            size_t framesPerSecond=60, // Number of frames rendered per second
            float secsPerHCycle=20,    // Seconds to go through a full horizontal cycle
            float secsPerVCycle=7,     // Seconds to go through a full vertical cycle
            float hMaxDeg=60.0,        // Horizontal absolute max range (degrees)
            float vMaxDeg=10.0);       // Vertical absolute max range (degrees)
    ~CameraWorker() override;

    FaceModelViewer* viewer() { return _viewer;}

    void start();
    void stop();

    bool isStarted() const { return _timer != nullptr;}

private slots:
    void createFrame();

private:
    FaceModelViewer* _viewer;
    const size_t _fps;
    const float _hs;
    const float _vs;
    const float _ABS_MAX_H_RADS;
    const float _ABS_MAX_V_RADS;
    r3d::CameraParams _cp;
    QTimer *_timer;
    float _hdiff, _vdiff;
    float _hangle, _vangle;
};  // end class

}}   // end namespace

#endif
