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

#include <ActionAlignLandmarks.h>
#include <FaceModelViewer.h>
#include <FaceTools.h>
#include <FaceModel.h>
#include <algorithm>
#include <ObjModelAligner.h>
#include <Transformer.h>
using FaceTools::Action::ActionAlignLandmarks;
using FaceTools::Action::EventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FVS;
using FaceTools::FMS;
using FaceTools::Vis::FV;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::FM;
using RFeatures::ObjModel;

namespace {

// Check the landmark sets in the given set of models and on return, set fms to
// only contain those models that share at least three landmarks in common (by name)
// from the given set of landmarks lmks. Returns the changed size of fms.
size_t findSharedLandmarksModels( FMS& fms, const LandmarkSet::Ptr& lmks)
{
    FMS iset = fms;    // Copy
    fms.clear();
    for ( FM* fm : iset)
    {
        fm->lockForRead();
        LandmarkSet::Ptr lfm = fm->landmarks();
        size_t scount = 0;  // The shared count

        for ( int id : lmks->ids())
        {
            if ( lfm->has(id))
            {
                scount++;
                if ( scount == 3)
                    break;
            }   // end if
        }   // end for

        if ( scount == 3)
            fms.insert(fm);
        fm->unlock();
    }   // end for
    return fms.size();
}   // end findSharedLandmarksModels


// Make an ObjModel from the subset of landmarks of lmks given by clmks and correspond the added vertex IDs to the originating landmark IDs.
ObjModel::Ptr makeModelFromLandmarks( const LandmarkSet::Ptr& lmks, const std::vector<int>& clmks)
{
    ObjModel::Ptr mod = ObjModel::create();
    for ( int id : clmks)
    {
        if ( LDMKS_MAN::landmark(id)->isBilateral())
        {
            mod->addVertex( *lmks->pos(id, FaceTools::FACE_LATERAL_LEFT));
            mod->addVertex( *lmks->pos(id, FaceTools::FACE_LATERAL_RIGHT));
        }   // end if
        else
            mod->addVertex( *lmks->pos(id, FaceTools::FACE_LATERAL_MEDIAL));
    }   // end for
    return mod;
}   // end makeModelFromLandmarks


ObjModel::Ptr makeMeanLandmarksModel( const std::vector<ObjModel::Ptr>& lms, double &rms)
{
    assert(lms.size() > 0);

    ObjModel::Ptr nm = ObjModel::create();

    // For each vertex from the provided models, set the new landmark vertex as the mean from all the models.
    const double nfactor = 1.0 / static_cast<int>(lms.size());
    const int n = static_cast<int>((*lms.begin())->getNumVertices());
    for ( int vidx = 0; vidx < n; ++vidx)   // Okay to do this since all vertex IDs [0,n-1] will be present
    {
        cv::Vec3d v(0,0,0);
        for ( ObjModel::Ptr m : lms)
            v += m->vtx(vidx);
        v *= nfactor;
        nm->addVertex(v);
    }   // end for

    // Calculate the new root mean square error of displacement
    rms = 0;
    for ( int vidx = 0; vidx < n; ++vidx)
    {
        const cv::Vec3f& mv = nm->vtx(vidx);
        for ( ObjModel::Ptr m : lms)
        {
            const cv::Vec3f& v = m->vtx(vidx);
            rms += double(powf(v[0] - mv[0],2) + powf(v[1] - mv[1],2) + powf(v[2] - mv[2],2));
        }   // end for
    }   // end for
    rms = sqrt( rms / (n*int(lms.size())));

    return nm;
}   // end makeMeanLandmarksModel

}   // end namespace


ActionAlignLandmarks::ActionAlignLandmarks( const QString& dn, const QIcon& ico, QProgressBar* pb)
    : FaceAction(dn, ico)
{
    if ( pb)
        setAsync(true, QTools::QProgressUpdater::create(pb));
}   // end ctor


