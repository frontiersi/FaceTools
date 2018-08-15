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

#include <ActionAlignICP.h>
#include <FaceModelViewer.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <algorithm>
#include <ObjModelAligner.h>
using FaceTools::Action::ActionAlignICP;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
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
    FaceControl* fc = ready1();
    return fc && fc->viewer()->attached().size() >= 2;
}   // end testEnabled


bool ActionAlignICP::doAction( FaceControlSet& rset, const QPoint&)
{
    assert(rset.size() == 1);
    FaceControl* fc = rset.first();
    FaceModel* sfm = fc->data();    
    rset.erase(fc); // Won't actually do work on the source FaceControl!

    // Get the source model to align against
    sfm->lockForRead();
    RFeatures::ObjModelAligner aligner( sfm->info()->cmodel());
    sfm->unlock();

    // In the same viewer, look at every other model and align to the source.
    const FaceControlSet& aset = fc->viewer()->attached();
    for ( FaceControl* fc : aset)
    {
        FaceModel* fm = fc->data();
        fm->lockForWrite();
        cv::Matx44d T = aligner.calcTransform( fm->info()->cmodel());
        fm->transform(T);
        rset.insert(fc);    // Worked on this view!
        fm->unlock();
    }   // end for

    return true;
}   // end doAction
