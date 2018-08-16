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

#include <RadialSelectInteractor.h>
#include <RadialSelectVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Interactor::RadialSelectInteractor;
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::Vis::RadialSelectVisualisation;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


const QString RadialSelectInteractor::s_msg( tr("Reposition radial area by left-click and dragging the centre; change radius using the mouse wheel."));


// public
RadialSelectInteractor::RadialSelectInteractor( MEEI* meei, RadialSelectVisualisation* vis, QStatusBar* sbar)
    : ModelViewerInteractor( nullptr, sbar), _meei(meei), _vis(vis), _move(false)
{
    setEnabled(false);
}   // end ctor


bool RadialSelectInteractor::leftButtonDown( const QPoint& p)
{
    _move = testOnReticule( p); // _move set okay if the point is on the centre reticule
    _vis->setPickable( _meei->model(), false); // So projecting points onto the face works
    return _move;
}   // end leftButtonDown


bool RadialSelectInteractor::leftButtonUp( const QPoint&)
{
    _vis->setPickable( _meei->model(), true);
    return _move = false;
}   // end leftButtonUp


// Left drag (but only after _move flag set with leftButtonDown)
// to reposition the centre of the boundary.
bool RadialSelectInteractor::leftDrag( const QPoint& p)
{
    FaceControl* fc = _meei->model();
    if ( fc && _move)
    {
        cv::Vec3f v;
        if ( fc->view()->pointToFace( p, v))
        {
            _vis->setCentre( fc->data(), v);
            fc->data()->updateRenderers();
        }   // end if
    }   // end if
    return _move;
}   // end leftDrag


// Increase the radius
bool RadialSelectInteractor::mouseWheelForward( const QPoint& p)
{
    const bool changeRadius = testOnReticule(p);
    if ( changeRadius)
    {
        const FaceModel* fm = _meei->model()->data();
        _vis->setRadius( fm, _vis->radius(fm)+1);
        fm->updateRenderers();
    }   // end if
    return changeRadius;
}   // end mouseWheelForward


// Decrease the radius
bool RadialSelectInteractor::mouseWheelBackward( const QPoint& p)
{
    const bool changeRadius = testOnReticule(p);
    if ( changeRadius)
    {
        const FaceModel* fm = _meei->model()->data();
        _vis->setRadius( fm, _vis->radius(fm)-1);
        fm->updateRenderers();
    }   // end if
    return changeRadius;
}   // end mouseWheeBackward


bool RadialSelectInteractor::mouseMove( const QPoint& p)
{
    const bool onReticule = testOnReticule(p);
    if ( onReticule)
        showStatus( s_msg, 10000);

    FaceControl* fc = _meei->model();
    if ( fc)
    {
        const FaceModel* fm = fc->data();
        for ( FaceControl* f : fm->faceControls())
            _vis->setHighlighted( f, onReticule);
        fm->updateRenderers();
    }   // end if
    return false;
}   // end mouseMove


// protected
void RadialSelectInteractor::onEnabledStateChanged( bool v)
{
    if ( v)
        showStatus( s_msg, 10000);    // 10 seconds display time
    else
        clearStatus();
}   // end onEnabledStateChanged


// private
bool RadialSelectInteractor::testOnReticule( const QPoint& p) const
{
    FaceControl* fc = _meei->model();
    return fc && _vis->belongs( fc->viewer()->getPointedAt(p), fc);
}   // end testOnReticule

