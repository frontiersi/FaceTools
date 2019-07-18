/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <ActionSmooth.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <ObjModelSmoother.h>   // RFeatures
#include <FaceModelCurvature.h>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionSmooth;
using FaceTools::Action::Event;
using FaceTools::FaceModelCurvature;
using MS = FaceTools::Action::ModelSelector;


ActionSmooth::ActionSmooth( const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico), _maxc(0.8)
{
    setAsync(true);
}   // end ctor


bool ActionSmooth::checkEnable( Event)
{
    const FM* fm = MS::selectedModel();
    return fm && FaceModelCurvature::rmetrics(fm) != nullptr;
}   // end checkEnabled


bool ActionSmooth::doBeforeAction(Event)
{
    MS::showStatus( "Smoothing model surface...");
    storeUndo( this, {Event::GEOMETRY_CHANGE, Event::LANDMARKS_CHANGE});
    return true;
}   // end doBeforeAction


void ActionSmooth::doAction( Event)
{
    using namespace RFeatures;
    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    ObjModel::Ptr model = fm->wmodel();
    const ObjModelManifolds& manfs = fm->manifolds();
    FaceModelCurvature::WPtr cmap = FaceModelCurvature::wmetrics(fm);   // Curvature write lock obtained via shared ptr mechanic (released on return)
    ObjModelSmoother smoother( *model, *cmap, manfs);
    smoother.smooth( maxCurvature());   // Updates curvature data for the model but should be reconstructed anyway.
    fm->update( model, false);
    fm->moveLandmarksToSurface();
    fm->unlock();
}   // end doAction


void ActionSmooth::doAfterAction( Event)
{
    MS::showStatus("Finished smooth.", 5000);
    emit onEvent( {Event::GEOMETRY_CHANGE, Event::LANDMARKS_CHANGE});
}   // end doAfterAction

