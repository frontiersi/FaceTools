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

#include <Action/ModelSelector.h>
#include <Action/ActionResetCamera.h>
#include <Action/ActionOrientCameraToFace.h>
#include <Vis/BoundingVisualisation.h>
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <cassert>
using FaceTools::Action::ModelSelector;
using FaceTools::Interactor::SelectNotifier;
using FaceTools::Interactor::MouseHandler;
using FaceTools::ModelViewer;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FMS;
using FaceTools::FM;

ModelSelector::Ptr ModelSelector::_me;


void ModelSelector::addViewer( FMV* fmv, bool setDefault)
{
    me()->_viewers.push_back(fmv);
    if ( setDefault)
        me()->_defv = int(me()->_viewers.size()) - 1;
}   // end addViewer


void ModelSelector::setStatusBar( QStatusBar* sb) { me()->_sbar = sb;}

void ModelSelector::showStatus( const QString& msg, int timeOut)
{
    if ( me()->_sbar)
        me()->_sbar->showMessage( QObject::tr( msg.toStdString().c_str()), timeOut);
}   // end showStatus

void ModelSelector::clearStatus()
{
    if ( me()->_sbar)
        me()->_sbar->clearMessage();
}   // end clearStatus


void ModelSelector::setCursor( Qt::CursorShape cs)
{
    for ( FMV* fmv : me()->_viewers)
        fmv->setCursor(cs);
}   // end setCursor


void ModelSelector::restoreCursor()
{
    if ( defaultViewer()->interactionMode() == IMode::ACTOR_INTERACTION)
        setCursor( Qt::CursorShape::DragMoveCursor);
    else
        setCursor( Qt::CursorShape::ArrowCursor);
}   // end restoreCursor


const SelectNotifier* ModelSelector::selector() { return &sn();}

void ModelSelector::setInteractionMode(IMode m, bool v)
{
    //std::cerr << "SET INTERACTION MODE TO: " << (m == IMode::ACTOR_INTERACTION ? "ACTOR" : "CAMERA") << std::endl;
    setCursor( Qt::CursorShape::ArrowCursor);
    if ( m == IMode::ACTOR_INTERACTION)
        setCursor( Qt::CursorShape::DragMoveCursor);
    for ( ModelViewer* mv : viewers())
        mv->setInteractionMode(m, v);
}   // end setInteractionMode

IMode ModelSelector::interactionMode()
{
    IMode m = IMode::CAMERA_INTERACTION;
    if ( !viewers().empty())
        m = viewers().at(0)->interactionMode();
    return m;
}   // end interactionMode


FMV* ModelSelector::mouseViewer() { return static_cast<FMV*>(sn().mouseViewer());}

FMV* ModelSelector::defaultViewer()
{
    if ( me()->_defv < 0)
    {
        assert(false);
        return nullptr;
    }   // end if
    return me()->_viewers.at(size_t(me()->_defv));
}   // end defaultViewer

const std::vector<FMV*>& ModelSelector::viewers() { return me()->_viewers;}

FV* ModelSelector::selectedView() { return sn().selected();}


FV* ModelSelector::addFaceView( FM* fm, FMV* tv)
{
    // If the initial viewer isn't specified, use the same viewer as the currently selected view.
    // If there is no currently selected view, use the default viewer.
    if ( !tv)
    {
        if ( selectedView())
            tv = selectedView()->viewer();
        else if ( me()->_defv >= 0)
        {
            tv = defaultViewer();
            assert(tv);
        }   // end else if
        else
            tv = mouseViewer();
    }   // end if
    // Attaches the viewer and creates the base models (calls FaceView::reset)
    FV* fv = new FV( fm, tv);
    // Called *after* viewer is attached and causes doOnSelect(fv, true) to be called via signal.
    sn().add(fv);
    return fv;
}   // end addFaceView


void ModelSelector::removeFaceView( FV* fv)
{
    // Called *before* viewer is detached and causes doOnSelect( fv, false) to be called via signal.
    sn().remove(fv);
    delete fv;  // Ensures all visualisations removed
}   // end removeFaceView


void ModelSelector::remove( const FM* fm)
{
    while ( !fm->fvs().empty())
        removeFaceView( fm->fvs().first());
}   // end remove


void ModelSelector::setSelected( FV* fv)
{
    FV* sv = selectedView();
    if ( sv) // First deselect any currently selected view.
        sn().setSelected( sv, false);
    if ( fv) // The select the given view.
        sn().setSelected( fv, true);
}   // end setSelected


void ModelSelector::setAutoFocusOnSelectEnabled( bool v) { me()->_autoFocus = v;}


void ModelSelector::setShowBoundingBoxesOnSelected( bool v)
{
    me()->_showBoxes = v;

    // Do nothing if the SelectNotifier is not yet constructed
    if ( me()->_sn == nullptr)
        return;

    FV *fv = me()->selectedView();
    if ( fv)
    {
        me()->_doOnSelected( fv, true); // Refresh presence of bounding box around selected view
        fv->viewer()->updateRender();
    }   // end if
}   // end setShowBoundingBoxesOnSelected


void ModelSelector::updateAllViews( const std::function<void(FV*)>& fn, bool doUpdateRender)
{
    for ( FMV* fmv : me()->_viewers)
    {
        for ( FV *fv : fmv->attached())
            fn(fv);
        if ( doUpdateRender)
            fmv->updateRender();
    }   // end for

}   // end updateAllViews


void ModelSelector::updateRender()
{
    for ( FMV* fmv : me()->_viewers)
        fmv->updateRender();
}   // end updateRender


// Used by FaceActionManager and LandmarksInteractor
void ModelSelector::syncBoundingVisualisation( const FM* fm)
{
    for ( FV* fv : fm->fvs())
        me()->_bvis.syncToViewTransform( fv, fv->actor()->GetMatrix());
}   // end syncBoundingVisualisation


// private static
ModelSelector::Ptr ModelSelector::me()
{
    if ( !_me)
        _me = Ptr( new ModelSelector(), [](ModelSelector* d) { delete d;});
    return _me;
}   // end me


// private static
SelectNotifier& ModelSelector::sn()
{
    if ( !me()->_sn)
    {
        SelectNotifier* fvsi = me()->_sn = new SelectNotifier;
        QObject::connect( fvsi, &SelectNotifier::onSelected, [](FV* fv, bool s){ me()->_doOnSelected( fv, s);});
    }   // end if
    return *me()->_sn;
}   // end sn


// private
ModelSelector::ModelSelector() : _sbar(nullptr), _autoFocus(true), _showBoxes(true), _defv(-1), _sn(nullptr) {}


void ModelSelector::_doOnSelected( FV *fv, bool v)
{
    if ( v)
    {
        if ( _showBoxes)
            fv->apply(&_bvis);   // Apply the bounding box around the model
        else
            _bvis.setVisible(fv,false);

        if ( _autoFocus)
        {
            if ( fv->data()->currentAssessment()->landmarks().empty())
                ActionResetCamera::resetCamera( fv);
            else
                ActionOrientCameraToFace::orientToFace( fv, DEFAULT_CAMERA_DISTANCE);
        }   // end if
    }   // end if
    else
        _bvis.setVisible(fv,false);
}   // end _doOnSelected
