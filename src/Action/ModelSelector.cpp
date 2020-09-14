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
#include <Interactor/SelectNotifier.h>
#include <Vis/BoundingVisualisation.h>
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <vtkCamera.h>
#include <QSignalBlocker>
#include <cassert>
using FaceTools::Action::ModelSelector;
using FaceTools::Interactor::SelectNotifier;
using FaceTools::Interactor::MouseHandler;
using FaceTools::Interactor::GizmoHandler;
using FaceTools::ModelViewer;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FMS;
using FaceTools::FM;
using FaceTools::Action::IMode;

ModelSelector::Ptr ModelSelector::_me;


void ModelSelector::addViewer( FMV* fmv, bool setDefault)
{
    ModelSelector::Ptr ms = me();
    ms->_viewers.push_back(fmv);
    ms->_mouseHandler->addViewer( fmv, setDefault);
    if ( setDefault)
        ms->_defv = int(me()->_viewers.size()) - 1;
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


const SelectNotifier* ModelSelector::selectNotifier() { return _selectNotifier();}


void ModelSelector::registerHandler( GizmoHandler *gh) { me()->_mouseHandler->registerHandler(gh);}

void ModelSelector::refreshHandlers() { me()->_mouseHandler->refreshHandlers();}


void ModelSelector::setInteractionMode( IMode m)
{
    const FV *fv = selectedView();
    if ( !fv || m == IMode::CAMERA_INTERACTION)
    {
        setLockSelected(false);
        for ( ModelViewer* mv : viewers())
            mv->setCameraInteraction();
    }   // end if
    else
    {
        setLockSelected(true);
        // Note that this prop will only actually be on one of the viewers
        // meaning that the other viewers will default to camera interaction.
        const vtkProp3D *p = fv->actor();
        for ( ModelViewer* mv : viewers())
            mv->setActorInteraction( p);
    }   // end else
    restoreCursor();
}   // end setInteractionMode


IMode ModelSelector::interactionMode()
{
    IMode m = IMode::CAMERA_INTERACTION;
    if ( !viewers().empty())
        m = viewers().at(0)->interactionMode();
    return m;
}   // end interactionMode


FMV* ModelSelector::mouseViewer() { return static_cast<FMV*>(me()->_mouseHandler->mouseViewer());}

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


FV* ModelSelector::selectedView() { return _selectNotifier()->selected();}


FV* ModelSelector::add( FM *fm, FMV *tviewer)
{
    FV *fv = new FV( fm, tviewer);
    _selectNotifier()->add(fv);
    return fv;
}   // end add


void ModelSelector::remove( FV* fv)
{
    // First ensure selection is unlocked
    me()->_lockCount = 0;
    setLockSelected(false);
    _selectNotifier()->setSelected( fv, false);
    _selectNotifier()->remove( fv);
    delete fv;  // Ensures all visualisations removed from the corresponding viewer
}   // end remove


void ModelSelector::remove( const FM* fm)
{
    QSignalBlocker blocker(_selectNotifier());
    while ( !fm->fvs().empty())
        remove( fm->fvs().first());
}   // end remove


void ModelSelector::setSelected( FV* fv)
{
    // First ensure selection is unlocked
    me()->_lockCount = 0;
    setLockSelected(false);

    FV* sv = selectedView();
    if ( sv) // First deselect any currently selected view.
        _selectNotifier()->setSelected( sv, false);
    if ( fv) // The select the given view.
        _selectNotifier()->setSelected( fv, true);
}   // end setSelected


void ModelSelector::setLockSelected( bool v)
{
    int lc = me()->_lockCount;
    if ( v)
        lc++;
    else
        lc = std::max<int>( lc - 1, 0);
    me()->_lockCount = lc;
    _selectNotifier()->setLocked( lc > 0);
    //std::cerr << "Model select: (" << lc << ") " << (lc > 0 ? "LOCKED" : "UNLOCKED") << std::endl;
}   // end setLockSelected


void ModelSelector::setAutoFocusOnSelectEnabled( bool v) { me()->_autoFocus = v;}


void ModelSelector::setShowBoundingBoxesOnSelected( bool v)
{
    me()->_showBoxes = v;

    // Do nothing if the MouseHandler is not yet constructed
    if ( me()->_mouseHandler == nullptr)
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
SelectNotifier* ModelSelector::_selectNotifier() { return me()->_mouseHandler->selectNotifier();}


// private
ModelSelector::ModelSelector()
    : _sbar(nullptr), _autoFocus(true), _showBoxes(true), _defv(-1), _lockCount(0), _mouseHandler( new MouseHandler)
{
    SelectNotifier *mn = _mouseHandler->selectNotifier();
    QObject::connect( mn, &SelectNotifier::onSelected, [this](FV* fv, bool s){ _doOnSelected( fv, s);});
}   // end ctor


// private
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
