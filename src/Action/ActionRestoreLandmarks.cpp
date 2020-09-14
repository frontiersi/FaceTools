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
using MS = FaceTools::Action::ModelSelector;
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
    const FM *fm = MS::selectedModel();
    return fm && fm->hasMask()
              && !fm->currentLandmarks().empty()  // Shouldn't be possible if has a mask
              && MaskRegistration::maskHash() == fm->maskHash();   // Same mask?
}   // end isAllowed


bool ActionRestoreLandmarks::doBeforeAction( Event)
{
    _ulmks.clear();
    if ( _cdialog->open( MS::selectedModel()))
        _ulmks = _cdialog->landmarks(); // Copy out
    return !_ulmks.empty();
}   // end doBeforeAction


void ActionRestoreLandmarks::doAction( Event)
{
    storeUndo( this, Event::LANDMARKS_CHANGE);
    FM *fm = MS::selectedModel();
    fm->lockForWrite();
    restoreLandmarks( fm, _ulmks);
    fm->unlock();
}   // end doAction


Event ActionRestoreLandmarks::doAfterAction( Event)
{
    MS::showStatus("Landmark positions restored.", 5000);
    return Event::LANDMARKS_CHANGE;
}   // end doAfterAction


namespace {

void updateVisualisation( const FM *fm, const IntSet &ulmks)
{
    using namespace FaceTools;
    Vis::LandmarksVisualisation &vis = MS::handler<Interactor::LandmarksHandler>()->visualisation();

    for ( int lmid : ulmks)
    {
        if ( LMAN::isBilateral(lmid))
        {
            for ( const Vis::FV *fv : fm->fvs())
            {
                vis.refreshLandmarkPosition( fv, lmid, LEFT);
                vis.refreshLandmarkPosition( fv, lmid, RIGHT);
            }   // end for
        }   // end if
        else
        {
            for ( const Vis::FV *fv : fm->fvs())
                vis.refreshLandmarkPosition( fv, lmid, MID);
        }   // end else
    }   // end for
}   // end updateVisualisation

}   // end namespace


bool ActionRestoreLandmarks::restoreLandmarks( FM *fm, const IntSet &ulmks, bool uvis)
{
    if ( !fm->hasMask())
        return false;

    const r3d::Mesh &msk = ((const FM*)fm)->mask();
    const r3d::KDTree &kdt = fm->kdtree();
    MaskRegistration::MaskPtr mdata = MaskRegistration::maskData();
    for ( int lmid : ulmks)
    {
        const std::pair<int, r3d::Vec3f> *bcds;
        if ( LMAN::isBilateral(lmid))
        {
            bcds = &mdata->lmksL.at(lmid);
            fm->setLandmarkPosition( lmid, LEFT, toSurface( kdt, msk.fromBarycentric( bcds->first, bcds->second)));
            bcds = &mdata->lmksR.at(lmid);
            fm->setLandmarkPosition( lmid, RIGHT, toSurface( kdt, msk.fromBarycentric( bcds->first, bcds->second)));
        }   // end if
        else
        {
            bcds = &mdata->lmksM.at(lmid);
            fm->setLandmarkPosition( lmid, MID, toSurface( kdt, msk.fromBarycentric( bcds->first, bcds->second)));
        }   // end else
    }   // end for

    if ( uvis)
        updateVisualisation( fm, ulmks);

    return true;
}   // end restoreLandmarks
