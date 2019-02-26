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

#include <ActionBackfaceCulling.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Action::ActionBackfaceCulling;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::FMVS;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FMV;


ActionBackfaceCulling::ActionBackfaceCulling( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
    setCheckable( true, false);

    // If the face normal is in the same space half as the orientation normal,
    // then we want to automatically turn on backface culling.
    TestFVSTrue tfvs = []( const FVS& fvs)
    {
        if ( fvs.size() != 1)
            return false;

        // Find a polygon at the glabella
        FM* fm = fvs.first()->data();
        const cv::Vec3f* v = fm->landmarks()->pos( Landmark::G);

        // Don't respond if the landmark isn't present.
        if ( !v)
            return false;

        // Calculate the norm given by the ordering of the vertices on the polygon.
        const RFeatures::ObjModel* model = fm->info()->cmodel();
        int vidx = fm->kdtree()->find(*v);
        const cv::Vec3f fnrm = model->calcFaceNorm( *model->getFaceIds(vidx).begin());

        // If normal in same direction (positive inner product) as orientation, respond (return true).
        const cv::Vec3f onrm = fm->landmarks()->orientation().nvec();

        return onrm.dot(fnrm) > 0;
    };  // end tfvs

    setRespondToEventIf( GEOMETRY_CHANGE, tfvs, true);
    setRespondToEventIf( ORIENTATION_CHANGE, tfvs, true);
    //setRespondToEventIf( VIEWER_CHANGE, tfvs, true);
}   // end ctor


bool ActionBackfaceCulling::testIfCheck( const FV* fv) const
{
    return fv && fv->backfaceCulling();
}   // end testIfCheck


bool ActionBackfaceCulling::doAction( FVS& fvs, const QPoint&)
{
    // Apply to all FaceViews in all directly selected viewers.
    const bool ischecked = isChecked();
    FMVS fmvs = fvs.dviewers();
    fvs.clear();
    for ( const FMV* v : fmvs)
    {
        for ( FV* f : v->attached())
        {
            f->setBackfaceCulling( ischecked);
            fvs.insert(f);
        }   // end for
    }   // end for
    return true;
}   // end doAction
