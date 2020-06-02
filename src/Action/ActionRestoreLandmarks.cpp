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
#include "Action/ActionUpdateMeasurements.h"
#include <LndMrk/LandmarksManager.h>
#include <MaskRegistration.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceTools.h>
using FaceTools::Action::ActionRestoreLandmarks;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Interactor::LandmarksHandler;
using FaceTools::FM;
using FaceTools::Widget::LandmarksCheckDialog;
using MS = FaceTools::Action::ModelSelector;
using LMAN = FaceTools::Landmark::LandmarksManager;


ActionRestoreLandmarks::ActionRestoreLandmarks( const QString& dn, const QIcon& icon, LandmarksHandler::Ptr handler)
    : FaceAction(dn, icon), _cdialog(nullptr), _handler(handler)
{
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
    storeUndo( this, Event::LANDMARKS_CHANGE | Event::METRICS_CHANGE);
    FM *fm = MS::selectedModel();
    fm->lockForWrite();
    restoreLandmarks( fm, _ulmks);
    for ( const Vis::FV *fv : fm->fvs())
        for ( int lmid : _ulmks)
            _handler->visualisation().refreshLandmark(fv, lmid);
    ActionUpdateMeasurements::updateMeasurementsForLandmarks( fm, _ulmks);
    fm->unlock();
}   // end doAction


Event ActionRestoreLandmarks::doAfterAction( Event)
{
    MS::showStatus("Landmark positions restored.", 5000);
    return Event::LANDMARKS_CHANGE | Event::METRICS_CHANGE;
}   // end doAfterAction


bool ActionRestoreLandmarks::restoreLandmarks( FM *fm, const IntSet &ulmks)
{
    if ( !fm->hasMask())
        return false;

    const r3d::Mesh &msk = fm->mask();
    const r3d::KDTree &kdt = fm->kdtree();
    MaskRegistration::MaskPtr mdata = MaskRegistration::maskData();
    for ( int lmid : ulmks)
    {
        const std::pair<int, r3d::Vec3f> *bcds;
        if ( LMAN::isBilateral(lmid))
        {
            bcds = &mdata->lmksL.at(lmid);
            fm->setLandmarkPosition( lmid, FACE_LATERAL_LEFT, toSurface( kdt, msk.fromBarycentric( bcds->first, bcds->second)));

            bcds = &mdata->lmksR.at(lmid);
            fm->setLandmarkPosition( lmid, FACE_LATERAL_RIGHT, toSurface( kdt, msk.fromBarycentric( bcds->first, bcds->second)));
        }   // end if
        else
        {
            bcds = &mdata->lmksM.at(lmid);
            fm->setLandmarkPosition( lmid, FACE_LATERAL_MEDIAL, toSurface( kdt, msk.fromBarycentric( bcds->first, bcds->second)));
        }   // end else
    }   // end for
    return true;
}   // end restoreLandmarks
