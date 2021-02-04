/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <ModelSelect.h>
#include <FaceModel.h>
#include <FaceModelViewer.h>
#include <Action/FaceActionWorker.h>
#include <Interactor/SelectNotifier.h>
#include <FileIO/FaceModelManager.h>
#include <Vis/FaceView.h>
#include <vtkCamera.h>
#include <QSignalBlocker>
#include <cassert>
using FaceTools::ModelSelect;
using FaceTools::Interactor::SelectNotifier;
using FaceTools::Interactor::MouseHandler;
using FaceTools::Interactor::GizmoHandler;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FM;
using FaceTools::IMode;
using FMM = FaceTools::FileIO::FaceModelManager;

ModelSelect::Ptr ModelSelect::_me;


void ModelSelect::addViewer( FMV* fmv, bool setDefault)
{
    ModelSelect* ms = me();
    ms->_viewers.push_back(fmv);
    ms->_mouseHandler->addViewer( fmv, setDefault);
    if ( setDefault)
        ms->_defv = int(ms->_viewers.size()) - 1;
}   // end addViewer


void ModelSelect::setStatusBar( QStatusBar* sb) { me()->_sbar = sb;}


void ModelSelect::showStatus( const QString& msg, int timeOut, bool repaintNow)
{
    QStatusBar *sbar = me()->_sbar;
    if ( sbar)
    {
        sbar->showMessage( QObject::tr( msg.toStdString().c_str()), timeOut);
        if ( repaintNow)
            sbar->repaint( sbar->rect());
    }   // end if
}   // end showStatus


QString ModelSelect::currentStatus()
{
    return me()->_sbar ? me()->_sbar->currentMessage() : "";
}   // end currentStatus


void ModelSelect::clearStatus()
{
    if ( me()->_sbar)
        me()->_sbar->clearMessage();
}   // end clearStatus


void ModelSelect::setCursor( Qt::CursorShape cs)
{
    for ( FMV* fmv : me()->_viewers)
        fmv->setCursor(cs);
}   // end setCursor


void ModelSelect::restoreCursor()
{
    if ( Action::FaceActionWorker::isUserWorking())
        setCursor( Qt::CursorShape::BusyCursor);
    else if ( defaultViewer()->interactionMode() == IMode::ACTOR_INTERACTION)
        setCursor( Qt::CursorShape::DragMoveCursor);
    else
        setCursor( Qt::CursorShape::ArrowCursor);
}   // end restoreCursor


const SelectNotifier* ModelSelect::selectNotifier() { return _selectNotifier();}


void ModelSelect::registerHandler( GizmoHandler *gh) { me()->_mouseHandler->registerHandler(gh);}

void ModelSelect::refreshHandlers() { me()->_mouseHandler->refreshHandlers();}


void ModelSelect::setInteractionMode( IMode m)
{
    const FV *fv = selectedView();
    if ( !fv || m == IMode::CAMERA_INTERACTION)
    {
        unlockSelect(me()->_lockKey);
        for ( ModelViewer* mv : viewers())
            mv->setCameraInteraction();
    }   // end if
    else
    {
        me()->_lockKey = lockSelect();
        // Note that this prop will only actually be on one of the viewers
        // meaning that the other viewers will default to camera interaction.
        const vtkProp3D *p = fv->actor();
        for ( ModelViewer* mv : viewers())
            mv->setActorInteraction( p);
    }   // end else
    restoreCursor();
}   // end setInteractionMode


IMode ModelSelect::interactionMode()
{
    IMode m = IMode::CAMERA_INTERACTION;
    if ( !viewers().empty())
        m = viewers().at(0)->interactionMode();
    return m;
}   // end interactionMode


FMV* ModelSelect::mouseViewer() { return static_cast<FMV*>(me()->_mouseHandler->mouseViewer());}


FMV* ModelSelect::defaultViewer()
{
    if ( me()->_defv < 0)
        return nullptr;
    return me()->_viewers.at(size_t(me()->_defv));
}   // end defaultViewer


const std::vector<FMV*>& ModelSelect::viewers() { return me()->_viewers;}


void ModelSelect::setViewAngle( double v)
{
    for ( FMV *fmv : viewers())
        const_cast<vtkRenderer*>(fmv->getRenderer())->GetActiveCamera()->SetViewAngle(v);
}   // end setViewAngle


FV* ModelSelect::selectedView() { return _selectNotifier()->selected();}


