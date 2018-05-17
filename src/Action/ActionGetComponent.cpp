/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
#include <FaceControl.h>
#include <FaceModel.h>
#include <ObjModelCopier.h> // RFeatures
#include <cassert>
using FaceTools::Action::ActionGetComponent;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionGetComponent::ActionGetComponent( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico, true/*disable before other*/)
{
    addChangeTo( MODEL_GEOMETRY_CHANGED);
    addRespondTo( LANDMARK_ADDED);
    addRespondTo( LANDMARK_DELETED);
    addRespondTo( LANDMARK_CHANGED);
}   // end ctor


bool ActionGetComponent::testReady( FaceControl* fc)
{
    return fc->data()->landmarks().has( FaceTools::Landmarks::NASAL_TIP);
}   // end testReady


bool ActionGetComponent::doAction( FaceControlSet& rset)
{
    using namespace RFeatures;
    const FaceModelSet& fms = rset.models();
    for ( FaceModel* fm : fms)
    {
        int svidx = fm->kdtree().find( fm->landmarks().pos( FaceTools::Landmarks::NASAL_TIP));
        const ObjModel* model = fm->cmodel();
        int fidx = *model->getFaceIds(svidx).begin();    // Get a polygon attached to this vertex

        // Find which of the components of the model has this polygon as a member
        int foundC = -1;
        const ObjModelInfo& info = fm->info();
        int nc = info.components().size();
        for ( int c = 0; c < nc; ++c)
        {
            const IntSet* fids = info.components().componentPolygons(c);
            assert(fids);
            if ( fids->count(fidx) > 0)
            {
                foundC = c;
                break;
            }   // end if
        }   // end for

        assert(foundC >= 0);
        const IntSet* cfids = info.components().componentPolygons(foundC);
        assert( cfids);
        ObjModelCopier copier( model);
        std::for_each( std::begin(*cfids), std::end(*cfids), [&](int fid){ copier.addTriangle(fid);});
        fm->updateData( copier.getCopiedModel());
    }   // end for
    return true;
}   // end doAction
