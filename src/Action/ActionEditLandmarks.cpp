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


ActionEditLandmarks::ActionEditLandmarks( const QString& dn, const QIcon& ico, MEEI* meei, QStatusBar* sbar, bool visOnLoad)
    : ActionVisualise( _vis = new LandmarksVisualisation( dn, ico), visOnLoad),
     _interactor( new LandmarksInteractor( meei, _vis, sbar))
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
    // TODO: Should probably consider making this a separate action instead of making this use ActionVisualise.
    //const FMS& fms = fvs.models();
    //std::for_each( std::begin(fms), std::end(fms), [=](FM* fm){ this->refreshVisualisation(fm);});
    return ActionVisualise::doAction(fvs, mc);
}   // end doAction

/*
void ActionEditLandmarks::refreshVisualisation( const FM* fm)
{
    _vis->refresh(fm);
    const FVS& fvs = fm->fvs();
    std::for_each( std::begin(fvs), std::end(fvs), [=](FV* fv){ _vis->apply(fv);});
}   // end refreshVisualisation
*/


void ActionEditLandmarks::doAfterAction( EventSet& cs, const FVS& fvs, bool v)
{
    ActionVisualise::doAfterAction( cs, fvs, v);
    _interactor->setEnabled(isChecked());
}   // end doAfterAction


void ActionEditLandmarks::doOnEditedLandmark( const FV* fv)
{
    EventSet cset;
    cset.insert(LANDMARKS_CHANGE);
    FVS fvs;
    fvs.insert(const_cast<FV*>(fv));
    emit reportFinished( cset, fvs, true);
}   // end doOnEditedLandmark
