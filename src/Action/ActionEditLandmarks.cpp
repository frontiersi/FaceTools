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

#include <Action/ActionEditLandmarks.h>
#include <Interactor/LandmarksHandler.h>
#include <LndMrk/LandmarksManager.h>
#include <Metric/MetricManager.h>
using FaceTools::Action::ActionEditLandmarks;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Action::UndoState;
using FaceTools::Vis::FV;
using FaceTools::Widget::LandmarksDialog;
using FaceTools::FaceSide;
using MS = FaceTools::Action::ModelSelector;
using LMAN = FaceTools::Landmark::LandmarksManager;


ActionEditLandmarks::ActionEditLandmarks( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks),
      _dialog(nullptr), _actShow(nullptr), _actAlign(nullptr), _actRestore(nullptr), _actShowLabels(nullptr)
{
    setCheckable( true, false);
    addTriggerEvent( Event::MASK_CHANGE | Event::RESTORE_CHANGE);
}   // end ctor


void ActionEditLandmarks::postInit()
{
    assert(_actShow);
    assert(_actAlign);
    assert(_actRestore);
    assert(_actShowLabels);

    _dialog = new LandmarksDialog( static_cast<QWidget*>(parent()));
    _dialog->setShowAction( _actShow);
    _dialog->setAlignAction( _actAlign);
    _dialog->setRestoreAction( _actRestore);
    _dialog->setLabelsAction( _actShowLabels);

    connect( _dialog, &LandmarksDialog::finished, [this](){ this->setChecked(false);});

    using Interactor::LandmarksHandler;
    const LandmarksHandler *h = MS::handler<LandmarksHandler>();
    assert( h);
    connect( h, &LandmarksHandler::onEnterLandmark, _dialog, &LandmarksDialog::setSelectedLandmark);
    connect( h, &LandmarksHandler::onStartedDrag, this, &ActionEditLandmarks::_doOnStartedDrag);
    connect( h, &LandmarksHandler::onFinishedDrag, this, &ActionEditLandmarks::_doOnFinishedDrag);
}   // end postInit


bool ActionEditLandmarks::isAllowed( Event e)
{
    const FM *fm = MS::selectedModel();
    return fm && fm->hasLandmarks();
}   // end isAllowed


bool ActionEditLandmarks::update( Event e)
{
    QString msg;
    bool chk = isChecked();
    const FV *fv = MS::selectedView();
    if ( !fv || has( e, Event::CLOSED_MODEL))
        chk = false;
    else if ( isTriggerEvent(e))
    {
        if ( has(e, Event::LANDMARKS_CHANGE))
        {
            chk = fv->data()->hasLandmarks();
            if ( has( e, Event::MASK_CHANGE) && fv->data()->hasMask())
                msg = tr("Confirm landmark placement in the main window");
            else
                msg = tr("Confirm restored landmark position(s)");
        }   // end if
    }   // end else if

    _dialog->setMessage(msg);
    _dialog->setVisible(chk);
    MS::setLockSelected(chk);
    for ( int id : LMAN::ids())
        LMAN::landmark(id)->setLocked( !chk);

    return chk;
}   // end update


void ActionEditLandmarks::_doOnStartedDrag( int lmid, FaceSide lat)
{
    storeUndo( this, Event::LANDMARKS_CHANGE);
}   // end _doOnStartedDrag


void ActionEditLandmarks::_doOnFinishedDrag( int lmid, FaceSide) { emit onEvent( Event::LANDMARKS_CHANGE);}
