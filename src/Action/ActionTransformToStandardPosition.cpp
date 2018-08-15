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

#include <ActionTransformToStandardPosition.h>
#include <FaceShapeLandmarks2DDetector.h>   // FaceTools::Landmarks
#include <Transformer.h>  // RFeatures
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <FaceView.h>
#include <VtkTools.h>
using FaceTools::Action::ActionTransformToStandardPosition;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceModel;

namespace {

// Find and return index to largest rectangular volume from the given vector of bounds.
int findLargest( const std::vector<cv::Vec6d>& bounds)
{
    int j = 0;
    double maxA = 0;    // Max area
    int n = (int)bounds.size();
    for ( int i = 0; i < n; ++i)
    {
        const cv::Vec6d& b = bounds[i];
        double a = (b[1] - b[0]) * (b[3] - b[2]) * (b[5] - b[4]);
        if ( a > maxA)
        {
            maxA = a;
            j = i;
        }   // end if
    }   // end for
    return j;
}   // end findLargest
}   // end namespace



ActionTransformToStandardPosition::ActionTransformToStandardPosition( const QString &dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
}   // end ctor


bool ActionTransformToStandardPosition::doAction( FaceControlSet& rset, const QPoint&)
{
    const FaceModelSet& fms = rset.models();
    for ( FaceModel* fm : fms)
    {
        fm->lockForWrite();

        // Get the transformation centre as calculated from the face centre if landmarks available,
        // or just as the centre point of the largest component otherwise.
        cv::Vec3f c(0,0,0);
        FaceTools::LandmarkSet::Ptr lmks = fm->landmarks();
        if ( FaceTools::hasReqLandmarks( lmks))
        {
            using namespace FaceTools::Landmarks;
            c = FaceTools::calcFaceCentre( lmks->pos(L_EYE_CENTRE), lmks->pos(R_EYE_CENTRE), lmks->pos(NASAL_TIP));
        }   // end if
        else
        {
            const cv::Vec6d& bd = fm->bounds()[ findLargest( fm->bounds())];
            c = cv::Vec3f( (bd[0] + bd[1])/2, (bd[2] + bd[3])/2, (bd[4] + bd[5])/2);
        }   // end else

        const RFeatures::Orientation& on = fm->orientation();
        cv::Matx44d m = RFeatures::toStandardPosition( on.norm(), on.up(), c);
        std::cerr << "Orientation pre-transform (norm,up)  : " << on << std::endl;
        fm->transform(m);
        std::cerr << "Orientation post-transform (norm,up) : " << on << std::endl;

        fm->unlock();
    }   // end for
    return true;
}   // end doAction
