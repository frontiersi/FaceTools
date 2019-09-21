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

#include <Action/ActionAlignICP.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <algorithm>
#include <ObjModelICPAligner.h>
#include <FaceModelCurvature.h>
using FaceTools::Action::ActionAlignICP;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionAlignICP::ActionAlignICP( const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico)
{
    setAsync(true);
}   // end ctor


bool ActionAlignICP::checkEnable( Event)
{
    // Enabled only if a model is selected and its viewer has other models.
    const FV* fv = MS::selectedView();
    if ( !fv || fv->viewer()->attached().size() < 2)
        return false;

    // Curvature maps need to be ready for all of the models in the viewer
    const FMS& fms = fv->viewer()->attached().models();
    for ( const FM* fm : fms)
        if ( FaceModelCurvature::rmetrics( fm) == nullptr)
            return false;

    return true;
}   // end checkEnabled


bool ActionAlignICP::doBeforeAction( Event)
{
    MS::showStatus( "Performing ICP alignment against selected model...");
    return true;
}   // end doBeforeAction


void ActionAlignICP::doAction( Event)
{
    storeUndo(this, {Event::AFFINE_CHANGE, Event::ALL_VIEWS});

    const FV* fv = MS::selectedView();
    const FM* sfm = fv->data();

    // Get the source model to align against
    sfm->lockForRead();
    RFeatures::ObjModelICPAligner aligner( sfm->model(), *FaceModelCurvature::rmetrics( sfm));
    sfm->unlock();

    // In the same viewer, look at every other model and align to the source.
    FMS fms = fv->viewer()->attached().models();
    fms.erase( const_cast<FM*>(sfm));
    for ( FM* fm : fms)
    {
        fm->lockForWrite();
        const cv::Matx44d tmat = aligner.calcTransform( fm->model(), *FaceModelCurvature::rmetrics( fm));
        fm->addTransformMatrix( tmat);
        fm->unlock();
    }   // end for
}   // end doAction


void ActionAlignICP::doAfterAction( Event)
{
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    emit onEvent( {Event::AFFINE_CHANGE, Event::ALL_VIEWS});
    MS::showStatus( "Finished aligning.", 5000);
}   // end doAfterAction

