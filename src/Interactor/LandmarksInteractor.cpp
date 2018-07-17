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
using FaceTools::Interactor::FaceHoveringInteractor;
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::Vis::LandmarkSetView;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::Path;


// public
LandmarksInteractor::LandmarksInteractor( FEEI* feei, LandmarksVisualisation* vis)
    : FaceHoveringInteractor( feei, vis), _vis(vis), _drag(-1), _hover(-1)
{
    connect( feei, &FEEI::onEnterLandmark, this, &LandmarksInteractor::doOnEnterLandmark);
    connect( feei, &FEEI::onLeaveLandmark, this, &LandmarksInteractor::doOnLeaveLandmark);
}   // end ctor


// public
int LandmarksInteractor::addLandmark( const std::string& lname)
{
    FaceControl *fc = hoverModel();
    if ( !fc)
        return -1;

    QPoint p = viewer()->getMouseCoords();
    cv::Vec3f hpos; // Get the position of the new landmark by projecting p onto the surface of the model.
    if ( !viewer()->calcSurfacePosition( fc->view()->surfaceActor(), p, hpos))
        return -1;

    FaceModel* fm = fc->data();
    fm->lockForWrite(); // Add landmark
    int id = fm->landmarks()->set( lname, hpos);
    fm->setSaved(false);
    fm->unlock();

    fm->lockForRead();
    const FaceControlSet& fcs = fm->faceControls();    // Add to the visualisations
    std::for_each( std::begin(fcs), std::end(fcs), [=](auto f){ _vis->refreshLandmark(f, id);});
    std::for_each( std::begin(fcs), std::end(fcs), [=](auto f){ _vis->setLandmarkVisible(f, id, true);});
    fm->unlock();

    return id;
}   // end addLandmark


// public
bool LandmarksInteractor::deleteLandmark()
{
    int id = hoverID();
    if ( id < 0)
        return false;

    FaceControl *fc = hoverModel();
    assert(fc);

    FaceModel* fm = fc->data();
    fm->lockForWrite();
    const bool removed = fm->landmarks()->erase(id);
    assert(removed);
    fm->setSaved(false);

    const FaceControlSet& fcs = fm->faceControls();    // Remove from the visualisations
    std::for_each( std::begin(fcs), std::end(fcs), [&](auto f){ _vis->refreshLandmark(f, id);});

    fm->unlock();

    _drag = -1;
    return true;
}   // end deletePath


bool LandmarksInteractor::setDrag( int id, const QPoint& p)
{
    FaceControl *fc = hoverModel();
    if ( !fc)
        return false;

    _drag = id;
    leftDrag( p);
    return true;
}   // end setDrag


bool LandmarksInteractor::leftButtonDown( const QPoint& p)
{
    leavingModel();
    _drag = _hover;
    if ( _drag >= 0)
    {
        viewer()->setCursor(Qt::CrossCursor);
        FaceControl *fc = hoverModel();
        assert(fc);
        FaceModel* fm = fc->data();
        fm->lockForRead();
        _origPos = fm->landmarks()->pos(_drag);
        fm->unlock();
    }   // end if
    return _drag >= 0;
}   // end leftButtonDown


bool LandmarksInteractor::leftButtonUp( const QPoint& p)
{
    leavingModel();
    viewer()->setCursor(Qt::ArrowCursor);
    return false;
}   // end leftButtonUp


void LandmarksInteractor::leavingModel()
{
    FaceControl *fc = hoverModel();
    assert(fc);
    bool isChanged = false;
    if ( _drag >= 0) // Did the postion of landmark change?
    {
        FaceModel* fm = fc->data();
        fm->lockForRead();
        isChanged = fm->landmarks()->pos(_drag) != _origPos;
        fm->unlock();
        _drag = -1;
    }   // end if

    if ( isChanged)
        emit onChangedData(fc);
}   // end leavingModel


bool LandmarksInteractor::leftDrag( const QPoint& p)
{
    if ( _drag < 0)
        return false;

    // Get the position on the surface of the actor
    FaceControl *fc = hoverModel();
    assert(fc);
    cv::Vec3f hpos;
    if ( !viewer()->calcSurfacePosition( fc->view()->surfaceActor(), p, hpos))
        return false;

    // Update the position of the landmark
    FaceModel* fm = fc->data();
    fm->lockForWrite();
    const bool setok = fm->landmarks()->set(_drag, hpos);
    assert(setok);

    // Update visualisation over all associated FaceControls
    for ( FaceControl* f : fm->faceControls())
        _vis->refreshLandmark( f, _drag);

    fm->unlock();
    fm->updateRenderers();
    return true;
}   // end leftDrag


// private slot
void LandmarksInteractor::doOnEnterLandmark( const FaceControl* fc, int lm)
{
    assert(fc == hoverModel());
    _hover = lm;
    for ( auto f : fc->data()->faceControls())
    {
        _vis->refreshLandmark( f, lm);
        _vis->setLandmarkHighlighted( f, lm, true);
    }   // end for
    fc->data()->updateRenderers();
}   // end doOnEnterLandmark


// private slot
void LandmarksInteractor::doOnLeaveLandmark( const FaceControl* fc, int lm)
{
    assert(fc == hoverModel());
    _hover = -1;
    for ( auto f : fc->data()->faceControls())
        _vis->setLandmarkHighlighted( f, lm, false);
    fc->data()->updateRenderers();
}   // end doOnLeaveLandmark