FM::RPtr ModelSelect::selectedModelScopedRead()
{
    const FV *fv = selectedView();
    return fv ? fv->data()->scopedReadLock() : nullptr;
}   // end selectedModelScopedRead


FM::WPtr ModelSelect::selectedModelScopedWrite()
{
    FV *fv = selectedView();
    return fv ? fv->data()->scopedWriteLock() : nullptr;
}   // end selectedModelScopedWrite


const FM *ModelSelect::nonSelectedModel()
{
    const FM *fm = selectedModel();
    const FM *ofm = nullptr;
    if ( fm)
        ofm = FMM::other(*fm);
    return ofm;
}   // end nonSelectedModel


FM::RPtr ModelSelect::otherModelScopedRead( const FM *fm)
{
    FM::RPtr fmptr;
    if ( fm)
    {
        FM *ofm = FMM::other(*fm);
        if ( ofm)
            fmptr = ofm->scopedReadLock();
    }   // end if
    return fmptr;
}   // end otherModelScopedRead


FM::RPtr ModelSelect::otherModelScopedRead()
{
    return otherModelScopedRead( selectedModel());
}   // end otherModelScopedRead


FM::WPtr ModelSelect::otherModelScopedWrite( const FM *fm)
{
    FM::WPtr fmptr;
    if ( fm)
    {
        FM *ofm = FMM::other(*fm);
        if ( ofm)
            fmptr = ofm->scopedWriteLock();
    }   // end if
    return fmptr;
}   // end otherModelScopedWrite


FM::WPtr ModelSelect::otherModelScopedWrite()
{
    return otherModelScopedWrite( selectedModel());
}   // end otherModelScopedWrite


FV* ModelSelect::add( FM *fm, FMV *tviewer)
{
    FV *fv = new FV( fm, tviewer);
    QSignalBlocker blocker(_selectNotifier());
    _selectNotifier()->add(fv);
    return fv;
}   // end add


void ModelSelect::remove( FV* fv)
{
    _forceUnlockSelect();
    _selectNotifier()->setSelected( fv, false);
    _selectNotifier()->remove( fv);
    delete fv;  // Ensures all visualisations removed from the corresponding viewer
}   // end remove


void ModelSelect::remove( const FM* fm)
{
    QSignalBlocker blocker(_selectNotifier());
    while ( !fm->fvs().empty())
        remove( fm->fvs().first());
}   // end remove


bool ModelSelect::exists( const FV *fv)
{
    for ( FMV* fmv : me()->_viewers)
        if ( fmv->has(fv))
            return true;
    return false;
}   // end exists


void ModelSelect::setSelected( FV* fv)
{
    QSignalBlocker blocker(_selectNotifier());
    _forceUnlockSelect();
    FV* sv = selectedView();
    if ( sv) // First deselect any currently selected view.
        _selectNotifier()->setSelected( sv, false);
    if ( fv) // Then select the given view.
        _selectNotifier()->setSelected( fv, true);
}   // end setSelected


int ModelSelect::lockSelect()
{
    _selectNotifier()->setLocked( true);
    return me()->_slocker.lock();
}   // end lockSelect


void ModelSelect::unlockSelect( int key)
{
    me()->_slocker.turnkey( key, true);
    _selectNotifier()->setLocked( me()->_slocker.isLocked());
}   // end unlockSelect


void ModelSelect::_forceUnlockSelect()
{
    me()->_slocker.reset();
    _selectNotifier()->setLocked( false);
}   // end _forceUnlockSelect


void ModelSelect::updateAllViews( const std::function<void(FV*)>& fn, bool doUpdateRender)
{
    for ( FMV* fmv : me()->_viewers)
        for ( FV *fv : fmv->attached())
            fn(fv);
    if ( doUpdateRender)
        updateRender();
}   // end updateAllViews


void ModelSelect::updateRender()
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


// private static
ModelSelect* ModelSelect::me()
{
    if ( !_me)
        _me = Ptr( new ModelSelect(), [](ModelSelect* d) { delete d;});
    return &*_me;
}   // end me


// private static
SelectNotifier* ModelSelect::_selectNotifier() { return me()->_mouseHandler->selectNotifier();}


// private
ModelSelect::ModelSelect()
    : _sbar(nullptr), _defv(-1), _lockKey(0), _mouseHandler( new MouseHandler)
{
}   // end ctor
