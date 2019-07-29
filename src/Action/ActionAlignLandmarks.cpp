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

#include <ActionAlignLandmarks.h>
#include <FaceModelViewer.h>
#include <FaceTools.h>
#include <FaceModel.h>
#include <algorithm>
#include <ObjModelAligner.h>
#include <Transformer.h>
using FaceTools::Action::ActionAlignLandmarks;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::FMS;
using FaceTools::Vis::FV;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::FM;
using RFeatures::ObjModel;
using MS = FaceTools::Action::ModelSelector;

namespace {

ObjModel::Ptr makeLandmarksModel( const LandmarkSet& lmks)
{
    ObjModel::Ptr mod = ObjModel::create();

    // Sort the landmark ids so that the vertices of a landmarks model are always added in the same order.
    const IntSet& lmids = lmks.ids();
    std::vector<int> ids( std::begin(lmids), std::end(lmids));
    std::sort( std::begin(ids), std::end(ids));

    for ( int id : ids)
    {
        if ( LDMKS_MAN::landmark(id)->isBilateral())
        {
            mod->addVertex( lmks.pos(id, FaceTools::FACE_LATERAL_LEFT));
            mod->addVertex( lmks.pos(id, FaceTools::FACE_LATERAL_RIGHT));
        }   // end if
        else
            mod->addVertex( lmks.pos(id, FaceTools::FACE_LATERAL_MEDIAL));
    }   // end for
    return mod;
}   // end makeLandmarksModel

/*
// Calculate and return the root mean square error of displacement over all models.
double calcRMS( const ObjModel* tmod, const std::vector<ObjModel::Ptr>& lms)
{
    assert(!lms.empty());

    double rms = 0;
    const int n = tmod->numVtxs();
    for ( int vidx = 0; vidx < n; ++vidx)
    {
        const cv::Vec3f& tv = tmod->vtx(vidx);
        for ( ObjModel::Ptr m : lms)
        {
            const cv::Vec3f& v = m->vtx(vidx);
            rms += double(powf(v[0] - tv[0],2) + powf(v[1] - tv[1],2) + powf(v[2] - tv[2],2));
        }   // end for
    }   // end for
    rms = sqrt( rms / (n*int(lms.size())));

    return rms;
}   // end calcRMS
*/

}   // end namespace


ActionAlignLandmarks::ActionAlignLandmarks( const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico)
{
    setAsync(true);
}   // end ctor


bool ActionAlignLandmarks::checkEnable( Event)
{
    // Enabled only if the selected viewer has other models with landmarks.
    const FV* fv = MS::selectedView();
    bool ready = false;
    if ( fv && fv->viewer()->attached().size() >= 2)
    {
        const FM* fm = fv->data();
        fm->lockForRead();
        const LandmarkSet& lmks = fm->currentAssessment()->landmarks();
        if ( !lmks.empty())
        {
            // Get the other models from the viewer.
            FMS fms = fv->viewer()->attached().models();
            fms.erase(const_cast<FM*>(fm));
            // If at least one other model in the viewer with landmarks, then enable this action.
            for ( const FM* fm2 : fms)
            {
                fm2->lockForRead();
                ready = !fm2->currentAssessment()->landmarks().empty();
                fm2->unlock();
                if ( ready)
                    break;
            }   // end for
        }   // end if
        fm->unlock();
    }   // end if
    return ready;
}   // end checkEnabled


bool ActionAlignLandmarks::doBeforeAction( Event)
{
    MS::showStatus( "Aligning to landmarks on selected face...");
    return true;
}   // end doBeforeAction


void ActionAlignLandmarks::doAction( Event)
{
    storeUndo(this, {Event::AFFINE_CHANGE, Event::ALL_VIEWS});
    FV* fv = MS::selectedView();

    FM* tfm = fv->data();   // Target model whose landmarks we want the other models' landmarks to align with.
    tfm->lockForRead();
    ObjModel::Ptr tlmks = makeLandmarksModel( tfm->currentAssessment()->landmarks());    // Create the target landmarks model
    tfm->unlock();

    FMS fms = fv->viewer()->attached().models(); // Get models from the same viewer as the target model
    fms.erase(tfm); // Ensure the target model is removed

    // Create the landmarks models for each FaceModel that isn't the target model
    std::unordered_map<FM*, ObjModel::Ptr> lmods;
    for ( FM* fm : fms)
    {
        fm->lockForRead();
        lmods[fm] = makeLandmarksModel( fm->currentAssessment()->landmarks());
        fm->unlock();
    }   // end for

    const std::vector<double> weights( size_t(tlmks->numVtxs()), 1.0);
    RFeatures::ObjModelProcrustesSuperimposition aligner( *tlmks, weights);

    // Align each of the landmark models to the target landmark model and use the calculated transform on the whole model.
    for ( const auto& p : lmods)
    {
        const ObjModel& lmod = *p.second;
        const cv::Matx44d T = aligner.calcTransform( lmod);    // Calculate transform to the target landmarks
        FM* fm = p.first;
        fm->lockForWrite();
        fm->addTransformMatrix( T);
        fm->unlock();
    }   // end for
}   // end doAction


void ActionAlignLandmarks::doAfterAction( Event)
{
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    MS::showStatus( "Finished alignment by landmarks.", 5000);
    emit onEvent( {Event::AFFINE_CHANGE, Event::ALL_VIEWS});
}   // end doAfterAction

