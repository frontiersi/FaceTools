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

#include <ActionRotateModel.h>
#include <Eigen/Geometry>
#include <FaceModel.h>
using FaceTools::Action::ActionRotateModel;
using FaceTools::Action::FaceAction;
using FaceTools::FVS;
using FaceTools::FM;


ActionRotateModel::ActionRotateModel( const QString &dn, const QIcon& ico, const cv::Vec3f& raxis, float degs)
    : FaceAction( dn, ico)
{
    // Ensure the rotation axis is normalized first
    Eigen::Vector3f axis( raxis[0], raxis[1], raxis[2]);
    axis.normalize();
    Eigen::Matrix3f m;
    m = Eigen::AngleAxisf( degs * static_cast<float>(CV_PI/180), axis);
    _rmat = cv::Matx44d( m(0,0), m(0,1), m(0,2), 0,
                         m(1,0), m(1,1), m(1,2), 0,
                         m(2,0), m(2,1), m(2,2), 0,
                              0,      0,      0, 0);
}   // end ctor


bool ActionRotateModel::doAction( FVS& fvs, const QPoint&)
{
    FMS fms = fvs.models(); // Copy out
    for ( FM* fm : fms)
    {
        fm->lockForWrite();
        fm->transform(_rmat);
        fm->unlock();
        fvs.insert(fm);
    }   // end for
    return true;
}   // end doAction
