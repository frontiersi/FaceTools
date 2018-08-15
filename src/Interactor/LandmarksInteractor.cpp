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

#include <LandmarksInteractor.h>
#include <ModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Interactor::LandmarksInteractor;
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::Vis::LandmarkSetView;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::Path;


const QString LandmarksInteractor::s_defaultMsg( QObject::tr("Add/remove/rename landmarks from the context menu; reposition by left-click and dragging."));

const QString LandmarksInteractor::s_moveMsg( QObject::tr("Reposition with left-click and drag; right click to remove/rename the landmark."));

// public
LandmarksInteractor::LandmarksInteractor( FEEI* feei, LandmarksVisualisation* vis, QStatusBar* sbar)
    : ModelViewerInteractor( nullptr, sbar), _feei(feei), _vis(vis), _drag(-1), _hover(-1), _model(nullptr)
{
    connect( feei, &FEEI::onEnterLandmark, this, &LandmarksInteractor::doOnEnterLandmark);
    connect( feei, &FEEI::onLeaveLandmark, this, &LandmarksInteractor::doOnLeaveLandmark);
    setEnabled(false);
}   // end ctor


// public
int LandmarksInteractor::addLandmark( const std::string& lname, const QPoint& p)
{
    FaceControl *fc = hoverModel();
    if ( !fc)
        return -1;

    cv::Vec3f hpos; // Get the position of the new landmark by projecting p onto the surface of the model.
    if ( !viewer()->calcSurfacePosition( fc->view()->surfaceActor(), p, hpos))
        return -1;

    FaceModel* fm = fc->data();
    fm->lockForWrite(); // Add landmark
    int id = fm->landmarks()->set( lname, hpos);
    fm->setSaved(false);
    _vis->updateLandmark( fm, id);
    _vis->setLandmarkVisible( fm, id, true);
    fm->unlock();

    return id;
}   // end addLandmark


// public
bool LandmarksInteractor::deleteLandmark()
{
    int id = _hover;
    if ( id < 0)
        return false;

    FaceControl *fc = hoverModel();
    assert(fc);

    FaceModel* fm = fc->data();
    fm->lockForWrite();
    const bool removed = fm->landmarks()->erase(id);
    assert(removed);
    fm->setSaved(false);
    _vis->updateLandmark( fm, id);
    fm->unlock();

    _drag = -1;
    return true;
}   // end deleteLandmark


bool LandmarksInteractor::leftButtonDown( const QPoint& p)
{
    _drag = _hover;
    _model = hoverModel();
    if ( _drag >= 0)
    {
        viewer()->setCursor(Qt::CrossCursor);
        _vis->setLandmarkHighlighted( _model->data(), _drag, true);
        showStatus( s_moveMsg, 10000);
    }   // end if
    return _drag >= 0;
}   // end leftButtonDown


bool LandmarksInteractor::leftButtonUp( const QPoint& p)
{
    viewer()->setCursor(Qt::ArrowCursor);
    if ( _drag >= 0)
    {
        _vis->setLandmarkHighlighted( _model->data(), _drag, false);
        assert(_model);
        emit onChangedData(_model);
    }   // end if
    _model = hoverModel();
    _drag = -1;
    return false;
}   // end leftButtonUp


bool LandmarksInteractor::leftDrag( const QPoint& p)
{
    if ( !_model || _drag < 0)
        return false;

    FaceControl *fc = _model;
    // Get the position on the surface of the actor
    cv::Vec3f hpos;
    if ( !viewer()->calcSurfacePosition( fc->view()->surfaceActor(), p, hpos))
        return false;

    // Update the position of the landmark
    FaceModel* fm = fc->data();
    fm->lockForWrite();
    const bool setok = fm->landmarks()->set(_drag, hpos);
    assert(setok);
    _vis->updateLandmark( fm, _drag);
    fm->unlock();
    fm->updateRenderers();
    return true;
}   // end leftDrag


// private slot
void LandmarksInteractor::doOnEnterLandmark( const FaceControl* fc, int lm)
{
    assert( fc);
    assert( fc == hoverModel());
    FaceModel* fm = fc->data();
    _hover = lm;
    _vis->updateLandmark( fm, lm);
    _vis->setLandmarkHighlighted( fm, lm, true);
    showStatus( s_moveMsg, 10000);
    fm->updateRenderers();
}   // end doOnEnterLandmark


// protected
void LandmarksInteractor::onEnabledStateChanged( bool v)
{
    ModelViewerInteractor::onEnabledStateChanged(v);
    if ( v)
        showStatus( s_defaultMsg, 10000);
}   // end onEnabledStateChanged


// private slot
void LandmarksInteractor::doOnLeaveLandmark( const FaceControl* fc, int lm)
{
    assert( fc);
    assert( hoverModel() == fc);
    FaceModel* fm = fc->data();
    _hover = -1;
    if ( _drag < 0)
        _vis->setLandmarkHighlighted( fm, lm, false);
    showStatus( s_defaultMsg, 10000);
    fm->updateRenderers();
}   // end doOnLeaveLandmark
