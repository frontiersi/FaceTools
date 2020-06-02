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

#include <Action/ActionRestoreSingleLandmark.h>
#include "Action/ActionUpdateMeasurements.h"
#include <Action/ActionRestoreLandmarks.h>
#include <LndMrk/LandmarksManager.h>
#include <FaceModel.h>
using FaceTools::Action::ActionRestoreSingleLandmark;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Interactor::LandmarksHandler;
using FaceTools::FaceLateral;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;
using LMAN = FaceTools::Landmark::LandmarksManager;


ActionRestoreSingleLandmark::ActionRestoreSingleLandmark( const QString& dn, const QIcon& ico, LandmarksHandler::Ptr handler)
    : FaceAction(dn, ico), _handler(handler), _lmid(-1)
{
}   // end ctor


int ActionRestoreSingleLandmark::_getLandmarkFromMousePos() const
{
    int lmid = -1;
    const FV *fv = MS::selectedView();
    if ( fv)
    {
        FaceLateral lat;
        const vtkProp *prop = fv->viewer()->getPointedAt( primedMousePos());
        lmid = _handler->visualisation().landmarkId( fv, prop, lat);
    }   // end if
    return lmid;
}   // end _getLandmarkFromMousePos


bool ActionRestoreSingleLandmark::isAllowed( Event)
{
    const FV* fv = MS::selectedView();
    if ( !fv || !fv->data()->hasLandmarks())
        return false;
    _lmid = _getLandmarkFromMousePos();
    return _lmid >= 0 && !LMAN::isLocked(_lmid);
}   // end isAllowed


void ActionRestoreSingleLandmark::doAction( Event)
{
    storeUndo( this, Event::LANDMARKS_CHANGE | Event::METRICS_CHANGE);
    IntSet ulmks;
    ulmks.insert( _lmid);
    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    ActionRestoreLandmarks::restoreLandmarks( fm, ulmks);
    for ( const FV *fv : fm->fvs())
        _handler->visualisation().refreshLandmark(fv, _lmid);
    ActionUpdateMeasurements::updateMeasurementsForLandmark( fm, _lmid);
    fm->unlock();
}   // end doAction


Event ActionRestoreSingleLandmark::doAfterAction( Event)
{
    MS::showStatus( QString("Restored %1 landmark(s).").arg( LMAN::landmark(_lmid)->name()), 5000);
    return Event::LANDMARKS_CHANGE | Event::METRICS_CHANGE;
}   // end doAfterAction
