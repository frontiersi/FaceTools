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

#include <Action/ActionRestoreLandmarks.h>
#include <Interactor/LandmarksHandler.h>
#include <LndMrk/LandmarksManager.h>
#include <MaskRegistration.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceTools.h>
using FaceTools::Action::ActionRestoreLandmarks;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FM;
using FaceTools::Widget::LandmarksCheckDialog;
using MS = FaceTools::ModelSelect;
using LMAN = FaceTools::Landmark::LandmarksManager;


ActionRestoreLandmarks::ActionRestoreLandmarks( const QString& dn, const QIcon& icon)
    : FaceAction(dn, icon), _cdialog(nullptr)
{
    addRefreshEvent( Event::LANDMARKS_CHANGE);
}   // end ctor


void ActionRestoreLandmarks::postInit()
{
    _cdialog = new LandmarksCheckDialog( static_cast<QWidget*>(parent()));
}   // end postInit


bool ActionRestoreLandmarks::isAllowed( Event)
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    return fm && fm->hasMask()
              && !fm->currentLandmarks().empty()  // Shouldn't be possible if has a mask
              && MaskRegistration::maskHash() == fm->maskHash();   // Same mask?
}   // end isAllowed


bool ActionRestoreLandmarks::doBeforeAction( Event)
{
    _ulmks.clear();
    if ( _cdialog->open( *MS::selectedModelScopedRead()))
        _ulmks = _cdialog->landmarks(); // Copy out
    return !_ulmks.empty();
}   // end doBeforeAction


void ActionRestoreLandmarks::doAction( Event)
{
    storeUndo( this, Event::LANDMARKS_CHANGE);
    FM::WPtr fm = MS::selectedModelScopedWrite();
    restoreLandmarks( *fm, _ulmks);
}   // end doAction


Event ActionRestoreLandmarks::doAfterAction( Event)
{
    MS::showStatus("Landmark positions restored.", 5000);
    return Event::LANDMARKS_CHANGE | Event::RESTORE_CHANGE;
}   // end doAfterAction


namespace {

void updateVisualisation( const FM &fm, const IntSet &ulmks)
{
    using namespace FaceTools;
    Vis::LandmarksVisualisation &vis = MS::handler<Interactor::LandmarksHandler>()->visualisation();

    for ( int lmid : ulmks)
    {
        if ( LMAN::isBilateral(lmid))
        {
            for ( const Vis::FV *fv : fm.fvs())
            {
                vis.refreshLandmarkPosition( fv, lmid, LEFT);
                vis.refreshLandmarkPosition( fv, lmid, RIGHT);
            }   // end for
        }   // end if
        else
        {
            for ( const Vis::FV *fv : fm.fvs())
                vis.refreshLandmarkPosition( fv, lmid, MID);
        }   // end else
    }   // end for
}   // end updateVisualisation

}   // end namespace


bool ActionRestoreLandmarks::restoreLandmark( FM &fm, int lmid, int aid)
{
    if ( !fm.hasMask())
        return false;

    FaceAssessment::Ptr ass = fm.assessment(aid);
    assert( ass);
    Landmark::LandmarkSet &lmset = ass->landmarks();

    const r3d::Mesh &msk = fm.mask();
    const r3d::KDTree &kdt = fm.kdtree();

    using MaskReg = MaskRegistration;
    if ( LMAN::isBilateral(lmid))
    {
        lmset.set( lmid, toSurface( kdt, MaskReg::maskLandmarkPosition( msk, lmid, LEFT)), LEFT);
        lmset.set( lmid, toSurface( kdt, MaskReg::maskLandmarkPosition( msk, lmid, RIGHT)), RIGHT);
    }   // end if
    else
        lmset.set( lmid, toSurface( kdt, MaskReg::maskLandmarkPosition( msk, lmid, MID)), MID);

    fm.setMetaSaved(false);
    return true;
}   // end restoreLandmark


bool ActionRestoreLandmarks::restoreLandmarks( FM &fm, const IntSet &ulmks, bool uvis)
{
    if ( !fm.hasMask())
        return false;

    for ( int lmid : ulmks)
        restoreLandmark( fm, lmid, -1); // Just the current assessment

    if ( uvis)
        updateVisualisation( fm, ulmks);

    return true;
}   // end restoreLandmarks


bool ActionRestoreLandmarks::restoreMissingLandmarks( FM &fm)
{
    if ( !fm.hasMask())
        return false;

    // Just check landmarks from the current assessment (others
    // will have the same missing landmarks so this is fine).
    const Landmark::LandmarkSet &lmset = fm.currentLandmarks();
    IntSet mlmks;
    for ( int lmid : LMAN::ids())
        if ( !lmset.has(lmid))
            mlmks.insert(lmid);

    for ( int aid : fm.assessmentIds())
        for ( int lmid : mlmks)
            restoreLandmark( fm, lmid, aid);

    return !mlmks.empty();
}   // end restoreMissingLandmarks
