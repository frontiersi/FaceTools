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
#include <Action/ActionUpdateMeasurements.h>
#include <Action/ActionOrientCameraToFace.h>
#include <LndMrk/LandmarksManager.h>
#include <Metric/MetricManager.h>
using FaceTools::Action::ActionEditLandmarks;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Action::UndoState;
using FaceTools::Vis::FV;
using FaceTools::Interactor::LandmarksHandler;
using FaceTools::Widget::LandmarksDialog;
using FaceTools::FaceLateral;
using MS = FaceTools::Action::ModelSelector;
using LMAN = FaceTools::Landmark::LandmarksManager;


ActionEditLandmarks::ActionEditLandmarks( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _handler( LandmarksHandler::create()),
        _dialog(nullptr),
        _actShow(nullptr), _actAlign(nullptr), _actRestore(nullptr), _actShowLabels(nullptr),
        _lmid(-1), _lat(FACE_LATERAL_MEDIAL), _egrp( Event::NONE)
{
    setCheckable( true, false);
    addTriggerEvent( Event::MASK_CHANGE | Event::CLOSED_MODEL);
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

    connect( &*_handler, &LandmarksHandler::onEnterLandmark, _dialog, &LandmarksDialog::setSelectedLandmark);
    connect( &*_handler, &LandmarksHandler::onStartedDrag, this, &ActionEditLandmarks::_doOnStartedDrag);
    connect( &*_handler, &LandmarksHandler::onDoingDrag, this, &ActionEditLandmarks::_doOnDoingDrag);
    connect( &*_handler, &LandmarksHandler::onFinishedDrag, this, &ActionEditLandmarks::_doOnDoingDrag);
    connect( _dialog, &LandmarksDialog::finished, this, &ActionEditLandmarks::_doOnClosedDialog);
}   // end postInit


bool ActionEditLandmarks::checkState( Event e)
{
    const FM *fm = MS::selectedModel();
    _handler->refreshState();
    return fm && (_dialog->isVisible() || (has(e, Event::MASK_CHANGE) && has(e, Event::LANDMARKS_CHANGE) && fm->hasLandmarks()));
}   // end checkState


void ActionEditLandmarks::doAction( Event e)
{
    if ( isChecked() && !_dialog->isVisible())
    {
        QString msg;
        const FM *fm = MS::selectedModel();
        MS::setLockSelected(true);
        if ( fm && has( e, Event::MASK_CHANGE) && fm->hasMask())
            msg = tr("Review and confirm landmark placement in the main viewer");
        _dialog->setMessage( msg);

        _lmid = -1;
        _lat = FACE_LATERAL_MEDIAL;
        for ( int id : LMAN::ids())
            LMAN::landmark(id)->setLocked( false);

        _dialog->show();
    }   // end if
    else if ( _dialog->isVisible())
        _doOnClosedDialog();
}   // end doAction


void ActionEditLandmarks::_doOnClosedDialog()
{
    setChecked(false);
    _dialog->hide();
    for ( int id : LMAN::ids())
        LMAN::landmark(id)->setLocked( true);

    const FV *fv = MS::selectedView();
    if ( fv)
    {
        ActionOrientCameraToFace::orientToFace( fv, 1);
        MS::setLockSelected(false);
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
        emit onEvent( Event::CAMERA_CHANGE);
    }   // end if
}   // end _doOnClosedDialog


void ActionEditLandmarks::saveState( UndoState &us) const
{
    us.setName( "Move Landmark");
    us.setUserData( "LandmarkId", QVariant::fromValue(_lmid));
    us.setUserData( "LandmarkLat", QVariant::fromValue(_lat));
    const Vec3f &pos = us.model()->currentLandmarks().pos(_lmid, _lat);
    us.setUserData( "LandmarkPos", QVariant::fromValue(pos));
}   // end saveState


void ActionEditLandmarks::restoreState( const UndoState &us)
{
    _lmid = us.userData("LandmarkId").value<int>();
    _lat = us.userData("LandmarkLat").value<FaceLateral>();
    us.model()->setLandmarkPosition( _lmid, _lat, us.userData("LandmarkPos").value<Vec3f>());
    if ( has( us.events(), Event::METRICS_CHANGE))
        ActionUpdateMeasurements::updateMeasurementsForLandmark( us.model(), _lmid);
}   // end restoreState


void ActionEditLandmarks::_doOnStartedDrag( int lmid, FaceLateral lat)
{
    _lmid = lmid;
    _lat = lat;
    _egrp = Event::LANDMARKS_CHANGE;
    if ( !Metric::MetricManager::metricsForLandmark( lmid).empty())
        _egrp |= Event::METRICS_CHANGE;
    storeUndo( this, _egrp, false);
}   // end _doOnStartedDrag


void ActionEditLandmarks::_doOnDoingDrag( int lmid, FaceLateral)
{
    // Update measurements related to the moved landmark
    ActionUpdateMeasurements::updateMeasurementsForLandmark( MS::selectedModel(), lmid);
    emit onEvent( _egrp);
    _egrp = Event::NONE;
}   // end _doOnDoingDrag
