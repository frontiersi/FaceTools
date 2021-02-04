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

#include <Action/ActionEditLandmarks.h>
#include <Interactor/LandmarksHandler.h>
#include <LndMrk/LandmarksManager.h>
#include <Metric/MetricManager.h>
#include <QApplication>
using FaceTools::Action::ActionEditLandmarks;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Action::UndoState;
using FaceTools::Vis::FV;
using FaceTools::Widget::LandmarksDialog;
using FaceTools::FaceSide;
using MS = FaceTools::ModelSelect;
using LMAN = FaceTools::Landmark::LandmarksManager;


ActionEditLandmarks::ActionEditLandmarks( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks),
      _dialog(nullptr), _key(0), _actShow(nullptr), _actAlign(nullptr), _actRestore(nullptr), _actShowLabels(nullptr)
{
    setCheckable( true, false);
    addTriggerEvent( Event::MASK_CHANGE | Event::LANDMARKS_CHANGE);
    addRefreshEvent( Event::VIEW_CHANGE);   // Allow the dialog to close on changing landmarks visualisation
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
    connect( h, &LandmarksHandler::onStartedDrag, this, &ActionEditLandmarks::_doOnStartedDrag);
    connect( h, &LandmarksHandler::onFinishedDrag, this, &ActionEditLandmarks::_doOnFinishedDrag);
    connect( h, &LandmarksHandler::onEnterLandmark, _dialog, &LandmarksDialog::setSelectedLandmark);
}   // end postInit


bool ActionEditLandmarks::update( Event)
{
    const FM *fm = MS::selectedModel();
    if ( !fm || !fm->hasLandmarks() || (!_actShow->isChecked() && _dialog->isVisible()))
        _closeDialog();
    return _dialog->isVisible();
}   // end update


bool ActionEditLandmarks::isAllowed( Event e)
{
    const FM *fm = MS::selectedModel();
    return fm && fm->hasLandmarks();
}   // end isAllowed


void ActionEditLandmarks::_closeDialog()
{
    if ( _key != 0)
    {
        MS::unlockSelect(_key);
        _key = 0;
    }   // end if
    _dialog->setVisible(false);
    _actShowLabels->setChecked(false);
    for ( int id : LMAN::ids())
        LMAN::landmark(id)->setLocked(true);
}   // end _closeDialog


void ActionEditLandmarks::_openDialog()
{
    _actShow->setChecked(true); // Ensure the landmarks are visible
    _dialog->setVisible(true);
    for ( int id : LMAN::ids())
        LMAN::landmark(id)->setLocked(false);
}   // end _openDialog


void ActionEditLandmarks::doAction( Event e)
{
    const FM *fm = MS::selectedModel();
    bool chk = fm && isChecked() && !has( e, Event::CLOSED_MODEL);

    QString msg;
    if ( has( e, Event::MASK_CHANGE) && fm->hasLandmarks())
    {
        msg = tr("Click and drag landmarks into their confirmed positions in the main viewer");
        chk = true;
        _key = MS::lockSelect();   // Prevent selecting a different view until close
        QApplication::beep();
    }   // end if
    _dialog->setMessage(msg);

    if ( !chk)
        _closeDialog();
    else
        _openDialog();
}   // end doAction


void ActionEditLandmarks::_doOnStartedDrag( int lmid, FaceSide lat) { storeUndo( this, Event::LANDMARKS_CHANGE);}

void ActionEditLandmarks::_doOnFinishedDrag( int lmid, FaceSide) { emit onEvent( Event::LANDMARKS_CHANGE);}
