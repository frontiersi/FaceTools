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
#include <FaceModel.h>
#include <algorithm>
#include <ObjModelAligner.h>
using FaceTools::Action::ActionAlignLandmarks;
using FaceTools::Action::EventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FVS;
using FaceTools::FaceModelSet;
using FaceTools::Vis::FV;
using FaceTools::LandmarkSet;
using FaceTools::FaceModel;


namespace {

// Check the landmark sets in the given set of models and on return, set fms to
// only contain those models that share at least three landmarks in common (by name)
// from the given set of landmarks lmks. Returns the changed size of fms.
size_t findSharedLandmarksModels( FaceModelSet& fms, const LandmarkSet::Ptr& lmks)
{
    FaceModelSet iset = fms;    // Copy
    fms.clear();
    for ( FaceModel* fm : iset)
    {
        fm->lockForRead();
        LandmarkSet::Ptr lfm = fm->landmarks();
        size_t scount = 0;  // The shared count

        for ( const std::string& lname : lmks->names())
        {
            if ( lfm->has(lname))
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


// For each of these models, look at the source landmarks, and remove from lmset those that don't appear.
size_t findCommonLandmarks( std::unordered_set<std::string>& lmset, const FaceModelSet& fms)
{
    std::unordered_set<std::string> remset;
    for ( const FaceModel* fm : fms)
    {
        fm->lockForRead();
        LandmarkSet::Ptr tlmks = fm->landmarks();
        for ( const std::string& lmname : lmset)
        {
            if ( !tlmks->has(lmname))
                remset.insert(lmname);  // Flag for removal - landmark not present in this model.
        }   // end for
        fm->unlock();
    }   // end for
    // Remove the flagged landmark names
    lmset.erase( remset.begin(), remset.end());
    return lmset.size();
}   // end findCommonLandmarks


// Make an ObjModel from the subset of landmarks of lmks given by lmset.
RFeatures::ObjModel::Ptr makeModelFromLandmarks( const LandmarkSet::Ptr& lmks, const std::unordered_set<std::string>& lmset)
{
    RFeatures::ObjModel::Ptr lmodel = RFeatures::ObjModel::create();
    for ( const std::string& lmname : lmset)
        lmodel->addVertex( lmks->pos(lmname));
    return lmodel;
}   // end makeModelFromLandmarks

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
        FaceModel* fm = fv->data();
        fm->lockForRead();
        LandmarkSet::Ptr lmks = fm->landmarks();
        if ( lmks->size() >= 3) // Need at least three landmarks on the selected model
        {
            // Get the other models from the viewer.
            FaceModelSet fms = fv->viewer()->attached().models();
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
    FaceModel* sfm = fv->data();    
    rset.erase(fv); // Won't actually do work on the source FaceView!

    // Find the landmarks common across all models in the viewer.
    sfm->lockForRead();
    std::unordered_set<std::string> lmset = sfm->landmarks()->names();  // Initially all (copy out)
    sfm->unlock();
    FaceModelSet fms = fv->viewer()->attached().models();
    fms.erase(sfm); // Erase the source
    const size_t ncommon = findCommonLandmarks( lmset, fms);
    assert(ncommon >= 3);   // Otherwise this shouldn't have been enabled!
    if ( ncommon < 3)
        return false;

    // Create the source model to align against
    RFeatures::ObjModel::Ptr lmodel = makeModelFromLandmarks( sfm->landmarks(), lmset);
    RFeatures::ObjModelAligner::Ptr aligner = RFeatures::ObjModelAligner::create( lmodel);

    // Look at the other models and align to the source
    for ( FaceModel* fm : fms)
    {
        fm->lockForWrite();
        RFeatures::ObjModel::Ptr m0 = makeModelFromLandmarks( fm->landmarks(), lmset);
        cv::Matx44d T = aligner->calcTransform( m0.get());
        fm->transform(T);
        // Set the FaceViews adjusted as a result
        for ( auto* f : fm->fvs())
            rset.insert(f);    // Worked on this view!
        fm->unlock();
    }   // end for

    return true;
}   // end doAction
