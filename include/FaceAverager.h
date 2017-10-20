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

#ifndef FACE_TOOLS_FACE_AVERAGER_H
#define FACE_TOOLS_FACE_AVERAGER_H

#include <string>
#include <vector>
#include <ObjModel.h>   // RFeatures
#include "FaceTools_Export.h"


namespace FaceTools
{

class FaceTools_EXPORT FaceAverager
{
public:
    typedef boost::shared_ptr<FaceAverager> Ptr;

    // Create averages of added faces using n uniformly interpolated points per face.
    static Ptr create( int n);
    explicit FaceAverager( int n);
    virtual ~FaceAverager();

    // Performs point to plane ICP around face centre, then samples the n closest
    // points on the model to the existing n mean points, adjusting the average.
    // Closest points are intepolated for heightened accuracy.
    int add( const RFeatures::ObjModel::Ptr);

    RFeatures::ObjModel::Ptr get() { return _model;}    // The average model from those added

    // Return the correspondence points
    const cv::Mat_<cv::Vec3f> getCorrespondencePoints() const { return _cmat;}

private:
    cv::Mat_<cv::Vec3f> _cmat;                   // M model rows, with N corresponding points (3 channel)
    RFeatures::ObjModel::Ptr _model;             // Uniformly sampled model

    void init( const RFeatures::ObjModel::Ptr);
    FaceAverager( const FaceAverager&);     // No copy
    void operator=( const FaceAverager&);   // No copy
};  // end class

}   // end namespace

#endif
