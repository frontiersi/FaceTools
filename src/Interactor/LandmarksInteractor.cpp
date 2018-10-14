/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#include <LandmarksInteractor.h>
#include <ModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Interactor::LandmarksInteractor;
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::Interactor::MEEI;
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::Vis::LandmarkSetView;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::Path;


// public
LandmarksInteractor::LandmarksInteractor( MEEI* meei, LandmarksVisualisation* vis)
    : ModelViewerInteractor(), _meei(meei), _vis(vis), _drag(-1), _hover(-1), _lat(FACE_LATERAL_MEDIAL), _view(nullptr)
{
    connect( meei, &MEEI::onEnterProp, this, &LandmarksInteractor::doOnEnterLandmark);
    connect( meei, &MEEI::onLeaveProp, this, &LandmarksInteractor::doOnLeaveLandmark);
    setEnabled(false);
}   // end ctor


// private slot
void LandmarksInteractor::doOnEnterLandmark( const FV* fv, const vtkProp* p)
{
    int lm = _vis->landmarkId( fv, p, _lat);
    if ( lm < 0)
        return;

    _hover = lm;
    FM* fm = fv->data();
    _vis->setLandmarkHighlighted( fm, lm, _lat, true);
    fm->updateRenderers();
}   // end doOnEnterLandmark


// private slot
void LandmarksInteractor::doOnLeaveLandmark( const FV* fv, const vtkProp* p)
{
    if ( _hover < 0)
        return;

    const int lm = _vis->landmarkId( fv, p, _lat);
    FM* fm = fv->data();
    if ( _drag < 0 && lm >= 0)
    {
        _vis->setLandmarkHighlighted( fm, _hover, _lat, false);
        fm->updateRenderers();
    }   // end if
    _hover = -1;
}   // end doOnLeaveLandmark


bool LandmarksInteractor::leftButtonDown( const QPoint&)
{
    _drag = _hover;
    _view = hoverModel();
    bool swallowed = false;
    if ( _drag >= 0 && _view->pickable())
    {
        viewer()->setCursor(Qt::CrossCursor);
        _vis->setLandmarkHighlighted( _view->data(), _drag, _lat, true);
        swallowed = true;
    }   // end if
    return swallowed;
}   // end leftButtonDown


bool LandmarksInteractor::leftButtonUp( const QPoint&)
{
    if ( _drag >= 0)
    {
        _vis->setLandmarkHighlighted( _view->data(), _drag, _lat, false);
        viewer()->setCursor(Qt::ArrowCursor);
        emit onChangedData(_view);
    }   // end if
    _view = hoverModel();
    _drag = -1;
    return false;
}   // end leftButtonUp


bool LandmarksInteractor::leftDrag( const QPoint& p)
{
    if ( !_view || _drag < 0)
        return false;

    FV *fv = _view;
    cv::Vec3f hpos; // Get the position on the surface of the actor
    if ( !fv->projectToSurface( p, hpos))
        return false;

    // Update the position of the landmark
    FM* fm = fv->data();
    fm->landmarks()->set(_drag, hpos, _lat);
    _vis->updateLandmark( fm, _drag);
    fm->updateRenderers();
    return true;
}   // end leftDrag
