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

#ifndef FACETOOLS_CORRESPONDENCE_FINDER_H
#define FACETOOLS_CORRESPONDENCE_FINDER_H

#include "ObjMetaData.h"
#include "FaceSampler.h"    // Use ObjModelRemesher

namespace FaceTools
{

class FaceTools_EXPORT CorrespondenceFinder
{
public:
    // Models called with find are corresponded to the reference model given here.
    // The reference model is a sampling of nc points of the given Object. If sampleUniformly
    // is set to false, a curvature dependent sampling method will be used to sample higher
    // curvature areas more densely.
    typedef boost::shared_ptr<CorrespondenceFinder> Ptr;
    static Ptr create( const ObjMetaData::Ptr, int nc, bool sampleUniformly=true);

    const RFeatures::ObjModel::Ptr getReferencePoints() const { return _refobj;}

    // Find correspondences on the model with others already added. Model may be realigned on return
    // according to high curvature points (first added model will be used as the one to align subsequent
    // model high curvature vertices against). Requires a curvature map on the model or will return false.
    void findCorrespondences( const RFeatures::ObjModelKDTree::Ptr);

    const cv::Mat_<cv::Vec3f>& getCorrespondencePoints() const { return _cmat;}
    size_t getNumModels() const { return _cmat.rows;}   // # rows == # calls to findCorrespondences().

    // Build the model - including vertices and faces from the given set of points.
    // vtxs.size() must be equal to the number of correspondence points.
    RFeatures::ObjModel::Ptr buildModel( const std::vector<cv::Vec3f>& vtxs) const;


private:
    FaceSampler *_faceSampler;
    RFeatures::ObjModel::Ptr _refobj;   // Reference vertices from face sampling (no triangles).
    cv::Mat_<cv::Vec3f> _cmat;          // M model rows, with N correspondence point columns (3 channel).

    CorrespondenceFinder( const ObjMetaData::Ptr, int nc, bool sampleUniformly);
    ~CorrespondenceFinder();
    CorrespondenceFinder( const CorrespondenceFinder&); // NO COPY
    void operator=( const CorrespondenceFinder&);       // NO COPY
    class Deleter;
};  // end class

}   // end namespace

#endif
