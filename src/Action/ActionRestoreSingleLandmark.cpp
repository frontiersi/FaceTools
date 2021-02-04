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

#include <Action/ActionRestoreSingleLandmark.h>
#include <Action/ActionRestoreLandmarks.h>
#include <Interactor/LandmarksHandler.h>
#include <LndMrk/LandmarksManager.h>
#include <FaceModel.h>
using FaceTools::Action::ActionRestoreSingleLandmark;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Interactor::LandmarksHandler;
using FaceTools::FaceSide;
using FaceTools::Vis::FV;
using MS = FaceTools::ModelSelect;
using LMAN = FaceTools::Landmark::LandmarksManager;


ActionRestoreSingleLandmark::ActionRestoreSingleLandmark( const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico), _lmid(-1)
{
    addRefreshEvent( Event::LANDMARKS_CHANGE);
}   // end ctor

namespace {
int landmarkFromMousePos( const QPoint &mp)
{
    const LandmarksHandler *lmksHandler = MS::handler<LandmarksHandler>();
    const FV *fv = MS::selectedView();
    FaceSide lat;
    const vtkProp *prop = fv->viewer()->getPointedAt( mp);
    return lmksHandler->visualisation().landmarkId( fv, prop, lat);
}   // end landmarkFromMousePos
}   // end namespace


bool ActionRestoreSingleLandmark::isAllowed( Event)
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    if ( !fm || !fm->hasLandmarks())
        return false;
    _lmid = landmarkFromMousePos( primedMousePos());
    //return _lmid >= 0 && !LMAN::isLocked(_lmid);
    return _lmid >= 0;
}   // end isAllowed


void ActionRestoreSingleLandmark::doAction( Event)
{
    storeUndo( this, Event::LANDMARKS_CHANGE);
    IntSet ulmks;
    ulmks.insert( _lmid);
    FM::WPtr fm = MS::selectedModelScopedWrite();
    ActionRestoreLandmarks::restoreLandmarks( *fm, ulmks);
}   // end doAction


Event ActionRestoreSingleLandmark::doAfterAction( Event)
{
    MS::showStatus( QString("Restored %1 landmark(s).").arg( LMAN::landmark(_lmid)->name()), 5000);
    return Event::LANDMARKS_CHANGE;
}   // end doAfterAction
