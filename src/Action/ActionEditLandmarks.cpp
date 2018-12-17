/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#include <ActionEditLandmarks.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <VtkTools.h>
#include <algorithm>
using FaceTools::Action::ActionEditLandmarks;
using FaceTools::Action::EventSet;
using FaceTools::Action::ActionVisualise;
using FaceTools::Interactor::LandmarksInteractor;
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::Interactor::MEEI;
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::FVS;
using FaceTools::FM;
using FaceTools::Vis::FV;


ActionEditLandmarks::ActionEditLandmarks( const QString& dn, const QIcon& ico, MEEI* meei, bool visOnLoad)
    : ActionVisualise( _vis = new LandmarksVisualisation( dn, ico), visOnLoad),
     _interactor( new LandmarksInteractor( meei, _vis))
{
    // Leverage this action's reportFinished signal to propagate landmark edits.
    connect( _interactor, &ModelViewerInteractor::onChangedData, this, &ActionEditLandmarks::doOnEditedLandmark);
    setRespondToEventIfAllReady( LANDMARKS_ADD);
}   // end ctor


ActionEditLandmarks::~ActionEditLandmarks()
{
    delete _interactor;
    delete _vis;
}   // end dtor


bool ActionEditLandmarks::doAction( FVS& fvs, const QPoint& mc)
{
    const bool aokay = ActionVisualise::doAction( fvs, mc);
    if ( isChecked())   // Is visualised, ensure all landmarks are refreshed and visible
    {
        for ( FM* fm : fvs.models())
        {
            _vis->refreshLandmarks(fm);
            for ( FV* fv : fm->fvs())
                _vis->apply( fv);
        }   // end for
    }   // end if
    return aokay;
}   // end doAction


void ActionEditLandmarks::doAfterAction( EventSet& cs, const FVS&, bool)
{
    cs.insert( VIEW_CHANGE);
    _interactor->setEnabled(isChecked());
}   // end doAfterAction


void ActionEditLandmarks::tellReady( const FV* fv, bool)
{
    _vis->refreshLandmarks(fv->data());
}   // end tellReady


void ActionEditLandmarks::doOnEditedLandmark( const FV* fv)
{
    EventSet cset;
    cset.insert(LANDMARKS_CHANGE);
    FVS fvs;
    fvs.insert(const_cast<FV*>(fv));
    emit reportFinished( cset, fvs, true);
}   // end doOnEditedLandmark
