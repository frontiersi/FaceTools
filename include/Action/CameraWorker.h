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

#ifndef FACE_TOOLS_CAMERA_WORKER_H
#define FACE_TOOLS_CAMERA_WORKER_H

#include <FaceTools_Export.h>
#include <QObject>
#include <QTimer>
#include <CameraParams.h>

namespace FaceTools {
class FaceModelViewer;

namespace Action {

class FaceTools_EXPORT CameraWorker : public QObject
{ Q_OBJECT
public:
    CameraWorker( FaceModelViewer* v,
            size_t framesPerSecond=60,  // Number of frames rendered per second
            double secsPerHCycle=20,    // Seconds to go through a full horizontal cycle
            double secsPerVCycle=7,     // Seconds to go through a full vertical cycle
            double hMaxDeg=60.0,        // Horizontal absolute max range (degrees)
            double vMaxDeg=10.0);       // Vertical absolute max range (degrees)
    ~CameraWorker() override;

    FaceModelViewer* viewer() { return _viewer;}

    void start();
    void stop();

private slots:
    void createFrame();

private:
    FaceModelViewer* _viewer;
    const size_t _fps;
    const double _hs;
    const double _vs;
    const double _ABS_MAX_H_RADS;
    const double _ABS_MAX_V_RADS;
    RFeatures::CameraParams _cp;
    QTimer *_timer;
    double _hdiff, _vdiff;
    double _hangle, _vangle;
};  // end class

}   // end namespace
}   // end namespace

#endif
