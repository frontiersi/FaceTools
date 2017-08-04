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

#ifndef FACE_TOOLS_NOSE_FINDER_H
#define FACE_TOOLS_NOSE_FINDER_H

#include "FaceTools_Export.h"
#include <ObjModelTools.h>  // RFeatures

namespace FaceTools
{

class FaceTools_EXPORT NoseFinder
{
public:
    // Provide the model curvature map and the vertex indices of the eyes.
    NoseFinder( const RFeatures::ObjModelCurvatureMap::Ptr,
                int leftEyeVertexId, int rightEyeVertexId);

    bool find();
    cv::Vec3f getMidEyesPoint() const { return _midEyes;}
    cv::Vec3f getNoseTip() const { return _ntip;}
    cv::Vec3f getNoseBridge() const { return _nbridge;}
    const std::vector<int>& getNoseRidgePath() const { return _noseRidgePath;}

private:
    const RFeatures::ObjModelCurvatureMap::Ptr _curvMap;
    const RFeatures::ObjModel::Ptr _model;
    const int _e0, _e1;   // Left and right eye centres
    cv::Vec3f _midEyes;   // Vertex midway between the eyes
    cv::Vec3f _nbridge;   // Vertex for nose bridge
    cv::Vec3f _ntip;      // Vertex for nose tip
    std::vector<int> _noseRidgePath;

    int searchForNoseTip( int startVtx, cv::Vec3d& dvec, double degsAllowed);
    bool calcMetrics( int, const cv::Vec3f&, bool, double&, double&) const;
};  // end class

}   // end namespace

#endif
