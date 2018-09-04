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

#include <ActionGetComponent.h>
#include <FaceShapeLandmarks2DDetector.h>   // FaceTools::Landmarks
#include <FaceModel.h>
#include <ObjModelCopier.h> // RFeatures
#include <cassert>
using FaceTools::Action::ActionGetComponent;
using FaceTools::Action::EventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FaceModel;


ActionGetComponent::ActionGetComponent( const QString& dn, const QIcon& ico, QProgressBar* pb)
    : FaceAction( dn, ico)
{
    if ( pb)
        setAsync(true, QTools::QProgressUpdater::create(pb));
}   // end ctor


bool ActionGetComponent::testReady( const FV* fv)
{
    const FaceModel* fm = fv->data();
    fm->lockForRead();
    const FaceTools::LandmarkSet::Ptr lmks = fm->landmarks();
    const bool rval = lmks->has( FaceTools::Landmarks::NASAL_TIP) && fm->info()->components().size() > 1;
    fm->unlock();
    return rval;
}   // end testReady


bool ActionGetComponent::doAction( FVS& rset, const QPoint&)
{
    bool success = true;
    using namespace RFeatures;
    const FaceModelSet& fms = rset.models();
    for ( FaceModel* fm : fms)
    {
        fm->lockForRead();
        const FaceTools::LandmarkSet::Ptr lmks = fm->landmarks();
        int svidx = fm->kdtree()->find( lmks->pos( FaceTools::Landmarks::NASAL_TIP));

        ObjModelInfo::Ptr info = fm->info();
        int fidx = *info->cmodel()->getFaceIds(svidx).begin();    // Get a polygon attached to this vertex

        // Find which of the components of the model has this polygon as a member
        int foundC = -1;
        int nc = info->components().size();
        for ( int c = 0; c < nc; ++c)
        {
            const IntSet* fids = info->components().componentPolygons(c);
            if ( fids->count(fidx) > 0)
            {
                foundC = c;
                break;
            }   // end if
        }   // end for

        assert(foundC >= 0);
        const IntSet* cfids = info->components().componentPolygons(foundC);
        assert( cfids);
        ObjModelCopier copier( info->cmodel());
        std::for_each( std::begin(*cfids), std::end(*cfids), [&](int fid){ copier.addTriangle(fid);});
        fm->unlock();

        ObjModelInfo::Ptr ninfo = ObjModelInfo::create( copier.getCopiedModel());
        assert( ninfo);
        if ( !ninfo)
        {
            std::cerr << "[ERROR] FaceTools::ActionGetComponent::doAction: Unable to create new model from component!" << std::endl;
            success = false;
            continue;
        }   // end if

        fm->lockForWrite();
        fm->update( ninfo);
        fm->unlock();
    }   // end for
    return success;
}   // end doAction