bool ActionAlignLandmarks::testEnabled( const QPoint*) const
{
    // Enabled only if the selected viewer has other models
    // and the other models share at least three landmarks in common.
    const FV* fv = ready1();
    bool ready = false;
    if ( fv && fv->viewer()->attached().size() >= 2)
    {
        FM* fm = fv->data();
        fm->lockForRead();
        LandmarkSet::Ptr lmks = fm->landmarks();
        if ( lmks->size() >= 3) // Need at least three landmarks on the selected model
        {
            // Get the other models from the viewer.
            FMS fms = fv->viewer()->attached().models();
            fms.erase(fm);  // Remember not to include the source model.
            // If at least one other model in the viewer with three or more shared landmarks, then enable this action.
            ready = findSharedLandmarksModels( fms, lmks) > 0;
        }   // end if
        fm->unlock();
    }   // end if
    return ready;
}   // end testEnabled



bool ActionAlignLandmarks::doAction( FVS& rset, const QPoint&)
{
    assert(rset.size() == 1);
    FV* fv = rset.first();
    FM* sfm = fv->data();   // Source model

    // Find the landmarks common across all models in the viewer.
    std::vector<int> clmks;
    FMS fms = fv->viewer()->attached().models();
    const size_t ncommon = findCommonLandmarks( clmks, fms);
    assert(ncommon >= 3);   // Otherwise this shouldn't have been enabled!
    if ( ncommon < 3)
        return false;

    const size_t n = fms.size();

    // Create the initial sets of common landmarks models associated with each FaceModel
    ObjModel::Ptr mlmks;    // Will be the "mean" landmark model updated at each iteration.
    std::vector<ObjModel::Ptr> lmodels, olmodels;   // lmodels is updated while olmodels holds a copy of the original.
    std::vector<FM*> fmodels;
    for ( FM* fm : fms)
    {
        fmodels.push_back(fm);
        fm->lockForRead();
        ObjModel::Ptr mod = makeModelFromLandmarks( fm->landmarks(), clmks);
        fm->unlock();
        lmodels.push_back( mod);
        olmodels.push_back( ObjModel::copy(mod.get()));
        if ( fm == sfm) // Ensure the first target landmark model to superimpose against is from the selected FaceModel.
            mlmks = mod;
    }   // end for

    double tol = 0.0001;
    double rms = DBL_MAX;
    double rmsDelta = DBL_MAX;
    while ( rmsDelta >= tol)
    {
        RFeatures::ObjModelProcrustesSuperimposition aligner( mlmks.get());

        // Align each of landmark models to the mean landmark model.
        for ( size_t i = 0; i < n; ++i)
        {
            ObjModel::Ptr mod = lmodels.at(i);
            cv::Matx44d T = aligner.calcTransform( mod.get());    // Calculate the transform to the source landmark model
            // Transform this landmark model to be superimposed against the current mean landmark model.
            RFeatures::Transformer transformer(T);
            transformer.transform(mod);
        }   // end for

        rmsDelta = rms;
        mlmks = makeMeanLandmarksModel( lmodels, rms);
        rmsDelta -= rms;
        std::cerr << "RMS: " << rms << " (delta = " << rmsDelta << ")" << std::endl;
    }   // end while

    // Calculate the transforms using the final mean landmark model created and the original untransformed landmark models.
    RFeatures::ObjModelProcrustesSuperimposition aligner( mlmks.get());

    rset.clear();
    for ( size_t i = 0; i < n; ++i)
    {
        ObjModel::Ptr lmod = olmodels.at(i);
        cv::Matx44d T = aligner.calcTransform( lmod.get());
        FM* fm = fmodels[i];
        fm->transform( T);
        // Set the FaceViews adjusted as a result
        for ( auto* f : fm->fvs())
            rset.insert(f);    // Worked on this view!
    }   // end for

    return true;
}   // end doAction
