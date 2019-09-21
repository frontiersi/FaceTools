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

#include <Action/ActionAlignVertices.h>
#include <FaceModelViewer.h>
#include <FaceTools.h>
#include <FaceModel.h>
#include <algorithm>
#include <ObjModelAligner.h>
#include <Transformer.h>
using FaceTools::Action::ActionAlignVertices;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::FMS;
using FaceTools::Vis::FV;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::FM;
using RFeatures::ObjModel;
using MS = FaceTools::Action::ModelSelector;


ActionAlignVertices::ActionAlignVertices( const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico)
{
    setAsync(true);
}   // end ctor


bool ActionAlignVertices::checkEnable( Event)
{
    // Enabled only if the selected viewer has other models with the same number of
    // vertices and those vertices are also in sequential order (which should always be the case).
    const FV* fv = MS::selectedView();
    const FM* fm = nullptr;
    int nvs = -1;
    if ( fv)
    {
        fm = fv->data();
        fm->lockForRead();
        assert( fm->model().hasSequentialVertexIds());
        nvs = fm->model().numVtxs();
    }   // end if

    bool ready = false;
    if ( fm && fv->viewer()->attached().size() >= 2)
    {
        FMS fms = fv->viewer()->attached().models(); // Get other models from the viewer.
        fms.erase(const_cast<FM*>(fm));

        // If all other models have the same number of vertices, then enable this action.
        ready = true;
        for ( const FM* fm2 : fms)
        {
            fm2->lockForRead();
            const int nv = fm2->model().numVtxs();
            fm2->unlock();
            if ( nv != nvs)
            {
                ready = false;
                break;
            }   // end if
        }   // end for
    }   // end if

    if ( fm)
        fm->unlock();

    return ready;
}   // end checkEnabled


bool ActionAlignVertices::doBeforeAction( Event)
{
    MS::showStatus( "Aligning other model vertices to selected model's vertices...");
    return true;
}   // end doBeforeAction


void ActionAlignVertices::doAction( Event)
{
    storeUndo(this, {Event::AFFINE_CHANGE, Event::ALL_VIEWS});
    FV* fv = MS::selectedView();

    FM* tfm = fv->data();   // Target model that we want the other models to align with
    FMS fms = fv->viewer()->attached().models(); // Get models from the same viewer as the target model
    fms.erase(tfm); // Ensure the target model is removed

    const std::vector<double> weights( size_t( tfm->model().numVtxs()), 1.0);
    RFeatures::ObjModelProcrustesSuperimposition aligner( tfm->model(), weights);

    // Align each model to the target model.
    for ( FM* fm : fms)
    {
        fm->lockForWrite();
        const ObjModel& lmod = fm->model();
        cv::Matx44d T = aligner.calcTransform( lmod);    // Calculate transform to the target landmarks
        fm->addTransformMatrix( T);
        fm->unlock();
    }   // end for
}   // end doAction


void ActionAlignVertices::doAfterAction( Event)
{
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    MS::showStatus( "Finished vertex alignment.", 5000);
    emit onEvent( {Event::AFFINE_CHANGE, Event::ALL_VIEWS});
}   // end doAfterAction

