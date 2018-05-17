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

#include <CameraWorker.h>
#include <FaceModelViewer.h>
#include <Transformer.h>
#include <QThread>
#include <cmath>
using FaceTools::Action::CameraWorker;
using FaceTools::FaceModelViewer;
using RFeatures::Transformer;
using RFeatures::CameraParams;


// public
CameraWorker::CameraWorker( FaceModelViewer* v, size_t fps, double hs, double vs, double hmax, double vmax)
    : _viewer(v),
      _fps(fps),
      _hs(hs), _vs(vs),
      _ABS_MAX_H_RADS(hmax * CV_PI/180),
      _ABS_MAX_V_RADS(vmax * CV_PI/180),
      _cp( _viewer->getCamera()),
      _timer(NULL),
      _hdiff(0), _vdiff(0),
      _hangle(0), _vangle(0)
{
}   // end ctor


CameraWorker::~CameraWorker() { stop();}


void CameraWorker::start()
{
    stop(); // Ensure stopped
    _cp = _viewer->getCamera(); // The base camera

    qint64 mspf =  qint64(1000.0/_fps);
    _hdiff = 2.0*mspf*_ABS_MAX_H_RADS/(_hs * 1000);
    _vdiff = 2.0*mspf*_ABS_MAX_V_RADS/(_vs * 1000);

    _hangle = 0.0;
    _vangle = 0.0;

    _timer = new QTimer(this);
    connect( _timer, &QTimer::timeout, this, &CameraWorker::createFrame);
    _timer->start( mspf);
}   // end start


void CameraWorker::stop()
{
    if ( _timer)
        delete _timer;
    _timer = NULL;
}   // end stop


void CameraWorker::createFrame()
{
    _hangle += _hdiff;
    _vangle += _vdiff;

    // Reverse direction if reached horizontal/vertical bounds?
    if ( fabs(_hangle) > _ABS_MAX_H_RADS)
        _hdiff = -_hdiff;
    if ( fabs(_vangle) > _ABS_MAX_V_RADS)
        _vdiff = -_vdiff;

    cv::Vec3f cdir;
    cv::normalize( _cp.pos - _cp.focus, cdir);

    Transformer r0( _hangle, _cp.up); // Angle about camera vertical (left and right)
    Transformer r1( _vangle, _cp.up.cross(cdir)); // Angle about camera horizontal (up and down)

    r0.rotate( cdir);
    r1.rotate( cdir);
    cdir *= _viewer->cameraDistance();   // Set magnitude of position vector

    CameraParams cp = _cp;
    cp.pos = cp.focus + cdir;    // Set new camera position given current focus
    _viewer->setCamera(cp);
    _viewer->updateRender();
}   // end createFrame
