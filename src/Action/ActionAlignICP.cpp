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

#include <ActionAlignICP.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <algorithm>
#include <ObjModelAligner.h>
using FaceTools::Action::ActionAlignICP;
using FaceTools::Action::EventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FaceModel;


ActionAlignICP::ActionAlignICP( const QString& dn, const QIcon& ico, QProgressBar* pb)
    : FaceAction(dn, ico)
{
    if ( pb)
        setAsync(true, QTools::QProgressUpdater::create(pb));
}   // end ctor


bool ActionAlignICP::testEnabled( const QPoint*) const
{
    // Enabled only if a single model is selected and its viewer has other models.
    const FV* fv = ready1();
    return fv && fv->viewer()->attached().size() >= 2;
}   // end testEnabled


bool ActionAlignICP::doAction( FVS& rset, const QPoint&)
{
    assert(rset.size() == 1);
    FV* fv = rset.first();
    FaceModel* sfm = fv->data();    
    rset.erase(fv); // Won't actually do work on the source FaceView!

    // Get the source model to align against
    sfm->lockForRead();
    RFeatures::ObjModelAligner aligner( sfm->info()->cmodel());
    sfm->unlock();

    // In the same viewer, look at every other model and align to the source.
    const FVS& aset = fv->viewer()->attached();
    for ( FV* fv : aset)
    {
        FaceModel* fm = fv->data();
        fm->lockForWrite();
        cv::Matx44d T = aligner.calcTransform( fm->info()->cmodel());
        fm->transform(T);
        rset.insert(fv);    // Worked on this view!
        fm->unlock();
    }   // end for

    return true;
}   // end doAction
