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
#include <MaskRegistration.h>
#include <QApplication>
#include <rNonRigid.h>
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
      _dialog(nullptr), _key(0), _initPos(r3d::Vec3f::Zero()), _ev(Event::NONE),
      _actShow(nullptr), _actAlign(nullptr), _actShowLabels(nullptr)
{
    setCheckable( true, false);
    addTriggerEvent( Event::MASK_CHANGE | Event::LANDMARKS_CHANGE);
    // Allow the dialog to close on turning off landmarks visibility.
    addRefreshEvent( Event::VIEW_CHANGE);
}   // end ctor


void ActionEditLandmarks::postInit()
{
    assert(_actShow);
    assert(_actAlign);
    assert(_actShowLabels);

    _dialog = new LandmarksDialog( static_cast<QWidget*>(parent()));
    _dialog->setShowAction( _actShow);
    _dialog->setAlignAction( _actAlign);
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


bool ActionEditLandmarks::doBeforeAction( Event)
{
    /*
    // Check if triggered by event emitted by the _doOnFinishedDrag handler and ignore if so.
    const bool triggeredExternally = _ev == Event::NONE;
    _ev = Event::NONE;
    return triggeredExternally;
    */
    return true;
}   // end doBeforeAction


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


namespace {

rNonRigid::Mesh makeNRRTarget( const r3d::KDTree &kdt, const r3d::Vec3f &pos, float ld)
{
    rNonRigid::Mesh tgt;
    tgt.features = kdt.mesh().toFeatures( true/*use transformed*/);

    // Set flags and update positions for vertices within ld of pos.
    std::vector<std::pair<size_t,float> > matches;
    kdt.findr( pos, ld*ld, matches);
    for ( const std::pair<size_t,float> &p : matches)
    {
        const int i = int(p.first);
        const r3d::Vec3f &v = kdt.mesh().vtx(i);
    }   // end for

    // Set the normals
    return tgt;
}   // end makeNRRTarget


rNonRigid::Mesh makeNRRFloating( const r3d::KDTree &kdt, const r3d::Vec3f &pos, float ld,
                                 const r3d::Vec3f &offset)
{
    rNonRigid::Mesh flt;
    flt.features = kdt.mesh().toFeatures( true/*use transformed*/);
    flt.topology = kdt.mesh().toFaces();

    // Set flags and update positions for vertices within ld of pos.
    std::vector<std::pair<size_t,float> > matches;
    kdt.findr( pos, ld*ld, matches);
    for ( const std::pair<size_t,float> &p : matches)
    {
        const int i = int(p.first);
        const r3d::Vec3f &v = kdt.mesh().vtx(i);
        // Scale offset in inverse proportion to distance from initial point (landmark).
        flt.features.block<1,3>(i,0) += offset * (ld - (v-pos).norm()) / ld;
    }   // end for

    // Set the normals
    return flt;
}   // end makeNRRFloating

}   // end namespace


void ActionEditLandmarks::_doOnStartedDrag( int lmid, FaceSide lat)
{
    const FM *fm = MS::selectedModel();
    // Record the initial position of the landmark *on the underlying mask*.
    if ( fm->hasMask())
        _initPos = MaskRegistration::maskLandmarkPosition( fm->mask(), lmid, lat);
    //storeUndo( this, Event::LANDMARKS_CHANGE | Event::MASK_CHANGE);
    storeUndo( this, Event::LANDMARKS_CHANGE);
}   // end _doOnStartedDrag


void ActionEditLandmarks::_doOnFinishedDrag( int lmid, FaceSide lat)
{
    _ev = Event::LANDMARKS_CHANGE;

    /*
    FM *fm = MS::selectedModel();
    const Vec3f &lmpos = fm->currentLandmarks().pos( lmid, lat);    // Ending position on the face
    const Vec3f offset = lmpos - _initPos;
    if ( !offset.isZero()) // Refit the local mask region?
    {
        const float ld = 2*offset.norm();

        // Use the currently mapped mask as the source of the new mask with offsets.
        rNonRigid::Mesh flt = makeNRRFloating( fm->maskKDTree(), _initPos, ld, offset);

        // Only use the region of the target model in similar region for correspondence.
        const rNonRigid::Mesh tgt = makeNRRTarget( fm->kdtree(), lmpos, ld);

        // Set parameters appropriate for local adjustment of non-rigid registration
        const rNonRigid::NonRigidRegistration nrr(20,
                                                  3, 0.9f, true,  // K, flagthresh, eqPushPull
                                                  4.0f, true, 10, // kappa, useOrient, numInlierIts
                                                  80, 3.0f,       // smoothK, sigmaSmooth
                                                  100, 1,         // viscous start/end
                                                  100, 1);        // elastic start/end
        nrr( flt, tgt); // Update flt

        // Rebuild and set the new mask
        r3d::Mesh::Ptr nmask = r3d::Mesh::fromVertices( flt.features.leftCols(3));
        assert( nmask->numVtxs() == MaskRegistration::maskData()->mask->mesh().numVtxs());
        nmask->setFaces( flt.topology);
        fm->setMask( nmask);
        _ev |= Event::MASK_CHANGE;
        // Realign afterwards?
    }   // end if
    */

    // Since emitting this event, it will also be handled by this action as a trigger
    // so record the event in _ev and cancel the trigger if the event isn't none.
    emit onEvent( _ev);
}   // end _doOnFinishedDrag
