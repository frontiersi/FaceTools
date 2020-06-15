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

#include <Action/ModelSelector.h>
#include <Action/ActionOrientCameraToFace.h>
#include <Action/FaceActionWorker.h>
#include <Vis/BoundingVisualisation.h>
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <vtkCamera.h>
#include <QTimer>
#include <cassert>
using FaceTools::Action::ModelSelector;
using FaceTools::Interactor::SelectNotifier;
using FaceTools::Interactor::MouseHandler;
using FaceTools::ModelViewer;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FMS;
using FaceTools::FM;
using FaceTools::Action::IMode;

ModelSelector::Ptr ModelSelector::_me;


void ModelSelector::addViewer( FMV* fmv, bool setDefault)
{
    me()->_viewers.push_back(fmv);
    if ( setDefault)
        me()->_defv = int(me()->_viewers.size()) - 1;
}   // end addViewer


void ModelSelector::setStatusBar( QStatusBar* sb) { me()->_sbar = sb;}


void ModelSelector::showStatus( const QString& msg, int timeOut, bool repaintNow)
{
    QStatusBar *sbar = me()->_sbar;
    if ( sbar)
    {
        sbar->showMessage( QObject::tr( msg.toStdString().c_str()), timeOut);
        if ( repaintNow)
            sbar->repaint( sbar->rect());
    }   // end if
}   // end showStatus


QString ModelSelector::currentStatus()
{
    return me()->_sbar ? me()->_sbar->currentMessage() : "";
}   // end currentStatus


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
    if ( FaceActionWorker::isUserWorking())
        setCursor( Qt::CursorShape::BusyCursor);
    else if ( defaultViewer()->interactionMode() == IMode::ACTOR_INTERACTION)
        setCursor( Qt::CursorShape::DragMoveCursor);
    else
        setCursor( Qt::CursorShape::ArrowCursor);
}   // end restoreCursor


const SelectNotifier* ModelSelector::selector() { return &sn();}

void ModelSelector::setInteractionMode(IMode m, bool v)
{
    for ( ModelViewer* mv : viewers())
        mv->setInteractionMode(m, v);
    restoreCursor();
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
        return nullptr;
    return me()->_viewers.at(size_t(me()->_defv));
}   // end defaultViewer

const std::vector<FMV*>& ModelSelector::viewers() { return me()->_viewers;}


void ModelSelector::setViewAngle( double v)
{
    for ( FMV *fmv : viewers())
    {
        vtkCamera *cam = const_cast<vtkRenderer*>(fmv->getRenderer())->GetActiveCamera();
        cam->SetViewAngle(v);
    }   // end fmv
}   // end setViewAngle


FV* ModelSelector::selectedView() { return sn().selected();}


FV* ModelSelector::add( FM *fm, FMV *tviewer)
{
    FV *fv = new FV( fm, tviewer);
    sn().add(fv);
    return fv;
}   // end add


void ModelSelector::remove( FV* fv)
{
    sn().setSelected( fv, false);
    sn().remove( fv);
    delete fv;  // Ensures all visualisations removed from the corresponding viewer
}   // end remove


void ModelSelector::remove( const FM* fm)
{
    while ( !fm->fvs().empty())
        remove( fm->fvs().first());
    sn().testMouse();   // Force signals to be emitted with the updated absense of the FaceView's actor
}   // end remove


void ModelSelector::setSelected( FV* fv)
{
    FV* sv = selectedView();
    if ( sv) // First deselect any currently selected view.
        sn().setSelected( sv, false);
    if ( fv) // The select the given view.
        sn().setSelected( fv, true);
}   // end setSelected


void ModelSelector::setLockSelected( bool v) { sn().setLocked(v);}


void ModelSelector::setAutoFocusOnSelectEnabled( bool v) { me()->_autoFocus = v;}


void ModelSelector::setShowBoundingBoxesOnSelected( bool v)
{
    me()->_showBoxes = v;

    // Do nothing if the SelectNotifier is not yet constructed
    if ( me()->_sn == nullptr)
        return;

    FV *fv = me()->selectedView();
    if ( fv)
        me()->_doOnSelected( fv, true); // Refresh presence of bounding box around selected view
}   // end setShowBoundingBoxesOnSelected


void ModelSelector::updateAllViews( const std::function<void(FV*)>& fn, bool doUpdateRender)
{
    for ( FMV* fmv : me()->_viewers)
        for ( FV *fv : fmv->attached())
            fn(fv);
    if ( doUpdateRender)
        updateRender();
}   // end updateAllViews


void ModelSelector::updateRender()
{
    for ( FMV* fmv : me()->_viewers)
        fmv->updateRender();
    QWidget *pw = defaultViewer();
    if ( pw)
    {
        while ( pw->parentWidget())
            pw = pw->parentWidget();
        pw->repaint( pw->rect());
    }   // end if
}   // end updateRender


// Used by FaceActionManager
void ModelSelector::syncBoundingVisualisation( const FM* fm)
{
    for ( FV* fv : fm->fvs())
        me()->_bvis.syncWithViewTransform( fv);
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
    if ( v && _showBoxes)
        fv->apply( &_bvis);   // Apply the bounding box around the model
    else
        fv->purge( &_bvis);

    if ( v)
    {
        if ( _autoFocus)
            ActionOrientCameraToFace::orientToFace( fv, 1.0f);
    }   // end if
}   // end _doOnSelected
